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

#include <drmP.h>
#include <drm.h>
#include "i915_drm.h"
#include "i915_drv.h"
#include "intel_drv.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <errno-base.h>
#include <memory.h>

#include <syscall.h>

#include "../bitmap.h"

#include "sna.h"
//#include "sna_reg.h"
#include "sna_render.h"
//#include "sna_render_inline.h"
//#include "sna_video.h"

#include "gen6_render.h"


#define NO_COMPOSITE 0
#define NO_COMPOSITE_SPANS 0
#define NO_COPY 0
#define NO_COPY_BOXES 0
#define NO_FILL 0
#define NO_FILL_BOXES 0
#define NO_CLEAR 0

#define NO_RING_SWITCH 1

#define GEN6_MAX_SIZE 8192

static const uint32_t ps_kernel_nomask_affine[][4] = {
#include "exa_wm_src_affine.g6b"
#include "exa_wm_src_sample_argb.g6b"
#include "exa_wm_write.g6b"
};

static const uint32_t ps_kernel_masknoca_affine[][4] = {
#include "exa_wm_src_affine.g6b"
#include "exa_wm_src_sample_argb.g6b"
#include "exa_wm_mask_affine.g6b"
#include "exa_wm_mask_sample_a.g6b"
#include "exa_wm_noca.g6b"
#include "exa_wm_write.g6b"
};


