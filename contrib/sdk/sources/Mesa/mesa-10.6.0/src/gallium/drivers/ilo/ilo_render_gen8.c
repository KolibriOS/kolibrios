/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2013 LunarG, Inc.
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

#include "genhw/genhw.h"
#include "core/ilo_builder_3d.h"
#include "core/ilo_builder_render.h"
#include "util/u_dual_blend.h"

#include "ilo_blitter.h"
#include "ilo_shader.h"
#include "ilo_state.h"
#include "ilo_render_gen.h"

static void
gen8_wa_pre_depth(struct ilo_render *r)
{
   ILO_DEV_ASSERT(r->dev, 8, 8);

   /*
    * From the Ivy Bridge PRM, volume 2 part 1, page 315:
    *
    *     "Restriction: Prior to changing Depth/Stencil Buffer state (i.e.,
    *      any combination of 3DSTATE_DEPTH_BUFFER, 3DSTATE_CLEAR_PARAMS,
    *      3DSTATE_STENCIL_BUFFER, 3DSTATE_HIER_DEPTH_BUFFER) SW must first
    *      issue a pipelined depth stall (PIPE_CONTROL with Depth Stall bit
    *      set), followed by a pipelined depth cache flush (PIPE_CONTROL with
    *      Depth Flush Bit set, followed by another pipelined depth stall
    *      (PIPE_CONTROL with Depth Stall Bit set), unless SW can otherwise
    *      guarantee that the pipeline from WM onwards is already flushed
    *      (e.g., via a preceding MI_FLUSH)."
    */
   ilo_render_pipe_control(r, GEN6_PIPE_CONTROL_DEPTH_STALL);
   ilo_render_pipe_control(r, GEN6_PIPE_CONTROL_DEPTH_CACHE_FLUSH);
   ilo_render_pipe_control(r, GEN6_PIPE_CONTROL_DEPTH_STALL);
}

#define DIRTY(state) (session->pipe_dirty & ILO_DIRTY_ ## state)

static void
gen8_draw_sf(struct ilo_render *r,
             const struct ilo_state_vector *vec,
             struct ilo_render_draw_session *session)
{
   /* 3DSTATE_RASTER */
   if (DIRTY(RASTERIZER)) {
      gen8_3DSTATE_RASTER(r->builder, (vec->rasterizer) ?
            &vec->rasterizer->sf : NULL);
   }

   /* 3DSTATE_SBE */
   if (DIRTY(RASTERIZER) || DIRTY(FS)) {
      gen8_3DSTATE_SBE(r->builder, vec->fs, (vec->rasterizer) ?
            vec->rasterizer->state.sprite_coord_mode : 0);
   }

   /* 3DSTATE_SBE_SWIZ */
   if (DIRTY(FS))
      gen8_3DSTATE_SBE_SWIZ(r->builder, vec->fs);

   /* 3DSTATE_SF */
   if (DIRTY(RASTERIZER)) {
      gen8_3DSTATE_SF(r->builder, (vec->rasterizer) ?
            &vec->rasterizer->sf : NULL);
   }
}

static void
gen8_draw_wm(struct ilo_render *r,
             const struct ilo_state_vector *vec,
             struct ilo_render_draw_session *session)
{
   /* 3DSTATE_WM */
   if (DIRTY(FS) || DIRTY(RASTERIZER))
      gen8_3DSTATE_WM(r->builder, vec->fs, vec->rasterizer);

   if (DIRTY(DSA))
      gen8_3DSTATE_WM_DEPTH_STENCIL(r->builder, vec->dsa);

   /* 3DSTATE_WM_HZ_OP and 3DSTATE_WM_CHROMAKEY */
   if (r->hw_ctx_changed) {
      gen8_disable_3DSTATE_WM_HZ_OP(r->builder);
      gen8_3DSTATE_WM_CHROMAKEY(r->builder);
   }

   /* 3DSTATE_BINDING_TABLE_POINTERS_PS */
   if (session->binding_table_fs_changed) {
      gen7_3DSTATE_BINDING_TABLE_POINTERS_PS(r->builder,
            r->state.wm.BINDING_TABLE_STATE);
   }

   /* 3DSTATE_SAMPLER_STATE_POINTERS_PS */
   if (session->sampler_fs_changed) {
      gen7_3DSTATE_SAMPLER_STATE_POINTERS_PS(r->builder,
            r->state.wm.SAMPLER_STATE);
   }

