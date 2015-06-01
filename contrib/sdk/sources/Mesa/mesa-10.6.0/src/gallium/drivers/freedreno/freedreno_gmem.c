/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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

#include "freedreno_gmem.h"
#include "freedreno_context.h"
#include "freedreno_resource.h"
#include "freedreno_query_hw.h"
#include "freedreno_util.h"

/*
 * GMEM is the small (ie. 256KiB for a200, 512KiB for a220, etc) tile buffer
 * inside the GPU.  All rendering happens to GMEM.  Larger render targets
 * are split into tiles that are small enough for the color (and depth and/or
 * stencil, if enabled) buffers to fit within GMEM.  Before rendering a tile,
 * if there was not a clear invalidating the previous tile contents, we need
 * to restore the previous tiles contents (system mem -> GMEM), and after all
 * the draw calls, before moving to the next tile, we need to save the tile
 * contents (GMEM -> system mem).
 *
 * The code in this file handles dealing with GMEM and tiling.
 *
 * The structure of the ringbuffer ends up being:
 *
 *     +--<---<-- IB ---<---+---<---+---<---<---<--+
 *     |                    |       |              |
 *     v                    ^       ^              ^
 *   ------------------------------------------------------
 *     | clear/draw cmds | Tile0 | Tile1 | .... | TileN |
 *   ------------------------------------------------------
 *                       ^
 *                       |
 *                       address submitted in issueibcmds
 *
 * Where the per-tile section handles scissor setup, mem2gmem restore (if
 * needed), IB to draw cmds earlier in the ringbuffer, and then gmem2mem
 * resolve.
 */

static uint32_t bin_width(struct fd_context *ctx)
{
	if (is_a4xx(ctx->screen))
		return 1024;
	if (is_a3xx(ctx->screen))
		return 992;
	return 512;
}

static uint32_t
total_size(uint8_t cbuf_cpp[], uint8_t zsbuf_cpp[2],
		   uint32_t bin_w, uint32_t bin_h, struct fd_gmem_stateobj *gmem)
{
	uint32_t total = 0, i;

	for (i = 0; i < 4; i++) {
		if (cbuf_cpp[i]) {
			gmem->cbuf_base[i] = align(total, 0x4000);
			total = gmem->cbuf_base[i] + cbuf_cpp[i] * bin_w * bin_h;
		}
	}

	if (zsbuf_cpp[0]) {
		gmem->zsbuf_base[0] = align(total, 0x4000);
		total = gmem->zsbuf_base[0] + zsbuf_cpp[0] * bin_w * bin_h;
	}

	if (zsbuf_cpp[1]) {
		gmem->zsbuf_base[1] = align(total, 0x4000);
		total = gmem->zsbuf_base[1] + zsbuf_cpp[1] * bin_w * bin_h;
	}

	return total;
}

