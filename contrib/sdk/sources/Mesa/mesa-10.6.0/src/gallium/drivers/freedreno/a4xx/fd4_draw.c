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
#include "util/u_prim.h"

#include "freedreno_state.h"
#include "freedreno_resource.h"

#include "fd4_draw.h"
#include "fd4_context.h"
#include "fd4_emit.h"
#include "fd4_program.h"
#include "fd4_format.h"
#include "fd4_zsa.h"


static void
draw_impl(struct fd_context *ctx, struct fd_ringbuffer *ring,
		struct fd4_emit *emit)
{
	const struct pipe_draw_info *info = emit->info;

	fd4_emit_state(ctx, ring, emit);

	if (emit->dirty & (FD_DIRTY_VTXBUF | FD_DIRTY_VTXSTATE))
		fd4_emit_vertex_bufs(ring, emit);

	OUT_PKT0(ring, REG_A4XX_VFD_INDEX_OFFSET, 2);
	OUT_RING(ring, info->indexed ? info->index_bias : info->start); /* VFD_INDEX_OFFSET */
	OUT_RING(ring, info->start_instance);   /* ??? UNKNOWN_2209 */

	OUT_PKT0(ring, REG_A4XX_PC_RESTART_INDEX, 1);
	OUT_RING(ring, info->primitive_restart ? /* PC_RESTART_INDEX */
			info->restart_index : 0xffffffff);

	fd4_draw_emit(ctx, ring,
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
	struct fd4_context *fd4_ctx = fd4_context(ctx);
	struct ir3_shader_key *last_key = &fd4_ctx->last_key;

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
					(last_key->fsaturate_r != key->fsaturate_r))
				ctx->prog.dirty |= FD_SHADER_DIRTY_FP;
		}

		if (last_key->color_two_side != key->color_two_side)
			ctx->prog.dirty |= FD_SHADER_DIRTY_FP;

		if (last_key->half_precision != key->half_precision)
			ctx->prog.dirty |= FD_SHADER_DIRTY_FP;

		if (last_key->rasterflat != key->rasterflat)
			ctx->prog.dirty |= FD_SHADER_DIRTY_FP;

		fd4_ctx->last_key = *key;
	}
}