   /* 3DSTATE_CONSTANT_PS */
   if (session->pcb_fs_changed) {
      gen7_3DSTATE_CONSTANT_PS(r->builder,
            &r->state.wm.PUSH_CONSTANT_BUFFER,
            &r->state.wm.PUSH_CONSTANT_BUFFER_size,
            1);
   }

   /* 3DSTATE_PS */
   if (DIRTY(FS) || r->instruction_bo_changed)
      gen8_3DSTATE_PS(r->builder, vec->fs);

   /* 3DSTATE_PS_EXTRA */
   if (DIRTY(FS) || DIRTY(DSA) || DIRTY(BLEND)) {
      const bool cc_may_kill = (vec->dsa->dw_blend_alpha ||
                                vec->blend->alpha_to_coverage);
      gen8_3DSTATE_PS_EXTRA(r->builder, vec->fs, cc_may_kill, false);
   }

   /* 3DSTATE_PS_BLEND */
   if (DIRTY(BLEND) || DIRTY(FB) || DIRTY(DSA))
      gen8_3DSTATE_PS_BLEND(r->builder, vec->blend, &vec->fb, vec->dsa);

   /* 3DSTATE_SCISSOR_STATE_POINTERS */
   if (session->scissor_changed) {
      gen6_3DSTATE_SCISSOR_STATE_POINTERS(r->builder,
            r->state.SCISSOR_RECT);
   }

   /* 3DSTATE_DEPTH_BUFFER and 3DSTATE_CLEAR_PARAMS */
   if (DIRTY(FB) || r->batch_bo_changed) {
      const struct ilo_zs_surface *zs;
      uint32_t clear_params;

      if (vec->fb.state.zsbuf) {
         const struct ilo_surface_cso *surface =
            (const struct ilo_surface_cso *) vec->fb.state.zsbuf;
         const struct ilo_texture_slice *slice =
            ilo_texture_get_slice(ilo_texture(surface->base.texture),
                  surface->base.u.tex.level, surface->base.u.tex.first_layer);

         assert(!surface->is_rt);
         zs = &surface->u.zs;
         clear_params = slice->clear_value;
      }
      else {
         zs = &vec->fb.null_zs;
         clear_params = 0;
      }

      gen8_wa_pre_depth(r);

      gen6_3DSTATE_DEPTH_BUFFER(r->builder, zs, false);
      gen6_3DSTATE_HIER_DEPTH_BUFFER(r->builder, zs);
      gen6_3DSTATE_STENCIL_BUFFER(r->builder, zs);
      gen7_3DSTATE_CLEAR_PARAMS(r->builder, clear_params);
   }
}

static void
gen8_draw_wm_sample_pattern(struct ilo_render *r,
                            const struct ilo_state_vector *vec,
                            struct ilo_render_draw_session *session)
{
   /* 3DSTATE_SAMPLE_PATTERN */
   if (r->hw_ctx_changed) {
      gen8_3DSTATE_SAMPLE_PATTERN(r->builder,
            &r->sample_pattern_1x,
            &r->sample_pattern_2x,
            &r->sample_pattern_4x,
            r->sample_pattern_8x,
            r->sample_pattern_16x);
   }
}

static void
gen8_draw_wm_multisample(struct ilo_render *r,
                         const struct ilo_state_vector *vec,
                         struct ilo_render_draw_session *session)
{
   /* 3DSTATE_MULTISAMPLE and 3DSTATE_SAMPLE_MASK */
   if (DIRTY(SAMPLE_MASK) || DIRTY(FB) || DIRTY(RASTERIZER)) {
      gen8_3DSTATE_MULTISAMPLE(r->builder, vec->fb.num_samples,
            vec->rasterizer->state.half_pixel_center);

      gen7_3DSTATE_SAMPLE_MASK(r->builder,
            (vec->fb.num_samples > 1) ? vec->sample_mask : 0x1,
            vec->fb.num_samples);
   }
}

static void
gen8_draw_vf(struct ilo_render *r,
             const struct ilo_state_vector *vec,
             struct ilo_render_draw_session *session)
{
   int i;

   /* 3DSTATE_INDEX_BUFFER */
   if (DIRTY(IB) || r->batch_bo_changed)
      gen8_3DSTATE_INDEX_BUFFER(r->builder, &vec->ib);

   /* 3DSTATE_VF */
   if (session->primitive_restart_changed) {
      gen75_3DSTATE_VF(r->builder, vec->draw->primitive_restart,
            vec->draw->restart_index);
   }