static void
calculate_tiles(struct fd_context *ctx)
{
	struct fd_gmem_stateobj *gmem = &ctx->gmem;
	struct pipe_scissor_state *scissor = &ctx->max_scissor;
	struct pipe_framebuffer_state *pfb = &ctx->framebuffer;
	uint32_t gmem_size = ctx->screen->gmemsize_bytes;
	uint32_t minx, miny, width, height;
	uint32_t nbins_x = 1, nbins_y = 1;
	uint32_t bin_w, bin_h;
	uint32_t max_width = bin_width(ctx);
	uint8_t cbuf_cpp[4] = {0}, zsbuf_cpp[2] = {0};
	uint32_t i, j, t, xoff, yoff;
	uint32_t tpp_x, tpp_y;
	bool has_zs = !!(ctx->resolve & (FD_BUFFER_DEPTH | FD_BUFFER_STENCIL));
	int tile_n[ARRAY_SIZE(ctx->pipe)];

	if (has_zs) {
		struct fd_resource *rsc = fd_resource(pfb->zsbuf->texture);
		zsbuf_cpp[0] = rsc->cpp;
		if (rsc->stencil)
			zsbuf_cpp[1] = rsc->stencil->cpp;
	}
	for (i = 0; i < pfb->nr_cbufs; i++) {
		if (pfb->cbufs[i])
			cbuf_cpp[i] = util_format_get_blocksize(pfb->cbufs[i]->format);
		else
			cbuf_cpp[i] = 4;
	}

	if (!memcmp(gmem->zsbuf_cpp, zsbuf_cpp, sizeof(zsbuf_cpp)) &&
		!memcmp(gmem->cbuf_cpp, cbuf_cpp, sizeof(cbuf_cpp)) &&
		!memcmp(&gmem->scissor, scissor, sizeof(gmem->scissor))) {
		/* everything is up-to-date */
		return;
	}

	if (fd_mesa_debug & FD_DBG_NOSCIS) {
		minx = 0;
		miny = 0;
		width = pfb->width;
		height = pfb->height;
	} else {
		minx = scissor->minx & ~31; /* round down to multiple of 32 */
		miny = scissor->miny & ~31;
		width = scissor->maxx - minx;
		height = scissor->maxy - miny;
	}

	bin_w = align(width, 32);
	bin_h = align(height, 32);

	/* first, find a bin width that satisfies the maximum width
	 * restrictions:
	 */
	while (bin_w > max_width) {
		nbins_x++;
		bin_w = align(width / nbins_x, 32);
	}

	/* then find a bin width/height that satisfies the memory
	 * constraints:
	 */
	DBG("binning input: cbuf cpp: %d %d %d %d, zsbuf cpp: %d; %dx%d",
		cbuf_cpp[0], cbuf_cpp[1], cbuf_cpp[2], cbuf_cpp[3], zsbuf_cpp[0],
		width, height);
	while (total_size(cbuf_cpp, zsbuf_cpp, bin_w, bin_h, gmem) > gmem_size) {
		if (bin_w > bin_h) {
			nbins_x++;
			bin_w = align(width / nbins_x, 32);
		} else {
			nbins_y++;
			bin_h = align(height / nbins_y, 32);
		}
	}

	DBG("using %d bins of size %dx%d", nbins_x*nbins_y, bin_w, bin_h);

	gmem->scissor = *scissor;
	memcpy(gmem->cbuf_cpp, cbuf_cpp, sizeof(cbuf_cpp));
	memcpy(gmem->zsbuf_cpp, zsbuf_cpp, sizeof(zsbuf_cpp));
	gmem->bin_h = bin_h;
	gmem->bin_w = bin_w;
	gmem->nbins_x = nbins_x;
	gmem->nbins_y = nbins_y;
	gmem->minx = minx;
	gmem->miny = miny;
	gmem->width = width;
	gmem->height = height;

	/*
	 * Assign tiles and pipes:
	 *
	 * At some point it might be worth playing with different
	 * strategies and seeing if that makes much impact on
	 * performance.
	 */

#define div_round_up(v, a)  (((v) + (a) - 1) / (a))
	/* figure out number of tiles per pipe: */
	tpp_x = tpp_y = 1;
	while (div_round_up(nbins_y, tpp_y) > 8)
		tpp_y += 2;
	while ((div_round_up(nbins_y, tpp_y) *
			div_round_up(nbins_x, tpp_x)) > 8)
		tpp_x += 1;

	/* configure pipes: */
	xoff = yoff = 0;
	for (i = 0; i < ARRAY_SIZE(ctx->pipe); i++) {
		struct fd_vsc_pipe *pipe = &ctx->pipe[i];

		if (xoff >= nbins_x) {
			xoff = 0;
			yoff += tpp_y;
		}

		if (yoff >= nbins_y) {
			break;
		}

		pipe->x = xoff;
		pipe->y = yoff;
		pipe->w = MIN2(tpp_x, nbins_x - xoff);
		pipe->h = MIN2(tpp_y, nbins_y - yoff);

		xoff += tpp_x;
	}

	for (; i < ARRAY_SIZE(ctx->pipe); i++) {
		struct fd_vsc_pipe *pipe = &ctx->pipe[i];
		pipe->x = pipe->y = pipe->w = pipe->h = 0;
	}

#if 0 /* debug */
	printf("%dx%d ... tpp=%dx%d\n", nbins_x, nbins_y, tpp_x, tpp_y);
	for (i = 0; i < 8; i++) {
		struct fd_vsc_pipe *pipe = &ctx->pipe[i];
		printf("pipe[%d]: %ux%u @ %u,%u\n", i,
				pipe->w, pipe->h, pipe->x, pipe->y);
	}
#endif

	/* configure tiles: */
	t = 0;
	yoff = miny;
	memset(tile_n, 0, sizeof(tile_n));
	for (i = 0; i < nbins_y; i++) {
		uint32_t bw, bh;

		xoff = minx;

		/* clip bin height: */
		bh = MIN2(bin_h, miny + height - yoff);

		for (j = 0; j < nbins_x; j++) {
			struct fd_tile *tile = &ctx->tile[t];
			uint32_t p;

			assert(t < ARRAY_SIZE(ctx->tile));

			/* pipe number: */
			p = ((i / tpp_y) * div_round_up(nbins_x, tpp_x)) + (j / tpp_x);

			/* clip bin width: */
			bw = MIN2(bin_w, minx + width - xoff);

			tile->n = tile_n[p]++;
			tile->p = p;
			tile->bin_w = bw;
			tile->bin_h = bh;
			tile->xoff = xoff;
			tile->yoff = yoff;

			t++;

			xoff += bw;
		}

		yoff += bh;
	}

#if 0 /* debug */
	t = 0;
	for (i = 0; i < nbins_y; i++) {
		for (j = 0; j < nbins_x; j++) {
			struct fd_tile *tile = &ctx->tile[t++];
			printf("|p:%u n:%u|", tile->p, tile->n);
		}
		printf("\n");
	}
#endif
}

