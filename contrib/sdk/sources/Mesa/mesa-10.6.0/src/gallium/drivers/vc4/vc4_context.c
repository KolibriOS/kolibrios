/*
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

#include <xf86drm.h>
#include <err.h>

#include "pipe/p_defines.h"
#include "util/ralloc.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_blitter.h"
#include "indices/u_primconvert.h"
#include "pipe/p_screen.h"

#include "vc4_screen.h"
#include "vc4_context.h"
#include "vc4_resource.h"

/**
 * Emits a no-op STORE_TILE_BUFFER_GENERAL.
 *
 * If we emit a PACKET_TILE_COORDINATES, it must be followed by a store of
 * some sort before another load is triggered.
 */
static void
vc4_store_before_load(struct vc4_context *vc4, bool *coords_emitted)
{
        if (!*coords_emitted)
                return;

        cl_u8(&vc4->rcl, VC4_PACKET_STORE_TILE_BUFFER_GENERAL);
        cl_u8(&vc4->rcl, VC4_LOADSTORE_TILE_BUFFER_NONE);
        cl_u8(&vc4->rcl, (VC4_STORE_TILE_BUFFER_DISABLE_COLOR_CLEAR |
                          VC4_STORE_TILE_BUFFER_DISABLE_ZS_CLEAR |
                          VC4_STORE_TILE_BUFFER_DISABLE_VG_MASK_CLEAR));
        cl_u32(&vc4->rcl, 0); /* no address, since we're in None mode */

        *coords_emitted = false;
}

/**
 * Emits a PACKET_TILE_COORDINATES if one isn't already pending.
 *
 * The tile coordinates packet triggers a pending load if there is one, are
 * used for clipping during rendering, and determine where loads/stores happen
 * relative to their base address.
 */
static void
vc4_tile_coordinates(struct vc4_context *vc4, uint32_t x, uint32_t y,
                       bool *coords_emitted)
{
        if (*coords_emitted)
                return;

        cl_u8(&vc4->rcl, VC4_PACKET_TILE_COORDINATES);
        cl_u8(&vc4->rcl, x);
        cl_u8(&vc4->rcl, y);

        *coords_emitted = true;
}