   /* 3DSTATE_VERTEX_BUFFERS */
   if (DIRTY(VB) || DIRTY(VE) || r->batch_bo_changed)
      gen6_3DSTATE_VERTEX_BUFFERS(r->builder, vec->ve, &vec->vb);

   /* 3DSTATE_VERTEX_ELEMENTS */
   if (DIRTY(VE))
      gen6_3DSTATE_VERTEX_ELEMENTS(r->builder, vec->ve);

   gen8_3DSTATE_VF_TOPOLOGY(r->builder, vec->draw->mode);

   for (i = 0; i < vec->ve->vb_count; i++) {
      gen8_3DSTATE_VF_INSTANCING(r->builder, i,
            vec->ve->instance_divisors[i]);
   }

   gen8_3DSTATE_VF_SGVS(r->builder,
         false, 0, 0,
         false, 0, 0);
}

void
ilo_render_emit_draw_commands_gen8(struct ilo_render *render,
                                   const struct ilo_state_vector *vec,
                                   struct ilo_render_draw_session *session)
{
   ILO_DEV_ASSERT(render->dev, 8, 8);

   /*
    * We try to keep the order of the commands match, as closely as possible,
    * that of the classic i965 driver.  It allows us to compare the command
    * streams easily.
    */
   gen6_draw_common_select(render, vec, session);
   gen6_draw_common_sip(render, vec, session);
   gen6_draw_vf_statistics(render, vec, session);
   gen8_draw_wm_sample_pattern(render, vec, session);
   gen6_draw_common_base_address(render, vec, session);
   gen7_draw_common_pointers_1(render, vec, session);
   gen7_draw_common_pcb_alloc(render, vec, session);
   gen7_draw_common_urb(render, vec, session);
   gen7_draw_common_pointers_2(render, vec, session);
   gen8_draw_wm_multisample(render, vec, session);
   gen7_draw_gs(render, vec, session);
   gen7_draw_hs(render, vec, session);
   gen7_draw_te(render, vec, session);
   gen7_draw_ds(render, vec, session);
   gen7_draw_vs(render, vec, session);
   gen7_draw_sol(render, vec, session);
   gen6_draw_clip(render, vec, session);
   gen8_draw_sf(render, vec, session);
   gen8_draw_wm(render, vec, session);
   gen6_draw_wm_raster(render, vec, session);
   gen6_draw_sf_rect(render, vec, session);
   gen8_draw_vf(render, vec, session);

   ilo_render_3dprimitive(render, vec->draw, &vec->ib);
}

int
ilo_render_get_draw_commands_len_gen8(const struct ilo_render *render,
                                      const struct ilo_state_vector *vec)
{
   static int len;

   ILO_DEV_ASSERT(render->dev, 8, 8);

   if (!len) {
      len += GEN7_3DSTATE_URB_ANY__SIZE * 4;
      len += GEN7_3DSTATE_PUSH_CONSTANT_ALLOC_ANY__SIZE * 5;
      len += GEN6_3DSTATE_CONSTANT_ANY__SIZE * 5;
      len += GEN7_3DSTATE_POINTERS_ANY__SIZE * (5 + 5 + 4);
      len += GEN7_3DSTATE_SO_BUFFER__SIZE * 4;
      len += GEN6_PIPE_CONTROL__SIZE * 5;

      len +=
         GEN6_STATE_BASE_ADDRESS__SIZE +
         GEN6_STATE_SIP__SIZE +
         GEN6_3DSTATE_VF_STATISTICS__SIZE +
         GEN6_PIPELINE_SELECT__SIZE +
         GEN6_3DSTATE_CLEAR_PARAMS__SIZE +
         GEN6_3DSTATE_DEPTH_BUFFER__SIZE +
         GEN6_3DSTATE_STENCIL_BUFFER__SIZE +
         GEN6_3DSTATE_HIER_DEPTH_BUFFER__SIZE +
         GEN6_3DSTATE_VERTEX_BUFFERS__SIZE +
         GEN6_3DSTATE_VERTEX_ELEMENTS__SIZE +
         GEN6_3DSTATE_INDEX_BUFFER__SIZE +
         GEN75_3DSTATE_VF__SIZE +
         GEN6_3DSTATE_VS__SIZE +
         GEN6_3DSTATE_GS__SIZE +
         GEN6_3DSTATE_CLIP__SIZE +
         GEN6_3DSTATE_SF__SIZE +
         GEN6_3DSTATE_WM__SIZE +
         GEN6_3DSTATE_SAMPLE_MASK__SIZE +
         GEN7_3DSTATE_HS__SIZE +
         GEN7_3DSTATE_TE__SIZE +
         GEN7_3DSTATE_DS__SIZE +
         GEN7_3DSTATE_STREAMOUT__SIZE +
         GEN7_3DSTATE_SBE__SIZE +
         GEN7_3DSTATE_PS__SIZE +
         GEN6_3DSTATE_DRAWING_RECTANGLE__SIZE +
         GEN6_3DSTATE_POLY_STIPPLE_OFFSET__SIZE +
         GEN6_3DSTATE_POLY_STIPPLE_PATTERN__SIZE +
         GEN6_3DSTATE_LINE_STIPPLE__SIZE +
         GEN6_3DSTATE_AA_LINE_PARAMETERS__SIZE +
         GEN6_3DSTATE_MULTISAMPLE__SIZE +
         GEN7_3DSTATE_SO_DECL_LIST__SIZE +
         GEN6_3DPRIMITIVE__SIZE;

      len +=
         GEN8_3DSTATE_VF_INSTANCING__SIZE * 33 +
         GEN8_3DSTATE_VF_SGVS__SIZE +
         GEN8_3DSTATE_VF_TOPOLOGY__SIZE +
         GEN8_3DSTATE_SBE_SWIZ__SIZE +
         GEN8_3DSTATE_RASTER__SIZE +
         GEN8_3DSTATE_WM_CHROMAKEY__SIZE +
         GEN8_3DSTATE_WM_DEPTH_STENCIL__SIZE +
         GEN8_3DSTATE_WM_HZ_OP__SIZE +
         GEN8_3DSTATE_PS_EXTRA__SIZE +
         GEN8_3DSTATE_PS_BLEND__SIZE +
         GEN8_3DSTATE_SAMPLE_PATTERN__SIZE;
   }

   return len;
}