#define KERNEL(kernel_enum, kernel, masked) \
    [GEN6_WM_KERNEL_##kernel_enum] = {#kernel_enum, kernel, sizeof(kernel), masked}
static const struct wm_kernel_info {
	const char *name;
	const void *data;
	unsigned int size;
	Bool has_mask;
} wm_kernels[] = {
//   KERNEL(NOMASK, ps_kernel_nomask_affine, FALSE),
//    KERNEL(MASK, ps_kernel_masknoca_affine, TRUE),
    KERNEL(NOMASK, ps_kernel_masknoca_affine, TRUE),
    KERNEL(MASK, ps_kernel_masknoca_affine, TRUE),
};
#undef KERNEL

static const struct blendinfo {
	Bool src_alpha;
	uint32_t src_blend;
	uint32_t dst_blend;
} gen6_blend_op[] = {
	/* Clear */	{0, GEN6_BLENDFACTOR_ZERO, GEN6_BLENDFACTOR_ZERO},
	/* Src */	{0, GEN6_BLENDFACTOR_ONE, GEN6_BLENDFACTOR_ZERO},
	/* Dst */	{0, GEN6_BLENDFACTOR_ZERO, GEN6_BLENDFACTOR_ONE},
	/* Over */	{1, GEN6_BLENDFACTOR_ONE, GEN6_BLENDFACTOR_INV_SRC_ALPHA},
	/* OverReverse */ {0, GEN6_BLENDFACTOR_INV_DST_ALPHA, GEN6_BLENDFACTOR_ONE},
	/* In */	{0, GEN6_BLENDFACTOR_DST_ALPHA, GEN6_BLENDFACTOR_ZERO},
	/* InReverse */	{1, GEN6_BLENDFACTOR_ZERO, GEN6_BLENDFACTOR_SRC_ALPHA},
	/* Out */	{0, GEN6_BLENDFACTOR_INV_DST_ALPHA, GEN6_BLENDFACTOR_ZERO},
	/* OutReverse */ {1, GEN6_BLENDFACTOR_ZERO, GEN6_BLENDFACTOR_INV_SRC_ALPHA},
	/* Atop */	{1, GEN6_BLENDFACTOR_DST_ALPHA, GEN6_BLENDFACTOR_INV_SRC_ALPHA},
	/* AtopReverse */ {1, GEN6_BLENDFACTOR_INV_DST_ALPHA, GEN6_BLENDFACTOR_SRC_ALPHA},
	/* Xor */	{1, GEN6_BLENDFACTOR_INV_DST_ALPHA, GEN6_BLENDFACTOR_INV_SRC_ALPHA},
	/* Add */	{0, GEN6_BLENDFACTOR_ONE, GEN6_BLENDFACTOR_ONE},
};


/**
 * Highest-valued BLENDFACTOR used in gen6_blend_op.
 *
 * This leaves out GEN6_BLENDFACTOR_INV_DST_COLOR,
 * GEN6_BLENDFACTOR_INV_CONST_{COLOR,ALPHA},
 * GEN6_BLENDFACTOR_INV_SRC1_{COLOR,ALPHA}
 */
#define GEN6_BLENDFACTOR_COUNT (GEN6_BLENDFACTOR_INV_DST_ALPHA + 1)

/* FIXME: surface format defined in gen6_defines.h, shared Sampling engine
 * 1.7.2

static const struct formatinfo {
	CARD32 pict_fmt;
	uint32_t card_fmt;
} gen6_tex_formats[] = {
	{PICT_a8, GEN6_SURFACEFORMAT_A8_UNORM},
	{PICT_a8r8g8b8, GEN6_SURFACEFORMAT_B8G8R8A8_UNORM},
	{PICT_x8r8g8b8, GEN6_SURFACEFORMAT_B8G8R8X8_UNORM},
	{PICT_a8b8g8r8, GEN6_SURFACEFORMAT_R8G8B8A8_UNORM},
	{PICT_x8b8g8r8, GEN6_SURFACEFORMAT_R8G8B8X8_UNORM},
	{PICT_r8g8b8, GEN6_SURFACEFORMAT_R8G8B8_UNORM},
	{PICT_r5g6b5, GEN6_SURFACEFORMAT_B5G6R5_UNORM},
	{PICT_a1r5g5b5, GEN6_SURFACEFORMAT_B5G5R5A1_UNORM},
	{PICT_a2r10g10b10, GEN6_SURFACEFORMAT_B10G10R10A2_UNORM},
	{PICT_x2r10g10b10, GEN6_SURFACEFORMAT_B10G10R10X2_UNORM},
	{PICT_a2b10g10r10, GEN6_SURFACEFORMAT_R10G10B10A2_UNORM},
	{PICT_x2r10g10b10, GEN6_SURFACEFORMAT_B10G10R10X2_UNORM},
	{PICT_a4r4g4b4, GEN6_SURFACEFORMAT_B4G4R4A4_UNORM},
};
 */

#define GEN6_BLEND_STATE_PADDED_SIZE	ALIGN(sizeof(struct gen6_blend_state), 64)

#define BLEND_OFFSET(s, d) \
	(((s) * GEN6_BLENDFACTOR_COUNT + (d)) * GEN6_BLEND_STATE_PADDED_SIZE)

#define SAMPLER_OFFSET(sf, se, mf, me) \
	(((((sf) * EXTEND_COUNT + (se)) * FILTER_COUNT + (mf)) * EXTEND_COUNT + (me)) * 2 * sizeof(struct gen6_sampler_state))

#define OUT_BATCH(v) batch_emit(sna, v)
#define OUT_VERTEX(x,y) vertex_emit_2s(sna, x,y)
#define OUT_VERTEX_F(v) vertex_emit(sna, v)

static inline bool too_large(int width, int height)
{
	return width > GEN6_MAX_SIZE || height > GEN6_MAX_SIZE;
}

static uint32_t gen6_get_blend(int op,
			       bool has_component_alpha,
			       uint32_t dst_format)
{
	uint32_t src, dst;

//    src = GEN6_BLENDFACTOR_ONE; //gen6_blend_op[op].src_blend;
//    dst = GEN6_BLENDFACTOR_ZERO; //gen6_blend_op[op].dst_blend;

    src = GEN6_BLENDFACTOR_ONE; //gen6_blend_op[op].src_blend;
    dst = GEN6_BLENDFACTOR_INV_SRC_ALPHA; //gen6_blend_op[op].dst_blend;

#if 0
	/* If there's no dst alpha channel, adjust the blend op so that
	 * we'll treat it always as 1.
	 */
	if (PICT_FORMAT_A(dst_format) == 0) {
		if (src == GEN6_BLENDFACTOR_DST_ALPHA)
			src = GEN6_BLENDFACTOR_ONE;
		else if (src == GEN6_BLENDFACTOR_INV_DST_ALPHA)
			src = GEN6_BLENDFACTOR_ZERO;
	}

	/* If the source alpha is being used, then we should only be in a
	 * case where the source blend factor is 0, and the source blend
	 * value is the mask channels multiplied by the source picture's alpha.
	 */
	if (has_component_alpha && gen6_blend_op[op].src_alpha) {
		if (dst == GEN6_BLENDFACTOR_SRC_ALPHA)
			dst = GEN6_BLENDFACTOR_SRC_COLOR;
		else if (dst == GEN6_BLENDFACTOR_INV_SRC_ALPHA)
			dst = GEN6_BLENDFACTOR_INV_SRC_COLOR;
	}

	DBG(("blend op=%d, dst=%x [A=%d] => src=%d, dst=%d => offset=%x\n",
	     op, dst_format, PICT_FORMAT_A(dst_format),
	     src, dst, (int)BLEND_OFFSET(src, dst)));
#endif

	return BLEND_OFFSET(src, dst);
}

static uint32_t gen6_get_dest_format(CARD32 format)
{
    return GEN6_SURFACEFORMAT_B8G8R8A8_UNORM;

/*
	switch (format) {
	default:
		assert(0);
	case PICT_a8r8g8b8:
	case PICT_x8r8g8b8:
		return GEN6_SURFACEFORMAT_B8G8R8A8_UNORM;
	case PICT_a8b8g8r8:
	case PICT_x8b8g8r8:
		return GEN6_SURFACEFORMAT_R8G8B8A8_UNORM;
	case PICT_a2r10g10b10:
	case PICT_x2r10g10b10:
		return GEN6_SURFACEFORMAT_B10G10R10A2_UNORM;
	case PICT_r5g6b5:
		return GEN6_SURFACEFORMAT_B5G6R5_UNORM;
	case PICT_x1r5g5b5:
	case PICT_a1r5g5b5:
		return GEN6_SURFACEFORMAT_B5G5R5A1_UNORM;
	case PICT_a8:
		return GEN6_SURFACEFORMAT_A8_UNORM;
	case PICT_a4r4g4b4:
	case PICT_x4r4g4b4:
		return GEN6_SURFACEFORMAT_B4G4R4A4_UNORM;
	}
 */
}

#if 0
static Bool gen6_check_dst_format(PictFormat format)
{
	switch (format) {
	case PICT_a8r8g8b8:
	case PICT_x8r8g8b8:
	case PICT_a8b8g8r8:
	case PICT_x8b8g8r8:
	case PICT_a2r10g10b10:
	case PICT_x2r10g10b10:
	case PICT_r5g6b5:
	case PICT_x1r5g5b5:
	case PICT_a1r5g5b5:
	case PICT_a8:
	case PICT_a4r4g4b4:
	case PICT_x4r4g4b4:
		return TRUE;
	}
	return FALSE;
}

static bool gen6_check_format(uint32_t format)
{
	switch (format) {
	case PICT_a8r8g8b8:
	case PICT_x8r8g8b8:
	case PICT_a8b8g8r8:
	case PICT_x8b8g8r8:
	case PICT_a2r10g10b10:
	case PICT_x2r10g10b10:
	case PICT_r8g8b8:
	case PICT_r5g6b5:
	case PICT_a1r5g5b5:
	case PICT_a8:
	case PICT_a4r4g4b4:
	case PICT_x4r4g4b4:
		return true;
	default:
		DBG(("%s: unhandled format: %x\n", __FUNCTION__, format));
		return false;
	}
}

static uint32_t gen6_filter(uint32_t filter)
{
	switch (filter) {
	default:
		assert(0);
	case PictFilterNearest:
		return SAMPLER_FILTER_NEAREST;
	case PictFilterBilinear:
		return SAMPLER_FILTER_BILINEAR;
	}
}

static uint32_t gen6_check_filter(PicturePtr picture)
{
	switch (picture->filter) {
	case PictFilterNearest:
	case PictFilterBilinear:
		return TRUE;
	default:
		return FALSE;
	}
}

static uint32_t gen6_repeat(uint32_t repeat)
{
	switch (repeat) {
	default:
		assert(0);
	case RepeatNone:
		return SAMPLER_EXTEND_NONE;
	case RepeatNormal:
		return SAMPLER_EXTEND_REPEAT;
	case RepeatPad:
		return SAMPLER_EXTEND_PAD;
	case RepeatReflect:
		return SAMPLER_EXTEND_REFLECT;
	}
}

static bool gen6_check_repeat(PicturePtr picture)
{
	if (!picture->repeat)
		return TRUE;

	switch (picture->repeatType) {
	case RepeatNone:
	case RepeatNormal:
	case RepeatPad:
	case RepeatReflect:
		return TRUE;
	default:
		return FALSE;
	}
}
#endif

static int
gen6_choose_composite_kernel(int op, Bool has_mask, Bool is_ca, Bool is_affine)
{
	int base;

	if (has_mask) {
/*
		if (is_ca) {
			if (gen6_blend_op[op].src_alpha)
				base = GEN6_WM_KERNEL_MASKCA_SRCALPHA;
			else
				base = GEN6_WM_KERNEL_MASKCA;
		} else
			base = GEN6_WM_KERNEL_MASK;
*/
	} else
		base = GEN6_WM_KERNEL_NOMASK;

	return base + !is_affine;
}

static void
gen6_emit_urb(struct sna *sna)
{
	OUT_BATCH(GEN6_3DSTATE_URB | (3 - 2));
	OUT_BATCH(((1 - 1) << GEN6_3DSTATE_URB_VS_SIZE_SHIFT) |
		  (24 << GEN6_3DSTATE_URB_VS_ENTRIES_SHIFT)); /* at least 24 on GEN6 */
	OUT_BATCH((0 << GEN6_3DSTATE_URB_GS_SIZE_SHIFT) |
		  (0 << GEN6_3DSTATE_URB_GS_ENTRIES_SHIFT)); /* no GS thread */
}

static void
gen6_emit_state_base_address(struct sna *sna)
{
	OUT_BATCH(GEN6_STATE_BASE_ADDRESS | (10 - 2));
	OUT_BATCH(0); /* general */

    OUT_BATCH((sna->kgem.batch_obj->gtt_offset+
              sna->kgem.batch_idx*4096)|BASE_ADDRESS_MODIFY);

    OUT_BATCH(sna->render_state.gen6.general_bo->gaddr|BASE_ADDRESS_MODIFY);

	OUT_BATCH(0); /* indirect */

    OUT_BATCH(sna->render_state.gen6.general_bo->gaddr|BASE_ADDRESS_MODIFY);

	/* upper bounds, disable */
	OUT_BATCH(0);
	OUT_BATCH(BASE_ADDRESS_MODIFY);
	OUT_BATCH(0);
	OUT_BATCH(BASE_ADDRESS_MODIFY);
}

static void
gen6_emit_viewports(struct sna *sna)
{
	OUT_BATCH(GEN6_3DSTATE_VIEWPORT_STATE_POINTERS |
		  GEN6_3DSTATE_VIEWPORT_STATE_MODIFY_CC |
		  (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(sna->render_state.gen6.cc_vp);
}

static void
gen6_emit_vs(struct sna *sna)
{
	/* disable VS constant buffer */
	OUT_BATCH(GEN6_3DSTATE_CONSTANT_VS | (5 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_VS | (6 - 2));
	OUT_BATCH(0); /* no VS kernel */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */
}

static void
gen6_emit_gs(struct sna *sna)
{
	/* disable GS constant buffer */
	OUT_BATCH(GEN6_3DSTATE_CONSTANT_GS | (5 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_GS | (7 - 2));
	OUT_BATCH(0); /* no GS kernel */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */
}

static void
gen6_emit_clip(struct sna *sna)
{
	OUT_BATCH(GEN6_3DSTATE_CLIP | (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */
	OUT_BATCH(0);
}

static void
gen6_emit_wm_constants(struct sna *sna)
{
	/* disable WM constant buffer */
	OUT_BATCH(GEN6_3DSTATE_CONSTANT_PS | (5 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

static void
gen6_emit_null_depth_buffer(struct sna *sna)
{
	OUT_BATCH(GEN6_3DSTATE_DEPTH_BUFFER | (7 - 2));
	OUT_BATCH(GEN6_SURFACE_NULL << GEN6_3DSTATE_DEPTH_BUFFER_TYPE_SHIFT |
		  GEN6_DEPTHFORMAT_D32_FLOAT << GEN6_3DSTATE_DEPTH_BUFFER_FORMAT_SHIFT);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_CLEAR_PARAMS | (2 - 2));
	OUT_BATCH(0);
}

static void
gen6_emit_invariant(struct sna *sna)
{
	OUT_BATCH(GEN6_PIPELINE_SELECT | PIPELINE_SELECT_3D);

	OUT_BATCH(GEN6_3DSTATE_MULTISAMPLE | (3 - 2));
	OUT_BATCH(GEN6_3DSTATE_MULTISAMPLE_PIXEL_LOCATION_CENTER |
              GEN6_3DSTATE_MULTISAMPLE_NUMSAMPLES_1); /* 1 sample/pixel */
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_SAMPLE_MASK | (2 - 2));
	OUT_BATCH(1);

	gen6_emit_urb(sna);

	gen6_emit_state_base_address(sna);

	gen6_emit_viewports(sna);
	gen6_emit_vs(sna);
	gen6_emit_gs(sna);
	gen6_emit_clip(sna);
	gen6_emit_wm_constants(sna);
	gen6_emit_null_depth_buffer(sna);

	sna->render_state.gen6.needs_invariant = FALSE;
}

static bool
gen6_emit_cc(struct sna *sna,
	     int op, bool has_component_alpha, uint32_t dst_format)
{
	struct gen6_render_state *render = &sna->render_state.gen6;
	uint32_t blend;

	blend = gen6_get_blend(op, has_component_alpha, dst_format);

	DBG(("%s(op=%d, ca=%d, format=%x): new=%x, current=%x\n",
	     __FUNCTION__,
	     op, has_component_alpha, dst_format,
	     blend, render->blend));
	if (render->blend == blend)
		return op <= PictOpSrc;

	OUT_BATCH(GEN6_3DSTATE_CC_STATE_POINTERS | (4 - 2));
	OUT_BATCH((render->cc_blend + blend) | 1);
	if (render->blend == (unsigned)-1) {
		OUT_BATCH(1);
		OUT_BATCH(1);
	} else {
		OUT_BATCH(0);
		OUT_BATCH(0);
	}

	render->blend = blend;
	return op <= PictOpSrc;
}

static void
gen6_emit_sampler(struct sna *sna, uint32_t state)
{
	assert(state <
	       2 * sizeof(struct gen6_sampler_state) *
	       FILTER_COUNT * EXTEND_COUNT *
	       FILTER_COUNT * EXTEND_COUNT);

	if (sna->render_state.gen6.samplers == state)
		return;

	sna->render_state.gen6.samplers = state;

	OUT_BATCH(GEN6_3DSTATE_SAMPLER_STATE_POINTERS |
		  GEN6_3DSTATE_SAMPLER_STATE_MODIFY_PS |
		  (4 - 2));
	OUT_BATCH(0); /* VS */
	OUT_BATCH(0); /* GS */
	OUT_BATCH(sna->render_state.gen6.wm_state + state);
}

static void
gen6_emit_sf(struct sna *sna, Bool has_mask)
{
	int num_sf_outputs = has_mask ? 2 : 1;

	if (sna->render_state.gen6.num_sf_outputs == num_sf_outputs)
		return;

	DBG(("%s: num_sf_outputs=%d, read_length=%d, read_offset=%d\n",
	     __FUNCTION__, num_sf_outputs, 1, 0));

	sna->render_state.gen6.num_sf_outputs = num_sf_outputs;

	OUT_BATCH(GEN6_3DSTATE_SF | (20 - 2));
	OUT_BATCH(num_sf_outputs << GEN6_3DSTATE_SF_NUM_OUTPUTS_SHIFT |
		  1 << GEN6_3DSTATE_SF_URB_ENTRY_READ_LENGTH_SHIFT |
		  1 << GEN6_3DSTATE_SF_URB_ENTRY_READ_OFFSET_SHIFT);
	OUT_BATCH(0);
	OUT_BATCH(GEN6_3DSTATE_SF_CULL_NONE);
	OUT_BATCH(2 << GEN6_3DSTATE_SF_TRIFAN_PROVOKE_SHIFT); /* DW4 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW9 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW14 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW19 */
}

static void
gen6_emit_wm(struct sna *sna, unsigned int kernel, int nr_surfaces, int nr_inputs)
{
	if (sna->render_state.gen6.kernel == kernel)
		return;

	sna->render_state.gen6.kernel = kernel;

	DBG(("%s: switching to %s\n", __FUNCTION__, wm_kernels[kernel].name));

	OUT_BATCH(GEN6_3DSTATE_WM | (9 - 2));
	OUT_BATCH(sna->render_state.gen6.wm_kernel[kernel]);
	OUT_BATCH(1 << GEN6_3DSTATE_WM_SAMPLER_COUNT_SHIFT |
		  nr_surfaces << GEN6_3DSTATE_WM_BINDING_TABLE_ENTRY_COUNT_SHIFT);
	OUT_BATCH(0);
	OUT_BATCH(6 << GEN6_3DSTATE_WM_DISPATCH_START_GRF_0_SHIFT); /* DW4 */
	OUT_BATCH((40 - 1) << GEN6_3DSTATE_WM_MAX_THREADS_SHIFT |
		  GEN6_3DSTATE_WM_DISPATCH_ENABLE |
		  GEN6_3DSTATE_WM_16_DISPATCH_ENABLE);
	OUT_BATCH(nr_inputs << GEN6_3DSTATE_WM_NUM_SF_OUTPUTS_SHIFT |
		  GEN6_3DSTATE_WM_PERSPECTIVE_PIXEL_BARYCENTRIC);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

static bool
gen6_emit_binding_table(struct sna *sna, uint16_t offset)
{
	if (sna->render_state.gen6.surface_table == offset)
		return false;

	/* Binding table pointers */
	OUT_BATCH(GEN6_3DSTATE_BINDING_TABLE_POINTERS |
		  GEN6_3DSTATE_BINDING_TABLE_MODIFY_PS |
		  (4 - 2));
	OUT_BATCH(0);		/* vs */
	OUT_BATCH(0);		/* gs */
	/* Only the PS uses the binding table */
	OUT_BATCH(offset*4);

	sna->render_state.gen6.surface_table = offset;
	return true;
}

static bool
gen6_emit_drawing_rectangle(struct sna *sna,
			    const struct sna_composite_op *op)
{
	uint32_t limit = (op->dst.height - 1) << 16 | (op->dst.width - 1);
	uint32_t offset = (uint16_t)op->dst.y << 16 | (uint16_t)op->dst.x;

	assert(!too_large(op->dst.x, op->dst.y));
	assert(!too_large(op->dst.width, op->dst.height));

	if (sna->render_state.gen6.drawrect_limit  == limit &&
	    sna->render_state.gen6.drawrect_offset == offset)
		return false;

	/* [DevSNB-C+{W/A}] Before any depth stall flush (including those
	 * produced by non-pipelined state commands), software needs to first
	 * send a PIPE_CONTROL with no bits set except Post-Sync Operation !=
	 * 0.
	 *
	 * [Dev-SNB{W/A}]: Pipe-control with CS-stall bit set must be sent
	 * BEFORE the pipe-control with a post-sync op and no write-cache
	 * flushes.
	 */
	OUT_BATCH(GEN6_PIPE_CONTROL | (4 - 2));
	OUT_BATCH(GEN6_PIPE_CONTROL_CS_STALL |
		  GEN6_PIPE_CONTROL_STALL_AT_SCOREBOARD);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN6_PIPE_CONTROL | (4 - 2));
	OUT_BATCH(GEN6_PIPE_CONTROL_WRITE_TIME);

    OUT_BATCH(sna->render_state.gen6.general_bo->gaddr+64);

	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_DRAWING_RECTANGLE | (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(limit);
	OUT_BATCH(offset);

	sna->render_state.gen6.drawrect_offset = offset;
	sna->render_state.gen6.drawrect_limit = limit;
	return true;
}

static void
gen6_emit_vertex_elements(struct sna *sna,
			  const struct sna_composite_op *op)
{
	/*
	 * vertex data in vertex buffer
	 *    position: (x, y)
	 *    texture coordinate 0: (u0, v0) if (is_affine is TRUE) else (u0, v0, w0)
	 *    texture coordinate 1 if (has_mask is TRUE): same as above
	 */
	struct gen6_render_state *render = &sna->render_state.gen6;
	int nelem = op->mask.bo ? 2 : 1;
	int selem = op->is_affine ? 2 : 3;
	uint32_t w_component;
	uint32_t src_format;
	int id = op->u.gen6.ve_id;

	if (render->ve_id == id)
		return;
	render->ve_id = id;

	if (op->is_affine) {
		src_format = GEN6_SURFACEFORMAT_R32G32_FLOAT;
		w_component = GEN6_VFCOMPONENT_STORE_1_FLT;
	} else {
		src_format = GEN6_SURFACEFORMAT_R32G32B32_FLOAT;
		w_component = GEN6_VFCOMPONENT_STORE_SRC;
	}

	/* The VUE layout
	 *    dword 0-3: pad (0.0, 0.0, 0.0. 0.0)
	 *    dword 4-7: position (x, y, 1.0, 1.0),
	 *    dword 8-11: texture coordinate 0 (u0, v0, w0, 1.0)
	 *    dword 12-15: texture coordinate 1 (u1, v1, w1, 1.0)
	 *
	 * dword 4-15 are fetched from vertex buffer
	 */
	OUT_BATCH(GEN6_3DSTATE_VERTEX_ELEMENTS |
		((2 * (2 + nelem)) + 1 - 2));

	OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
		  GEN6_SURFACEFORMAT_R32G32B32A32_FLOAT << VE0_FORMAT_SHIFT |
		  0 << VE0_OFFSET_SHIFT);
	OUT_BATCH(GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_0_SHIFT |
		  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_1_SHIFT |
		  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT |
		  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_3_SHIFT);

	/* x,y */
	OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
		  GEN6_SURFACEFORMAT_R16G16_SSCALED << VE0_FORMAT_SHIFT |
		  0 << VE0_OFFSET_SHIFT); /* offsets vb in bytes */
	OUT_BATCH(GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT |
		  GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT |
		  GEN6_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT |
		  GEN6_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT);

	/* u0, v0, w0 */
	OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
		  src_format << VE0_FORMAT_SHIFT |
		  4 << VE0_OFFSET_SHIFT);	/* offset vb in bytes */
	OUT_BATCH(GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT |
		  GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT |
		  w_component << VE1_VFCOMPONENT_2_SHIFT |
		  GEN6_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT);

	/* u1, v1, w1 */
	if (op->mask.bo) {
		OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
			  src_format << VE0_FORMAT_SHIFT |
			  ((1 + selem) * 4) << VE0_OFFSET_SHIFT); /* vb offset in bytes */
		OUT_BATCH(GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT |
			  GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT |
			  w_component << VE1_VFCOMPONENT_2_SHIFT |
			  GEN6_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT);
	}
}

static void
gen6_emit_flush(struct sna *sna)
{
	OUT_BATCH(GEN6_PIPE_CONTROL | (4 - 2));
	OUT_BATCH(GEN6_PIPE_CONTROL_WC_FLUSH |
		  GEN6_PIPE_CONTROL_TC_FLUSH |
		  GEN6_PIPE_CONTROL_CS_STALL);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

static void
gen6_emit_state(struct sna *sna,
		const struct sna_composite_op *op,
		uint16_t wm_binding_table)
{
	bool need_stall = wm_binding_table & 1;

	if (gen6_emit_cc(sna, op->op, op->has_component_alpha, op->dst.format))
		need_stall = false;
	gen6_emit_sampler(sna,
			  SAMPLER_OFFSET(op->src.filter,
					 op->src.repeat,
					 op->mask.filter,
					 op->mask.repeat));
	gen6_emit_sf(sna, op->mask.bo != NULL);
	gen6_emit_wm(sna,
		     op->u.gen6.wm_kernel,
		     op->u.gen6.nr_surfaces,
		     op->u.gen6.nr_inputs);
	gen6_emit_vertex_elements(sna, op);
	need_stall |= gen6_emit_binding_table(sna, wm_binding_table & ~1);
	if (gen6_emit_drawing_rectangle(sna, op))
		need_stall = false;
//    if (kgem_bo_is_dirty(op->src.bo) || kgem_bo_is_dirty(op->mask.bo)) {
        gen6_emit_flush(sna);
        kgem_clear_dirty(&sna->kgem);
		kgem_bo_mark_dirty(op->dst.bo);
		need_stall = false;
//   }
	if (need_stall) {
		OUT_BATCH(GEN6_PIPE_CONTROL | (4 - 2));
		OUT_BATCH(GEN6_PIPE_CONTROL_CS_STALL |
			  GEN6_PIPE_CONTROL_STALL_AT_SCOREBOARD);
		OUT_BATCH(0);
		OUT_BATCH(0);
	}
}

static void gen6_magic_ca_pass(struct sna *sna,
			       const struct sna_composite_op *op)
{
	struct gen6_render_state *state = &sna->render_state.gen6;

	if (!op->need_magic_ca_pass)
		return;

	DBG(("%s: CA fixup (%d -> %d)\n", __FUNCTION__,
	     sna->render.vertex_start, sna->render.vertex_index));

	gen6_emit_flush(sna);

	gen6_emit_cc(sna, PictOpAdd, TRUE, op->dst.format);
	gen6_emit_wm(sna,
		     gen6_choose_composite_kernel(PictOpAdd,
						  TRUE, TRUE,
						  op->is_affine),
		     3, 2);

	OUT_BATCH(GEN6_3DPRIMITIVE |
		  GEN6_3DPRIMITIVE_VERTEX_SEQUENTIAL |
		  _3DPRIM_RECTLIST << GEN6_3DPRIMITIVE_TOPOLOGY_SHIFT |
		  0 << 9 |
		  4);
	OUT_BATCH(sna->render.vertex_index - sna->render.vertex_start);
	OUT_BATCH(sna->render.vertex_start);
	OUT_BATCH(1);	/* single instance */
	OUT_BATCH(0);	/* start instance location */
	OUT_BATCH(0);	/* index buffer offset, ignored */

	state->last_primitive = sna->kgem.nbatch;
}

static void gen6_vertex_flush(struct sna *sna)
{
	assert(sna->render_state.gen6.vertex_offset);

	DBG(("%s[%x] = %d\n", __FUNCTION__,
	     4*sna->render_state.gen6.vertex_offset,
	     sna->render.vertex_index - sna->render.vertex_start));
	sna->kgem.batch[sna->render_state.gen6.vertex_offset] =
		sna->render.vertex_index - sna->render.vertex_start;
	sna->render_state.gen6.vertex_offset = 0;
}

static int gen6_vertex_finish(struct sna *sna)
{
	struct kgem_bo *bo;
	unsigned int i;

	DBG(("%s: used=%d / %d\n", __FUNCTION__,
	     sna->render.vertex_used, sna->render.vertex_size));
	assert(sna->render.vertex_used);

	/* Note: we only need dword alignment (currently) */
/*
	bo = sna->render.vbo;
	if (bo) {
		for (i = 0; i < ARRAY_SIZE(sna->render.vertex_reloc); i++) {
			if (sna->render.vertex_reloc[i]) {
				DBG(("%s: reloc[%d] = %d\n", __FUNCTION__,
				     i, sna->render.vertex_reloc[i]));

				sna->kgem.batch[sna->render.vertex_reloc[i]] =
					kgem_add_reloc(&sna->kgem,
						       sna->render.vertex_reloc[i],
						       bo,
						       I915_GEM_DOMAIN_VERTEX << 16,
						       0);
				sna->kgem.batch[sna->render.vertex_reloc[i]+1] =
					kgem_add_reloc(&sna->kgem,
						       sna->render.vertex_reloc[i]+1,
						       bo,
						       I915_GEM_DOMAIN_VERTEX << 16,
						       0 + sna->render.vertex_used * 4 - 1);
				sna->render.vertex_reloc[i] = 0;
			}
		}

		sna->render.vertex_used = 0;
		sna->render.vertex_index = 0;
		sna->render_state.gen6.vb_id = 0;

		kgem_bo_destroy(&sna->kgem, bo);
	}
*/
	sna->render.vertices = NULL;
	sna->render.vbo = kgem_create_linear(&sna->kgem, 256*1024);
	if (sna->render.vbo)
		sna->render.vertices = kgem_bo_map__cpu(&sna->kgem, sna->render.vbo);
	if (sna->render.vertices == NULL) {
		kgem_bo_destroy(&sna->kgem, sna->render.vbo);
		sna->render.vbo = NULL;
		return 0;
	}

//   kgem_bo_sync__cpu(&sna->kgem, sna->render.vbo);
	if (sna->render.vertex_used) {
		DBG(("%s: copying initial buffer x %d to handle=%d\n",
		     __FUNCTION__,
		     sna->render.vertex_used,
		     sna->render.vbo->handle));
		memcpy(sna->render.vertices,
		       sna->render.vertex_data,
		       sizeof(float)*sna->render.vertex_used);
	}
	sna->render.vertex_size = 64 * 1024 - 1;
	return sna->render.vertex_size - sna->render.vertex_used;
}

static void gen6_vertex_close(struct sna *sna)
{
	struct kgem_bo *bo;
	unsigned int i, delta = 0;

	if (!sna->render.vertex_used) {
		assert(sna->render.vbo == NULL);
		assert(sna->render.vertices == sna->render.vertex_data);
		assert(sna->render.vertex_size == ARRAY_SIZE(sna->render.vertex_data));
		return;
	}

	DBG(("%s: used=%d / %d\n", __FUNCTION__,
	     sna->render.vertex_used, sna->render.vertex_size));

	bo = sna->render.vbo;
	if (bo == NULL) {
		assert(sna->render.vertices == sna->render.vertex_data);
		assert(sna->render.vertex_used < ARRAY_SIZE(sna->render.vertex_data));
		if (sna->kgem.nbatch + sna->render.vertex_used <= sna->kgem.surface) {
			DBG(("%s: copy to batch: %d @ %d\n", __FUNCTION__,
			     sna->render.vertex_used, sna->kgem.nbatch));
			memcpy(sna->kgem.batch + sna->kgem.nbatch,
			       sna->render.vertex_data,
			       sna->render.vertex_used * 4);
			delta = sna->kgem.nbatch * 4;
			bo = NULL;
			sna->kgem.nbatch += sna->render.vertex_used;
		} else {
			bo = kgem_create_linear(&sna->kgem, 4*sna->render.vertex_used);
			if (bo && !kgem_bo_write(&sna->kgem, bo,
						 sna->render.vertex_data,
						 4*sna->render.vertex_used)) {
				kgem_bo_destroy(&sna->kgem, bo);
				goto reset;
			}
			DBG(("%s: new vbo: %d\n", __FUNCTION__,
			     sna->render.vertex_used));
		}
	}

	for (i = 0; i < ARRAY_SIZE(sna->render.vertex_reloc); i++) {
		if (sna->render.vertex_reloc[i]) {
			DBG(("%s: reloc[%d] = %d\n", __FUNCTION__,
			     i, sna->render.vertex_reloc[i]));

            sna->kgem.batch[sna->render.vertex_reloc[i]] =
                            sna->kgem.batch_obj->gtt_offset+delta+
                            sna->kgem.batch_idx*4096;

            sna->kgem.batch[sna->render.vertex_reloc[i]+1] =
                            sna->kgem.batch_obj->gtt_offset+delta+
                            sna->kgem.batch_idx*4096+
                            sna->render.vertex_used * 4 - 1;

			sna->render.vertex_reloc[i] = 0;
		}
	}

//   if (bo)
//       kgem_bo_destroy(&sna->kgem, bo);

reset:
	sna->render.vertex_used = 0;
	sna->render.vertex_index = 0;
	sna->render_state.gen6.vb_id = 0;

	sna->render.vbo = NULL;
	sna->render.vertices = sna->render.vertex_data;
	sna->render.vertex_size = ARRAY_SIZE(sna->render.vertex_data);
}

typedef struct gen6_surface_state_padded {
	struct gen6_surface_state state;
	char pad[32 - sizeof(struct gen6_surface_state)];
} gen6_surface_state_padded;

static void null_create(struct sna_static_stream *stream)
{
	/* A bunch of zeros useful for legacy border color and depth-stencil */
	sna_static_stream_map(stream, 64, 64);
}

static void scratch_create(struct sna_static_stream *stream)
{
	/* 64 bytes of scratch space for random writes, such as
	 * the pipe-control w/a.
	 */
	sna_static_stream_map(stream, 64, 64);
}

static void
sampler_state_init(struct gen6_sampler_state *sampler_state,
		   sampler_filter_t filter,
		   sampler_extend_t extend)
{
	sampler_state->ss0.lod_preclamp = 1;	/* GL mode */

	/* We use the legacy mode to get the semantics specified by
	 * the Render extension. */
	sampler_state->ss0.border_color_mode = GEN6_BORDER_COLOR_MODE_LEGACY;

	switch (filter) {
	default:
	case SAMPLER_FILTER_NEAREST:
		sampler_state->ss0.min_filter = GEN6_MAPFILTER_NEAREST;
		sampler_state->ss0.mag_filter = GEN6_MAPFILTER_NEAREST;
		break;
	case SAMPLER_FILTER_BILINEAR:
		sampler_state->ss0.min_filter = GEN6_MAPFILTER_LINEAR;
		sampler_state->ss0.mag_filter = GEN6_MAPFILTER_LINEAR;
		break;
	}

	switch (extend) {
	default:
	case SAMPLER_EXTEND_NONE:
		sampler_state->ss1.r_wrap_mode = GEN6_TEXCOORDMODE_CLAMP_BORDER;
		sampler_state->ss1.s_wrap_mode = GEN6_TEXCOORDMODE_CLAMP_BORDER;
		sampler_state->ss1.t_wrap_mode = GEN6_TEXCOORDMODE_CLAMP_BORDER;
		break;
	case SAMPLER_EXTEND_REPEAT:
		sampler_state->ss1.r_wrap_mode = GEN6_TEXCOORDMODE_WRAP;
		sampler_state->ss1.s_wrap_mode = GEN6_TEXCOORDMODE_WRAP;
		sampler_state->ss1.t_wrap_mode = GEN6_TEXCOORDMODE_WRAP;
		break;
	case SAMPLER_EXTEND_PAD:
		sampler_state->ss1.r_wrap_mode = GEN6_TEXCOORDMODE_CLAMP;
		sampler_state->ss1.s_wrap_mode = GEN6_TEXCOORDMODE_CLAMP;
		sampler_state->ss1.t_wrap_mode = GEN6_TEXCOORDMODE_CLAMP;
		break;
	case SAMPLER_EXTEND_REFLECT:
		sampler_state->ss1.r_wrap_mode = GEN6_TEXCOORDMODE_MIRROR;
		sampler_state->ss1.s_wrap_mode = GEN6_TEXCOORDMODE_MIRROR;
		sampler_state->ss1.t_wrap_mode = GEN6_TEXCOORDMODE_MIRROR;
		break;
	}
}

static uint32_t gen6_create_cc_viewport(struct sna_static_stream *stream)
{
	struct gen6_cc_viewport vp;

	vp.min_depth = -1.e35;
	vp.max_depth = 1.e35;

	return sna_static_stream_add(stream, &vp, sizeof(vp), 32);
}

#if 0

static uint32_t gen6_get_card_format(PictFormat format)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(gen6_tex_formats); i++) {
		if (gen6_tex_formats[i].pict_fmt == format)
			return gen6_tex_formats[i].card_fmt;
	}
	return -1;
}
#endif

static uint32_t
gen6_tiling_bits(uint32_t tiling)
{
    return 0;
/*
	switch (tiling) {
	default: assert(0);
	case I915_TILING_NONE: return 0;
	case I915_TILING_X: return GEN6_SURFACE_TILED;
	case I915_TILING_Y: return GEN6_SURFACE_TILED | GEN6_SURFACE_TILED_Y;
	}
*/
}

/**
 * Sets up the common fields for a surface state buffer for the given
 * picture in the given surface state buffer.
 */
static int
gen6_bind_bo(struct sna *sna,
         struct kgem_bo *bo,
	     uint32_t width,
	     uint32_t height,
	     uint32_t format,
	     Bool is_dst)
{
	uint32_t *ss;
	uint32_t domains;
	uint16_t offset;

	/* After the first bind, we manage the cache domains within the batch */
	if (is_dst) {
		domains = I915_GEM_DOMAIN_RENDER << 16 |I915_GEM_DOMAIN_RENDER;
//       kgem_bo_mark_dirty(bo);
	} else
		domains = I915_GEM_DOMAIN_SAMPLER << 16;

//   offset = kgem_bo_get_binding(bo, format);
//   if (offset) {
//        DBG(("[%x]  bo(handle=%x), format=%d, reuse %s binding\n",
//            offset, bo->handle, format,
//            domains & 0xffff ? "render" : "sampler"));
//       return offset;
//   }

	offset = sna->kgem.surface - sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);
	offset *= sizeof(uint32_t);

	sna->kgem.surface -=
		sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);
	ss = sna->kgem.batch + sna->kgem.surface;
	ss[0] = (GEN6_SURFACE_2D << GEN6_SURFACE_TYPE_SHIFT |
		 GEN6_SURFACE_BLEND_ENABLED |
		 format << GEN6_SURFACE_FORMAT_SHIFT);
    ss[1] = bo->gaddr;
	ss[2] = ((width - 1)  << GEN6_SURFACE_WIDTH_SHIFT |
		 (height - 1) << GEN6_SURFACE_HEIGHT_SHIFT);
	assert(bo->pitch <= (1 << 18));
    ss[3] = (gen6_tiling_bits(0) |
		 (bo->pitch - 1) << GEN6_SURFACE_PITCH_SHIFT);
	ss[4] = 0;
	ss[5] = 0;

//   kgem_bo_set_binding(bo, format, offset);

	DBG(("[%x] bind bo(handle=%d, addr=%d), format=%d, width=%d, height=%d, pitch=%d, tiling=%d -> %s\n",
	     offset, bo->handle, ss[1],
	     format, width, height, bo->pitch, bo->tiling,
	     domains & 0xffff ? "render" : "sampler"));

	return offset;
}


static void gen6_emit_vertex_buffer(struct sna *sna,
				    const struct sna_composite_op *op)
{
	int id = op->u.gen6.ve_id;

	OUT_BATCH(GEN6_3DSTATE_VERTEX_BUFFERS | 3);
	OUT_BATCH(id << VB0_BUFFER_INDEX_SHIFT | VB0_VERTEXDATA |
		  4*op->floats_per_vertex << VB0_BUFFER_PITCH_SHIFT);
	sna->render.vertex_reloc[id] = sna->kgem.nbatch;
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	sna->render_state.gen6.vb_id |= 1 << id;
}

static void gen6_emit_primitive(struct sna *sna)
{
	if (sna->kgem.nbatch == sna->render_state.gen6.last_primitive) {
		DBG(("%s: continuing previous primitive, start=%d, index=%d\n",
		     __FUNCTION__,
		     sna->render.vertex_start,
		     sna->render.vertex_index));
		sna->render_state.gen6.vertex_offset = sna->kgem.nbatch - 5;
		return;
	}

	OUT_BATCH(GEN6_3DPRIMITIVE |
		  GEN6_3DPRIMITIVE_VERTEX_SEQUENTIAL |
		  _3DPRIM_RECTLIST << GEN6_3DPRIMITIVE_TOPOLOGY_SHIFT |
		  0 << 9 |
		  4);
	sna->render_state.gen6.vertex_offset = sna->kgem.nbatch;
	OUT_BATCH(0);	/* vertex count, to be filled in later */
	OUT_BATCH(sna->render.vertex_index);
	OUT_BATCH(1);	/* single instance */
	OUT_BATCH(0);	/* start instance location */
	OUT_BATCH(0);	/* index buffer offset, ignored */
	sna->render.vertex_start = sna->render.vertex_index;
	DBG(("%s: started new primitive: index=%d\n",
	     __FUNCTION__, sna->render.vertex_start));

	sna->render_state.gen6.last_primitive = sna->kgem.nbatch;
}

static bool gen6_rectangle_begin(struct sna *sna,
				 const struct sna_composite_op *op)
{
	int id = 1 << op->u.gen6.ve_id;
	int ndwords;

	ndwords = op->need_magic_ca_pass ? 60 : 6;
	if ((sna->render_state.gen6.vb_id & id) == 0)
		ndwords += 5;
	if (!kgem_check_batch(&sna->kgem, ndwords))
		return false;

	if ((sna->render_state.gen6.vb_id & id) == 0)
		gen6_emit_vertex_buffer(sna, op);

	gen6_emit_primitive(sna);
	return true;
}

static int gen6_get_rectangles__flush(struct sna *sna,
				      const struct sna_composite_op *op)
{
	if (sna->render_state.gen6.vertex_offset) {
		gen6_vertex_flush(sna);
		gen6_magic_ca_pass(sna, op);
	}

	if (!kgem_check_batch(&sna->kgem, op->need_magic_ca_pass ? 65 : 5))
		return 0;
	if (sna->kgem.nexec > KGEM_EXEC_SIZE(&sna->kgem) - 1)
		return 0;
	if (sna->kgem.nreloc > KGEM_RELOC_SIZE(&sna->kgem) - 2)
		return 0;

	return gen6_vertex_finish(sna);
}

inline static int gen6_get_rectangles(struct sna *sna,
				      const struct sna_composite_op *op,
				      int want)
{
	int rem = vertex_space(sna);

	if (rem < op->floats_per_rect) {
		DBG(("flushing vbo for %s: %d < %d\n",
		     __FUNCTION__, rem, op->floats_per_rect));
		rem = gen6_get_rectangles__flush(sna, op);
		if (rem == 0)
			return 0;
	}

	if (sna->render_state.gen6.vertex_offset == 0 &&
	    !gen6_rectangle_begin(sna, op))
		return 0;

	if (want > 1 && want * op->floats_per_rect > rem)
		want = rem / op->floats_per_rect;

	assert(want > 0);
	sna->render.vertex_index += 3*want;
	return want;
}

inline static uint32_t *gen6_composite_get_binding_table(struct sna *sna,
							 uint16_t *offset)
{
	uint32_t *table;

	sna->kgem.surface -=
		sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);
	/* Clear all surplus entries to zero in case of prefetch */
	table = memset(sna->kgem.batch + sna->kgem.surface,
		       0, sizeof(struct gen6_surface_state_padded));

	DBG(("%s(%x)\n", __FUNCTION__, 4*sna->kgem.surface));

	*offset = sna->kgem.surface;
	return table;
}

static uint32_t
gen6_choose_composite_vertex_buffer(const struct sna_composite_op *op)
{
	int has_mask = op->mask.bo != NULL;
	int is_affine = op->is_affine;
	return has_mask << 1 | is_affine;
}

static void
gen6_get_batch(struct sna *sna)
{
	kgem_set_mode(&sna->kgem, KGEM_RENDER);
/*
	if (!kgem_check_batch_with_surfaces(&sna->kgem, 150, 4)) {
		DBG(("%s: flushing batch: %d < %d+%d\n",
		     __FUNCTION__, sna->kgem.surface - sna->kgem.nbatch,
		     150, 4*8));
		kgem_submit(&sna->kgem);
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}
*/

	if (sna->render_state.gen6.needs_invariant)
		gen6_emit_invariant(sna);
}

static void gen6_emit_composite_state(struct sna *sna,
                      const struct sna_composite_op *op)
{
    uint32_t *binding_table;
    uint16_t offset;
    bool dirty;

    gen6_get_batch(sna);
    dirty = FALSE;

    binding_table = gen6_composite_get_binding_table(sna, &offset);

    binding_table[0] =
        gen6_bind_bo(sna,
                op->dst.bo, op->dst.width, op->dst.height,
                op->dst.format,
                TRUE);
    binding_table[1] =
        gen6_bind_bo(sna,
                 op->src.bo, op->src.width, op->src.height,
                 op->src.card_format,
                 FALSE);
    if (op->mask.bo) {
        binding_table[2] =
            gen6_bind_bo(sna,
                     op->mask.bo,
                     op->mask.width,
                     op->mask.height,
                     op->mask.card_format,
                     FALSE);
    }

    if (sna->kgem.surface == offset &&
        *(uint64_t *)(sna->kgem.batch + sna->render_state.gen6.surface_table) == *(uint64_t*)binding_table &&
        (op->mask.bo == NULL ||
         sna->kgem.batch[sna->render_state.gen6.surface_table+2] == binding_table[2])) {
        sna->kgem.surface += sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);
        offset = sna->render_state.gen6.surface_table;
    }

    gen6_emit_state(sna, op, offset | dirty);
}



