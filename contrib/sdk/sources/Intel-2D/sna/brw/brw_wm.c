#include "brw.h"

#define X16 8
#define Y16 10

static void brw_wm_xy(struct brw_compile *p, int dw)
{
	struct brw_reg r1 = brw_vec1_grf(1, 0);
	struct brw_reg r1_uw = __retype_uw(r1);
	struct brw_reg x_uw, y_uw;

	brw_set_compression_control(p, BRW_COMPRESSION_NONE);

	if (dw == 16) {
		x_uw = brw_uw16_grf(30, 0);
		y_uw = brw_uw16_grf(28, 0);
	} else {
		x_uw = brw_uw8_grf(30, 0);
		y_uw = brw_uw8_grf(28, 0);
	}

	brw_ADD(p,
		x_uw,
		__stride(__suboffset(r1_uw, 4), 2, 4, 0),
		brw_imm_v(0x10101010));
	brw_ADD(p,
		y_uw,
		__stride(__suboffset(r1_uw, 5), 2, 4, 0),
		brw_imm_v(0x11001100));

	brw_set_compression_control(p, BRW_COMPRESSION_COMPRESSED);

	brw_ADD(p, brw_vec8_grf(X16, 0), vec8(x_uw), brw_negate(r1));
	brw_ADD(p, brw_vec8_grf(Y16, 0), vec8(y_uw), brw_negate(__suboffset(r1, 1)));
}

static void brw_wm_affine_st(struct brw_compile *p, int dw,
			     int channel, int msg)
{
	int uv;

	if (dw == 16) {
		brw_set_compression_control(p, BRW_COMPRESSION_COMPRESSED);
		uv = p->gen >= 060 ? 6 : 3;
	} else {
		brw_set_compression_control(p, BRW_COMPRESSION_NONE);
		uv = p->gen >= 060 ? 4 : 3;
	}
	uv += 2*channel;

	msg++;
	if (p->gen >= 060) {
		brw_PLN(p,
			brw_message_reg(msg),
			brw_vec1_grf(uv, 0),
			brw_vec8_grf(2, 0));
		msg += dw/8;

		brw_PLN(p,
			brw_message_reg(msg),
			brw_vec1_grf(uv, 4),
			brw_vec8_grf(2, 0));
	} else {
		struct brw_reg r = brw_vec1_grf(uv, 0);

		brw_LINE(p, brw_null_reg(), __suboffset(r, 0), brw_vec8_grf(X16, 0));
		brw_MAC(p, brw_message_reg(msg), __suboffset(r, 1), brw_vec8_grf(Y16, 0));
		msg += dw/8;

		brw_LINE(p, brw_null_reg(), __suboffset(r, 4), brw_vec8_grf(X16, 0));
		brw_MAC(p, brw_message_reg(msg), __suboffset(r, 5), brw_vec8_grf(Y16, 0));
	}
}

static inline unsigned simd(int dw)
{
	return dw == 16 ? BRW_SAMPLER_SIMD_MODE_SIMD16 : BRW_SAMPLER_SIMD_MODE_SIMD8;
}

static inline struct brw_reg sample_result(int dw, int result)
{
	return brw_reg(BRW_GENERAL_REGISTER_FILE, result, 0,
		       BRW_REGISTER_TYPE_UW,
		       dw == 16 ? BRW_VERTICAL_STRIDE_16 : BRW_VERTICAL_STRIDE_8,
		       dw == 16 ? BRW_WIDTH_16 : BRW_WIDTH_8,
		       BRW_HORIZONTAL_STRIDE_1,
		       BRW_SWIZZLE_XYZW,
		       WRITEMASK_XYZW);
}

static int brw_wm_sample(struct brw_compile *p, int dw,
			 int channel, int msg, int result)
{
	struct brw_reg src0;
	bool header;
	int len;

	len = dw == 16 ? 4 : 2;
	if (p->gen >= 060) {
		header = false;
		src0 = brw_message_reg(++msg);
	} else {
		header = true;
		src0 = brw_vec8_grf(0, 0);
	}

	brw_SAMPLE(p, sample_result(dw, result), msg, src0,
		   channel+1, channel, WRITEMASK_XYZW, 0,
		   2*len, len+header, header, simd(dw));
	return result;
}

