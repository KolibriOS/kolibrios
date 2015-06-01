/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "pipe/p_state.h"
#include "util/u_string.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "util/u_format.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_parse.h"

#include "freedreno_program.h"

#include "fd4_program.h"
#include "fd4_emit.h"
#include "fd4_texture.h"
#include "fd4_format.h"

static void
delete_shader_stateobj(struct fd4_shader_stateobj *so)
{
	ir3_shader_destroy(so->shader);
	free(so);
}

static struct fd4_shader_stateobj *
create_shader_stateobj(struct pipe_context *pctx, const struct pipe_shader_state *cso,
		enum shader_t type)
{
	struct fd4_shader_stateobj *so = CALLOC_STRUCT(fd4_shader_stateobj);
	so->shader = ir3_shader_create(pctx, cso->tokens, type);
	return so;
}

static void *
fd4_fp_state_create(struct pipe_context *pctx,
		const struct pipe_shader_state *cso)
{
	return create_shader_stateobj(pctx, cso, SHADER_FRAGMENT);
}

static void
fd4_fp_state_delete(struct pipe_context *pctx, void *hwcso)
{
	struct fd4_shader_stateobj *so = hwcso;
	delete_shader_stateobj(so);
}

static void *
fd4_vp_state_create(struct pipe_context *pctx,
		const struct pipe_shader_state *cso)
{
	return create_shader_stateobj(pctx, cso, SHADER_VERTEX);
}

static void
fd4_vp_state_delete(struct pipe_context *pctx, void *hwcso)
{
	struct fd4_shader_stateobj *so = hwcso;
	delete_shader_stateobj(so);
}

static void
emit_shader(struct fd_ringbuffer *ring, const struct ir3_shader_variant *so)
{
	const struct ir3_info *si = &so->info;
	enum adreno_state_block sb;
	enum adreno_state_src src;
	uint32_t i, sz, *bin;

	if (so->type == SHADER_VERTEX) {
		sb = SB_VERT_SHADER;
	} else {
		sb = SB_FRAG_SHADER;
	}

	if (fd_mesa_debug & FD_DBG_DIRECT) {
		sz = si->sizedwords;
		src = SS_DIRECT;
		bin = fd_bo_map(so->bo);
	} else {
		sz = 0;
		src = 2;  // enums different on a4xx..
		bin = NULL;
	}

	OUT_PKT3(ring, CP_LOAD_STATE, 2 + sz);
	OUT_RING(ring, CP_LOAD_STATE_0_DST_OFF(0) |
			CP_LOAD_STATE_0_STATE_SRC(src) |
			CP_LOAD_STATE_0_STATE_BLOCK(sb) |
			CP_LOAD_STATE_0_NUM_UNIT(so->instrlen));
	if (bin) {
		OUT_RING(ring, CP_LOAD_STATE_1_EXT_SRC_ADDR(0) |
				CP_LOAD_STATE_1_STATE_TYPE(ST_SHADER));
	} else {
		OUT_RELOC(ring, so->bo, 0,
				CP_LOAD_STATE_1_STATE_TYPE(ST_SHADER), 0);
	}
	for (i = 0; i < sz; i++) {
		OUT_RING(ring, bin[i]);
	}
}

struct stage {
	const struct ir3_shader_variant *v;
	const struct ir3_info *i;
	/* const sizes are in units of 4 * vec4 */
	uint8_t constoff;
	uint8_t constlen;
	/* instr sizes are in units of 16 instructions */
	uint8_t instroff;
	uint8_t instrlen;
};

enum {
	VS = 0,
	FS = 1,
	HS = 2,
	DS = 3,
	GS = 4,
	MAX_STAGES
};

