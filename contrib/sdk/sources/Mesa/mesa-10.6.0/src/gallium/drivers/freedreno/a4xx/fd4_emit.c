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
#include "util/u_helpers.h"
#include "util/u_format.h"

#include "freedreno_resource.h"

#include "fd4_emit.h"
#include "fd4_blend.h"
#include "fd4_context.h"
#include "fd4_program.h"
#include "fd4_rasterizer.h"
#include "fd4_texture.h"
#include "fd4_format.h"
#include "fd4_zsa.h"

/* regid:          base const register
 * prsc or dwords: buffer containing constant values
 * sizedwords:     size of const value buffer
 */
void
fd4_emit_constant(struct fd_ringbuffer *ring,
		enum adreno_state_block sb,
		uint32_t regid, uint32_t offset, uint32_t sizedwords,
		const uint32_t *dwords, struct pipe_resource *prsc)
{
	uint32_t i, sz;
	enum adreno_state_src src;

	if (prsc) {
		sz = 0;
		src = 0x2;  // TODO ??
	} else {
		sz = sizedwords;
		src = SS_DIRECT;
	}

	OUT_PKT3(ring, CP_LOAD_STATE, 2 + sz);
	OUT_RING(ring, CP_LOAD_STATE_0_DST_OFF(regid/4) |
			CP_LOAD_STATE_0_STATE_SRC(src) |
			CP_LOAD_STATE_0_STATE_BLOCK(sb) |
			CP_LOAD_STATE_0_NUM_UNIT(sizedwords/4));
	if (prsc) {
		struct fd_bo *bo = fd_resource(prsc)->bo;
		OUT_RELOC(ring, bo, offset,
				CP_LOAD_STATE_1_STATE_TYPE(ST_CONSTANTS), 0);
	} else {
		OUT_RING(ring, CP_LOAD_STATE_1_EXT_SRC_ADDR(0) |
				CP_LOAD_STATE_1_STATE_TYPE(ST_CONSTANTS));
		dwords = (uint32_t *)&((uint8_t *)dwords)[offset];
	}
	for (i = 0; i < sz; i++) {
		OUT_RING(ring, dwords[i]);
	}
}

static void
emit_constants(struct fd_ringbuffer *ring,
		enum adreno_state_block sb,
		struct fd_constbuf_stateobj *constbuf,
		struct ir3_shader_variant *shader,
		bool emit_immediates)
{
	uint32_t enabled_mask = constbuf->enabled_mask;
	uint32_t max_const;
	int i;

	// XXX TODO only emit dirty consts.. but we need to keep track if
	// they are clobbered by a clear, gmem2mem, or mem2gmem..
	constbuf->dirty_mask = enabled_mask;

	/* in particular, with binning shader we may end up with unused
	 * consts, ie. we could end up w/ constlen that is smaller
	 * than first_immediate.  In that case truncate the user consts
	 * early to avoid HLSQ lockup caused by writing too many consts
	 */
	max_const = MIN2(shader->first_driver_param, shader->constlen);

	/* emit user constants: */
	if (enabled_mask & 1) {
		const unsigned index = 0;
		struct pipe_constant_buffer *cb = &constbuf->cb[index];
		unsigned size = align(cb->buffer_size, 4) / 4; /* size in dwords */

		// I expect that size should be a multiple of vec4's:
		assert(size == align(size, 4));

		/* and even if the start of the const buffer is before
		 * first_immediate, the end may not be:
		 */
		size = MIN2(size, 4 * max_const);

		if (size && (constbuf->dirty_mask & (1 << index))) {
			fd4_emit_constant(ring, sb, 0,
					cb->buffer_offset, size,
					cb->user_buffer, cb->buffer);
			constbuf->dirty_mask &= ~(1 << index);
		}

		enabled_mask &= ~(1 << index);
	}

	/* emit ubos: */
	if (shader->constlen > shader->first_driver_param) {
		uint32_t params = MIN2(4, shader->constlen - shader->first_driver_param);
		OUT_PKT3(ring, CP_LOAD_STATE, 2 + params * 4);
		OUT_RING(ring, CP_LOAD_STATE_0_DST_OFF(shader->first_driver_param) |
				CP_LOAD_STATE_0_STATE_SRC(SS_DIRECT) |
				CP_LOAD_STATE_0_STATE_BLOCK(sb) |
				CP_LOAD_STATE_0_NUM_UNIT(params));
		OUT_RING(ring, CP_LOAD_STATE_1_EXT_SRC_ADDR(0) |
				CP_LOAD_STATE_1_STATE_TYPE(ST_CONSTANTS));

		for (i = 1; i <= params * 4; i++) {
			struct pipe_constant_buffer *cb = &constbuf->cb[i];
			assert(!cb->user_buffer);
			if ((enabled_mask & (1 << i)) && cb->buffer)
				OUT_RELOC(ring, fd_resource(cb->buffer)->bo, cb->buffer_offset, 0, 0);
			else
				OUT_RING(ring, 0xbad00000 | ((i - 1) << 16));
		}
	}

	/* emit shader immediates: */
	if (shader && emit_immediates) {
		int size = shader->immediates_count;
		uint32_t base = shader->first_immediate;

		/* truncate size to avoid writing constants that shader
		 * does not use:
		 */
		size = MIN2(size + base, shader->constlen) - base;

		/* convert out of vec4: */
		base *= 4;
		size *= 4;

		if (size > 0) {
			fd4_emit_constant(ring, sb, base,
				0, size, shader->immediates[0].val, NULL);
		}
	}
}