static int brw_wm_sample__alpha(struct brw_compile *p, int dw,
				int channel, int msg, int result)
{
	struct brw_reg src0;
	int mlen, rlen;

	if (dw == 8) {
		/* SIMD8 sample return is not masked */
		mlen = 3;
		rlen = 4;
	} else {
		mlen = 5;
		rlen = 2;
	}

	if (p->gen >= 060)
		src0 = brw_message_reg(msg);
	else
		src0 = brw_vec8_grf(0, 0);

	brw_SAMPLE(p, sample_result(dw, result), msg, src0,
		   channel+1, channel, WRITEMASK_W, 0,
		   rlen, mlen, true, simd(dw));

	if (dw == 8)
		result += 3;

	return result;
}

static int brw_wm_affine(struct brw_compile *p, int dw,
			 int channel, int msg, int result)
{
	brw_wm_affine_st(p, dw, channel, msg);
	return brw_wm_sample(p, dw, channel, msg, result);
}

static int brw_wm_affine__alpha(struct brw_compile *p, int dw,
				int channel, int msg, int result)
{
	brw_wm_affine_st(p, dw, channel, msg);
	return brw_wm_sample__alpha(p, dw, channel, msg, result);
}

static inline struct brw_reg null_result(int dw)
{
	return brw_reg(BRW_ARCHITECTURE_REGISTER_FILE, BRW_ARF_NULL, 0,
		       BRW_REGISTER_TYPE_UW,
		       dw == 16 ? BRW_VERTICAL_STRIDE_16 : BRW_VERTICAL_STRIDE_8,
		       dw == 16 ? BRW_WIDTH_16 : BRW_WIDTH_8,
		       BRW_HORIZONTAL_STRIDE_1,
		       BRW_SWIZZLE_XYZW,
		       WRITEMASK_XYZW);
}

static void brw_fb_write(struct brw_compile *p, int dw)
{
	struct brw_instruction *insn;
	unsigned msg_control, msg_type, msg_len;
	struct brw_reg src0;
	bool header;

	if (dw == 16) {
		brw_set_compression_control(p, BRW_COMPRESSION_COMPRESSED);
		msg_control = BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD16_SINGLE_SOURCE;
		msg_len = 8;
	} else {
		brw_set_compression_control(p, BRW_COMPRESSION_NONE);
		msg_control = BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD8_SINGLE_SOURCE_SUBSPAN01;
		msg_len = 4;
	}

	if (p->gen < 060) {
		brw_push_insn_state(p);
		brw_set_compression_control(p, BRW_COMPRESSION_NONE);
		brw_set_mask_control(p, BRW_MASK_DISABLE);
		brw_MOV(p, brw_message_reg(1), brw_vec8_grf(1, 0));
		brw_pop_insn_state(p);

		msg_len += 2;
	}

	/* The execution mask is ignored for render target writes. */
	insn = brw_next_insn(p, BRW_OPCODE_SEND);
	insn->header.predicate_control = 0;
	insn->header.compression_control = BRW_COMPRESSION_NONE;

	if (p->gen >= 060) {
		msg_type = GEN6_DATAPORT_WRITE_MESSAGE_RENDER_TARGET_WRITE;
		src0 = brw_message_reg(2);
		header = false;
	} else {
		insn->header.destreg__conditionalmod = 0;
		msg_type = BRW_DATAPORT_WRITE_MESSAGE_RENDER_TARGET_WRITE;
		src0 = __retype_uw(brw_vec8_grf(0, 0));
		header = true;
	}

	brw_set_dest(p, insn, null_result(dw));
	brw_set_src0(p, insn, src0);
	brw_set_dp_write_message(p, insn, 0,
				 msg_control, msg_type, msg_len,
				 header, true, 0, true, false);
}

