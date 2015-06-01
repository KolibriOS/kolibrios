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
#include "core/ilo_builder.h"
#include "core/ilo_builder_mi.h"
#include "core/ilo_builder_render.h"
#include "core/intel_winsys.h"
#include "util/u_prim.h"

#include "ilo_query.h"
#include "ilo_render_gen.h"

/* in S1.3 */
struct sample_position {
   int8_t x, y;
};

static const struct sample_position ilo_sample_pattern_1x[1] = {
   {  0,  0 },
};

static const struct sample_position ilo_sample_pattern_2x[2] = {
   { -4, -4 },
   {  4,  4 },
};

static const struct sample_position ilo_sample_pattern_4x[4] = {
   { -2, -6 },
   {  6, -2 },
   { -6,  2 },
   {  2,  6 },
};

/* \see brw_multisample_positions_8x */
static const struct sample_position ilo_sample_pattern_8x[8] = {
   { -1,  1 },
   {  1,  5 },
   {  3, -5 },
   {  5,  3 },
   { -7, -1 },
   { -3, -7 },
   {  7, -3 },
   { -5,  7 },
};

static const struct sample_position ilo_sample_pattern_16x[16] = {
   {  0,  2 },
   {  3,  0 },
   { -3, -2 },
   { -2, -4 },
   {  4,  3 },
   {  5,  1 },
   {  6, -1 },
   {  2, -6 },
   { -4,  5 },
   { -5, -5 },
   { -1, -7 },
   {  7, -3 },
   { -7,  4 },
   {  1, -8 },
   { -6,  6 },
   { -8,  7 },
};

static uint8_t
pack_sample_position(const struct sample_position *pos)
{
   return (pos->x + 8) << 4 | (pos->y + 8);
}

static void
get_sample_position(const struct sample_position *pos, float *x, float *y)
{
   *x = (float) (pos->x + 8) / 16.0f;
   *y = (float) (pos->y + 8) / 16.0f;
}

struct ilo_render *
ilo_render_create(struct ilo_builder *builder)
{
   struct ilo_render *render;
   int i;

   render = CALLOC_STRUCT(ilo_render);
   if (!render)
      return NULL;

   render->dev = builder->dev;
   render->builder = builder;

   render->workaround_bo = intel_winsys_alloc_bo(builder->winsys,
         "PIPE_CONTROL workaround", 4096, false);
   if (!render->workaround_bo) {
      ilo_warn("failed to allocate PIPE_CONTROL workaround bo\n");
      FREE(render);
      return NULL;
   }

   /* pack into dwords */
   render->sample_pattern_1x = pack_sample_position(ilo_sample_pattern_1x);
   render->sample_pattern_2x =
      pack_sample_position(&ilo_sample_pattern_2x[1]) << 8 |
      pack_sample_position(&ilo_sample_pattern_2x[0]);
   for (i = 0; i < 4; i++) {
      render->sample_pattern_4x |=
         pack_sample_position(&ilo_sample_pattern_4x[i]) << (8 * i);

      render->sample_pattern_8x[0] |=
         pack_sample_position(&ilo_sample_pattern_8x[i]) << (8 * i);
      render->sample_pattern_8x[1] |=
         pack_sample_position(&ilo_sample_pattern_8x[i + 4]) << (8 * i);

      render->sample_pattern_16x[0] |=
         pack_sample_position(&ilo_sample_pattern_16x[i]) << (8 * i);
      render->sample_pattern_16x[1] |=
         pack_sample_position(&ilo_sample_pattern_16x[i + 4]) << (8 * i);
      render->sample_pattern_16x[2] |=
         pack_sample_position(&ilo_sample_pattern_16x[i + 8]) << (8 * i);
      render->sample_pattern_16x[3] |=
         pack_sample_position(&ilo_sample_pattern_16x[i + 12]) << (8 * i);
   }

   ilo_render_invalidate_hw(render);
   ilo_render_invalidate_builder(render);

   return render;
}

void
ilo_render_destroy(struct ilo_render *render)
{
   intel_bo_unref(render->workaround_bo);
   FREE(render);
}