int
ilo_render_get_rectlist_commands_len_gen8(const struct ilo_render *render,
                                          const struct ilo_blitter *blitter)
{
   ILO_DEV_ASSERT(render->dev, 8, 8);

   return 96;
}

void
ilo_render_emit_rectlist_commands_gen8(struct ilo_render *r,
                                       const struct ilo_blitter *blitter,
                                       const struct ilo_render_rectlist_session *session)
{
   uint32_t op;

   ILO_DEV_ASSERT(r->dev, 8, 8);

   gen8_wa_pre_depth(r);

   if (blitter->uses & (ILO_BLITTER_USE_FB_DEPTH |
                        ILO_BLITTER_USE_FB_STENCIL)) {
      gen6_3DSTATE_DEPTH_BUFFER(r->builder,
            &blitter->fb.dst.u.zs, true);
   }

   if (blitter->uses & ILO_BLITTER_USE_FB_DEPTH) {
      gen6_3DSTATE_HIER_DEPTH_BUFFER(r->builder,
            &blitter->fb.dst.u.zs);
   }

   if (blitter->uses & ILO_BLITTER_USE_FB_STENCIL) {
      gen6_3DSTATE_STENCIL_BUFFER(r->builder,
            &blitter->fb.dst.u.zs);
   }

   gen7_3DSTATE_CLEAR_PARAMS(r->builder,
         blitter->depth_clear_value);

   gen6_3DSTATE_DRAWING_RECTANGLE(r->builder, 0, 0,
         blitter->fb.width, blitter->fb.height);

   switch (blitter->op) {
   case ILO_BLITTER_RECTLIST_CLEAR_ZS:
      op = 0;
      if (blitter->uses & ILO_BLITTER_USE_FB_DEPTH)
         op |= GEN8_WM_HZ_DW1_DEPTH_CLEAR;
      if (blitter->uses & ILO_BLITTER_USE_FB_STENCIL)
         op |= GEN8_WM_HZ_DW1_STENCIL_CLEAR;
      break;
   case ILO_BLITTER_RECTLIST_RESOLVE_Z:
      op = GEN8_WM_HZ_DW1_DEPTH_RESOLVE;
      break;
   case ILO_BLITTER_RECTLIST_RESOLVE_HIZ:
      op = GEN8_WM_HZ_DW1_HIZ_RESOLVE;
      break;
   default:
      op = 0;
      break;
   }

   gen8_3DSTATE_WM_HZ_OP(r->builder, op, blitter->fb.width,
         blitter->fb.height, blitter->fb.num_samples);

   ilo_render_pipe_control(r, GEN6_PIPE_CONTROL_WRITE_IMM);

   gen8_disable_3DSTATE_WM_HZ_OP(r->builder);
}
