/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2012-2013 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
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
 *    Chia-I Wu <olv@lunarg.com>
 */

#ifndef ILO_STATE_H
#define ILO_STATE_H

#include "core/ilo_state_3d.h"
#include "pipe/p_state.h"
#include "util/u_dynarray.h"

#include "ilo_common.h"

/**
 * States that we track.
 *
 * XXX Do we want to count each sampler or vertex buffer as a state?  If that
 * is the case, there are simply not enough bits.
 *
 * XXX We want to treat primitive type and depth clear value as states, but
 * there are not enough bits.
 */
enum ilo_state {
   ILO_STATE_VB,
   ILO_STATE_VE,
   ILO_STATE_IB,
   ILO_STATE_VS,
   ILO_STATE_GS,
   ILO_STATE_SO,
   ILO_STATE_CLIP,
   ILO_STATE_VIEWPORT,
   ILO_STATE_SCISSOR,
   ILO_STATE_RASTERIZER,
   ILO_STATE_POLY_STIPPLE,
   ILO_STATE_SAMPLE_MASK,
   ILO_STATE_FS,
   ILO_STATE_DSA,
   ILO_STATE_STENCIL_REF,
   ILO_STATE_BLEND,
   ILO_STATE_BLEND_COLOR,
   ILO_STATE_FB,

   ILO_STATE_SAMPLER_VS,
   ILO_STATE_SAMPLER_GS,
   ILO_STATE_SAMPLER_FS,
   ILO_STATE_SAMPLER_CS,
   ILO_STATE_VIEW_VS,
   ILO_STATE_VIEW_GS,
   ILO_STATE_VIEW_FS,
   ILO_STATE_VIEW_CS,
   ILO_STATE_CBUF,
   ILO_STATE_RESOURCE,

   ILO_STATE_CS,
   ILO_STATE_CS_RESOURCE,
   ILO_STATE_GLOBAL_BINDING,

   ILO_STATE_COUNT,
};

/**
 * Dirty flags of the states.
 */
enum ilo_dirty_flags {
   ILO_DIRTY_VB               = 1 << ILO_STATE_VB,
   ILO_DIRTY_VE               = 1 << ILO_STATE_VE,
   ILO_DIRTY_IB               = 1 << ILO_STATE_IB,
   ILO_DIRTY_VS               = 1 << ILO_STATE_VS,
   ILO_DIRTY_GS               = 1 << ILO_STATE_GS,
   ILO_DIRTY_SO               = 1 << ILO_STATE_SO,
   ILO_DIRTY_CLIP             = 1 << ILO_STATE_CLIP,
   ILO_DIRTY_VIEWPORT         = 1 << ILO_STATE_VIEWPORT,
   ILO_DIRTY_SCISSOR          = 1 << ILO_STATE_SCISSOR,
   ILO_DIRTY_RASTERIZER       = 1 << ILO_STATE_RASTERIZER,
   ILO_DIRTY_POLY_STIPPLE     = 1 << ILO_STATE_POLY_STIPPLE,
   ILO_DIRTY_SAMPLE_MASK      = 1 << ILO_STATE_SAMPLE_MASK,
   ILO_DIRTY_FS               = 1 << ILO_STATE_FS,
   ILO_DIRTY_DSA              = 1 << ILO_STATE_DSA,
   ILO_DIRTY_STENCIL_REF      = 1 << ILO_STATE_STENCIL_REF,
   ILO_DIRTY_BLEND            = 1 << ILO_STATE_BLEND,
   ILO_DIRTY_BLEND_COLOR      = 1 << ILO_STATE_BLEND_COLOR,
   ILO_DIRTY_FB               = 1 << ILO_STATE_FB,
   ILO_DIRTY_SAMPLER_VS       = 1 << ILO_STATE_SAMPLER_VS,
   ILO_DIRTY_SAMPLER_GS       = 1 << ILO_STATE_SAMPLER_GS,
   ILO_DIRTY_SAMPLER_FS       = 1 << ILO_STATE_SAMPLER_FS,
   ILO_DIRTY_SAMPLER_CS       = 1 << ILO_STATE_SAMPLER_CS,
   ILO_DIRTY_VIEW_VS          = 1 << ILO_STATE_VIEW_VS,
   ILO_DIRTY_VIEW_GS          = 1 << ILO_STATE_VIEW_GS,
   ILO_DIRTY_VIEW_FS          = 1 << ILO_STATE_VIEW_FS,
   ILO_DIRTY_VIEW_CS          = 1 << ILO_STATE_VIEW_CS,
   ILO_DIRTY_CBUF             = 1 << ILO_STATE_CBUF,
   ILO_DIRTY_RESOURCE         = 1 << ILO_STATE_RESOURCE,
   ILO_DIRTY_CS               = 1 << ILO_STATE_CS,
   ILO_DIRTY_CS_RESOURCE      = 1 << ILO_STATE_CS_RESOURCE,
   ILO_DIRTY_GLOBAL_BINDING   = 1 << ILO_STATE_GLOBAL_BINDING,
   ILO_DIRTY_ALL              = 0xffffffff,
};

