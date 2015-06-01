/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
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
#include "util/u_prim.h"
#include "util/u_format.h"

#include "freedreno_state.h"
#include "freedreno_resource.h"

#include "fd3_draw.h"
#include "fd3_context.h"
#include "fd3_emit.h"
#include "fd3_program.h"
#include "fd3_format.h"
#include "fd3_zsa.h"

static inline uint32_t
add_sat(uint32_t a, int32_t b)
{
	int64_t ret = (uint64_t)a + (int64_t)b;
	if (ret > ~0U)
		return ~0U;
	if (ret < 0)
		return 0;
	return (uint32_t)ret;
}

static void
draw_impl(struct fd_context *ctx, struct fd_ringbuffer *ring,
		struct fd3_emit *emit)
{
	const struct pipe_draw_info *info = emit->info;
	enum pc_di_primtype primtype = ctx->primtypes[info->mode];

	fd3_emit_state(ctx, ring, emit);

	if (emit->dirty & (FD_DIRTY_VTXBUF | FD_DIRTY_VTXSTATE))
		fd3_emit_vertex_bufs(ring, emit);

	OUT_PKT0(ring, REG_A3XX_PC_VERTEX_REUSE_BLOCK_CNTL, 1);
	OUT_RING(ring, 0x0000000b);             /* PC_VERTEX_REUSE_BLOCK_CNTL */

	OUT_PKT0(ring, REG_A3XX_VFD_INDEX_MIN, 4);
	OUT_RING(ring, add_sat(info->min_index, info->index_bias)); /* VFD_INDEX_MIN */
	OUT_RING(ring, add_sat(info->max_index, info->index_bias)); /* VFD_INDEX_MAX */
	OUT_RING(ring, info->start_instance);   /* VFD_INSTANCEID_OFFSET */
	OUT_RING(ring, info->indexed ? info->index_bias : info->start); /* VFD_INDEX_OFFSET */

	OUT_PKT0(ring, REG_A3XX_PC_RESTART_INDEX, 1);
	OUT_RING(ring, info->primitive_restart ? /* PC_RESTART_INDEX */
			info->restart_index : 0xffffffff);

	if (ctx->rasterizer && ctx->rasterizer->point_size_per_vertex &&
		info->mode == PIPE_PRIM_POINTS)
		primtype = DI_PT_POINTLIST_A2XX;

	fd_draw_emit(ctx, ring,
			primtype,
			emit->key.binning_pass ? IGNORE_VISIBILITY : USE_VISIBILITY,
			info);
}

/* fixup dirty shader state in case some "unrelated" (from the state-
 * tracker's perspective) state change causes us to switch to a
 * different variant.
 */
static void
fixup_shader_state(struct fd_context *ctx, struct ir3_shader_key *key)
{
	struct fd3_context *fd3_ctx = fd3_context(ctx);
	struct ir3_shader_key *last_key = &fd3_ctx->last_key;

	if (!ir3_shader_key_equal(last_key, key)) {
		ctx->dirty |= FD_DIRTY_PROG;

		if (last_key->has_per_samp || key->has_per_samp) {
			if ((last_key->vsaturate_s != key->vsaturate_s) ||
					(last_key->vsaturate_t != key->vsaturate_t) ||
					(last_key->vsaturate_r != key->vsaturate_r) ||
					(last_key->vinteger_s != key->vinteger_s))
				ctx->prog.dirty |= FD_SHADER_DIRTY_VP;

			if ((last_key->fsaturate_s != key->fsaturate_s) ||
					(last_key->fsaturate_t != key->fsaturate_t) ||
					(last_key->fsaturate_r != key->fsaturate_r) ||
					(last_key->finteger_s != key->finteger_s))
				ctx->prog.dirty |= FD_SHADER_DIRTY_FP;
		}

		if (last_key->color_two_side != key->color_two_side)
			ctx->prog.dirty |= FD_SHADER_DIRTY_FP;

		if (last_key->half_precision != key->half_precision)
			ctx->prog.dirty |= FD_SHADER_DIRTY_FP;

		fd3_ctx->last_key = *key;
	}
}

