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

#include "util/u_format.h"
#include "util/u_format_zs.h"
#include "util/u_inlines.h"
#include "util/u_transfer.h"
#include "util/u_string.h"
#include "util/u_surface.h"

#include "freedreno_resource.h"
#include "freedreno_screen.h"
#include "freedreno_surface.h"
#include "freedreno_context.h"
#include "freedreno_query_hw.h"
#include "freedreno_util.h"

#include <errno.h>

static void
fd_invalidate_resource(struct fd_context *ctx, struct pipe_resource *prsc)
{
	int i;

	/* Go through the entire state and see if the resource is bound
	 * anywhere. If it is, mark the relevant state as dirty. This is called on
	 * realloc_bo.
	 */

	/* Constbufs */
	for (i = 1; i < PIPE_MAX_CONSTANT_BUFFERS && !(ctx->dirty & FD_DIRTY_CONSTBUF); i++) {
		if (ctx->constbuf[PIPE_SHADER_VERTEX].cb[i].buffer == prsc)
			ctx->dirty |= FD_DIRTY_CONSTBUF;
		if (ctx->constbuf[PIPE_SHADER_FRAGMENT].cb[i].buffer == prsc)
			ctx->dirty |= FD_DIRTY_CONSTBUF;
	}

	/* VBOs */
	for (i = 0; i < ctx->vtx.vertexbuf.count && !(ctx->dirty & FD_DIRTY_VTXBUF); i++) {
		if (ctx->vtx.vertexbuf.vb[i].buffer == prsc)
			ctx->dirty |= FD_DIRTY_VTXBUF;
	}

	/* Index buffer */
	if (ctx->indexbuf.buffer == prsc)
		ctx->dirty |= FD_DIRTY_INDEXBUF;

	/* Textures */
	for (i = 0; i < ctx->verttex.num_textures && !(ctx->dirty & FD_DIRTY_VERTTEX); i++) {
		if (ctx->verttex.textures[i]->texture == prsc)
			ctx->dirty |= FD_DIRTY_VERTTEX;
	}
	for (i = 0; i < ctx->fragtex.num_textures && !(ctx->dirty & FD_DIRTY_FRAGTEX); i++) {
		if (ctx->fragtex.textures[i]->texture == prsc)
			ctx->dirty |= FD_DIRTY_FRAGTEX;
	}
}

static void
realloc_bo(struct fd_resource *rsc, uint32_t size)
{
	struct fd_screen *screen = fd_screen(rsc->base.b.screen);
	uint32_t flags = DRM_FREEDRENO_GEM_CACHE_WCOMBINE |
			DRM_FREEDRENO_GEM_TYPE_KMEM; /* TODO */

	/* if we start using things other than write-combine,
	 * be sure to check for PIPE_RESOURCE_FLAG_MAP_COHERENT
	 */

	if (rsc->bo)
		fd_bo_del(rsc->bo);

	rsc->bo = fd_bo_new(screen->dev, size, flags);
	rsc->timestamp = 0;
	rsc->dirty = rsc->reading = false;
	list_delinit(&rsc->list);
	util_range_set_empty(&rsc->valid_buffer_range);
}

/* Currently this is only used for flushing Z32_S8 texture transfers, but
 * eventually it should handle everything.
 */
static void
fd_resource_flush(struct fd_transfer *trans, const struct pipe_box *box)
{
	struct fd_resource *rsc = fd_resource(trans->base.resource);
	struct fd_resource_slice *slice = fd_resource_slice(rsc, trans->base.level);
	struct fd_resource_slice *sslice = fd_resource_slice(rsc->stencil, trans->base.level);
	enum pipe_format format = trans->base.resource->format;

	float *depth = fd_bo_map(rsc->bo) + slice->offset +
		(trans->base.box.y + box->y) * slice->pitch * 4 + (trans->base.box.x + box->x) * 4;
	uint8_t *stencil = fd_bo_map(rsc->stencil->bo) + sslice->offset +
		(trans->base.box.y + box->y) * sslice->pitch + trans->base.box.x + box->x;

	assert(format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT ||
		   format == PIPE_FORMAT_X32_S8X24_UINT);

	if (format != PIPE_FORMAT_X32_S8X24_UINT)
		util_format_z32_float_s8x24_uint_unpack_z_float(
				depth, slice->pitch * 4,
				trans->staging, trans->base.stride,
				box->width, box->height);

	util_format_z32_float_s8x24_uint_unpack_s_8uint(
			stencil, sslice->pitch,
			trans->staging, trans->base.stride,
			box->width, box->height);
}