static void
gen6_align_vertex(struct sna *sna, const struct sna_composite_op *op)
{
	assert (sna->render_state.gen6.vertex_offset == 0);
	if (op->floats_per_vertex != sna->render_state.gen6.floats_per_vertex) {
		if (sna->render.vertex_size - sna->render.vertex_used < 2*op->floats_per_rect)
			/* XXX propagate failure */
			gen6_vertex_finish(sna);

		DBG(("aligning vertex: was %d, now %d floats per vertex, %d->%d\n",
		     sna->render_state.gen6.floats_per_vertex,
		     op->floats_per_vertex,
		     sna->render.vertex_index,
		     (sna->render.vertex_used + op->floats_per_vertex - 1) / op->floats_per_vertex));
		sna->render.vertex_index = (sna->render.vertex_used + op->floats_per_vertex - 1) / op->floats_per_vertex;
		sna->render.vertex_used = sna->render.vertex_index * op->floats_per_vertex;
		sna->render_state.gen6.floats_per_vertex = op->floats_per_vertex;
	}
}


#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

static uint32_t
gen6_composite_create_blend_state(struct sna_static_stream *stream)
{
	char *base, *ptr;
	int src, dst;

	base = sna_static_stream_map(stream,
				     GEN6_BLENDFACTOR_COUNT * GEN6_BLENDFACTOR_COUNT * GEN6_BLEND_STATE_PADDED_SIZE,
				     64);

	ptr = base;
	for (src = 0; src < GEN6_BLENDFACTOR_COUNT; src++) {
		for (dst= 0; dst < GEN6_BLENDFACTOR_COUNT; dst++) {
			struct gen6_blend_state *blend =
				(struct gen6_blend_state *)ptr;

			blend->blend0.dest_blend_factor = dst;
			blend->blend0.source_blend_factor = src;
			blend->blend0.blend_func = GEN6_BLENDFUNCTION_ADD;
			blend->blend0.blend_enable =
				!(dst == GEN6_BLENDFACTOR_ZERO && src == GEN6_BLENDFACTOR_ONE);

			blend->blend1.post_blend_clamp_enable = 1;
			blend->blend1.pre_blend_clamp_enable = 1;

			ptr += GEN6_BLEND_STATE_PADDED_SIZE;
		}
	}

	return sna_static_stream_offsetof(stream, base);
}