static void brw_wm_write(struct brw_compile *p, int dw, int src)
{
	int n;

	if (dw == 8 && p->gen >= 060) {
		/* XXX pixel execution mask? */
		brw_set_compression_control(p, BRW_COMPRESSION_NONE);

		brw_MOV(p, brw_message_reg(2), brw_vec8_grf(src+0, 0));
		brw_MOV(p, brw_message_reg(3), brw_vec8_grf(src+1, 0));
		brw_MOV(p, brw_message_reg(4), brw_vec8_grf(src+2, 0));
		brw_MOV(p, brw_message_reg(5), brw_vec8_grf(src+3, 0));
		goto done;
	}

	brw_set_compression_control(p, BRW_COMPRESSION_COMPRESSED);

	for (n = 0; n < 4; n++) {
		if (p->gen >= 060) {
			brw_MOV(p,
				brw_message_reg(2 + 2*n),
				brw_vec8_grf(src + 2*n, 0));
		} else if (p->gen >= 045 && dw == 16) {
			brw_MOV(p,
				brw_message_reg(2 + n + BRW_MRF_COMPR4),
				brw_vec8_grf(src + 2*n, 0));
		} else {
			brw_set_compression_control(p, BRW_COMPRESSION_NONE);
			brw_MOV(p,
				brw_message_reg(2 + n),
				brw_vec8_grf(src + 2*n, 0));

			if (dw == 16) {
				brw_set_compression_control(p, BRW_COMPRESSION_2NDHALF);
				brw_MOV(p,
					brw_message_reg(2 + n + 4),
					brw_vec8_grf(src + 2*n+1, 0));
			}
		}
	}

done:
	brw_fb_write(p, dw);
}

static void brw_wm_write__mask(struct brw_compile *p, int dw,
			       int src, int mask)
{
	int n;

	if (dw == 8 && p->gen >= 060) {
		brw_set_compression_control(p, BRW_COMPRESSION_NONE);

		brw_MUL(p,
			brw_message_reg(2),
			brw_vec8_grf(src+0, 0),
			brw_vec8_grf(mask, 0));
		brw_MUL(p,
			brw_message_reg(3),
			brw_vec8_grf(src+1, 0),
			brw_vec8_grf(mask, 0));
		brw_MUL(p,
			brw_message_reg(4),
			brw_vec8_grf(src+2, 0),
			brw_vec8_grf(mask, 0));
		brw_MUL(p,
			brw_message_reg(5),
			brw_vec8_grf(src+3, 0),
			brw_vec8_grf(mask, 0));

		goto done;
	}

	brw_set_compression_control(p, BRW_COMPRESSION_COMPRESSED);

	for (n = 0; n < 4; n++) {
		if (p->gen >= 060) {
			brw_MUL(p,
				brw_message_reg(2 + 2*n),
				brw_vec8_grf(src + 2*n, 0),
				brw_vec8_grf(mask, 0));
		} else if (p->gen >= 045 && dw == 16) {
			brw_MUL(p,
				brw_message_reg(2 + n + BRW_MRF_COMPR4),
				brw_vec8_grf(src + 2*n, 0),
				brw_vec8_grf(mask, 0));
		} else {
			brw_set_compression_control(p, BRW_COMPRESSION_NONE);
			brw_MUL(p,
				brw_message_reg(2 + n),
				brw_vec8_grf(src + 2*n, 0),
				brw_vec8_grf(mask, 0));

			if (dw == 16) {
				brw_set_compression_control(p, BRW_COMPRESSION_2NDHALF);
				brw_MUL(p,
					brw_message_reg(2 + n + 4),
					brw_vec8_grf(src + 2*n+1, 0),
					brw_vec8_grf(mask+1, 0));
			}
		}
	}

done:
	brw_fb_write(p, dw);
}