static void
emit_textures(struct fd_context *ctx, struct fd_ringbuffer *ring,
		enum adreno_state_block sb, struct fd_texture_stateobj *tex)
{
	unsigned i;

	if (tex->num_samplers > 0) {
		int num_samplers;

		/* not sure if this is an a420.0 workaround, but we seem
		 * to need to emit these in pairs.. emit a final dummy
		 * entry if odd # of samplers:
		 */
		num_samplers = align(tex->num_samplers, 2);

		/* output sampler state: */
		OUT_PKT3(ring, CP_LOAD_STATE, 2 + (2 * num_samplers));
		OUT_RING(ring, CP_LOAD_STATE_0_DST_OFF(0) |
				CP_LOAD_STATE_0_STATE_SRC(SS_DIRECT) |
				CP_LOAD_STATE_0_STATE_BLOCK(sb) |
				CP_LOAD_STATE_0_NUM_UNIT(num_samplers));
		OUT_RING(ring, CP_LOAD_STATE_1_STATE_TYPE(ST_SHADER) |
				CP_LOAD_STATE_1_EXT_SRC_ADDR(0));
		for (i = 0; i < tex->num_samplers; i++) {
			static const struct fd4_sampler_stateobj dummy_sampler = {};
			const struct fd4_sampler_stateobj *sampler = tex->samplers[i] ?
					fd4_sampler_stateobj(tex->samplers[i]) :
					&dummy_sampler;
			OUT_RING(ring, sampler->texsamp0);
			OUT_RING(ring, sampler->texsamp1);
		}

		for (; i < num_samplers; i++) {
			OUT_RING(ring, 0x00000000);
			OUT_RING(ring, 0x00000000);
		}
	}

	if (tex->num_textures > 0) {
		/* emit texture state: */
		OUT_PKT3(ring, CP_LOAD_STATE, 2 + (8 * tex->num_textures));
		OUT_RING(ring, CP_LOAD_STATE_0_DST_OFF(0) |
				CP_LOAD_STATE_0_STATE_SRC(SS_DIRECT) |
				CP_LOAD_STATE_0_STATE_BLOCK(sb) |
				CP_LOAD_STATE_0_NUM_UNIT(tex->num_textures));
		OUT_RING(ring, CP_LOAD_STATE_1_STATE_TYPE(ST_CONSTANTS) |
				CP_LOAD_STATE_1_EXT_SRC_ADDR(0));
		for (i = 0; i < tex->num_textures; i++) {
			static const struct fd4_pipe_sampler_view dummy_view = {};
			const struct fd4_pipe_sampler_view *view = tex->textures[i] ?
					fd4_pipe_sampler_view(tex->textures[i]) :
					&dummy_view;
			struct fd_resource *rsc = fd_resource(view->base.texture);
			unsigned start = view->base.u.tex.first_level;
			uint32_t offset = fd_resource_offset(rsc, start, 0);

			OUT_RING(ring, view->texconst0);
			OUT_RING(ring, view->texconst1);
			OUT_RING(ring, view->texconst2);
			OUT_RING(ring, view->texconst3);
			OUT_RELOC(ring, rsc->bo, offset, view->textconst4, 0);
			OUT_RING(ring, 0x00000000);
			OUT_RING(ring, 0x00000000);
			OUT_RING(ring, 0x00000000);
		}
	}
}

/* emit texture state for mem->gmem restore operation.. eventually it would
 * be good to get rid of this and use normal CSO/etc state for more of these
 * special cases..
 */