#if 0
static uint32_t gen6_bind_video_source(struct sna *sna,
				       struct kgem_bo *src_bo,
				       uint32_t src_offset,
				       int src_width,
				       int src_height,
				       int src_pitch,
				       uint32_t src_surf_format)
{
	struct gen6_surface_state *ss;

	sna->kgem.surface -= sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);

	ss = memset(sna->kgem.batch + sna->kgem.surface, 0, sizeof(*ss));
	ss->ss0.surface_type = GEN6_SURFACE_2D;
	ss->ss0.surface_format = src_surf_format;

	ss->ss1.base_addr =
		kgem_add_reloc(&sna->kgem,
			       sna->kgem.surface + 1,
			       src_bo,
			       I915_GEM_DOMAIN_SAMPLER << 16,
			       src_offset);

	ss->ss2.width  = src_width - 1;
	ss->ss2.height = src_height - 1;
	ss->ss3.pitch  = src_pitch - 1;

	return sna->kgem.surface * sizeof(uint32_t);
}

static void gen6_emit_video_state(struct sna *sna,
				  struct sna_composite_op *op,
				  struct sna_video_frame *frame)
{
	uint32_t src_surf_format;
	uint32_t src_surf_base[6];
	int src_width[6];
	int src_height[6];
	int src_pitch[6];
	uint32_t *binding_table;
	uint16_t offset;
	bool dirty;
	int n_src, n;

