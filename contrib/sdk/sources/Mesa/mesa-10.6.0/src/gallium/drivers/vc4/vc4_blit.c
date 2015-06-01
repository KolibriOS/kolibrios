/*
 * Copyright © 2015 Broadcom
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

#include "util/u_format.h"
#include "util/u_surface.h"
#include "util/u_blitter.h"
#include "vc4_context.h"

static void
vc4_tile_blit_color_rcl(struct vc4_context *vc4,
                        struct vc4_surface *dst_surf,
                        struct vc4_surface *src_surf)
{
        struct vc4_resource *src = vc4_resource(src_surf->base.texture);
        struct vc4_resource *dst = vc4_resource(dst_surf->base.texture);

        uint32_t min_x_tile = 0;
        uint32_t min_y_tile = 0;
        uint32_t max_x_tile = (dst_surf->base.width - 1) / 64;
        uint32_t max_y_tile = (dst_surf->base.height - 1) / 64;
        uint32_t xtiles = max_x_tile - min_x_tile + 1;
        uint32_t ytiles = max_y_tile - min_y_tile + 1;
        uint32_t reloc_size = 9;
        uint32_t config_size = 11 + reloc_size;
        uint32_t loadstore_size = 7 + reloc_size;
        uint32_t tilecoords_size = 3;
        cl_ensure_space(&vc4->rcl,
                        config_size +
                        xtiles * ytiles * (loadstore_size * 2 +
                                           tilecoords_size * 1));
        cl_ensure_space(&vc4->bo_handles, 2 * sizeof(uint32_t));
        cl_ensure_space(&vc4->bo_pointers, 2 * sizeof(struct vc4_bo *));

        cl_start_reloc(&vc4->rcl, 1);
        cl_u8(&vc4->rcl, VC4_PACKET_TILE_RENDERING_MODE_CONFIG);
        cl_reloc(vc4, &vc4->rcl, dst->bo, dst_surf->offset);
        cl_u16(&vc4->rcl, dst_surf->base.width);
        cl_u16(&vc4->rcl, dst_surf->base.height);
        cl_u16(&vc4->rcl, ((dst_surf->tiling <<
                            VC4_RENDER_CONFIG_MEMORY_FORMAT_SHIFT) |
                           (vc4_rt_format_is_565(dst_surf->base.format) ?
                            VC4_RENDER_CONFIG_FORMAT_BGR565 :
                            VC4_RENDER_CONFIG_FORMAT_RGBA8888)));

        uint32_t src_hindex = vc4_gem_hindex(vc4, src->bo);

        for (int y = min_y_tile; y <= max_y_tile; y++) {
                for (int x = min_x_tile; x <= max_x_tile; x++) {
                        bool end_of_frame = (x == max_x_tile &&
                                             y == max_y_tile);

                        cl_start_reloc(&vc4->rcl, 1);
                        cl_u8(&vc4->rcl, VC4_PACKET_LOAD_TILE_BUFFER_GENERAL);
                        cl_u8(&vc4->rcl,
                              VC4_LOADSTORE_TILE_BUFFER_COLOR |
                              (src_surf->tiling <<
                               VC4_LOADSTORE_TILE_BUFFER_FORMAT_SHIFT));
                        cl_u8(&vc4->rcl,
                              vc4_rt_format_is_565(src_surf->base.format) ?
                              VC4_LOADSTORE_TILE_BUFFER_BGR565 :
                              VC4_LOADSTORE_TILE_BUFFER_RGBA8888);
                        cl_reloc_hindex(&vc4->rcl, src_hindex,
                                        src_surf->offset);

                        cl_u8(&vc4->rcl, VC4_PACKET_TILE_COORDINATES);
                        cl_u8(&vc4->rcl, x);
                        cl_u8(&vc4->rcl, y);

                        if (end_of_frame) {
                                cl_u8(&vc4->rcl,
                                      VC4_PACKET_STORE_MS_TILE_BUFFER_AND_EOF);
                        } else {
                                cl_u8(&vc4->rcl,
                                      VC4_PACKET_STORE_MS_TILE_BUFFER);
                        }
                }
        }

        vc4->draw_min_x = 0;
        vc4->draw_min_y = 0;
        vc4->draw_max_x = dst_surf->base.width;
        vc4->draw_max_y = dst_surf->base.height;

        dst->writes++;
        vc4->needs_flush = true;
}

static struct vc4_surface *
vc4_get_blit_surface(struct pipe_context *pctx,
                     struct pipe_resource *prsc, unsigned level)
{
        struct pipe_surface tmpl;

        memset(&tmpl, 0, sizeof(tmpl));
        tmpl.format = prsc->format;
        tmpl.u.tex.level = level;
        tmpl.u.tex.first_layer = 0;
        tmpl.u.tex.last_layer = 0;

        return vc4_surface(pctx->create_surface(pctx, prsc, &tmpl));
}

static bool
vc4_tile_blit(struct pipe_context *pctx, const struct pipe_blit_info *info)
{
        struct vc4_context *vc4 = vc4_context(pctx);

        if (util_format_is_depth_or_stencil(info->dst.resource->format))
                return false;

        if ((info->mask & PIPE_MASK_RGBA) == 0)
                return false;

        if (info->dst.box.x != 0 || info->dst.box.y != 0 ||
            info->src.box.x != 0 || info->src.box.y != 0 ||
            info->dst.box.width != info->src.box.width ||
            info->dst.box.height != info->src.box.height) {
                return false;
        }

        if (info->dst.resource->format != info->src.resource->format)
                return false;

        struct vc4_surface *dst_surf =
                vc4_get_blit_surface(pctx, info->dst.resource, info->dst.level);
        struct vc4_surface *src_surf =
                vc4_get_blit_surface(pctx, info->src.resource, info->src.level);

        vc4_flush(pctx);
        vc4_tile_blit_color_rcl(vc4, dst_surf, src_surf);
        vc4_job_submit(vc4);

        pctx->surface_destroy(pctx, &dst_surf->base);
        pctx->surface_destroy(pctx, &src_surf->base);

        return true;
}

static bool
vc4_render_blit(struct pipe_context *ctx, struct pipe_blit_info *info)
{
        struct vc4_context *vc4 = vc4_context(ctx);

        if (!util_blitter_is_blit_supported(vc4->blitter, info)) {
                fprintf(stderr, "blit unsupported %s -> %s",
                    util_format_short_name(info->src.resource->format),
                    util_format_short_name(info->dst.resource->format));
                return false;
        }

        util_blitter_save_vertex_buffer_slot(vc4->blitter, vc4->vertexbuf.vb);
        util_blitter_save_vertex_elements(vc4->blitter, vc4->vtx);
        util_blitter_save_vertex_shader(vc4->blitter, vc4->prog.bind_vs);
        util_blitter_save_rasterizer(vc4->blitter, vc4->rasterizer);
        util_blitter_save_viewport(vc4->blitter, &vc4->viewport);
        util_blitter_save_scissor(vc4->blitter, &vc4->scissor);
        util_blitter_save_fragment_shader(vc4->blitter, vc4->prog.bind_fs);
        util_blitter_save_blend(vc4->blitter, vc4->blend);
        util_blitter_save_depth_stencil_alpha(vc4->blitter, vc4->zsa);
        util_blitter_save_stencil_ref(vc4->blitter, &vc4->stencil_ref);
        util_blitter_save_sample_mask(vc4->blitter, vc4->sample_mask);
        util_blitter_save_framebuffer(vc4->blitter, &vc4->framebuffer);
        util_blitter_save_fragment_sampler_states(vc4->blitter,
                        vc4->fragtex.num_samplers,
                        (void **)vc4->fragtex.samplers);
        util_blitter_save_fragment_sampler_views(vc4->blitter,
                        vc4->fragtex.num_textures, vc4->fragtex.textures);

        util_blitter_blit(vc4->blitter, info);

        return true;
}

/* Optimal hardware path for blitting pixels.
 * Scaling, format conversion, up- and downsampling (resolve) are allowed.
 */
void
vc4_blit(struct pipe_context *pctx, const struct pipe_blit_info *blit_info)
{
        struct pipe_blit_info info = *blit_info;

        if (info.src.resource->nr_samples > 1 &&
            info.dst.resource->nr_samples <= 1 &&
            !util_format_is_depth_or_stencil(info.src.resource->format) &&
            !util_format_is_pure_integer(info.src.resource->format)) {
                fprintf(stderr, "color resolve unimplemented");
                return;
        }

        if (vc4_tile_blit(pctx, blit_info))
                return;

        if (util_try_blit_via_copy_region(pctx, &info)) {
                return; /* done */
        }

        if (info.mask & PIPE_MASK_S) {
                fprintf(stderr, "cannot blit stencil, skipping");
                info.mask &= ~PIPE_MASK_S;
        }

        vc4_render_blit(pctx, &info);
}
