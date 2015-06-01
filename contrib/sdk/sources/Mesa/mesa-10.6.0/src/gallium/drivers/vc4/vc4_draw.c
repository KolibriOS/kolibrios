/*
 * Copyright (c) 2014 Scott Mansell
 * Copyright © 2014 Broadcom
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "util/u_prim.h"
#include "util/u_format.h"
#include "util/u_pack_color.h"
#include "indices/u_primconvert.h"

#include "vc4_context.h"
#include "vc4_resource.h"

static void
vc4_get_draw_cl_space(struct vc4_context *vc4)
{
        /* Binner gets our packet state -- vc4_emit.c contents,
         * and the primitive itself.
         */
        cl_ensure_space(&vc4->bcl, 256);

        /* Nothing for rcl -- that's covered by vc4_context.c */

        /* shader_rec gets up to 12 dwords of reloc handles plus a maximally
         * sized shader_rec (104 bytes base for 8 vattrs plus 32 bytes of
         * vattr stride).
         */
        cl_ensure_space(&vc4->shader_rec, 12 * sizeof(uint32_t) + 104 + 8 * 32);

        /* Uniforms are covered by vc4_write_uniforms(). */

        /* There could be up to 16 textures per stage, plus misc other
         * pointers.
         */
        cl_ensure_space(&vc4->bo_handles, (2 * 16 + 20) * sizeof(uint32_t));
        cl_ensure_space(&vc4->bo_pointers,
                        (2 * 16 + 20) * sizeof(struct vc4_bo *));
}

/**
 * Does the initial bining command list setup for drawing to a given FBO.
 */
static void
vc4_start_draw(struct vc4_context *vc4)
{
        if (vc4->needs_flush)
                return;

        vc4_get_draw_cl_space(vc4);

        uint32_t width = vc4->framebuffer.width;
        uint32_t height = vc4->framebuffer.height;
        uint32_t tilew = align(width, 64) / 64;
        uint32_t tileh = align(height, 64) / 64;

        /* Tile alloc memory setup: We use an initial alloc size of 32b.  The
         * hardware then aligns that to 256b (we use 4096, because all of our
         * BO allocations align to that anyway), then for some reason the
         * simulator wants an extra page available, even if you have overflow
         * memory set up.
         *
         * XXX: The binner only does 28-bit addressing math, so the tile alloc
         * and tile state should be in the same BO and that BO needs to not
         * cross a 256MB boundary, somehow.
         */
        uint32_t tile_alloc_size = 32 * tilew * tileh;
        tile_alloc_size = align(tile_alloc_size, 4096);
        tile_alloc_size += 4096;
        uint32_t tile_state_size = 48 * tilew * tileh;
        if (!vc4->tile_alloc || vc4->tile_alloc->size < tile_alloc_size) {
                vc4_bo_unreference(&vc4->tile_alloc);
                vc4->tile_alloc = vc4_bo_alloc(vc4->screen, tile_alloc_size,
                                               "tile_alloc");
        }
        if (!vc4->tile_state || vc4->tile_state->size < tile_state_size) {
                vc4_bo_unreference(&vc4->tile_state);
                vc4->tile_state = vc4_bo_alloc(vc4->screen, tile_state_size,
                                               "tile_state");
        }

        //   Tile state data is 48 bytes per tile, I think it can be thrown away
        //   as soon as binning is finished.
        cl_start_reloc(&vc4->bcl, 2);
        cl_u8(&vc4->bcl, VC4_PACKET_TILE_BINNING_MODE_CONFIG);
        cl_reloc(vc4, &vc4->bcl, vc4->tile_alloc, 0);
        cl_u32(&vc4->bcl, vc4->tile_alloc->size);
        cl_reloc(vc4, &vc4->bcl, vc4->tile_state, 0);
        cl_u8(&vc4->bcl, tilew);
        cl_u8(&vc4->bcl, tileh);
        cl_u8(&vc4->bcl,
              VC4_BIN_CONFIG_AUTO_INIT_TSDA |
              VC4_BIN_CONFIG_ALLOC_BLOCK_SIZE_32 |
              VC4_BIN_CONFIG_ALLOC_INIT_BLOCK_SIZE_32);

        /* START_TILE_BINNING resets the statechange counters in the hardware,
         * which are what is used when a primitive is binned to a tile to
         * figure out what new state packets need to be written to that tile's
         * command list.
         */
        cl_u8(&vc4->bcl, VC4_PACKET_START_TILE_BINNING);

        /* Reset the current compressed primitives format.  This gets modified
         * by VC4_PACKET_GL_INDEXED_PRIMITIVE and
         * VC4_PACKET_GL_ARRAY_PRIMITIVE, so it needs to be reset at the start
         * of every tile.
         */
        cl_u8(&vc4->bcl, VC4_PACKET_PRIMITIVE_LIST_FORMAT);
        cl_u8(&vc4->bcl, (VC4_PRIMITIVE_LIST_FORMAT_16_INDEX |
                          VC4_PRIMITIVE_LIST_FORMAT_TYPE_TRIANGLES));

        vc4->needs_flush = true;
        vc4->draw_call_queued = true;
}

