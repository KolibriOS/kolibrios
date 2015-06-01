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


#include "fd4_context.h"
#include "fd4_blend.h"
#include "fd4_draw.h"
#include "fd4_emit.h"
#include "fd4_gmem.h"
#include "fd4_program.h"
#include "fd4_query.h"
#include "fd4_rasterizer.h"
#include "fd4_texture.h"
#include "fd4_zsa.h"

static void
fd4_context_destroy(struct pipe_context *pctx)
{
	struct fd4_context *fd4_ctx = fd4_context(fd_context(pctx));

	util_dynarray_fini(&fd4_ctx->rbrc_patches);

	fd_bo_del(fd4_ctx->vs_pvt_mem);
	fd_bo_del(fd4_ctx->fs_pvt_mem);
	fd_bo_del(fd4_ctx->vsc_size_mem);

	pctx->delete_vertex_elements_state(pctx, fd4_ctx->solid_vbuf_state.vtx);
	pctx->delete_vertex_elements_state(pctx, fd4_ctx->blit_vbuf_state.vtx);

	pipe_resource_reference(&fd4_ctx->solid_vbuf, NULL);
	pipe_resource_reference(&fd4_ctx->blit_texcoord_vbuf, NULL);

	fd_context_destroy(pctx);
}

/* TODO we could combine a few of these small buffers (solid_vbuf,
 * blit_texcoord_vbuf, and vsc_size_mem, into a single buffer and
 * save a tiny bit of memory
 */

static struct pipe_resource *
create_solid_vertexbuf(struct pipe_context *pctx)
{
	static const float init_shader_const[] = {
			-1.000000, +1.000000, +1.000000,
			+1.000000, -1.000000, +1.000000,
	};
	struct pipe_resource *prsc = pipe_buffer_create(pctx->screen,
			PIPE_BIND_CUSTOM, PIPE_USAGE_IMMUTABLE, sizeof(init_shader_const));
	pipe_buffer_write(pctx, prsc, 0,
			sizeof(init_shader_const), init_shader_const);
	return prsc;
}

static struct pipe_resource *
create_blit_texcoord_vertexbuf(struct pipe_context *pctx)
{
	struct pipe_resource *prsc = pipe_buffer_create(pctx->screen,
			PIPE_BIND_CUSTOM, PIPE_USAGE_DYNAMIC, 16);
	return prsc;
}

static const uint8_t primtypes[PIPE_PRIM_MAX] = {
		[PIPE_PRIM_POINTS]         = DI_PT_POINTLIST_A3XX,
		[PIPE_PRIM_LINES]          = DI_PT_LINELIST,
		[PIPE_PRIM_LINE_STRIP]     = DI_PT_LINESTRIP,
		[PIPE_PRIM_LINE_LOOP]      = DI_PT_LINELOOP,
		[PIPE_PRIM_TRIANGLES]      = DI_PT_TRILIST,
		[PIPE_PRIM_TRIANGLE_STRIP] = DI_PT_TRISTRIP,
		[PIPE_PRIM_TRIANGLE_FAN]   = DI_PT_TRIFAN,
};

struct pipe_context *
fd4_context_create(struct pipe_screen *pscreen, void *priv)
{
	struct fd_screen *screen = fd_screen(pscreen);
	struct fd4_context *fd4_ctx = CALLOC_STRUCT(fd4_context);
	struct pipe_context *pctx;

	if (!fd4_ctx)
		return NULL;

	pctx = &fd4_ctx->base.base;

	fd4_ctx->base.dev = fd_device_ref(screen->dev);
	fd4_ctx->base.screen = fd_screen(pscreen);

	pctx->destroy = fd4_context_destroy;
	pctx->create_blend_state = fd4_blend_state_create;
	pctx->create_rasterizer_state = fd4_rasterizer_state_create;
	pctx->create_depth_stencil_alpha_state = fd4_zsa_state_create;

	fd4_draw_init(pctx);
	fd4_gmem_init(pctx);
	fd4_texture_init(pctx);
	fd4_prog_init(pctx);

	pctx = fd_context_init(&fd4_ctx->base, pscreen, primtypes, priv);
	if (!pctx)
		return NULL;

	util_dynarray_init(&fd4_ctx->rbrc_patches);

	fd4_ctx->vs_pvt_mem = fd_bo_new(screen->dev, 0x2000,
			DRM_FREEDRENO_GEM_TYPE_KMEM);

	fd4_ctx->fs_pvt_mem = fd_bo_new(screen->dev, 0x2000,
			DRM_FREEDRENO_GEM_TYPE_KMEM);

	fd4_ctx->vsc_size_mem = fd_bo_new(screen->dev, 0x1000,
			DRM_FREEDRENO_GEM_TYPE_KMEM);

	fd4_ctx->solid_vbuf = create_solid_vertexbuf(pctx);
	fd4_ctx->blit_texcoord_vbuf = create_blit_texcoord_vertexbuf(pctx);

	/* setup solid_vbuf_state: */
	fd4_ctx->solid_vbuf_state.vtx = pctx->create_vertex_elements_state(
			pctx, 1, (struct pipe_vertex_element[]){{
				.vertex_buffer_index = 0,
				.src_offset = 0,
				.src_format = PIPE_FORMAT_R32G32B32_FLOAT,
			}});
	fd4_ctx->solid_vbuf_state.vertexbuf.count = 1;
	fd4_ctx->solid_vbuf_state.vertexbuf.vb[0].stride = 12;
	fd4_ctx->solid_vbuf_state.vertexbuf.vb[0].buffer = fd4_ctx->solid_vbuf;

	/* setup blit_vbuf_state: */
	fd4_ctx->blit_vbuf_state.vtx = pctx->create_vertex_elements_state(
			pctx, 2, (struct pipe_vertex_element[]){{
				.vertex_buffer_index = 0,
				.src_offset = 0,
				.src_format = PIPE_FORMAT_R32G32_FLOAT,
			}, {
				.vertex_buffer_index = 1,
				.src_offset = 0,
				.src_format = PIPE_FORMAT_R32G32B32_FLOAT,
			}});
	fd4_ctx->blit_vbuf_state.vertexbuf.count = 2;
	fd4_ctx->blit_vbuf_state.vertexbuf.vb[0].stride = 8;
	fd4_ctx->blit_vbuf_state.vertexbuf.vb[0].buffer = fd4_ctx->blit_texcoord_vbuf;
	fd4_ctx->blit_vbuf_state.vertexbuf.vb[1].stride = 12;
	fd4_ctx->blit_vbuf_state.vertexbuf.vb[1].buffer = fd4_ctx->solid_vbuf;

	fd4_query_context_init(pctx);

	return pctx;
}
