/*
 * Copyright © 2014 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS, AUTHORS
 * AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * Authors:
 *   Marek Olšák <maraeo@gmail.com>
 */

#include "radeon_drm_winsys.h"

#include <radeon_surface.h>

static void surf_level_winsys_to_drm(struct radeon_surface_level *level_drm,
                                     const struct radeon_surf_level *level_ws)
{
    level_drm->offset = level_ws->offset;
    level_drm->slice_size = level_ws->slice_size;
    level_drm->npix_x = level_ws->npix_x;
    level_drm->npix_y = level_ws->npix_y;
    level_drm->npix_z = level_ws->npix_z;
    level_drm->nblk_x = level_ws->nblk_x;
    level_drm->nblk_y = level_ws->nblk_y;
    level_drm->nblk_z = level_ws->nblk_z;
    level_drm->pitch_bytes = level_ws->pitch_bytes;
    level_drm->mode = level_ws->mode;
}

static void surf_level_drm_to_winsys(struct radeon_surf_level *level_ws,
                                     const struct radeon_surface_level *level_drm)
{
    level_ws->offset = level_drm->offset;
    level_ws->slice_size = level_drm->slice_size;
    level_ws->npix_x = level_drm->npix_x;
    level_ws->npix_y = level_drm->npix_y;
    level_ws->npix_z = level_drm->npix_z;
    level_ws->nblk_x = level_drm->nblk_x;
    level_ws->nblk_y = level_drm->nblk_y;
    level_ws->nblk_z = level_drm->nblk_z;
    level_ws->pitch_bytes = level_drm->pitch_bytes;
    level_ws->mode = level_drm->mode;
}

static void surf_winsys_to_drm(struct radeon_surface *surf_drm,
                               const struct radeon_surf *surf_ws)
{
    int i;

    memset(surf_drm, 0, sizeof(*surf_drm));

    surf_drm->npix_x = surf_ws->npix_x;
    surf_drm->npix_y = surf_ws->npix_y;
    surf_drm->npix_z = surf_ws->npix_z;
    surf_drm->blk_w = surf_ws->blk_w;
    surf_drm->blk_h = surf_ws->blk_h;
    surf_drm->blk_d = surf_ws->blk_d;
    surf_drm->array_size = surf_ws->array_size;
    surf_drm->last_level = surf_ws->last_level;
    surf_drm->bpe = surf_ws->bpe;
    surf_drm->nsamples = surf_ws->nsamples;
    surf_drm->flags = surf_ws->flags;

    surf_drm->bo_size = surf_ws->bo_size;
    surf_drm->bo_alignment = surf_ws->bo_alignment;

    surf_drm->bankw = surf_ws->bankw;
    surf_drm->bankh = surf_ws->bankh;
    surf_drm->mtilea = surf_ws->mtilea;
    surf_drm->tile_split = surf_ws->tile_split;
    surf_drm->stencil_tile_split = surf_ws->stencil_tile_split;
    surf_drm->stencil_offset = surf_ws->stencil_offset;

    for (i = 0; i < RADEON_SURF_MAX_LEVEL; i++) {
        surf_level_winsys_to_drm(&surf_drm->level[i], &surf_ws->level[i]);
        surf_level_winsys_to_drm(&surf_drm->stencil_level[i],
                                 &surf_ws->stencil_level[i]);

        surf_drm->tiling_index[i] = surf_ws->tiling_index[i];
        surf_drm->stencil_tiling_index[i] = surf_ws->stencil_tiling_index[i];
    }
}

static void surf_drm_to_winsys(struct radeon_surf *surf_ws,
                               const struct radeon_surface *surf_drm)
{
    int i;

    memset(surf_ws, 0, sizeof(*surf_ws));

    surf_ws->npix_x = surf_drm->npix_x;
    surf_ws->npix_y = surf_drm->npix_y;
    surf_ws->npix_z = surf_drm->npix_z;
    surf_ws->blk_w = surf_drm->blk_w;
    surf_ws->blk_h = surf_drm->blk_h;
    surf_ws->blk_d = surf_drm->blk_d;
    surf_ws->array_size = surf_drm->array_size;
    surf_ws->last_level = surf_drm->last_level;
    surf_ws->bpe = surf_drm->bpe;
    surf_ws->nsamples = surf_drm->nsamples;
    surf_ws->flags = surf_drm->flags;

    surf_ws->bo_size = surf_drm->bo_size;
    surf_ws->bo_alignment = surf_drm->bo_alignment;

    surf_ws->bankw = surf_drm->bankw;
    surf_ws->bankh = surf_drm->bankh;
    surf_ws->mtilea = surf_drm->mtilea;
    surf_ws->tile_split = surf_drm->tile_split;
    surf_ws->stencil_tile_split = surf_drm->stencil_tile_split;
    surf_ws->stencil_offset = surf_drm->stencil_offset;

    for (i = 0; i < RADEON_SURF_MAX_LEVEL; i++) {
        surf_level_drm_to_winsys(&surf_ws->level[i], &surf_drm->level[i]);
        surf_level_drm_to_winsys(&surf_ws->stencil_level[i],
                                 &surf_drm->stencil_level[i]);

        surf_ws->tiling_index[i] = surf_drm->tiling_index[i];
        surf_ws->stencil_tiling_index[i] = surf_drm->stencil_tiling_index[i];
    }
}

static int radeon_winsys_surface_init(struct radeon_winsys *rws,
                                      struct radeon_surf *surf_ws)
{
    struct radeon_drm_winsys *ws = (struct radeon_drm_winsys*)rws;
    struct radeon_surface surf_drm;
    int r;

    surf_winsys_to_drm(&surf_drm, surf_ws);

    r = radeon_surface_init(ws->surf_man, &surf_drm);
    if (r)
        return r;

    surf_drm_to_winsys(surf_ws, &surf_drm);
    return 0;
}

static int radeon_winsys_surface_best(struct radeon_winsys *rws,
                                      struct radeon_surf *surf_ws)
{
    struct radeon_drm_winsys *ws = (struct radeon_drm_winsys*)rws;
    struct radeon_surface surf_drm;
    int r;

    surf_winsys_to_drm(&surf_drm, surf_ws);

    r = radeon_surface_best(ws->surf_man, &surf_drm);
    if (r)
        return r;

    surf_drm_to_winsys(surf_ws, &surf_drm);
    return 0;
}

void radeon_surface_init_functions(struct radeon_drm_winsys *ws)
{
    ws->base.surface_init = radeon_winsys_surface_init;
    ws->base.surface_best = radeon_winsys_surface_best;
}