static void
render_tiles(struct fd_context *ctx)
{
	struct fd_gmem_stateobj *gmem = &ctx->gmem;
	int i;

	ctx->emit_tile_init(ctx);

	if (ctx->restore)
		ctx->stats.batch_restore++;

	for (i = 0; i < (gmem->nbins_x * gmem->nbins_y); i++) {
		struct fd_tile *tile = &ctx->tile[i];

		DBG("bin_h=%d, yoff=%d, bin_w=%d, xoff=%d",
			tile->bin_h, tile->yoff, tile->bin_w, tile->xoff);

		ctx->emit_tile_prep(ctx, tile);

		if (ctx->restore) {
			fd_hw_query_set_stage(ctx, ctx->ring, FD_STAGE_MEM2GMEM);
			ctx->emit_tile_mem2gmem(ctx, tile);
			fd_hw_query_set_stage(ctx, ctx->ring, FD_STAGE_NULL);
		}

		ctx->emit_tile_renderprep(ctx, tile);

		fd_hw_query_prepare_tile(ctx, i, ctx->ring);

		/* emit IB to drawcmds: */
		OUT_IB(ctx->ring, ctx->draw_start, ctx->draw_end);
		fd_reset_wfi(ctx);

		/* emit gmem2mem to transfer tile back to system memory: */
		fd_hw_query_set_stage(ctx, ctx->ring, FD_STAGE_GMEM2MEM);
		ctx->emit_tile_gmem2mem(ctx, tile);
		fd_hw_query_set_stage(ctx, ctx->ring, FD_STAGE_NULL);
	}
}

static void
render_sysmem(struct fd_context *ctx)
{
	ctx->emit_sysmem_prep(ctx);

	fd_hw_query_prepare_tile(ctx, 0, ctx->ring);

	/* emit IB to drawcmds: */
	OUT_IB(ctx->ring, ctx->draw_start, ctx->draw_end);
	fd_reset_wfi(ctx);
}

