/*
 * Copyright © 2010-2011 Intel Corporation
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
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sna.h"
#include "sna_render.h"
#include "sna_render_inline.h"
#include "sna_reg.h"
//#include "sna_video.h"

#include "gen3_render.h"

#define NO_COMPOSITE 0
#define NO_COMPOSITE_SPANS 0
#define NO_COPY 0
#define NO_COPY_BOXES 0
#define NO_FILL 0
#define NO_FILL_ONE 0
#define NO_FILL_BOXES 0

#define PREFER_BLT_FILL 1

enum {
	SHADER_NONE = 0,
	SHADER_ZERO,
	SHADER_BLACK,
	SHADER_WHITE,
	SHADER_CONSTANT,
	SHADER_LINEAR,
	SHADER_RADIAL,
	SHADER_TEXTURE,
	SHADER_OPACITY,
};

#define MAX_3D_SIZE 2048
#define MAX_3D_PITCH 8192

#define OUT_BATCH(v) batch_emit(sna, v)
#define OUT_BATCH_F(v) batch_emit_float(sna, v)
#define OUT_VERTEX(v) vertex_emit(sna, v)

enum gen3_radial_mode {
	RADIAL_ONE,
	RADIAL_TWO
};

static const struct blendinfo {
	bool dst_alpha;
	bool src_alpha;
	uint32_t src_blend;
	uint32_t dst_blend;
} gen3_blend_op[] = {
	/* Clear */	{0, 0, BLENDFACT_ZERO, BLENDFACT_ZERO},
	/* Src */	{0, 0, BLENDFACT_ONE, BLENDFACT_ZERO},
	/* Dst */	{0, 0, BLENDFACT_ZERO, BLENDFACT_ONE},
	/* Over */	{0, 1, BLENDFACT_ONE, BLENDFACT_INV_SRC_ALPHA},
	/* OverReverse */ {1, 0, BLENDFACT_INV_DST_ALPHA, BLENDFACT_ONE},
	/* In */	{1, 0, BLENDFACT_DST_ALPHA, BLENDFACT_ZERO},
	/* InReverse */ {0, 1, BLENDFACT_ZERO, BLENDFACT_SRC_ALPHA},
	/* Out */	{1, 0, BLENDFACT_INV_DST_ALPHA, BLENDFACT_ZERO},
	/* OutReverse */ {0, 1, BLENDFACT_ZERO, BLENDFACT_INV_SRC_ALPHA},
	/* Atop */	{1, 1, BLENDFACT_DST_ALPHA, BLENDFACT_INV_SRC_ALPHA},
	/* AtopReverse */ {1, 1, BLENDFACT_INV_DST_ALPHA, BLENDFACT_SRC_ALPHA},
	/* Xor */	{1, 1, BLENDFACT_INV_DST_ALPHA, BLENDFACT_INV_SRC_ALPHA},
	/* Add */	{0, 0, BLENDFACT_ONE, BLENDFACT_ONE},
};

#define S6_COLOR_WRITE_ONLY \
	(S6_COLOR_WRITE_ENABLE | \
	 BLENDFUNC_ADD << S6_CBUF_BLEND_FUNC_SHIFT | \
	 BLENDFACT_ONE << S6_CBUF_SRC_BLEND_FACT_SHIFT | \
	 BLENDFACT_ZERO << S6_CBUF_DST_BLEND_FACT_SHIFT)

static const struct formatinfo {
	unsigned int fmt, xfmt;
	uint32_t card_fmt;
	bool rb_reversed;
} gen3_tex_formats[] = {
	{PICT_a8, 0, MAPSURF_8BIT | MT_8BIT_A8, false},
	{PICT_a8r8g8b8, 0, MAPSURF_32BIT | MT_32BIT_ARGB8888, false},
	{PICT_x8r8g8b8, 0, MAPSURF_32BIT | MT_32BIT_XRGB8888, false},
	{PICT_a8b8g8r8, 0, MAPSURF_32BIT | MT_32BIT_ABGR8888, false},
	{PICT_x8b8g8r8, 0, MAPSURF_32BIT | MT_32BIT_XBGR8888, false},
	{PICT_a2r10g10b10, PICT_x2r10g10b10, MAPSURF_32BIT | MT_32BIT_ARGB2101010, false},
	{PICT_a2b10g10r10, PICT_x2b10g10r10, MAPSURF_32BIT | MT_32BIT_ABGR2101010, false},
	{PICT_r5g6b5, 0, MAPSURF_16BIT | MT_16BIT_RGB565, false},
	{PICT_b5g6r5, 0, MAPSURF_16BIT | MT_16BIT_RGB565, true},
	{PICT_a1r5g5b5, PICT_x1r5g5b5, MAPSURF_16BIT | MT_16BIT_ARGB1555, false},
	{PICT_a1b5g5r5, PICT_x1b5g5r5, MAPSURF_16BIT | MT_16BIT_ARGB1555, true},
	{PICT_a4r4g4b4, PICT_x4r4g4b4, MAPSURF_16BIT | MT_16BIT_ARGB4444, false},
	{PICT_a4b4g4r4, PICT_x4b4g4r4, MAPSURF_16BIT | MT_16BIT_ARGB4444, true},
};

#define xFixedToDouble(f) pixman_fixed_to_double(f)

static inline bool too_large(int width, int height)
{
	return width > MAX_3D_SIZE || height > MAX_3D_SIZE;
}

static inline uint32_t gen3_buf_tiling(uint32_t tiling)
{
	uint32_t v = 0;
	switch (tiling) {
	case I915_TILING_Y: v |= BUF_3D_TILE_WALK_Y;
	case I915_TILING_X: v |= BUF_3D_TILED_SURFACE;
	case I915_TILING_NONE: break;
	}
	return v;
}
static uint32_t gen3_get_blend_cntl(int op,
				    bool has_component_alpha,
				    uint32_t dst_format)
{
	uint32_t sblend;
	uint32_t dblend;

    sblend = BLENDFACT_ONE;
    dblend = BLENDFACT_INV_SRC_ALPHA;

#if 0
	if (op <= PictOpSrc) /* for clear and src disable blending */
		return S6_COLOR_WRITE_ONLY;

	/* If there's no dst alpha channel, adjust the blend op so that we'll
	 * treat it as always 1.
	 */
	if (gen3_blend_op[op].dst_alpha) {
		if (PICT_FORMAT_A(dst_format) == 0) {
			if (sblend == BLENDFACT_DST_ALPHA)
				sblend = BLENDFACT_ONE;
			else if (sblend == BLENDFACT_INV_DST_ALPHA)
				sblend = BLENDFACT_ZERO;
		}

		/* gen3 engine reads 8bit color buffer into green channel
		 * in cases like color buffer blending etc., and also writes
		 * back green channel.  So with dst_alpha blend we should use
		 * color factor. See spec on "8-bit rendering".
		 */
		if (dst_format == PICT_a8) {
			if (sblend == BLENDFACT_DST_ALPHA)
				sblend = BLENDFACT_DST_COLR;
			else if (sblend == BLENDFACT_INV_DST_ALPHA)
				sblend = BLENDFACT_INV_DST_COLR;
		}
	}

	/* If the source alpha is being used, then we should only be in a case
	 * where the source blend factor is 0, and the source blend value is the
	 * mask channels multiplied by the source picture's alpha.
	 */
	if (has_component_alpha && gen3_blend_op[op].src_alpha) {
		if (dblend == BLENDFACT_SRC_ALPHA)
			dblend = BLENDFACT_SRC_COLR;
		else if (dblend == BLENDFACT_INV_SRC_ALPHA)
			dblend = BLENDFACT_INV_SRC_COLR;
	}
#endif

	return (S6_CBUF_BLEND_ENABLE | S6_COLOR_WRITE_ENABLE |
		BLENDFUNC_ADD << S6_CBUF_BLEND_FUNC_SHIFT |
		sblend << S6_CBUF_SRC_BLEND_FACT_SHIFT |
		dblend << S6_CBUF_DST_BLEND_FACT_SHIFT);
}
static bool gen3_dst_rb_reversed(uint32_t format)
{
	switch (format) {
	case PICT_a8r8g8b8:
	case PICT_x8r8g8b8:
	case PICT_r5g6b5:
	case PICT_a1r5g5b5:
	case PICT_x1r5g5b5:
	case PICT_a2r10g10b10:
	case PICT_x2r10g10b10:
	case PICT_a8:
	case PICT_a4r4g4b4:
	case PICT_x4r4g4b4:
		return false;
	default:
		return true;
	}
}

#define DSTORG_HORT_BIAS(x)             ((x)<<20)
#define DSTORG_VERT_BIAS(x)             ((x)<<16)

static uint32_t gen3_get_dst_format(uint32_t format)
{
#define BIAS (DSTORG_HORT_BIAS(0x8) | DSTORG_VERT_BIAS(0x8))
	switch (format) {
	default:
	case PICT_a8r8g8b8:
	case PICT_x8r8g8b8:
	case PICT_a8b8g8r8:
	case PICT_x8b8g8r8:
		return BIAS | COLR_BUF_ARGB8888;
	case PICT_r5g6b5:
	case PICT_b5g6r5:
		return BIAS | COLR_BUF_RGB565;
	case PICT_a1r5g5b5:
	case PICT_x1r5g5b5:
	case PICT_a1b5g5r5:
	case PICT_x1b5g5r5:
		return BIAS | COLR_BUF_ARGB1555;
	case PICT_a2r10g10b10:
	case PICT_x2r10g10b10:
	case PICT_a2b10g10r10:
	case PICT_x2b10g10r10:
		return BIAS | COLR_BUF_ARGB2AAA;
	case PICT_a8:
		return BIAS | COLR_BUF_8BIT;
	case PICT_a4r4g4b4:
	case PICT_x4r4g4b4:
	case PICT_a4b4g4r4:
	case PICT_x4b4g4r4:
		return BIAS | COLR_BUF_ARGB4444;
	}
#undef BIAS
}


#if 0
static bool gen3_check_repeat(PicturePtr p)
{
	if (!p->repeat)
		return true;

	switch (p->repeatType) {
	case RepeatNone:
	case RepeatNormal:
	case RepeatPad:
	case RepeatReflect:
		return true;
	default:
		return false;
	}
}

static uint32_t gen3_filter(uint32_t filter)
{
	switch (filter) {
	default:
		assert(0);
	case PictFilterNearest:
		return (FILTER_NEAREST << SS2_MAG_FILTER_SHIFT |
			FILTER_NEAREST << SS2_MIN_FILTER_SHIFT |
			MIPFILTER_NONE << SS2_MIP_FILTER_SHIFT);
	case PictFilterBilinear:
		return (FILTER_LINEAR  << SS2_MAG_FILTER_SHIFT |
			FILTER_LINEAR  << SS2_MIN_FILTER_SHIFT |
			MIPFILTER_NONE << SS2_MIP_FILTER_SHIFT);
	}
}

static bool gen3_check_filter(PicturePtr p)
{
	switch (p->filter) {
	case PictFilterNearest:
	case PictFilterBilinear:
		return true;
	default:
		return false;
	}
}
fastcall static void
gen3_emit_composite_primitive_identity_gradient(struct sna *sna,
						const struct sna_composite_op *op,
						const struct sna_composite_rectangles *r)
{
	int16_t dst_x, dst_y;
	int16_t src_x, src_y;

	dst_x = r->dst.x + op->dst.x;
	dst_y = r->dst.y + op->dst.y;
	src_x = r->src.x + op->src.offset[0];
	src_y = r->src.y + op->src.offset[1];

	gen3_emit_composite_dstcoord(sna, dst_x + r->width, dst_y + r->height);
	OUT_VERTEX(src_x + r->width);
	OUT_VERTEX(src_y + r->height);

	gen3_emit_composite_dstcoord(sna, dst_x, dst_y + r->height);
	OUT_VERTEX(src_x);
	OUT_VERTEX(src_y + r->height);

	gen3_emit_composite_dstcoord(sna, dst_x, dst_y);
	OUT_VERTEX(src_x);
	OUT_VERTEX(src_y);
}

fastcall static void
gen3_emit_composite_boxes_identity_gradient(const struct sna_composite_op *op,
					    const BoxRec *box, int nbox,
					    float *v)
{
	do {
		v[0] = box->x2;
		v[1] = box->y2;
		v[2] = box->x2 + op->src.offset[0];
		v[3] = box->y2 + op->src.offset[1];

		v[4] = box->x1;
		v[5] = box->y2;
		v[6] = box->x1 + op->src.offset[0];
		v[7] = box->y2 + op->src.offset[1];

		v[8] = box->x1;
		v[9] = box->y1;
		v[10] = box->x1 + op->src.offset[0];
		v[11] = box->y1 + op->src.offset[1];

		v += 12;
		box++;
	} while (--nbox);
}
fastcall static void
gen3_emit_composite_boxes_affine_gradient(const struct sna_composite_op *op,
					  const BoxRec *box, int nbox,
					  float *v)
{
	const PictTransform *transform = op->src.transform;

	do {
		v[0] = box->x2;
		v[1] = box->y2;
		_sna_get_transformed_scaled(box->x2 + op->src.offset[0],
					    box->y2 + op->src.offset[1],
					    transform, op->src.scale,
					    &v[2], &v[3]);

		v[4] = box->x1;
		v[5] = box->y2;
		_sna_get_transformed_scaled(box->x1 + op->src.offset[0],
					    box->y2 + op->src.offset[1],
					    transform, op->src.scale,
					    &v[6], &v[7]);

		v[8] = box->x1;
		v[9] = box->y1;
		_sna_get_transformed_scaled(box->x1 + op->src.offset[0],
					    box->y1 + op->src.offset[1],
					    transform, op->src.scale,
					    &v[10], &v[11]);

		box++;
		v += 12;
	} while (--nbox);
}

fastcall static void
gen3_emit_composite_primitive_identity_source(struct sna *sna,
					      const struct sna_composite_op *op,
					      const struct sna_composite_rectangles *r)
{
	float w = r->width;
	float h = r->height;
	float *v;

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 12;

	v[8] = v[4] = r->dst.x + op->dst.x;
	v[0] = v[4] + w;

	v[9] = r->dst.y + op->dst.y;
	v[5] = v[1] = v[9] + h;

	v[10] = v[6] = (r->src.x + op->src.offset[0]) * op->src.scale[0];
	v[2] = v[6] + w * op->src.scale[0];

	v[11] = (r->src.y + op->src.offset[1]) * op->src.scale[1];
	v[7] = v[3] = v[11] + h * op->src.scale[1];
}

fastcall static void
gen3_emit_composite_boxes_identity_source(const struct sna_composite_op *op,
					  const BoxRec *box, int nbox,
					  float *v)
{
	do {
		v[0] = box->x2 + op->dst.x;
		v[8] = v[4] = box->x1 + op->dst.x;
		v[5] = v[1] = box->y2 + op->dst.y;
		v[9] = box->y1 + op->dst.y;

		v[10] = v[6] = (box->x1 + op->src.offset[0]) * op->src.scale[0];
		v[2] = (box->x2 + op->src.offset[0]) * op->src.scale[0];

		v[11] = (box->y1 + op->src.offset[1]) * op->src.scale[1];
		v[7] = v[3] = (box->y2 + op->src.offset[1]) * op->src.scale[1];

		v += 12;
		box++;
	} while (--nbox);
}

fastcall static void
gen3_emit_composite_primitive_identity_source_no_offset(struct sna *sna,
							const struct sna_composite_op *op,
							const struct sna_composite_rectangles *r)
{
	float w = r->width;
	float h = r->height;
	float *v;

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 12;

	v[8] = v[4] = r->dst.x;
	v[9] = r->dst.y;

	v[0] = v[4] + w;
	v[5] = v[1] = v[9] + h;

	v[10] = v[6] = r->src.x * op->src.scale[0];
	v[11] = r->src.y * op->src.scale[1];

	v[2] = v[6] + w * op->src.scale[0];
	v[7] = v[3] = v[11] + h * op->src.scale[1];
}
fastcall static void
gen3_emit_composite_primitive_constant_identity_mask(struct sna *sna,
						     const struct sna_composite_op *op,
						     const struct sna_composite_rectangles *r)
{
	float w = r->width;
	float h = r->height;
	float *v;

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 12;

	v[8] = v[4] = r->dst.x + op->dst.x;
	v[0] = v[4] + w;

	v[9] = r->dst.y + op->dst.y;
	v[5] = v[1] = v[9] + h;

	v[10] = v[6] = (r->mask.x + op->mask.offset[0]) * op->mask.scale[0];
	v[2] = v[6] + w * op->mask.scale[0];

	v[11] = (r->mask.y + op->mask.offset[1]) * op->mask.scale[1];
	v[7] = v[3] = v[11] + h * op->mask.scale[1];
}
#endif

fastcall static void
gen3_emit_composite_primitive_identity_source_mask(struct sna *sna,
						   const struct sna_composite_op *op,
						   const struct sna_composite_rectangles *r)
{
	float dst_x, dst_y;
	float src_x, src_y;
	float msk_x, msk_y;
	float w, h;
	float *v;

	dst_x = r->dst.x + op->dst.x;
	dst_y = r->dst.y + op->dst.y;
	src_x = r->src.x + op->src.offset[0];
	src_y = r->src.y + op->src.offset[1];
	msk_x = r->mask.x + op->mask.offset[0];
	msk_y = r->mask.y + op->mask.offset[1];
	w = r->width;
	h = r->height;

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 18;

	v[0] = dst_x + w;
	v[1] = dst_y + h;
	v[2] = (src_x + w) * op->src.scale[0];
	v[3] = (src_y + h) * op->src.scale[1];
	v[4] = (msk_x + w) * op->mask.scale[0];
	v[5] = (msk_y + h) * op->mask.scale[1];

	v[6] = dst_x;
	v[7] = v[1];
	v[8] = src_x * op->src.scale[0];
	v[9] = v[3];
	v[10] = msk_x * op->mask.scale[0];
	v[11] =v[5];

	v[12] = v[6];
	v[13] = dst_y;
	v[14] = v[8];
	v[15] = src_y * op->src.scale[1];
	v[16] = v[10];
	v[17] = msk_y * op->mask.scale[1];
}































































static inline void
gen3_2d_perspective(struct sna *sna, int in, int out)
{
	gen3_fs_rcp(out, 0, gen3_fs_operand(in, W, W, W, W));
	gen3_fs_mul(out,
		    gen3_fs_operand(in, X, Y, ZERO, ONE),
		    gen3_fs_operand_reg(out));
}

static inline void
gen3_linear_coord(struct sna *sna,
		  const struct sna_composite_channel *channel,
		  int in, int out)
{
	int c = channel->u.gen3.constants;

	if (!channel->is_affine) {
		gen3_2d_perspective(sna, in, FS_U0);
		in = FS_U0;
	}

	gen3_fs_mov(out, gen3_fs_operand_zero());
	gen3_fs_dp3(out, MASK_X,
		    gen3_fs_operand(in, X, Y, ONE, ZERO),
		    gen3_fs_operand_reg(c));
}

static void
gen3_radial_coord(struct sna *sna,
		  const struct sna_composite_channel *channel,
		  int in, int out)
{
	int c = channel->u.gen3.constants;

	if (!channel->is_affine) {
		gen3_2d_perspective(sna, in, FS_U0);
		in = FS_U0;
	}

	switch (channel->u.gen3.mode) {
	case RADIAL_ONE:
		/*
		   pdx = (x - c1x) / dr, pdy = (y - c1y) / dr;
		   r? = pdx*pdx + pdy*pdy
		   t = r?/sqrt(r?) - r1/dr;
		   */
		gen3_fs_mad(FS_U0, MASK_X | MASK_Y,
			    gen3_fs_operand(in, X, Y, ZERO, ZERO),
			    gen3_fs_operand(c, Z, Z, ZERO, ZERO),
			    gen3_fs_operand(c, NEG_X, NEG_Y, ZERO, ZERO));
		gen3_fs_dp2add(FS_U0, MASK_X,
			       gen3_fs_operand(FS_U0, X, Y, ZERO, ZERO),
			       gen3_fs_operand(FS_U0, X, Y, ZERO, ZERO),
			       gen3_fs_operand_zero());
		gen3_fs_rsq(out, MASK_X, gen3_fs_operand(FS_U0, X, X, X, X));
		gen3_fs_mad(out, 0,
			    gen3_fs_operand(FS_U0, X, ZERO, ZERO, ZERO),
			    gen3_fs_operand(out, X, ZERO, ZERO, ZERO),
			    gen3_fs_operand(c, W, ZERO, ZERO, ZERO));
		break;

	case RADIAL_TWO:
		/*
		   pdx = x - c1x, pdy = y - c1y;
		   A = dx? + dy? - dr?
		   B = -2*(pdx*dx + pdy*dy + r1*dr);
		   C = pdx? + pdy? - r1?;
		   det = B*B - 4*A*C;
		   t = (-B + sqrt (det)) / (2 * A)
		   */

		/* u0.x = pdx, u0.y = pdy, u[0].z = r1; */
		gen3_fs_add(FS_U0,
			    gen3_fs_operand(in, X, Y, ZERO, ZERO),
			    gen3_fs_operand(c, X, Y, Z, ZERO));
		/* u0.x = pdx, u0.y = pdy, u[0].z = r1, u[0].w = B; */
		gen3_fs_dp3(FS_U0, MASK_W,
			    gen3_fs_operand(FS_U0, X, Y, ONE, ZERO),
			    gen3_fs_operand(c+1, X, Y, Z, ZERO));
		/* u1.x = pdx? + pdy? - r1?; [C] */
		gen3_fs_dp3(FS_U1, MASK_X,
			    gen3_fs_operand(FS_U0, X, Y, Z, ZERO),
			    gen3_fs_operand(FS_U0, X, Y, NEG_Z, ZERO));
		/* u1.x = C, u1.y = B, u1.z=-4*A; */
		gen3_fs_mov_masked(FS_U1, MASK_Y, gen3_fs_operand(FS_U0, W, W, W, W));
		gen3_fs_mov_masked(FS_U1, MASK_Z, gen3_fs_operand(c, W, W, W, W));
		/* u1.x = B? - 4*A*C */
		gen3_fs_dp2add(FS_U1, MASK_X,
			       gen3_fs_operand(FS_U1, X, Y, ZERO, ZERO),
			       gen3_fs_operand(FS_U1, Z, Y, ZERO, ZERO),
			       gen3_fs_operand_zero());
		/* out.x = -B + sqrt (B? - 4*A*C), */
		gen3_fs_rsq(out, MASK_X, gen3_fs_operand(FS_U1, X, X, X, X));
		gen3_fs_mad(out, MASK_X,
			    gen3_fs_operand(out, X, ZERO, ZERO, ZERO),
			    gen3_fs_operand(FS_U1, X, ZERO, ZERO, ZERO),
			    gen3_fs_operand(FS_U0, NEG_W, ZERO, ZERO, ZERO));
		/* out.x = (-B + sqrt (B? - 4*A*C)) / (2 * A), */
		gen3_fs_mul(out,
			    gen3_fs_operand(out, X, ZERO, ZERO, ZERO),
			    gen3_fs_operand(c+1, W, ZERO, ZERO, ZERO));
		break;
	}
}