static void
setup_stages(struct fd4_emit *emit, struct stage *s)
{
	unsigned i;

	s[VS].v = fd4_emit_get_vp(emit);

	if (emit->key.binning_pass) {
		/* use dummy stateobj to simplify binning vs non-binning: */
		static const struct ir3_shader_variant binning_fp = {};
		s[FS].v = &binning_fp;
	} else {
		s[FS].v = fd4_emit_get_fp(emit);
	}

	s[HS].v = s[DS].v = s[GS].v = NULL;  /* for now */

	for (i = 0; i < MAX_STAGES; i++) {
		if (s[i].v) {
			s[i].i = &s[i].v->info;
			/* constlen is in units of 4 * vec4: */
			s[i].constlen = align(s[i].v->constlen, 4) / 4;
			/* instrlen is already in units of 16 instr.. although
			 * probably we should ditch that and not make the compiler
			 * care about instruction group size of a3xx vs a4xx
			 */
			s[i].instrlen = s[i].v->instrlen;
		} else {
			s[i].i = NULL;
			s[i].constlen = 0;
			s[i].instrlen = 0;
		}
	}

	/* NOTE: at least for gles2, blob partitions VS at bottom of const
	 * space and FS taking entire remaining space.  We probably don't
	 * need to do that the same way, but for now mimic what the blob
	 * does to make it easier to diff against register values from blob
	 *
	 * NOTE: if VS.instrlen + FS.instrlen > 64, then one or both shaders
	 * is run from external memory.
	 */
	if ((s[VS].instrlen + s[FS].instrlen) > 64) {
		/* prioritize FS for internal memory: */
		if (s[FS].instrlen < 64) {
			/* if FS can fit, kick VS out to external memory: */
			s[VS].instrlen = 0;
		} else if (s[VS].instrlen < 64) {
			/* otherwise if VS can fit, kick out FS: */
			s[FS].instrlen = 0;
		} else {
			/* neither can fit, run both from external memory: */
			s[VS].instrlen = 0;
			s[FS].instrlen = 0;
		}
	}
	s[VS].constlen = 66;
	s[FS].constlen = 128 - s[VS].constlen;
	s[VS].instroff = 0;
	s[VS].constoff = 0;
	s[FS].instroff = 64 - s[FS].instrlen;
	s[FS].constoff = s[VS].constlen;
	s[HS].instroff = s[DS].instroff = s[GS].instroff = s[FS].instroff;
	s[HS].constoff = s[DS].constoff = s[GS].constoff = s[FS].constoff;
}