static void
vc4_setup_rcl(struct vc4_context *vc4)
{
        struct vc4_surface *csurf = vc4_surface(vc4->framebuffer.cbufs[0]);
        struct vc4_resource *ctex = csurf ? vc4_resource(csurf->base.texture) : NULL;
        struct vc4_surface *zsurf = vc4_surface(vc4->framebuffer.zsbuf);
        struct vc4_resource *ztex = zsurf ? vc4_resource(zsurf->base.texture) : NULL;

        if (!csurf)
                vc4->resolve &= ~PIPE_CLEAR_COLOR0;
        if (!zsurf)
                vc4->resolve &= ~(PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL);
        uint32_t resolve_uncleared = vc4->resolve & ~vc4->cleared;
        uint32_t width = vc4->framebuffer.width;
        uint32_t height = vc4->framebuffer.height;
        uint32_t stride_in_tiles = align(width, 64) / 64;

        assert(vc4->draw_min_x != ~0 && vc4->draw_min_y != ~0);
        uint32_t min_x_tile = vc4->draw_min_x / 64;
        uint32_t min_y_tile = vc4->draw_min_y / 64;
        uint32_t max_x_tile = (vc4->draw_max_x - 1) / 64;
        uint32_t max_y_tile = (vc4->draw_max_y - 1) / 64;
        uint32_t xtiles = max_x_tile - min_x_tile + 1;
        uint32_t ytiles = max_y_tile - min_y_tile + 1;

#if 0
        fprintf(stderr, "RCL: resolve 0x%x clear 0x%x resolve uncleared 0x%x\n",
                vc4->resolve,
                vc4->cleared,
                resolve_uncleared);
#endif

        uint32_t reloc_size = 9;
        uint32_t clear_size = 14;
        uint32_t config_size = 11 + reloc_size;
        uint32_t loadstore_size = 7 + reloc_size;
        uint32_t tilecoords_size = 3;
        uint32_t branch_size = 5 + reloc_size;
        uint32_t color_store_size = 1;
        uint32_t semaphore_size = 1;
        cl_ensure_space(&vc4->rcl,
                        clear_size +
                        config_size +
                        loadstore_size +
                        semaphore_size +
                        xtiles * ytiles * (loadstore_size * 4 +
                                           tilecoords_size * 3 +
                                           branch_size +
                                           color_store_size));

        if (vc4->cleared) {
                cl_u8(&vc4->rcl, VC4_PACKET_CLEAR_COLORS);
                cl_u32(&vc4->rcl, vc4->clear_color[0]);
                cl_u32(&vc4->rcl, vc4->clear_color[1]);
                cl_u32(&vc4->rcl, vc4->clear_depth);
                cl_u8(&vc4->rcl, vc4->clear_stencil);
        }

        /* The rendering mode config determines the pointer that's used for
         * VC4_PACKET_STORE_MS_TILE_BUFFER address computations.  The kernel
         * could handle a no-relocation rendering mode config and deny those
         * packets, but instead we just tell the kernel we're doing our color
         * rendering to the Z buffer, and just don't emit any of those
         * packets.
         */
        struct vc4_surface *render_surf = csurf ? csurf : zsurf;
        struct vc4_resource *render_tex = vc4_resource(render_surf->base.texture);
        cl_start_reloc(&vc4->rcl, 1);
        cl_u8(&vc4->rcl, VC4_PACKET_TILE_RENDERING_MODE_CONFIG);
        cl_reloc(vc4, &vc4->rcl, render_tex->bo, render_surf->offset);
        cl_u16(&vc4->rcl, width);
        cl_u16(&vc4->rcl, height);
        cl_u16(&vc4->rcl, ((render_surf->tiling <<
                            VC4_RENDER_CONFIG_MEMORY_FORMAT_SHIFT) |
                           (vc4_rt_format_is_565(render_surf->base.format) ?
                            VC4_RENDER_CONFIG_FORMAT_BGR565 :
                            VC4_RENDER_CONFIG_FORMAT_RGBA8888)));

        /* The tile buffer normally gets cleared when the previous tile is
         * stored.  If the clear values changed between frames, then the tile
         * buffer has stale clear values in it, so we have to do a store in
         * None mode (no writes) so that we trigger the tile buffer clear.
         *
         * Excess clearing is only a performance cost, since per-tile contents
         * will be loaded/stored in the loop below.
         */
        if (vc4->cleared & (PIPE_CLEAR_COLOR0 |
                            PIPE_CLEAR_DEPTH |
                            PIPE_CLEAR_STENCIL)) {
                cl_u8(&vc4->rcl, VC4_PACKET_TILE_COORDINATES);
                cl_u8(&vc4->rcl, 0);
                cl_u8(&vc4->rcl, 0);

                cl_u8(&vc4->rcl, VC4_PACKET_STORE_TILE_BUFFER_GENERAL);
                cl_u16(&vc4->rcl, VC4_LOADSTORE_TILE_BUFFER_NONE);
                cl_u32(&vc4->rcl, 0); /* no address, since we're in None mode */
        }

        uint32_t color_hindex = ctex ? vc4_gem_hindex(vc4, ctex->bo) : 0;
        uint32_t depth_hindex = ztex ? vc4_gem_hindex(vc4, ztex->bo) : 0;
        uint32_t tile_alloc_hindex = vc4_gem_hindex(vc4, vc4->tile_alloc);

        for (int y = min_y_tile; y <= max_y_tile; y++) {
                for (int x = min_x_tile; x <= max_x_tile; x++) {
                        bool end_of_frame = (x == max_x_tile &&
                                             y == max_y_tile);
                        bool coords_emitted = false;

                        /* Note that the load doesn't actually occur until the
                         * tile coords packet is processed, and only one load
                         * may be outstanding at a time.
                         */
                        if (resolve_uncleared & PIPE_CLEAR_COLOR) {
                                vc4_store_before_load(vc4, &coords_emitted);

                                cl_start_reloc(&vc4->rcl, 1);
                                cl_u8(&vc4->rcl, VC4_PACKET_LOAD_TILE_BUFFER_GENERAL);
                                cl_u8(&vc4->rcl,
                                      VC4_LOADSTORE_TILE_BUFFER_COLOR |
                                      (csurf->tiling <<
                                       VC4_LOADSTORE_TILE_BUFFER_FORMAT_SHIFT));
                                cl_u8(&vc4->rcl,
                                      vc4_rt_format_is_565(csurf->base.format) ?
                                      VC4_LOADSTORE_TILE_BUFFER_BGR565 :
                                      VC4_LOADSTORE_TILE_BUFFER_RGBA8888);
                                cl_reloc_hindex(&vc4->rcl, color_hindex,
                                                csurf->offset);

                                vc4_tile_coordinates(vc4, x, y, &coords_emitted);
                        }

                        if (resolve_uncleared & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)) {
                                vc4_store_before_load(vc4, &coords_emitted);

                                cl_start_reloc(&vc4->rcl, 1);
                                cl_u8(&vc4->rcl, VC4_PACKET_LOAD_TILE_BUFFER_GENERAL);
                                cl_u8(&vc4->rcl,
                                      VC4_LOADSTORE_TILE_BUFFER_ZS |
                                      (zsurf->tiling <<
                                       VC4_LOADSTORE_TILE_BUFFER_FORMAT_SHIFT));
                                cl_u8(&vc4->rcl, 0);
                                cl_reloc_hindex(&vc4->rcl, depth_hindex,
                                                zsurf->offset);

                                vc4_tile_coordinates(vc4, x, y, &coords_emitted);
                        }

                        /* Clipping depends on tile coordinates having been
                         * emitted, so make sure it's happened even if
                         * everything was cleared to start.
                         */
                        vc4_tile_coordinates(vc4, x, y, &coords_emitted);

                        /* Wait for the binner before jumping to the first
                         * tile's lists.
                         */
                        if (x == min_x_tile && y == min_y_tile)
                                cl_u8(&vc4->rcl, VC4_PACKET_WAIT_ON_SEMAPHORE);

                        cl_start_reloc(&vc4->rcl, 1);
                        cl_u8(&vc4->rcl, VC4_PACKET_BRANCH_TO_SUB_LIST);
                        cl_reloc_hindex(&vc4->rcl, tile_alloc_hindex,
                                        (y * stride_in_tiles + x) * 32);

                        if (vc4->resolve & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)) {
                                vc4_tile_coordinates(vc4, x, y, &coords_emitted);

                                cl_start_reloc(&vc4->rcl, 1);
                                cl_u8(&vc4->rcl, VC4_PACKET_STORE_TILE_BUFFER_GENERAL);
                                cl_u8(&vc4->rcl,
                                      VC4_LOADSTORE_TILE_BUFFER_ZS |
                                      (zsurf->tiling <<
                                       VC4_LOADSTORE_TILE_BUFFER_FORMAT_SHIFT));
                                cl_u8(&vc4->rcl,
                                      VC4_STORE_TILE_BUFFER_DISABLE_COLOR_CLEAR);
                                cl_reloc_hindex(&vc4->rcl, depth_hindex,
                                                zsurf->offset |
                                                ((end_of_frame &&
                                                  !(vc4->resolve & PIPE_CLEAR_COLOR0)) ?
                                                 VC4_LOADSTORE_TILE_BUFFER_EOF : 0));

                                coords_emitted = false;
                        }

                        if (vc4->resolve & PIPE_CLEAR_COLOR0) {
                                vc4_tile_coordinates(vc4, x, y, &coords_emitted);
                                if (end_of_frame) {
                                        cl_u8(&vc4->rcl,
                                              VC4_PACKET_STORE_MS_TILE_BUFFER_AND_EOF);
                                } else {
                                        cl_u8(&vc4->rcl,
                                              VC4_PACKET_STORE_MS_TILE_BUFFER);
                                }

                                coords_emitted = false;
                        }

                        /* One of the bits needs to have been set that would
                         * have triggered an EOF.
                         */
                        assert(vc4->resolve & (PIPE_CLEAR_COLOR0 |
                                               PIPE_CLEAR_DEPTH |
                                               PIPE_CLEAR_STENCIL));
                        /* Any coords emitted must also have been consumed by
                         * a store.
                         */
                        assert(!coords_emitted);
                }
        }

        if (vc4->resolve & PIPE_CLEAR_COLOR0)
                ctex->writes++;

        if (vc4->resolve & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL))
                ztex->writes++;
}