void
fd4_emit_gmem_restore_tex(struct fd_ringbuffer *ring, struct pipe_surface *psurf)
{
	struct fd_resource *rsc = fd_resource(psurf->texture);
	unsigned lvl = psurf->u.tex.level;
	struct fd_resource_slice *slice = fd_resource_slice(rsc, lvl);
	uint32_t offset = fd_resource_offset(rsc, lvl, psurf->u.tex.first_layer);
	enum pipe_format format = fd4_gmem_restore_format(psurf->format);

	debug_assert(psurf->u.tex.first_layer == psurf->u.tex.last_layer);

	/* output sampler state: */
	OUT_PKT3(ring, CP_LOAD_STATE, 4);
	OUT_RING(ring, CP_LOAD_STATE_0_DST_OFF(0) |
			CP_LOAD_STATE_0_STATE_SRC(SS_DIRECT) |
			CP_LOAD_STATE_0_STATE_BLOCK(SB_FRAG_TEX) |
			CP_LOAD_STATE_0_NUM_UNIT(1));
	OUT_RING(ring, CP_LOAD_STATE_1_STATE_TYPE(ST_SHADER) |
			CP_LOAD_STATE_1_EXT_SRC_ADDR(0));
	OUT_RING(ring, A4XX_TEX_SAMP_0_XY_MAG(A4XX_TEX_NEAREST) |
			A4XX_TEX_SAMP_0_XY_MIN(A4XX_TEX_NEAREST) |
			A4XX_TEX_SAMP_0_WRAP_S(A4XX_TEX_CLAMP_TO_EDGE) |
			A4XX_TEX_SAMP_0_WRAP_T(A4XX_TEX_CLAMP_TO_EDGE) |
			A4XX_TEX_SAMP_0_WRAP_R(A4XX_TEX_REPEAT));
	OUT_RING(ring, 0x00000000);

	/* emit texture state: */
	OUT_PKT3(ring, CP_LOAD_STATE, 10);
	OUT_RING(ring, CP_LOAD_STATE_0_DST_OFF(0) |
			CP_LOAD_STATE_0_STATE_SRC(SS_DIRECT) |
			CP_LOAD_STATE_0_STATE_BLOCK(SB_FRAG_TEX) |
			CP_LOAD_STATE_0_NUM_UNIT(1));
	OUT_RING(ring, CP_LOAD_STATE_1_STATE_TYPE(ST_CONSTANTS) |
			CP_LOAD_STATE_1_EXT_SRC_ADDR(0));
	OUT_RING(ring, A4XX_TEX_CONST_0_FMT(fd4_pipe2tex(format)) |
			A4XX_TEX_CONST_0_TYPE(A4XX_TEX_2D) |
			fd4_tex_swiz(format,  PIPE_SWIZZLE_RED, PIPE_SWIZZLE_GREEN,
					PIPE_SWIZZLE_BLUE, PIPE_SWIZZLE_ALPHA));
	OUT_RING(ring, A4XX_TEX_CONST_1_WIDTH(psurf->width) |
			A4XX_TEX_CONST_1_HEIGHT(psurf->height));
	OUT_RING(ring, A4XX_TEX_CONST_2_PITCH(slice->pitch * rsc->cpp));
	OUT_RING(ring, 0x00000000);
	OUT_RELOC(ring, rsc->bo, offset, 0, 0);
	OUT_RING(ring, 0x00000000);
	OUT_RING(ring, 0x00000000);
	OUT_RING(ring, 0x00000000);
}