static void fd_resource_transfer_flush_region(struct pipe_context *pctx,
		struct pipe_transfer *ptrans,
		const struct pipe_box *box)
{
	struct fd_resource *rsc = fd_resource(ptrans->resource);
	struct fd_transfer *trans = fd_transfer(ptrans);

	if (ptrans->resource->target == PIPE_BUFFER)
		util_range_add(&rsc->valid_buffer_range,
					   ptrans->box.x + box->x,
					   ptrans->box.x + box->x + box->width);

	if (trans->staging)
		fd_resource_flush(trans, box);
}

static void
fd_resource_transfer_unmap(struct pipe_context *pctx,
		struct pipe_transfer *ptrans)
{
	struct fd_context *ctx = fd_context(pctx);
	struct fd_resource *rsc = fd_resource(ptrans->resource);
	struct fd_transfer *trans = fd_transfer(ptrans);

	if (trans->staging && !(ptrans->usage & PIPE_TRANSFER_FLUSH_EXPLICIT)) {
		struct pipe_box box;
		u_box_2d(0, 0, ptrans->box.width, ptrans->box.height, &box);
		fd_resource_flush(trans, &box);
	}

	if (!(ptrans->usage & PIPE_TRANSFER_UNSYNCHRONIZED)) {
		fd_bo_cpu_fini(rsc->bo);
		if (rsc->stencil)
			fd_bo_cpu_fini(rsc->stencil->bo);
	}

	util_range_add(&rsc->valid_buffer_range,
				   ptrans->box.x,
				   ptrans->box.x + ptrans->box.width);

	pipe_resource_reference(&ptrans->resource, NULL);
	util_slab_free(&ctx->transfer_pool, ptrans);

	if (trans->staging)
		free(trans->staging);
}

static void *
fd_resource_transfer_map(struct pipe_context *pctx,
		struct pipe_resource *prsc,
		unsigned level, unsigned usage,
		const struct pipe_box *box,
		struct pipe_transfer **pptrans)
{
	struct fd_context *ctx = fd_context(pctx);
	struct fd_resource *rsc = fd_resource(prsc);
	struct fd_resource_slice *slice = fd_resource_slice(rsc, level);
	struct fd_transfer *trans;
	struct pipe_transfer *ptrans;
	enum pipe_format format = prsc->format;
	uint32_t op = 0;
	uint32_t offset;
	char *buf;
	int ret = 0;

	DBG("prsc=%p, level=%u, usage=%x, box=%dx%d+%d,%d", prsc, level, usage,
		box->width, box->height, box->x, box->y);

	ptrans = util_slab_alloc(&ctx->transfer_pool);
	if (!ptrans)
		return NULL;

	/* util_slab_alloc() doesn't zero: */
	trans = fd_transfer(ptrans);
	memset(trans, 0, sizeof(*trans));

	pipe_resource_reference(&ptrans->resource, prsc);
	ptrans->level = level;
	ptrans->usage = usage;
	ptrans->box = *box;
	ptrans->stride = slice->pitch * rsc->cpp;
	ptrans->layer_stride = slice->size0;

	if (usage & PIPE_TRANSFER_READ)
		op |= DRM_FREEDRENO_PREP_READ;

	if (usage & PIPE_TRANSFER_WRITE)
		op |= DRM_FREEDRENO_PREP_WRITE;