static void brw_wm_write__opacity(struct brw_compile *p, int dw,
				  int src, int mask)
{
	int n;

	if (dw == 8 && p->gen >= 060) {
		brw_set_compression_control(p, BRW_COMPRESSION_NONE);

		brw_MUL(p,
			brw_message_reg(2),
			brw_vec8_grf(src+0, 0),
			brw_vec1_grf(mask, 3));
		brw_MUL(p,
			brw_message_reg(3),
			brw_vec8_grf(src+1, 0),
			brw_vec1_grf(mask, 3));
		brw_MUL(p,
			brw_message_reg(4),
			brw_vec8_grf(src+2, 0),
			brw_vec1_grf(mask, 3));
		brw_MUL(p,
			brw_message_reg(5),
			brw_vec8_grf(src+3, 0),
			brw_vec1_grf(mask, 3));

		goto done;
	}

	brw_set_compression_control(p, BRW_COMPRESSION_COMPRESSED);

	for (n = 0; n < 4; n++) {
		if (p->gen >= 060) {
			brw_MUL(p,
				brw_message_reg(2 + 2*n),
				brw_vec8_grf(src + 2*n, 0),
				brw_vec1_grf(mask, 3));
		} else if (p->gen >= 045 && dw == 16) {
			brw_MUL(p,
				brw_message_reg(2 + n + BRW_MRF_COMPR4),
				brw_vec8_grf(src + 2*n, 0),
				brw_vec1_grf(mask, 3));
		} else {
			brw_set_compression_control(p, BRW_COMPRESSION_NONE);
			brw_MUL(p,
				brw_message_reg(2 + n),
				brw_vec8_grf(src + 2*n, 0),
				brw_vec1_grf(mask, 3));

			if (dw == 16) {
				brw_set_compression_control(p, BRW_COMPRESSION_2NDHALF);
				brw_MUL(p,
					brw_message_reg(2 + n + 4),
					brw_vec8_grf(src + 2*n+1, 0),
					brw_vec1_grf(mask, 3));
			}
		}
	}

done:
	brw_fb_write(p, dw);
}

static void brw_wm_write__mask_ca(struct brw_compile *p, int dw,
				  int src, int mask)
{
	int n;

	if (dw == 8 && p->gen >= 060) {
		brw_set_compression_control(p, BRW_COMPRESSION_NONE);

		brw_MUL(p,
			brw_message_reg(2),
			brw_vec8_grf(src  + 0, 0),
			brw_vec8_grf(mask + 0, 0));
		brw_MUL(p,
			brw_message_reg(3),
			brw_vec8_grf(src  + 1, 0),
			brw_vec8_grf(mask + 1, 0));
		brw_MUL(p,
			brw_message_reg(4),
			brw_vec8_grf(src  + 2, 0),
			brw_vec8_grf(mask + 2, 0));
		brw_MUL(p,
			brw_message_reg(5),
			brw_vec8_grf(src  + 3, 0),
			brw_vec8_grf(mask + 3, 0));

		goto done;
	}

	brw_set_compression_control(p, BRW_COMPRESSION_COMPRESSED);

	for (n = 0; n < 4; n++) {
		if (p->gen >= 060) {
			brw_MUL(p,
				brw_message_reg(2 + 2*n),
				brw_vec8_grf(src + 2*n, 0),
				brw_vec8_grf(mask + 2*n, 0));
		} else if (p->gen >= 045 && dw == 16) {
			brw_MUL(p,
				brw_message_reg(2 + n + BRW_MRF_COMPR4),
				brw_vec8_grf(src + 2*n, 0),
				brw_vec8_grf(mask + 2*n, 0));
		} else {
			brw_set_compression_control(p, BRW_COMPRESSION_NONE);
			brw_MUL(p,
				brw_message_reg(2 + n),
				brw_vec8_grf(src + 2*n, 0),
				brw_vec8_grf(mask + 2*n, 0));

			if (dw == 16) {
				brw_set_compression_control(p, BRW_COMPRESSION_2NDHALF);
				brw_MUL(p,
					brw_message_reg(2 + n + 4),
					brw_vec8_grf(src + 2*n + 1, 0),
					brw_vec8_grf(mask + 2*n + 1, 0));
			}
		}
	}

done:
	brw_fb_write(p, dw);
}