static void
vc4_update_shadow_textures(struct pipe_context *pctx,
                           struct vc4_texture_stateobj *stage_tex)
{
        for (int i = 0; i < stage_tex->num_textures; i++) {
                struct pipe_sampler_view *view = stage_tex->textures[i];
                if (!view)
                        continue;
                struct vc4_resource *rsc = vc4_resource(view->texture);
                if (rsc->shadow_parent)
                        vc4_update_shadow_baselevel_texture(pctx, view);
        }
}

static void
vc4_draw_vbo(struct pipe_context *pctx, const struct pipe_draw_info *info)
{
        struct vc4_context *vc4 = vc4_context(pctx);

        if (info->mode >= PIPE_PRIM_QUADS) {
                util_primconvert_save_index_buffer(vc4->primconvert, &vc4->indexbuf);
                util_primconvert_save_rasterizer_state(vc4->primconvert, &vc4->rasterizer->base);
                util_primconvert_draw_vbo(vc4->primconvert, info);
                perf_debug("Fallback conversion for %d %s vertices\n",
                           info->count, u_prim_name(info->mode));
                return;
        }

        /* Before setting up the draw, do any fixup blits necessary. */
        vc4_update_shadow_textures(pctx, &vc4->verttex);
        vc4_update_shadow_textures(pctx, &vc4->fragtex);

        vc4_get_draw_cl_space(vc4);

        struct vc4_vertex_stateobj *vtx = vc4->vtx;
        struct vc4_vertexbuf_stateobj *vertexbuf = &vc4->vertexbuf;

        if (vc4->prim_mode != info->mode) {
                vc4->prim_mode = info->mode;
                vc4->dirty |= VC4_DIRTY_PRIM_MODE;
        }

        vc4_start_draw(vc4);
        vc4_update_compiled_shaders(vc4, info->mode);

        vc4_emit_state(pctx);
        vc4->dirty = 0;

        vc4_write_uniforms(vc4, vc4->prog.fs,
                           &vc4->constbuf[PIPE_SHADER_FRAGMENT],
                           &vc4->fragtex);
        vc4_write_uniforms(vc4, vc4->prog.vs,
                           &vc4->constbuf[PIPE_SHADER_VERTEX],
                           &vc4->verttex);
        vc4_write_uniforms(vc4, vc4->prog.cs,
                           &vc4->constbuf[PIPE_SHADER_VERTEX],
                           &vc4->verttex);

        /* The simulator throws a fit if VS or CS don't read an attribute, so
         * we emit a dummy read.
         */
        uint32_t num_elements_emit = MAX2(vtx->num_elements, 1);
        /* Emit the shader record. */
        cl_start_shader_reloc(&vc4->shader_rec, 3 + num_elements_emit);
        cl_u16(&vc4->shader_rec,
               VC4_SHADER_FLAG_ENABLE_CLIPPING |
               ((info->mode == PIPE_PRIM_POINTS &&
                 vc4->rasterizer->base.point_size_per_vertex) ?
                VC4_SHADER_FLAG_VS_POINT_SIZE : 0));
        cl_u8(&vc4->shader_rec, 0); /* fs num uniforms (unused) */
        cl_u8(&vc4->shader_rec, vc4->prog.fs->num_inputs);
        cl_reloc(vc4, &vc4->shader_rec, vc4->prog.fs->bo, 0);
        cl_u32(&vc4->shader_rec, 0); /* UBO offset written by kernel */

        cl_u16(&vc4->shader_rec, 0); /* vs num uniforms */
        cl_u8(&vc4->shader_rec, vc4->prog.vs->vattrs_live);
        cl_u8(&vc4->shader_rec, vc4->prog.vs->vattr_offsets[8]);
        cl_reloc(vc4, &vc4->shader_rec, vc4->prog.vs->bo, 0);
        cl_u32(&vc4->shader_rec, 0); /* UBO offset written by kernel */

        cl_u16(&vc4->shader_rec, 0); /* cs num uniforms */
        cl_u8(&vc4->shader_rec, vc4->prog.cs->vattrs_live);
        cl_u8(&vc4->shader_rec, vc4->prog.cs->vattr_offsets[8]);
        cl_reloc(vc4, &vc4->shader_rec, vc4->prog.cs->bo, 0);
        cl_u32(&vc4->shader_rec, 0); /* UBO offset written by kernel */

        uint32_t max_index = 0xffff;
        uint32_t vpm_offset = 0;
        for (int i = 0; i < vtx->num_elements; i++) {
                struct pipe_vertex_element *elem = &vtx->pipe[i];
                struct pipe_vertex_buffer *vb =
                        &vertexbuf->vb[elem->vertex_buffer_index];
                struct vc4_resource *rsc = vc4_resource(vb->buffer);
                uint32_t offset = vb->buffer_offset + elem->src_offset;
                uint32_t vb_size = rsc->bo->size - offset;
                uint32_t elem_size =
                        util_format_get_blocksize(elem->src_format);

                cl_reloc(vc4, &vc4->shader_rec, rsc->bo, offset);
                cl_u8(&vc4->shader_rec, elem_size - 1);
                cl_u8(&vc4->shader_rec, vb->stride);
                cl_u8(&vc4->shader_rec, vc4->prog.vs->vattr_offsets[i]);
                cl_u8(&vc4->shader_rec, vc4->prog.cs->vattr_offsets[i]);

                vpm_offset += align(elem_size, 4);

                if (vb->stride > 0) {
                        max_index = MIN2(max_index,
                                         (vb_size - elem_size) / vb->stride);
                }
        }

        if (vtx->num_elements == 0) {
                assert(num_elements_emit == 1);
                struct vc4_bo *bo = vc4_bo_alloc(vc4->screen, 4096, "scratch VBO");
                cl_reloc(vc4, &vc4->shader_rec, bo, 0);
                cl_u8(&vc4->shader_rec, 16 - 1); /* element size */
                cl_u8(&vc4->shader_rec, 0); /* stride */
                cl_u8(&vc4->shader_rec, 0); /* VS VPM offset */
                cl_u8(&vc4->shader_rec, 0); /* CS VPM offset */
                vc4_bo_unreference(&bo);
        }

        /* the actual draw call. */
        cl_u8(&vc4->bcl, VC4_PACKET_GL_SHADER_STATE);
        assert(vtx->num_elements <= 8);
        /* Note that number of attributes == 0 in the packet means 8
         * attributes.  This field also contains the offset into shader_rec.
         */
        cl_u32(&vc4->bcl, num_elements_emit & 0x7);

        /* Note that the primitive type fields match with OpenGL/gallium
         * definitions, up to but not including QUADS.
         */
        if (info->indexed) {
                struct vc4_resource *rsc = vc4_resource(vc4->indexbuf.buffer);
                uint32_t offset = vc4->indexbuf.offset;
                uint32_t index_size = vc4->indexbuf.index_size;
                if (rsc->shadow_parent) {
                        vc4_update_shadow_index_buffer(pctx, &vc4->indexbuf);
                        offset = 0;
                }

                cl_start_reloc(&vc4->bcl, 1);
                cl_u8(&vc4->bcl, VC4_PACKET_GL_INDEXED_PRIMITIVE);
                cl_u8(&vc4->bcl,
                      info->mode |
                      (index_size == 2 ?
                       VC4_INDEX_BUFFER_U16:
                       VC4_INDEX_BUFFER_U8));
                cl_u32(&vc4->bcl, info->count);
                cl_reloc(vc4, &vc4->bcl, rsc->bo, offset);
                cl_u32(&vc4->bcl, max_index);
        } else {
                cl_u8(&vc4->bcl, VC4_PACKET_GL_ARRAY_PRIMITIVE);
                cl_u8(&vc4->bcl, info->mode);
                cl_u32(&vc4->bcl, info->count);
                cl_u32(&vc4->bcl, info->start);
        }

        if (vc4->zsa && vc4->zsa->base.depth.enabled) {
                vc4->resolve |= PIPE_CLEAR_DEPTH;
        }
        if (vc4->zsa && vc4->zsa->base.stencil[0].enabled)
                vc4->resolve |= PIPE_CLEAR_STENCIL;
        vc4->resolve |= PIPE_CLEAR_COLOR0;

        vc4->shader_rec_count++;

        if (vc4_debug & VC4_DEBUG_ALWAYS_FLUSH)
                vc4_flush(pctx);
}

