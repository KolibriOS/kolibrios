/*
 * Copyright © 2006,2008,2011 Intel Corporation
 * Copyright © 2007 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Wang Zhenyu <zhenyu.z.wang@sna.com>
 *    Eric Anholt <eric@anholt.net>
 *    Carl Worth <cworth@redhat.com>
 *    Keith Packard <keithp@keithp.com>
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sna.h"
#include "sna_reg.h"
#include "sna_render.h"
#include "sna_render_inline.h"
//#include "sna_video.h"

#include "brw/brw.h"
#include "gen7_render.h"
#include "gen4_common.h"
#include "gen4_source.h"
#include "gen4_vertex.h"
#include "gen6_common.h"

#define ALWAYS_INVALIDATE 0
#define ALWAYS_FLUSH 0
#define ALWAYS_STALL 0

#define NO_COMPOSITE 0
#define NO_COMPOSITE_SPANS 0
#define NO_COPY 0
#define NO_COPY_BOXES 0
#define NO_FILL 0
#define NO_FILL_BOXES 0
#define NO_FILL_ONE 0
#define NO_FILL_CLEAR 0

#define NO_RING_SWITCH 0

#define USE_8_PIXEL_DISPATCH 1
#define USE_16_PIXEL_DISPATCH 1
#define USE_32_PIXEL_DISPATCH 0

#if !USE_8_PIXEL_DISPATCH && !USE_16_PIXEL_DISPATCH && !USE_32_PIXEL_DISPATCH
#error "Must select at least 8, 16 or 32 pixel dispatch"
#endif

#define GEN7_MAX_SIZE 16384

/* XXX Todo
 *
 * STR (software tiled rendering) mode. No, really.
 * 64x32 pixel blocks align with the rendering cache. Worth considering.
 */

#define is_aligned(x, y) (((x) & ((y) - 1)) == 0)

struct gt_info {
	const char *name;
	uint32_t max_vs_threads;
	uint32_t max_gs_threads;
	uint32_t max_wm_threads;
	struct {
		int size;
		int max_vs_entries;
		int max_gs_entries;
		int push_ps_size; /* in 1KBs */
	} urb;
	int gt;
};

static const struct gt_info ivb_gt_info = {
	.name = "Ivybridge (gen7)",
	.max_vs_threads = 16,
	.max_gs_threads = 16,
	.max_wm_threads = (16-1) << IVB_PS_MAX_THREADS_SHIFT,
	.urb = { 128, 64, 64, 8 },
	.gt = 0,
};

static const struct gt_info ivb_gt1_info = {
	.name = "Ivybridge (gen7, gt1)",
	.max_vs_threads = 36,
	.max_gs_threads = 36,
	.max_wm_threads = (48-1) << IVB_PS_MAX_THREADS_SHIFT,
	.urb = { 128, 512, 192, 8 },
	.gt = 1,
};

static const struct gt_info ivb_gt2_info = {
	.name = "Ivybridge (gen7, gt2)",
	.max_vs_threads = 128,
	.max_gs_threads = 128,
	.max_wm_threads = (172-1) << IVB_PS_MAX_THREADS_SHIFT,
	.urb = { 256, 704, 320, 8 },
	.gt = 2,
};

static const struct gt_info byt_gt_info = {
	.name = "Baytrail (gen7)",
	.urb = { 128, 64, 64 },
	.max_vs_threads = 36,
	.max_gs_threads = 36,
	.max_wm_threads = (48-1) << IVB_PS_MAX_THREADS_SHIFT,
	.urb = { 128, 512, 192, 8 },
	.gt = 1,
};

static const struct gt_info hsw_gt_info = {
	.name = "Haswell (gen7.5)",
	.max_vs_threads = 8,
	.max_gs_threads = 8,
	.max_wm_threads =
		(8 - 1) << HSW_PS_MAX_THREADS_SHIFT |
		1 << HSW_PS_SAMPLE_MASK_SHIFT,
	.urb = { 128, 64, 64, 8 },
	.gt = 0,
};

static const struct gt_info hsw_gt1_info = {
	.name = "Haswell (gen7.5, gt1)",
	.max_vs_threads = 70,
	.max_gs_threads = 70,
	.max_wm_threads =
		(102 - 1) << HSW_PS_MAX_THREADS_SHIFT |
		1 << HSW_PS_SAMPLE_MASK_SHIFT,
	.urb = { 128, 640, 256, 8 },
	.gt = 1,
};

static const struct gt_info hsw_gt2_info = {
	.name = "Haswell (gen7.5, gt2)",
	.max_vs_threads = 140,
	.max_gs_threads = 140,
	.max_wm_threads =
		(140 - 1) << HSW_PS_MAX_THREADS_SHIFT |
		1 << HSW_PS_SAMPLE_MASK_SHIFT,
	.urb = { 256, 1664, 640, 8 },
	.gt = 2,
};

static const struct gt_info hsw_gt3_info = {
	.name = "Haswell (gen7.5, gt3)",
	.max_vs_threads = 280,
	.max_gs_threads = 280,
	.max_wm_threads =
		(280 - 1) << HSW_PS_MAX_THREADS_SHIFT |
		1 << HSW_PS_SAMPLE_MASK_SHIFT,
	.urb = { 512, 3328, 1280, 16 },
	.gt = 3,
};

inline static bool is_ivb(struct sna *sna)
{
	return sna->kgem.gen == 070;
}

inline static bool is_byt(struct sna *sna)
{
	return sna->kgem.gen == 071;
}

inline static bool is_hsw(struct sna *sna)
{
	return sna->kgem.gen == 075;
}

static const uint32_t ps_kernel_packed[][4] = {
#include "exa_wm_src_affine.g7b"
#include "exa_wm_src_sample_argb.g7b"
#include "exa_wm_yuv_rgb.g7b"
#include "exa_wm_write.g7b"
};

static const uint32_t ps_kernel_planar[][4] = {
#include "exa_wm_src_affine.g7b"
#include "exa_wm_src_sample_planar.g7b"
#include "exa_wm_yuv_rgb.g7b"
#include "exa_wm_write.g7b"
};