bool
brw_wm_kernel__affine(struct brw_compile *p, int dispatch)
{
	if (p->gen < 060)
		brw_wm_xy(p, dispatch);
	brw_wm_write(p, dispatch, brw_wm_affine(p, dispatch, 0, 1, 12));

	return true;
}

bool
brw_wm_kernel__affine_mask(struct brw_compile *p, int dispatch)
{
	int src, mask;

	if (p->gen < 060)
		brw_wm_xy(p, dispatch);

	src = brw_wm_affine(p, dispatch, 0, 1, 12);
	mask = brw_wm_affine__alpha(p, dispatch, 1, 6, 20);
	brw_wm_write__mask(p, dispatch, src, mask);

	return true;
}

bool
brw_wm_kernel__affine_mask_ca(struct brw_compile *p, int dispatch)
{
	int src, mask;

	if (p->gen < 060)
		brw_wm_xy(p, dispatch);

	src = brw_wm_affine(p, dispatch, 0, 1, 12);
	mask = brw_wm_affine(p, dispatch, 1, 6, 20);
	brw_wm_write__mask_ca(p, dispatch, src, mask);

	return true;
}

bool
brw_wm_kernel__affine_mask_sa(struct brw_compile *p, int dispatch)
{
	int src, mask;

	if (p->gen < 060)
		brw_wm_xy(p, dispatch);

	src = brw_wm_affine__alpha(p, dispatch, 0, 1, 12);
	mask = brw_wm_affine(p, dispatch, 1, 6, 16);
	brw_wm_write__mask(p, dispatch, mask, src);

	return true;
}

/* Projective variants */

static void brw_wm_projective_st(struct brw_compile *p, int dw,
				 int channel, int msg)
{
	int uv;

	if (dw == 16) {
		brw_set_compression_control(p, BRW_COMPRESSION_COMPRESSED);
		uv = p->gen >= 060 ? 6 : 3;
	} else {
		brw_set_compression_control(p, BRW_COMPRESSION_NONE);
		uv = p->gen >= 060 ? 4 : 3;
	}
	uv += 2*channel;

	msg++;
	if (p->gen >= 060) {
		/* First compute 1/z */
		brw_PLN(p,
			brw_vec8_grf(30, 0),
			brw_vec1_grf(uv+1, 0),
			brw_vec8_grf(2, 0));

		if (dw == 16) {
			brw_set_compression_control(p, BRW_COMPRESSION_NONE);
			brw_math_invert(p, brw_vec8_grf(30, 0), brw_vec8_grf(30, 0));
			brw_math_invert(p, brw_vec8_grf(31, 0), brw_vec8_grf(31, 0));
			brw_set_compression_control(p, BRW_COMPRESSION_COMPRESSED);
		} else
			brw_math_invert(p, brw_vec8_grf(30, 0), brw_vec8_grf(30, 0));

		brw_PLN(p,
			brw_vec8_grf(26, 0),
			brw_vec1_grf(uv, 0),
			brw_vec8_grf(2, 0));
		brw_PLN(p,
			brw_vec8_grf(28, 0),
			brw_vec1_grf(uv, 0),
			brw_vec8_grf(4, 0));

		brw_MUL(p,
			brw_message_reg(msg),
			brw_vec8_grf(26, 0),
			brw_vec8_grf(30, 0));
		brw_MUL(p,
			brw_message_reg(msg + dw/8),
			brw_vec8_grf(28, 0),
			brw_vec8_grf(30, 0));
	} else {
		struct brw_reg r = brw_vec1_grf(uv, 0);

		/* First compute 1/z */
		brw_LINE(p, brw_null_reg(), brw_vec1_grf(uv+1, 0), brw_vec8_grf(X16, 0));
		brw_MAC(p, brw_vec8_grf(30, 0), brw_vec1_grf(uv+1, 1), brw_vec8_grf(Y16, 0));

		if (dw == 16) {
			brw_set_compression_control(p, BRW_COMPRESSION_NONE);
			brw_math_invert(p, brw_vec8_grf(30, 0), brw_vec8_grf(30, 0));
			brw_math_invert(p, brw_vec8_grf(31, 0), brw_vec8_grf(31, 0));
			brw_set_compression_control(p, BRW_COMPRESSION_COMPRESSED);
		} else
			brw_math_invert(p, brw_vec8_grf(30, 0), brw_vec8_grf(30, 0));

		/* Now compute the output s,t values */
		brw_LINE(p, brw_null_reg(), __suboffset(r, 0), brw_vec8_grf(X16, 0));
		brw_MAC(p, brw_vec8_grf(28, 0), __suboffset(r, 1), brw_vec8_grf(Y16, 0));
		brw_MUL(p, brw_message_reg(msg), brw_vec8_grf(28, 0), brw_vec8_grf(30, 0));
		msg += dw/8;

		brw_LINE(p, brw_null_reg(), __suboffset(r, 4), brw_vec8_grf(X16, 0));
		brw_MAC(p, brw_vec8_grf(28, 0), __suboffset(r, 5), brw_vec8_grf(Y16, 0));
		brw_MUL(p, brw_message_reg(msg), brw_vec8_grf(28, 0), brw_vec8_grf(30, 0));
	}
}