void
ilo_render_get_sample_position(const struct ilo_render *render,
                               unsigned sample_count,
                               unsigned sample_index,
                               float *x, float *y)
{
   const struct sample_position *pattern;

   switch (sample_count) {
   case 1:
      assert(sample_index < Elements(ilo_sample_pattern_1x));
      pattern = ilo_sample_pattern_1x;
      break;
   case 2:
      assert(sample_index < Elements(ilo_sample_pattern_2x));
      pattern = ilo_sample_pattern_2x;
      break;
   case 4:
      assert(sample_index < Elements(ilo_sample_pattern_4x));
      pattern = ilo_sample_pattern_4x;
      break;
   case 8:
      assert(sample_index < Elements(ilo_sample_pattern_8x));
      pattern = ilo_sample_pattern_8x;
      break;
   case 16:
      assert(sample_index < Elements(ilo_sample_pattern_16x));
      pattern = ilo_sample_pattern_16x;
      break;
   default:
      assert(!"unknown sample count");
      *x = 0.5f;
      *y = 0.5f;
      return;
      break;
   }

   get_sample_position(&pattern[sample_index], x, y);
}

void
ilo_render_invalidate_hw(struct ilo_render *render)
{
   render->hw_ctx_changed = true;
}

void
ilo_render_invalidate_builder(struct ilo_render *render)
{
   render->batch_bo_changed = true;
   render->state_bo_changed = true;
   render->instruction_bo_changed = true;

   /* Kernel flushes everything.  Shouldn't we set all bits here? */
   render->state.current_pipe_control_dw1 = 0;
}

/**
 * Return the command length of ilo_render_emit_flush().
 */
int
ilo_render_get_flush_len(const struct ilo_render *render)
{
   int len;

   ILO_DEV_ASSERT(render->dev, 6, 8);

   len = GEN6_PIPE_CONTROL__SIZE;

   /* plus gen6_wa_pre_pipe_control() */
   if (ilo_dev_gen(render->dev) == ILO_GEN(6))
      len *= 3;

   return len;
}

/**
 * Emit PIPE_CONTROLs to flush all caches.
 */
void
ilo_render_emit_flush(struct ilo_render *render)
{
   const uint32_t dw1 = GEN6_PIPE_CONTROL_INSTRUCTION_CACHE_INVALIDATE |
                        GEN6_PIPE_CONTROL_RENDER_CACHE_FLUSH |
                        GEN6_PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                        GEN6_PIPE_CONTROL_VF_CACHE_INVALIDATE |
                        GEN6_PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
                        GEN6_PIPE_CONTROL_CS_STALL;
   const unsigned batch_used = ilo_builder_batch_used(render->builder);

   ILO_DEV_ASSERT(render->dev, 6, 8);

   if (ilo_dev_gen(render->dev) == ILO_GEN(6))
      gen6_wa_pre_pipe_control(render, dw1);

   ilo_render_pipe_control(render, dw1);

   assert(ilo_builder_batch_used(render->builder) <= batch_used +
         ilo_render_get_flush_len(render));
}

/**
 * Return the command length of ilo_render_emit_query().
 */
int
ilo_render_get_query_len(const struct ilo_render *render,
                         unsigned query_type)
{
   int len;

   ILO_DEV_ASSERT(render->dev, 6, 8);

   /* always a flush or a variant of flush */
   len = ilo_render_get_flush_len(render);

   switch (query_type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIME_ELAPSED:
      /* no reg */
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      len += GEN6_MI_STORE_REGISTER_MEM__SIZE * 2;
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      {
         const int num_regs =
            (ilo_dev_gen(render->dev) >= ILO_GEN(7)) ? 10 : 8;
         const int num_pads =
            (ilo_dev_gen(render->dev) >= ILO_GEN(7)) ? 1 : 3;

         len += GEN6_MI_STORE_REGISTER_MEM__SIZE * 2 * num_regs +
                GEN6_MI_STORE_DATA_IMM__SIZE * num_pads;
      }
      break;
   default:
      len = 0;
      break;
   }

   return len;
}

/**
 * Emit PIPE_CONTROLs or MI_STORE_REGISTER_MEMs to store register values.
 */