	if (usage & PIPE_TRANSFER_DISCARD_WHOLE_RESOURCE) {
		realloc_bo(rsc, fd_bo_size(rsc->bo));
		if (rsc->stencil)
			realloc_bo(rsc->stencil, fd_bo_size(rsc->stencil->bo));
		fd_invalidate_resource(ctx, prsc);
	} else if ((usage & PIPE_TRANSFER_WRITE) &&
			   prsc->target == PIPE_BUFFER &&
			   !util_ranges_intersect(&rsc->valid_buffer_range,
									  box->x, box->x + box->width)) {
		/* We are trying to write to a previously uninitialized range. No need
		 * to wait.
		 */
	} else if (!(usage & PIPE_TRANSFER_UNSYNCHRONIZED)) {
		/* If the GPU is writing to the resource, or if it is reading from the
		 * resource and we're trying to write to it, flush the renders.
		 */
		if (rsc->dirty || (rsc->stencil && rsc->stencil->dirty) ||
			((ptrans->usage & PIPE_TRANSFER_WRITE) && rsc->reading))
			fd_context_render(pctx);

		/* The GPU keeps track of how the various bo's are being used, and
		 * will wait if necessary for the proper operation to have
		 * completed.
		 */
		ret = fd_bo_cpu_prep(rsc->bo, ctx->screen->pipe, op);
		if (ret)
			goto fail;
	}

	buf = fd_bo_map(rsc->bo);
	if (!buf) {
		fd_resource_transfer_unmap(pctx, ptrans);
		return NULL;
	}

	if (rsc->layer_first) {
		offset = slice->offset +
			box->y / util_format_get_blockheight(format) * ptrans->stride +
			box->x / util_format_get_blockwidth(format) * rsc->cpp +
			box->z * rsc->layer_size;
	} else {
		offset = slice->offset +
			box->y / util_format_get_blockheight(format) * ptrans->stride +
			box->x / util_format_get_blockwidth(format) * rsc->cpp +
			box->z * slice->size0;
	}

	if (prsc->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT ||
		prsc->format == PIPE_FORMAT_X32_S8X24_UINT) {
		trans->base.stride = trans->base.box.width * rsc->cpp * 2;
		trans->staging = malloc(trans->base.stride * trans->base.box.height);
		if (!trans->staging)
			goto fail;

		/* if we're not discarding the whole range (or resource), we must copy
		 * the real data in.
		 */
		if (!(usage & (PIPE_TRANSFER_DISCARD_WHOLE_RESOURCE |
					   PIPE_TRANSFER_DISCARD_RANGE))) {
			struct fd_resource_slice *sslice =
				fd_resource_slice(rsc->stencil, level);
			void *sbuf = fd_bo_map(rsc->stencil->bo);
			if (!sbuf)
				goto fail;

			float *depth = (float *)(buf + slice->offset +
				box->y * slice->pitch * 4 + box->x * 4);
			uint8_t *stencil = sbuf + sslice->offset +
				box->y * sslice->pitch + box->x;

			if (format != PIPE_FORMAT_X32_S8X24_UINT)
				util_format_z32_float_s8x24_uint_pack_z_float(
						trans->staging, trans->base.stride,
						depth, slice->pitch * 4,
						box->width, box->height);

			util_format_z32_float_s8x24_uint_pack_s_8uint(
					trans->staging, trans->base.stride,
					stencil, sslice->pitch,
					box->width, box->height);
		}

		buf = trans->staging;
		offset = 0;
	}

	*pptrans = ptrans;

	return buf + offset;

fail:
	fd_resource_transfer_unmap(pctx, ptrans);
	return NULL;
}

static void
fd_resource_destroy(struct pipe_screen *pscreen,
		struct pipe_resource *prsc)
{
	struct fd_resource *rsc = fd_resource(prsc);
	if (rsc->bo)
		fd_bo_del(rsc->bo);
	list_delinit(&rsc->list);
	util_range_destroy(&rsc->valid_buffer_range);
	FREE(rsc);
}

static boolean
fd_resource_get_handle(struct pipe_screen *pscreen,
		struct pipe_resource *prsc,
		struct winsys_handle *handle)
{
	struct fd_resource *rsc = fd_resource(prsc);

	return fd_screen_bo_get_handle(pscreen, rsc->bo,
			rsc->slices[0].pitch * rsc->cpp, handle);
}


static const struct u_resource_vtbl fd_resource_vtbl = {
		.resource_get_handle      = fd_resource_get_handle,
		.resource_destroy         = fd_resource_destroy,
		.transfer_map             = fd_resource_transfer_map,
		.transfer_flush_region    = fd_resource_transfer_flush_region,
		.transfer_unmap           = fd_resource_transfer_unmap,
		.transfer_inline_write    = u_default_transfer_inline_write,
};