	gen6_get_batch(sna);
	dirty = kgem_bo_is_dirty(op->dst.bo);

	src_surf_base[0] = 0;
	src_surf_base[1] = 0;
	src_surf_base[2] = frame->VBufOffset;
	src_surf_base[3] = frame->VBufOffset;
	src_surf_base[4] = frame->UBufOffset;
	src_surf_base[5] = frame->UBufOffset;

	if (is_planar_fourcc(frame->id)) {
		src_surf_format = GEN6_SURFACEFORMAT_R8_UNORM;
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
			src_surf_format = GEN6_SURFACEFORMAT_YCRCB_SWAPY;
		else
			src_surf_format = GEN6_SURFACEFORMAT_YCRCB_NORMAL;

		src_width[0]  = frame->width;
		src_height[0] = frame->height;
		src_pitch[0]  = frame->pitch[0];
		n_src = 1;
	}

	binding_table = gen6_composite_get_binding_table(sna, &offset);

	binding_table[0] =
		gen6_bind_bo(sna,
			     op->dst.bo, op->dst.width, op->dst.height,
			     gen6_get_dest_format(op->dst.format),
			     TRUE);
	for (n = 0; n < n_src; n++) {
		binding_table[1+n] =
			gen6_bind_video_source(sna,
					       frame->bo,
					       src_surf_base[n],
					       src_width[n],
					       src_height[n],
					       src_pitch[n],
					       src_surf_format);
	}