#define KERNEL(kernel_enum, kernel, num_surfaces) \
    [GEN7_WM_KERNEL_##kernel_enum] = {#kernel_enum, kernel, sizeof(kernel), num_surfaces}
#define NOKERNEL(kernel_enum, func, num_surfaces) \
    [GEN7_WM_KERNEL_##kernel_enum] = {#kernel_enum, (void *)func, 0, num_surfaces}
static const struct wm_kernel_info {
	const char *name;
	const void *data;
	unsigned int size;
	int num_surfaces;
} wm_kernels[] = {
	NOKERNEL(NOMASK, brw_wm_kernel__affine, 2),
	NOKERNEL(NOMASK_P, brw_wm_kernel__projective, 2),

	NOKERNEL(MASK, brw_wm_kernel__affine_mask, 3),
	NOKERNEL(MASK_P, brw_wm_kernel__projective_mask, 3),

	NOKERNEL(MASKCA, brw_wm_kernel__affine_mask_ca, 3),
	NOKERNEL(MASKCA_P, brw_wm_kernel__projective_mask_ca, 3),

	NOKERNEL(MASKSA, brw_wm_kernel__affine_mask_sa, 3),
	NOKERNEL(MASKSA_P, brw_wm_kernel__projective_mask_sa, 3),

	NOKERNEL(OPACITY, brw_wm_kernel__affine_opacity, 2),
	NOKERNEL(OPACITY_P, brw_wm_kernel__projective_opacity, 2),

	KERNEL(VIDEO_PLANAR, ps_kernel_planar, 7),
	KERNEL(VIDEO_PACKED, ps_kernel_packed, 2),
};
#undef KERNEL

static const struct blendinfo {
	bool src_alpha;
	uint32_t src_blend;
	uint32_t dst_blend;
} gen7_blend_op[] = {
	/* Clear */	{0, GEN7_BLENDFACTOR_ZERO, GEN7_BLENDFACTOR_ZERO},
	/* Src */	{0, GEN7_BLENDFACTOR_ONE, GEN7_BLENDFACTOR_ZERO},
	/* Dst */	{0, GEN7_BLENDFACTOR_ZERO, GEN7_BLENDFACTOR_ONE},
	/* Over */	{1, GEN7_BLENDFACTOR_ONE, GEN7_BLENDFACTOR_INV_SRC_ALPHA},
	/* OverReverse */ {0, GEN7_BLENDFACTOR_INV_DST_ALPHA, GEN7_BLENDFACTOR_ONE},
	/* In */	{0, GEN7_BLENDFACTOR_DST_ALPHA, GEN7_BLENDFACTOR_ZERO},
	/* InReverse */	{1, GEN7_BLENDFACTOR_ZERO, GEN7_BLENDFACTOR_SRC_ALPHA},
	/* Out */	{0, GEN7_BLENDFACTOR_INV_DST_ALPHA, GEN7_BLENDFACTOR_ZERO},
	/* OutReverse */ {1, GEN7_BLENDFACTOR_ZERO, GEN7_BLENDFACTOR_INV_SRC_ALPHA},
	/* Atop */	{1, GEN7_BLENDFACTOR_DST_ALPHA, GEN7_BLENDFACTOR_INV_SRC_ALPHA},
	/* AtopReverse */ {1, GEN7_BLENDFACTOR_INV_DST_ALPHA, GEN7_BLENDFACTOR_SRC_ALPHA},
	/* Xor */	{1, GEN7_BLENDFACTOR_INV_DST_ALPHA, GEN7_BLENDFACTOR_INV_SRC_ALPHA},
	/* Add */	{0, GEN7_BLENDFACTOR_ONE, GEN7_BLENDFACTOR_ONE},
};

/**
 * Highest-valued BLENDFACTOR used in gen7_blend_op.
 *
 * This leaves out GEN7_BLENDFACTOR_INV_DST_COLOR,
 * GEN7_BLENDFACTOR_INV_CONST_{COLOR,ALPHA},
 * GEN7_BLENDFACTOR_INV_SRC1_{COLOR,ALPHA}
 */
#define GEN7_BLENDFACTOR_COUNT (GEN7_BLENDFACTOR_INV_DST_ALPHA + 1)

#define GEN7_BLEND_STATE_PADDED_SIZE	ALIGN(sizeof(struct gen7_blend_state), 64)

#define BLEND_OFFSET(s, d) \
	((d != GEN7_BLENDFACTOR_ZERO) << 15 | \
	 (((s) * GEN7_BLENDFACTOR_COUNT + (d)) * GEN7_BLEND_STATE_PADDED_SIZE))

#define NO_BLEND BLEND_OFFSET(GEN7_BLENDFACTOR_ONE, GEN7_BLENDFACTOR_ZERO)
#define CLEAR BLEND_OFFSET(GEN7_BLENDFACTOR_ZERO, GEN7_BLENDFACTOR_ZERO)

#define SAMPLER_OFFSET(sf, se, mf, me) \
	((((((sf) * EXTEND_COUNT + (se)) * FILTER_COUNT + (mf)) * EXTEND_COUNT + (me)) + 2) * 2 * sizeof(struct gen7_sampler_state))

#define VERTEX_2s2s 0

#define COPY_SAMPLER 0
#define COPY_VERTEX VERTEX_2s2s
#define COPY_FLAGS(a) GEN7_SET_FLAGS(COPY_SAMPLER, (a) == GXcopy ? NO_BLEND : CLEAR, GEN7_WM_KERNEL_NOMASK, COPY_VERTEX)

#define FILL_SAMPLER (2 * sizeof(struct gen7_sampler_state))
#define FILL_VERTEX VERTEX_2s2s
#define FILL_FLAGS(op, format) GEN7_SET_FLAGS(FILL_SAMPLER, gen7_get_blend((op), false, (format)), GEN7_WM_KERNEL_NOMASK, FILL_VERTEX)
#define FILL_FLAGS_NOBLEND GEN7_SET_FLAGS(FILL_SAMPLER, NO_BLEND, GEN7_WM_KERNEL_NOMASK, FILL_VERTEX)

#define GEN7_SAMPLER(f) (((f) >> 16) & 0xfff0)
#define GEN7_BLEND(f) (((f) >> 0) & 0x7ff0)
#define GEN7_READS_DST(f) (((f) >> 15) & 1)
#define GEN7_KERNEL(f) (((f) >> 16) & 0xf)
#define GEN7_VERTEX(f) (((f) >> 0) & 0xf)
#define GEN7_SET_FLAGS(S, B, K, V)  (((S) | (K)) << 16 | ((B) | (V)))

#define OUT_BATCH(v) batch_emit(sna, v)
#define OUT_VERTEX(x,y) vertex_emit_2s(sna, x,y)
#define OUT_VERTEX_F(v) vertex_emit(sna, v)

static inline bool too_large(int width, int height)
{
	return width > GEN7_MAX_SIZE || height > GEN7_MAX_SIZE;
}

static uint32_t gen7_get_blend(int op,
			       bool has_component_alpha,
			       uint32_t dst_format)
{
	uint32_t src, dst;


    src = GEN7_BLENDFACTOR_ONE; //gen6_blend_op[op].src_blend;
    dst = GEN7_BLENDFACTOR_INV_SRC_ALPHA; //gen6_blend_op[op].dst_blend;


#if 0
	/* If there's no dst alpha channel, adjust the blend op so that
	 * we'll treat it always as 1.
	 */
	if (PICT_FORMAT_A(dst_format) == 0) {
		if (src == GEN7_BLENDFACTOR_DST_ALPHA)
			src = GEN7_BLENDFACTOR_ONE;
		else if (src == GEN7_BLENDFACTOR_INV_DST_ALPHA)
			src = GEN7_BLENDFACTOR_ZERO;
	}

	/* If the source alpha is being used, then we should only be in a
	 * case where the source blend factor is 0, and the source blend
	 * value is the mask channels multiplied by the source picture's alpha.
	 */
	if (has_component_alpha && gen7_blend_op[op].src_alpha) {
		if (dst == GEN7_BLENDFACTOR_SRC_ALPHA)
			dst = GEN7_BLENDFACTOR_SRC_COLOR;
		else if (dst == GEN7_BLENDFACTOR_INV_SRC_ALPHA)
			dst = GEN7_BLENDFACTOR_INV_SRC_COLOR;
	}
#endif

	DBG(("blend op=%d, dst=%x [A=%d] => src=%d, dst=%d => offset=%x\n",
	     op, dst_format, PICT_FORMAT_A(dst_format),
	     src, dst, (int)BLEND_OFFSET(src, dst)));
	return BLEND_OFFSET(src, dst);
}

static uint32_t gen7_get_card_format(PictFormat format)
{
	switch (format) {
	default:
		return -1;
	case PICT_a8r8g8b8:
		return GEN7_SURFACEFORMAT_B8G8R8A8_UNORM;
	case PICT_x8r8g8b8:
		return GEN7_SURFACEFORMAT_B8G8R8X8_UNORM;
	case PICT_a8b8g8r8:
		return GEN7_SURFACEFORMAT_R8G8B8A8_UNORM;
	case PICT_x8b8g8r8:
		return GEN7_SURFACEFORMAT_R8G8B8X8_UNORM;
	case PICT_a2r10g10b10:
		return GEN7_SURFACEFORMAT_B10G10R10A2_UNORM;
	case PICT_x2r10g10b10:
		return GEN7_SURFACEFORMAT_B10G10R10X2_UNORM;
	case PICT_r8g8b8:
		return GEN7_SURFACEFORMAT_R8G8B8_UNORM;
	case PICT_r5g6b5:
		return GEN7_SURFACEFORMAT_B5G6R5_UNORM;
	case PICT_a1r5g5b5:
		return GEN7_SURFACEFORMAT_B5G5R5A1_UNORM;
	case PICT_a8:
		return GEN7_SURFACEFORMAT_A8_UNORM;
	case PICT_a4r4g4b4:
		return GEN7_SURFACEFORMAT_B4G4R4A4_UNORM;
	}
}

static uint32_t gen7_get_dest_format(PictFormat format)
{
	switch (format) {
	default:
		return -1;
	case PICT_a8r8g8b8:
	case PICT_x8r8g8b8:
		return GEN7_SURFACEFORMAT_B8G8R8A8_UNORM;
	case PICT_a8b8g8r8:
	case PICT_x8b8g8r8:
		return GEN7_SURFACEFORMAT_R8G8B8A8_UNORM;
	case PICT_a2r10g10b10:
	case PICT_x2r10g10b10:
		return GEN7_SURFACEFORMAT_B10G10R10A2_UNORM;
	case PICT_r5g6b5:
		return GEN7_SURFACEFORMAT_B5G6R5_UNORM;
	case PICT_x1r5g5b5:
	case PICT_a1r5g5b5:
		return GEN7_SURFACEFORMAT_B5G5R5A1_UNORM;
	case PICT_a8:
		return GEN7_SURFACEFORMAT_A8_UNORM;
	case PICT_a4r4g4b4:
	case PICT_x4r4g4b4:
		return GEN7_SURFACEFORMAT_B4G4R4A4_UNORM;
	}
}

static int
gen7_choose_composite_kernel(int op, bool has_mask, bool is_ca, bool is_affine)
{
	int base;

	if (has_mask) {
		if (is_ca) {
			if (gen7_blend_op[op].src_alpha)
				base = GEN7_WM_KERNEL_MASKSA;
			else
				base = GEN7_WM_KERNEL_MASKCA;
		} else
			base = GEN7_WM_KERNEL_MASK;
	} else
		base = GEN7_WM_KERNEL_NOMASK;

	return base + !is_affine;
}

static void
gen7_emit_urb(struct sna *sna)
{
	OUT_BATCH(GEN7_3DSTATE_PUSH_CONSTANT_ALLOC_PS | (2 - 2));
	OUT_BATCH(sna->render_state.gen7.info->urb.push_ps_size);

	/* num of VS entries must be divisible by 8 if size < 9 */
	OUT_BATCH(GEN7_3DSTATE_URB_VS | (2 - 2));
	OUT_BATCH((sna->render_state.gen7.info->urb.max_vs_entries << GEN7_URB_ENTRY_NUMBER_SHIFT) |
		  (2 - 1) << GEN7_URB_ENTRY_SIZE_SHIFT |
		  (1 << GEN7_URB_STARTING_ADDRESS_SHIFT));

	OUT_BATCH(GEN7_3DSTATE_URB_HS | (2 - 2));
	OUT_BATCH((0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
		  (2 << GEN7_URB_STARTING_ADDRESS_SHIFT));

	OUT_BATCH(GEN7_3DSTATE_URB_DS | (2 - 2));
	OUT_BATCH((0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
		  (2 << GEN7_URB_STARTING_ADDRESS_SHIFT));

	OUT_BATCH(GEN7_3DSTATE_URB_GS | (2 - 2));
	OUT_BATCH((0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
		  (1 << GEN7_URB_STARTING_ADDRESS_SHIFT));
}

static void
gen7_emit_state_base_address(struct sna *sna)
{
	uint32_t mocs;

	mocs = is_hsw(sna) ? 5 << 8 : 3 << 8;

	OUT_BATCH(GEN7_STATE_BASE_ADDRESS | (10 - 2));
	OUT_BATCH(0); /* general */
	OUT_BATCH(kgem_add_reloc(&sna->kgem, /* surface */
				 sna->kgem.nbatch,
				 NULL,
				 I915_GEM_DOMAIN_INSTRUCTION << 16,
				 BASE_ADDRESS_MODIFY));
	OUT_BATCH(kgem_add_reloc(&sna->kgem, /* dynamic */
				 sna->kgem.nbatch,
				 sna->render_state.gen7.general_bo,
				 I915_GEM_DOMAIN_INSTRUCTION << 16,
				 mocs | BASE_ADDRESS_MODIFY));
	OUT_BATCH(0); /* indirect */
	OUT_BATCH(kgem_add_reloc(&sna->kgem, /* instruction */
				 sna->kgem.nbatch,
				 sna->render_state.gen7.general_bo,
				 I915_GEM_DOMAIN_INSTRUCTION << 16,
				 mocs | BASE_ADDRESS_MODIFY));

	/* upper bounds, disable */
	OUT_BATCH(0);
	OUT_BATCH(BASE_ADDRESS_MODIFY);
	OUT_BATCH(0);
	OUT_BATCH(BASE_ADDRESS_MODIFY);
}

static void
gen7_disable_vs(struct sna *sna)
{
	/* For future reference:
	 * A PIPE_CONTROL with post-sync op set to 1 and a depth stall needs
	 * to be emitted just prior to change VS state, i.e. 3DSTATE_VS,
	 * 3DSTATE_URB_VS, 3DSTATE_CONSTANT_VS,
	 * 3DSTATE_BINDING_TABLE_POINTER_VS, 3DSTATE_SAMPLER_STATE_POINTER_VS.
	 *
	 * Here we saved by the full-flush incurred when emitting
	 * the batchbuffer.
	 */
	OUT_BATCH(GEN7_3DSTATE_VS | (6 - 2));
	OUT_BATCH(0); /* no VS kernel */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */

#if 0
	OUT_BATCH(GEN7_3DSTATE_CONSTANT_VS | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_BINDING_TABLE_POINTERS_VS | (2 - 2));
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_SAMPLER_STATE_POINTERS_VS | (2 - 2));
	OUT_BATCH(0);
#endif
}

static void
gen7_disable_hs(struct sna *sna)
{
	OUT_BATCH(GEN7_3DSTATE_HS | (7 - 2));
	OUT_BATCH(0); /* no HS kernel */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */

#if 0
	OUT_BATCH(GEN7_3DSTATE_CONSTANT_HS | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_BINDING_TABLE_POINTERS_HS | (2 - 2));
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_SAMPLER_STATE_POINTERS_HS | (2 - 2));
	OUT_BATCH(0);
#endif
}

static void
gen7_disable_te(struct sna *sna)
{
	OUT_BATCH(GEN7_3DSTATE_TE | (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

static void
gen7_disable_ds(struct sna *sna)
{
	OUT_BATCH(GEN7_3DSTATE_DS | (6 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

#if 0
	OUT_BATCH(GEN7_3DSTATE_CONSTANT_DS | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_BINDING_TABLE_POINTERS_DS | (2 - 2));
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_SAMPLER_STATE_POINTERS_DS | (2 - 2));
	OUT_BATCH(0);
#endif
}

static void
gen7_disable_gs(struct sna *sna)
{
	OUT_BATCH(GEN7_3DSTATE_GS | (7 - 2));
	OUT_BATCH(0); /* no GS kernel */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */

#if 0
	OUT_BATCH(GEN7_3DSTATE_CONSTANT_GS | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_BINDING_TABLE_POINTERS_GS | (2 - 2));
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_SAMPLER_STATE_POINTERS_GS | (2 - 2));
	OUT_BATCH(0);
#endif
}

static void
gen7_disable_streamout(struct sna *sna)
{
	OUT_BATCH(GEN7_3DSTATE_STREAMOUT | (3 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
}

static void
gen7_emit_sf_invariant(struct sna *sna)
{
	OUT_BATCH(GEN7_3DSTATE_SF | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(GEN7_3DSTATE_SF_CULL_NONE);
	OUT_BATCH(2 << GEN7_3DSTATE_SF_TRIFAN_PROVOKE_SHIFT);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

static void
gen7_emit_cc_invariant(struct sna *sna)
{
#if 0 /* unused, no change */
	OUT_BATCH(GEN7_3DSTATE_CC_STATE_POINTERS | (2 - 2));
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_DEPTH_STENCIL_STATE_POINTERS | (2 - 2));
	OUT_BATCH(0);
#endif

	/* XXX clear to be safe */
	OUT_BATCH(GEN7_3DSTATE_VIEWPORT_STATE_POINTERS_CC | (2 - 2));
	OUT_BATCH(0);
}

static void
gen7_disable_clip(struct sna *sna)
{
	OUT_BATCH(GEN7_3DSTATE_CLIP | (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CL | (2 - 2));
	OUT_BATCH(0);
}

static void
gen7_emit_wm_invariant(struct sna *sna)
{
	OUT_BATCH(GEN7_3DSTATE_WM | (3 - 2));
	OUT_BATCH(GEN7_WM_DISPATCH_ENABLE |
		  GEN7_WM_PERSPECTIVE_PIXEL_BARYCENTRIC);
	OUT_BATCH(0);

#if 0
	/* XXX length bias of 7 in old spec? */
	OUT_BATCH(GEN7_3DSTATE_CONSTANT_PS | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
#endif
}

static void
gen7_emit_null_depth_buffer(struct sna *sna)
{
	OUT_BATCH(GEN7_3DSTATE_DEPTH_BUFFER | (7 - 2));
	OUT_BATCH(GEN7_SURFACE_NULL << GEN7_3DSTATE_DEPTH_BUFFER_TYPE_SHIFT |
		  GEN7_DEPTHFORMAT_D32_FLOAT << GEN7_3DSTATE_DEPTH_BUFFER_FORMAT_SHIFT);
	OUT_BATCH(0); /* disable depth, stencil and hiz */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

#if 0
	OUT_BATCH(GEN7_3DSTATE_CLEAR_PARAMS | (3 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
#endif
}

static void
gen7_emit_invariant(struct sna *sna)
{
	OUT_BATCH(GEN7_PIPELINE_SELECT | PIPELINE_SELECT_3D);

	OUT_BATCH(GEN7_3DSTATE_MULTISAMPLE | (4 - 2));
	OUT_BATCH(GEN7_3DSTATE_MULTISAMPLE_PIXEL_LOCATION_CENTER |
		  GEN7_3DSTATE_MULTISAMPLE_NUMSAMPLES_1); /* 1 sample/pixel */
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_SAMPLE_MASK | (2 - 2));
	OUT_BATCH(1);

	gen7_emit_urb(sna);

	gen7_emit_state_base_address(sna);

	gen7_disable_vs(sna);
	gen7_disable_hs(sna);
	gen7_disable_te(sna);
	gen7_disable_ds(sna);
	gen7_disable_gs(sna);
	gen7_disable_clip(sna);
	gen7_emit_sf_invariant(sna);
	gen7_emit_wm_invariant(sna);
	gen7_emit_cc_invariant(sna);
	gen7_disable_streamout(sna);
	gen7_emit_null_depth_buffer(sna);

	sna->render_state.gen7.needs_invariant = false;
}

static void
gen7_emit_cc(struct sna *sna, uint32_t blend_offset)
{
	struct gen7_render_state *render = &sna->render_state.gen7;

	if (render->blend == blend_offset)
		return;

	DBG(("%s: blend = %x\n", __FUNCTION__, blend_offset));

	/* XXX can have upto 8 blend states preload, selectable via
	 * Render Target Index. What other side-effects of Render Target Index?
	 */

	assert (is_aligned(render->cc_blend + blend_offset, 64));
	OUT_BATCH(GEN7_3DSTATE_BLEND_STATE_POINTERS | (2 - 2));
	OUT_BATCH((render->cc_blend + blend_offset) | 1);

	render->blend = blend_offset;
}

static void
gen7_emit_sampler(struct sna *sna, uint32_t state)
{
	if (sna->render_state.gen7.samplers == state)
		return;

	sna->render_state.gen7.samplers = state;

	DBG(("%s: sampler = %x\n", __FUNCTION__, state));

	assert (is_aligned(sna->render_state.gen7.wm_state + state, 32));
	OUT_BATCH(GEN7_3DSTATE_SAMPLER_STATE_POINTERS_PS | (2 - 2));
	OUT_BATCH(sna->render_state.gen7.wm_state + state);
}

static void
gen7_emit_sf(struct sna *sna, bool has_mask)
{
	int num_sf_outputs = has_mask ? 2 : 1;

	if (sna->render_state.gen7.num_sf_outputs == num_sf_outputs)
		return;

	DBG(("%s: num_sf_outputs=%d, read_length=%d, read_offset=%d\n",
	     __FUNCTION__, num_sf_outputs, 1, 0));

	sna->render_state.gen7.num_sf_outputs = num_sf_outputs;

	OUT_BATCH(GEN7_3DSTATE_SBE | (14 - 2));
	OUT_BATCH(num_sf_outputs << GEN7_SBE_NUM_OUTPUTS_SHIFT |
		  1 << GEN7_SBE_URB_ENTRY_READ_LENGTH_SHIFT |
		  1 << GEN7_SBE_URB_ENTRY_READ_OFFSET_SHIFT);
	OUT_BATCH(0);
	OUT_BATCH(0); /* dw4 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* dw8 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* dw12 */
	OUT_BATCH(0);
	OUT_BATCH(0);
}

static void
gen7_emit_wm(struct sna *sna, int kernel)
{
	const uint32_t *kernels;

	if (sna->render_state.gen7.kernel == kernel)
		return;

	sna->render_state.gen7.kernel = kernel;
	kernels = sna->render_state.gen7.wm_kernel[kernel];

	DBG(("%s: switching to %s, num_surfaces=%d (8-wide? %d, 16-wide? %d, 32-wide? %d)\n",
	     __FUNCTION__,
	     wm_kernels[kernel].name,
	     wm_kernels[kernel].num_surfaces,
	     kernels[0], kernels[1], kernels[2]));

	OUT_BATCH(GEN7_3DSTATE_PS | (8 - 2));
	OUT_BATCH(kernels[0] ?: kernels[1] ?: kernels[2]);
	OUT_BATCH(1 << GEN7_PS_SAMPLER_COUNT_SHIFT |
		  wm_kernels[kernel].num_surfaces << GEN7_PS_BINDING_TABLE_ENTRY_COUNT_SHIFT);
	OUT_BATCH(0); /* scratch address */
	OUT_BATCH(sna->render_state.gen7.info->max_wm_threads |
		  (kernels[0] ? GEN7_PS_8_DISPATCH_ENABLE : 0) |
		  (kernels[1] ? GEN7_PS_16_DISPATCH_ENABLE : 0) |
		  (kernels[2] ? GEN7_PS_32_DISPATCH_ENABLE : 0) |
		  GEN7_PS_ATTRIBUTE_ENABLE);
	OUT_BATCH((kernels[0] ? 4 : kernels[1] ? 6 : 8) << GEN7_PS_DISPATCH_START_GRF_SHIFT_0 |
		  8 << GEN7_PS_DISPATCH_START_GRF_SHIFT_1 |
		  6 << GEN7_PS_DISPATCH_START_GRF_SHIFT_2);
	OUT_BATCH(kernels[2]);
	OUT_BATCH(kernels[1]);
}

static bool
gen7_emit_binding_table(struct sna *sna, uint16_t offset)
{
	if (sna->render_state.gen7.surface_table == offset)
		return false;

	/* Binding table pointers */
	assert(is_aligned(4*offset, 32));
	OUT_BATCH(GEN7_3DSTATE_BINDING_TABLE_POINTERS_PS | (2 - 2));
	OUT_BATCH(offset*4);

	sna->render_state.gen7.surface_table = offset;
	return true;
}

static bool
gen7_emit_drawing_rectangle(struct sna *sna,
			    const struct sna_composite_op *op)
{
	uint32_t limit = (op->dst.height - 1) << 16 | (op->dst.width - 1);
	uint32_t offset = (uint16_t)op->dst.y << 16 | (uint16_t)op->dst.x;

	assert(!too_large(op->dst.x, op->dst.y));
	assert(!too_large(op->dst.width, op->dst.height));

	if (sna->render_state.gen7.drawrect_limit == limit &&
	    sna->render_state.gen7.drawrect_offset == offset)
		return true;

	sna->render_state.gen7.drawrect_offset = offset;
	sna->render_state.gen7.drawrect_limit = limit;

	OUT_BATCH(GEN7_3DSTATE_DRAWING_RECTANGLE | (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(limit);
	OUT_BATCH(offset);
	return false;
}

static void
gen7_emit_vertex_elements(struct sna *sna,
			  const struct sna_composite_op *op)
{
	/*
	 * vertex data in vertex buffer
	 *    position: (x, y)
	 *    texture coordinate 0: (u0, v0) if (is_affine is true) else (u0, v0, w0)
	 *    texture coordinate 1 if (has_mask is true): same as above
	 */
	struct gen7_render_state *render = &sna->render_state.gen7;
	uint32_t src_format, dw;
	int id = GEN7_VERTEX(op->u.gen7.flags);
	bool has_mask;

	DBG(("%s: setup id=%d\n", __FUNCTION__, id));

	if (render->ve_id == id)
		return;
	render->ve_id = id;

	/* The VUE layout
	 *    dword 0-3: pad (0.0, 0.0, 0.0. 0.0)
	 *    dword 4-7: position (x, y, 1.0, 1.0),
	 *    dword 8-11: texture coordinate 0 (u0, v0, w0, 1.0)
	 *    dword 12-15: texture coordinate 1 (u1, v1, w1, 1.0)
	 *
	 * dword 4-15 are fetched from vertex buffer
	 */
	has_mask = (id >> 2) != 0;
	OUT_BATCH(GEN7_3DSTATE_VERTEX_ELEMENTS |
		((2 * (3 + has_mask)) + 1 - 2));

	OUT_BATCH(id << GEN7_VE0_VERTEX_BUFFER_INDEX_SHIFT | GEN7_VE0_VALID |
		  GEN7_SURFACEFORMAT_R32G32B32A32_FLOAT << GEN7_VE0_FORMAT_SHIFT |
		  0 << GEN7_VE0_OFFSET_SHIFT);
	OUT_BATCH(GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_0_SHIFT |
		  GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_1_SHIFT |
		  GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_2_SHIFT |
		  GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_3_SHIFT);

	/* x,y */
	OUT_BATCH(id << GEN7_VE0_VERTEX_BUFFER_INDEX_SHIFT | GEN7_VE0_VALID |
		  GEN7_SURFACEFORMAT_R16G16_SSCALED << GEN7_VE0_FORMAT_SHIFT |
		  0 << GEN7_VE0_OFFSET_SHIFT);
	OUT_BATCH(GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_0_SHIFT |
		  GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_1_SHIFT |
		  GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_2_SHIFT |
		  GEN7_VFCOMPONENT_STORE_1_FLT << GEN7_VE1_VFCOMPONENT_3_SHIFT);

	/* u0, v0, w0 */
	DBG(("%s: first channel %d floats, offset=4b\n", __FUNCTION__, id & 3));
	dw = GEN7_VFCOMPONENT_STORE_1_FLT << GEN7_VE1_VFCOMPONENT_3_SHIFT;
	switch (id & 3) {
	default:
		assert(0);
	case 0:
		src_format = GEN7_SURFACEFORMAT_R16G16_SSCALED;
		dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_0_SHIFT;
		dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_1_SHIFT;
		dw |= GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_2_SHIFT;
		break;
	case 1:
		src_format = GEN7_SURFACEFORMAT_R32_FLOAT;
		dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_0_SHIFT;
		dw |= GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_1_SHIFT;
		dw |= GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_2_SHIFT;
		break;
	case 2:
		src_format = GEN7_SURFACEFORMAT_R32G32_FLOAT;
		dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_0_SHIFT;
		dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_1_SHIFT;
		dw |= GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_2_SHIFT;
		break;
	case 3:
		src_format = GEN7_SURFACEFORMAT_R32G32B32_FLOAT;
		dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_0_SHIFT;
		dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_1_SHIFT;
		dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_2_SHIFT;
		break;
	}
	OUT_BATCH(id << GEN7_VE0_VERTEX_BUFFER_INDEX_SHIFT | GEN7_VE0_VALID |
		  src_format << GEN7_VE0_FORMAT_SHIFT |
		  4 << GEN7_VE0_OFFSET_SHIFT);
	OUT_BATCH(dw);

	/* u1, v1, w1 */
	if (has_mask) {
		unsigned offset = 4 + ((id & 3) ?: 1) * sizeof(float);
		DBG(("%s: second channel %d floats, offset=%db\n", __FUNCTION__, id >> 2, offset));
		dw = GEN7_VFCOMPONENT_STORE_1_FLT << GEN7_VE1_VFCOMPONENT_3_SHIFT;
		switch (id >> 2) {
		case 1:
			src_format = GEN7_SURFACEFORMAT_R32_FLOAT;
			dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_0_SHIFT;
			dw |= GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_1_SHIFT;
			dw |= GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_2_SHIFT;
			break;
		default:
			assert(0);
		case 2:
			src_format = GEN7_SURFACEFORMAT_R32G32_FLOAT;
			dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_0_SHIFT;
			dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_1_SHIFT;
			dw |= GEN7_VFCOMPONENT_STORE_0 << GEN7_VE1_VFCOMPONENT_2_SHIFT;
			break;
		case 3:
			src_format = GEN7_SURFACEFORMAT_R32G32B32_FLOAT;
			dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_0_SHIFT;
			dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_1_SHIFT;
			dw |= GEN7_VFCOMPONENT_STORE_SRC << GEN7_VE1_VFCOMPONENT_2_SHIFT;
			break;
		}
		OUT_BATCH(id << GEN7_VE0_VERTEX_BUFFER_INDEX_SHIFT | GEN7_VE0_VALID |
			  src_format << GEN7_VE0_FORMAT_SHIFT |
			  offset << GEN7_VE0_OFFSET_SHIFT);
		OUT_BATCH(dw);
	}
}

inline static void
gen7_emit_pipe_invalidate(struct sna *sna)
{
	OUT_BATCH(GEN7_PIPE_CONTROL | (4 - 2));
	OUT_BATCH(GEN7_PIPE_CONTROL_WC_FLUSH |
		  GEN7_PIPE_CONTROL_TC_FLUSH |
		  GEN7_PIPE_CONTROL_CS_STALL);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

inline static void
gen7_emit_pipe_flush(struct sna *sna, bool need_stall)
{
	unsigned stall;

	stall = 0;
	if (need_stall)
		stall = (GEN7_PIPE_CONTROL_CS_STALL |
			 GEN7_PIPE_CONTROL_STALL_AT_SCOREBOARD);

	OUT_BATCH(GEN7_PIPE_CONTROL | (4 - 2));
	OUT_BATCH(GEN7_PIPE_CONTROL_WC_FLUSH | stall);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

inline static void
gen7_emit_pipe_stall(struct sna *sna)
{
	OUT_BATCH(GEN7_PIPE_CONTROL | (4 - 2));
	OUT_BATCH(GEN7_PIPE_CONTROL_CS_STALL |
		  GEN7_PIPE_CONTROL_STALL_AT_SCOREBOARD);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

static void
gen7_emit_state(struct sna *sna,
		const struct sna_composite_op *op,
		uint16_t wm_binding_table)
{
	bool need_invalidate;
	bool need_flush;
	bool need_stall;

	assert(op->dst.bo->exec);

	need_invalidate = kgem_bo_is_dirty(op->src.bo) || kgem_bo_is_dirty(op->mask.bo);
	if (ALWAYS_INVALIDATE)
		need_invalidate = true;

	need_flush =
		sna->render_state.gen7.emit_flush &&
		wm_binding_table & GEN7_READS_DST(op->u.gen7.flags);
	if (ALWAYS_FLUSH)
		need_flush = true;

	wm_binding_table &= ~1;

	need_stall = sna->render_state.gen7.surface_table != wm_binding_table;
	need_stall &= gen7_emit_drawing_rectangle(sna, op);
	if (ALWAYS_STALL)
		need_stall = true;

	if (need_invalidate) {
		gen7_emit_pipe_invalidate(sna);
		kgem_clear_dirty(&sna->kgem);
		assert(op->dst.bo->exec);
			kgem_bo_mark_dirty(op->dst.bo);

		need_flush = false;
		need_stall = false;
	}
	if (need_flush) {
		gen7_emit_pipe_flush(sna, need_stall);
		need_stall = false;
	}
	if (need_stall)
		gen7_emit_pipe_stall(sna);

	gen7_emit_cc(sna, GEN7_BLEND(op->u.gen7.flags));
	gen7_emit_sampler(sna, GEN7_SAMPLER(op->u.gen7.flags));
	gen7_emit_sf(sna, GEN7_VERTEX(op->u.gen7.flags) >> 2);
	gen7_emit_wm(sna, GEN7_KERNEL(op->u.gen7.flags));
	gen7_emit_vertex_elements(sna, op);
	gen7_emit_binding_table(sna, wm_binding_table);

	sna->render_state.gen7.emit_flush = GEN7_READS_DST(op->u.gen7.flags);
}

static bool gen7_magic_ca_pass(struct sna *sna,
			       const struct sna_composite_op *op)
{
	struct gen7_render_state *state = &sna->render_state.gen7;

	if (!op->need_magic_ca_pass)
		return false;

	DBG(("%s: CA fixup (%d -> %d)\n", __FUNCTION__,
	     sna->render.vertex_start, sna->render.vertex_index));

	gen7_emit_pipe_stall(sna);

	gen7_emit_cc(sna,
		     GEN7_BLEND(gen7_get_blend(PictOpAdd, true,
					       op->dst.format)));
	gen7_emit_wm(sna,
		     gen7_choose_composite_kernel(PictOpAdd,
						  true, true,
						  op->is_affine));

	OUT_BATCH(GEN7_3DPRIMITIVE | (7- 2));
	OUT_BATCH(GEN7_3DPRIMITIVE_VERTEX_SEQUENTIAL | _3DPRIM_RECTLIST);
	OUT_BATCH(sna->render.vertex_index - sna->render.vertex_start);
	OUT_BATCH(sna->render.vertex_start);
	OUT_BATCH(1);	/* single instance */
	OUT_BATCH(0);	/* start instance location */
	OUT_BATCH(0);	/* index buffer offset, ignored */

	state->last_primitive = sna->kgem.nbatch;
	return true;
}

static void null_create(struct sna_static_stream *stream)
{
	/* A bunch of zeros useful for legacy border color and depth-stencil */
	sna_static_stream_map(stream, 64, 64);
}

static void
sampler_state_init(struct gen7_sampler_state *sampler_state,
		   sampler_filter_t filter,
		   sampler_extend_t extend)
{
	sampler_state->ss0.lod_preclamp = 1;	/* GL mode */

	/* We use the legacy mode to get the semantics specified by
	 * the Render extension. */
	sampler_state->ss0.default_color_mode = GEN7_BORDER_COLOR_MODE_LEGACY;

	switch (filter) {
	default:
	case SAMPLER_FILTER_NEAREST:
		sampler_state->ss0.min_filter = GEN7_MAPFILTER_NEAREST;
		sampler_state->ss0.mag_filter = GEN7_MAPFILTER_NEAREST;
		break;
	case SAMPLER_FILTER_BILINEAR:
		sampler_state->ss0.min_filter = GEN7_MAPFILTER_LINEAR;
		sampler_state->ss0.mag_filter = GEN7_MAPFILTER_LINEAR;
		break;
	}

	switch (extend) {
	default:
	case SAMPLER_EXTEND_NONE:
		sampler_state->ss3.r_wrap_mode = GEN7_TEXCOORDMODE_CLAMP_BORDER;
		sampler_state->ss3.s_wrap_mode = GEN7_TEXCOORDMODE_CLAMP_BORDER;
		sampler_state->ss3.t_wrap_mode = GEN7_TEXCOORDMODE_CLAMP_BORDER;
		break;
	case SAMPLER_EXTEND_REPEAT:
		sampler_state->ss3.r_wrap_mode = GEN7_TEXCOORDMODE_WRAP;
		sampler_state->ss3.s_wrap_mode = GEN7_TEXCOORDMODE_WRAP;
		sampler_state->ss3.t_wrap_mode = GEN7_TEXCOORDMODE_WRAP;
		break;
	case SAMPLER_EXTEND_PAD:
		sampler_state->ss3.r_wrap_mode = GEN7_TEXCOORDMODE_CLAMP;
		sampler_state->ss3.s_wrap_mode = GEN7_TEXCOORDMODE_CLAMP;
		sampler_state->ss3.t_wrap_mode = GEN7_TEXCOORDMODE_CLAMP;
		break;
	case SAMPLER_EXTEND_REFLECT:
		sampler_state->ss3.r_wrap_mode = GEN7_TEXCOORDMODE_MIRROR;
		sampler_state->ss3.s_wrap_mode = GEN7_TEXCOORDMODE_MIRROR;
		sampler_state->ss3.t_wrap_mode = GEN7_TEXCOORDMODE_MIRROR;
		break;
	}
}

static void
sampler_copy_init(struct gen7_sampler_state *ss)
{
	sampler_state_init(ss, SAMPLER_FILTER_NEAREST, SAMPLER_EXTEND_NONE);
	ss->ss3.non_normalized_coord = 1;

	sampler_state_init(ss+1, SAMPLER_FILTER_NEAREST, SAMPLER_EXTEND_NONE);
}

static void
sampler_fill_init(struct gen7_sampler_state *ss)
{
	sampler_state_init(ss, SAMPLER_FILTER_NEAREST, SAMPLER_EXTEND_REPEAT);
	ss->ss3.non_normalized_coord = 1;

	sampler_state_init(ss+1, SAMPLER_FILTER_NEAREST, SAMPLER_EXTEND_NONE);
}

static uint32_t
gen7_tiling_bits(uint32_t tiling)
{
	switch (tiling) {
	default: assert(0);
	case I915_TILING_NONE: return 0;
	case I915_TILING_X: return GEN7_SURFACE_TILED;
	case I915_TILING_Y: return GEN7_SURFACE_TILED | GEN7_SURFACE_TILED_Y;
	}
}

/**
 * Sets up the common fields for a surface state buffer for the given
 * picture in the given surface state buffer.
 */
static uint32_t
gen7_bind_bo(struct sna *sna,
	     struct kgem_bo *bo,
	     uint32_t width,
	     uint32_t height,
	     uint32_t format,
	     bool is_dst)
{
	uint32_t *ss;
	uint32_t domains;
	int offset;
	uint32_t is_scanout = is_dst && bo->scanout;

	COMPILE_TIME_ASSERT(sizeof(struct gen7_surface_state) == 32);

	/* After the first bind, we manage the cache domains within the batch */
	offset = kgem_bo_get_binding(bo, format | is_dst << 30 | is_scanout << 31);
	if (offset) {
		if (is_dst)
			kgem_bo_mark_dirty(bo);
		return offset * sizeof(uint32_t);
	}

	offset = sna->kgem.surface -=
		sizeof(struct gen7_surface_state) / sizeof(uint32_t);
	ss = sna->kgem.batch + offset;
	ss[0] = (GEN7_SURFACE_2D << GEN7_SURFACE_TYPE_SHIFT |
		 gen7_tiling_bits(bo->tiling) |
		 format << GEN7_SURFACE_FORMAT_SHIFT);
	if (bo->tiling == I915_TILING_Y)
		ss[0] |= GEN7_SURFACE_VALIGN_4;
	if (is_dst) {
		ss[0] |= GEN7_SURFACE_RC_READ_WRITE;
		domains = I915_GEM_DOMAIN_RENDER << 16 |I915_GEM_DOMAIN_RENDER;
	} else
		domains = I915_GEM_DOMAIN_SAMPLER << 16;
	ss[1] = kgem_add_reloc(&sna->kgem, offset + 1, bo, domains, 0);
	ss[2] = ((width - 1)  << GEN7_SURFACE_WIDTH_SHIFT |
		 (height - 1) << GEN7_SURFACE_HEIGHT_SHIFT);
	ss[3] = (bo->pitch - 1) << GEN7_SURFACE_PITCH_SHIFT;
	ss[4] = 0;
	ss[5] = (is_scanout || bo->io) ? 0 : is_hsw(sna) ? 5 << 16 : 3 << 16;
	ss[6] = 0;
	ss[7] = 0;
	if (is_hsw(sna))
		ss[7] |= HSW_SURFACE_SWIZZLE(RED, GREEN, BLUE, ALPHA);

	kgem_bo_set_binding(bo, format | is_dst << 30 | is_scanout << 31, offset);

	DBG(("[%x] bind bo(handle=%d, addr=%d), format=%d, width=%d, height=%d, pitch=%d, tiling=%d -> %s\n",
	     offset, bo->handle, ss[1],
	     format, width, height, bo->pitch, bo->tiling,
	     domains & 0xffff ? "render" : "sampler"));

	return offset * sizeof(uint32_t);
}

static void gen7_emit_vertex_buffer(struct sna *sna,
				    const struct sna_composite_op *op)
{
	int id = GEN7_VERTEX(op->u.gen7.flags);

	OUT_BATCH(GEN7_3DSTATE_VERTEX_BUFFERS | (5 - 2));
	OUT_BATCH(id << GEN7_VB0_BUFFER_INDEX_SHIFT |
		  GEN7_VB0_VERTEXDATA |
		  GEN7_VB0_ADDRESS_MODIFY_ENABLE |
		  4*op->floats_per_vertex << GEN7_VB0_BUFFER_PITCH_SHIFT);
	sna->render.vertex_reloc[sna->render.nvertex_reloc++] = sna->kgem.nbatch;
	OUT_BATCH(0);
	OUT_BATCH(~0); /* max address: disabled */
	OUT_BATCH(0);

	sna->render.vb_id |= 1 << id;
}

static void gen7_emit_primitive(struct sna *sna)
{
	if (sna->kgem.nbatch == sna->render_state.gen7.last_primitive) {
		sna->render.vertex_offset = sna->kgem.nbatch - 5;
		return;
	}

	OUT_BATCH(GEN7_3DPRIMITIVE | (7- 2));
	OUT_BATCH(GEN7_3DPRIMITIVE_VERTEX_SEQUENTIAL | _3DPRIM_RECTLIST);
	sna->render.vertex_offset = sna->kgem.nbatch;
	OUT_BATCH(0);	/* vertex count, to be filled in later */
	OUT_BATCH(sna->render.vertex_index);
	OUT_BATCH(1);	/* single instance */
	OUT_BATCH(0);	/* start instance location */
	OUT_BATCH(0);	/* index buffer offset, ignored */
	sna->render.vertex_start = sna->render.vertex_index;

	sna->render_state.gen7.last_primitive = sna->kgem.nbatch;
}

static bool gen7_rectangle_begin(struct sna *sna,
				 const struct sna_composite_op *op)
{
	int id = 1 << GEN7_VERTEX(op->u.gen7.flags);
	int ndwords;

	if (sna_vertex_wait__locked(&sna->render) && sna->render.vertex_offset)
		return true;

	ndwords = op->need_magic_ca_pass ? 60 : 6;
	if ((sna->render.vb_id & id) == 0)
		ndwords += 5;
	if (!kgem_check_batch(&sna->kgem, ndwords))
		return false;

	if ((sna->render.vb_id & id) == 0)
		gen7_emit_vertex_buffer(sna, op);

	gen7_emit_primitive(sna);
	return true;
}

static int gen7_get_rectangles__flush(struct sna *sna,
				      const struct sna_composite_op *op)
{
	/* Preventing discarding new vbo after lock contention */
	if (sna_vertex_wait__locked(&sna->render)) {
		int rem = vertex_space(sna);
		if (rem > op->floats_per_rect)
			return rem;
	}

	if (!kgem_check_batch(&sna->kgem, op->need_magic_ca_pass ? 65 : 6))
		return 0;
	if (!kgem_check_reloc_and_exec(&sna->kgem, 2))
		return 0;

	if (sna->render.vertex_offset) {
		gen4_vertex_flush(sna);
		if (gen7_magic_ca_pass(sna, op)) {
			gen7_emit_pipe_stall(sna);
			gen7_emit_cc(sna, GEN7_BLEND(op->u.gen7.flags));
			gen7_emit_wm(sna, GEN7_KERNEL(op->u.gen7.flags));
		}
	}

	return gen4_vertex_finish(sna);
}

inline static int gen7_get_rectangles(struct sna *sna,
				      const struct sna_composite_op *op,
				      int want,
				      void (*emit_state)(struct sna *sna, const struct sna_composite_op *op))
{
	int rem;

	assert(want);

start:
	rem = vertex_space(sna);
	if (unlikely(rem < op->floats_per_rect)) {
		DBG(("flushing vbo for %s: %d < %d\n",
		     __FUNCTION__, rem, op->floats_per_rect));
		rem = gen7_get_rectangles__flush(sna, op);
		if (unlikely(rem == 0))
			goto flush;
	}

	if (unlikely(sna->render.vertex_offset == 0)) {
		if (!gen7_rectangle_begin(sna, op))
			goto flush;
		else
			goto start;
	}

	assert(rem <= vertex_space(sna));
	assert(op->floats_per_rect <= rem);
	if (want > 1 && want * op->floats_per_rect > rem)
		want = rem / op->floats_per_rect;

	assert(want > 0);
	sna->render.vertex_index += 3*want;
	return want;

flush:
	if (sna->render.vertex_offset) {
		gen4_vertex_flush(sna);
		gen7_magic_ca_pass(sna, op);
	}
	sna_vertex_wait__locked(&sna->render);
	_kgem_submit(&sna->kgem);
	emit_state(sna, op);
	goto start;
}

inline static uint32_t *gen7_composite_get_binding_table(struct sna *sna,
							 uint16_t *offset)
{
	uint32_t *table;

	sna->kgem.surface -=
		sizeof(struct gen7_surface_state) / sizeof(uint32_t);
	/* Clear all surplus entries to zero in case of prefetch */
	table = memset(sna->kgem.batch + sna->kgem.surface,
		       0, sizeof(struct gen7_surface_state));

	DBG(("%s(%x)\n", __FUNCTION__, 4*sna->kgem.surface));

	*offset = sna->kgem.surface;
	return table;
}

static void
gen7_get_batch(struct sna *sna, const struct sna_composite_op *op)
{
	kgem_set_mode(&sna->kgem, KGEM_RENDER, op->dst.bo);

	if (!kgem_check_batch_with_surfaces(&sna->kgem, 150, 4)) {
		DBG(("%s: flushing batch: %d < %d+%d\n",
		     __FUNCTION__, sna->kgem.surface - sna->kgem.nbatch,
		     150, 4*8));
		_kgem_submit(&sna->kgem);
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	assert(sna->kgem.mode == KGEM_RENDER);
	assert(sna->kgem.ring == KGEM_RENDER);

	if (sna->render_state.gen7.needs_invariant)
		gen7_emit_invariant(sna);
}

static void gen7_emit_composite_state(struct sna *sna,
				      const struct sna_composite_op *op)
{
	uint32_t *binding_table;
	uint16_t offset, dirty;

	gen7_get_batch(sna, op);

	binding_table = gen7_composite_get_binding_table(sna, &offset);

	dirty = kgem_bo_is_dirty(op->dst.bo);

	binding_table[0] =
		gen7_bind_bo(sna,
			    op->dst.bo, op->dst.width, op->dst.height,
			    gen7_get_dest_format(op->dst.format),
			    true);
	binding_table[1] =
		gen7_bind_bo(sna,
			     op->src.bo, op->src.width, op->src.height,
			     op->src.card_format,
			     false);
	if (op->mask.bo) {
		binding_table[2] =
			gen7_bind_bo(sna,
				     op->mask.bo,
				     op->mask.width,
				     op->mask.height,
				     op->mask.card_format,
				     false);
	}

	if (sna->kgem.surface == offset &&
	    *(uint64_t *)(sna->kgem.batch + sna->render_state.gen7.surface_table) == *(uint64_t*)binding_table &&
	    (op->mask.bo == NULL ||
	     sna->kgem.batch[sna->render_state.gen7.surface_table+2] == binding_table[2])) {
		sna->kgem.surface += sizeof(struct gen7_surface_state) / sizeof(uint32_t);
		offset = sna->render_state.gen7.surface_table;
	}

	gen7_emit_state(sna, op, offset | dirty);
}

static void
gen7_align_vertex(struct sna *sna, const struct sna_composite_op *op)
{
	if (op->floats_per_vertex != sna->render_state.gen7.floats_per_vertex) {
		DBG(("aligning vertex: was %d, now %d floats per vertex\n",
		     sna->render_state.gen7.floats_per_vertex, op->floats_per_vertex));
		gen4_vertex_align(sna, op);
		sna->render_state.gen7.floats_per_vertex = op->floats_per_vertex;
	}
}

fastcall static void
gen7_render_composite_blt(struct sna *sna,
			  const struct sna_composite_op *op,
			  const struct sna_composite_rectangles *r)
{
	gen7_get_rectangles(sna, op, 1, gen7_emit_composite_state);
	op->prim_emit(sna, op, r);
}
static uint32_t
gen7_composite_create_blend_state(struct sna_static_stream *stream)
{
	char *base, *ptr;
	int src, dst;

	base = sna_static_stream_map(stream,
				     GEN7_BLENDFACTOR_COUNT * GEN7_BLENDFACTOR_COUNT * GEN7_BLEND_STATE_PADDED_SIZE,
				     64);

	ptr = base;
	for (src = 0; src < GEN7_BLENDFACTOR_COUNT; src++) {
		for (dst= 0; dst < GEN7_BLENDFACTOR_COUNT; dst++) {
			struct gen7_blend_state *blend =
				(struct gen7_blend_state *)ptr;

			blend->blend0.dest_blend_factor = dst;
			blend->blend0.source_blend_factor = src;
			blend->blend0.blend_func = GEN7_BLENDFUNCTION_ADD;
			blend->blend0.blend_enable =
				!(dst == GEN7_BLENDFACTOR_ZERO && src == GEN7_BLENDFACTOR_ONE);

			blend->blend1.post_blend_clamp_enable = 1;
			blend->blend1.pre_blend_clamp_enable = 1;

			ptr += GEN7_BLEND_STATE_PADDED_SIZE;
		}
	}

	return sna_static_stream_offsetof(stream, base);
}

#if 0
static uint32_t gen7_bind_video_source(struct sna *sna,
				       struct kgem_bo *bo,
				       uint32_t offset,
				       int width,
				       int height,
				       int pitch,
				       uint32_t format)
{
	uint32_t *ss, bind;

	bind = sna->kgem.surface -=
		sizeof(struct gen7_surface_state) / sizeof(uint32_t);

	assert(bo->tiling == I915_TILING_NONE);

	ss = sna->kgem.batch + bind;
	ss[0] = (GEN7_SURFACE_2D << GEN7_SURFACE_TYPE_SHIFT |
		 format << GEN7_SURFACE_FORMAT_SHIFT);
	ss[1] = kgem_add_reloc(&sna->kgem, bind + 1, bo,
			       I915_GEM_DOMAIN_SAMPLER << 16,
			       offset);
	ss[2] = ((width - 1)  << GEN7_SURFACE_WIDTH_SHIFT |
		 (height - 1) << GEN7_SURFACE_HEIGHT_SHIFT);
	ss[3] = (pitch - 1) << GEN7_SURFACE_PITCH_SHIFT;
	ss[4] = 0;
	ss[5] = 0;
	ss[6] = 0;
	ss[7] = 0;
	if (is_hsw(sna))
		ss[7] |= HSW_SURFACE_SWIZZLE(RED, GREEN, BLUE, ALPHA);

	DBG(("[%x] bind bo(handle=%d, addr=%d), format=%d, width=%d, height=%d, pitch=%d, offset=%d\n",
	     bind, bo->handle, ss[1],
	     format, width, height, pitch, offset));

	return bind * sizeof(uint32_t);
}

static void gen7_emit_video_state(struct sna *sna,
				  const struct sna_composite_op *op)
{
	struct sna_video_frame *frame = op->priv;
	uint32_t src_surf_format;
	uint32_t src_surf_base[6];
	int src_width[6];
	int src_height[6];
	int src_pitch[6];
	uint32_t *binding_table;
	uint16_t offset, dirty;
	int n_src, n;

	gen7_get_batch(sna, op);

	src_surf_base[0] = 0;
	src_surf_base[1] = 0;
	src_surf_base[2] = frame->VBufOffset;
	src_surf_base[3] = frame->VBufOffset;
	src_surf_base[4] = frame->UBufOffset;
	src_surf_base[5] = frame->UBufOffset;

	if (is_planar_fourcc(frame->id)) {
		src_surf_format = GEN7_SURFACEFORMAT_R8_UNORM;
		src_width[1]  = src_width[0]  = frame->width;
		src_height[1] = src_height[0] = frame->height;
		src_pitch[1]  = src_pitch[0]  = frame->pitch[1];
		src_width[4]  = src_width[5]  = src_width[2]  = src_width[3] =
			frame->width / 2;
		src_height[4] = src_height[5] = src_height[2] = src_height[3] =
			frame->height / 2;
		src_pitch[4]  = src_pitch[5]  = src_pitch[2]  = src_pitch[3] =
			frame->pitch[0];
		n_src = 6;
	} else {
		if (frame->id == FOURCC_UYVY)
			src_surf_format = GEN7_SURFACEFORMAT_YCRCB_SWAPY;
		else
			src_surf_format = GEN7_SURFACEFORMAT_YCRCB_NORMAL;

		src_width[0]  = frame->width;
		src_height[0] = frame->height;
		src_pitch[0]  = frame->pitch[0];
		n_src = 1;
	}

	binding_table = gen7_composite_get_binding_table(sna, &offset);

	dirty = kgem_bo_is_dirty(op->dst.bo);

	binding_table[0] =
		gen7_bind_bo(sna,
			     op->dst.bo, op->dst.width, op->dst.height,
			     gen7_get_dest_format(op->dst.format),
			     true);
	for (n = 0; n < n_src; n++) {
		binding_table[1+n] =
			gen7_bind_video_source(sna,
					       frame->bo,
					       src_surf_base[n],
					       src_width[n],
					       src_height[n],
					       src_pitch[n],
					       src_surf_format);
	}

	gen7_emit_state(sna, op, offset | dirty);
}

static bool
gen7_render_video(struct sna *sna,
		  struct sna_video *video,
		  struct sna_video_frame *frame,
		  RegionPtr dstRegion,
		  PixmapPtr pixmap)
{
	struct sna_composite_op tmp;
	int dst_width = dstRegion->extents.x2 - dstRegion->extents.x1;
	int dst_height = dstRegion->extents.y2 - dstRegion->extents.y1;
	int src_width = frame->src.x2 - frame->src.x1;
	int src_height = frame->src.y2 - frame->src.y1;
	float src_offset_x, src_offset_y;
	float src_scale_x, src_scale_y;
	int nbox, pix_xoff, pix_yoff;
	struct sna_pixmap *priv;
	unsigned filter;
	BoxPtr box;

	DBG(("%s: src=(%d, %d), dst=(%d, %d), %ldx[(%d, %d), (%d, %d)...]\n",
	     __FUNCTION__,
	     src_width, src_height, dst_width, dst_height,
	     (long)REGION_NUM_RECTS(dstRegion),
	     REGION_EXTENTS(NULL, dstRegion)->x1,
	     REGION_EXTENTS(NULL, dstRegion)->y1,
	     REGION_EXTENTS(NULL, dstRegion)->x2,
	     REGION_EXTENTS(NULL, dstRegion)->y2));

	priv = sna_pixmap_force_to_gpu(pixmap, MOVE_READ | MOVE_WRITE);
	if (priv == NULL)
		return false;

	memset(&tmp, 0, sizeof(tmp));

	tmp.dst.pixmap = pixmap;
	tmp.dst.width  = pixmap->drawable.width;
	tmp.dst.height = pixmap->drawable.height;
	tmp.dst.format = sna_render_format_for_depth(pixmap->drawable.depth);
	tmp.dst.bo = priv->gpu_bo;

	tmp.src.bo = frame->bo;
	tmp.mask.bo = NULL;

	tmp.floats_per_vertex = 3;
	tmp.floats_per_rect = 9;

	if (src_width == dst_width && src_height == dst_height)
		filter = SAMPLER_FILTER_NEAREST;
	else
		filter = SAMPLER_FILTER_BILINEAR;

	tmp.u.gen7.flags =
		GEN7_SET_FLAGS(SAMPLER_OFFSET(filter, SAMPLER_EXTEND_PAD,
					      SAMPLER_FILTER_NEAREST, SAMPLER_EXTEND_NONE),
			       NO_BLEND,
			       is_planar_fourcc(frame->id) ?
			       GEN7_WM_KERNEL_VIDEO_PLANAR :
			       GEN7_WM_KERNEL_VIDEO_PACKED,
			       2);
	tmp.priv = frame;

	kgem_set_mode(&sna->kgem, KGEM_RENDER, tmp.dst.bo);
	if (!kgem_check_bo(&sna->kgem, tmp.dst.bo, frame->bo, NULL)) {
		kgem_submit(&sna->kgem);
		if (!kgem_check_bo(&sna->kgem, tmp.dst.bo, frame->bo, NULL))
			return false;

		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	gen7_align_vertex(sna, &tmp);
	gen7_emit_video_state(sna, &tmp);

	/* Set up the offset for translating from the given region (in screen
	 * coordinates) to the backing pixmap.
	 */
#ifdef COMPOSITE
	pix_xoff = -pixmap->screen_x + pixmap->drawable.x;
	pix_yoff = -pixmap->screen_y + pixmap->drawable.y;
#else
	pix_xoff = 0;
	pix_yoff = 0;
#endif

	DBG(("%s: src=(%d, %d)x(%d, %d); frame=(%dx%d), dst=(%dx%d)\n",
	     __FUNCTION__,
	     frame->src.x1, frame->src.y1,
	     src_width, src_height,
	     dst_width, dst_height,
	     frame->width, frame->height));

	src_scale_x = (float)src_width / dst_width / frame->width;
	src_offset_x = (float)frame->src.x1 / frame->width - dstRegion->extents.x1 * src_scale_x;

	src_scale_y = (float)src_height / dst_height / frame->height;
	src_offset_y = (float)frame->src.y1 / frame->height - dstRegion->extents.y1 * src_scale_y;

	DBG(("%s: scale=(%f, %f), offset=(%f, %f)\n",
	     __FUNCTION__,
	     src_scale_x, src_scale_y,
	     src_offset_x, src_offset_y));

	box = REGION_RECTS(dstRegion);
	nbox = REGION_NUM_RECTS(dstRegion);
	while (nbox--) {
		BoxRec r;

		DBG(("%s: dst=(%d, %d), (%d, %d) + (%d, %d); src=(%f, %f), (%f, %f)\n",
		     __FUNCTION__,
		     box->x1, box->y1,
		     box->x2, box->y2,
		     pix_xoff, pix_yoff,
		     box->x1 * src_scale_x + src_offset_x,
		     box->y1 * src_scale_y + src_offset_y,
		     box->x2 * src_scale_x + src_offset_x,
		     box->y2 * src_scale_y + src_offset_y));

		r.x1 = box->x1 + pix_xoff;
		r.x2 = box->x2 + pix_xoff;
		r.y1 = box->y1 + pix_yoff;
		r.y2 = box->y2 + pix_yoff;

		gen7_get_rectangles(sna, &tmp, 1, gen7_emit_video_state);

		OUT_VERTEX(r.x2, r.y2);
		OUT_VERTEX_F(box->x2 * src_scale_x + src_offset_x);
		OUT_VERTEX_F(box->y2 * src_scale_y + src_offset_y);

		OUT_VERTEX(r.x1, r.y2);
		OUT_VERTEX_F(box->x1 * src_scale_x + src_offset_x);
		OUT_VERTEX_F(box->y2 * src_scale_y + src_offset_y);

		OUT_VERTEX(r.x1, r.y1);
		OUT_VERTEX_F(box->x1 * src_scale_x + src_offset_x);
		OUT_VERTEX_F(box->y1 * src_scale_y + src_offset_y);

		if (!DAMAGE_IS_ALL(priv->gpu_damage)) {
			sna_damage_add_box(&priv->gpu_damage, &r);
			sna_damage_subtract_box(&priv->cpu_damage, &r);
		}
		box++;
	}

	gen4_vertex_flush(sna);
	return true;
}
#endif

static void gen7_render_composite_done(struct sna *sna,
				       const struct sna_composite_op *op)
{
	if (sna->render.vertex_offset) {
		gen4_vertex_flush(sna);
		gen7_magic_ca_pass(sna, op);
	}
}





























































































#if 0
static bool
gen7_render_fill_boxes(struct sna *sna,
		       CARD8 op,
		       PictFormat format,
		       const xRenderColor *color,
		       PixmapPtr dst, struct kgem_bo *dst_bo,
		       const BoxRec *box, int n)
{
	struct sna_composite_op tmp;
	uint32_t pixel;

	DBG(("%s (op=%d, color=(%04x, %04x, %04x, %04x) [%08x])\n",
	     __FUNCTION__, op,
	     color->red, color->green, color->blue, color->alpha, (int)format));

	if (op >= ARRAY_SIZE(gen7_blend_op)) {
		DBG(("%s: fallback due to unhandled blend op: %d\n",
		     __FUNCTION__, op));
		return false;
	}

	if (prefer_blt_fill(sna, dst_bo, FILL_BOXES) ||
	    !gen7_check_dst_format(format)) {
		uint8_t alu = GXinvalid;

		if (op <= PictOpSrc) {
			pixel = 0;
			if (op == PictOpClear)
				alu = GXclear;
			else if (sna_get_pixel_from_rgba(&pixel,
							 color->red,
							 color->green,
							 color->blue,
							 color->alpha,
							 format))
				alu = GXcopy;
		}

		if (alu != GXinvalid &&
		    sna_blt_fill_boxes(sna, alu,
				       dst_bo, dst->drawable.bitsPerPixel,
				       pixel, box, n))
			return true;

		if (!gen7_check_dst_format(format))
			return false;
	}

	if (op == PictOpClear) {
		pixel = 0;
		op = PictOpSrc;
	} else if (!sna_get_pixel_from_rgba(&pixel,
					    color->red,
					    color->green,
					    color->blue,
					    color->alpha,
					    PICT_a8r8g8b8))
		return false;

	DBG(("%s(%08x x %d [(%d, %d), (%d, %d) ...])\n",
	     __FUNCTION__, pixel, n,
	     box[0].x1, box[0].y1, box[0].x2, box[0].y2));

	tmp.dst.pixmap = dst;
	tmp.dst.width  = dst->drawable.width;
	tmp.dst.height = dst->drawable.height;
	tmp.dst.format = format;
	tmp.dst.bo = dst_bo;
	tmp.dst.x = tmp.dst.y = 0;
	tmp.damage = NULL;

	sna_render_composite_redirect_init(&tmp);
	if (too_large(dst->drawable.width, dst->drawable.height)) {
		BoxRec extents;

		boxes_extents(box, n, &extents);
		if (!sna_render_composite_redirect(sna, &tmp,
						   extents.x1, extents.y1,
						   extents.x2 - extents.x1,
						   extents.y2 - extents.y1,
						   n > 1))
			return sna_tiling_fill_boxes(sna, op, format, color,
						     dst, dst_bo, box, n);
	}

	tmp.src.bo = sna_render_get_solid(sna, pixel);
	tmp.mask.bo = NULL;

	tmp.floats_per_vertex = 2;
	tmp.floats_per_rect = 6;
	tmp.need_magic_ca_pass = false;

	tmp.u.gen7.flags = FILL_FLAGS(op, format);

	kgem_set_mode(&sna->kgem, KGEM_RENDER, dst_bo);
	if (!kgem_check_bo(&sna->kgem, dst_bo, NULL)) {
		kgem_submit(&sna->kgem);
		if (!kgem_check_bo(&sna->kgem, dst_bo, NULL)) {
			kgem_bo_destroy(&sna->kgem, tmp.src.bo);
			if (tmp.redirect.real_bo)
				kgem_bo_destroy(&sna->kgem, tmp.dst.bo);
			return false;
		}
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	gen7_align_vertex(sna, &tmp);
	gen7_emit_fill_state(sna, &tmp);

	do {
		int n_this_time;
		int16_t *v;

		n_this_time = gen7_get_rectangles(sna, &tmp, n,
						  gen7_emit_fill_state);
		n -= n_this_time;

		v = (int16_t *)(sna->render.vertices + sna->render.vertex_used);
		sna->render.vertex_used += 6 * n_this_time;
		assert(sna->render.vertex_used <= sna->render.vertex_size);
		do {
			DBG(("	(%d, %d), (%d, %d)\n",
			     box->x1, box->y1, box->x2, box->y2));

			v[0] = box->x2;
			v[5] = v[1] = box->y2;
			v[8] = v[4] = box->x1;
			v[9] = box->y1;
			v[2] = v[3]  = v[7]  = 1;
			v[6] = v[10] = v[11] = 0;
			v += 12; box++;
		} while (--n_this_time);
	} while (n);

	gen4_vertex_flush(sna);
	kgem_bo_destroy(&sna->kgem, tmp.src.bo);
	sna_render_composite_redirect_done(sna, &tmp);
	return true;
}
#endif

static void gen7_render_reset(struct sna *sna)
{
	sna->render_state.gen7.emit_flush = false;
	sna->render_state.gen7.needs_invariant = true;
	sna->render_state.gen7.ve_id = 3 << 2;
	sna->render_state.gen7.last_primitive = -1;

	sna->render_state.gen7.num_sf_outputs = 0;
	sna->render_state.gen7.samplers = -1;
	sna->render_state.gen7.blend = -1;
	sna->render_state.gen7.kernel = -1;
	sna->render_state.gen7.drawrect_offset = -1;
	sna->render_state.gen7.drawrect_limit = -1;
	sna->render_state.gen7.surface_table = -1;

	if (sna->render.vbo && !kgem_bo_can_map(&sna->kgem, sna->render.vbo)) {
		DBG(("%s: discarding unmappable vbo\n", __FUNCTION__));
		discard_vbo(sna);
	}

	sna->render.vertex_offset = 0;
	sna->render.nvertex_reloc = 0;
	sna->render.vb_id = 0;
}

static void gen7_render_fini(struct sna *sna)
{
	kgem_bo_destroy(&sna->kgem, sna->render_state.gen7.general_bo);
}

static bool is_gt3(struct sna *sna, int devid)
{
	assert(sna->kgem.gen == 075);
	return devid & 0x20;
}

static bool is_gt2(struct sna *sna, int devid)
{
	return devid & (is_hsw(sna)? 0x30 : 0x20);
}

static bool is_mobile(struct sna *sna, int devid)
{
	return (devid & 0xf) == 0x6;
}

static bool gen7_render_setup(struct sna *sna, int devid)
{
    struct gen7_render_state *state = &sna->render_state.gen7;
    struct sna_static_stream general;
    struct gen7_sampler_state *ss;
    int i, j, k, l, m;

	if (is_ivb(sna)) {
        state->info = &ivb_gt_info;
		if (devid & 0xf) {
            state->info = &ivb_gt1_info;
			if (is_gt2(sna, devid))
                state->info = &ivb_gt2_info; /* XXX requires GT_MODE WiZ disabled */
        }
	} else if (is_byt(sna)) {
		state->info = &byt_gt_info;
	} else if (is_hsw(sna)) {
        state->info = &hsw_gt_info;
		if (devid & 0xf) {
			if (is_gt3(sna, devid))
				state->info = &hsw_gt3_info;
			else if (is_gt2(sna, devid))
				state->info = &hsw_gt2_info;
			else
            state->info = &hsw_gt1_info;
        }
    } else
        return false;

	state->gt = state->info->gt;

    sna_static_stream_init(&general);

    /* Zero pad the start. If you see an offset of 0x0 in the batchbuffer
     * dumps, you know it points to zero.
     */
    null_create(&general);

    for (m = 0; m < GEN7_WM_KERNEL_COUNT; m++) {
        if (wm_kernels[m].size) {
            state->wm_kernel[m][1] =
                sna_static_stream_add(&general,
                              wm_kernels[m].data,
                              wm_kernels[m].size,
                              64);
        } else {
            if (USE_8_PIXEL_DISPATCH) {
                state->wm_kernel[m][0] =
                    sna_static_stream_compile_wm(sna, &general,
                                     wm_kernels[m].data, 8);
            }

            if (USE_16_PIXEL_DISPATCH) {
                state->wm_kernel[m][1] =
                    sna_static_stream_compile_wm(sna, &general,
                                     wm_kernels[m].data, 16);
            }

            if (USE_32_PIXEL_DISPATCH) {
                state->wm_kernel[m][2] =
                    sna_static_stream_compile_wm(sna, &general,
                                     wm_kernels[m].data, 32);
            }
        }
        assert(state->wm_kernel[m][0]|state->wm_kernel[m][1]|state->wm_kernel[m][2]);
    }

    ss = sna_static_stream_map(&general,
                   2 * sizeof(*ss) *
                   (2 +
                    FILTER_COUNT * EXTEND_COUNT *
                    FILTER_COUNT * EXTEND_COUNT),
                   32);
    state->wm_state = sna_static_stream_offsetof(&general, ss);
    sampler_copy_init(ss); ss += 2;
    sampler_fill_init(ss); ss += 2;
    for (i = 0; i < FILTER_COUNT; i++) {
        for (j = 0; j < EXTEND_COUNT; j++) {
            for (k = 0; k < FILTER_COUNT; k++) {
                for (l = 0; l < EXTEND_COUNT; l++) {
                    sampler_state_init(ss++, i, j);
                    sampler_state_init(ss++, k, l);
                }
            }
        }
    }

    state->cc_blend = gen7_composite_create_blend_state(&general);

    state->general_bo = sna_static_stream_fini(sna, &general);
    return state->general_bo != NULL;
}

const char *gen7_render_init(struct sna *sna, const char *backend)
{
	int devid = intel_get_device_id(sna);

	if (!gen7_render_setup(sna, devid))
		return backend;

	sna->kgem.context_switch = gen6_render_context_switch;
	sna->kgem.retire = gen6_render_retire;
	sna->kgem.expire = gen4_render_expire;

#if 0
#if !NO_COMPOSITE
	sna->render.composite = gen7_render_composite;
	sna->render.prefer_gpu |= PREFER_GPU_RENDER;
#endif
#if !NO_COMPOSITE_SPANS
	sna->render.check_composite_spans = gen7_check_composite_spans;
	sna->render.composite_spans = gen7_render_composite_spans;
	if (is_mobile(sna, devid) || is_gt2(sna, devid) || is_byt(sna))
		sna->render.prefer_gpu |= PREFER_GPU_SPANS;
#endif
	sna->render.video = gen7_render_video;

#if !NO_COPY_BOXES
	sna->render.copy_boxes = gen7_render_copy_boxes;
#endif
#if !NO_COPY
	sna->render.copy = gen7_render_copy;
#endif

#if !NO_FILL_BOXES
	sna->render.fill_boxes = gen7_render_fill_boxes;
#endif
#if !NO_FILL
	sna->render.fill = gen7_render_fill;
#endif
#if !NO_FILL_ONE
	sna->render.fill_one = gen7_render_fill_one;
#endif
#if !NO_FILL_CLEAR
	sna->render.clear = gen7_render_clear;
#endif
#endif

    sna->render.blit_tex = gen7_blit_tex;
    sna->render.caps = HW_BIT_BLIT | HW_TEX_BLIT;

	sna->render.flush = gen4_render_flush;
    sna->render.reset = gen7_render_reset;
    sna->render.fini = gen7_render_fini;

    sna->render.max_3d_size = GEN7_MAX_SIZE;
    sna->render.max_3d_pitch = 1 << 18;
	return sna->render_state.gen7.info->name;
}


static bool
gen7_blit_tex(struct sna *sna,
              uint8_t op, bool scale,
		      PixmapPtr src, struct kgem_bo *src_bo,
		      PixmapPtr mask,struct kgem_bo *mask_bo,
		      PixmapPtr dst, struct kgem_bo *dst_bo,
              int32_t src_x, int32_t src_y,
              int32_t msk_x, int32_t msk_y,
              int32_t dst_x, int32_t dst_y,
              int32_t width, int32_t height,
              struct sna_composite_op *tmp)
{


    tmp->op = PictOpSrc;

    tmp->dst.pixmap = dst;
    tmp->dst.bo     = dst_bo;
    tmp->dst.width  = dst->drawable.width;
    tmp->dst.height = dst->drawable.height;
    tmp->dst.format = PICT_a8r8g8b8;


	tmp->src.repeat = RepeatNone;
	tmp->src.filter = PictFilterNearest;
    tmp->src.is_affine = true;

    tmp->src.bo = src_bo;
	tmp->src.pict_format = PICT_x8r8g8b8;
    tmp->src.card_format = gen7_get_card_format(tmp->src.pict_format);
    tmp->src.width  = src->drawable.width;
    tmp->src.height = src->drawable.height;


	tmp->is_affine = tmp->src.is_affine;
	tmp->has_component_alpha = false;
	tmp->need_magic_ca_pass = false;

	tmp->mask.repeat = SAMPLER_EXTEND_NONE;
	tmp->mask.filter = SAMPLER_FILTER_NEAREST;
    tmp->mask.is_affine = true;

    tmp->mask.bo = mask_bo;
    tmp->mask.pict_format = PIXMAN_a8;
    tmp->mask.card_format = gen7_get_card_format(tmp->mask.pict_format);
    tmp->mask.width  = mask->drawable.width;
    tmp->mask.height = mask->drawable.height;

    if( scale )
    {
        tmp->src.scale[0] = 1.f/width;
        tmp->src.scale[1] = 1.f/height;
    }
    else
    {
        tmp->src.scale[0] = 1.f/src->drawable.width;
        tmp->src.scale[1] = 1.f/src->drawable.height;
    }

    tmp->mask.scale[0] = 1.f/mask->drawable.width;
    tmp->mask.scale[1] = 1.f/mask->drawable.height;



	tmp->u.gen7.flags =
		GEN7_SET_FLAGS(SAMPLER_OFFSET(tmp->src.filter,
					      tmp->src.repeat,
					      tmp->mask.filter,
					      tmp->mask.repeat),
			       gen7_get_blend(tmp->op,
					      tmp->has_component_alpha,
					      tmp->dst.format),
/*			       gen7_choose_composite_kernel(tmp->op,
							    tmp->mask.bo != NULL,
							    tmp->has_component_alpha,
							    tmp->is_affine), */
                   GEN7_WM_KERNEL_MASK,
			       gen4_choose_composite_emitter(sna, tmp));

	tmp->blt   = gen7_render_composite_blt;
//	tmp->box   = gen7_render_composite_box;
	tmp->done  = gen7_render_composite_done;

	kgem_set_mode(&sna->kgem, KGEM_RENDER, dst_bo);
	if (!kgem_check_bo(&sna->kgem,
			   tmp->dst.bo, tmp->src.bo, tmp->mask.bo,
			   NULL)) {
		kgem_submit(&sna->kgem);
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	gen7_align_vertex(sna, tmp);
	gen7_emit_composite_state(sna, tmp);
	return true;
}