void
fd4_emit_vertex_bufs(struct fd_ringbuffer *ring, struct fd4_emit *emit)
{
	int32_t i, j, last = -1;
	uint32_t total_in = 0;
	const struct fd_vertex_state *vtx = emit->vtx;
	struct ir3_shader_variant *vp = fd4_emit_get_vp(emit);
	unsigned vertex_regid = regid(63, 0), instance_regid = regid(63, 0);

	for (i = 0; i < vp->inputs_count; i++) {
		uint8_t semantic = sem2name(vp->inputs[i].semantic);
		if (semantic == TGSI_SEMANTIC_VERTEXID_NOBASE)
			vertex_regid = vp->inputs[i].regid;
		else if (semantic == TGSI_SEMANTIC_INSTANCEID)
			instance_regid = vp->inputs[i].regid;
		else if ((i < vtx->vtx->num_elements) && vp->inputs[i].compmask)
			last = i;
	}

	/* hw doesn't like to be configured for zero vbo's, it seems: */
	if ((vtx->vtx->num_elements == 0) &&
			(vertex_regid == regid(63, 0)) &&
			(instance_regid == regid(63, 0)))
		return;

	for (i = 0, j = 0; i <= last; i++) {
		assert(sem2name(vp->inputs[i].semantic) == 0);
		if (vp->inputs[i].compmask) {
			struct pipe_vertex_element *elem = &vtx->vtx->pipe[i];
			const struct pipe_vertex_buffer *vb =
					&vtx->vertexbuf.vb[elem->vertex_buffer_index];
			struct fd_resource *rsc = fd_resource(vb->buffer);
			enum pipe_format pfmt = elem->src_format;
			enum a4xx_vtx_fmt fmt = fd4_pipe2vtx(pfmt);
			bool switchnext = (i != last) ||
					(vertex_regid != regid(63, 0)) ||
					(instance_regid != regid(63, 0));
			bool isint = util_format_is_pure_integer(pfmt);
			uint32_t fs = util_format_get_blocksize(pfmt);
			uint32_t off = vb->buffer_offset + elem->src_offset;
			uint32_t size = fd_bo_size(rsc->bo) - off;
			debug_assert(fmt != ~0);

			OUT_PKT0(ring, REG_A4XX_VFD_FETCH(j), 4);
			OUT_RING(ring, A4XX_VFD_FETCH_INSTR_0_FETCHSIZE(fs - 1) |
					A4XX_VFD_FETCH_INSTR_0_BUFSTRIDE(vb->stride) |
					COND(elem->instance_divisor, A4XX_VFD_FETCH_INSTR_0_INSTANCED) |
					COND(switchnext, A4XX_VFD_FETCH_INSTR_0_SWITCHNEXT));
			OUT_RELOC(ring, rsc->bo, off, 0, 0);
			OUT_RING(ring, A4XX_VFD_FETCH_INSTR_2_SIZE(size));
			OUT_RING(ring, A4XX_VFD_FETCH_INSTR_3_STEPRATE(MAX2(1, elem->instance_divisor)));

			OUT_PKT0(ring, REG_A4XX_VFD_DECODE_INSTR(j), 1);
			OUT_RING(ring, A4XX_VFD_DECODE_INSTR_CONSTFILL |
					A4XX_VFD_DECODE_INSTR_WRITEMASK(vp->inputs[i].compmask) |
					A4XX_VFD_DECODE_INSTR_FORMAT(fmt) |
					A4XX_VFD_DECODE_INSTR_SWAP(fd4_pipe2swap(pfmt)) |
					A4XX_VFD_DECODE_INSTR_REGID(vp->inputs[i].regid) |
					A4XX_VFD_DECODE_INSTR_SHIFTCNT(fs) |
					A4XX_VFD_DECODE_INSTR_LASTCOMPVALID |
					COND(isint, A4XX_VFD_DECODE_INSTR_INT) |
					COND(switchnext, A4XX_VFD_DECODE_INSTR_SWITCHNEXT));

			total_in += vp->inputs[i].ncomp;
			j++;
		}
	}

	OUT_PKT0(ring, REG_A4XX_VFD_CONTROL_0, 5);
	OUT_RING(ring, A4XX_VFD_CONTROL_0_TOTALATTRTOVS(total_in) |
			0xa0000 | /* XXX */
			A4XX_VFD_CONTROL_0_STRMDECINSTRCNT(j) |
			A4XX_VFD_CONTROL_0_STRMFETCHINSTRCNT(j));
	OUT_RING(ring, A4XX_VFD_CONTROL_1_MAXSTORAGE(129) | // XXX
			A4XX_VFD_CONTROL_1_REGID4VTX(vertex_regid) |
			A4XX_VFD_CONTROL_1_REGID4INST(instance_regid));
	OUT_RING(ring, 0x00000000);   /* XXX VFD_CONTROL_2 */
	OUT_RING(ring, A4XX_VFD_CONTROL_3_REGID_VTXCNT(regid(63, 0)));
	OUT_RING(ring, 0x00000000);   /* XXX VFD_CONTROL_4 */

	/* cache invalidate, otherwise vertex fetch could see
	 * stale vbo contents:
	 */
	OUT_PKT0(ring, REG_A4XX_UCHE_INVALIDATE0, 2);
	OUT_RING(ring, 0x00000000);
	OUT_RING(ring, 0x00000012);
}