static void
gen3_composite_emit_shader(struct sna *sna,
			   const struct sna_composite_op *op,
			   uint8_t blend)
{
	bool dst_is_alpha = PIXMAN_FORMAT_RGB(op->dst.format) == 0;
	const struct sna_composite_channel *src, *mask;
	struct gen3_render_state *state = &sna->render_state.gen3;
	uint32_t shader_offset, id;
	int src_reg, mask_reg;
	int t, length;

	src = &op->src;
	mask = &op->mask;
	if (mask->u.gen3.type == SHADER_NONE)
		mask = NULL;

	id = (src->u.gen3.type |
	      src->is_affine << 4 |
	      src->alpha_fixup << 5 |
	      src->rb_reversed << 6);
	if (mask) {
		id |= (mask->u.gen3.type << 8 |
		       mask->is_affine << 12 |
		       gen3_blend_op[blend].src_alpha << 13 |
		       op->has_component_alpha << 14 |
		       mask->alpha_fixup << 15 |
		       mask->rb_reversed << 16);
	}
	id |= dst_is_alpha << 24;
	id |= op->rb_reversed << 25;

	if (id == state->last_shader)
		return;

	state->last_shader = id;

	shader_offset = sna->kgem.nbatch++;
	t = 0;
	switch (src->u.gen3.type) {
	case SHADER_NONE:
	case SHADER_OPACITY:
		assert(0);
	case SHADER_ZERO:
	case SHADER_BLACK:
	case SHADER_WHITE:
		break;
	case SHADER_CONSTANT:
		gen3_fs_dcl(FS_T8);
		src_reg = FS_T8;
		break;
	case SHADER_TEXTURE:
	case SHADER_RADIAL:
	case SHADER_LINEAR:
		gen3_fs_dcl(FS_S0);
		gen3_fs_dcl(FS_T0);
		t++;
		break;
	}

	if (mask == NULL) {
		switch (src->u.gen3.type) {
		case SHADER_ZERO:
			gen3_fs_mov(FS_OC, gen3_fs_operand_zero());
			goto done;
		case SHADER_BLACK:
			if (dst_is_alpha)
				gen3_fs_mov(FS_OC, gen3_fs_operand_one());
			else
				gen3_fs_mov(FS_OC, gen3_fs_operand(FS_R0, ZERO, ZERO, ZERO, ONE));
			goto done;
		case SHADER_WHITE:
			gen3_fs_mov(FS_OC, gen3_fs_operand_one());
			goto done;
		}
		if (src->alpha_fixup && dst_is_alpha) {
			gen3_fs_mov(FS_OC, gen3_fs_operand_one());
			goto done;
		}
		/* No mask, so load directly to output color */
		if (src->u.gen3.type != SHADER_CONSTANT) {
			if (dst_is_alpha || src->rb_reversed ^ op->rb_reversed)
				src_reg = FS_R0;
			else
				src_reg = FS_OC;
		}
		switch (src->u.gen3.type) {
		case SHADER_LINEAR:
			gen3_linear_coord(sna, src, FS_T0, FS_R0);
			gen3_fs_texld(src_reg, FS_S0, FS_R0);
			break;

		case SHADER_RADIAL:
			gen3_radial_coord(sna, src, FS_T0, FS_R0);
			gen3_fs_texld(src_reg, FS_S0, FS_R0);
			break;

		case SHADER_TEXTURE:
			if (src->is_affine)
				gen3_fs_texld(src_reg, FS_S0, FS_T0);
			else
				gen3_fs_texldp(src_reg, FS_S0, FS_T0);
			break;

		case SHADER_NONE:
		case SHADER_WHITE:
		case SHADER_BLACK:
		case SHADER_ZERO:
			assert(0);
		case SHADER_CONSTANT:
			break;
		}

		if (src_reg != FS_OC) {
			if (src->alpha_fixup)
				gen3_fs_mov(FS_OC,
					    src->rb_reversed ^ op->rb_reversed ?
					    gen3_fs_operand(src_reg, Z, Y, X, ONE) :
					    gen3_fs_operand(src_reg, X, Y, Z, ONE));
			else if (dst_is_alpha)
				gen3_fs_mov(FS_OC, gen3_fs_operand(src_reg, W, W, W, W));
			else if (src->rb_reversed ^ op->rb_reversed)
				gen3_fs_mov(FS_OC, gen3_fs_operand(src_reg, Z, Y, X, W));
			else
				gen3_fs_mov(FS_OC, gen3_fs_operand_reg(src_reg));
		} else if (src->alpha_fixup)
			gen3_fs_mov_masked(FS_OC, MASK_W, gen3_fs_operand_one());
	} else {
		int out_reg = FS_OC;
		if (op->rb_reversed)
			out_reg = FS_U0;

		switch (mask->u.gen3.type) {
		case SHADER_CONSTANT:
			gen3_fs_dcl(FS_T9);
			mask_reg = FS_T9;
			break;
		case SHADER_TEXTURE:
		case SHADER_LINEAR:
		case SHADER_RADIAL:
			gen3_fs_dcl(FS_S0 + t);
			/* fall through */
		case SHADER_OPACITY:
			gen3_fs_dcl(FS_T0 + t);
			break;
		case SHADER_ZERO:
		case SHADER_BLACK:
			assert(0);
		case SHADER_NONE:
		case SHADER_WHITE:
			break;
		}

		t = 0;
		switch (src->u.gen3.type) {
		case SHADER_LINEAR:
			gen3_linear_coord(sna, src, FS_T0, FS_R0);
			gen3_fs_texld(FS_R0, FS_S0, FS_R0);
			src_reg = FS_R0;
			t++;
			break;

		case SHADER_RADIAL:
			gen3_radial_coord(sna, src, FS_T0, FS_R0);
			gen3_fs_texld(FS_R0, FS_S0, FS_R0);
			src_reg = FS_R0;
			t++;
			break;

		case SHADER_TEXTURE:
			if (src->is_affine)
				gen3_fs_texld(FS_R0, FS_S0, FS_T0);
			else
				gen3_fs_texldp(FS_R0, FS_S0, FS_T0);
			src_reg = FS_R0;
			t++;
			break;

		case SHADER_CONSTANT:
		case SHADER_NONE:
		case SHADER_ZERO:
		case SHADER_BLACK:
		case SHADER_WHITE:
			break;
		}
		if (src->alpha_fixup)
			gen3_fs_mov_masked(src_reg, MASK_W, gen3_fs_operand_one());
		if (src->rb_reversed)
			gen3_fs_mov(src_reg, gen3_fs_operand(src_reg, Z, Y, X, W));

		switch (mask->u.gen3.type) {
		case SHADER_LINEAR:
			gen3_linear_coord(sna, mask, FS_T0 + t, FS_R1);
			gen3_fs_texld(FS_R1, FS_S0 + t, FS_R1);
			mask_reg = FS_R1;
			break;

		case SHADER_RADIAL:
			gen3_radial_coord(sna, mask, FS_T0 + t, FS_R1);
			gen3_fs_texld(FS_R1, FS_S0 + t, FS_R1);
			mask_reg = FS_R1;
			break;

		case SHADER_TEXTURE:
			if (mask->is_affine)
				gen3_fs_texld(FS_R1, FS_S0 + t, FS_T0 + t);
			else
				gen3_fs_texldp(FS_R1, FS_S0 + t, FS_T0 + t);
			mask_reg = FS_R1;
			break;

		case SHADER_OPACITY:
			switch (src->u.gen3.type) {
			case SHADER_BLACK:
			case SHADER_WHITE:
				if (dst_is_alpha || src->u.gen3.type == SHADER_WHITE) {
					gen3_fs_mov(out_reg,
						    gen3_fs_operand(FS_T0 + t, X, X, X, X));
				} else {
					gen3_fs_mov(out_reg,
						    gen3_fs_operand(FS_T0 + t, ZERO, ZERO, ZERO, X));
				}
				break;
			default:
				if (dst_is_alpha) {
					gen3_fs_mul(out_reg,
						    gen3_fs_operand(src_reg, W, W, W, W),
						    gen3_fs_operand(FS_T0 + t, X, X, X, X));
				} else {
					gen3_fs_mul(out_reg,
						    gen3_fs_operand(src_reg, X, Y, Z, W),
						    gen3_fs_operand(FS_T0 + t, X, X, X, X));
				}
			}
			goto mask_done;

		case SHADER_CONSTANT:
		case SHADER_ZERO:
		case SHADER_BLACK:
		case SHADER_WHITE:
		case SHADER_NONE:
			break;
		}
		if (mask->alpha_fixup)
			gen3_fs_mov_masked(mask_reg, MASK_W, gen3_fs_operand_one());
		if (mask->rb_reversed)
			gen3_fs_mov(mask_reg, gen3_fs_operand(mask_reg, Z, Y, X, W));

		if (dst_is_alpha) {
			switch (src->u.gen3.type) {
			case SHADER_BLACK:
			case SHADER_WHITE:
				gen3_fs_mov(out_reg,
					    gen3_fs_operand(mask_reg, W, W, W, W));
				break;
			default:
				gen3_fs_mul(out_reg,
					    gen3_fs_operand(src_reg, W, W, W, W),
					    gen3_fs_operand(mask_reg, W, W, W, W));
				break;
			}
		} else {
			/* If component alpha is active in the mask and the blend
			 * operation uses the source alpha, then we know we don't
			 * need the source value (otherwise we would have hit a
			 * fallback earlier), so we provide the source alpha (src.A *
			 * mask.X) as output color.
			 * Conversely, if CA is set and we don't need the source alpha,
			 * then we produce the source value (src.X * mask.X) and the
			 * source alpha is unused.  Otherwise, we provide the non-CA
			 * source value (src.X * mask.A).
			 */
			if (op->has_component_alpha) {
				switch (src->u.gen3.type) {
				case SHADER_BLACK:
					if (gen3_blend_op[blend].src_alpha)
						gen3_fs_mov(out_reg,
							    gen3_fs_operand_reg(mask_reg));
					else
						gen3_fs_mov(out_reg,
							    gen3_fs_operand(mask_reg, ZERO, ZERO, ZERO, W));
					break;
				case SHADER_WHITE:
					gen3_fs_mov(out_reg,
						    gen3_fs_operand_reg(mask_reg));
					break;
				default:
					if (gen3_blend_op[blend].src_alpha)
						gen3_fs_mul(out_reg,
							    gen3_fs_operand(src_reg, W, W, W, W),
							    gen3_fs_operand_reg(mask_reg));
					else
						gen3_fs_mul(out_reg,
							    gen3_fs_operand_reg(src_reg),
							    gen3_fs_operand_reg(mask_reg));
					break;
				}
			} else {
				switch (src->u.gen3.type) {
				case SHADER_WHITE:
					gen3_fs_mov(out_reg,
						    gen3_fs_operand(mask_reg, W, W, W, W));
					break;
				case SHADER_BLACK:
					gen3_fs_mov(out_reg,
						    gen3_fs_operand(mask_reg, ZERO, ZERO, ZERO, W));
					break;
				default:
					gen3_fs_mul(out_reg,
						    gen3_fs_operand_reg(src_reg),
						    gen3_fs_operand(mask_reg, W, W, W, W));
					break;
				}
			}
		}
mask_done:
		if (op->rb_reversed)
			gen3_fs_mov(FS_OC, gen3_fs_operand(FS_U0, Z, Y, X, W));
	}

done:
	length = sna->kgem.nbatch - shader_offset;
	sna->kgem.batch[shader_offset] =
		_3DSTATE_PIXEL_SHADER_PROGRAM | (length - 2);
}

static uint32_t gen3_ms_tiling(uint32_t tiling)
{
	uint32_t v = 0;
	switch (tiling) {
	case I915_TILING_Y: v |= MS3_TILE_WALK;
	case I915_TILING_X: v |= MS3_TILED_SURFACE;
	case I915_TILING_NONE: break;
	}
	return v;
}

static void gen3_emit_invariant(struct sna *sna)
{
	/* Disable independent alpha blend */
	OUT_BATCH(_3DSTATE_INDEPENDENT_ALPHA_BLEND_CMD | IAB_MODIFY_ENABLE |
		  IAB_MODIFY_FUNC | BLENDFUNC_ADD << IAB_FUNC_SHIFT |
		  IAB_MODIFY_SRC_FACTOR | BLENDFACT_ONE << IAB_SRC_FACTOR_SHIFT |
		  IAB_MODIFY_DST_FACTOR | BLENDFACT_ZERO << IAB_DST_FACTOR_SHIFT);

	OUT_BATCH(_3DSTATE_COORD_SET_BINDINGS |
		  CSB_TCB(0, 0) |
		  CSB_TCB(1, 1) |
		  CSB_TCB(2, 2) |
		  CSB_TCB(3, 3) |
		  CSB_TCB(4, 4) |
		  CSB_TCB(5, 5) |
		  CSB_TCB(6, 6) |
		  CSB_TCB(7, 7));

	OUT_BATCH(_3DSTATE_LOAD_STATE_IMMEDIATE_1 | I1_LOAD_S(3) | I1_LOAD_S(4) | I1_LOAD_S(5) | I1_LOAD_S(6) | 3);
	OUT_BATCH(0); /* Disable texture coordinate wrap-shortest */
	OUT_BATCH((1 << S4_POINT_WIDTH_SHIFT) |
		  S4_LINE_WIDTH_ONE |
		  S4_CULLMODE_NONE |
		  S4_VFMT_XY);
	OUT_BATCH(0); /* Disable fog/stencil. *Enable* write mask. */
	OUT_BATCH(S6_COLOR_WRITE_ONLY); /* Disable blending, depth */

	OUT_BATCH(_3DSTATE_SCISSOR_ENABLE_CMD | DISABLE_SCISSOR_RECT);
	OUT_BATCH(_3DSTATE_DEPTH_SUBRECT_DISABLE);

	OUT_BATCH(_3DSTATE_LOAD_INDIRECT);
	OUT_BATCH(0x00000000);

	OUT_BATCH(_3DSTATE_STIPPLE);
	OUT_BATCH(0x00000000);

	sna->render_state.gen3.need_invariant = false;
}

#define MAX_OBJECTS 3 /* worst case: dst + src + mask  */