static int brw_wm_projective(struct brw_compile *p, int dw,
			     int channel, int msg, int result)
{
	brw_wm_projective_st(p, dw, channel, msg);
	return brw_wm_sample(p, dw, channel, msg, result);
}

static int brw_wm_projective__alpha(struct brw_compile *p, int dw,
				     int channel, int msg, int result)
{
	brw_wm_projective_st(p, dw, channel, msg);
	return brw_wm_sample__alpha(p, dw, channel, msg, result);
}

bool
brw_wm_kernel__projective(struct brw_compile *p, int dispatch)
{
	if (p->gen < 060)
		brw_wm_xy(p, dispatch);
	brw_wm_write(p, dispatch, brw_wm_projective(p, dispatch, 0, 1, 12));

	return true;
}

bool
brw_wm_kernel__projective_mask(struct brw_compile *p, int dispatch)
{
	int src, mask;

	if (p->gen < 060)
		brw_wm_xy(p, dispatch);

	src = brw_wm_projective(p, dispatch, 0, 1, 12);
	mask = brw_wm_projective__alpha(p, dispatch, 1, 6, 20);
	brw_wm_write__mask(p, dispatch, src, mask);

	return true;
}

bool
brw_wm_kernel__projective_mask_ca(struct brw_compile *p, int dispatch)
{
	int src, mask;

	if (p->gen < 060)
		brw_wm_xy(p, dispatch);

	src = brw_wm_projective(p, dispatch, 0, 1, 12);
	mask = brw_wm_projective(p, dispatch, 1, 6, 20);
	brw_wm_write__mask_ca(p, dispatch, src, mask);

	return true;
}

bool
brw_wm_kernel__projective_mask_sa(struct brw_compile *p, int dispatch)
{
	int src, mask;

	if (p->gen < 060)
		brw_wm_xy(p, dispatch);

	src = brw_wm_projective__alpha(p, dispatch, 0, 1, 12);
	mask = brw_wm_projective(p, dispatch, 1, 6, 16);
	brw_wm_write__mask(p, dispatch, mask, src);

	return true;
}

bool
brw_wm_kernel__affine_opacity(struct brw_compile *p, int dispatch)
{
	int src, mask;

	if (p->gen < 060) {
		brw_wm_xy(p, dispatch);
		mask = 5;
	} else
		mask = dispatch == 16 ? 8 : 6;

	src = brw_wm_affine(p, dispatch, 0, 1, 12);
	brw_wm_write__opacity(p, dispatch, src, mask);

	return true;
}

bool
brw_wm_kernel__projective_opacity(struct brw_compile *p, int dispatch)
{
	int src, mask;

	if (p->gen < 060) {
		brw_wm_xy(p, dispatch);
		mask = 5;
	} else
		mask = dispatch == 16 ? 8 : 6;

	src = brw_wm_projective(p, dispatch, 0, 1, 12);
	brw_wm_write__opacity(p, dispatch, src, mask);

	return true;
}