void
fd4_emit_state(struct fd_context *ctx, struct fd_ringbuffer *ring,
		struct fd4_emit *emit)
{
	struct ir3_shader_variant *vp = fd4_emit_get_vp(emit);
	struct ir3_shader_variant *fp = fd4_emit_get_fp(emit);
	uint32_t dirty = emit->dirty;

	emit_marker(ring, 5);

	if ((dirty & (FD_DIRTY_ZSA | FD_DIRTY_PROG)) && !emit->key.binning_pass) {
		uint32_t val = fd4_zsa_stateobj(ctx->zsa)->rb_render_control;

		/* I suppose if we needed to (which I don't *think* we need
		 * to), we could emit this for binning pass too.  But we
		 * would need to keep a different patch-list for binning
		 * vs render pass.
		 */

		OUT_PKT0(ring, REG_A4XX_RB_RENDER_CONTROL, 1);
		OUT_RINGP(ring, val, &fd4_context(ctx)->rbrc_patches);
	}

	if (dirty & FD_DIRTY_ZSA) {
		struct fd4_zsa_stateobj *zsa = fd4_zsa_stateobj(ctx->zsa);

		OUT_PKT0(ring, REG_A4XX_RB_ALPHA_CONTROL, 1);
		OUT_RING(ring, zsa->rb_alpha_control);

		OUT_PKT0(ring, REG_A4XX_RB_STENCIL_CONTROL, 2);
		OUT_RING(ring, zsa->rb_stencil_control);
		OUT_RING(ring, zsa->rb_stencil_control2);
	}

	if (dirty & (FD_DIRTY_ZSA | FD_DIRTY_STENCIL_REF)) {
		struct fd4_zsa_stateobj *zsa = fd4_zsa_stateobj(ctx->zsa);
		struct pipe_stencil_ref *sr = &ctx->stencil_ref;

		OUT_PKT0(ring, REG_A4XX_RB_STENCILREFMASK, 2);
		OUT_RING(ring, zsa->rb_stencilrefmask |
				A4XX_RB_STENCILREFMASK_STENCILREF(sr->ref_value[0]));
		OUT_RING(ring, zsa->rb_stencilrefmask_bf |
				A4XX_RB_STENCILREFMASK_BF_STENCILREF(sr->ref_value[1]));
	}

	if (dirty & (FD_DIRTY_ZSA | FD_DIRTY_PROG)) {
		struct fd4_zsa_stateobj *zsa = fd4_zsa_stateobj(ctx->zsa);
		bool fragz = fp->has_kill | fp->writes_pos;

		OUT_PKT0(ring, REG_A4XX_RB_DEPTH_CONTROL, 1);
		OUT_RING(ring, zsa->rb_depth_control |
				COND(fragz, A4XX_RB_DEPTH_CONTROL_EARLY_Z_DISABLE));

		/* maybe this register/bitfield needs a better name.. this
		 * appears to be just disabling early-z
		 */
		OUT_PKT0(ring, REG_A4XX_GRAS_ALPHA_CONTROL, 1);
		OUT_RING(ring, zsa->gras_alpha_control |
				COND(fragz, A4XX_GRAS_ALPHA_CONTROL_ALPHA_TEST_ENABLE));
	}

	if (dirty & FD_DIRTY_RASTERIZER) {
		struct fd4_rasterizer_stateobj *rasterizer =
				fd4_rasterizer_stateobj(ctx->rasterizer);

		OUT_PKT0(ring, REG_A4XX_GRAS_SU_MODE_CONTROL, 1);
		OUT_RING(ring, rasterizer->gras_su_mode_control |
				A4XX_GRAS_SU_MODE_CONTROL_RENDERING_PASS);

		OUT_PKT0(ring, REG_A4XX_GRAS_SU_POINT_MINMAX, 2);
		OUT_RING(ring, rasterizer->gras_su_point_minmax);
		OUT_RING(ring, rasterizer->gras_su_point_size);

		OUT_PKT0(ring, REG_A4XX_GRAS_SU_POLY_OFFSET_SCALE, 2);
		OUT_RING(ring, rasterizer->gras_su_poly_offset_scale);
		OUT_RING(ring, rasterizer->gras_su_poly_offset_offset);

		OUT_PKT0(ring, REG_A4XX_GRAS_CL_CLIP_CNTL, 1);
		OUT_RING(ring, rasterizer->gras_cl_clip_cntl);
	}