static void
gen3_get_batch(struct sna *sna, const struct sna_composite_op *op)
{
	kgem_set_mode(&sna->kgem, KGEM_RENDER, op->dst.bo);

	if (!kgem_check_batch(&sna->kgem, 200)) {
		DBG(("%s: flushing batch: size %d > %d\n",
		     __FUNCTION__, 200,
		     sna->kgem.surface-sna->kgem.nbatch));
		kgem_submit(&sna->kgem);
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	if (!kgem_check_reloc(&sna->kgem, MAX_OBJECTS)) {
		DBG(("%s: flushing batch: reloc %d >= %d\n",
		     __FUNCTION__,
		     sna->kgem.nreloc,
		     (int)KGEM_RELOC_SIZE(&sna->kgem) - MAX_OBJECTS));
		kgem_submit(&sna->kgem);
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	if (!kgem_check_exec(&sna->kgem, MAX_OBJECTS)) {
		DBG(("%s: flushing batch: exec %d >= %d\n",
		     __FUNCTION__,
		     sna->kgem.nexec,
		     (int)KGEM_EXEC_SIZE(&sna->kgem) - MAX_OBJECTS - 1));
		kgem_submit(&sna->kgem);
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	if (sna->render_state.gen3.need_invariant)
		gen3_emit_invariant(sna);
#undef MAX_OBJECTS
}

static void gen3_emit_target(struct sna *sna,
			     struct kgem_bo *bo,
			     int width,
			     int height,
			     int format)
{
	struct gen3_render_state *state = &sna->render_state.gen3;

	assert(!too_large(width, height));

	/* BUF_INFO is an implicit flush, so skip if the target is unchanged. */
	assert(bo->unique_id != 0);
	if (bo->unique_id != state->current_dst) {
		uint32_t v;

		DBG(("%s: setting new target id=%d, handle=%d\n",
		     __FUNCTION__, bo->unique_id, bo->handle));

		OUT_BATCH(_3DSTATE_BUF_INFO_CMD);
		OUT_BATCH(BUF_3D_ID_COLOR_BACK |
			  gen3_buf_tiling(bo->tiling) |
			  bo->pitch);
		OUT_BATCH(kgem_add_reloc(&sna->kgem, sna->kgem.nbatch,
					 bo,
					 I915_GEM_DOMAIN_RENDER << 16 |
					 I915_GEM_DOMAIN_RENDER,
					 0));

		OUT_BATCH(_3DSTATE_DST_BUF_VARS_CMD);
		OUT_BATCH(gen3_get_dst_format(format));

		v = DRAW_YMAX(height - 1) | DRAW_XMAX(width - 1);
		if (v != state->last_drawrect_limit) {
			OUT_BATCH(_3DSTATE_DRAW_RECT_CMD);
			OUT_BATCH(0); /* XXX dither origin? */
			OUT_BATCH(0);
			OUT_BATCH(v);
			OUT_BATCH(0);
			state->last_drawrect_limit = v;
		}

		state->current_dst = bo->unique_id;
	}
	assert(bo->exec);
	kgem_bo_mark_dirty(bo);
}

static void gen3_emit_composite_state(struct sna *sna,
				      const struct sna_composite_op *op)
{
	struct gen3_render_state *state = &sna->render_state.gen3;
	uint32_t map[4];
	uint32_t sampler[4];
	struct kgem_bo *bo[2];
	unsigned int tex_count, n;
	uint32_t ss2;

	gen3_get_batch(sna, op);

	if (kgem_bo_is_dirty(op->src.bo) || kgem_bo_is_dirty(op->mask.bo)) {
		if (op->src.bo == op->dst.bo || op->mask.bo == op->dst.bo)
			OUT_BATCH(MI_FLUSH | MI_INVALIDATE_MAP_CACHE);
		else
			OUT_BATCH(_3DSTATE_MODES_5_CMD |
				  PIPELINE_FLUSH_RENDER_CACHE |
				  PIPELINE_FLUSH_TEXTURE_CACHE);
		kgem_clear_dirty(&sna->kgem);
	}

	gen3_emit_target(sna,
			 op->dst.bo,
			 op->dst.width,
			 op->dst.height,
			 op->dst.format);

	ss2 = ~0;
	tex_count = 0;
	switch (op->src.u.gen3.type) {
	case SHADER_OPACITY:
	case SHADER_NONE:
		assert(0);
	case SHADER_ZERO:
	case SHADER_BLACK:
	case SHADER_WHITE:
		break;
	case SHADER_CONSTANT:
		if (op->src.u.gen3.mode != state->last_diffuse) {
			OUT_BATCH(_3DSTATE_DFLT_DIFFUSE_CMD);
			OUT_BATCH(op->src.u.gen3.mode);
			state->last_diffuse = op->src.u.gen3.mode;
		}
		break;
	case SHADER_LINEAR:
	case SHADER_RADIAL:
	case SHADER_TEXTURE:
		ss2 &= ~S2_TEXCOORD_FMT(tex_count, TEXCOORDFMT_NOT_PRESENT);
		ss2 |= S2_TEXCOORD_FMT(tex_count,
				       op->src.is_affine ? TEXCOORDFMT_2D : TEXCOORDFMT_4D);
		assert(op->src.card_format);
		map[tex_count * 2 + 0] =
			op->src.card_format |
			gen3_ms_tiling(op->src.bo->tiling) |
			(op->src.height - 1) << MS3_HEIGHT_SHIFT |
			(op->src.width - 1) << MS3_WIDTH_SHIFT;
		map[tex_count * 2 + 1] =
			(op->src.bo->pitch / 4 - 1) << MS4_PITCH_SHIFT;

		sampler[tex_count * 2 + 0] = op->src.filter;
		sampler[tex_count * 2 + 1] =
			op->src.repeat |
			tex_count << SS3_TEXTUREMAP_INDEX_SHIFT;
		bo[tex_count] = op->src.bo;
		tex_count++;
		break;
	}
	switch (op->mask.u.gen3.type) {
	case SHADER_NONE:
	case SHADER_ZERO:
	case SHADER_BLACK:
	case SHADER_WHITE:
		break;
	case SHADER_CONSTANT:
		if (op->mask.u.gen3.mode != state->last_specular) {
			OUT_BATCH(_3DSTATE_DFLT_SPEC_CMD);
			OUT_BATCH(op->mask.u.gen3.mode);
			state->last_specular = op->mask.u.gen3.mode;
		}
		break;
	case SHADER_LINEAR:
	case SHADER_RADIAL:
	case SHADER_TEXTURE:
		ss2 &= ~S2_TEXCOORD_FMT(tex_count, TEXCOORDFMT_NOT_PRESENT);
		ss2 |= S2_TEXCOORD_FMT(tex_count,
				       op->mask.is_affine ? TEXCOORDFMT_2D : TEXCOORDFMT_4D);
		assert(op->mask.card_format);
		map[tex_count * 2 + 0] =
			op->mask.card_format |
			gen3_ms_tiling(op->mask.bo->tiling) |
			(op->mask.height - 1) << MS3_HEIGHT_SHIFT |
			(op->mask.width - 1) << MS3_WIDTH_SHIFT;
		map[tex_count * 2 + 1] =
			(op->mask.bo->pitch / 4 - 1) << MS4_PITCH_SHIFT;

		sampler[tex_count * 2 + 0] = op->mask.filter;
		sampler[tex_count * 2 + 1] =
			op->mask.repeat |
			tex_count << SS3_TEXTUREMAP_INDEX_SHIFT;
		bo[tex_count] = op->mask.bo;
		tex_count++;
		break;
	case SHADER_OPACITY:
		ss2 &= ~S2_TEXCOORD_FMT(tex_count, TEXCOORDFMT_NOT_PRESENT);
		ss2 |= S2_TEXCOORD_FMT(tex_count, TEXCOORDFMT_1D);
		break;
	}

	{
		uint32_t blend_offset = sna->kgem.nbatch;

		OUT_BATCH(_3DSTATE_LOAD_STATE_IMMEDIATE_1 | I1_LOAD_S(2) | I1_LOAD_S(6) | 1);
		OUT_BATCH(ss2);
		OUT_BATCH(gen3_get_blend_cntl(op->op,
					      op->has_component_alpha,
					      op->dst.format));

		if (memcmp(sna->kgem.batch + state->last_blend + 1,
			   sna->kgem.batch + blend_offset + 1,
			   2 * 4) == 0)
			sna->kgem.nbatch = blend_offset;
		else
			state->last_blend = blend_offset;
	}

	if (op->u.gen3.num_constants) {
		int count = op->u.gen3.num_constants;
		if (state->last_constants) {
			int last = sna->kgem.batch[state->last_constants+1];
			if (last == (1 << (count >> 2)) - 1 &&
			    memcmp(&sna->kgem.batch[state->last_constants+2],
				   op->u.gen3.constants,
				   count * sizeof(uint32_t)) == 0)
				count = 0;
		}
		if (count) {
			state->last_constants = sna->kgem.nbatch;
			OUT_BATCH(_3DSTATE_PIXEL_SHADER_CONSTANTS | count);
			OUT_BATCH((1 << (count >> 2)) - 1);

			memcpy(sna->kgem.batch + sna->kgem.nbatch,
			       op->u.gen3.constants,
			       count * sizeof(uint32_t));
			sna->kgem.nbatch += count;
		}
	}

	if (tex_count != 0) {
		uint32_t rewind;

		n = 0;
		if (tex_count == state->tex_count) {
			for (; n < tex_count; n++) {
				if (map[2*n+0] != state->tex_map[2*n+0] ||
				    map[2*n+1] != state->tex_map[2*n+1] ||
				    state->tex_handle[n] != bo[n]->handle ||
				    state->tex_delta[n] != bo[n]->delta)
					break;
			}
		}
		if (n < tex_count) {
			OUT_BATCH(_3DSTATE_MAP_STATE | (3 * tex_count));
			OUT_BATCH((1 << tex_count) - 1);
			for (n = 0; n < tex_count; n++) {
				OUT_BATCH(kgem_add_reloc(&sna->kgem,
							 sna->kgem.nbatch,
							 bo[n],
							 I915_GEM_DOMAIN_SAMPLER<< 16,
							 0));
				OUT_BATCH(map[2*n + 0]);
				OUT_BATCH(map[2*n + 1]);

				state->tex_map[2*n+0] = map[2*n+0];
				state->tex_map[2*n+1] = map[2*n+1];
				state->tex_handle[n] = bo[n]->handle;
				state->tex_delta[n] = bo[n]->delta;
			}
			state->tex_count = n;
		}

		rewind = sna->kgem.nbatch;
		OUT_BATCH(_3DSTATE_SAMPLER_STATE | (3 * tex_count));
		OUT_BATCH((1 << tex_count) - 1);
		for (n = 0; n < tex_count; n++) {
			OUT_BATCH(sampler[2*n + 0]);
			OUT_BATCH(sampler[2*n + 1]);
			OUT_BATCH(0);
		}
		if (state->last_sampler &&
		    memcmp(&sna->kgem.batch[state->last_sampler+1],
			   &sna->kgem.batch[rewind + 1],
			   (3*tex_count + 1)*sizeof(uint32_t)) == 0)
			sna->kgem.nbatch = rewind;
		else
			state->last_sampler = rewind;
	}

	gen3_composite_emit_shader(sna, op, op->op);
}

static bool gen3_magic_ca_pass(struct sna *sna,
			       const struct sna_composite_op *op)
{
	if (!op->need_magic_ca_pass)
		return false;

	DBG(("%s(%d)\n", __FUNCTION__,
	     sna->render.vertex_index - sna->render.vertex_start));

	OUT_BATCH(_3DSTATE_LOAD_STATE_IMMEDIATE_1 | I1_LOAD_S(6) | 0);
	OUT_BATCH(gen3_get_blend_cntl(PictOpAdd, true, op->dst.format));
	gen3_composite_emit_shader(sna, op, PictOpAdd);

	OUT_BATCH(PRIM3D_RECTLIST | PRIM3D_INDIRECT_SEQUENTIAL |
		  (sna->render.vertex_index - sna->render.vertex_start));
	OUT_BATCH(sna->render.vertex_start);

	sna->render_state.gen3.last_blend = 0;
	return true;
}

static void gen3_vertex_flush(struct sna *sna)
{
	assert(sna->render.vertex_offset);

	DBG(("%s[%x] = %d\n", __FUNCTION__,
	     4*sna->render.vertex_offset,
	     sna->render.vertex_index - sna->render.vertex_start));

	sna->kgem.batch[sna->render.vertex_offset] =
		PRIM3D_RECTLIST | PRIM3D_INDIRECT_SEQUENTIAL |
		(sna->render.vertex_index - sna->render.vertex_start);
	sna->kgem.batch[sna->render.vertex_offset + 1] =
		sna->render.vertex_start;

	sna->render.vertex_offset = 0;
}

static int gen3_vertex_finish(struct sna *sna)
{
	struct kgem_bo *bo;

	DBG(("%s: used=%d/%d, vbo active? %d\n",
	     __FUNCTION__, sna->render.vertex_used, sna->render.vertex_size,
	     sna->render.vbo ? sna->render.vbo->handle : 0));
	assert(sna->render.vertex_offset == 0);
	assert(sna->render.vertex_used);
	assert(sna->render.vertex_used <= sna->render.vertex_size);

	sna_vertex_wait__locked(&sna->render);

	bo = sna->render.vbo;
	if (bo) {
		DBG(("%s: reloc = %d\n", __FUNCTION__,
		     sna->render.vertex_reloc[0]));

		if (sna->render.vertex_reloc[0]) {
			sna->kgem.batch[sna->render.vertex_reloc[0]] =
				kgem_add_reloc(&sna->kgem, sna->render.vertex_reloc[0],
					       bo, I915_GEM_DOMAIN_VERTEX << 16, 0);

			sna->render.vertex_reloc[0] = 0;
		}
		sna->render.vertex_used = 0;
		sna->render.vertex_index = 0;
		sna->render.vbo = NULL;

		kgem_bo_destroy(&sna->kgem, bo);
	}

	sna->render.vertices = NULL;
	sna->render.vbo = kgem_create_linear(&sna->kgem,
					     256*1024, CREATE_GTT_MAP);
	if (sna->render.vbo)
		sna->render.vertices = kgem_bo_map(&sna->kgem, sna->render.vbo);
	if (sna->render.vertices == NULL) {
		if (sna->render.vbo)
			kgem_bo_destroy(&sna->kgem, sna->render.vbo);
		sna->render.vbo = NULL;
		return 0;
	}
	assert(sna->render.vbo->snoop == false);

	if (sna->render.vertex_used) {
		memcpy(sna->render.vertices,
		       sna->render.vertex_data,
		       sizeof(float)*sna->render.vertex_used);
	}
	sna->render.vertex_size = 64 * 1024 - 1;
	return sna->render.vertex_size - sna->render.vertex_used;
}

static void gen3_vertex_close(struct sna *sna)
{
	struct kgem_bo *bo, *free_bo = NULL;
	unsigned int delta = 0;

	assert(sna->render.vertex_offset == 0);
	if (sna->render.vertex_reloc[0] == 0)
		return;

	DBG(("%s: used=%d/%d, vbo active? %d\n",
	     __FUNCTION__, sna->render.vertex_used, sna->render.vertex_size,
	     sna->render.vbo ? sna->render.vbo->handle : 0));

	bo = sna->render.vbo;
	if (bo) {
		if (sna->render.vertex_size - sna->render.vertex_used < 64) {
			DBG(("%s: discarding full vbo\n", __FUNCTION__));
			sna->render.vbo = NULL;
			sna->render.vertices = sna->render.vertex_data;
			sna->render.vertex_size = ARRAY_SIZE(sna->render.vertex_data);
			free_bo = bo;
		} else if (sna->render.vertices == MAP(bo->map__cpu)) {
			DBG(("%s: converting CPU map to GTT\n", __FUNCTION__));
			sna->render.vertices = kgem_bo_map__gtt(&sna->kgem, bo);
			if (sna->render.vertices == NULL) {
				DBG(("%s: discarding non-mappable vertices\n",__FUNCTION__));
				sna->render.vbo = NULL;
				sna->render.vertices = sna->render.vertex_data;
				sna->render.vertex_size = ARRAY_SIZE(sna->render.vertex_data);
				free_bo = bo;
			}
		}
	} else {
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
			DBG(("%s: new vbo: %d\n", __FUNCTION__,
			     sna->render.vertex_used));
			bo = kgem_create_linear(&sna->kgem,
						4*sna->render.vertex_used,
						CREATE_NO_THROTTLE);
			if (bo) {
				assert(bo->snoop == false);
				kgem_bo_write(&sna->kgem, bo,
					      sna->render.vertex_data,
					      4*sna->render.vertex_used);
			}
			free_bo = bo;
		}
	}

	DBG(("%s: reloc = %d\n", __FUNCTION__, sna->render.vertex_reloc[0]));
	sna->kgem.batch[sna->render.vertex_reloc[0]] =
		kgem_add_reloc(&sna->kgem, sna->render.vertex_reloc[0],
			       bo, I915_GEM_DOMAIN_VERTEX << 16, delta);
	sna->render.vertex_reloc[0] = 0;

	if (sna->render.vbo == NULL) {
		DBG(("%s: resetting vbo\n", __FUNCTION__));
		sna->render.vertex_used = 0;
		sna->render.vertex_index = 0;
		assert(sna->render.vertices == sna->render.vertex_data);
		assert(sna->render.vertex_size == ARRAY_SIZE(sna->render.vertex_data));
	}

	if (free_bo)
		kgem_bo_destroy(&sna->kgem, free_bo);
}

static bool gen3_rectangle_begin(struct sna *sna,
				 const struct sna_composite_op *op)
{
	struct gen3_render_state *state = &sna->render_state.gen3;
	int ndwords, i1_cmd = 0, i1_len = 0;

	if (sna_vertex_wait__locked(&sna->render) && sna->render.vertex_offset)
		return true;

	ndwords = 2;
	if (op->need_magic_ca_pass)
		ndwords += 100;
	if (sna->render.vertex_reloc[0] == 0)
		i1_len++, i1_cmd |= I1_LOAD_S(0), ndwords++;
	if (state->floats_per_vertex != op->floats_per_vertex)
		i1_len++, i1_cmd |= I1_LOAD_S(1), ndwords++;

	if (!kgem_check_batch(&sna->kgem, ndwords+1))
		return false;

	if (i1_cmd) {
		OUT_BATCH(_3DSTATE_LOAD_STATE_IMMEDIATE_1 | i1_cmd | (i1_len - 1));
		if (sna->render.vertex_reloc[0] == 0)
			sna->render.vertex_reloc[0] = sna->kgem.nbatch++;
		if (state->floats_per_vertex != op->floats_per_vertex) {
			state->floats_per_vertex = op->floats_per_vertex;
			OUT_BATCH(state->floats_per_vertex << S1_VERTEX_WIDTH_SHIFT |
				  state->floats_per_vertex << S1_VERTEX_PITCH_SHIFT);
		}
	}

	if (sna->kgem.nbatch == 2 + state->last_vertex_offset &&
	    !op->need_magic_ca_pass) {
		sna->render.vertex_offset = state->last_vertex_offset;
	} else {
		sna->render.vertex_offset = sna->kgem.nbatch;
		OUT_BATCH(MI_NOOP); /* to be filled later */
		OUT_BATCH(MI_NOOP);
		sna->render.vertex_start = sna->render.vertex_index;
		state->last_vertex_offset = sna->render.vertex_offset;
	}

	return true;
}

static int gen3_get_rectangles__flush(struct sna *sna,
				      const struct sna_composite_op *op)
{
	/* Preventing discarding new vbo after lock contention */
	if (sna_vertex_wait__locked(&sna->render)) {
		int rem = vertex_space(sna);
		if (rem > op->floats_per_rect)
			return rem;
	}

	if (!kgem_check_batch(&sna->kgem, op->need_magic_ca_pass ? 105: 5))
		return 0;
	if (!kgem_check_reloc_and_exec(&sna->kgem, 1))
		return 0;

	if (sna->render.vertex_offset) {
		gen3_vertex_flush(sna);
		if (gen3_magic_ca_pass(sna, op)) {
			OUT_BATCH(_3DSTATE_LOAD_STATE_IMMEDIATE_1 | I1_LOAD_S(6) | 0);
			OUT_BATCH(gen3_get_blend_cntl(op->op,
						      op->has_component_alpha,
						      op->dst.format));
			gen3_composite_emit_shader(sna, op, op->op);
		}
	}

	return gen3_vertex_finish(sna);
}

inline static int gen3_get_rectangles(struct sna *sna,
				      const struct sna_composite_op *op,
				      int want)
{
	int rem;

	DBG(("%s: want=%d, rem=%d\n",
	     __FUNCTION__, want*op->floats_per_rect, vertex_space(sna)));

	assert(want);
	assert(sna->render.vertex_index * op->floats_per_vertex == sna->render.vertex_used);

start:
	rem = vertex_space(sna);
	if (unlikely(op->floats_per_rect > rem)) {
		DBG(("flushing vbo for %s: %d < %d\n",
		     __FUNCTION__, rem, op->floats_per_rect));
		rem = gen3_get_rectangles__flush(sna, op);
		if (unlikely(rem == 0))
			goto flush;
	}

	if (unlikely(sna->render.vertex_offset == 0)) {
		if (!gen3_rectangle_begin(sna, op))
			goto flush;
		else
			goto start;
	}

	assert(rem <= vertex_space(sna));
	assert(op->floats_per_rect <= rem);
	if (want > 1 && want * op->floats_per_rect > rem)
		want = rem / op->floats_per_rect;
	sna->render.vertex_index += 3*want;

	assert(want);
	assert(sna->render.vertex_index * op->floats_per_vertex <= sna->render.vertex_size);
	return want;

flush:
	DBG(("%s: flushing batch\n", __FUNCTION__));
	if (sna->render.vertex_offset) {
		gen3_vertex_flush(sna);
		gen3_magic_ca_pass(sna, op);
	}
	sna_vertex_wait__locked(&sna->render);
	_kgem_submit(&sna->kgem);
	gen3_emit_composite_state(sna, op);
	assert(sna->render.vertex_offset == 0);
	assert(sna->render.vertex_reloc[0] == 0);
	goto start;
}

fastcall static void
gen3_render_composite_blt(struct sna *sna,
			  const struct sna_composite_op *op,
			  const struct sna_composite_rectangles *r)
{
	DBG(("%s: src=(%d, %d)+(%d, %d), mask=(%d, %d)+(%d, %d), dst=(%d, %d)+(%d, %d), size=(%d, %d)\n", __FUNCTION__,
	     r->src.x, r->src.y, op->src.offset[0], op->src.offset[1],
	     r->mask.x, r->mask.y, op->mask.offset[0], op->mask.offset[1],
	     r->dst.x, r->dst.y, op->dst.x, op->dst.y,
	     r->width, r->height));

	gen3_get_rectangles(sna, op, 1);

	op->prim_emit(sna, op, r);
}

#if 0
fastcall static void
gen3_render_composite_box(struct sna *sna,
			  const struct sna_composite_op *op,
			  const BoxRec *box)
{
	struct sna_composite_rectangles r;

	DBG(("%s: src=+(%d, %d), mask=+(%d, %d), dst=+(%d, %d)\n",
	     __FUNCTION__,
	     op->src.offset[0], op->src.offset[1],
	     op->mask.offset[0], op->mask.offset[1],
	     op->dst.x, op->dst.y));

	gen3_get_rectangles(sna, op, 1);

	r.dst.x  = box->x1;
	r.dst.y  = box->y1;
	r.width  = box->x2 - box->x1;
	r.height = box->y2 - box->y1;
	r.src = r.mask = r.dst;

	op->prim_emit(sna, op, &r);
}

static void
gen3_render_composite_boxes__blt(struct sna *sna,
				 const struct sna_composite_op *op,
				 const BoxRec *box, int nbox)
{
	DBG(("%s: nbox=%d, src=+(%d, %d), mask=+(%d, %d), dst=+(%d, %d)\n",
	     __FUNCTION__, nbox,
	     op->src.offset[0], op->src.offset[1],
	     op->mask.offset[0], op->mask.offset[1],
	     op->dst.x, op->dst.y));

	do {
		int nbox_this_time;

		nbox_this_time = gen3_get_rectangles(sna, op, nbox);
		nbox -= nbox_this_time;

		do {
			struct sna_composite_rectangles r;

			DBG(("  %s: (%d, %d) x (%d, %d)\n", __FUNCTION__,
			     box->x1, box->y1,
			     box->x2 - box->x1,
			     box->y2 - box->y1));

			r.dst.x  = box->x1; r.dst.y  = box->y1;
			r.width = box->x2 - box->x1;
			r.height = box->y2 - box->y1;
			r.src = r.mask = r.dst;

			op->prim_emit(sna, op, &r);
			box++;
		} while (--nbox_this_time);
	} while (nbox);
}

static void
gen3_render_composite_boxes(struct sna *sna,
			    const struct sna_composite_op *op,
			    const BoxRec *box, int nbox)
{
	DBG(("%s: nbox=%d\n", __FUNCTION__, nbox));

	do {
		int nbox_this_time;
		float *v;

		nbox_this_time = gen3_get_rectangles(sna, op, nbox);
		assert(nbox_this_time);
		nbox -= nbox_this_time;

		v = sna->render.vertices + sna->render.vertex_used;
		sna->render.vertex_used += nbox_this_time * op->floats_per_rect;

		op->emit_boxes(op, box, nbox_this_time, v);
		box += nbox_this_time;
	} while (nbox);
}

static void
gen3_render_composite_boxes__thread(struct sna *sna,
				    const struct sna_composite_op *op,
				    const BoxRec *box, int nbox)
{
	DBG(("%s: nbox=%d\n", __FUNCTION__, nbox));

	sna_vertex_lock(&sna->render);
	do {
		int nbox_this_time;
		float *v;

		nbox_this_time = gen3_get_rectangles(sna, op, nbox);
		assert(nbox_this_time);
		nbox -= nbox_this_time;

		v = sna->render.vertices + sna->render.vertex_used;
		sna->render.vertex_used += nbox_this_time * op->floats_per_rect;

		sna_vertex_acquire__locked(&sna->render);
		sna_vertex_unlock(&sna->render);

		op->emit_boxes(op, box, nbox_this_time, v);
		box += nbox_this_time;

		sna_vertex_lock(&sna->render);
		sna_vertex_release__locked(&sna->render);
	} while (nbox);
	sna_vertex_unlock(&sna->render);
}
#endif

static void
gen3_render_composite_done(struct sna *sna,
			   const struct sna_composite_op *op)
{
	DBG(("%s()\n", __FUNCTION__));

	if (sna->render.vertex_offset) {
		gen3_vertex_flush(sna);
		gen3_magic_ca_pass(sna, op);
	}

}

static void
discard_vbo(struct sna *sna)
{
	kgem_bo_destroy(&sna->kgem, sna->render.vbo);
	sna->render.vbo = NULL;
	sna->render.vertices = sna->render.vertex_data;
	sna->render.vertex_size = ARRAY_SIZE(sna->render.vertex_data);
	sna->render.vertex_used = 0;
	sna->render.vertex_index = 0;
}

static void
gen3_render_reset(struct sna *sna)
{
	struct gen3_render_state *state = &sna->render_state.gen3;

	state->need_invariant = true;
	state->current_dst = 0;
	state->tex_count = 0;
	state->last_drawrect_limit = ~0U;
	state->last_target = 0;
	state->last_blend = 0;
	state->last_constants = 0;
	state->last_sampler = 0;
	state->last_shader = 0x7fffffff;
	state->last_diffuse = 0xcc00ffee;
	state->last_specular = 0xcc00ffee;

	state->floats_per_vertex = 0;
	state->last_floats_per_vertex = 0;
	state->last_vertex_offset = 0;

	if (sna->render.vbo && !kgem_bo_can_map(&sna->kgem, sna->render.vbo)) {
		DBG(("%s: discarding vbo as next access will stall: %d\n",
		     __FUNCTION__, sna->render.vbo->presumed_offset));
		discard_vbo(sna);
	}

	sna->render.vertex_reloc[0] = 0;
	sna->render.vertex_offset = 0;
}

static void
gen3_render_retire(struct kgem *kgem)
{
	struct sna *sna;

	sna = container_of(kgem, struct sna, kgem);
	if (sna->render.vertex_reloc[0] == 0 &&
	    sna->render.vbo && !kgem_bo_is_busy(sna->render.vbo)) {
		DBG(("%s: resetting idle vbo\n", __FUNCTION__));
		sna->render.vertex_used = 0;
		sna->render.vertex_index = 0;
	}
}

static void
gen3_render_expire(struct kgem *kgem)
{
	struct sna *sna;

	sna = container_of(kgem, struct sna, kgem);
	if (sna->render.vbo && !sna->render.vertex_used) {
		DBG(("%s: discarding vbo\n", __FUNCTION__));
		discard_vbo(sna);
	}
}

static bool gen3_composite_channel_set_format(struct sna_composite_channel *channel,
					      CARD32 format)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(gen3_tex_formats); i++) {
		if (gen3_tex_formats[i].fmt == format) {
			channel->card_format = gen3_tex_formats[i].card_fmt;
			channel->rb_reversed = gen3_tex_formats[i].rb_reversed;
			return true;
		}
	}
	return false;
}