static void
fd3_draw_vbo(struct fd_context *ctx, const struct pipe_draw_info *info)
{
	struct fd3_context *fd3_ctx = fd3_context(ctx);
	struct fd3_emit emit = {
		.vtx  = &ctx->vtx,
		.prog = &ctx->prog,
		.info = info,
		.key = {
			/* do binning pass first: */
			.binning_pass = true,
			.color_two_side = ctx->rasterizer ? ctx->rasterizer->light_twoside : false,
			// TODO set .half_precision based on render target format,
			// ie. float16 and smaller use half, float32 use full..
			.half_precision = !!(fd_mesa_debug & FD_DBG_FRAGHALF),
			.has_per_samp = (fd3_ctx->fsaturate || fd3_ctx->vsaturate ||
							 fd3_ctx->vinteger_s || fd3_ctx->finteger_s),
			.vsaturate_s = fd3_ctx->vsaturate_s,
			.vsaturate_t = fd3_ctx->vsaturate_t,
			.vsaturate_r = fd3_ctx->vsaturate_r,
			.fsaturate_s = fd3_ctx->fsaturate_s,
			.fsaturate_t = fd3_ctx->fsaturate_t,
			.fsaturate_r = fd3_ctx->fsaturate_r,
			.vinteger_s = fd3_ctx->vinteger_s,
			.finteger_s = fd3_ctx->finteger_s,
		},
		.rasterflat = ctx->rasterizer && ctx->rasterizer->flatshade,
		.sprite_coord_enable = ctx->rasterizer ? ctx->rasterizer->sprite_coord_enable : 0,
		.sprite_coord_mode = ctx->rasterizer ? ctx->rasterizer->sprite_coord_mode : false,
	};
	unsigned dirty;

	fixup_shader_state(ctx, &emit.key);

	dirty = ctx->dirty;
	emit.dirty = dirty & ~(FD_DIRTY_BLEND);
	draw_impl(ctx, ctx->binning_ring, &emit);

	/* and now regular (non-binning) pass: */
	emit.key.binning_pass = false;
	emit.dirty = dirty;
	emit.vp = NULL;   /* we changed key so need to refetch vp */
	draw_impl(ctx, ctx->ring, &emit);
}

/* clear operations ignore viewport state, so we need to reset it
 * based on framebuffer state:
 */
static void
reset_viewport(struct fd_ringbuffer *ring, struct pipe_framebuffer_state *pfb)
{
	float half_width = pfb->width * 0.5f;
	float half_height = pfb->height * 0.5f;

	OUT_PKT0(ring, REG_A3XX_GRAS_CL_VPORT_XOFFSET, 4);
	OUT_RING(ring, A3XX_GRAS_CL_VPORT_XOFFSET(half_width - 0.5));
	OUT_RING(ring, A3XX_GRAS_CL_VPORT_XSCALE(half_width));
	OUT_RING(ring, A3XX_GRAS_CL_VPORT_YOFFSET(half_height - 0.5));
	OUT_RING(ring, A3XX_GRAS_CL_VPORT_YSCALE(-half_height));
}

/* binning pass cmds for a clear:
 * NOTE: newer blob drivers don't use binning for clear, which is probably
 * preferable since it is low vtx count.  However that doesn't seem to
 * actually work for me.  Not sure if it is depending on support for
 * clear pass (rather than using solid-fill shader), or something else
 * that newer blob is doing differently.  Once that is figured out, we
 * can remove fd3_clear_binning().
 */