	/* NOTE: since primitive_restart is not actually part of any
	 * state object, we need to make sure that we always emit
	 * PRIM_VTX_CNTL.. either that or be more clever and detect
	 * when it changes.
	 */
	if (emit->info) {
		const struct pipe_draw_info *info = emit->info;
		uint32_t val = fd4_rasterizer_stateobj(ctx->rasterizer)
				->pc_prim_vtx_cntl;

		if (info->indexed && info->primitive_restart)
			val |= A4XX_PC_PRIM_VTX_CNTL_PRIMITIVE_RESTART;

		val |= COND(vp->writes_psize, A4XX_PC_PRIM_VTX_CNTL_PSIZE);

		if (fp->total_in > 0) {
			uint32_t varout = align(fp->total_in, 16) / 16;
			if (varout > 1)
				varout = align(varout, 2);
			val |= A4XX_PC_PRIM_VTX_CNTL_VAROUT(varout);
		}

		OUT_PKT0(ring, REG_A4XX_PC_PRIM_VTX_CNTL, 2);
		OUT_RING(ring, val);
		OUT_RING(ring, 0x12);     /* XXX UNKNOWN_21C5 */
	}

	if (dirty & FD_DIRTY_SCISSOR) {
		struct pipe_scissor_state *scissor = fd_context_get_scissor(ctx);

		OUT_PKT0(ring, REG_A4XX_GRAS_SC_WINDOW_SCISSOR_BR, 2);
		OUT_RING(ring, A4XX_GRAS_SC_WINDOW_SCISSOR_BR_X(scissor->maxx - 1) |
				A4XX_GRAS_SC_WINDOW_SCISSOR_BR_Y(scissor->maxy - 1));
		OUT_RING(ring, A4XX_GRAS_SC_WINDOW_SCISSOR_TL_X(scissor->minx) |
				A4XX_GRAS_SC_WINDOW_SCISSOR_TL_Y(scissor->miny));

		ctx->max_scissor.minx = MIN2(ctx->max_scissor.minx, scissor->minx);
		ctx->max_scissor.miny = MIN2(ctx->max_scissor.miny, scissor->miny);
		ctx->max_scissor.maxx = MAX2(ctx->max_scissor.maxx, scissor->maxx);
		ctx->max_scissor.maxy = MAX2(ctx->max_scissor.maxy, scissor->maxy);
	}

	if (dirty & FD_DIRTY_VIEWPORT) {
		fd_wfi(ctx, ring);
		OUT_PKT0(ring, REG_A4XX_GRAS_CL_VPORT_XOFFSET_0, 6);
		OUT_RING(ring, A4XX_GRAS_CL_VPORT_XOFFSET_0(ctx->viewport.translate[0]));
		OUT_RING(ring, A4XX_GRAS_CL_VPORT_XSCALE_0(ctx->viewport.scale[0]));
		OUT_RING(ring, A4XX_GRAS_CL_VPORT_YOFFSET_0(ctx->viewport.translate[1]));
		OUT_RING(ring, A4XX_GRAS_CL_VPORT_YSCALE_0(ctx->viewport.scale[1]));
		OUT_RING(ring, A4XX_GRAS_CL_VPORT_ZOFFSET_0(ctx->viewport.translate[2]));
		OUT_RING(ring, A4XX_GRAS_CL_VPORT_ZSCALE_0(ctx->viewport.scale[2]));
	}

	if (dirty & FD_DIRTY_PROG)
		fd4_program_emit(ring, emit);

	if ((dirty & (FD_DIRTY_PROG | FD_DIRTY_CONSTBUF)) &&
			/* evil hack to deal sanely with clear path: */
			(emit->prog == &ctx->prog)) {
		fd_wfi(ctx, ring);
		emit_constants(ring,  SB_VERT_SHADER,
				&ctx->constbuf[PIPE_SHADER_VERTEX],
				vp, emit->prog->dirty & FD_SHADER_DIRTY_VP);
		if (!emit->key.binning_pass) {
			emit_constants(ring, SB_FRAG_SHADER,
					&ctx->constbuf[PIPE_SHADER_FRAGMENT],
					fp, emit->prog->dirty & FD_SHADER_DIRTY_FP);
		}
	}

	/* emit driver params every time */
	if (emit->info && emit->prog == &ctx->prog) {
		uint32_t vertex_params[4] = {
			emit->info->indexed ? emit->info->index_bias : emit->info->start,
			0,
			0,
			0
		};
		if (vp->constlen >= vp->first_driver_param + 4) {
			fd4_emit_constant(ring, SB_VERT_SHADER,
							  (vp->first_driver_param + 4) * 4,
							  0, 4, vertex_params, NULL);
		}
	}