#if 0
static bool source_is_covered(PicturePtr picture,
			      int x, int y,
			      int width, int height)
{
	int x1, y1, x2, y2;

	if (picture->repeat && picture->repeatType != RepeatNone)
		return true;

	if (picture->pDrawable == NULL)
		return false;

	if (picture->transform) {
		pixman_box16_t sample;

		sample.x1 = x;
		sample.y1 = y;
		sample.x2 = x + width;
		sample.y2 = y + height;

		pixman_transform_bounds(picture->transform, &sample);

		x1 = sample.x1;
		x2 = sample.x2;
		y1 = sample.y1;
		y2 = sample.y2;
	} else {
		x1 = x;
		y1 = y;
		x2 = x + width;
		y2 = y + height;
	}

	return
		x1 >= 0 && y1 >= 0 &&
		x2 <= picture->pDrawable->width &&
		y2 <= picture->pDrawable->height;
}

static bool gen3_composite_channel_set_xformat(PicturePtr picture,
					       struct sna_composite_channel *channel,
					       int x, int y,
					       int width, int height)
{
	unsigned int i;

	if (PICT_FORMAT_A(picture->format) != 0)
		return false;

	if (width == 0 || height == 0)
		return false;

	if (!source_is_covered(picture, x, y, width, height))
		return false;

	for (i = 0; i < ARRAY_SIZE(gen3_tex_formats); i++) {
		if (gen3_tex_formats[i].xfmt == picture->format) {
			channel->card_format = gen3_tex_formats[i].card_fmt;
			channel->rb_reversed = gen3_tex_formats[i].rb_reversed;
			channel->alpha_fixup = true;
			return true;
		}
	}

	return false;
}

static int
gen3_init_solid(struct sna_composite_channel *channel, uint32_t color)
{
	channel->u.gen3.mode = color;
	channel->u.gen3.type = SHADER_CONSTANT;
	if (color == 0)
		channel->u.gen3.type = SHADER_ZERO;
	else if (color == 0xff000000)
		channel->u.gen3.type = SHADER_BLACK;
	else if (color == 0xffffffff)
		channel->u.gen3.type = SHADER_WHITE;

	channel->bo = NULL;
	channel->is_opaque = (color >> 24) == 0xff;
	channel->is_affine = 1;
	channel->alpha_fixup = 0;
	channel->rb_reversed = 0;

	DBG(("%s: color=%08x, is_opaque=%d, type=%d\n",
	     __FUNCTION__, color, channel->is_opaque, channel->u.gen3.type));

	/* for consistency */
	channel->repeat = RepeatNormal;
	channel->filter = PictFilterNearest;
	channel->pict_format = PICT_a8r8g8b8;
	channel->card_format = MAPSURF_32BIT | MT_32BIT_ARGB8888;

	return 1;
}

static void gen3_composite_channel_convert(struct sna_composite_channel *channel)
{
	if (channel->u.gen3.type == SHADER_TEXTURE)
		channel->repeat = gen3_texture_repeat(channel->repeat);
	else
		channel->repeat = gen3_gradient_repeat(channel->repeat);

	channel->filter = gen3_filter(channel->filter);
	if (channel->card_format == 0)
		gen3_composite_channel_set_format(channel, channel->pict_format);
	assert(channel->card_format);
}

static bool gen3_gradient_setup(struct sna *sna,
				PicturePtr picture,
				struct sna_composite_channel *channel,
				int16_t ox, int16_t oy)
{
	int16_t dx, dy;

	if (picture->repeat == 0) {
		channel->repeat = RepeatNone;
	} else switch (picture->repeatType) {
	case RepeatNone:
	case RepeatNormal:
	case RepeatPad:
	case RepeatReflect:
		channel->repeat = picture->repeatType;
		break;
	default:
		return false;
	}

	channel->bo =
		sna_render_get_gradient(sna,
					(PictGradient *)picture->pSourcePict);
	if (channel->bo == NULL)
		return false;

	channel->pict_format = PICT_a8r8g8b8;
	channel->card_format = MAPSURF_32BIT | MT_32BIT_ARGB8888;
	channel->filter = PictFilterNearest;
	channel->is_affine = sna_transform_is_affine(picture->transform);
	if (sna_transform_is_integer_translation(picture->transform, &dx, &dy)) {
		DBG(("%s: integer translation (%d, %d), removing\n",
		     __FUNCTION__, dx, dy));
		ox += dx;
		oy += dy;
		channel->transform = NULL;
	} else
		channel->transform = picture->transform;
	channel->width  = channel->bo->pitch / 4;
	channel->height = 1;
	channel->offset[0] = ox;
	channel->offset[1] = oy;
	channel->scale[0] = channel->scale[1] = 1;
	return true;
}

static int
gen3_init_linear(struct sna *sna,
		 PicturePtr picture,
		 struct sna_composite_op *op,
		 struct sna_composite_channel *channel,
		 int ox, int oy)
{
	PictLinearGradient *linear =
		(PictLinearGradient *)picture->pSourcePict;
	float x0, y0, sf;
	float dx, dy, offset;
	int n;

	DBG(("%s: p1=(%f, %f), p2=(%f, %f)\n",
	     __FUNCTION__,
	     xFixedToDouble(linear->p1.x), xFixedToDouble(linear->p1.y),
	     xFixedToDouble(linear->p2.x), xFixedToDouble(linear->p2.y)));

	if (linear->p2.x == linear->p1.x && linear->p2.y == linear->p1.y)
		return 0;

	dx = xFixedToDouble(linear->p2.x - linear->p1.x);
	dy = xFixedToDouble(linear->p2.y - linear->p1.y);
	sf = dx*dx + dy*dy;
	dx /= sf;
	dy /= sf;

	x0 = xFixedToDouble(linear->p1.x);
	y0 = xFixedToDouble(linear->p1.y);
	offset = dx*x0 + dy*y0;

	n = op->u.gen3.num_constants;
	channel->u.gen3.constants = FS_C0 + n / 4;
	op->u.gen3.constants[n++] = dx;
	op->u.gen3.constants[n++] = dy;
	op->u.gen3.constants[n++] = -offset;
	op->u.gen3.constants[n++] = 0;

	if (!gen3_gradient_setup(sna, picture, channel, ox, oy))
		return -1;

	channel->u.gen3.type = SHADER_LINEAR;
	op->u.gen3.num_constants = n;

	DBG(("%s: dx=%f, dy=%f, offset=%f, constants=%d\n",
	     __FUNCTION__, dx, dy, -offset, channel->u.gen3.constants - FS_C0));
	return 1;
}

static int
gen3_init_radial(struct sna *sna,
		 PicturePtr picture,
		 struct sna_composite_op *op,
		 struct sna_composite_channel *channel,
		 int ox, int oy)
{
	PictRadialGradient *radial = (PictRadialGradient *)picture->pSourcePict;
	double dx, dy, dr, r1;
	int n;

	dx = xFixedToDouble(radial->c2.x - radial->c1.x);
	dy = xFixedToDouble(radial->c2.y - radial->c1.y);
	dr = xFixedToDouble(radial->c2.radius - radial->c1.radius);

	r1 = xFixedToDouble(radial->c1.radius);

	n = op->u.gen3.num_constants;
	channel->u.gen3.constants = FS_C0 + n / 4;
	if (radial->c2.x == radial->c1.x && radial->c2.y == radial->c1.y) {
		if (radial->c2.radius == radial->c1.radius) {
			channel->u.gen3.type = SHADER_ZERO;
			return 1;
		}

		op->u.gen3.constants[n++] = xFixedToDouble(radial->c1.x) / dr;
		op->u.gen3.constants[n++] = xFixedToDouble(radial->c1.y) / dr;
		op->u.gen3.constants[n++] = 1. / dr;
		op->u.gen3.constants[n++] = -r1 / dr;

		channel->u.gen3.mode = RADIAL_ONE;
	} else {
		op->u.gen3.constants[n++] = -xFixedToDouble(radial->c1.x);
		op->u.gen3.constants[n++] = -xFixedToDouble(radial->c1.y);
		op->u.gen3.constants[n++] = r1;
		op->u.gen3.constants[n++] = -4 * (dx*dx + dy*dy - dr*dr);

		op->u.gen3.constants[n++] = -2 * dx;
		op->u.gen3.constants[n++] = -2 * dy;
		op->u.gen3.constants[n++] = -2 * r1 * dr;
		op->u.gen3.constants[n++] = 1 / (2 * (dx*dx + dy*dy - dr*dr));

		channel->u.gen3.mode = RADIAL_TWO;
	}

	if (!gen3_gradient_setup(sna, picture, channel, ox, oy))
		return -1;

	channel->u.gen3.type = SHADER_RADIAL;
	op->u.gen3.num_constants = n;
	return 1;
}

static bool
sna_picture_is_clear(PicturePtr picture,
		     int x, int y, int w, int h,
		     uint32_t *color)
{
	struct sna_pixmap *priv;

	if (!picture->pDrawable)
		return false;

	priv = sna_pixmap(get_drawable_pixmap(picture->pDrawable));
	if (priv == NULL || !priv->clear)
		return false;

	if (!source_is_covered(picture, x, y, w, h))
		return false;

	*color = priv->clear_color;
	return true;
}

static int
gen3_composite_picture(struct sna *sna,
		       PicturePtr picture,
		       struct sna_composite_op *op,
		       struct sna_composite_channel *channel,
		       int16_t x, int16_t y,
		       int16_t w, int16_t h,
		       int16_t dst_x, int16_t dst_y,
		       bool precise)
{
	PixmapPtr pixmap;
	uint32_t color;
	int16_t dx, dy;

	DBG(("%s: (%d, %d)x(%d, %d), dst=(%d, %d)\n",
	     __FUNCTION__, x, y, w, h, dst_x, dst_y));

	channel->card_format = 0;

	if (picture->pDrawable == NULL) {
		SourcePict *source = picture->pSourcePict;
		int ret = -1;

		switch (source->type) {
		case SourcePictTypeSolidFill:
			DBG(("%s: solid fill [%08x], format %08x\n",
			     __FUNCTION__,
			     (unsigned)source->solidFill.color,
			     (unsigned)picture->format));
			ret = gen3_init_solid(channel, source->solidFill.color);
			break;

		case SourcePictTypeLinear:
			ret = gen3_init_linear(sna, picture, op, channel,
					       x - dst_x, y - dst_y);
			break;

		case SourcePictTypeRadial:
			ret = gen3_init_radial(sna, picture, op, channel,
					       x - dst_x, y - dst_y);
			break;
		}

		if (ret == -1) {
			if (!precise)
				ret = sna_render_picture_approximate_gradient(sna, picture, channel,
									      x, y, w, h, dst_x, dst_y);
			if (ret == -1)
				ret = sna_render_picture_fixup(sna, picture, channel,
							       x, y, w, h, dst_x, dst_y);
		}
		return ret;
	}

	if (picture->alphaMap) {
		DBG(("%s -- fallback, alphamap\n", __FUNCTION__));
		return sna_render_picture_fixup(sna, picture, channel,
						x, y, w, h, dst_x, dst_y);
	}

	if (sna_picture_is_solid(picture, &color)) {
		DBG(("%s: solid drawable [%08x]\n", __FUNCTION__, color));
		return gen3_init_solid(channel, color);
	}

	if (sna_picture_is_clear(picture, x, y, w, h, &color)) {
		DBG(("%s: clear drawable [%08x]\n", __FUNCTION__, color));
		return gen3_init_solid(channel, color_convert(color, picture->format, PICT_a8r8g8b8));
	}

	if (!gen3_check_repeat(picture))
		return sna_render_picture_fixup(sna, picture, channel,
						x, y, w, h, dst_x, dst_y);

	if (!gen3_check_filter(picture))
		return sna_render_picture_fixup(sna, picture, channel,
						x, y, w, h, dst_x, dst_y);

	channel->repeat = picture->repeat ? picture->repeatType : RepeatNone;
	channel->filter = picture->filter;
	channel->pict_format = picture->format;

	pixmap = get_drawable_pixmap(picture->pDrawable);
	get_drawable_deltas(picture->pDrawable, pixmap, &dx, &dy);

	x += dx + picture->pDrawable->x;
	y += dy + picture->pDrawable->y;

	if (sna_transform_is_integer_translation(picture->transform, &dx, &dy)) {
		DBG(("%s: integer translation (%d, %d), removing\n",
		     __FUNCTION__, dx, dy));
		x += dx;
		y += dy;
		channel->transform = NULL;
		channel->filter = PictFilterNearest;
	} else {
		channel->transform = picture->transform;
		channel->is_affine = sna_transform_is_affine(picture->transform);
	}

	if (!gen3_composite_channel_set_format(channel, picture->format) &&
	    !gen3_composite_channel_set_xformat(picture, channel, x, y, w, h))
		return sna_render_picture_convert(sna, picture, channel, pixmap,
						  x, y, w, h, dst_x, dst_y,
						  false);
	assert(channel->card_format);

	if (too_large(pixmap->drawable.width, pixmap->drawable.height)) {
		DBG(("%s: pixmap too large (%dx%d), extracting (%d, %d)x(%d,%d)\n",
		     __FUNCTION__,
		     pixmap->drawable.width, pixmap->drawable.height,
		     x, y, w, h));
		return sna_render_picture_extract(sna, picture, channel,
						  x, y, w, h, dst_x, dst_y);
	}

	return sna_render_pixmap_bo(sna, channel, pixmap,
				    x, y, w, h, dst_x, dst_y);
}
#endif

static void
gen3_align_vertex(struct sna *sna,
		  const struct sna_composite_op *op)
{
	int vertex_index;

	if (op->floats_per_vertex == sna->render_state.gen3.last_floats_per_vertex)
		return;

	DBG(("aligning vertex: was %d, now %d floats per vertex\n",
	     sna->render_state.gen3.last_floats_per_vertex,
	     op->floats_per_vertex));

	assert(op->floats_per_rect == 3*op->floats_per_vertex);

	vertex_index = (sna->render.vertex_used + op->floats_per_vertex - 1) / op->floats_per_vertex;
	if ((int)sna->render.vertex_size - vertex_index * op->floats_per_vertex < 2*op->floats_per_rect) {
		DBG(("%s: flushing vertex buffer: new index=%d, max=%d\n",
		     __FUNCTION__, vertex_index, sna->render.vertex_size / op->floats_per_vertex));
		if (gen3_vertex_finish(sna) < op->floats_per_vertex)
			kgem_submit(&sna->kgem);

		vertex_index = (sna->render.vertex_used + op->floats_per_vertex - 1) / op->floats_per_vertex;
	}

	sna->render.vertex_index = vertex_index;
	sna->render.vertex_used = vertex_index * op->floats_per_vertex;
}

static inline bool is_constant_ps(uint32_t type)
{
	switch (type) {
	case SHADER_NONE: /* be warned! */
	case SHADER_ZERO:
	case SHADER_BLACK:
	case SHADER_WHITE:
	case SHADER_CONSTANT:
		return true;
	default:
		return false;
	}
}

#if 0
static bool
gen3_composite_fallback(struct sna *sna,
			uint8_t op,
			PicturePtr src,
			PicturePtr mask,
			PicturePtr dst)
{
	PixmapPtr src_pixmap;
	PixmapPtr mask_pixmap;
	PixmapPtr dst_pixmap;
	bool src_fallback, mask_fallback;

	if (!gen3_check_dst_format(dst->format)) {
		DBG(("%s: unknown destination format: %d\n",
		     __FUNCTION__, dst->format));
		return true;
	}

	dst_pixmap = get_drawable_pixmap(dst->pDrawable);

	src_pixmap = src->pDrawable ? get_drawable_pixmap(src->pDrawable) : NULL;
	src_fallback = source_fallback(src, src_pixmap,
				       dst->polyMode == PolyModePrecise);

	if (mask) {
		mask_pixmap = mask->pDrawable ? get_drawable_pixmap(mask->pDrawable) : NULL;
		mask_fallback = source_fallback(mask, mask_pixmap,
						dst->polyMode == PolyModePrecise);
	} else {
		mask_pixmap = NULL;
		mask_fallback = false;
	}

	/* If we are using the destination as a source and need to
	 * readback in order to upload the source, do it all
	 * on the cpu.
	 */
	if (src_pixmap == dst_pixmap && src_fallback) {
		DBG(("%s: src is dst and will fallback\n",__FUNCTION__));
		return true;
	}
	if (mask_pixmap == dst_pixmap && mask_fallback) {
		DBG(("%s: mask is dst and will fallback\n",__FUNCTION__));
		return true;
	}

	if (mask &&
	    mask->componentAlpha && PICT_FORMAT_RGB(mask->format) &&
	    gen3_blend_op[op].src_alpha &&
	    gen3_blend_op[op].src_blend != BLENDFACT_ZERO &&
	    op != PictOpOver) {
		DBG(("%s: component-alpha mask with op=%d, should fallback\n",
		     __FUNCTION__, op));
		return true;
	}

	/* If anything is on the GPU, push everything out to the GPU */
	if (dst_use_gpu(dst_pixmap)) {
		DBG(("%s: dst is already on the GPU, try to use GPU\n",
		     __FUNCTION__));
		return false;
	}

	if (src_pixmap && !src_fallback) {
		DBG(("%s: src is already on the GPU, try to use GPU\n",
		     __FUNCTION__));
		return false;
	}
	if (mask_pixmap && !mask_fallback) {
		DBG(("%s: mask is already on the GPU, try to use GPU\n",
		     __FUNCTION__));
		return false;
	}

	/* However if the dst is not on the GPU and we need to
	 * render one of the sources using the CPU, we may
	 * as well do the entire operation in place onthe CPU.
	 */
	if (src_fallback) {
		DBG(("%s: dst is on the CPU and src will fallback\n",
		     __FUNCTION__));
		return true;
	}

	if (mask && mask_fallback) {
		DBG(("%s: dst is on the CPU and mask will fallback\n",
		     __FUNCTION__));
		return true;
	}

	if (too_large(dst_pixmap->drawable.width,
		      dst_pixmap->drawable.height) &&
	    dst_is_cpu(dst_pixmap)) {
		DBG(("%s: dst is on the CPU and too large\n", __FUNCTION__));
		return true;
	}

	DBG(("%s: dst is not on the GPU and the operation should not fallback: use-cpu? %d\n",
	     __FUNCTION__, dst_use_cpu(dst_pixmap)));
	return dst_use_cpu(dst_pixmap);
}

static int
reuse_source(struct sna *sna,
	     PicturePtr src, struct sna_composite_channel *sc, int src_x, int src_y,
	     PicturePtr mask, struct sna_composite_channel *mc, int msk_x, int msk_y)
{
	if (src_x != msk_x || src_y != msk_y)
		return false;

	if (mask == src) {
		*mc = *sc;
		if (mc->bo)
			kgem_bo_reference(mc->bo);
		return true;
	}

	if ((src->pDrawable == NULL || mask->pDrawable != src->pDrawable))
		return false;

	if (sc->is_solid)
		return false;

	DBG(("%s: mask reuses source drawable\n", __FUNCTION__));

	if (!sna_transform_equal(src->transform, mask->transform))
		return false;

	if (!sna_picture_alphamap_equal(src, mask))
		return false;

	if (!gen3_check_repeat(mask))
		return false;

	if (!gen3_check_filter(mask))
		return false;

	if (!gen3_check_format(mask))
		return false;

	DBG(("%s: reusing source channel for mask with a twist\n",
	     __FUNCTION__));

	*mc = *sc;
	mc->repeat = gen3_texture_repeat(mask->repeat ? mask->repeatType : RepeatNone);
	mc->filter = gen3_filter(mask->filter);
	mc->pict_format = mask->format;
	gen3_composite_channel_set_format(mc, mask->format);
	assert(mc->card_format);
	if (mc->bo)
		kgem_bo_reference(mc->bo);
	return true;
}