	gen6_emit_state(sna, op, offset | dirty);
}

static Bool
gen6_render_video(struct sna *sna,
		  struct sna_video *video,
		  struct sna_video_frame *frame,
		  RegionPtr dstRegion,
		  short src_w, short src_h,
		  short drw_w, short drw_h,
		  PixmapPtr pixmap)
{
	struct sna_composite_op tmp;
	int nbox, dxo, dyo, pix_xoff, pix_yoff;
	float src_scale_x, src_scale_y;
	struct sna_pixmap *priv;
	BoxPtr box;

	DBG(("%s: src=(%d, %d), dst=(%d, %d), %dx[(%d, %d), (%d, %d)...]\n",
	     __FUNCTION__, src_w, src_h, drw_w, drw_h,
	     REGION_NUM_RECTS(dstRegion),
	     REGION_EXTENTS(NULL, dstRegion)->x1,
	     REGION_EXTENTS(NULL, dstRegion)->y1,
	     REGION_EXTENTS(NULL, dstRegion)->x2,
	     REGION_EXTENTS(NULL, dstRegion)->y2));

	priv = sna_pixmap_force_to_gpu(pixmap, MOVE_READ | MOVE_WRITE);
	if (priv == NULL)
		return FALSE;

	memset(&tmp, 0, sizeof(tmp));

	tmp.op = PictOpSrc;
	tmp.dst.pixmap = pixmap;
	tmp.dst.width  = pixmap->drawable.width;
	tmp.dst.height = pixmap->drawable.height;
	tmp.dst.format = sna_render_format_for_depth(pixmap->drawable.depth);
	tmp.dst.bo = priv->gpu_bo;

	tmp.src.bo = frame->bo;
	tmp.src.filter = SAMPLER_FILTER_BILINEAR;
	tmp.src.repeat = SAMPLER_EXTEND_PAD;

	tmp.mask.bo = NULL;

	tmp.is_affine = TRUE;
	tmp.floats_per_vertex = 3;
	tmp.floats_per_rect = 9;

	if (is_planar_fourcc(frame->id)) {
		tmp.u.gen6.wm_kernel = GEN6_WM_KERNEL_VIDEO_PLANAR;
		tmp.u.gen6.nr_surfaces = 7;
	} else {
		tmp.u.gen6.wm_kernel = GEN6_WM_KERNEL_VIDEO_PACKED;
		tmp.u.gen6.nr_surfaces = 2;
	}
	tmp.u.gen6.nr_inputs = 1;
	tmp.u.gen6.ve_id = 1;

	kgem_set_mode(&sna->kgem, KGEM_RENDER);
	if (!kgem_check_bo(&sna->kgem, tmp.dst.bo, frame->bo, NULL)) {
		kgem_submit(&sna->kgem);
		assert(kgem_check_bo(&sna->kgem, tmp.dst.bo, frame->bo, NULL));
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	gen6_emit_video_state(sna, &tmp, frame);
	gen6_align_vertex(sna, &tmp);

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

	dxo = dstRegion->extents.x1;
	dyo = dstRegion->extents.y1;

	/* Use normalized texture coordinates */
	src_scale_x = ((float)src_w / frame->width) / (float)drw_w;
	src_scale_y = ((float)src_h / frame->height) / (float)drw_h;

	box = REGION_RECTS(dstRegion);
	nbox = REGION_NUM_RECTS(dstRegion);
	while (nbox--) {
		BoxRec r;

		r.x1 = box->x1 + pix_xoff;
		r.x2 = box->x2 + pix_xoff;
		r.y1 = box->y1 + pix_yoff;
		r.y2 = box->y2 + pix_yoff;

		if (unlikely(!gen6_get_rectangles(sna, &tmp, 1))) {
			_kgem_submit(&sna->kgem);
			gen6_emit_video_state(sna, &tmp, frame);
			gen6_get_rectangles(sna, &tmp, 1);
		}

		OUT_VERTEX(r.x2, r.y2);
		OUT_VERTEX_F((box->x2 - dxo) * src_scale_x);
		OUT_VERTEX_F((box->y2 - dyo) * src_scale_y);

		OUT_VERTEX(r.x1, r.y2);
		OUT_VERTEX_F((box->x1 - dxo) * src_scale_x);
		OUT_VERTEX_F((box->y2 - dyo) * src_scale_y);

		OUT_VERTEX(r.x1, r.y1);
		OUT_VERTEX_F((box->x1 - dxo) * src_scale_x);
		OUT_VERTEX_F((box->y1 - dyo) * src_scale_y);

		if (!DAMAGE_IS_ALL(priv->gpu_damage)) {
			sna_damage_add_box(&priv->gpu_damage, &r);
			sna_damage_subtract_box(&priv->cpu_damage, &r);
		}
		box++;
	}
	priv->clear = false;

	gen6_vertex_flush(sna);
	return TRUE;
}

#endif


fastcall static void
gen6_emit_composite_primitive_identity_source_mask(struct sna *sna,
                           const struct sna_composite_op *op,
                           const struct sna_composite_rectangles *r)
{
    union {
        struct sna_coordinate p;
        float f;
    } dst;
    float src_x, src_y;
    float msk_x, msk_y;
    float w, h;
    float *v;

    src_x = r->src.x + op->src.offset[0];
    src_y = r->src.y + op->src.offset[1];
    msk_x = r->mask.x + op->mask.offset[0];
    msk_y = r->mask.y + op->mask.offset[1];
    w = r->width;
    h = r->height;

    v = sna->render.vertices + sna->render.vertex_used;
    sna->render.vertex_used += 15;

    dst.p.x = r->dst.x + r->width;
    dst.p.y = r->dst.y + r->height;
    v[0] = dst.f;
    v[1] = (src_x + w) * op->src.scale[0];
    v[2] = (src_y + h) * op->src.scale[1];
    v[3] = (msk_x + w) * op->mask.scale[0];
    v[4] = (msk_y + h) * op->mask.scale[1];

    dst.p.x = r->dst.x;
    v[5] = dst.f;
    v[6] = src_x * op->src.scale[0];
    v[7] = v[2];
    v[8] = msk_x * op->mask.scale[0];
    v[9] = v[4];

    dst.p.y = r->dst.y;
    v[10] = dst.f;
    v[11] = v[6];
    v[12] = src_y * op->src.scale[1];
    v[13] = v[8];
    v[14] = msk_y * op->mask.scale[1];
}

fastcall static void
gen6_render_composite_box(struct sna *sna,
              const struct sna_composite_op *op,
              const BoxRec *box)
{
    struct sna_composite_rectangles r;

    if (unlikely(!gen6_get_rectangles(sna, op, 1))) {
//        _kgem_submit(&sna->kgem);
//        gen6_emit_composite_state(sna, op);
//        gen6_get_rectangles(sna, op, 1);
    }

    DBG(("  %s: (%d, %d), (%d, %d)\n",
         __FUNCTION__,
         box->x1, box->y1, box->x2, box->y2));

    r.dst.x = box->x1;
    r.dst.y = box->y1;
    r.width  = box->x2 - box->x1;
    r.height = box->y2 - box->y1;
    r.src = r.mask = r.dst;

    op->prim_emit(sna, op, &r);
}


static void gen6_render_composite_done(struct sna *sna,
                       const struct sna_composite_op *op)
{
    DBG(("%s\n", __FUNCTION__));

    if (sna->render_state.gen6.vertex_offset) {
        gen6_vertex_flush(sna);
        gen6_magic_ca_pass(sna, op);
    }

//   if (op->mask.bo)
//       kgem_bo_destroy(&sna->kgem, op->mask.bo);
//   if (op->src.bo)
//       kgem_bo_destroy(&sna->kgem, op->src.bo);

//   sna_render_composite_redirect_done(sna, op);
}


static Bool
gen6_render_composite(struct sna *sna,
              uint8_t op,
              bitmap_t *src,
              struct kgem_bo *src_bo,
              bitmap_t *mask,
              struct kgem_bo *mask_bo,
              bitmap_t *dst,
              struct kgem_bo *dst_bo,
              int16_t src_x, int16_t src_y,
              int16_t msk_x, int16_t msk_y,
              int16_t dst_x, int16_t dst_y,
              int16_t width, int16_t height,
              struct sna_composite_op *tmp)
{
//    if (op >= ARRAY_SIZE(gen6_blend_op))
//        return FALSE;

//    ENTER();

    DBG(("%s: %dx%d, current mode=%d\n", __FUNCTION__,
         width, height, sna->kgem.ring));

    tmp->op = PictOpSrc;

    tmp->dst.bo     = dst_bo;
    tmp->dst.width  = dst->width;
    tmp->dst.height = dst->height;
    tmp->dst.format = GEN6_SURFACEFORMAT_B8G8R8A8_UNORM;


    tmp->src.bo = src_bo;
    tmp->src.card_format = GEN6_SURFACEFORMAT_B8G8R8A8_UNORM;
    tmp->src.width  = src->width;
    tmp->src.height = src->height;

    tmp->src.scale[0] = 1.f/width;            //src->width;
    tmp->src.scale[1] = 1.f/height;            //src->height;
    tmp->src.filter = SAMPLER_FILTER_BILINEAR;
    tmp->src.repeat = SAMPLER_EXTEND_NONE;
    tmp->src.offset[0] = -dst_x;
    tmp->src.offset[1] = -dst_y;
    tmp->src.is_affine = TRUE;


    tmp->mask.bo = mask_bo;
    tmp->mask.card_format = GEN6_SURFACEFORMAT_A8_UNORM;
    tmp->mask.width  = mask->width;
    tmp->mask.height = mask->height;

    tmp->mask.scale[0] = 1.f/mask->width;
    tmp->mask.scale[1] = 1.f/mask->height;
    tmp->mask.filter = SAMPLER_FILTER_NEAREST;
    tmp->mask.repeat = SAMPLER_EXTEND_NONE;
    tmp->mask.offset[0] = -dst_x;
    tmp->mask.offset[1] = -dst_y;
    tmp->mask.is_affine = TRUE;

    tmp->is_affine = TRUE;
    tmp->has_component_alpha = FALSE;
    tmp->need_magic_ca_pass = FALSE;

    tmp->prim_emit = gen6_emit_composite_primitive_identity_source_mask;

    tmp->floats_per_vertex = 5 + 2 * !tmp->is_affine;

    tmp->floats_per_rect = 3 * tmp->floats_per_vertex;

    tmp->u.gen6.wm_kernel = GEN6_WM_KERNEL_MASK;
    tmp->u.gen6.nr_surfaces = 2 + 1;
    tmp->u.gen6.nr_inputs = 1 + 1;
    tmp->u.gen6.ve_id = gen6_choose_composite_vertex_buffer(tmp);

    tmp->need_magic_ca_pass = TRUE;

//    tmp->blt   = gen6_render_composite_blt;
    tmp->box   = gen6_render_composite_box;
//    tmp->boxes = gen6_render_composite_boxes;
    tmp->done  = gen6_render_composite_done;

    gen6_emit_composite_state(sna, tmp);
    gen6_align_vertex(sna, tmp);

//    LEAVE();

    return TRUE;
}


static void
gen6_emit_copy_state(struct sna *sna,
		     const struct sna_composite_op *op)
{
	uint32_t *binding_table;
	uint16_t offset;
	bool dirty;

	gen6_get_batch(sna);

	binding_table = gen6_composite_get_binding_table(sna, &offset);

	binding_table[0] =
		gen6_bind_bo(sna,
			     op->dst.bo, op->dst.width, op->dst.height,
                 GEN6_SURFACEFORMAT_B8G8R8A8_UNORM,
			     TRUE);
	binding_table[1] =
		gen6_bind_bo(sna,
			     op->src.bo, op->src.width, op->src.height,
                 GEN6_SURFACEFORMAT_B8G8R8A8_UNORM,
			     FALSE);

	if (sna->kgem.surface == offset &&
	    *(uint64_t *)(sna->kgem.batch + sna->render_state.gen6.surface_table) == *(uint64_t*)binding_table) {
		sna->kgem.surface += sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);
		offset = sna->render_state.gen6.surface_table;
	}

	gen6_emit_state(sna, op, offset | dirty);
}


