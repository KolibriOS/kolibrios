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

#include "util/u_upload_mgr.h"
#include "ilo_context.h"
#include "ilo_render.h"
#include "ilo_shader.h"
#include "ilo_gpgpu.h"

static void
launch_grid(struct ilo_context *ilo,
            const uint *block_layout, const uint *grid_layout,
            const struct pipe_constant_buffer *input, uint32_t pc)
{
   const unsigned grid_offset[3] = { 0, 0, 0 };
   const unsigned thread_group_size =
      block_layout[0] * block_layout[1] * block_layout[2];
   int max_len, before_space;

   ilo_cp_set_owner(ilo->cp, INTEL_RING_RENDER, NULL);

   max_len = ilo_render_get_launch_grid_len(ilo->render, &ilo->state_vector);
   max_len += ilo_render_get_flush_len(ilo->render) * 2;

   if (max_len > ilo_cp_space(ilo->cp)) {
      ilo_cp_submit(ilo->cp, "out of space");
      assert(max_len <= ilo_cp_space(ilo->cp));
   }

   before_space = ilo_cp_space(ilo->cp);

   while (true) {
      struct ilo_builder_snapshot snapshot;

      ilo_builder_batch_snapshot(&ilo->cp->builder, &snapshot);

      ilo_render_emit_launch_grid(ilo->render, &ilo->state_vector,
            grid_offset, grid_layout, thread_group_size, input, pc);

      if (!ilo_builder_validate(&ilo->cp->builder, 0, NULL)) {
         ilo_builder_batch_restore(&ilo->cp->builder, &snapshot);

         /* flush and try again */
         if (ilo_builder_batch_used(&ilo->cp->builder)) {
            ilo_cp_submit(ilo->cp, "out of aperture");
            continue;
         }
      }

      break;
   }

   /* sanity check size estimation */
   assert(before_space - ilo_cp_space(ilo->cp) <= max_len);
}

static void
ilo_launch_grid(struct pipe_context *pipe,
                const uint *block_layout, const uint *grid_layout,
                uint32_t pc, const void *input)
{
   struct ilo_context *ilo = ilo_context(pipe);
   struct ilo_shader_state *cs = ilo->state_vector.cs;
   struct pipe_constant_buffer input_buf;

   memset(&input_buf, 0, sizeof(input_buf));

   input_buf.buffer_size =
      ilo_shader_get_kernel_param(cs, ILO_KERNEL_CS_INPUT_SIZE);
   if (input_buf.buffer_size) {
      u_upload_data(ilo->uploader, 0, input_buf.buffer_size, input,
            &input_buf.buffer_offset, &input_buf.buffer);
   }

   ilo_shader_cache_upload(ilo->shader_cache, &ilo->cp->builder);

   launch_grid(ilo, block_layout, grid_layout, &input_buf, pc);

   ilo_render_invalidate_hw(ilo->render);

   if (ilo_debug & ILO_DEBUG_NOCACHE)
      ilo_render_emit_flush(ilo->render);

   if (input_buf.buffer_size)
      pipe_resource_reference(&input_buf.buffer, NULL);
}

/**
 * Initialize GPGPU-related functions.
 */
void
ilo_init_gpgpu_functions(struct ilo_context *ilo)
{
   ilo->base.launch_grid = ilo_launch_grid;
}