static bool
gen3_render_composite(struct sna *sna,
		      uint8_t op,
		      PicturePtr src,
		      PicturePtr mask,
		      PicturePtr dst,
		      int16_t src_x,  int16_t src_y,
		      int16_t mask_x, int16_t mask_y,
		      int16_t dst_x,  int16_t dst_y,
		      int16_t width,  int16_t height,
		      struct sna_composite_op *tmp)
{
	DBG(("%s()\n", __FUNCTION__));

	if (op >= ARRAY_SIZE(gen3_blend_op)) {
		DBG(("%s: fallback due to unhandled blend op: %d\n",
		     __FUNCTION__, op));
		return false;
	}

	/* Try to use the BLT engine unless it implies a
	 * 3D -> 2D context switch.
	 */
	if (mask == NULL &&
	    sna_blt_composite(sna,
			      op, src, dst,
			      src_x, src_y,
			      dst_x, dst_y,
			      width, height,
			      tmp, false))
		return true;

	if (gen3_composite_fallback(sna, op, src, mask, dst))
		goto fallback;

	if (need_tiling(sna, width, height))
		return sna_tiling_composite(op, src, mask, dst,
					    src_x,  src_y,
					    mask_x, mask_y,
					    dst_x,  dst_y,
					    width,  height,
					    tmp);

	if (!gen3_composite_set_target(sna, tmp, dst,
				       dst_x, dst_y, width, height)) {
		DBG(("%s: unable to set render target\n",
		     __FUNCTION__));
		return false;
	}

	tmp->op = op;
	tmp->rb_reversed = gen3_dst_rb_reversed(tmp->dst.format);
	if (too_large(tmp->dst.width, tmp->dst.height) ||
	    !gen3_check_pitch_3d(tmp->dst.bo)) {
		if (!sna_render_composite_redirect(sna, tmp,
						   dst_x, dst_y, width, height,
						   op > PictOpSrc || dst->pCompositeClip->data))
			goto fallback;
	}

	tmp->u.gen3.num_constants = 0;
	tmp->src.u.gen3.type = SHADER_TEXTURE;
	tmp->src.is_affine = true;
	DBG(("%s: preparing source\n", __FUNCTION__));
	switch (gen3_composite_picture(sna, src, tmp, &tmp->src,
				       src_x, src_y,
				       width, height,
				       dst_x, dst_y,
				       dst->polyMode == PolyModePrecise)) {
	case -1:
		goto cleanup_dst;
	case 0:
		tmp->src.u.gen3.type = SHADER_ZERO;
		break;
	case 1:
		if (mask == NULL && tmp->src.bo &&
		    sna_blt_composite__convert(sna,
					       dst_x, dst_y, width, height,
					       tmp))
			return true;

		gen3_composite_channel_convert(&tmp->src);
		break;
	}
	DBG(("%s: source type=%d\n", __FUNCTION__, tmp->src.u.gen3.type));

	tmp->mask.u.gen3.type = SHADER_NONE;
	tmp->mask.is_affine = true;
	tmp->need_magic_ca_pass = false;
	tmp->has_component_alpha = false;
	if (mask && tmp->src.u.gen3.type != SHADER_ZERO) {
		if (!reuse_source(sna,
				  src, &tmp->src, src_x, src_y,
				  mask, &tmp->mask, mask_x, mask_y)) {
			tmp->mask.u.gen3.type = SHADER_TEXTURE;
			DBG(("%s: preparing mask\n", __FUNCTION__));
			switch (gen3_composite_picture(sna, mask, tmp, &tmp->mask,
						       mask_x, mask_y,
						       width,  height,
						       dst_x,  dst_y,
						       dst->polyMode == PolyModePrecise)) {
			case -1:
				goto cleanup_src;
			case 0:
				tmp->mask.u.gen3.type = SHADER_ZERO;
				break;
			case 1:
				gen3_composite_channel_convert(&tmp->mask);
				break;
			}
		}
		DBG(("%s: mask type=%d\n", __FUNCTION__, tmp->mask.u.gen3.type));
		if (tmp->mask.u.gen3.type == SHADER_ZERO) {
			if (tmp->src.bo) {
				kgem_bo_destroy(&sna->kgem,
						tmp->src.bo);
				tmp->src.bo = NULL;
			}
			tmp->src.u.gen3.type = SHADER_ZERO;
			tmp->mask.u.gen3.type = SHADER_NONE;
		}

		if (tmp->mask.u.gen3.type != SHADER_NONE) {
			if (mask->componentAlpha && PICT_FORMAT_RGB(mask->format)) {
				/* Check if it's component alpha that relies on a source alpha
				 * and on the source value.  We can only get one of those
				 * into the single source value that we get to blend with.
				 */
				DBG(("%s: component-alpha mask: %d\n",
				     __FUNCTION__, tmp->mask.u.gen3.type));
				tmp->has_component_alpha = true;
				if (tmp->mask.u.gen3.type == SHADER_WHITE) {
					tmp->mask.u.gen3.type = SHADER_NONE;
					tmp->has_component_alpha = false;
				} else if (gen3_blend_op[op].src_alpha &&
					   gen3_blend_op[op].src_blend != BLENDFACT_ZERO) {
					if (op != PictOpOver)
						goto cleanup_mask;

					tmp->need_magic_ca_pass = true;
					tmp->op = PictOpOutReverse;
				}
			} else {
				if (tmp->mask.is_opaque) {
					tmp->mask.u.gen3.type = SHADER_NONE;
				} else if (is_constant_ps(tmp->src.u.gen3.type) &&
					   is_constant_ps(tmp->mask.u.gen3.type)) {
					uint32_t v;

					v = multa(tmp->src.u.gen3.mode,
						  tmp->mask.u.gen3.mode,
						  24);
					v |= multa(tmp->src.u.gen3.mode,
						   tmp->mask.u.gen3.mode,
						   16);
					v |= multa(tmp->src.u.gen3.mode,
						   tmp->mask.u.gen3.mode,
						   8);
					v |= multa(tmp->src.u.gen3.mode,
						   tmp->mask.u.gen3.mode,
						   0);

					DBG(("%s: combining constant source/mask: %x x %x -> %x\n",
					     __FUNCTION__,
					     tmp->src.u.gen3.mode,
					     tmp->mask.u.gen3.mode,
					     v));

					tmp->src.u.gen3.type = SHADER_CONSTANT;
					tmp->src.u.gen3.mode = v;
					tmp->src.is_opaque = false;

					tmp->mask.u.gen3.type = SHADER_NONE;
				}
			}
		}
	}
	DBG(("%s: final src/mask type=%d/%d, affine=%d/%d\n", __FUNCTION__,
	     tmp->src.u.gen3.type, tmp->mask.u.gen3.type,
	     tmp->src.is_affine, tmp->mask.is_affine));

	tmp->prim_emit = gen3_emit_composite_primitive;
	if (is_constant_ps(tmp->mask.u.gen3.type)) {
		switch (tmp->src.u.gen3.type) {
		case SHADER_NONE:
		case SHADER_ZERO:
		case SHADER_BLACK:
		case SHADER_WHITE:
		case SHADER_CONSTANT:
#if defined(sse2) && !defined(__x86_64__)
			if (sna->cpu_features & SSE2) {
				tmp->prim_emit = gen3_emit_composite_primitive_constant__sse2;
				tmp->emit_boxes = gen3_emit_composite_boxes_constant__sse2;
			} else
#endif
			{
				tmp->prim_emit = gen3_emit_composite_primitive_constant;
				tmp->emit_boxes = gen3_emit_composite_boxes_constant;
			}

			break;
		case SHADER_LINEAR:
		case SHADER_RADIAL:
			if (tmp->src.transform == NULL) {
#if defined(sse2) && !defined(__x86_64__)
				if (sna->cpu_features & SSE2) {
					tmp->prim_emit = gen3_emit_composite_primitive_identity_gradient__sse2;
					tmp->emit_boxes = gen3_emit_composite_boxes_identity_gradient__sse2;
				} else
#endif
				{
					tmp->prim_emit = gen3_emit_composite_primitive_identity_gradient;
					tmp->emit_boxes = gen3_emit_composite_boxes_identity_gradient;
				}
			} else if (tmp->src.is_affine) {
				tmp->src.scale[1] = tmp->src.scale[0] = 1. / tmp->src.transform->matrix[2][2];
#if defined(sse2) && !defined(__x86_64__)
				if (sna->cpu_features & SSE2) {
					tmp->prim_emit = gen3_emit_composite_primitive_affine_gradient__sse2;
					tmp->emit_boxes = gen3_emit_composite_boxes_affine_gradient__sse2;
				} else
#endif
				{
					tmp->prim_emit = gen3_emit_composite_primitive_affine_gradient;
					tmp->emit_boxes = gen3_emit_composite_boxes_affine_gradient;
				}
			}
			break;
		case SHADER_TEXTURE:
			if (tmp->src.transform == NULL) {
				if ((tmp->src.offset[0]|tmp->src.offset[1]|tmp->dst.x|tmp->dst.y) == 0) {
#if defined(sse2) && !defined(__x86_64__)
					if (sna->cpu_features & SSE2) {
						tmp->prim_emit = gen3_emit_composite_primitive_identity_source_no_offset__sse2;
						tmp->emit_boxes = gen3_emit_composite_boxes_identity_source_no_offset__sse2;
					} else
#endif
					{
						tmp->prim_emit = gen3_emit_composite_primitive_identity_source_no_offset;
						tmp->emit_boxes = gen3_emit_composite_boxes_identity_source_no_offset;
					}
				} else {
#if defined(sse2) && !defined(__x86_64__)
					if (sna->cpu_features & SSE2) {
						tmp->prim_emit = gen3_emit_composite_primitive_identity_source__sse2;
						tmp->emit_boxes = gen3_emit_composite_boxes_identity_source__sse2;
					} else
#endif
					{
						tmp->prim_emit = gen3_emit_composite_primitive_identity_source;
						tmp->emit_boxes = gen3_emit_composite_boxes_identity_source;
					}
				}
			} else if (tmp->src.is_affine) {
				tmp->src.scale[0] /= tmp->src.transform->matrix[2][2];
				tmp->src.scale[1] /= tmp->src.transform->matrix[2][2];
#if defined(sse2) && !defined(__x86_64__)
				if (sna->cpu_features & SSE2) {
					tmp->prim_emit = gen3_emit_composite_primitive_affine_source__sse2;
					tmp->emit_boxes = gen3_emit_composite_boxes_affine_source__sse2;
				} else
#endif
				{
					tmp->prim_emit = gen3_emit_composite_primitive_affine_source;
					tmp->emit_boxes = gen3_emit_composite_boxes_affine_source;
				}
			}
			break;
		}
	} else if (tmp->mask.u.gen3.type == SHADER_TEXTURE) {
		if (tmp->mask.transform == NULL) {
			if (is_constant_ps(tmp->src.u.gen3.type)) {
				if ((tmp->mask.offset[0]|tmp->mask.offset[1]|tmp->dst.x|tmp->dst.y) == 0) {
#if defined(sse2) && !defined(__x86_64__)
					if (sna->cpu_features & SSE2) {
						tmp->prim_emit = gen3_emit_composite_primitive_constant_identity_mask_no_offset__sse2;
					} else
#endif
					{
						tmp->prim_emit = gen3_emit_composite_primitive_constant_identity_mask_no_offset;
					}
				} else {
#if defined(sse2) && !defined(__x86_64__)
					if (sna->cpu_features & SSE2) {
						tmp->prim_emit = gen3_emit_composite_primitive_constant_identity_mask__sse2;
					} else
#endif
					{
						tmp->prim_emit = gen3_emit_composite_primitive_constant_identity_mask;
					}
				}
			} else if (tmp->src.transform == NULL) {
#if defined(sse2) && !defined(__x86_64__)
				if (sna->cpu_features & SSE2) {
					tmp->prim_emit = gen3_emit_composite_primitive_identity_source_mask__sse2;
				} else
#endif
				{
					tmp->prim_emit = gen3_emit_composite_primitive_identity_source_mask;
				}
			} else if (tmp->src.is_affine) {
				tmp->src.scale[0] /= tmp->src.transform->matrix[2][2];
				tmp->src.scale[1] /= tmp->src.transform->matrix[2][2];
#if defined(sse2) && !defined(__x86_64__)
				if (sna->cpu_features & SSE2) {
					tmp->prim_emit = gen3_emit_composite_primitive_affine_source_mask__sse2;
				} else
#endif
				{
					tmp->prim_emit = gen3_emit_composite_primitive_affine_source_mask;
				}
			}
		}
	}

	tmp->floats_per_vertex = 2;
	if (!is_constant_ps(tmp->src.u.gen3.type))
		tmp->floats_per_vertex += tmp->src.is_affine ? 2 : 4;
	if (!is_constant_ps(tmp->mask.u.gen3.type))
		tmp->floats_per_vertex += tmp->mask.is_affine ? 2 : 4;
	DBG(("%s: floats_per_vertex = 2 + %d + %d = %d [specialised emitter? %d]\n", __FUNCTION__,
	     !is_constant_ps(tmp->src.u.gen3.type) ? tmp->src.is_affine ? 2 : 4 : 0,
	     !is_constant_ps(tmp->mask.u.gen3.type) ? tmp->mask.is_affine ? 2 : 4 : 0,
	     tmp->floats_per_vertex,
	     tmp->prim_emit != gen3_emit_composite_primitive));
	tmp->floats_per_rect = 3 * tmp->floats_per_vertex;

	tmp->blt   = gen3_render_composite_blt;
	tmp->box   = gen3_render_composite_box;
	tmp->boxes = gen3_render_composite_boxes__blt;
	if (tmp->emit_boxes) {
		tmp->boxes = gen3_render_composite_boxes;
		tmp->thread_boxes = gen3_render_composite_boxes__thread;
	}
	tmp->done  = gen3_render_composite_done;

	if (!kgem_check_bo(&sna->kgem,
			   tmp->dst.bo, tmp->src.bo, tmp->mask.bo,
			   NULL)) {
		kgem_submit(&sna->kgem);
		if (!kgem_check_bo(&sna->kgem,
				   tmp->dst.bo, tmp->src.bo, tmp->mask.bo,
				   NULL))
			goto cleanup_mask;
	}

	gen3_align_vertex(sna, tmp);
	gen3_emit_composite_state(sna, tmp);
	return true;

cleanup_mask:
	if (tmp->mask.bo)
		kgem_bo_destroy(&sna->kgem, tmp->mask.bo);
cleanup_src:
	if (tmp->src.bo)
		kgem_bo_destroy(&sna->kgem, tmp->src.bo);
cleanup_dst:
	if (tmp->redirect.real_bo)
		kgem_bo_destroy(&sna->kgem, tmp->dst.bo);
fallback:
	return (mask == NULL &&
		sna_blt_composite(sna,
				  op, src, dst,
				  src_x, src_y,
				  dst_x, dst_y,
				  width, height,
				  tmp, true));
}

static void
gen3_emit_composite_spans_vertex(struct sna *sna,
				 const struct sna_composite_spans_op *op,
				 int16_t x, int16_t y,
				 float opacity)
{
	gen3_emit_composite_dstcoord(sna, x + op->base.dst.x, y + op->base.dst.y);
	gen3_emit_composite_texcoord(sna, &op->base.src, x, y);
	OUT_VERTEX(opacity);
}

fastcall static void
gen3_emit_composite_spans_primitive_zero(struct sna *sna,
					 const struct sna_composite_spans_op *op,
					 const BoxRec *box,
					 float opacity)
{
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 6;

	v[0] = op->base.dst.x + box->x2;
	v[1] = op->base.dst.y + box->y2;

	v[2] = op->base.dst.x + box->x1;
	v[3] = v[1];

	v[4] = v[2];
	v[5] = op->base.dst.x + box->y1;
}

fastcall static void
gen3_emit_composite_spans_primitive_zero__boxes(const struct sna_composite_spans_op *op,
						const struct sna_opacity_box *b,
						int nbox, float *v)
{
	do {
		v[0] = op->base.dst.x + b->box.x2;
		v[1] = op->base.dst.y + b->box.y2;

		v[2] = op->base.dst.x + b->box.x1;
		v[3] = v[1];

		v[4] = v[2];
		v[5] = op->base.dst.x + b->box.y1;

		v += 6;
		b++;
	} while (--nbox);
}

fastcall static void
gen3_emit_composite_spans_primitive_zero_no_offset(struct sna *sna,
						   const struct sna_composite_spans_op *op,
						   const BoxRec *box,
						   float opacity)
{
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 6;

	v[0] = box->x2;
	v[3] = v[1] = box->y2;
	v[4] = v[2] = box->x1;
	v[5] = box->y1;
}

fastcall static void
gen3_emit_composite_spans_primitive_zero_no_offset__boxes(const struct sna_composite_spans_op *op,
							  const struct sna_opacity_box *b,
							  int nbox, float *v)
{
	do {
		v[0] = b->box.x2;
		v[3] = v[1] = b->box.y2;
		v[4] = v[2] = b->box.x1;
		v[5] = b->box.y1;

		b++;
		v += 6;
	} while (--nbox);
}

fastcall static void
gen3_emit_composite_spans_primitive_constant(struct sna *sna,
					     const struct sna_composite_spans_op *op,
					     const BoxRec *box,
					     float opacity)
{
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 9;

	v[0] = op->base.dst.x + box->x2;
	v[6] = v[3] = op->base.dst.x + box->x1;
	v[4] = v[1] = op->base.dst.y + box->y2;
	v[7] = op->base.dst.y + box->y1;
	v[8] = v[5] = v[2] = opacity;
}

fastcall static void
gen3_emit_composite_spans_primitive_constant__boxes(const struct sna_composite_spans_op *op,
						    const struct sna_opacity_box *b,
						    int nbox,
						    float *v)
{
	do {
		v[0] = op->base.dst.x + b->box.x2;
		v[6] = v[3] = op->base.dst.x + b->box.x1;
		v[4] = v[1] = op->base.dst.y + b->box.y2;
		v[7] = op->base.dst.y + b->box.y1;
		v[8] = v[5] = v[2] = b->alpha;

		v += 9;
		b++;
	} while (--nbox);
}

fastcall static void
gen3_emit_composite_spans_primitive_constant_no_offset(struct sna *sna,
						       const struct sna_composite_spans_op *op,
						       const BoxRec *box,
						       float opacity)
{
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 9;

	v[0] = box->x2;
	v[6] = v[3] = box->x1;
	v[4] = v[1] = box->y2;
	v[7] = box->y1;
	v[8] = v[5] = v[2] = opacity;
}

fastcall static void
gen3_emit_composite_spans_primitive_constant_no_offset__boxes(const struct sna_composite_spans_op *op,
							      const struct sna_opacity_box *b,
							      int nbox, float *v)
{
	do {
		v[0] = b->box.x2;
		v[6] = v[3] = b->box.x1;
		v[4] = v[1] = b->box.y2;
		v[7] = b->box.y1;
		v[8] = v[5] = v[2] = b->alpha;

		v += 9;
		b++;
	} while (--nbox);
}

fastcall static void
gen3_emit_composite_spans_primitive_identity_source(struct sna *sna,
						    const struct sna_composite_spans_op *op,
						    const BoxRec *box,
						    float opacity)
{
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	v[0] = op->base.dst.x + box->x2;
	v[1] = op->base.dst.y + box->y2;
	v[2] = (op->base.src.offset[0] + box->x2) * op->base.src.scale[0];
	v[3] = (op->base.src.offset[1] + box->y2) * op->base.src.scale[1];
	v[4] = opacity;

	v[5] = op->base.dst.x + box->x1;
	v[6] = v[1];
	v[7] = (op->base.src.offset[0] + box->x1) * op->base.src.scale[0];
	v[8] = v[3];
	v[9] = opacity;

	v[10] = v[5];
	v[11] = op->base.dst.y + box->y1;
	v[12] = v[7];
	v[13] = (op->base.src.offset[1] + box->y1) * op->base.src.scale[1];
	v[14] = opacity;
}

fastcall static void
gen3_emit_composite_spans_primitive_identity_source__boxes(const struct sna_composite_spans_op *op,
							   const struct sna_opacity_box *b,
							   int nbox,
							   float *v)
{
	do {
		v[0] = op->base.dst.x + b->box.x2;
		v[1] = op->base.dst.y + b->box.y2;
		v[2] = (op->base.src.offset[0] + b->box.x2) * op->base.src.scale[0];
		v[3] = (op->base.src.offset[1] + b->box.y2) * op->base.src.scale[1];
		v[4] = b->alpha;

		v[5] = op->base.dst.x + b->box.x1;
		v[6] = v[1];
		v[7] = (op->base.src.offset[0] + b->box.x1) * op->base.src.scale[0];
		v[8] = v[3];
		v[9] = b->alpha;

		v[10] = v[5];
		v[11] = op->base.dst.y + b->box.y1;
		v[12] = v[7];
		v[13] = (op->base.src.offset[1] + b->box.y1) * op->base.src.scale[1];
		v[14] = b->alpha;

		v += 15;
		b++;
	} while (--nbox);
}