static void
gen6_render_copy_blt(struct sna *sna,
             const struct sna_composite_op *op,
		     int16_t sx, int16_t sy,
		     int16_t w,  int16_t h,
		     int16_t dx, int16_t dy)
{
    if (unlikely(!gen6_get_rectangles(sna, op, 1))) {
//       _kgem_submit(&sna->kgem);
        gen6_emit_copy_state(sna, op);
        gen6_get_rectangles(sna, op, 1);
	}

	OUT_VERTEX(dx+w, dy+h);
    OUT_VERTEX_F((sx+w)*op->src.scale[0]);
    OUT_VERTEX_F((sy+h)*op->src.scale[1]);

	OUT_VERTEX(dx, dy+h);
    OUT_VERTEX_F(sx*op->src.scale[0]);
    OUT_VERTEX_F((sy+h)*op->src.scale[1]);

	OUT_VERTEX(dx, dy);
    OUT_VERTEX_F(sx*op->src.scale[0]);
    OUT_VERTEX_F(sy*op->src.scale[1]);
}

static void
gen6_render_copy_done(struct sna *sna)
{
	DBG(("%s()\n", __FUNCTION__));

	if (sna->render_state.gen6.vertex_offset)
		gen6_vertex_flush(sna);
}

static Bool
gen6_render_copy(struct sna *sna, uint8_t alu,
         bitmap_t *src, struct kgem_bo *src_bo,
         bitmap_t *dst, struct kgem_bo *dst_bo,
         int dst_x, int dst_y, int src_x, int src_y, int w, int h)
{
    struct sna_composite_op op;

    memset(&op, 0, sizeof(op));

	DBG(("%s (alu=%d, src=(%dx%d), dst=(%dx%d))\n",
	     __FUNCTION__, alu,
         src->width, src->height,
         dst->width, dst->height));

//    printf("%s %dx%d  src=(%dx%d), dst=(%dx%d)\n",
//         __FUNCTION__,dst_x, dst_y,
//         src->width, src->height,
//         dst->width, dst->height);

    op.dst.format = 0;
    op.src.pict_format = 0;

    op.op = PictOpSrc;

    op.dst.pixmap = dst;
    op.dst.width  = dst->width;
    op.dst.height = dst->height;
    op.dst.bo = dst_bo;

    op.src.bo = src_bo;
    op.src.card_format = GEN6_SURFACEFORMAT_B8G8R8X8_UNORM;
    op.src.width  = src->width;
    op.src.height = src->height;

    op.src.scale[0] = 1.f/w;            //src->width;
    op.src.scale[1] = 1.f/h;            //src->height;
    op.src.filter = SAMPLER_FILTER_BILINEAR;
    op.src.repeat = SAMPLER_EXTEND_NONE;

    op.mask.bo = NULL;

    op.is_affine = true;
    op.floats_per_vertex = 3;
    op.floats_per_rect = 9;

    op.u.gen6.wm_kernel = GEN6_WM_KERNEL_NOMASK;
    op.u.gen6.nr_surfaces = 2;
    op.u.gen6.nr_inputs = 1;
    op.u.gen6.ve_id = 1;

    gen6_emit_copy_state(sna, &op);
    gen6_align_vertex(sna, &op);

    gen6_render_copy_blt(sna, &op, src_x, src_y, w, h, dst_x, dst_y);
    gen6_render_copy_done(sna);

	return TRUE;
}