	if ((dirty & FD_DIRTY_BLEND) && ctx->blend) {
		struct fd4_blend_stateobj *blend = fd4_blend_stateobj(ctx->blend);
		uint32_t i;

		for (i = 0; i < 8; i++) {
			OUT_PKT0(ring, REG_A4XX_RB_MRT_CONTROL(i), 1);
			OUT_RING(ring, blend->rb_mrt[i].control);

			OUT_PKT0(ring, REG_A4XX_RB_MRT_BLEND_CONTROL(i), 1);
			OUT_RING(ring, blend->rb_mrt[i].blend_control);
		}

		OUT_PKT0(ring, REG_A4XX_RB_FS_OUTPUT, 1);
		OUT_RING(ring, blend->rb_fs_output |
				A4XX_RB_FS_OUTPUT_SAMPLE_MASK(0xffff));
	}

	if (dirty & FD_DIRTY_BLEND_COLOR) {
		struct pipe_blend_color *bcolor = &ctx->blend_color;
		OUT_PKT0(ring, REG_A4XX_RB_BLEND_RED, 4);
		OUT_RING(ring, A4XX_RB_BLEND_RED_UINT(bcolor->color[0] * 255.0) |
				A4XX_RB_BLEND_RED_FLOAT(bcolor->color[0]));
		OUT_RING(ring, A4XX_RB_BLEND_GREEN_UINT(bcolor->color[1] * 255.0) |
				A4XX_RB_BLEND_GREEN_FLOAT(bcolor->color[1]));
		OUT_RING(ring, A4XX_RB_BLEND_BLUE_UINT(bcolor->color[2] * 255.0) |
				A4XX_RB_BLEND_BLUE_FLOAT(bcolor->color[2]));
		OUT_RING(ring, A4XX_RB_BLEND_ALPHA_UINT(bcolor->color[3] * 255.0) |
				A4XX_RB_BLEND_ALPHA_FLOAT(bcolor->color[3]));
	}

	if (dirty & FD_DIRTY_VERTTEX) {
		if (vp->has_samp)
			emit_textures(ctx, ring, SB_VERT_TEX, &ctx->verttex);
		else
			dirty &= ~FD_DIRTY_VERTTEX;
	}

	if (dirty & FD_DIRTY_FRAGTEX) {
		if (fp->has_samp)
			emit_textures(ctx, ring, SB_FRAG_TEX, &ctx->fragtex);
		else
			dirty &= ~FD_DIRTY_FRAGTEX;
	}

	ctx->dirty &= ~dirty;
}

/* emit setup at begin of new cmdstream buffer (don't rely on previous
 * state, there could have been a context switch between ioctls):
 */