fastcall static void
gen3_emit_composite_spans_primitive_affine_source(struct sna *sna,
						  const struct sna_composite_spans_op *op,
						  const BoxRec *box,
						  float opacity)
{
	PictTransform *transform = op->base.src.transform;
	float *v;

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	v[0]  = op->base.dst.x + box->x2;
	v[6]  = v[1] = op->base.dst.y + box->y2;
	v[10] = v[5] = op->base.dst.x + box->x1;
	v[11] = op->base.dst.y + box->y1;
	v[14] = v[9] = v[4]  = opacity;

	_sna_get_transformed_scaled((int)op->base.src.offset[0] + box->x2,
				    (int)op->base.src.offset[1] + box->y2,
				    transform, op->base.src.scale,
				    &v[2], &v[3]);

	_sna_get_transformed_scaled((int)op->base.src.offset[0] + box->x1,
				    (int)op->base.src.offset[1] + box->y2,
				    transform, op->base.src.scale,
				    &v[7], &v[8]);

	_sna_get_transformed_scaled((int)op->base.src.offset[0] + box->x1,
				    (int)op->base.src.offset[1] + box->y1,
				    transform, op->base.src.scale,
				    &v[12], &v[13]);
}

fastcall static void
gen3_emit_composite_spans_primitive_affine_source__boxes(const struct sna_composite_spans_op *op,
							 const struct sna_opacity_box *b,
							 int nbox,
							 float *v)
{
	PictTransform *transform = op->base.src.transform;

	do {
		v[0]  = op->base.dst.x + b->box.x2;
		v[6]  = v[1] = op->base.dst.y + b->box.y2;
		v[10] = v[5] = op->base.dst.x + b->box.x1;
		v[11] = op->base.dst.y + b->box.y1;
		v[14] = v[9] = v[4]  = b->alpha;

		_sna_get_transformed_scaled((int)op->base.src.offset[0] + b->box.x2,
					    (int)op->base.src.offset[1] + b->box.y2,
					    transform, op->base.src.scale,
					    &v[2], &v[3]);

		_sna_get_transformed_scaled((int)op->base.src.offset[0] + b->box.x1,
					    (int)op->base.src.offset[1] + b->box.y2,
					    transform, op->base.src.scale,
					    &v[7], &v[8]);

		_sna_get_transformed_scaled((int)op->base.src.offset[0] + b->box.x1,
					    (int)op->base.src.offset[1] + b->box.y1,
					    transform, op->base.src.scale,
					    &v[12], &v[13]);
		v += 15;
		b++;
	} while (--nbox);
}

fastcall static void
gen3_emit_composite_spans_primitive_identity_gradient(struct sna *sna,
						      const struct sna_composite_spans_op *op,
						      const BoxRec *box,
						      float opacity)
{
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	v[0] = op->base.dst.x + box->x2;
	v[1] = op->base.dst.y + box->y2;
	v[2] = op->base.src.offset[0] + box->x2;
	v[3] = op->base.src.offset[1] + box->y2;
	v[4] = opacity;

	v[5] = op->base.dst.x + box->x1;
	v[6] = v[1];
	v[7] = op->base.src.offset[0] + box->x1;
	v[8] = v[3];
	v[9] = opacity;

	v[10] = v[5];
	v[11] = op->base.dst.y + box->y1;
	v[12] = v[7];
	v[13] = op->base.src.offset[1] + box->y1;
	v[14] = opacity;
}

fastcall static void
gen3_emit_composite_spans_primitive_identity_gradient__boxes(const struct sna_composite_spans_op *op,
							     const struct sna_opacity_box *b,
							     int nbox,
							     float *v)
{
	do {
		v[0] = op->base.dst.x + b->box.x2;
		v[1] = op->base.dst.y + b->box.y2;
		v[2] = op->base.src.offset[0] + b->box.x2;
		v[3] = op->base.src.offset[1] + b->box.y2;
		v[4] = b->alpha;

		v[5] = op->base.dst.x + b->box.x1;
		v[6] = v[1];
		v[7] = op->base.src.offset[0] + b->box.x1;
		v[8] = v[3];
		v[9] = b->alpha;

		v[10] = v[5];
		v[11] = op->base.dst.y + b->box.y1;
		v[12] = v[7];
		v[13] = op->base.src.offset[1] + b->box.y1;
		v[14] = b->alpha;

		v += 15;
		b++;
	} while (--nbox);
}

#if defined(sse2) && !defined(__x86_64__)
sse2 fastcall static void
gen3_emit_composite_spans_primitive_constant__sse2(struct sna *sna,
						   const struct sna_composite_spans_op *op,
						   const BoxRec *box,
						   float opacity)
{
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 9;

	v[0] = op->base.dst.x + box->x2;
	v[6] = v[3] = op->base.dst.x + box->x1;
	v[4] = v[1] = op->base.dst.y + box->y2;
	v[7] = op->base.dst.y + box->y1;
	v[8] = v[5] = v[2] = opacity;
}

sse2 fastcall static void
gen3_emit_composite_spans_primitive_constant__sse2__boxes(const struct sna_composite_spans_op *op,
							  const struct sna_opacity_box *b,
							  int nbox,
							  float *v)
{
	do {
		v[0] = op->base.dst.x + b->box.x2;
		v[6] = v[3] = op->base.dst.x + b->box.x1;
		v[4] = v[1] = op->base.dst.y + b->box.y2;
		v[7] = op->base.dst.y + b->box.y1;
		v[8] = v[5] = v[2] = b->alpha;

		v += 9;
		b++;
	} while (--nbox);
}

sse2 fastcall static void
gen3_render_composite_spans_constant_box__sse2(struct sna *sna,
					       const struct sna_composite_spans_op *op,
					       const BoxRec *box, float opacity)
{
	float *v;
	DBG(("%s: src=+(%d, %d), opacity=%f, dst=+(%d, %d), box=(%d, %d) x (%d, %d)\n",
	     __FUNCTION__,
	     op->base.src.offset[0], op->base.src.offset[1],
	     opacity,
	     op->base.dst.x, op->base.dst.y,
	     box->x1, box->y1,
	     box->x2 - box->x1,
	     box->y2 - box->y1));

	gen3_get_rectangles(sna, &op->base, 1);

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 9;

	v[0] = box->x2;
	v[6] = v[3] = box->x1;
	v[4] = v[1] = box->y2;
	v[7] = box->y1;
	v[8] = v[5] = v[2] = opacity;
}

sse2 fastcall static void
gen3_render_composite_spans_constant_thread__sse2__boxes(struct sna *sna,
							 const struct sna_composite_spans_op *op,
							 const struct sna_opacity_box *box,
							 int nbox)
{
	DBG(("%s: nbox=%d, src=+(%d, %d), dst=+(%d, %d)\n",
	     __FUNCTION__, nbox,
	     op->base.src.offset[0], op->base.src.offset[1],
	     op->base.dst.x, op->base.dst.y));

	sna_vertex_lock(&sna->render);
	do {
		int nbox_this_time;
		float *v;

		nbox_this_time = gen3_get_rectangles(sna, &op->base, nbox);
		assert(nbox_this_time);
		nbox -= nbox_this_time;

		v = sna->render.vertices + sna->render.vertex_used;
		sna->render.vertex_used += nbox_this_time * 9;

		sna_vertex_acquire__locked(&sna->render);
		sna_vertex_unlock(&sna->render);

		do {
			v[0] = box->box.x2;
			v[6] = v[3] = box->box.x1;
			v[4] = v[1] = box->box.y2;
			v[7] = box->box.y1;
			v[8] = v[5] = v[2] = box->alpha;
			v += 9;
			box++;
		} while (--nbox_this_time);

		sna_vertex_lock(&sna->render);
		sna_vertex_release__locked(&sna->render);
	} while (nbox);
	sna_vertex_unlock(&sna->render);
}

sse2 fastcall static void
gen3_emit_composite_spans_primitive_constant__sse2__no_offset(struct sna *sna,
							      const struct sna_composite_spans_op *op,
							      const BoxRec *box,
							      float opacity)
{
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 9;

	v[0] = box->x2;
	v[6] = v[3] = box->x1;
	v[4] = v[1] = box->y2;
	v[7] = box->y1;
	v[8] = v[5] = v[2] = opacity;
}

sse2 fastcall static void
gen3_emit_composite_spans_primitive_constant__sse2__no_offset__boxes(const struct sna_composite_spans_op *op,
								     const struct sna_opacity_box *b,
								     int nbox, float *v)
{
	do {
		v[0] = b->box.x2;
		v[6] = v[3] = b->box.x1;
		v[4] = v[1] = b->box.y2;
		v[7] = b->box.y1;
		v[8] = v[5] = v[2] = b->alpha;

		v += 9;
		b++;
	} while (--nbox);
}

sse2 fastcall static void
gen3_emit_composite_spans_primitive_identity_source__sse2(struct sna *sna,
							  const struct sna_composite_spans_op *op,
							  const BoxRec *box,
							  float opacity)
{
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	v[0] = op->base.dst.x + box->x2;
	v[1] = op->base.dst.y + box->y2;
	v[2] = (op->base.src.offset[0] + box->x2) * op->base.src.scale[0];
	v[3] = (op->base.src.offset[1] + box->y2) * op->base.src.scale[1];
	v[4] = opacity;

	v[5] = op->base.dst.x + box->x1;
	v[6] = v[1];
	v[7] = (op->base.src.offset[0] + box->x1) * op->base.src.scale[0];
	v[8] = v[3];
	v[9] = opacity;

	v[10] = v[5];
	v[11] = op->base.dst.y + box->y1;
	v[12] = v[7];
	v[13] = (op->base.src.offset[1] + box->y1) * op->base.src.scale[1];
	v[14] = opacity;
}

sse2 fastcall static void
gen3_emit_composite_spans_primitive_identity_source__sse2__boxes(const struct sna_composite_spans_op *op,
								 const struct sna_opacity_box *b,
								 int nbox,
								 float *v)
{
	do {
		v[0] = op->base.dst.x + b->box.x2;
		v[1] = op->base.dst.y + b->box.y2;
		v[2] = (op->base.src.offset[0] + b->box.x2) * op->base.src.scale[0];
		v[3] = (op->base.src.offset[1] + b->box.y2) * op->base.src.scale[1];
		v[4] = b->alpha;

		v[5] = op->base.dst.x + b->box.x1;
		v[6] = v[1];
		v[7] = (op->base.src.offset[0] + b->box.x1) * op->base.src.scale[0];
		v[8] = v[3];
		v[9] = b->alpha;

		v[10] = v[5];
		v[11] = op->base.dst.y + b->box.y1;
		v[12] = v[7];
		v[13] = (op->base.src.offset[1] + b->box.y1) * op->base.src.scale[1];
		v[14] = b->alpha;

		v += 15;
		b++;
	} while (--nbox);
}
sse2 fastcall static void
gen3_emit_composite_spans_primitive_affine_source__sse2(struct sna *sna,
							const struct sna_composite_spans_op *op,
							const BoxRec *box,
							float opacity)
{
	PictTransform *transform = op->base.src.transform;
	float *v;

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	v[0]  = op->base.dst.x + box->x2;
	v[6]  = v[1] = op->base.dst.y + box->y2;
	v[10] = v[5] = op->base.dst.x + box->x1;
	v[11] = op->base.dst.y + box->y1;
	v[14] = v[9] = v[4]  = opacity;

	_sna_get_transformed_scaled((int)op->base.src.offset[0] + box->x2,
				    (int)op->base.src.offset[1] + box->y2,
				    transform, op->base.src.scale,
				    &v[2], &v[3]);

	_sna_get_transformed_scaled((int)op->base.src.offset[0] + box->x1,
				    (int)op->base.src.offset[1] + box->y2,
				    transform, op->base.src.scale,
				    &v[7], &v[8]);

	_sna_get_transformed_scaled((int)op->base.src.offset[0] + box->x1,
				    (int)op->base.src.offset[1] + box->y1,
				    transform, op->base.src.scale,
				    &v[12], &v[13]);
}

sse2 fastcall static void
gen3_emit_composite_spans_primitive_affine_source__sse2__boxes(const struct sna_composite_spans_op *op,
							       const struct sna_opacity_box *b,
							       int nbox,
							       float *v)
{
	PictTransform *transform = op->base.src.transform;

	do {
		v[0]  = op->base.dst.x + b->box.x2;
		v[6]  = v[1] = op->base.dst.y + b->box.y2;
		v[10] = v[5] = op->base.dst.x + b->box.x1;
		v[11] = op->base.dst.y + b->box.y1;
		v[14] = v[9] = v[4]  = b->alpha;

		_sna_get_transformed_scaled((int)op->base.src.offset[0] + b->box.x2,
					    (int)op->base.src.offset[1] + b->box.y2,
					    transform, op->base.src.scale,
					    &v[2], &v[3]);

		_sna_get_transformed_scaled((int)op->base.src.offset[0] + b->box.x1,
					    (int)op->base.src.offset[1] + b->box.y2,
					    transform, op->base.src.scale,
					    &v[7], &v[8]);

		_sna_get_transformed_scaled((int)op->base.src.offset[0] + b->box.x1,
					    (int)op->base.src.offset[1] + b->box.y1,
					    transform, op->base.src.scale,
					    &v[12], &v[13]);
		v += 15;
		b++;
	} while (--nbox);
}

sse2 fastcall static void
gen3_emit_composite_spans_primitive_identity_gradient__sse2(struct sna *sna,
							    const struct sna_composite_spans_op *op,
							    const BoxRec *box,
							    float opacity)
{
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	v[0] = op->base.dst.x + box->x2;
	v[1] = op->base.dst.y + box->y2;
	v[2] = op->base.src.offset[0] + box->x2;
	v[3] = op->base.src.offset[1] + box->y2;
	v[4] = opacity;

	v[5] = op->base.dst.x + box->x1;
	v[6] = v[1];
	v[7] = op->base.src.offset[0] + box->x1;
	v[8] = v[3];
	v[9] = opacity;

	v[10] = v[5];
	v[11] = op->base.dst.y + box->y1;
	v[12] = v[7];
	v[13] = op->base.src.offset[1] + box->y1;
	v[14] = opacity;
}

sse2 fastcall static void
gen3_emit_composite_spans_primitive_identity_gradient__sse2__boxes(const struct sna_composite_spans_op *op,
								   const struct sna_opacity_box *b,
								   int nbox,
								   float *v)
{
	do {
		v[0] = op->base.dst.x + b->box.x2;
		v[1] = op->base.dst.y + b->box.y2;
		v[2] = op->base.src.offset[0] + b->box.x2;
		v[3] = op->base.src.offset[1] + b->box.y2;
		v[4] = b->alpha;

		v[5] = op->base.dst.x + b->box.x1;
		v[6] = v[1];
		v[7] = op->base.src.offset[0] + b->box.x1;
		v[8] = v[3];
		v[9] = b->alpha;

		v[10] = v[5];
		v[11] = op->base.dst.y + b->box.y1;
		v[12] = v[7];
		v[13] = op->base.src.offset[1] + b->box.y1;
		v[14] = b->alpha;

		v += 15;
		b++;
	} while (--nbox);
}

sse2 fastcall static void
gen3_emit_composite_spans_primitive_affine_gradient__sse2(struct sna *sna,
							  const struct sna_composite_spans_op *op,
							  const BoxRec *box,
							  float opacity)
{
	PictTransform *transform = op->base.src.transform;
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	v[0] = op->base.dst.x + box->x2;
	v[1] = op->base.dst.y + box->y2;
	_sna_get_transformed_scaled(op->base.src.offset[0] + box->x2,
				    op->base.src.offset[1] + box->y2,
				    transform, op->base.src.scale,
				    &v[2], &v[3]);
	v[4] = opacity;

	v[5] = op->base.dst.x + box->x1;
	v[6] = v[1];
	_sna_get_transformed_scaled(op->base.src.offset[0] + box->x1,
				    op->base.src.offset[1] + box->y2,
				    transform, op->base.src.scale,
				    &v[7], &v[8]);
	v[9] = opacity;

	v[10] = v[5];
	v[11] = op->base.dst.y + box->y1;
	_sna_get_transformed_scaled(op->base.src.offset[0] + box->x1,
				    op->base.src.offset[1] + box->y1,
				    transform, op->base.src.scale,
				    &v[12], &v[13]);
	v[14] = opacity;
}

sse2 fastcall static void
gen3_emit_composite_spans_primitive_affine_gradient__sse2__boxes(const struct sna_composite_spans_op *op,
								 const struct sna_opacity_box *b,
								 int nbox,
								 float *v)
{
	PictTransform *transform = op->base.src.transform;

	do {
		v[0] = op->base.dst.x + b->box.x2;
		v[1] = op->base.dst.y + b->box.y2;
		_sna_get_transformed_scaled(op->base.src.offset[0] + b->box.x2,
					    op->base.src.offset[1] + b->box.y2,
					    transform, op->base.src.scale,
					    &v[2], &v[3]);
		v[4] = b->alpha;

		v[5] = op->base.dst.x + b->box.x1;
		v[6] = v[1];
		_sna_get_transformed_scaled(op->base.src.offset[0] + b->box.x1,
					    op->base.src.offset[1] + b->box.y2,
					    transform, op->base.src.scale,
					    &v[7], &v[8]);
		v[9] = b->alpha;

		v[10] = v[5];
		v[11] = op->base.dst.y + b->box.y1;
		_sna_get_transformed_scaled(op->base.src.offset[0] + b->box.x1,
					    op->base.src.offset[1] + b->box.y1,
					    transform, op->base.src.scale,
					    &v[12], &v[13]);
		v[14] = b->alpha;
		v += 15;
		b++;
	} while (--nbox);
}
#endif

fastcall static void
gen3_emit_composite_spans_primitive_affine_gradient(struct sna *sna,
						    const struct sna_composite_spans_op *op,
						    const BoxRec *box,
						    float opacity)
{
	PictTransform *transform = op->base.src.transform;
	float *v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	v[0] = op->base.dst.x + box->x2;
	v[1] = op->base.dst.y + box->y2;
	_sna_get_transformed_scaled(op->base.src.offset[0] + box->x2,
				    op->base.src.offset[1] + box->y2,
				    transform, op->base.src.scale,
				    &v[2], &v[3]);
	v[4] = opacity;

	v[5] = op->base.dst.x + box->x1;
	v[6] = v[1];
	_sna_get_transformed_scaled(op->base.src.offset[0] + box->x1,
				    op->base.src.offset[1] + box->y2,
				    transform, op->base.src.scale,
				    &v[7], &v[8]);
	v[9] = opacity;

	v[10] = v[5];
	v[11] = op->base.dst.y + box->y1;
	_sna_get_transformed_scaled(op->base.src.offset[0] + box->x1,
				    op->base.src.offset[1] + box->y1,
				    transform, op->base.src.scale,
				    &v[12], &v[13]);
	v[14] = opacity;
}

fastcall static void
gen3_emit_composite_spans_primitive_affine_gradient__boxes(const struct sna_composite_spans_op *op,
							   const struct sna_opacity_box *b,
							   int nbox,
							   float *v)
{
	PictTransform *transform = op->base.src.transform;

	do {
		v[0] = op->base.dst.x + b->box.x2;
		v[1] = op->base.dst.y + b->box.y2;
		_sna_get_transformed_scaled(op->base.src.offset[0] + b->box.x2,
					    op->base.src.offset[1] + b->box.y2,
					    transform, op->base.src.scale,
					    &v[2], &v[3]);
		v[4] = b->alpha;

		v[5] = op->base.dst.x + b->box.x1;
		v[6] = v[1];
		_sna_get_transformed_scaled(op->base.src.offset[0] + b->box.x1,
					    op->base.src.offset[1] + b->box.y2,
					    transform, op->base.src.scale,
					    &v[7], &v[8]);
		v[9] = b->alpha;

		v[10] = v[5];
		v[11] = op->base.dst.y + b->box.y1;
		_sna_get_transformed_scaled(op->base.src.offset[0] + b->box.x1,
					    op->base.src.offset[1] + b->box.y1,
					    transform, op->base.src.scale,
					    &v[12], &v[13]);
		v[14] = b->alpha;
		v += 15;
		b++;
	} while (--nbox);
}

fastcall static void
gen3_emit_composite_spans_primitive(struct sna *sna,
				    const struct sna_composite_spans_op *op,
				    const BoxRec *box,
				    float opacity)
{
	gen3_emit_composite_spans_vertex(sna, op,
					 box->x2, box->y2,
					 opacity);
	gen3_emit_composite_spans_vertex(sna, op,
					 box->x1, box->y2,
					 opacity);
	gen3_emit_composite_spans_vertex(sna, op,
					 box->x1, box->y1,
					 opacity);
}

fastcall static void
gen3_render_composite_spans_constant_box(struct sna *sna,
					 const struct sna_composite_spans_op *op,
					 const BoxRec *box, float opacity)
{
	float *v;
	DBG(("%s: src=+(%d, %d), opacity=%f, dst=+(%d, %d), box=(%d, %d) x (%d, %d)\n",
	     __FUNCTION__,
	     op->base.src.offset[0], op->base.src.offset[1],
	     opacity,
	     op->base.dst.x, op->base.dst.y,
	     box->x1, box->y1,
	     box->x2 - box->x1,
	     box->y2 - box->y1));

	gen3_get_rectangles(sna, &op->base, 1);

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 9;

	v[0] = box->x2;
	v[6] = v[3] = box->x1;
	v[4] = v[1] = box->y2;
	v[7] = box->y1;
	v[8] = v[5] = v[2] = opacity;
}

