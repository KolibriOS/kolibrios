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
	{PICT_x8b8g8r8, 0, MAPSURF_32BIT | MT_32BIT_XBGR8888, false}
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
	case PICT_a8:
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
	case PICT_a8:
		return BIAS | COLR_BUF_8BIT;
	}
#undef BIAS
}



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
		} else if (IS_CPU_MAP(bo->map)) {
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

	assert(op->floats_per_rect >= vertex_space(sna));
	assert(rem <= vertex_space(sna));
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

	if (sna->render.vbo != NULL &&
	    !kgem_bo_is_mappable(&sna->kgem, sna->render.vbo)) {
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



























































static void
gen3_align_vertex(struct sna *sna,
		  const struct sna_composite_op *op)
{
	if (op->floats_per_vertex != sna->render_state.gen3.last_floats_per_vertex) {
		if (sna->render.vertex_size - sna->render.vertex_used < 2*op->floats_per_rect)
			gen3_vertex_finish(sna);

		DBG(("aligning vertex: was %d, now %d floats per vertex, %d->%d\n",
		     sna->render_state.gen3.last_floats_per_vertex,
		     op->floats_per_vertex,
		     sna->render.vertex_index,
		     (sna->render.vertex_used + op->floats_per_vertex - 1) / op->floats_per_vertex));
		sna->render.vertex_index = (sna->render.vertex_used + op->floats_per_vertex - 1) / op->floats_per_vertex;
		sna->render.vertex_used = sna->render.vertex_index * op->floats_per_vertex;
		assert(sna->render.vertex_used < sna->render.vertex_size - op->floats_per_rect);
		sna->render_state.gen3.last_floats_per_vertex = op->floats_per_vertex;
	}
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
	DBG(("%s: floats_per_vertex = 2 + %d + %d = %d [specialised emitter? %d]\n", __FUNCTION__,
	     !is_constant_ps(tmp->src.u.gen3.type) ? tmp->src.is_affine ? 2 : 4 : 0,
	     !is_constant_ps(tmp->mask.u.gen3.type) ? tmp->mask.is_affine ? 2 : 4 : 0,
	     tmp->floats_per_vertex,
	     tmp->prim_emit != gen3_emit_composite_primitive));
	tmp->floats_per_rect = 3 * tmp->floats_per_vertex;

	tmp->blt   = gen3_render_composite_blt;

	tmp->done  = gen3_render_composite_done;

	if (!kgem_check_bo(&sna->kgem,
			   tmp->dst.bo, tmp->src.bo, tmp->mask.bo,
			   NULL)) {
		kgem_submit(&sna->kgem);
	}

	gen3_emit_composite_state(sna, tmp);
	gen3_align_vertex(sna, tmp);
	return true;
}

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

bool gen3_render_init(struct sna *sna)
{
	struct sna_render *render = &sna->render;


//	render->video = gen3_render_video;

    render->blit_tex = gen3_blit_tex;

	render->reset = gen3_render_reset;
	render->flush = gen3_render_flush;
	render->fini = gen3_render_fini;

	render->max_3d_size = MAX_3D_SIZE;
	render->max_3d_pitch = MAX_3D_PITCH;

    render->caps = HW_BIT_BLIT | HW_TEX_BLIT;

	sna->kgem.retire = gen3_render_retire;
	sna->kgem.expire = gen3_render_expire;
	return true;
}