static void
gen6_emit_fill_state(struct sna *sna, const struct sna_composite_op *op)
{
	uint32_t *binding_table;
	uint16_t offset;
	bool dirty;

	gen6_get_batch(sna);
//   dirty = kgem_bo_is_dirty(op->dst.bo);

	binding_table = gen6_composite_get_binding_table(sna, &offset);

	binding_table[0] =
		gen6_bind_bo(sna,
                 op->dst.bo, 1024, 768,
                 GEN6_SURFACEFORMAT_B8G8R8A8_UNORM,
			     TRUE);
	binding_table[1] =
		gen6_bind_bo(sna,
			     op->src.bo, 1, 1,
			     GEN6_SURFACEFORMAT_B8G8R8A8_UNORM,
			     FALSE);

	if (sna->kgem.surface == offset &&
	    *(uint64_t *)(sna->kgem.batch + sna->render_state.gen6.surface_table) == *(uint64_t*)binding_table) {
		sna->kgem.surface +=
			sizeof(struct gen6_surface_state_padded)/sizeof(uint32_t);
		offset = sna->render_state.gen6.surface_table;
	}

	gen6_emit_state(sna, op, offset | dirty);
}


static Bool
gen6_render_clear(struct sna *sna, bitmap_t *dst, struct kgem_bo *bo)
{
	struct sna_composite_op tmp;


	DBG(("%s: %dx%d\n",
	     __FUNCTION__,
         dst->width,
         dst->height));

	tmp.op = PictOpSrc;

	tmp.dst.pixmap = dst;
    tmp.dst.width  = dst->width;
    tmp.dst.height = dst->height;
    tmp.dst.format = 0; //PICT_a8r8g8b8;
	tmp.dst.bo = bo;
	tmp.dst.x = tmp.dst.y = 0;

//   tmp.src.bo = sna_render_get_solid(sna, 0);
    tmp.src.bo     = bo;
	tmp.src.filter = SAMPLER_FILTER_NEAREST;
	tmp.src.repeat = SAMPLER_EXTEND_REPEAT;

	tmp.mask.bo = NULL;
	tmp.mask.filter = SAMPLER_FILTER_NEAREST;
	tmp.mask.repeat = SAMPLER_EXTEND_NONE;

	tmp.is_affine = TRUE;
	tmp.floats_per_vertex = 3;
	tmp.floats_per_rect = 9;
	tmp.has_component_alpha = 0;
	tmp.need_magic_ca_pass = FALSE;

	tmp.u.gen6.wm_kernel = GEN6_WM_KERNEL_NOMASK;
	tmp.u.gen6.nr_surfaces = 2;
	tmp.u.gen6.nr_inputs = 1;
	tmp.u.gen6.ve_id = 1;

//   if (!kgem_check_bo(&sna->kgem, bo, NULL)) {
//       _kgem_submit(&sna->kgem);
//       assert(kgem_check_bo(&sna->kgem, bo, NULL));
//   }

	gen6_emit_fill_state(sna, &tmp);
	gen6_align_vertex(sna, &tmp);

	if (unlikely(!gen6_get_rectangles(sna, &tmp, 1))) {
//       _kgem_submit(&sna->kgem);
		gen6_emit_fill_state(sna, &tmp);
		gen6_get_rectangles(sna, &tmp, 1);
	}

    OUT_VERTEX(dst->width, dst->height);
	OUT_VERTEX_F(1);
	OUT_VERTEX_F(1);

    OUT_VERTEX(0, dst->height);
	OUT_VERTEX_F(0);
	OUT_VERTEX_F(1);

	OUT_VERTEX(0, 0);
	OUT_VERTEX_F(0);
	OUT_VERTEX_F(0);

    gen6_vertex_flush(sna);
//   kgem_bo_destroy(&sna->kgem, tmp.src.bo);
//    gen6_render_composite_done(sna, &tmp);
//    _kgem_submit(&sna->kgem);

	return TRUE;
}

static void gen6_render_flush(struct sna *sna)
{
	gen6_vertex_close(sna);
}


static void
gen6_render_retire(struct kgem *kgem)
{
	if (kgem->ring && (kgem->has_semaphores || !kgem->need_retire))
		kgem->ring = kgem->mode;
}

static void gen6_render_reset(struct sna *sna)
{
	sna->render_state.gen6.needs_invariant = TRUE;
	sna->render_state.gen6.vb_id = 0;
	sna->render_state.gen6.ve_id = -1;
	sna->render_state.gen6.last_primitive = -1;

	sna->render_state.gen6.num_sf_outputs = 0;
	sna->render_state.gen6.samplers = -1;
	sna->render_state.gen6.blend = -1;
	sna->render_state.gen6.kernel = -1;
	sna->render_state.gen6.drawrect_offset = -1;
	sna->render_state.gen6.drawrect_limit = -1;
	sna->render_state.gen6.surface_table = -1;
}

static void gen6_render_fini(struct sna *sna)
{
//   kgem_bo_destroy(&sna->kgem, sna->render_state.gen6.general_bo);
}

static Bool gen6_render_setup(struct sna *sna)
{
	struct gen6_render_state *state = &sna->render_state.gen6;
	struct sna_static_stream general;
	struct gen6_sampler_state *ss;
	int i, j, k, l, m;

    sna_static_stream_init(&general);

	/* Zero pad the start. If you see an offset of 0x0 in the batchbuffer
	 * dumps, you know it points to zero.
	 */
    null_create(&general);
    scratch_create(&general);

	for (m = 0; m < GEN6_KERNEL_COUNT; m++)
		state->wm_kernel[m] =
			sna_static_stream_add(&general,
					       wm_kernels[m].data,
					       wm_kernels[m].size,
					       64);

	ss = sna_static_stream_map(&general,
				   2 * sizeof(*ss) *
				   FILTER_COUNT * EXTEND_COUNT *
				   FILTER_COUNT * EXTEND_COUNT,
				   32);
	state->wm_state = sna_static_stream_offsetof(&general, ss);
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

    state->cc_vp = gen6_create_cc_viewport(&general);
    state->cc_blend = gen6_composite_create_blend_state(&general);

    state->general_bo = sna_static_stream_fini(sna, &general);
    return state->general_bo != NULL;
}

Bool gen6_render_init(struct sna *sna)
{
    if (!gen6_render_setup(sna))
        return FALSE;

//    sna->kgem.context_switch = gen6_render_context_switch;
      sna->kgem.retire = gen6_render_retire;

    sna->render.composite = gen6_render_composite;
//    sna->render.video = gen6_render_video;

//    sna->render.copy_boxes = gen6_render_copy_boxes;
    sna->render.copy = gen6_render_copy;

//    sna->render.fill_boxes = gen6_render_fill_boxes;
//    sna->render.fill = gen6_render_fill;
//    sna->render.fill_one = gen6_render_fill_one;
    sna->render.clear = gen6_render_clear;

    sna->render.flush = gen6_render_flush;
    sna->render.reset = gen6_render_reset;
//    sna->render.fini = gen6_render_fini;

    sna->render.max_3d_size = GEN6_MAX_SIZE;
    sna->render.max_3d_pitch = 1 << 18;
    return TRUE;
}