fastcall static void
gen3_render_composite_spans_constant_thread_boxes(struct sna *sna,
						  const struct sna_composite_spans_op *op,
						  const struct sna_opacity_box *box,
						  int nbox)
{
	DBG(("%s: nbox=%d, src=+(%d, %d), dst=+(%d, %d)\n",
	     __FUNCTION__, nbox,
	     op->base.src.offset[0], op->base.src.offset[1],
	     op->base.dst.x, op->base.dst.y));

	sna_vertex_lock(&sna->render);
	do {
		int nbox_this_time;
		float *v;

		nbox_this_time = gen3_get_rectangles(sna, &op->base, nbox);
		assert(nbox_this_time);
		nbox -= nbox_this_time;

		v = sna->render.vertices + sna->render.vertex_used;
		sna->render.vertex_used += nbox_this_time * 9;

		sna_vertex_acquire__locked(&sna->render);
		sna_vertex_unlock(&sna->render);

		do {
			v[0] = box->box.x2;
			v[6] = v[3] = box->box.x1;
			v[4] = v[1] = box->box.y2;
			v[7] = box->box.y1;
			v[8] = v[5] = v[2] = box->alpha;
			v += 9;
			box++;
		} while (--nbox_this_time);

		sna_vertex_lock(&sna->render);
		sna_vertex_release__locked(&sna->render);
	} while (nbox);
	sna_vertex_unlock(&sna->render);
}

fastcall static void
gen3_render_composite_spans_box(struct sna *sna,
				const struct sna_composite_spans_op *op,
				const BoxRec *box, float opacity)
{
	DBG(("%s: src=+(%d, %d), opacity=%f, dst=+(%d, %d), box=(%d, %d) x (%d, %d)\n",
	     __FUNCTION__,
	     op->base.src.offset[0], op->base.src.offset[1],
	     opacity,
	     op->base.dst.x, op->base.dst.y,
	     box->x1, box->y1,
	     box->x2 - box->x1,
	     box->y2 - box->y1));

	gen3_get_rectangles(sna, &op->base, 1);
	op->prim_emit(sna, op, box, opacity);
}

static void
gen3_render_composite_spans_boxes(struct sna *sna,
				  const struct sna_composite_spans_op *op,
				  const BoxRec *box, int nbox,
				  float opacity)
{
	DBG(("%s: nbox=%d, src=+(%d, %d), opacity=%f, dst=+(%d, %d)\n",
	     __FUNCTION__, nbox,
	     op->base.src.offset[0], op->base.src.offset[1],
	     opacity,
	     op->base.dst.x, op->base.dst.y));

	do {
		int nbox_this_time;

		nbox_this_time = gen3_get_rectangles(sna, &op->base, nbox);
		nbox -= nbox_this_time;

		do {
			DBG(("  %s: (%d, %d) x (%d, %d)\n", __FUNCTION__,
			     box->x1, box->y1,
			     box->x2 - box->x1,
			     box->y2 - box->y1));

			op->prim_emit(sna, op, box++, opacity);
		} while (--nbox_this_time);
	} while (nbox);
}

fastcall static void
gen3_render_composite_spans_boxes__thread(struct sna *sna,
					  const struct sna_composite_spans_op *op,
					  const struct sna_opacity_box *box,
					  int nbox)
{
	DBG(("%s: nbox=%d, src=+(%d, %d), dst=+(%d, %d)\n",
	     __FUNCTION__, nbox,
	     op->base.src.offset[0], op->base.src.offset[1],
	     op->base.dst.x, op->base.dst.y));

	sna_vertex_lock(&sna->render);
	do {
		int nbox_this_time;
		float *v;

		nbox_this_time = gen3_get_rectangles(sna, &op->base, nbox);
		assert(nbox_this_time);
		nbox -= nbox_this_time;

		v = sna->render.vertices + sna->render.vertex_used;
		sna->render.vertex_used += nbox_this_time * op->base.floats_per_rect;

		sna_vertex_acquire__locked(&sna->render);
		sna_vertex_unlock(&sna->render);

		op->emit_boxes(op, box, nbox_this_time, v);
		box += nbox_this_time;

		sna_vertex_lock(&sna->render);
		sna_vertex_release__locked(&sna->render);
	} while (nbox);
	sna_vertex_unlock(&sna->render);
}

fastcall static void
gen3_render_composite_spans_done(struct sna *sna,
				 const struct sna_composite_spans_op *op)
{
	if (sna->render.vertex_offset)
		gen3_vertex_flush(sna);

	DBG(("%s()\n", __FUNCTION__));

	if (op->base.src.bo)
		kgem_bo_destroy(&sna->kgem, op->base.src.bo);

	sna_render_composite_redirect_done(sna, &op->base);
}

static bool
gen3_check_composite_spans(struct sna *sna,
			   uint8_t op, PicturePtr src, PicturePtr dst,
			   int16_t width, int16_t height, unsigned flags)
{
	if (op >= ARRAY_SIZE(gen3_blend_op))
		return false;

	if (gen3_composite_fallback(sna, op, src, NULL, dst))
		return false;

	if (need_tiling(sna, width, height) &&
	    !is_gpu(sna, dst->pDrawable, PREFER_GPU_SPANS)) {
		DBG(("%s: fallback, tiled operation not on GPU\n",
		     __FUNCTION__));
		return false;
	}

	return true;
}

static bool
gen3_render_composite_spans(struct sna *sna,
			    uint8_t op,
			    PicturePtr src,
			    PicturePtr dst,
			    int16_t src_x,  int16_t src_y,
			    int16_t dst_x,  int16_t dst_y,
			    int16_t width,  int16_t height,
			    unsigned flags,
			    struct sna_composite_spans_op *tmp)
{
	bool no_offset;

	DBG(("%s(src=(%d, %d), dst=(%d, %d), size=(%d, %d))\n", __FUNCTION__,
	     src_x, src_y, dst_x, dst_y, width, height));

	assert(gen3_check_composite_spans(sna, op, src, dst, width, height, flags));

	if (need_tiling(sna, width, height)) {
		DBG(("%s: tiling, operation (%dx%d) too wide for pipeline\n",
		     __FUNCTION__, width, height));
		return sna_tiling_composite_spans(op, src, dst,
						  src_x, src_y, dst_x, dst_y,
						  width, height, flags, tmp);
	}

	if (!gen3_composite_set_target(sna, &tmp->base, dst,
				       dst_x, dst_y, width, height)) {
		DBG(("%s: unable to set render target\n",
		     __FUNCTION__));
		return false;
	}

	tmp->base.op = op;
	tmp->base.rb_reversed = gen3_dst_rb_reversed(tmp->base.dst.format);
	if (too_large(tmp->base.dst.width, tmp->base.dst.height) ||
	    !gen3_check_pitch_3d(tmp->base.dst.bo)) {
		if (!sna_render_composite_redirect(sna, &tmp->base,
						   dst_x, dst_y, width, height,
						   true))
			return false;
	}

	tmp->base.src.u.gen3.type = SHADER_TEXTURE;
	tmp->base.src.is_affine = true;
	DBG(("%s: preparing source\n", __FUNCTION__));
	switch (gen3_composite_picture(sna, src, &tmp->base, &tmp->base.src,
				       src_x, src_y,
				       width, height,
				       dst_x, dst_y,
				       dst->polyMode == PolyModePrecise)) {
	case -1:
		goto cleanup_dst;
	case 0:
		tmp->base.src.u.gen3.type = SHADER_ZERO;
		break;
	case 1:
		gen3_composite_channel_convert(&tmp->base.src);
		break;
	}
	DBG(("%s: source type=%d\n", __FUNCTION__, tmp->base.src.u.gen3.type));

	if (tmp->base.src.u.gen3.type != SHADER_ZERO)
		tmp->base.mask.u.gen3.type = SHADER_OPACITY;

	no_offset = tmp->base.dst.x == 0 && tmp->base.dst.y == 0;
	tmp->box   = gen3_render_composite_spans_box;
	tmp->boxes = gen3_render_composite_spans_boxes;
	tmp->thread_boxes = gen3_render_composite_spans_boxes__thread;
	tmp->done  = gen3_render_composite_spans_done;
	tmp->prim_emit = gen3_emit_composite_spans_primitive;
	switch (tmp->base.src.u.gen3.type) {
	case SHADER_NONE:
		assert(0);
	case SHADER_ZERO:
		if (no_offset) {
			tmp->prim_emit = gen3_emit_composite_spans_primitive_zero_no_offset;
			tmp->emit_boxes = gen3_emit_composite_spans_primitive_zero_no_offset__boxes;
		} else {
			tmp->prim_emit = gen3_emit_composite_spans_primitive_zero;
			tmp->emit_boxes = gen3_emit_composite_spans_primitive_zero__boxes;
		}
		break;
	case SHADER_BLACK:
	case SHADER_WHITE:
	case SHADER_CONSTANT:
		if (no_offset) {
#if defined(sse2) && !defined(__x86_64__)
			if (sna->cpu_features & SSE2) {
				tmp->box = gen3_render_composite_spans_constant_box__sse2;
				tmp->thread_boxes = gen3_render_composite_spans_constant_thread__sse2__boxes;
				tmp->prim_emit = gen3_emit_composite_spans_primitive_constant__sse2__no_offset;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_constant__sse2__no_offset__boxes;
			} else
#endif
			{
				tmp->box = gen3_render_composite_spans_constant_box;
				tmp->thread_boxes = gen3_render_composite_spans_constant_thread_boxes;
				tmp->prim_emit = gen3_emit_composite_spans_primitive_constant_no_offset;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_constant_no_offset__boxes;
			}
		} else {
#if defined(sse2) && !defined(__x86_64__)
			if (sna->cpu_features & SSE2) {
				tmp->prim_emit = gen3_emit_composite_spans_primitive_constant__sse2;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_constant__sse2__boxes;
			} else
#endif
			{
				tmp->prim_emit = gen3_emit_composite_spans_primitive_constant;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_constant__boxes;
			}
		}
		break;
	case SHADER_LINEAR:
	case SHADER_RADIAL:
		if (tmp->base.src.transform == NULL) {
#if defined(sse2) && !defined(__x86_64__)
			if (sna->cpu_features & SSE2) {
				tmp->prim_emit = gen3_emit_composite_spans_primitive_identity_gradient__sse2;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_identity_gradient__sse2__boxes;
			} else
#endif
			{
				tmp->prim_emit = gen3_emit_composite_spans_primitive_identity_gradient;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_identity_gradient__boxes;
			}
		} else if (tmp->base.src.is_affine) {
			tmp->base.src.scale[1] = tmp->base.src.scale[0] = 1. / tmp->base.src.transform->matrix[2][2];
#if defined(sse2) && !defined(__x86_64__)
			if (sna->cpu_features & SSE2) {
				tmp->prim_emit = gen3_emit_composite_spans_primitive_affine_gradient__sse2;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_affine_gradient__sse2__boxes;
			} else
#endif
			{
				tmp->prim_emit = gen3_emit_composite_spans_primitive_affine_gradient;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_affine_gradient__boxes;
			}
		}
		break;
	case SHADER_TEXTURE:
		if (tmp->base.src.transform == NULL) {
#if defined(sse2) && !defined(__x86_64__)
			if (sna->cpu_features & SSE2) {
				tmp->prim_emit = gen3_emit_composite_spans_primitive_identity_source__sse2;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_identity_source__sse2__boxes;
			} else
#endif
			{
				tmp->prim_emit = gen3_emit_composite_spans_primitive_identity_source;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_identity_source__boxes;
			}
		} else if (tmp->base.src.is_affine) {
			tmp->base.src.scale[0] /= tmp->base.src.transform->matrix[2][2];
			tmp->base.src.scale[1] /= tmp->base.src.transform->matrix[2][2];
#if defined(sse2) && !defined(__x86_64__)
			if (sna->cpu_features & SSE2) {
				tmp->prim_emit = gen3_emit_composite_spans_primitive_affine_source__sse2;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_affine_source__sse2__boxes;
			} else
#endif
			{
				tmp->prim_emit = gen3_emit_composite_spans_primitive_affine_source;
				tmp->emit_boxes = gen3_emit_composite_spans_primitive_affine_source__boxes;
			}
		}
		break;
	}
	if (tmp->emit_boxes == NULL)
		tmp->thread_boxes = NULL;

	tmp->base.mask.bo = NULL;

	tmp->base.floats_per_vertex = 2;
	if (!is_constant_ps(tmp->base.src.u.gen3.type))
		tmp->base.floats_per_vertex += tmp->base.src.is_affine ? 2 : 3;
	tmp->base.floats_per_vertex +=
		tmp->base.mask.u.gen3.type == SHADER_OPACITY;
	tmp->base.floats_per_rect = 3 * tmp->base.floats_per_vertex;

	if (!kgem_check_bo(&sna->kgem,
			   tmp->base.dst.bo, tmp->base.src.bo,
			   NULL)) {
		kgem_submit(&sna->kgem);
		if (!kgem_check_bo(&sna->kgem,
				   tmp->base.dst.bo, tmp->base.src.bo,
				   NULL))
			goto cleanup_src;
	}

	gen3_align_vertex(sna, &tmp->base);
	gen3_emit_composite_state(sna, &tmp->base);
	return true;

cleanup_src:
	if (tmp->base.src.bo)
		kgem_bo_destroy(&sna->kgem, tmp->base.src.bo);
cleanup_dst:
	if (tmp->base.redirect.real_bo)
		kgem_bo_destroy(&sna->kgem, tmp->base.dst.bo);
	return false;
}

static void
gen3_emit_video_state(struct sna *sna,
		      struct sna_video *video,
		      struct sna_video_frame *frame,
		      PixmapPtr pixmap,
		      struct kgem_bo *dst_bo,
		      int width, int height,
		      bool bilinear)
{
	struct gen3_render_state *state = &sna->render_state.gen3;
	uint32_t id, ms3, rewind;

	gen3_emit_target(sna, dst_bo, width, height,
			 sna_format_for_depth(pixmap->drawable.depth));

	/* XXX share with composite? Is it worth the effort? */
	if ((state->last_shader & (1<<31)) == 0) {
		OUT_BATCH(_3DSTATE_LOAD_STATE_IMMEDIATE_1 |
			  I1_LOAD_S(1) | I1_LOAD_S(2) | I1_LOAD_S(6) |
			  2);
		OUT_BATCH((4 << S1_VERTEX_WIDTH_SHIFT) | (4 << S1_VERTEX_PITCH_SHIFT));
		OUT_BATCH(S2_TEXCOORD_FMT(0, TEXCOORDFMT_2D) |
			  S2_TEXCOORD_FMT(1, TEXCOORDFMT_NOT_PRESENT) |
			  S2_TEXCOORD_FMT(2, TEXCOORDFMT_NOT_PRESENT) |
			  S2_TEXCOORD_FMT(3, TEXCOORDFMT_NOT_PRESENT) |
			  S2_TEXCOORD_FMT(4, TEXCOORDFMT_NOT_PRESENT) |
			  S2_TEXCOORD_FMT(5, TEXCOORDFMT_NOT_PRESENT) |
			  S2_TEXCOORD_FMT(6, TEXCOORDFMT_NOT_PRESENT) |
			  S2_TEXCOORD_FMT(7, TEXCOORDFMT_NOT_PRESENT));
		OUT_BATCH((2 << S6_CBUF_SRC_BLEND_FACT_SHIFT) |
			  (1 << S6_CBUF_DST_BLEND_FACT_SHIFT) |
			  S6_COLOR_WRITE_ENABLE);

		state->last_blend = 0;
		state->floats_per_vertex = 4;
	}

	if (!is_planar_fourcc(frame->id)) {
		rewind = sna->kgem.nbatch;
		OUT_BATCH(_3DSTATE_PIXEL_SHADER_CONSTANTS | 4);
		OUT_BATCH(0x0000001);	/* constant 0 */
		/* constant 0: brightness/contrast */
		OUT_BATCH_F(video->brightness / 128.0);
		OUT_BATCH_F(video->contrast / 255.0);
		OUT_BATCH_F(0.0);
		OUT_BATCH_F(0.0);
		if (state->last_constants &&
		    memcmp(&sna->kgem.batch[state->last_constants],
			   &sna->kgem.batch[rewind],
			   6*sizeof(uint32_t)) == 0)
			sna->kgem.nbatch = rewind;
		else
			state->last_constants = rewind;

		rewind = sna->kgem.nbatch;
		OUT_BATCH(_3DSTATE_SAMPLER_STATE | 3);
		OUT_BATCH(0x00000001);
		OUT_BATCH(SS2_COLORSPACE_CONVERSION |
			  (FILTER_LINEAR << SS2_MAG_FILTER_SHIFT) |
			  (FILTER_LINEAR << SS2_MIN_FILTER_SHIFT));
		OUT_BATCH((TEXCOORDMODE_CLAMP_EDGE << SS3_TCX_ADDR_MODE_SHIFT) |
			  (TEXCOORDMODE_CLAMP_EDGE << SS3_TCY_ADDR_MODE_SHIFT) |
			  (0 << SS3_TEXTUREMAP_INDEX_SHIFT) |
			  SS3_NORMALIZED_COORDS);
		OUT_BATCH(0x00000000);
		if (state->last_sampler &&
		    memcmp(&sna->kgem.batch[state->last_sampler],
			   &sna->kgem.batch[rewind],
			   5*sizeof(uint32_t)) == 0)
			sna->kgem.nbatch = rewind;
		else
			state->last_sampler = rewind;

		OUT_BATCH(_3DSTATE_MAP_STATE | 3);
		OUT_BATCH(0x00000001);	/* texture map #1 */
		OUT_BATCH(kgem_add_reloc(&sna->kgem, sna->kgem.nbatch,
					 frame->bo,
					 I915_GEM_DOMAIN_SAMPLER << 16,
					 0));

		ms3 = MAPSURF_422;
		switch (frame->id) {
		case FOURCC_YUY2:
			ms3 |= MT_422_YCRCB_NORMAL;
			break;
		case FOURCC_UYVY:
			ms3 |= MT_422_YCRCB_SWAPY;
			break;
		}
		ms3 |= (frame->height - 1) << MS3_HEIGHT_SHIFT;
		ms3 |= (frame->width - 1) << MS3_WIDTH_SHIFT;
		OUT_BATCH(ms3);
		OUT_BATCH(((frame->pitch[0] / 4) - 1) << MS4_PITCH_SHIFT);

		id = 1<<31 | 1<<1 | !!video->brightness;
		if (state->last_shader != id) {
			state->last_shader = id;
			id = sna->kgem.nbatch++;

			gen3_fs_dcl(FS_S0);
			gen3_fs_dcl(FS_T0);
			gen3_fs_texld(FS_OC, FS_S0, FS_T0);
			if (video->brightness != 0) {
				gen3_fs_add(FS_OC,
					    gen3_fs_operand_reg(FS_OC),
					    gen3_fs_operand(FS_C0, X, X, X, ZERO));
			}

			sna->kgem.batch[id] =
				_3DSTATE_PIXEL_SHADER_PROGRAM |
				(sna->kgem.nbatch - id - 2);
		}
	} else {
		/* For the planar formats, we set up three samplers --
		 * one for each plane, in a Y8 format.  Because I
		 * couldn't get the special PLANAR_TO_PACKED
		 * shader setup to work, I did the manual pixel shader:
		 *
		 * y' = y - .0625
		 * u' = u - .5
		 * v' = v - .5;
		 *
		 * r = 1.1643 * y' + 0.0     * u' + 1.5958  * v'
		 * g = 1.1643 * y' - 0.39173 * u' - 0.81290 * v'
		 * b = 1.1643 * y' + 2.017   * u' + 0.0     * v'
		 *
		 * register assignment:
		 * r0 = (y',u',v',0)
		 * r1 = (y,y,y,y)
		 * r2 = (u,u,u,u)
		 * r3 = (v,v,v,v)
		 * OC = (r,g,b,1)
		 */
		rewind = sna->kgem.nbatch;
		OUT_BATCH(_3DSTATE_PIXEL_SHADER_CONSTANTS | (22 - 2));
		OUT_BATCH(0x000001f);	/* constants 0-4 */
		/* constant 0: normalization offsets */
		OUT_BATCH_F(-0.0625);
		OUT_BATCH_F(-0.5);
		OUT_BATCH_F(-0.5);
		OUT_BATCH_F(0.0);
		/* constant 1: r coefficients */
		OUT_BATCH_F(1.1643);
		OUT_BATCH_F(0.0);
		OUT_BATCH_F(1.5958);
		OUT_BATCH_F(0.0);
		/* constant 2: g coefficients */
		OUT_BATCH_F(1.1643);
		OUT_BATCH_F(-0.39173);
		OUT_BATCH_F(-0.81290);
		OUT_BATCH_F(0.0);
		/* constant 3: b coefficients */
		OUT_BATCH_F(1.1643);
		OUT_BATCH_F(2.017);
		OUT_BATCH_F(0.0);
		OUT_BATCH_F(0.0);
		/* constant 4: brightness/contrast */
		OUT_BATCH_F(video->brightness / 128.0);
		OUT_BATCH_F(video->contrast / 255.0);
		OUT_BATCH_F(0.0);
		OUT_BATCH_F(0.0);
		if (state->last_constants &&
		    memcmp(&sna->kgem.batch[state->last_constants],
			   &sna->kgem.batch[rewind],
			   22*sizeof(uint32_t)) == 0)
			sna->kgem.nbatch = rewind;
		else
			state->last_constants = rewind;

		rewind = sna->kgem.nbatch;
		OUT_BATCH(_3DSTATE_SAMPLER_STATE | 9);
		OUT_BATCH(0x00000007);
		/* sampler 0 */
		OUT_BATCH((FILTER_LINEAR << SS2_MAG_FILTER_SHIFT) |
			  (FILTER_LINEAR << SS2_MIN_FILTER_SHIFT));
		OUT_BATCH((TEXCOORDMODE_CLAMP_EDGE << SS3_TCX_ADDR_MODE_SHIFT) |
			  (TEXCOORDMODE_CLAMP_EDGE << SS3_TCY_ADDR_MODE_SHIFT) |
			  (0 << SS3_TEXTUREMAP_INDEX_SHIFT) |
			  SS3_NORMALIZED_COORDS);
		OUT_BATCH(0x00000000);
		/* sampler 1 */
		OUT_BATCH((FILTER_LINEAR << SS2_MAG_FILTER_SHIFT) |
			  (FILTER_LINEAR << SS2_MIN_FILTER_SHIFT));
		OUT_BATCH((TEXCOORDMODE_CLAMP_EDGE << SS3_TCX_ADDR_MODE_SHIFT) |
			  (TEXCOORDMODE_CLAMP_EDGE << SS3_TCY_ADDR_MODE_SHIFT) |
			  (1 << SS3_TEXTUREMAP_INDEX_SHIFT) |
			  SS3_NORMALIZED_COORDS);
		OUT_BATCH(0x00000000);
		/* sampler 2 */
		OUT_BATCH((FILTER_LINEAR << SS2_MAG_FILTER_SHIFT) |
			  (FILTER_LINEAR << SS2_MIN_FILTER_SHIFT));
		OUT_BATCH((TEXCOORDMODE_CLAMP_EDGE << SS3_TCX_ADDR_MODE_SHIFT) |
			  (TEXCOORDMODE_CLAMP_EDGE << SS3_TCY_ADDR_MODE_SHIFT) |
			  (2 << SS3_TEXTUREMAP_INDEX_SHIFT) |
			  SS3_NORMALIZED_COORDS);
		OUT_BATCH(0x00000000);
		if (state->last_sampler &&
		    memcmp(&sna->kgem.batch[state->last_sampler],
			   &sna->kgem.batch[rewind],
			   11*sizeof(uint32_t)) == 0)
			sna->kgem.nbatch = rewind;
		else
			state->last_sampler = rewind;

		OUT_BATCH(_3DSTATE_MAP_STATE | 9);
		OUT_BATCH(0x00000007);

		OUT_BATCH(kgem_add_reloc(&sna->kgem, sna->kgem.nbatch,
					 frame->bo,
					 I915_GEM_DOMAIN_SAMPLER << 16,
					 0));

		ms3 = MAPSURF_8BIT | MT_8BIT_I8;
		ms3 |= (frame->height - 1) << MS3_HEIGHT_SHIFT;
		ms3 |= (frame->width - 1) << MS3_WIDTH_SHIFT;
		OUT_BATCH(ms3);
		/* check to see if Y has special pitch than normal
		 * double u/v pitch, e.g i915 XvMC hw requires at
		 * least 1K alignment, so Y pitch might
		 * be same as U/V's.*/
		if (frame->pitch[1])
			OUT_BATCH(((frame->pitch[1] / 4) - 1) << MS4_PITCH_SHIFT);
		else
			OUT_BATCH(((frame->pitch[0] * 2 / 4) - 1) << MS4_PITCH_SHIFT);

		OUT_BATCH(kgem_add_reloc(&sna->kgem, sna->kgem.nbatch,
					 frame->bo,
					 I915_GEM_DOMAIN_SAMPLER << 16,
					 frame->UBufOffset));

		ms3 = MAPSURF_8BIT | MT_8BIT_I8;
		ms3 |= (frame->height / 2 - 1) << MS3_HEIGHT_SHIFT;
		ms3 |= (frame->width / 2 - 1) << MS3_WIDTH_SHIFT;
		OUT_BATCH(ms3);
		OUT_BATCH(((frame->pitch[0] / 4) - 1) << MS4_PITCH_SHIFT);

		OUT_BATCH(kgem_add_reloc(&sna->kgem, sna->kgem.nbatch,
					 frame->bo,
					 I915_GEM_DOMAIN_SAMPLER << 16,
					 frame->VBufOffset));

		ms3 = MAPSURF_8BIT | MT_8BIT_I8;
		ms3 |= (frame->height / 2 - 1) << MS3_HEIGHT_SHIFT;
		ms3 |= (frame->width / 2 - 1) << MS3_WIDTH_SHIFT;
		OUT_BATCH(ms3);
		OUT_BATCH(((frame->pitch[0] / 4) - 1) << MS4_PITCH_SHIFT);

		id = 1<<31 | 2<<1 | !!video->brightness;
		if (state->last_shader != id) {
			state->last_shader = id;
			id = sna->kgem.nbatch++;

			/* Declare samplers */
			gen3_fs_dcl(FS_S0);	/* Y */
			gen3_fs_dcl(FS_S1);	/* U */
			gen3_fs_dcl(FS_S2);	/* V */
			gen3_fs_dcl(FS_T0);	/* normalized coords */

			/* Load samplers to temporaries. */
			gen3_fs_texld(FS_R1, FS_S0, FS_T0);
			gen3_fs_texld(FS_R2, FS_S1, FS_T0);
			gen3_fs_texld(FS_R3, FS_S2, FS_T0);

			/* Move the sampled YUV data in R[123] to the first
			 * 3 channels of R0.
			 */
			gen3_fs_mov_masked(FS_R0, MASK_X,
					   gen3_fs_operand_reg(FS_R1));
			gen3_fs_mov_masked(FS_R0, MASK_Y,
					   gen3_fs_operand_reg(FS_R2));
			gen3_fs_mov_masked(FS_R0, MASK_Z,
					   gen3_fs_operand_reg(FS_R3));

			/* Normalize the YUV data */
			gen3_fs_add(FS_R0, gen3_fs_operand_reg(FS_R0),
				    gen3_fs_operand_reg(FS_C0));
			/* dot-product the YUV data in R0 by the vectors of
			 * coefficients for calculating R, G, and B, storing
			 * the results in the R, G, or B channels of the output
			 * color.  The OC results are implicitly clamped
			 * at the end of the program.
			 */
			gen3_fs_dp3(FS_OC, MASK_X,
				    gen3_fs_operand_reg(FS_R0),
				    gen3_fs_operand_reg(FS_C1));
			gen3_fs_dp3(FS_OC, MASK_Y,
				    gen3_fs_operand_reg(FS_R0),
				    gen3_fs_operand_reg(FS_C2));
			gen3_fs_dp3(FS_OC, MASK_Z,
				    gen3_fs_operand_reg(FS_R0),
				    gen3_fs_operand_reg(FS_C3));
			/* Set alpha of the output to 1.0, by wiring W to 1
			 * and not actually using the source.
			 */
			gen3_fs_mov_masked(FS_OC, MASK_W,
					   gen3_fs_operand_one());

			if (video->brightness != 0) {
				gen3_fs_add(FS_OC,
					    gen3_fs_operand_reg(FS_OC),
					    gen3_fs_operand(FS_C4, X, X, X, ZERO));
			}

			sna->kgem.batch[id] =
				_3DSTATE_PIXEL_SHADER_PROGRAM |
				(sna->kgem.nbatch - id - 2);
		}
	}
}