void
vc4_flush(struct pipe_context *pctx)
{
        struct vc4_context *vc4 = vc4_context(pctx);

        if (!vc4->needs_flush)
                return;

        /* The RCL setup would choke if the draw bounds cause no drawing, so
         * just drop the drawing if that's the case.
         */
        if (vc4->draw_max_x <= vc4->draw_min_x ||
            vc4->draw_max_y <= vc4->draw_min_y) {
                vc4_job_reset(vc4);
                return;
        }

        /* Increment the semaphore indicating that binning is done and
         * unblocking the render thread.  Note that this doesn't act until the
         * FLUSH completes.
         */
        cl_ensure_space(&vc4->bcl, 8);
        cl_u8(&vc4->bcl, VC4_PACKET_INCREMENT_SEMAPHORE);
        /* The FLUSH caps all of our bin lists with a VC4_PACKET_RETURN. */
        cl_u8(&vc4->bcl, VC4_PACKET_FLUSH);

        vc4_setup_rcl(vc4);

        vc4_job_submit(vc4);
}

static void
vc4_pipe_flush(struct pipe_context *pctx, struct pipe_fence_handle **fence,
               unsigned flags)
{
        struct vc4_context *vc4 = vc4_context(pctx);

        vc4_flush(pctx);

        if (fence) {
                struct vc4_fence *f = vc4_fence_create(vc4->screen,
                                                       vc4->last_emit_seqno);
                *fence = (struct pipe_fence_handle *)f;
        }
}