static void
fd3_clear_binning(struct fd_context *ctx, unsigned dirty)
{
	struct fd3_context *fd3_ctx = fd3_context(ctx);
	struct fd_ringbuffer *ring = ctx->binning_ring;
	struct fd3_emit emit = {
		.vtx  = &fd3_ctx->solid_vbuf_state,
		.prog = &ctx->solid_prog,
		.key = {
			.binning_pass = true,
			.half_precision = true,
		},
		.dirty = dirty,
	};

	fd3_emit_state(ctx, ring, &emit);
	fd3_emit_vertex_bufs(ring, &emit);
	reset_viewport(ring, &ctx->framebuffer);

	OUT_PKT0(ring, REG_A3XX_PC_PRIM_VTX_CNTL, 1);
	OUT_RING(ring, A3XX_PC_PRIM_VTX_CNTL_STRIDE_IN_VPC(0) |
			A3XX_PC_PRIM_VTX_CNTL_POLYMODE_FRONT_PTYPE(PC_DRAW_TRIANGLES) |
			A3XX_PC_PRIM_VTX_CNTL_POLYMODE_BACK_PTYPE(PC_DRAW_TRIANGLES) |
			A3XX_PC_PRIM_VTX_CNTL_PROVOKING_VTX_LAST);
	OUT_PKT0(ring, REG_A3XX_VFD_INDEX_MIN, 4);
	OUT_RING(ring, 0);            /* VFD_INDEX_MIN */
	OUT_RING(ring, 2);            /* VFD_INDEX_MAX */
	OUT_RING(ring, 0);            /* VFD_INSTANCEID_OFFSET */
	OUT_RING(ring, 0);            /* VFD_INDEX_OFFSET */
	OUT_PKT0(ring, REG_A3XX_PC_RESTART_INDEX, 1);
	OUT_RING(ring, 0xffffffff);   /* PC_RESTART_INDEX */

	fd_event_write(ctx, ring, PERFCOUNTER_STOP);

	fd_draw(ctx, ring, DI_PT_RECTLIST, IGNORE_VISIBILITY,
			DI_SRC_SEL_AUTO_INDEX, 2, 0, INDEX_SIZE_IGN, 0, 0, NULL);
}