static uint32_t
setup_slices(struct fd_resource *rsc, uint32_t alignment)
{
	struct pipe_resource *prsc = &rsc->base.b;
	uint32_t level, size = 0;
	uint32_t width = prsc->width0;
	uint32_t height = prsc->height0;
	uint32_t depth = prsc->depth0;
	/* in layer_first layout, the level (slice) contains just one
	 * layer (since in fact the layer contains the slices)
	 */
	uint32_t layers_in_level = rsc->layer_first ? 1 : prsc->array_size;

	for (level = 0; level <= prsc->last_level; level++) {
		struct fd_resource_slice *slice = fd_resource_slice(rsc, level);

		slice->pitch = width = align(width, 32);
		slice->offset = size;
		/* 1d array and 2d array textures must all have the same layer size
		 * for each miplevel on a3xx. 3d textures can have different layer
		 * sizes for high levels, but the hw auto-sizer is buggy (or at least
		 * different than what this code does), so as soon as the layer size
		 * range gets into range, we stop reducing it.
		 */
		if (prsc->target == PIPE_TEXTURE_3D && (
					level == 1 ||
					(level > 1 && rsc->slices[level - 1].size0 > 0xf000)))
			slice->size0 = align(slice->pitch * height * rsc->cpp, alignment);
		else if (level == 0 || rsc->layer_first || alignment == 1)
			slice->size0 = align(slice->pitch * height * rsc->cpp, alignment);
		else
			slice->size0 = rsc->slices[level - 1].size0;

		size += slice->size0 * depth * layers_in_level;

		width = u_minify(width, 1);
		height = u_minify(height, 1);
		depth = u_minify(depth, 1);
	}

	return size;
}

static uint32_t
slice_alignment(struct pipe_screen *pscreen, const struct pipe_resource *tmpl)
{
	/* on a3xx, 2d array and 3d textures seem to want their
	 * layers aligned to page boundaries:
	 */
	switch (tmpl->target) {
	case PIPE_TEXTURE_3D:
	case PIPE_TEXTURE_1D_ARRAY:
	case PIPE_TEXTURE_2D_ARRAY:
		return 4096;
	default:
		return 1;
	}
}

/**
 * Create a new texture object, using the given template info.
 */
static struct pipe_resource *
fd_resource_create(struct pipe_screen *pscreen,
		const struct pipe_resource *tmpl)
{
	struct fd_resource *rsc = CALLOC_STRUCT(fd_resource);
	struct pipe_resource *prsc = &rsc->base.b;
	uint32_t size;

	DBG("target=%d, format=%s, %ux%ux%u, array_size=%u, last_level=%u, "
			"nr_samples=%u, usage=%u, bind=%x, flags=%x",
			tmpl->target, util_format_name(tmpl->format),
			tmpl->width0, tmpl->height0, tmpl->depth0,
			tmpl->array_size, tmpl->last_level, tmpl->nr_samples,
			tmpl->usage, tmpl->bind, tmpl->flags);

	if (!rsc)
		return NULL;

	*prsc = *tmpl;

	pipe_reference_init(&prsc->reference, 1);
	list_inithead(&rsc->list);
	prsc->screen = pscreen;

	util_range_init(&rsc->valid_buffer_range);

	rsc->base.vtbl = &fd_resource_vtbl;
	if (tmpl->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT)
		rsc->cpp = util_format_get_blocksize(PIPE_FORMAT_Z32_FLOAT);
	else
		rsc->cpp = util_format_get_blocksize(tmpl->format);

	assert(rsc->cpp);

	if (is_a4xx(fd_screen(pscreen))) {
		switch (tmpl->target) {
		case PIPE_TEXTURE_3D:
			/* TODO 3D_ARRAY? */
			rsc->layer_first = false;
			break;
		default:
			rsc->layer_first = true;
			break;
		}
	}

	size = setup_slices(rsc, slice_alignment(pscreen, tmpl));

	if (rsc->layer_first) {
		rsc->layer_size = align(size, 4096);
		size = rsc->layer_size * prsc->array_size;
	}

	realloc_bo(rsc, size);
	if (!rsc->bo)
		goto fail;

	/* There is no native Z32F_S8 sampling or rendering format, so this must
	 * be emulated via two separate textures. The depth texture still keeps
	 * its Z32F_S8 format though, and we also keep a reference to a separate
	 * S8 texture.
	 */
	if (tmpl->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT) {
		struct pipe_resource stencil = *tmpl;
		stencil.format = PIPE_FORMAT_S8_UINT;
		rsc->stencil = fd_resource(fd_resource_create(pscreen, &stencil));
		if (!rsc->stencil)
			goto fail;
	}

	return prsc;
fail:
	fd_resource_destroy(pscreen, prsc);
	return NULL;
}