void
fd4_emit_restore(struct fd_context *ctx)
{
	struct fd4_context *fd4_ctx = fd4_context(ctx);
	struct fd_ringbuffer *ring = ctx->ring;

	OUT_PKT0(ring, REG_A4XX_RBBM_PERFCTR_CTL, 1);
	OUT_RING(ring, 0x00000001);

	OUT_PKT0(ring, REG_A4XX_GRAS_DEBUG_ECO_CONTROL, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_0EC3, 1);
	OUT_RING(ring, 0x00000006);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_0F03, 1);
	OUT_RING(ring, 0x0000003a);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_0D01, 1);
	OUT_RING(ring, 0x00000001);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_0E42, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UCHE_CACHE_WAYS_VFD, 1);
	OUT_RING(ring, 0x00000007);

	OUT_PKT0(ring, REG_A4XX_UCHE_CACHE_MODE_CONTROL, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UCHE_INVALIDATE0, 2);
	OUT_RING(ring, 0x00000000);
	OUT_RING(ring, 0x00000012);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_0E05, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_0CC5, 1);
	OUT_RING(ring, 0x00000006);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_0CC6, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_0EC2, 1);
	OUT_RING(ring, 0x00040000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_2001, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT3(ring, CP_INVALIDATE_STATE, 1);
	OUT_RING(ring, 0x00001000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_20EF, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_20F0, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_20F1, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_20F2, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_RB_BLEND_RED, 4);
	OUT_RING(ring, A4XX_RB_BLEND_RED_UINT(0) |
			A4XX_RB_BLEND_RED_FLOAT(0.0));
	OUT_RING(ring, A4XX_RB_BLEND_GREEN_UINT(0) |
			A4XX_RB_BLEND_GREEN_FLOAT(0.0));
	OUT_RING(ring, A4XX_RB_BLEND_BLUE_UINT(0) |
			A4XX_RB_BLEND_BLUE_FLOAT(0.0));
	OUT_RING(ring, A4XX_RB_BLEND_ALPHA_UINT(0x7fff) |
			A4XX_RB_BLEND_ALPHA_FLOAT(1.0));

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_20F7, 1);
	OUT_RING(ring, 0x3f800000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_2152, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_2153, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_2154, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_2155, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_2156, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_2157, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_21C3, 1);
	OUT_RING(ring, 0x0000001d);

	OUT_PKT0(ring, REG_A4XX_PC_GS_PARAM, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_21E6, 1);
	OUT_RING(ring, 0x00000001);

	OUT_PKT0(ring, REG_A4XX_PC_HS_PARAM, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_UNKNOWN_22D7, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_TPL1_TP_TEX_OFFSET, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_TPL1_TP_TEX_COUNT, 1);
	OUT_RING(ring, A4XX_TPL1_TP_TEX_COUNT_VS(16) |
			A4XX_TPL1_TP_TEX_COUNT_HS(0) |
			A4XX_TPL1_TP_TEX_COUNT_DS(0) |
			A4XX_TPL1_TP_TEX_COUNT_GS(0));

	OUT_PKT0(ring, REG_A4XX_TPL1_TP_FS_TEX_COUNT, 1);
	OUT_RING(ring, 16);

	/* we don't use this yet.. probably best to disable.. */
	OUT_PKT3(ring, CP_SET_DRAW_STATE, 2);
	OUT_RING(ring, CP_SET_DRAW_STATE_0_COUNT(0) |
			CP_SET_DRAW_STATE_0_DISABLE_ALL_GROUPS |
			CP_SET_DRAW_STATE_0_GROUP_ID(0));
	OUT_RING(ring, CP_SET_DRAW_STATE_1_ADDR(0));

	OUT_PKT0(ring, REG_A4XX_SP_VS_PVT_MEM_PARAM, 2);
	OUT_RING(ring, 0x08000001);                  /* SP_VS_PVT_MEM_PARAM */
	OUT_RELOC(ring, fd4_ctx->vs_pvt_mem, 0,0,0); /* SP_VS_PVT_MEM_ADDR */

	OUT_PKT0(ring, REG_A4XX_SP_FS_PVT_MEM_PARAM, 2);
	OUT_RING(ring, 0x08000001);                  /* SP_FS_PVT_MEM_PARAM */
	OUT_RELOC(ring, fd4_ctx->fs_pvt_mem, 0,0,0); /* SP_FS_PVT_MEM_ADDR */

	OUT_PKT0(ring, REG_A4XX_GRAS_SC_CONTROL, 1);
	OUT_RING(ring, A4XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
			A4XX_GRAS_SC_CONTROL_MSAA_DISABLE |
			A4XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
			A4XX_GRAS_SC_CONTROL_RASTER_MODE(0));

	OUT_PKT0(ring, REG_A4XX_RB_MSAA_CONTROL, 1);
	OUT_RING(ring, A4XX_RB_MSAA_CONTROL_DISABLE |
			A4XX_RB_MSAA_CONTROL_SAMPLES(MSAA_ONE));

	OUT_PKT0(ring, REG_A4XX_GRAS_CL_GB_CLIP_ADJ, 1);
	OUT_RING(ring, A4XX_GRAS_CL_GB_CLIP_ADJ_HORZ(0) |
			A4XX_GRAS_CL_GB_CLIP_ADJ_VERT(0));

	OUT_PKT0(ring, REG_A4XX_RB_ALPHA_CONTROL, 1);
	OUT_RING(ring, A4XX_RB_ALPHA_CONTROL_ALPHA_TEST_FUNC(FUNC_ALWAYS));

	OUT_PKT0(ring, REG_A4XX_RB_FS_OUTPUT, 1);
	OUT_RING(ring, A4XX_RB_FS_OUTPUT_SAMPLE_MASK(0xffff));

	OUT_PKT0(ring, REG_A4XX_RB_RENDER_COMPONENTS, 1);
	OUT_RING(ring, A4XX_RB_RENDER_COMPONENTS_RT0(0xf));

	OUT_PKT0(ring, REG_A4XX_GRAS_CLEAR_CNTL, 1);
	OUT_RING(ring, A4XX_GRAS_CLEAR_CNTL_NOT_FASTCLEAR);

	OUT_PKT0(ring, REG_A4XX_GRAS_ALPHA_CONTROL, 1);
	OUT_RING(ring, 0x0);

	ctx->needs_rb_fbd = true;
}