static void
gen3_video_get_batch(struct sna *sna, struct kgem_bo *bo)
{
	kgem_set_mode(&sna->kgem, KGEM_RENDER, bo);

	if (!kgem_check_batch(&sna->kgem, 120) ||
	    !kgem_check_reloc(&sna->kgem, 4) ||
	    !kgem_check_exec(&sna->kgem, 2)) {
		_kgem_submit(&sna->kgem);
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	if (sna->render_state.gen3.need_invariant)
		gen3_emit_invariant(sna);
}

static int
gen3_get_inline_rectangles(struct sna *sna, int want, int floats_per_vertex)
{
	int size = floats_per_vertex * 3;
	int rem = batch_space(sna) - 1;

	if (size * want > rem)
		want = rem / size;

	return want;
}

static bool
gen3_render_video(struct sna *sna,
		  struct sna_video *video,
		  struct sna_video_frame *frame,
		  RegionPtr dstRegion,
		  PixmapPtr pixmap)
{
	struct sna_pixmap *priv = sna_pixmap(pixmap);
	BoxPtr pbox = REGION_RECTS(dstRegion);
	int nbox = REGION_NUM_RECTS(dstRegion);
	int dst_width = dstRegion->extents.x2 - dstRegion->extents.x1;
	int dst_height = dstRegion->extents.y2 - dstRegion->extents.y1;
	int src_width = frame->src.x2 - frame->src.x1;
	int src_height = frame->src.y2 - frame->src.y1;
	float src_offset_x, src_offset_y;
	float src_scale_x, src_scale_y;
	int pix_xoff, pix_yoff;
	struct kgem_bo *dst_bo;
	bool bilinear;
	int copy = 0;

	DBG(("%s: src:%dx%d (frame:%dx%d) -> dst:%dx%d\n", __FUNCTION__,
	     src_width, src_height, frame->width, frame->height, dst_width, dst_height));

	dst_bo = priv->gpu_bo;
	if (dst_bo == NULL)
		return false;

	bilinear = src_width != dst_width || src_height != dst_height;

	src_scale_x = (float)src_width / dst_width / frame->width;
	src_offset_x = (float)frame->src.x1 / frame->width - dstRegion->extents.x1 * src_scale_x;

	src_scale_y = (float)src_height / dst_height / frame->height;
	src_offset_y = (float)frame->src.y1 / frame->height - dstRegion->extents.y1 * src_scale_y;
	DBG(("%s: src offset (%f, %f), scale (%f, %f)\n",
	     __FUNCTION__, src_offset_x, src_offset_y, src_scale_x, src_scale_y));

	if (too_large(pixmap->drawable.width, pixmap->drawable.height) ||
	    !gen3_check_pitch_3d(dst_bo)) {
		int bpp = pixmap->drawable.bitsPerPixel;

		if (too_large(dst_width, dst_height))
			return false;

		dst_bo = kgem_create_2d(&sna->kgem,
					dst_width, dst_height, bpp,
					kgem_choose_tiling(&sna->kgem,
							   I915_TILING_X,
							   dst_width, dst_height, bpp),
					0);
		if (!dst_bo)
			return false;

		pix_xoff = -dstRegion->extents.x1;
		pix_yoff = -dstRegion->extents.y1;
		copy = 1;
	} else {
		/* Set up the offset for translating from the given region
		 * (in screen coordinates) to the backing pixmap.
		 */
#ifdef COMPOSITE
		pix_xoff = -pixmap->screen_x + pixmap->drawable.x;
		pix_yoff = -pixmap->screen_y + pixmap->drawable.y;
#else
		pix_xoff = 0;
		pix_yoff = 0;
#endif

		dst_width  = pixmap->drawable.width;
		dst_height = pixmap->drawable.height;
	}

	gen3_video_get_batch(sna, dst_bo);
	gen3_emit_video_state(sna, video, frame, pixmap,
			      dst_bo, dst_width, dst_height, bilinear);
	do {
		int nbox_this_time = gen3_get_inline_rectangles(sna, nbox, 4);
		if (nbox_this_time == 0) {
			gen3_video_get_batch(sna, dst_bo);
			gen3_emit_video_state(sna, video, frame, pixmap,
					      dst_bo, dst_width, dst_height, bilinear);
			nbox_this_time = gen3_get_inline_rectangles(sna, nbox, 4);
			assert(nbox_this_time);
		}
		nbox -= nbox_this_time;

		OUT_BATCH(PRIM3D_RECTLIST | (12 * nbox_this_time - 1));
		do {
			int box_x1 = pbox->x1;
			int box_y1 = pbox->y1;
			int box_x2 = pbox->x2;
			int box_y2 = pbox->y2;

			pbox++;

			DBG(("%s: dst (%d, %d), (%d, %d) + (%d, %d); src (%f, %f), (%f, %f)\n",
			     __FUNCTION__, box_x1, box_y1, box_x2, box_y2, pix_xoff, pix_yoff,
			     box_x1 * src_scale_x + src_offset_x,
			     box_y1 * src_scale_y + src_offset_y,
			     box_x2 * src_scale_x + src_offset_x,
			     box_y2 * src_scale_y + src_offset_y));

			/* bottom right */
			OUT_BATCH_F(box_x2 + pix_xoff);
			OUT_BATCH_F(box_y2 + pix_yoff);
			OUT_BATCH_F(box_x2 * src_scale_x + src_offset_x);
			OUT_BATCH_F(box_y2 * src_scale_y + src_offset_y);

			/* bottom left */
			OUT_BATCH_F(box_x1 + pix_xoff);
			OUT_BATCH_F(box_y2 + pix_yoff);
			OUT_BATCH_F(box_x1 * src_scale_x + src_offset_x);
			OUT_BATCH_F(box_y2 * src_scale_y + src_offset_y);

			/* top left */
			OUT_BATCH_F(box_x1 + pix_xoff);
			OUT_BATCH_F(box_y1 + pix_yoff);
			OUT_BATCH_F(box_x1 * src_scale_x + src_offset_x);
			OUT_BATCH_F(box_y1 * src_scale_y + src_offset_y);
		} while (--nbox_this_time);
	} while (nbox);

	if (copy) {
#ifdef COMPOSITE
		pix_xoff = -pixmap->screen_x + pixmap->drawable.x;
		pix_yoff = -pixmap->screen_y + pixmap->drawable.y;
#else
		pix_xoff = 0;
		pix_yoff = 0;
#endif
		sna_blt_copy_boxes(sna, GXcopy,
				   dst_bo, -dstRegion->extents.x1, -dstRegion->extents.y1,
				   priv->gpu_bo, pix_xoff, pix_yoff,
				   pixmap->drawable.bitsPerPixel,
				   REGION_RECTS(dstRegion),
				   REGION_NUM_RECTS(dstRegion));

		kgem_bo_destroy(&sna->kgem, dst_bo);
	}

	if (!DAMAGE_IS_ALL(priv->gpu_damage)) {
		if ((pix_xoff | pix_yoff) == 0) {
			sna_damage_add(&priv->gpu_damage, dstRegion);
			sna_damage_subtract(&priv->cpu_damage, dstRegion);
		} else {
			sna_damage_add_boxes(&priv->gpu_damage,
					     REGION_RECTS(dstRegion),
					     REGION_NUM_RECTS(dstRegion),
					     pix_xoff, pix_yoff);
			sna_damage_subtract_boxes(&priv->cpu_damage,
						  REGION_RECTS(dstRegion),
						  REGION_NUM_RECTS(dstRegion),
						  pix_xoff, pix_yoff);
		}
	}

	return true;
}

#endif


















































































#if 0
static bool
gen3_render_fill_one(struct sna *sna, PixmapPtr dst, struct kgem_bo *bo,
		     uint32_t color,
		     int16_t x1, int16_t y1,
		     int16_t x2, int16_t y2,
		     uint8_t alu)
{
	struct sna_composite_op tmp;

#if NO_FILL_ONE
	return gen3_render_fill_one_try_blt(sna, dst, bo, color,
					    x1, y1, x2, y2, alu);
#endif

	/* Prefer to use the BLT if already engaged */
	if (prefer_fill_blt(sna) &&
	    gen3_render_fill_one_try_blt(sna, dst, bo, color,
					 x1, y1, x2, y2, alu))
		return true;

	/* Must use the BLT if we can't RENDER... */
	if (!(alu == GXcopy || alu == GXclear) ||
	    too_large(dst->drawable.width, dst->drawable.height) ||
	    bo->pitch > MAX_3D_PITCH)
		return gen3_render_fill_one_try_blt(sna, dst, bo, color,
						    x1, y1, x2, y2, alu);

	if (alu == GXclear)
		color = 0;

	tmp.op = color == 0 ? PictOpClear : PictOpSrc;
	tmp.dst.pixmap = dst;
	tmp.dst.width = dst->drawable.width;
	tmp.dst.height = dst->drawable.height;
	tmp.dst.format = sna_format_for_depth(dst->drawable.depth);
	tmp.dst.bo = bo;
	tmp.floats_per_vertex = 2;
	tmp.floats_per_rect = 6;
	tmp.need_magic_ca_pass = 0;
	tmp.has_component_alpha = 0;
	tmp.rb_reversed = 0;

	gen3_init_solid(&tmp.src,
			sna_rgba_for_color(color, dst->drawable.depth));
	tmp.mask.bo = NULL;
	tmp.mask.u.gen3.type = SHADER_NONE;
	tmp.u.gen3.num_constants = 0;

	if (!kgem_check_bo(&sna->kgem, bo, NULL)) {
		kgem_submit(&sna->kgem);

		if (gen3_render_fill_one_try_blt(sna, dst, bo, color,
						 x1, y1, x2, y2, alu))
			return true;

		if (!kgem_check_bo(&sna->kgem, bo, NULL))
			return false;
	}

	gen3_align_vertex(sna, &tmp);
	gen3_emit_composite_state(sna, &tmp);
	gen3_get_rectangles(sna, &tmp, 1);
	DBG(("	(%d, %d), (%d, %d): %x\n", x1, y1, x2, y2, color));
	OUT_VERTEX(x2);
	OUT_VERTEX(y2);
	OUT_VERTEX(x1);
	OUT_VERTEX(y2);
	OUT_VERTEX(x1);
	OUT_VERTEX(y1);
	gen3_vertex_flush(sna);

	return true;
}
#endif

static void gen3_render_flush(struct sna *sna)
{
	gen3_vertex_close(sna);

	assert(sna->render.vertex_reloc[0] == 0);
	assert(sna->render.vertex_offset == 0);
}

static void
gen3_render_fini(struct sna *sna)
{
}

const char *gen3_render_init(struct sna *sna, const char *backend)
{
	struct sna_render *render = &sna->render;

#if 0
#if !NO_COMPOSITE
	render->composite = gen3_render_composite;
	render->prefer_gpu |= PREFER_GPU_RENDER;
#endif
#if !NO_COMPOSITE_SPANS
	render->check_composite_spans = gen3_check_composite_spans;
	render->composite_spans = gen3_render_composite_spans;
	render->prefer_gpu |= PREFER_GPU_SPANS;
#endif

	render->video = gen3_render_video;

	render->copy_boxes = gen3_render_copy_boxes;
	render->copy = gen3_render_copy;

	render->fill_boxes = gen3_render_fill_boxes;
	render->fill = gen3_render_fill;
	render->fill_one = gen3_render_fill_one;
#endif

    render->blit_tex = gen3_blit_tex;
    render->caps = HW_BIT_BLIT | HW_TEX_BLIT;

	render->reset = gen3_render_reset;
	render->flush = gen3_render_flush;
	render->fini = gen3_render_fini;

	render->max_3d_size = MAX_3D_SIZE;
	render->max_3d_pitch = MAX_3D_PITCH;

	sna->kgem.retire = gen3_render_retire;
	sna->kgem.expire = gen3_render_expire;
	return "Alviso (gen3)";
}

static bool
gen3_blit_tex(struct sna *sna,
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

    DBG(("%s: %dx%d, current mode=%d\n", __FUNCTION__,
         width, height, sna->kgem.ring));

    tmp->op = PictOpSrc;

    tmp->dst.pixmap = dst;
    tmp->dst.bo     = dst_bo;
    tmp->dst.width  = dst->drawable.width;
    tmp->dst.height = dst->drawable.height;
    tmp->dst.format = PICT_x8r8g8b8;

	tmp->rb_reversed = gen3_dst_rb_reversed(tmp->dst.format);

	tmp->u.gen3.num_constants = 0;
	tmp->src.u.gen3.type = SHADER_TEXTURE;
	tmp->src.is_affine = true;


	tmp->src.repeat = RepeatNone;
	tmp->src.filter = PictFilterNearest;

    tmp->src.bo = src_bo;
	tmp->src.pict_format = PICT_x8r8g8b8;

	gen3_composite_channel_set_format(&tmp->src, tmp->src.pict_format);

    tmp->src.width  = src->drawable.width;
    tmp->src.height = src->drawable.height;

	tmp->mask.u.gen3.type = SHADER_TEXTURE;
	tmp->mask.is_affine = true;
	tmp->need_magic_ca_pass = false;
	tmp->has_component_alpha = false;


 	tmp->mask.repeat = RepeatNone;
	tmp->mask.filter = PictFilterNearest;
    tmp->mask.is_affine = true;

    tmp->mask.bo = mask_bo;
    tmp->mask.pict_format = PIXMAN_a8;
	gen3_composite_channel_set_format(&tmp->mask, tmp->mask.pict_format);
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

	tmp->prim_emit = gen3_emit_composite_primitive_identity_source_mask;


	tmp->floats_per_vertex = 2;
	if (!is_constant_ps(tmp->src.u.gen3.type))
		tmp->floats_per_vertex += tmp->src.is_affine ? 2 : 4;
	if (!is_constant_ps(tmp->mask.u.gen3.type))
		tmp->floats_per_vertex += tmp->mask.is_affine ? 2 : 4;
//	DBG(("%s: floats_per_vertex = 2 + %d + %d = %d [specialised emitter? %d]\n", __FUNCTION__,
//	     !is_constant_ps(tmp->src.u.gen3.type) ? tmp->src.is_affine ? 2 : 4 : 0,
//	     !is_constant_ps(tmp->mask.u.gen3.type) ? tmp->mask.is_affine ? 2 : 4 : 0,
//	     tmp->floats_per_vertex,
//	     tmp->prim_emit != gen3_emit_composite_primitive));
	tmp->floats_per_rect = 3 * tmp->floats_per_vertex;

	tmp->blt   = gen3_render_composite_blt;

	tmp->done  = gen3_render_composite_done;

	if (!kgem_check_bo(&sna->kgem,
			   tmp->dst.bo, tmp->src.bo, tmp->mask.bo,
			   NULL)) {
		kgem_submit(&sna->kgem);
	}

	gen3_align_vertex(sna, tmp);
	gen3_emit_composite_state(sna, tmp);
	return true;
}