void
ilo_render_emit_query(struct ilo_render *render,
                      struct ilo_query *q, uint32_t offset)
{
   const uint32_t pipeline_statistics_regs[11] = {
      GEN6_REG_IA_VERTICES_COUNT,
      GEN6_REG_IA_PRIMITIVES_COUNT,
      GEN6_REG_VS_INVOCATION_COUNT,
      GEN6_REG_GS_INVOCATION_COUNT,
      GEN6_REG_GS_PRIMITIVES_COUNT,
      GEN6_REG_CL_INVOCATION_COUNT,
      GEN6_REG_CL_PRIMITIVES_COUNT,
      GEN6_REG_PS_INVOCATION_COUNT,
      (ilo_dev_gen(render->dev) >= ILO_GEN(7)) ?
         GEN7_REG_HS_INVOCATION_COUNT : 0,
      (ilo_dev_gen(render->dev) >= ILO_GEN(7)) ?
         GEN7_REG_DS_INVOCATION_COUNT : 0,
      0,
   };
   const uint32_t primitives_generated_reg =
      (ilo_dev_gen(render->dev) >= ILO_GEN(7) && q->index > 0) ?
      GEN7_REG_SO_PRIM_STORAGE_NEEDED(q->index) :
      GEN6_REG_CL_INVOCATION_COUNT;
   const uint32_t primitives_emitted_reg =
      (ilo_dev_gen(render->dev) >= ILO_GEN(7)) ?
      GEN7_REG_SO_NUM_PRIMS_WRITTEN(q->index) :
      GEN6_REG_SO_NUM_PRIMS_WRITTEN;
   const unsigned batch_used = ilo_builder_batch_used(render->builder);
   const uint32_t *regs;
   int reg_count = 0, i;
   uint32_t pipe_control_dw1 = 0;

   ILO_DEV_ASSERT(render->dev, 6, 8);

   switch (q->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
      pipe_control_dw1 = GEN6_PIPE_CONTROL_DEPTH_STALL |
                         GEN6_PIPE_CONTROL_WRITE_PS_DEPTH_COUNT;
      break;
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIME_ELAPSED:
      pipe_control_dw1 = GEN6_PIPE_CONTROL_WRITE_TIMESTAMP;
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      regs = &primitives_generated_reg;
      reg_count = 1;
      break;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      regs = &primitives_emitted_reg;
      reg_count = 1;
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      regs = pipeline_statistics_regs;
      reg_count = Elements(pipeline_statistics_regs);
      break;
   default:
      break;
   }

   if (pipe_control_dw1) {
      assert(!reg_count);

      if (ilo_dev_gen(render->dev) == ILO_GEN(6))
         gen6_wa_pre_pipe_control(render, pipe_control_dw1);

      gen6_PIPE_CONTROL(render->builder, pipe_control_dw1, q->bo, offset, 0);

      render->state.current_pipe_control_dw1 |= pipe_control_dw1;
      render->state.deferred_pipe_control_dw1 &= ~pipe_control_dw1;
   } else if (reg_count) {
      ilo_render_emit_flush(render);
   }

   for (i = 0; i < reg_count; i++) {
      if (regs[i]) {
         /* store lower 32 bits */
         gen6_MI_STORE_REGISTER_MEM(render->builder, regs[i], q->bo, offset);
         /* store higher 32 bits */
         gen6_MI_STORE_REGISTER_MEM(render->builder, regs[i] + 4,
               q->bo, offset + 4);
      } else {
         gen6_MI_STORE_DATA_IMM(render->builder, q->bo, offset, 0);
      }

      offset += 8;
   }

   assert(ilo_builder_batch_used(render->builder) <= batch_used +
         ilo_render_get_query_len(render, q->type));
}

int
ilo_render_get_rectlist_len(const struct ilo_render *render,
                            const struct ilo_blitter *blitter)
{
   ILO_DEV_ASSERT(render->dev, 6, 8);

   return ilo_render_get_rectlist_dynamic_states_len(render, blitter) +
          ilo_render_get_rectlist_commands_len(render, blitter);
}