struct ilo_context;

struct ilo_global_binding_cso {
   struct pipe_resource *resource;
   uint32_t *handle;
};

/*
 * In theory, we would like a "virtual" bo that serves as the global memory
 * region.  The virtual bo would reserve a region in the GTT aperture, but the
 * pages of it would come from those of the global bindings.
 *
 * The virtual bo would be created in launch_grid().  The global bindings
 * would be added to the virtual bo.  A SURFACE_STATE for the virtual bo would
 * be created.  The handles returned by set_global_binding() would be offsets
 * into the virtual bo.
 *
 * But for now, we will create a SURFACE_STATE for each of the bindings.  The
 * handle of a global binding consists of the offset and the binding table
 * index.
 */
struct ilo_global_binding {
   struct util_dynarray bindings;
   unsigned count;
};

struct ilo_state_vector {
   const struct pipe_draw_info *draw;

   uint32_t dirty;

   struct ilo_vb_state vb;
   struct ilo_ve_state *ve;
   struct ilo_ib_state ib;

   struct ilo_shader_state *vs;
   struct ilo_shader_state *gs;

   struct ilo_so_state so;

   struct pipe_clip_state clip;
   struct ilo_viewport_state viewport;
   struct ilo_scissor_state scissor;

   const struct ilo_rasterizer_state *rasterizer;
   struct pipe_poly_stipple poly_stipple;
   unsigned sample_mask;

   struct ilo_shader_state *fs;

   const struct ilo_dsa_state *dsa;
   struct pipe_stencil_ref stencil_ref;
   const struct ilo_blend_state *blend;
   struct pipe_blend_color blend_color;
   struct ilo_fb_state fb;

   /* shader resources */
   struct ilo_sampler_state sampler[PIPE_SHADER_TYPES];
   struct ilo_view_state view[PIPE_SHADER_TYPES];
   struct ilo_cbuf_state cbuf[PIPE_SHADER_TYPES];
   struct ilo_resource_state resource;

   /* GPGPU */
   struct ilo_shader_state *cs;
   struct ilo_resource_state cs_resource;
   struct ilo_global_binding global_binding;
};

void
ilo_init_state_functions(struct ilo_context *ilo);

void
ilo_finalize_3d_states(struct ilo_context *ilo,
                       const struct pipe_draw_info *draw);

void
ilo_finalize_compute_states(struct ilo_context *ilo);

void
ilo_state_vector_init(const struct ilo_dev *dev,
                      struct ilo_state_vector *vec);

void
ilo_state_vector_cleanup(struct ilo_state_vector *vec);

void
ilo_state_vector_resource_renamed(struct ilo_state_vector *vec,
                                  struct pipe_resource *res);

void
ilo_state_vector_dump_dirty(const struct ilo_state_vector *vec);

#endif /* ILO_STATE_H */