void
fd4_program_emit(struct fd_ringbuffer *ring, struct fd4_emit *emit)
{
	struct stage s[MAX_STAGES];
	uint32_t pos_regid, posz_regid, psize_regid, color_regid;
	uint32_t face_regid, coord_regid, zwcoord_regid;
	int constmode;
	int i, j, k;

	setup_stages(emit, s);

	/* blob seems to always use constmode currently: */
	constmode = 1;

	pos_regid = ir3_find_output_regid(s[VS].v,
		ir3_semantic_name(TGSI_SEMANTIC_POSITION, 0));
	posz_regid = ir3_find_output_regid(s[FS].v,
		ir3_semantic_name(TGSI_SEMANTIC_POSITION, 0));
	psize_regid = ir3_find_output_regid(s[VS].v,
		ir3_semantic_name(TGSI_SEMANTIC_PSIZE, 0));
	color_regid = ir3_find_output_regid(s[FS].v,
		ir3_semantic_name(TGSI_SEMANTIC_COLOR, 0));

	if (util_format_is_alpha(emit->pformat))
		color_regid += 3;

	/* TODO get these dynamically: */
	face_regid = s[FS].v->frag_face ? regid(0,0) : regid(63,0);
	coord_regid = s[FS].v->frag_coord ? regid(0,0) : regid(63,0);
	zwcoord_regid = s[FS].v->frag_coord ? regid(0,2) : regid(63,0);

	/* we could probably divide this up into things that need to be
	 * emitted if frag-prog is dirty vs if vert-prog is dirty..
	 */

	OUT_PKT0(ring, REG_A4XX_HLSQ_UPDATE_CONTROL, 1);
	OUT_RING(ring, 0x00000003);

	OUT_PKT0(ring, REG_A4XX_HLSQ_CONTROL_0_REG, 5);
	OUT_RING(ring, A4XX_HLSQ_CONTROL_0_REG_FSTHREADSIZE(FOUR_QUADS) |
			A4XX_HLSQ_CONTROL_0_REG_CONSTMODE(constmode) |
			A4XX_HLSQ_CONTROL_0_REG_FSSUPERTHREADENABLE |
			/* NOTE:  I guess SHADERRESTART and CONSTFULLUPDATE maybe
			 * flush some caches? I think we only need to set those
			 * bits if we have updated const or shader..
			 */
			A4XX_HLSQ_CONTROL_0_REG_SPSHADERRESTART |
			A4XX_HLSQ_CONTROL_0_REG_SPCONSTFULLUPDATE);
	OUT_RING(ring, A4XX_HLSQ_CONTROL_1_REG_VSTHREADSIZE(TWO_QUADS) |
			A4XX_HLSQ_CONTROL_1_REG_VSSUPERTHREADENABLE |
			A4XX_HLSQ_CONTROL_1_REG_COORDREGID(coord_regid) |
			A4XX_HLSQ_CONTROL_1_REG_ZWCOORDREGID(zwcoord_regid));
	OUT_RING(ring, A4XX_HLSQ_CONTROL_2_REG_PRIMALLOCTHRESHOLD(63) |
			0x3f3f000 |           /* XXX */
			A4XX_HLSQ_CONTROL_2_REG_FACEREGID(face_regid));
	OUT_RING(ring, A4XX_HLSQ_CONTROL_3_REG_REGID(s[FS].v->pos_regid) |
			0xfcfcfc00);
	OUT_RING(ring, 0x00fcfcfc);   /* XXX HLSQ_CONTROL_4 */

	OUT_PKT0(ring, REG_A4XX_HLSQ_VS_CONTROL_REG, 5);
	OUT_RING(ring, A4XX_HLSQ_VS_CONTROL_REG_CONSTLENGTH(s[VS].constlen) |
			A4XX_HLSQ_VS_CONTROL_REG_CONSTOBJECTOFFSET(s[VS].constoff) |
			A4XX_HLSQ_VS_CONTROL_REG_INSTRLENGTH(s[VS].instrlen) |
			A4XX_HLSQ_VS_CONTROL_REG_SHADEROBJOFFSET(s[VS].instroff));
	OUT_RING(ring, A4XX_HLSQ_FS_CONTROL_REG_CONSTLENGTH(s[FS].constlen) |
			A4XX_HLSQ_FS_CONTROL_REG_CONSTOBJECTOFFSET(s[FS].constoff) |
			A4XX_HLSQ_FS_CONTROL_REG_INSTRLENGTH(s[FS].instrlen) |
			A4XX_HLSQ_FS_CONTROL_REG_SHADEROBJOFFSET(s[FS].instroff));
	OUT_RING(ring, A4XX_HLSQ_HS_CONTROL_REG_CONSTLENGTH(s[HS].constlen) |
			A4XX_HLSQ_HS_CONTROL_REG_CONSTOBJECTOFFSET(s[HS].constoff) |
			A4XX_HLSQ_HS_CONTROL_REG_INSTRLENGTH(s[HS].instrlen) |
			A4XX_HLSQ_HS_CONTROL_REG_SHADEROBJOFFSET(s[HS].instroff));
	OUT_RING(ring, A4XX_HLSQ_DS_CONTROL_REG_CONSTLENGTH(s[DS].constlen) |
			A4XX_HLSQ_DS_CONTROL_REG_CONSTOBJECTOFFSET(s[DS].constoff) |
			A4XX_HLSQ_DS_CONTROL_REG_INSTRLENGTH(s[DS].instrlen) |
			A4XX_HLSQ_DS_CONTROL_REG_SHADEROBJOFFSET(s[DS].instroff));
	OUT_RING(ring, A4XX_HLSQ_GS_CONTROL_REG_CONSTLENGTH(s[GS].constlen) |
			A4XX_HLSQ_GS_CONTROL_REG_CONSTOBJECTOFFSET(s[GS].constoff) |
			A4XX_HLSQ_GS_CONTROL_REG_INSTRLENGTH(s[GS].instrlen) |
			A4XX_HLSQ_GS_CONTROL_REG_SHADEROBJOFFSET(s[GS].instroff));

	OUT_PKT0(ring, REG_A4XX_SP_SP_CTRL_REG, 1);
	OUT_RING(ring, 0x140010 | /* XXX */
			COND(emit->key.binning_pass, A4XX_SP_SP_CTRL_REG_BINNING_PASS));

	OUT_PKT0(ring, REG_A4XX_SP_INSTR_CACHE_CTRL, 1);
	OUT_RING(ring, 0x7f | /* XXX */
			COND(s[VS].instrlen, A4XX_SP_INSTR_CACHE_CTRL_VS_BUFFER) |
			COND(s[FS].instrlen, A4XX_SP_INSTR_CACHE_CTRL_FS_BUFFER) |
			COND(s[VS].instrlen && s[FS].instrlen,
					A4XX_SP_INSTR_CACHE_CTRL_INSTR_BUFFER));

	OUT_PKT0(ring, REG_A4XX_SP_VS_LENGTH_REG, 1);
	OUT_RING(ring, s[VS].v->instrlen);      /* SP_VS_LENGTH_REG */

	OUT_PKT0(ring, REG_A4XX_SP_VS_CTRL_REG0, 3);
	OUT_RING(ring, A4XX_SP_VS_CTRL_REG0_THREADMODE(MULTI) |
			A4XX_SP_VS_CTRL_REG0_HALFREGFOOTPRINT(s[VS].i->max_half_reg + 1) |
			A4XX_SP_VS_CTRL_REG0_FULLREGFOOTPRINT(s[VS].i->max_reg + 1) |
			A4XX_SP_VS_CTRL_REG0_INOUTREGOVERLAP(0) |
			A4XX_SP_VS_CTRL_REG0_THREADSIZE(TWO_QUADS) |
			A4XX_SP_VS_CTRL_REG0_SUPERTHREADMODE |
			COND(s[VS].v->has_samp, A4XX_SP_VS_CTRL_REG0_PIXLODENABLE));
	OUT_RING(ring, A4XX_SP_VS_CTRL_REG1_CONSTLENGTH(s[VS].constlen) |
			A4XX_SP_VS_CTRL_REG1_INITIALOUTSTANDING(s[VS].v->total_in));
	OUT_RING(ring, A4XX_SP_VS_PARAM_REG_POSREGID(pos_regid) |
			A4XX_SP_VS_PARAM_REG_PSIZEREGID(psize_regid) |
			A4XX_SP_VS_PARAM_REG_TOTALVSOUTVAR(align(s[FS].v->total_in, 4) / 4));

	for (i = 0, j = -1; (i < 16) && (j < (int)s[FS].v->inputs_count); i++) {
		uint32_t reg = 0;

		OUT_PKT0(ring, REG_A4XX_SP_VS_OUT_REG(i), 1);

		j = ir3_next_varying(s[FS].v, j);
		if (j < s[FS].v->inputs_count) {
			k = ir3_find_output(s[VS].v, s[FS].v->inputs[j].semantic);
			reg |= A4XX_SP_VS_OUT_REG_A_REGID(s[VS].v->outputs[k].regid);
			reg |= A4XX_SP_VS_OUT_REG_A_COMPMASK(s[FS].v->inputs[j].compmask);
		}

		j = ir3_next_varying(s[FS].v, j);
		if (j < s[FS].v->inputs_count) {
			k = ir3_find_output(s[VS].v, s[FS].v->inputs[j].semantic);
			reg |= A4XX_SP_VS_OUT_REG_B_REGID(s[VS].v->outputs[k].regid);
			reg |= A4XX_SP_VS_OUT_REG_B_COMPMASK(s[FS].v->inputs[j].compmask);
		}

		OUT_RING(ring, reg);
	}

	for (i = 0, j = -1; (i < 8) && (j < (int)s[FS].v->inputs_count); i++) {
		uint32_t reg = 0;

		OUT_PKT0(ring, REG_A4XX_SP_VS_VPC_DST_REG(i), 1);

		j = ir3_next_varying(s[FS].v, j);
		if (j < s[FS].v->inputs_count)
			reg |= A4XX_SP_VS_VPC_DST_REG_OUTLOC0(s[FS].v->inputs[j].inloc);
		j = ir3_next_varying(s[FS].v, j);
		if (j < s[FS].v->inputs_count)
			reg |= A4XX_SP_VS_VPC_DST_REG_OUTLOC1(s[FS].v->inputs[j].inloc);
		j = ir3_next_varying(s[FS].v, j);
		if (j < s[FS].v->inputs_count)
			reg |= A4XX_SP_VS_VPC_DST_REG_OUTLOC2(s[FS].v->inputs[j].inloc);
		j = ir3_next_varying(s[FS].v, j);
		if (j < s[FS].v->inputs_count)
			reg |= A4XX_SP_VS_VPC_DST_REG_OUTLOC3(s[FS].v->inputs[j].inloc);

		OUT_RING(ring, reg);
	}

	OUT_PKT0(ring, REG_A4XX_SP_VS_OBJ_OFFSET_REG, 2);
	OUT_RING(ring, A4XX_SP_VS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[VS].constoff) |
			A4XX_SP_VS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[VS].instroff));
	OUT_RELOC(ring, s[VS].v->bo, 0, 0, 0);  /* SP_VS_OBJ_START_REG */

	OUT_PKT0(ring, REG_A4XX_SP_FS_LENGTH_REG, 1);
	OUT_RING(ring, s[FS].v->instrlen);  /* SP_FS_LENGTH_REG */

	OUT_PKT0(ring, REG_A4XX_SP_FS_CTRL_REG0, 2);
	OUT_RING(ring, A4XX_SP_FS_CTRL_REG0_THREADMODE(MULTI) |
			COND(s[FS].v->total_in > 0, A4XX_SP_FS_CTRL_REG0_VARYING) |
			A4XX_SP_FS_CTRL_REG0_HALFREGFOOTPRINT(s[FS].i->max_half_reg + 1) |
			A4XX_SP_FS_CTRL_REG0_FULLREGFOOTPRINT(s[FS].i->max_reg + 1) |
			A4XX_SP_FS_CTRL_REG0_INOUTREGOVERLAP(1) |
			A4XX_SP_FS_CTRL_REG0_THREADSIZE(FOUR_QUADS) |
			A4XX_SP_FS_CTRL_REG0_SUPERTHREADMODE |
			COND(s[FS].v->has_samp, A4XX_SP_FS_CTRL_REG0_PIXLODENABLE));
	OUT_RING(ring, A4XX_SP_FS_CTRL_REG1_CONSTLENGTH(s[FS].constlen) |
			0x80000000 |      /* XXX */
			COND(s[FS].v->frag_face, A4XX_SP_FS_CTRL_REG1_FACENESS) |
			COND(s[FS].v->total_in > 0, A4XX_SP_FS_CTRL_REG1_VARYING) |
			COND(s[FS].v->frag_coord, A4XX_SP_FS_CTRL_REG1_FRAGCOORD));

	OUT_PKT0(ring, REG_A4XX_SP_FS_OBJ_OFFSET_REG, 2);
	OUT_RING(ring, A4XX_SP_FS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[FS].constoff) |
			A4XX_SP_FS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[FS].instroff));
	if (emit->key.binning_pass)
		OUT_RING(ring, 0x00000000);
	else
		OUT_RELOC(ring, s[FS].v->bo, 0, 0, 0);  /* SP_FS_OBJ_START_REG */

	OUT_PKT0(ring, REG_A4XX_SP_HS_OBJ_OFFSET_REG, 1);
	OUT_RING(ring, A4XX_SP_HS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[HS].constoff) |
			A4XX_SP_HS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[HS].instroff));

	OUT_PKT0(ring, REG_A4XX_SP_DS_OBJ_OFFSET_REG, 1);
	OUT_RING(ring, A4XX_SP_DS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[DS].constoff) |
			A4XX_SP_DS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[DS].instroff));

	OUT_PKT0(ring, REG_A4XX_SP_GS_OBJ_OFFSET_REG, 1);
	OUT_RING(ring, A4XX_SP_GS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[GS].constoff) |
			A4XX_SP_GS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[GS].instroff));

	OUT_PKT0(ring, REG_A4XX_RB_RENDER_CONTROL2, 1);
	OUT_RING(ring, A4XX_RB_RENDER_CONTROL2_MSAA_SAMPLES(0) |
			COND(s[FS].v->total_in > 0, A4XX_RB_RENDER_CONTROL2_VARYING) |
			COND(s[FS].v->frag_face, A4XX_RB_RENDER_CONTROL2_FACENESS) |
			COND(s[FS].v->frag_coord, A4XX_RB_RENDER_CONTROL2_XCOORD |
					A4XX_RB_RENDER_CONTROL2_YCOORD |
// TODO enabling gl_FragCoord.z is causing lockups on 0ad (but seems
// to work everywhere else).
//					A4XX_RB_RENDER_CONTROL2_ZCOORD |
					A4XX_RB_RENDER_CONTROL2_WCOORD));

	OUT_PKT0(ring, REG_A4XX_RB_FS_OUTPUT_REG, 1);
	OUT_RING(ring, A4XX_RB_FS_OUTPUT_REG_MRT(1) |
			COND(s[FS].v->writes_pos, A4XX_RB_FS_OUTPUT_REG_FRAG_WRITES_Z));

	OUT_PKT0(ring, REG_A4XX_SP_FS_OUTPUT_REG, 1);
	if (s[FS].v->writes_pos) {
		OUT_RING(ring, 0x00000001 |
				A4XX_SP_FS_OUTPUT_REG_DEPTH_ENABLE |
				A4XX_SP_FS_OUTPUT_REG_DEPTH_REGID(posz_regid));
	} else {
		OUT_RING(ring, 0x00000001);
	}

	OUT_PKT0(ring, REG_A4XX_SP_FS_MRT_REG(0), 8);
	OUT_RING(ring, A4XX_SP_FS_MRT_REG_REGID(color_regid) |
			A4XX_SP_FS_MRT_REG_MRTFORMAT(emit->format) |
			COND(emit->key.half_precision, A4XX_SP_FS_MRT_REG_HALF_PRECISION));
	OUT_RING(ring, A4XX_SP_FS_MRT_REG_REGID(0));
	OUT_RING(ring, A4XX_SP_FS_MRT_REG_REGID(0));
	OUT_RING(ring, A4XX_SP_FS_MRT_REG_REGID(0));
	OUT_RING(ring, A4XX_SP_FS_MRT_REG_REGID(0));
	OUT_RING(ring, A4XX_SP_FS_MRT_REG_REGID(0));
	OUT_RING(ring, A4XX_SP_FS_MRT_REG_REGID(0));
	OUT_RING(ring, A4XX_SP_FS_MRT_REG_REGID(0));

	if (emit->key.binning_pass) {
		OUT_PKT0(ring, REG_A4XX_VPC_ATTR, 2);
		OUT_RING(ring, A4XX_VPC_ATTR_THRDASSIGN(1) |
				0x40000000 |      /* XXX */
				COND(s[VS].v->writes_psize, A4XX_VPC_ATTR_PSIZE));
		OUT_RING(ring, 0x00000000);
	} else {
		uint32_t vinterp[8], flatshade[2];

		memset(vinterp, 0, sizeof(vinterp));
		memset(flatshade, 0, sizeof(flatshade));

		/* looks like we need to do int varyings in the frag
		 * shader on a4xx (no flatshad reg?  or a420.0 bug?):
		 *
		 *    (sy)(ss)nop
		 *    (sy)ldlv.u32 r0.x,l[r0.x], 1
		 *    ldlv.u32 r0.y,l[r0.x+1], 1
		 *    (ss)bary.f (ei)r63.x, 0, r0.x
		 *    (ss)(rpt1)cov.s32f16 hr0.x, (r)r0.x
		 *    (rpt5)nop
		 *    sam (f16)(xyzw)hr0.x, hr0.x, s#0, t#0
		 *
		 * Possibly on later a4xx variants we'll be able to use
		 * something like the code below instead of workaround
		 * in the shader:
		 */
#if 0
		/* figure out VARYING_INTERP / FLAT_SHAD register values: */
		for (j = -1; (j = ir3_next_varying(s[FS].v, j)) < (int)s[FS].v->inputs_count; ) {
			uint32_t interp = s[FS].v->inputs[j].interpolate;
			if ((interp == TGSI_INTERPOLATE_CONSTANT) ||
					((interp == TGSI_INTERPOLATE_COLOR) && emit->rasterflat)) {
				/* TODO might be cleaner to just +8 in SP_VS_VPC_DST_REG
				 * instead.. rather than -8 everywhere else..
				 */
				uint32_t loc = s[FS].v->inputs[j].inloc - 8;

				/* currently assuming varyings aligned to 4 (not
				 * packed):
				 */
				debug_assert((loc % 4) == 0);

				for (i = 0; i < 4; i++, loc++) {
					vinterp[loc / 16] |= 1 << ((loc % 16) * 2);
					flatshade[loc / 32] |= 1 << (loc % 32);
				}
			}
		}
#endif

		OUT_PKT0(ring, REG_A4XX_VPC_ATTR, 2);
		OUT_RING(ring, A4XX_VPC_ATTR_TOTALATTR(s[FS].v->total_in) |
				A4XX_VPC_ATTR_THRDASSIGN(1) |
				COND(s[FS].v->total_in > 0, A4XX_VPC_ATTR_ENABLE) |
				0x40000000 |      /* XXX */
				COND(s[VS].v->writes_psize, A4XX_VPC_ATTR_PSIZE));
		OUT_RING(ring, A4XX_VPC_PACK_NUMFPNONPOSVAR(s[FS].v->total_in) |
				A4XX_VPC_PACK_NUMNONPOSVSVAR(s[FS].v->total_in));

		OUT_PKT0(ring, REG_A4XX_VPC_VARYING_INTERP_MODE(0), 8);
		for (i = 0; i < 8; i++)
			OUT_RING(ring, vinterp[i]);     /* VPC_VARYING_INTERP[i].MODE */

		OUT_PKT0(ring, REG_A4XX_VPC_VARYING_PS_REPL_MODE(0), 8);
		for (i = 0; i < 8; i++)
			OUT_RING(ring, s[FS].v->shader->vpsrepl[i]);   /* VPC_VARYING_PS_REPL[i] */
	}

	if (s[VS].instrlen)
		emit_shader(ring, s[VS].v);

	if (!emit->key.binning_pass)
		if (s[FS].instrlen)
			emit_shader(ring, s[FS].v);
}

/* hack.. until we figure out how to deal w/ vpsrepl properly.. */
static void
fix_blit_fp(struct pipe_context *pctx)
{
	struct fd_context *ctx = fd_context(pctx);
	struct fd4_shader_stateobj *so = ctx->blit_prog[0].fp;

	so->shader->vpsrepl[0] = 0x99999999;
	so->shader->vpsrepl[1] = 0x99999999;
	so->shader->vpsrepl[2] = 0x99999999;
	so->shader->vpsrepl[3] = 0x99999999;
}

void
fd4_prog_init(struct pipe_context *pctx)
{
	pctx->create_fs_state = fd4_fp_state_create;
	pctx->delete_fs_state = fd4_fp_state_delete;

	pctx->create_vs_state = fd4_vp_state_create;
	pctx->delete_vs_state = fd4_vp_state_delete;

	fd_prog_init(pctx);

	fix_blit_fp(pctx);
}