void
fd_gmem_render_tiles(struct fd_context *ctx)
{
	struct pipe_framebuffer_state *pfb = &ctx->framebuffer;
	bool sysmem = false;

	if (ctx->emit_sysmem_prep) {
		if (ctx->cleared || ctx->gmem_reason || (ctx->num_draws > 5)) {
			DBG("GMEM: cleared=%x, gmem_reason=%x, num_draws=%u",
				ctx->cleared, ctx->gmem_reason, ctx->num_draws);
		} else if (!(fd_mesa_debug & FD_DBG_NOBYPASS)) {
			sysmem = true;
		}
	}

	/* close out the draw cmds by making sure any active queries are
	 * paused:
	 */
	fd_hw_query_set_stage(ctx, ctx->ring, FD_STAGE_NULL);

	/* mark the end of the clear/draw cmds before emitting per-tile cmds: */
	fd_ringmarker_mark(ctx->draw_end);
	fd_ringmarker_mark(ctx->binning_end);

	fd_reset_wfi(ctx);

	ctx->stats.batch_total++;

	if (sysmem) {
		DBG("rendering sysmem (%s/%s)",
			util_format_short_name(pipe_surface_format(pfb->cbufs[0])),
			util_format_short_name(pipe_surface_format(pfb->zsbuf)));
		fd_hw_query_prepare(ctx, 1);
		render_sysmem(ctx);
		ctx->stats.batch_sysmem++;
	} else {
		struct fd_gmem_stateobj *gmem = &ctx->gmem;
		calculate_tiles(ctx);
		DBG("rendering %dx%d tiles (%s/%s)", gmem->nbins_x, gmem->nbins_y,
			util_format_short_name(pipe_surface_format(pfb->cbufs[0])),
			util_format_short_name(pipe_surface_format(pfb->zsbuf)));
		fd_hw_query_prepare(ctx, gmem->nbins_x * gmem->nbins_y);
		render_tiles(ctx);
		ctx->stats.batch_gmem++;
	}

	/* GPU executes starting from tile cmds, which IB back to draw cmds: */
	fd_ringmarker_flush(ctx->draw_end);

	/* mark start for next draw/binning cmds: */
	fd_ringmarker_mark(ctx->draw_start);
	fd_ringmarker_mark(ctx->binning_start);

	fd_reset_wfi(ctx);

	/* reset maximal bounds: */
	ctx->max_scissor.minx = ctx->max_scissor.miny = ~0;
	ctx->max_scissor.maxx = ctx->max_scissor.maxy = 0;

	ctx->dirty = ~0;
}

/* tile needs restore if it isn't completely contained within the
 * cleared scissor:
 */
static bool
skip_restore(struct pipe_scissor_state *scissor, struct fd_tile *tile)
{
	unsigned minx = tile->xoff;
	unsigned maxx = tile->xoff + tile->bin_w;
	unsigned miny = tile->yoff;
	unsigned maxy = tile->yoff + tile->bin_h;
	return (minx >= scissor->minx) && (maxx <= scissor->maxx) &&
			(miny >= scissor->miny) && (maxy <= scissor->maxy);
}

/* When deciding whether a tile needs mem2gmem, we need to take into
 * account the scissor rect(s) that were cleared.  To simplify we only
 * consider the last scissor rect for each buffer, since the common
 * case would be a single clear.
 */
bool
fd_gmem_needs_restore(struct fd_context *ctx, struct fd_tile *tile,
		uint32_t buffers)
{
	if (!(ctx->restore & buffers))
		return false;

	/* if buffers partially cleared, then slow-path to figure out
	 * if this particular tile needs restoring:
	 */
	if ((buffers & FD_BUFFER_COLOR) &&
			(ctx->partial_cleared & FD_BUFFER_COLOR) &&
			skip_restore(&ctx->cleared_scissor.color, tile))
		return false;
	if ((buffers & FD_BUFFER_DEPTH) &&
			(ctx->partial_cleared & FD_BUFFER_DEPTH) &&
			skip_restore(&ctx->cleared_scissor.depth, tile))
		return false;
	if ((buffers & FD_BUFFER_STENCIL) &&
			(ctx->partial_cleared & FD_BUFFER_STENCIL) &&
			skip_restore(&ctx->cleared_scissor.stencil, tile))
		return false;

	return true;
}