/**
 * Create a texture from a winsys_handle. The handle is often created in
 * another process by first creating a pipe texture and then calling
 * resource_get_handle.
 */
static struct pipe_resource *
fd_resource_from_handle(struct pipe_screen *pscreen,
		const struct pipe_resource *tmpl,
		struct winsys_handle *handle)
{
	struct fd_resource *rsc = CALLOC_STRUCT(fd_resource);
	struct fd_resource_slice *slice = &rsc->slices[0];
	struct pipe_resource *prsc = &rsc->base.b;

	DBG("target=%d, format=%s, %ux%ux%u, array_size=%u, last_level=%u, "
			"nr_samples=%u, usage=%u, bind=%x, flags=%x",
			tmpl->target, util_format_name(tmpl->format),
			tmpl->width0, tmpl->height0, tmpl->depth0,
			tmpl->array_size, tmpl->last_level, tmpl->nr_samples,
			tmpl->usage, tmpl->bind, tmpl->flags);

	if (!rsc)
		return NULL;

	*prsc = *tmpl;

	pipe_reference_init(&prsc->reference, 1);
	list_inithead(&rsc->list);
	prsc->screen = pscreen;

	util_range_init(&rsc->valid_buffer_range);

	rsc->bo = fd_screen_bo_from_handle(pscreen, handle, &slice->pitch);
	if (!rsc->bo)
		goto fail;

	rsc->base.vtbl = &fd_resource_vtbl;
	rsc->cpp = util_format_get_blocksize(tmpl->format);
	slice->pitch /= rsc->cpp;

	assert(rsc->cpp);

	return prsc;

fail:
	fd_resource_destroy(pscreen, prsc);
	return NULL;
}

static void fd_blitter_pipe_begin(struct fd_context *ctx);
static void fd_blitter_pipe_end(struct fd_context *ctx);

/**
 * _copy_region using pipe (3d engine)
 */
static bool
fd_blitter_pipe_copy_region(struct fd_context *ctx,
		struct pipe_resource *dst,
		unsigned dst_level,
		unsigned dstx, unsigned dsty, unsigned dstz,
		struct pipe_resource *src,
		unsigned src_level,
		const struct pipe_box *src_box)
{
	/* not until we allow rendertargets to be buffers */
	if (dst->target == PIPE_BUFFER || src->target == PIPE_BUFFER)
		return false;

	if (!util_blitter_is_copy_supported(ctx->blitter, dst, src))
		return false;

	fd_blitter_pipe_begin(ctx);
	util_blitter_copy_texture(ctx->blitter,
			dst, dst_level, dstx, dsty, dstz,
			src, src_level, src_box);
	fd_blitter_pipe_end(ctx);

	return true;
}

/**
 * Copy a block of pixels from one resource to another.
 * The resource must be of the same format.
 * Resources with nr_samples > 1 are not allowed.
 */
static void
fd_resource_copy_region(struct pipe_context *pctx,
		struct pipe_resource *dst,
		unsigned dst_level,
		unsigned dstx, unsigned dsty, unsigned dstz,
		struct pipe_resource *src,
		unsigned src_level,
		const struct pipe_box *src_box)
{
	struct fd_context *ctx = fd_context(pctx);

	/* TODO if we have 2d core, or other DMA engine that could be used
	 * for simple copies and reasonably easily synchronized with the 3d
	 * core, this is where we'd plug it in..
	 */

	/* try blit on 3d pipe: */
	if (fd_blitter_pipe_copy_region(ctx,
			dst, dst_level, dstx, dsty, dstz,
			src, src_level, src_box))
		return;

	/* else fallback to pure sw: */
	util_resource_copy_region(pctx,
			dst, dst_level, dstx, dsty, dstz,
			src, src_level, src_box);
}