static void
fd3_clear(struct fd_context *ctx, unsigned buffers,
		const union pipe_color_union *color, double depth, unsigned stencil)
{
	struct fd3_context *fd3_ctx = fd3_context(ctx);
	struct pipe_framebuffer_state *pfb = &ctx->framebuffer;
	struct fd_ringbuffer *ring = ctx->ring;
	unsigned dirty = ctx->dirty;
	unsigned i;
	struct fd3_emit emit = {
		.vtx  = &fd3_ctx->solid_vbuf_state,
		.prog = &ctx->solid_prog,
		.key = {
			.half_precision = (fd3_half_precision(pfb->cbufs[0]) &&
							   fd3_half_precision(pfb->cbufs[1]) &&
							   fd3_half_precision(pfb->cbufs[2]) &&
							   fd3_half_precision(pfb->cbufs[3])),
		},
	};

	dirty &= FD_DIRTY_FRAMEBUFFER | FD_DIRTY_SCISSOR;
	dirty |= FD_DIRTY_PROG;
	emit.dirty = dirty;

	fd3_clear_binning(ctx, dirty);

	/* emit generic state now: */
	fd3_emit_state(ctx, ring, &emit);
	reset_viewport(ring, &ctx->framebuffer);

	OUT_PKT0(ring, REG_A3XX_RB_BLEND_ALPHA, 1);
	OUT_RING(ring, A3XX_RB_BLEND_ALPHA_UINT(0xff) |
			A3XX_RB_BLEND_ALPHA_FLOAT(1.0));

	OUT_PKT0(ring, REG_A3XX_RB_RENDER_CONTROL, 1);
	OUT_RINGP(ring, A3XX_RB_RENDER_CONTROL_ALPHA_TEST_FUNC(FUNC_NEVER),
			&fd3_ctx->rbrc_patches);

	if (buffers & PIPE_CLEAR_DEPTH) {
		OUT_PKT0(ring, REG_A3XX_RB_DEPTH_CONTROL, 1);
		OUT_RING(ring, A3XX_RB_DEPTH_CONTROL_Z_WRITE_ENABLE |
				A3XX_RB_DEPTH_CONTROL_Z_ENABLE |
				A3XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_ALWAYS));

		fd_wfi(ctx, ring);
		OUT_PKT0(ring, REG_A3XX_GRAS_CL_VPORT_ZOFFSET, 2);
		OUT_RING(ring, A3XX_GRAS_CL_VPORT_ZOFFSET(0.0));
		OUT_RING(ring, A3XX_GRAS_CL_VPORT_ZSCALE(depth));
		ctx->dirty |= FD_DIRTY_VIEWPORT;
	} else {
		OUT_PKT0(ring, REG_A3XX_RB_DEPTH_CONTROL, 1);
		OUT_RING(ring, A3XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_NEVER));
	}

	if (buffers & PIPE_CLEAR_STENCIL) {
		OUT_PKT0(ring, REG_A3XX_RB_STENCILREFMASK, 2);
		OUT_RING(ring, A3XX_RB_STENCILREFMASK_STENCILREF(stencil) |
				A3XX_RB_STENCILREFMASK_STENCILMASK(stencil) |
				A3XX_RB_STENCILREFMASK_STENCILWRITEMASK(0xff));
		OUT_RING(ring, A3XX_RB_STENCILREFMASK_STENCILREF(0) |
				A3XX_RB_STENCILREFMASK_STENCILMASK(0) |
				0xff000000 | // XXX ???
				A3XX_RB_STENCILREFMASK_STENCILWRITEMASK(0xff));

		OUT_PKT0(ring, REG_A3XX_RB_STENCIL_CONTROL, 1);
		OUT_RING(ring, A3XX_RB_STENCIL_CONTROL_STENCIL_ENABLE |
				A3XX_RB_STENCIL_CONTROL_FUNC(FUNC_ALWAYS) |
				A3XX_RB_STENCIL_CONTROL_FAIL(STENCIL_KEEP) |
				A3XX_RB_STENCIL_CONTROL_ZPASS(STENCIL_REPLACE) |
				A3XX_RB_STENCIL_CONTROL_ZFAIL(STENCIL_KEEP) |
				A3XX_RB_STENCIL_CONTROL_FUNC_BF(FUNC_NEVER) |
				A3XX_RB_STENCIL_CONTROL_FAIL_BF(STENCIL_KEEP) |
				A3XX_RB_STENCIL_CONTROL_ZPASS_BF(STENCIL_KEEP) |
				A3XX_RB_STENCIL_CONTROL_ZFAIL_BF(STENCIL_KEEP));
	} else {
		OUT_PKT0(ring, REG_A3XX_RB_STENCILREFMASK, 2);
		OUT_RING(ring, A3XX_RB_STENCILREFMASK_STENCILREF(0) |
				A3XX_RB_STENCILREFMASK_STENCILMASK(0) |
				A3XX_RB_STENCILREFMASK_STENCILWRITEMASK(0));
		OUT_RING(ring, A3XX_RB_STENCILREFMASK_BF_STENCILREF(0) |
				A3XX_RB_STENCILREFMASK_BF_STENCILMASK(0) |
				A3XX_RB_STENCILREFMASK_BF_STENCILWRITEMASK(0));

		OUT_PKT0(ring, REG_A3XX_RB_STENCIL_CONTROL, 1);
		OUT_RING(ring, A3XX_RB_STENCIL_CONTROL_FUNC(FUNC_NEVER) |
				A3XX_RB_STENCIL_CONTROL_FAIL(STENCIL_KEEP) |
				A3XX_RB_STENCIL_CONTROL_ZPASS(STENCIL_KEEP) |
				A3XX_RB_STENCIL_CONTROL_ZFAIL(STENCIL_KEEP) |
				A3XX_RB_STENCIL_CONTROL_FUNC_BF(FUNC_NEVER) |
				A3XX_RB_STENCIL_CONTROL_FAIL_BF(STENCIL_KEEP) |
				A3XX_RB_STENCIL_CONTROL_ZPASS_BF(STENCIL_KEEP) |
				A3XX_RB_STENCIL_CONTROL_ZFAIL_BF(STENCIL_KEEP));
	}

	for (i = 0; i < 4; i++) {
		OUT_PKT0(ring, REG_A3XX_RB_MRT_CONTROL(i), 1);
		OUT_RING(ring, A3XX_RB_MRT_CONTROL_ROP_CODE(ROP_COPY) |
				A3XX_RB_MRT_CONTROL_DITHER_MODE(DITHER_ALWAYS) |
				COND(buffers & (PIPE_CLEAR_COLOR0 << i),
					 A3XX_RB_MRT_CONTROL_COMPONENT_ENABLE(0xf)));

		OUT_PKT0(ring, REG_A3XX_RB_MRT_BLEND_CONTROL(i), 1);
		OUT_RING(ring, A3XX_RB_MRT_BLEND_CONTROL_RGB_SRC_FACTOR(FACTOR_ONE) |
				A3XX_RB_MRT_BLEND_CONTROL_RGB_BLEND_OPCODE(BLEND_DST_PLUS_SRC) |
				A3XX_RB_MRT_BLEND_CONTROL_RGB_DEST_FACTOR(FACTOR_ZERO) |
				A3XX_RB_MRT_BLEND_CONTROL_ALPHA_SRC_FACTOR(FACTOR_ONE) |
				A3XX_RB_MRT_BLEND_CONTROL_ALPHA_BLEND_OPCODE(BLEND_DST_PLUS_SRC) |
				A3XX_RB_MRT_BLEND_CONTROL_ALPHA_DEST_FACTOR(FACTOR_ZERO));
	}

	OUT_PKT0(ring, REG_A3XX_GRAS_SU_MODE_CONTROL, 1);
	OUT_RING(ring, A3XX_GRAS_SU_MODE_CONTROL_LINEHALFWIDTH(0));

	fd3_emit_vertex_bufs(ring, &emit);

	fd3_emit_constant(ring, SB_FRAG_SHADER, 0, 0, 4, color->ui, NULL);

	OUT_PKT0(ring, REG_A3XX_PC_PRIM_VTX_CNTL, 1);
	OUT_RING(ring, A3XX_PC_PRIM_VTX_CNTL_STRIDE_IN_VPC(0) |
			A3XX_PC_PRIM_VTX_CNTL_POLYMODE_FRONT_PTYPE(PC_DRAW_TRIANGLES) |
			A3XX_PC_PRIM_VTX_CNTL_POLYMODE_BACK_PTYPE(PC_DRAW_TRIANGLES) |
			A3XX_PC_PRIM_VTX_CNTL_PROVOKING_VTX_LAST);
	OUT_PKT0(ring, REG_A3XX_VFD_INDEX_MIN, 4);
	OUT_RING(ring, 0);            /* VFD_INDEX_MIN */
	OUT_RING(ring, 2);            /* VFD_INDEX_MAX */
	OUT_RING(ring, 0);            /* VFD_INSTANCEID_OFFSET */
	OUT_RING(ring, 0);            /* VFD_INDEX_OFFSET */
	OUT_PKT0(ring, REG_A3XX_PC_RESTART_INDEX, 1);
	OUT_RING(ring, 0xffffffff);   /* PC_RESTART_INDEX */

	fd_event_write(ctx, ring, PERFCOUNTER_STOP);

	fd_draw(ctx, ring, DI_PT_RECTLIST, USE_VISIBILITY,
			DI_SRC_SEL_AUTO_INDEX, 2, 0, INDEX_SIZE_IGN, 0, 0, NULL);
}

void
fd3_draw_init(struct pipe_context *pctx)
{
	struct fd_context *ctx = fd_context(pctx);
	ctx->draw_vbo = fd3_draw_vbo;
	ctx->clear = fd3_clear;
}