static uint32_t
pack_rgba(enum pipe_format format, const float *rgba)
{
        union util_color uc;
        util_pack_color(rgba, format, &uc);
        if (util_format_get_blocksize(format) == 2)
                return uc.us;
        else
                return uc.ui[0];
}

static void
vc4_clear(struct pipe_context *pctx, unsigned buffers,
          const union pipe_color_union *color, double depth, unsigned stencil)
{
        struct vc4_context *vc4 = vc4_context(pctx);

        /* We can't flag new buffers for clearing once we've queued draws.  We
         * could avoid this by using the 3d engine to clear.
         */
        if (vc4->draw_call_queued) {
                perf_debug("Flushing rendering to process new clear.");
                vc4_flush(pctx);
        }

        if (buffers & PIPE_CLEAR_COLOR0) {
                vc4->clear_color[0] = vc4->clear_color[1] =
                        pack_rgba(vc4->framebuffer.cbufs[0]->format,
                                  color->f);
        }

        if (buffers & PIPE_CLEAR_DEPTH) {
                /* Though the depth buffer is stored with Z in the high 24,
                 * for this field we just need to store it in the low 24.
                 */
                vc4->clear_depth = util_pack_z(PIPE_FORMAT_Z24X8_UNORM, depth);
        }

        if (buffers & PIPE_CLEAR_STENCIL)
                vc4->clear_stencil = stencil;

        vc4->draw_min_x = 0;
        vc4->draw_min_y = 0;
        vc4->draw_max_x = vc4->framebuffer.width;
        vc4->draw_max_y = vc4->framebuffer.height;
        vc4->cleared |= buffers;
        vc4->resolve |= buffers;

        vc4_start_draw(vc4);
}

static void
vc4_clear_render_target(struct pipe_context *pctx, struct pipe_surface *ps,
                        const union pipe_color_union *color,
                        unsigned x, unsigned y, unsigned w, unsigned h)
{
        fprintf(stderr, "unimpl: clear RT\n");
}

static void
vc4_clear_depth_stencil(struct pipe_context *pctx, struct pipe_surface *ps,
                        unsigned buffers, double depth, unsigned stencil,
                        unsigned x, unsigned y, unsigned w, unsigned h)
{
        fprintf(stderr, "unimpl: clear DS\n");
}

void
vc4_draw_init(struct pipe_context *pctx)
{
        pctx->draw_vbo = vc4_draw_vbo;
        pctx->clear = vc4_clear;
        pctx->clear_render_target = vc4_clear_render_target;
        pctx->clear_depth_stencil = vc4_clear_depth_stencil;
}