static void
fd4_draw_vbo(struct fd_context *ctx, const struct pipe_draw_info *info)
{
	struct fd4_context *fd4_ctx = fd4_context(ctx);
	struct pipe_framebuffer_state *pfb = &ctx->framebuffer;
	struct fd4_emit emit = {
		.vtx  = &ctx->vtx,
		.prog = &ctx->prog,
		.info = info,
		.key = {
			/* do binning pass first: */
			.binning_pass = true,
			.color_two_side = ctx->rasterizer ? ctx->rasterizer->light_twoside : false,
			.rasterflat = ctx->rasterizer && ctx->rasterizer->flatshade,
			// TODO set .half_precision based on render target format,
			// ie. float16 and smaller use half, float32 use full..
			.half_precision = !!(fd_mesa_debug & FD_DBG_FRAGHALF),
			.has_per_samp = (fd4_ctx->fsaturate || fd4_ctx->vsaturate ||
					fd4_ctx->vinteger_s || fd4_ctx->finteger_s),
			.vsaturate_s = fd4_ctx->vsaturate_s,
			.vsaturate_t = fd4_ctx->vsaturate_t,
			.vsaturate_r = fd4_ctx->vsaturate_r,
			.fsaturate_s = fd4_ctx->fsaturate_s,
			.fsaturate_t = fd4_ctx->fsaturate_t,
			.fsaturate_r = fd4_ctx->fsaturate_r,
			.vinteger_s = fd4_ctx->vinteger_s,
			.finteger_s = fd4_ctx->finteger_s,
		},
		.format = fd4_emit_format(pfb->cbufs[0]),
		.pformat = pipe_surface_format(pfb->cbufs[0]),
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

	OUT_PKT0(ring, REG_A4XX_GRAS_CL_VPORT_XOFFSET_0, 4);
	OUT_RING(ring, A4XX_GRAS_CL_VPORT_XOFFSET_0(half_width));
	OUT_RING(ring, A4XX_GRAS_CL_VPORT_XSCALE_0(half_width));
	OUT_RING(ring, A4XX_GRAS_CL_VPORT_YOFFSET_0(half_height));
	OUT_RING(ring, A4XX_GRAS_CL_VPORT_YSCALE_0(-half_height));
}

static void
fd4_clear(struct fd_context *ctx, unsigned buffers,
		const union pipe_color_union *color, double depth, unsigned stencil)
{
	struct fd4_context *fd4_ctx = fd4_context(ctx);
	struct fd_ringbuffer *ring = ctx->ring;
	struct pipe_framebuffer_state *pfb = &ctx->framebuffer;
	unsigned dirty = ctx->dirty;
	unsigned ce, i;
	struct fd4_emit emit = {
		.vtx  = &fd4_ctx->solid_vbuf_state,
		.prog = &ctx->solid_prog,
		.key = {
			.half_precision = true,
		},
		.format = fd4_emit_format(pfb->cbufs[0]),
	};
	uint32_t colr = 0;

	if ((buffers & PIPE_CLEAR_COLOR) && pfb->nr_cbufs)
		colr  = pack_rgba(pfb->cbufs[0]->format, color->f);

	dirty &= FD_DIRTY_FRAMEBUFFER | FD_DIRTY_SCISSOR;
	dirty |= FD_DIRTY_PROG;
	emit.dirty = dirty;

	OUT_PKT0(ring, REG_A4XX_PC_PRIM_VTX_CNTL, 1);
	OUT_RING(ring, A4XX_PC_PRIM_VTX_CNTL_PROVOKING_VTX_LAST);

	/* emit generic state now: */
	fd4_emit_state(ctx, ring, &emit);
	reset_viewport(ring, pfb);

	if (buffers & PIPE_CLEAR_DEPTH) {
		OUT_PKT0(ring, REG_A4XX_RB_DEPTH_CONTROL, 1);
		OUT_RING(ring, A4XX_RB_DEPTH_CONTROL_Z_WRITE_ENABLE |
				A4XX_RB_DEPTH_CONTROL_Z_ENABLE |
				A4XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_ALWAYS));

		fd_wfi(ctx, ring);
		OUT_PKT0(ring, REG_A4XX_GRAS_CL_VPORT_ZOFFSET_0, 2);
		OUT_RING(ring, A4XX_GRAS_CL_VPORT_ZOFFSET_0(0.0));
		OUT_RING(ring, A4XX_GRAS_CL_VPORT_ZSCALE_0(depth));
		ctx->dirty |= FD_DIRTY_VIEWPORT;
	} else {
		OUT_PKT0(ring, REG_A4XX_RB_DEPTH_CONTROL, 1);
		OUT_RING(ring, A4XX_RB_DEPTH_CONTROL_ZFUNC(FUNC_NEVER));
	}

	if (buffers & PIPE_CLEAR_STENCIL) {
		OUT_PKT0(ring, REG_A4XX_RB_STENCILREFMASK, 2);
		OUT_RING(ring, A4XX_RB_STENCILREFMASK_STENCILREF(stencil) |
				A4XX_RB_STENCILREFMASK_STENCILMASK(stencil) |
				A4XX_RB_STENCILREFMASK_STENCILWRITEMASK(0xff));
		OUT_RING(ring, A4XX_RB_STENCILREFMASK_STENCILREF(0) |
				A4XX_RB_STENCILREFMASK_STENCILMASK(0) |
				0xff000000 | // XXX ???
				A4XX_RB_STENCILREFMASK_STENCILWRITEMASK(0xff));

		OUT_PKT0(ring, REG_A4XX_RB_STENCIL_CONTROL, 2);
		OUT_RING(ring, A4XX_RB_STENCIL_CONTROL_STENCIL_ENABLE |
				A4XX_RB_STENCIL_CONTROL_FUNC(FUNC_ALWAYS) |
				A4XX_RB_STENCIL_CONTROL_FAIL(STENCIL_KEEP) |
				A4XX_RB_STENCIL_CONTROL_ZPASS(STENCIL_REPLACE) |
				A4XX_RB_STENCIL_CONTROL_ZFAIL(STENCIL_KEEP) |
				A4XX_RB_STENCIL_CONTROL_FUNC_BF(FUNC_NEVER) |
				A4XX_RB_STENCIL_CONTROL_FAIL_BF(STENCIL_KEEP) |
				A4XX_RB_STENCIL_CONTROL_ZPASS_BF(STENCIL_KEEP) |
				A4XX_RB_STENCIL_CONTROL_ZFAIL_BF(STENCIL_KEEP));
		OUT_RING(ring, A4XX_RB_STENCIL_CONTROL2_STENCIL_BUFFER);
	} else {
		OUT_PKT0(ring, REG_A4XX_RB_STENCILREFMASK, 2);
		OUT_RING(ring, A4XX_RB_STENCILREFMASK_STENCILREF(0) |
				A4XX_RB_STENCILREFMASK_STENCILMASK(0) |
				A4XX_RB_STENCILREFMASK_STENCILWRITEMASK(0));
		OUT_RING(ring, A4XX_RB_STENCILREFMASK_BF_STENCILREF(0) |
				A4XX_RB_STENCILREFMASK_BF_STENCILMASK(0) |
				A4XX_RB_STENCILREFMASK_BF_STENCILWRITEMASK(0));

		OUT_PKT0(ring, REG_A4XX_RB_STENCIL_CONTROL, 2);
		OUT_RING(ring, A4XX_RB_STENCIL_CONTROL_FUNC(FUNC_NEVER) |
				A4XX_RB_STENCIL_CONTROL_FAIL(STENCIL_KEEP) |
				A4XX_RB_STENCIL_CONTROL_ZPASS(STENCIL_KEEP) |
				A4XX_RB_STENCIL_CONTROL_ZFAIL(STENCIL_KEEP) |
				A4XX_RB_STENCIL_CONTROL_FUNC_BF(FUNC_NEVER) |
				A4XX_RB_STENCIL_CONTROL_FAIL_BF(STENCIL_KEEP) |
				A4XX_RB_STENCIL_CONTROL_ZPASS_BF(STENCIL_KEEP) |
				A4XX_RB_STENCIL_CONTROL_ZFAIL_BF(STENCIL_KEEP));
		OUT_RING(ring, 0x00000000); /* RB_STENCIL_CONTROL2 */
	}

	if (buffers & PIPE_CLEAR_COLOR) {
		OUT_PKT0(ring, REG_A4XX_RB_ALPHA_CONTROL, 1);
		OUT_RING(ring, A4XX_RB_ALPHA_CONTROL_ALPHA_TEST_FUNC(FUNC_NEVER));
		ce = 0xf;
	} else {
		ce = 0x0;
	}

	for (i = 0; i < 8; i++) {
		OUT_PKT0(ring, REG_A4XX_RB_MRT_CONTROL(i), 1);
		OUT_RING(ring, A4XX_RB_MRT_CONTROL_FASTCLEAR |
				A4XX_RB_MRT_CONTROL_B11 |
				A4XX_RB_MRT_CONTROL_COMPONENT_ENABLE(ce));

		OUT_PKT0(ring, REG_A4XX_RB_MRT_BLEND_CONTROL(i), 1);
		OUT_RING(ring, A4XX_RB_MRT_BLEND_CONTROL_RGB_SRC_FACTOR(FACTOR_ONE) |
				A4XX_RB_MRT_BLEND_CONTROL_RGB_BLEND_OPCODE(BLEND_DST_PLUS_SRC) |
				A4XX_RB_MRT_BLEND_CONTROL_RGB_DEST_FACTOR(FACTOR_ZERO) |
				A4XX_RB_MRT_BLEND_CONTROL_ALPHA_SRC_FACTOR(FACTOR_ONE) |
				A4XX_RB_MRT_BLEND_CONTROL_ALPHA_BLEND_OPCODE(BLEND_DST_PLUS_SRC) |
				A4XX_RB_MRT_BLEND_CONTROL_ALPHA_DEST_FACTOR(FACTOR_ZERO));
	}

	fd4_emit_vertex_bufs(ring, &emit);

	OUT_PKT0(ring, REG_A4XX_GRAS_ALPHA_CONTROL, 1);
	OUT_RING(ring, 0x0);          /* XXX GRAS_ALPHA_CONTROL */

	OUT_PKT0(ring, REG_A4XX_GRAS_CLEAR_CNTL, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_RB_CLEAR_COLOR_DW0, 4);
	OUT_RING(ring, colr);         /* RB_CLEAR_COLOR_DW0 */
	OUT_RING(ring, colr);         /* RB_CLEAR_COLOR_DW1 */
	OUT_RING(ring, colr);         /* RB_CLEAR_COLOR_DW2 */
	OUT_RING(ring, colr);         /* RB_CLEAR_COLOR_DW3 */

	/* until fastclear works: */
	fd4_emit_constant(ring, SB_FRAG_SHADER, 0, 0, 4, color->ui, NULL);

	OUT_PKT0(ring, REG_A4XX_VFD_INDEX_OFFSET, 2);
	OUT_RING(ring, 0);            /* VFD_INDEX_OFFSET */
	OUT_RING(ring, 0);            /* ??? UNKNOWN_2209 */

	OUT_PKT0(ring, REG_A4XX_PC_RESTART_INDEX, 1);
	OUT_RING(ring, 0xffffffff);   /* PC_RESTART_INDEX */

	OUT_PKT3(ring, CP_UNKNOWN_1A, 1);
	OUT_RING(ring, 0x00000001);

	fd4_draw(ctx, ring, DI_PT_RECTLIST, USE_VISIBILITY,
			DI_SRC_SEL_AUTO_INDEX, 2, 1, INDEX_SIZE_IGN, 0, 0, NULL);

	OUT_PKT3(ring, CP_UNKNOWN_1A, 1);
	OUT_RING(ring, 0x00000000);

	OUT_PKT0(ring, REG_A4XX_GRAS_CLEAR_CNTL, 1);
	OUT_RING(ring, A4XX_GRAS_CLEAR_CNTL_NOT_FASTCLEAR);

	OUT_PKT0(ring, REG_A4XX_GRAS_SC_CONTROL, 1);
	OUT_RING(ring, A4XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
			A4XX_GRAS_SC_CONTROL_MSAA_DISABLE |
			A4XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
			A4XX_GRAS_SC_CONTROL_RASTER_MODE(0));
}

void
fd4_draw_init(struct pipe_context *pctx)
{
	struct fd_context *ctx = fd_context(pctx);
	ctx->draw_vbo = fd4_draw_vbo;
	ctx->clear = fd4_clear;
}