/**
 * Flushes the current command lists if they reference the given BO.
 *
 * This helps avoid flushing the command buffers when unnecessary.
 */
bool
vc4_cl_references_bo(struct pipe_context *pctx, struct vc4_bo *bo)
{
        struct vc4_context *vc4 = vc4_context(pctx);

        if (!vc4->needs_flush)
                return false;

        /* Walk all the referenced BOs in the drawing command list to see if
         * they match.
         */
        struct vc4_bo **referenced_bos = vc4->bo_pointers.base;
        for (int i = 0; i < (vc4->bo_handles.next -
                             vc4->bo_handles.base) / 4; i++) {
                if (referenced_bos[i] == bo) {
                        return true;
                }
        }

        /* Also check for the Z/color buffers, since the references to those
         * are only added immediately before submit.
         */
        struct vc4_surface *csurf = vc4_surface(vc4->framebuffer.cbufs[0]);
        if (csurf) {
                struct vc4_resource *ctex = vc4_resource(csurf->base.texture);
                if (ctex->bo == bo) {
                        return true;
                }
        }

        struct vc4_surface *zsurf = vc4_surface(vc4->framebuffer.zsbuf);
        if (zsurf) {
                struct vc4_resource *ztex =
                        vc4_resource(zsurf->base.texture);
                if (ztex->bo == bo) {
                        return true;
                }
        }

        return false;
}

static void
vc4_invalidate_resource(struct pipe_context *pctx, struct pipe_resource *prsc)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct pipe_surface *zsurf = vc4->framebuffer.zsbuf;

        if (zsurf && zsurf->texture == prsc)
                vc4->resolve &= ~(PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL);
}

static void
vc4_context_destroy(struct pipe_context *pctx)
{
        struct vc4_context *vc4 = vc4_context(pctx);

        if (vc4->blitter)
                util_blitter_destroy(vc4->blitter);

        if (vc4->primconvert)
                util_primconvert_destroy(vc4->primconvert);

        util_slab_destroy(&vc4->transfer_pool);

        pipe_surface_reference(&vc4->framebuffer.cbufs[0], NULL);
        pipe_surface_reference(&vc4->framebuffer.zsbuf, NULL);
        vc4_bo_unreference(&vc4->tile_alloc);
        vc4_bo_unreference(&vc4->tile_state);

        vc4_program_fini(pctx);

        ralloc_free(vc4);
}

struct pipe_context *
vc4_context_create(struct pipe_screen *pscreen, void *priv)
{
        struct vc4_screen *screen = vc4_screen(pscreen);
        struct vc4_context *vc4;

        /* Prevent dumping of the shaders built during context setup. */
        uint32_t saved_shaderdb_flag = vc4_debug & VC4_DEBUG_SHADERDB;
        vc4_debug &= ~VC4_DEBUG_SHADERDB;

        vc4 = rzalloc(NULL, struct vc4_context);
        if (vc4 == NULL)
                return NULL;
        struct pipe_context *pctx = &vc4->base;

        vc4->screen = screen;

        pctx->screen = pscreen;
        pctx->priv = priv;
        pctx->destroy = vc4_context_destroy;
        pctx->flush = vc4_pipe_flush;
        pctx->invalidate_resource = vc4_invalidate_resource;

        vc4_draw_init(pctx);
        vc4_state_init(pctx);
        vc4_program_init(pctx);
        vc4_query_init(pctx);
        vc4_resource_context_init(pctx);

        vc4_job_init(vc4);

        vc4->fd = screen->fd;

        util_slab_create(&vc4->transfer_pool, sizeof(struct vc4_transfer),
                         16, UTIL_SLAB_SINGLETHREADED);
        vc4->blitter = util_blitter_create(pctx);
        if (!vc4->blitter)
                goto fail;

        vc4->primconvert = util_primconvert_create(pctx,
                                                   (1 << PIPE_PRIM_QUADS) - 1);
        if (!vc4->primconvert)
                goto fail;

        vc4_debug |= saved_shaderdb_flag;

        return &vc4->base;

fail:
        pctx->destroy(pctx);
        return NULL;
}