/**
 * Optimal hardware path for blitting pixels.
 * Scaling, format conversion, up- and downsampling (resolve) are allowed.
 */
static void
fd_blit(struct pipe_context *pctx, const struct pipe_blit_info *blit_info)
{
	struct fd_context *ctx = fd_context(pctx);
	struct pipe_blit_info info = *blit_info;

	if (info.src.resource->nr_samples > 1 &&
			info.dst.resource->nr_samples <= 1 &&
			!util_format_is_depth_or_stencil(info.src.resource->format) &&
			!util_format_is_pure_integer(info.src.resource->format)) {
		DBG("color resolve unimplemented");
		return;
	}

	if (util_try_blit_via_copy_region(pctx, &info)) {
		return; /* done */
	}

	if (info.mask & PIPE_MASK_S) {
		DBG("cannot blit stencil, skipping");
		info.mask &= ~PIPE_MASK_S;
	}

	if (!util_blitter_is_blit_supported(ctx->blitter, &info)) {
		DBG("blit unsupported %s -> %s",
				util_format_short_name(info.src.resource->format),
				util_format_short_name(info.dst.resource->format));
		return;
	}

	fd_blitter_pipe_begin(ctx);
	util_blitter_blit(ctx->blitter, &info);
	fd_blitter_pipe_end(ctx);
}

static void
fd_blitter_pipe_begin(struct fd_context *ctx)
{
	util_blitter_save_vertex_buffer_slot(ctx->blitter, ctx->vtx.vertexbuf.vb);
	util_blitter_save_vertex_elements(ctx->blitter, ctx->vtx.vtx);
	util_blitter_save_vertex_shader(ctx->blitter, ctx->prog.vp);
	util_blitter_save_rasterizer(ctx->blitter, ctx->rasterizer);
	util_blitter_save_viewport(ctx->blitter, &ctx->viewport);
	util_blitter_save_scissor(ctx->blitter, &ctx->scissor);
	util_blitter_save_fragment_shader(ctx->blitter, ctx->prog.fp);
	util_blitter_save_blend(ctx->blitter, ctx->blend);
	util_blitter_save_depth_stencil_alpha(ctx->blitter, ctx->zsa);
	util_blitter_save_stencil_ref(ctx->blitter, &ctx->stencil_ref);
	util_blitter_save_sample_mask(ctx->blitter, ctx->sample_mask);
	util_blitter_save_framebuffer(ctx->blitter, &ctx->framebuffer);
	util_blitter_save_fragment_sampler_states(ctx->blitter,
			ctx->fragtex.num_samplers,
			(void **)ctx->fragtex.samplers);
	util_blitter_save_fragment_sampler_views(ctx->blitter,
			ctx->fragtex.num_textures, ctx->fragtex.textures);

	fd_hw_query_set_stage(ctx, ctx->ring, FD_STAGE_BLIT);
}

static void
fd_blitter_pipe_end(struct fd_context *ctx)
{
	fd_hw_query_set_stage(ctx, ctx->ring, FD_STAGE_NULL);
}

static void
fd_flush_resource(struct pipe_context *pctx, struct pipe_resource *prsc)
{
	struct fd_resource *rsc = fd_resource(prsc);

	if (rsc->dirty || (rsc->stencil && rsc->stencil->dirty))
		fd_context_render(pctx);
}

void
fd_resource_screen_init(struct pipe_screen *pscreen)
{
	pscreen->resource_create = fd_resource_create;
	pscreen->resource_from_handle = fd_resource_from_handle;
	pscreen->resource_get_handle = u_resource_get_handle_vtbl;
	pscreen->resource_destroy = u_resource_destroy_vtbl;
}

void
fd_resource_context_init(struct pipe_context *pctx)
{
	pctx->transfer_map = u_transfer_map_vtbl;
	pctx->transfer_flush_region = u_transfer_flush_region_vtbl;
	pctx->transfer_unmap = u_transfer_unmap_vtbl;
	pctx->transfer_inline_write = u_transfer_inline_write_vtbl;
	pctx->create_surface = fd_create_surface;
	pctx->surface_destroy = fd_surface_destroy;
	pctx->resource_copy_region = fd_resource_copy_region;
	pctx->blit = fd_blit;
	pctx->flush_resource = fd_flush_resource;
}
