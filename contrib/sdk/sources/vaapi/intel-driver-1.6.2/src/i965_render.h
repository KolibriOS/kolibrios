/*
 * Copyright © 2006 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Xiang Haihao <haihao.xiang@intel.com>
 *
 */

#ifndef _I965_RENDER_H_
#define _I965_RENDER_H_

#define MAX_SAMPLERS            16
#define MAX_RENDER_SURFACES     (MAX_SAMPLERS + 1)

#define NUM_RENDER_KERNEL       3

#define VA_SRC_COLOR_MASK       0x000000f0

#include "i965_post_processing.h"

struct i965_kernel;

struct i965_render_state
{
    struct {
        dri_bo *vertex_buffer;
    } vb;

    struct {
        dri_bo *state;
    } vs;
    
    struct {
        dri_bo *state;
    } sf;

    struct {
        int sampler_count;
        dri_bo *sampler;
        dri_bo *state;
        dri_bo *surface_state_binding_table_bo;
    } wm;

    struct {
        dri_bo *state;
        dri_bo *viewport;
        dri_bo *blend;
        dri_bo *depth_stencil;
    } cc;

    struct {
        dri_bo *bo;
    } curbe;

    struct intel_region *draw_region;

    int pp_flag; /* 0: disable, 1: enable */

    struct i965_kernel render_kernels[3];
    
    struct {
        dri_bo *bo;
        int bo_size;
        unsigned int end_offset;
    } instruction_state;

    struct {
        dri_bo *bo;
    } indirect_state;

    struct {
        dri_bo *bo;
        int bo_size;
        unsigned int end_offset;
    } dynamic_state;

    unsigned int curbe_offset;
    int curbe_size;

    unsigned int sampler_offset;
    int sampler_size;

    unsigned int cc_viewport_offset;
    int cc_viewport_size;

    unsigned int cc_state_offset;
    int cc_state_size;

    unsigned int blend_state_offset;
    int blend_state_size;
  
    unsigned int sf_clip_offset;
    int sf_clip_size;

    unsigned int scissor_offset;
    int scissor_size;

    void (*render_put_surface)(VADriverContextP ctx, struct object_surface *,
                               const VARectangle *src_rec,
                               const VARectangle *dst_rect,
                               unsigned int flags);
    void (*render_put_subpicture)(VADriverContextP ctx, struct object_surface *,
                               const VARectangle *src_rec,
                               const VARectangle *dst_rect);
    void (*render_terminate)(VADriverContextP ctx);
};

bool i965_render_init(VADriverContextP ctx);
void i965_render_terminate(VADriverContextP ctx);

void
intel_render_put_surface(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags
);

void
intel_render_put_subpicture(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
);

struct gen7_surface_state;

void
gen7_render_set_surface_scs(struct gen7_surface_state *ss);

struct gen8_surface_state;
void
gen8_render_set_surface_scs(struct gen8_surface_state *ss);

extern bool gen8_render_init(VADriverContextP ctx);

extern bool gen9_render_init(VADriverContextP ctx);

#endif /* _I965_RENDER_H_ */