void
ilo_render_emit_rectlist(struct ilo_render *render,
                         const struct ilo_blitter *blitter)
{
   struct ilo_render_rectlist_session session;

   ILO_DEV_ASSERT(render->dev, 6, 8);

   memset(&session, 0, sizeof(session));
   ilo_render_emit_rectlist_dynamic_states(render, blitter, &session);
   ilo_render_emit_rectlist_commands(render, blitter, &session);
}

int
ilo_render_get_draw_len(const struct ilo_render *render,
                        const struct ilo_state_vector *vec)
{
   ILO_DEV_ASSERT(render->dev, 6, 8);

   return ilo_render_get_draw_dynamic_states_len(render, vec) +
          ilo_render_get_draw_surface_states_len(render, vec) +
          ilo_render_get_draw_commands_len(render, vec);
}

static void
draw_session_prepare(struct ilo_render *render,
                     const struct ilo_state_vector *vec,
                     struct ilo_render_draw_session *session)
{
   memset(session, 0, sizeof(*session));
   session->pipe_dirty = vec->dirty;
   session->reduced_prim = u_reduced_prim(vec->draw->mode);

   if (render->hw_ctx_changed) {
      /* these should be enough to make everything uploaded */
      render->batch_bo_changed = true;
      render->state_bo_changed = true;
      render->instruction_bo_changed = true;

      session->prim_changed = true;
      session->primitive_restart_changed = true;
   } else {
      session->prim_changed =
         (render->state.reduced_prim != session->reduced_prim);
      session->primitive_restart_changed =
         (render->state.primitive_restart != vec->draw->primitive_restart);
   }
}

static void
draw_session_end(struct ilo_render *render,
                 const struct ilo_state_vector *vec,
                 struct ilo_render_draw_session *session)
{
   render->hw_ctx_changed = false;

   render->batch_bo_changed = false;
   render->state_bo_changed = false;
   render->instruction_bo_changed = false;

   render->state.reduced_prim = session->reduced_prim;
   render->state.primitive_restart = vec->draw->primitive_restart;
}

void
ilo_render_emit_draw(struct ilo_render *render,
                     const struct ilo_state_vector *vec)
{
   struct ilo_render_draw_session session;

   ILO_DEV_ASSERT(render->dev, 6, 8);

   draw_session_prepare(render, vec, &session);

   /* force all states to be uploaded if the state bo changed */
   if (render->state_bo_changed)
      session.pipe_dirty = ILO_DIRTY_ALL;
   else
      session.pipe_dirty = vec->dirty;

   ilo_render_emit_draw_dynamic_states(render, vec, &session);
   ilo_render_emit_draw_surface_states(render, vec, &session);

   /* force all commands to be uploaded if the HW context changed */
   if (render->hw_ctx_changed)
      session.pipe_dirty = ILO_DIRTY_ALL;
   else
      session.pipe_dirty = vec->dirty;

   ilo_render_emit_draw_commands(render, vec, &session);

   draw_session_end(render, vec, &session);
}

int
ilo_render_get_launch_grid_len(const struct ilo_render *render,
                               const struct ilo_state_vector *vec)
{
   ILO_DEV_ASSERT(render->dev, 7, 7.5);

   return ilo_render_get_launch_grid_surface_states_len(render, vec) +
          ilo_render_get_launch_grid_dynamic_states_len(render, vec) +
          ilo_render_get_launch_grid_commands_len(render, vec);
}

void
ilo_render_emit_launch_grid(struct ilo_render *render,
                            const struct ilo_state_vector *vec,
                            const unsigned thread_group_offset[3],
                            const unsigned thread_group_dim[3],
                            unsigned thread_group_size,
                            const struct pipe_constant_buffer *input,
                            uint32_t pc)
{
   struct ilo_render_launch_grid_session session;

   ILO_DEV_ASSERT(render->dev, 7, 7.5);

   assert(input->buffer);

   memset(&session, 0, sizeof(session));

   session.thread_group_offset = thread_group_offset;
   session.thread_group_dim = thread_group_dim;
   session.thread_group_size = thread_group_size;
   session.input = input;
   session.pc = pc;

   ilo_render_emit_launch_grid_surface_states(render, vec, &session);
   ilo_render_emit_launch_grid_dynamic_states(render, vec, &session);
   ilo_render_emit_launch_grid_commands(render, vec, &session);
}
