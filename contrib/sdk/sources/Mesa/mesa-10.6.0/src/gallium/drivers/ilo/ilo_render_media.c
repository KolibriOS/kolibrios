/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2014 LunarG, Inc.
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
#include "core/ilo_builder_media.h"
#include "core/ilo_builder_mi.h"
#include "core/ilo_builder_render.h"

#include "ilo_state.h"
#include "ilo_render_gen.h"

struct gen7_l3_config {
   int slm;
   int urb;
   int rest;
   int dc;
   int ro;
   int is;
   int c;
   int t;
};

/*
 * From the Ivy Bridge PRM, volume 1 part 7, page 10:
 *
 *     "Normal L3/URB mode (non-SLM mode), uses all 4 banks of L3 equally to
 *      distribute cycles. The following allocation is a suggested programming
 *      model. Note all numbers below are given in KBytes."
 *
 * From the Haswell PRM, volume 7, page 662:
 *
 *     "The configuration for {SLM = 0,URB = 224,DC = 32,RO = 256,IS = 0,C =
 *      0,T =0, SUM 512} was validated as a later supported configuration and
 *      can be utilized if desired."
 */
static const struct gen7_l3_config gen7_l3_non_slm_configs[] = {
   /*       SLM   URB  Rest    DC    RO   I/S     C     T */
   [0] = {    0,  256,    0,    0,  256,    0,    0,    0, },
   [1] = {    0,  256,    0,  128,  128,    0,    0,    0, },
   [2] = {    0,  256,    0,   32,    0,   64,   32,  128, },
   [3] = {    0,  224,    0,   64,    0,   64,   32,  128, },
   [4] = {    0,  224,    0,  128,    0,   64,   32,   64, },
   [5] = {    0,  224,    0,   64,    0,  128,   32,   64, },
   [6] = {    0,  224,    0,    0,    0,  128,   32,  128, },
   [7] = {    0,  256,    0,    0,    0,  128,    0,  128, },

   [8] = {    0,  224,    0,   32,  256,    0,    0,    0, },
};

/*
 * From the Ivy Bridge PRM, volume 1 part 7, page 11:
 *
 *     "With the existence of Shared Local Memory, a 64KB chunk from each of
 *      the 2 L3 banks will be reserved for SLM usage. The remaining cache
 *      space is divided between the remaining clients. SLM allocation is done
 *      via reducing the number of ways on the two banks from 64 to 32."
 *
 * From the Haswell PRM, volume 7, page 662:
 *
 *     "The configuration for {SLM = 128,URB = 128,DC = 0,RO = 256,IS = 0,C =
 *      0,T =0, SUM 512} was validated as a later supported configuration and
 *      can be utilized if desired. For this configuration, global atomics
 *      must be programmed to be in GTI."
 */
static const struct gen7_l3_config gen7_l3_slm_configs[] = {
   /*       SLM   URB  Rest    DC    RO   I/S     C     T */
   [0] = {  128,  128,    0,  128,  128,    0,    0,    0, },
   [1] = {  128,  128,    0,   64,    0,   64,   64,   64, },
   [2] = {  128,  128,    0,   32,    0,   64,   32,  128, },
   [3] = {  128,  128,    0,   32,    0,  128,   32,   64, },

   [4] = {  128,  128,    0,    0,  256,    0,    0,    0, },
};

static void
gen7_launch_grid_l3(struct ilo_render *r, bool use_slm)
{
   uint32_t l3sqcreg1, l3cntlreg2, l3cntlreg3;
   const struct gen7_l3_config *conf;

   /*
    * This function mostly follows what beignet does.  I do not know why, for
    * example, CON4DCUNC should be reset.  I do not know if it should be set
    * again after launch_grid().
    */

   ILO_DEV_ASSERT(r->dev, 7, 7.5);

   if (use_slm)
      conf = &gen7_l3_slm_configs[1];
   else
      conf = &gen7_l3_non_slm_configs[4];

   /* unset GEN7_REG_L3SQCREG1_CON4DCUNC (without readback first) */
   if (ilo_dev_gen(r->dev) >= ILO_GEN(7.5)) {
      l3sqcreg1 = GEN75_REG_L3SQCREG1_SQGPCI_24 |
                  GEN75_REG_L3SQCREG1_SQHPCI_8;
   } else {
      l3sqcreg1 = GEN7_REG_L3SQCREG1_SQGHPCI_18_6;
   }

   l3cntlreg2 = (conf->dc / 8) << GEN7_REG_L3CNTLREG2_DCWASS__SHIFT |
                (conf->ro / 8) << GEN7_REG_L3CNTLREG2_RDOCPL__SHIFT |
                (conf->urb / 8) << GEN7_REG_L3CNTLREG2_URBALL__SHIFT;

   l3cntlreg3 = (conf->t / 8) << GEN7_REG_L3CNTLREG3_TXWYALL__SHIFT |
                (conf->c / 8) << GEN7_REG_L3CNTLREG3_CTWYALL__SHIFT |
                (conf->is / 8) << GEN7_REG_L3CNTLREG3_ISWYALL__SHIFT;

   if (conf->slm) {
      /*
       * From the Ivy Bridge PRM, volume 1 part 7, page 11:
       *
       *     "Note that URB needs to be set as low b/w client in SLM mode,
       *      else the hash will fail. This is a required s/w model."
       */
      l3cntlreg2 |= GEN7_REG_L3CNTLREG2_URBSLMB |
                    GEN7_REG_L3CNTLREG2_SLMMENB;
   }

   gen6_MI_LOAD_REGISTER_IMM(r->builder, GEN7_REG_L3SQCREG1, l3sqcreg1);
   gen6_MI_LOAD_REGISTER_IMM(r->builder, GEN7_REG_L3CNTLREG2, l3cntlreg2);
   gen6_MI_LOAD_REGISTER_IMM(r->builder, GEN7_REG_L3CNTLREG3, l3cntlreg3);
}

int
ilo_render_get_launch_grid_commands_len(const struct ilo_render *render,
                                        const struct ilo_state_vector *vec)
{
   static int len;

   ILO_DEV_ASSERT(render->dev, 7, 7.5);

   if (!len) {
      len +=
         GEN6_PIPELINE_SELECT__SIZE +
         GEN6_STATE_BASE_ADDRESS__SIZE +
         GEN6_MEDIA_VFE_STATE__SIZE +
         GEN6_MEDIA_CURBE_LOAD__SIZE +
         GEN6_MEDIA_INTERFACE_DESCRIPTOR_LOAD__SIZE +
         GEN6_MEDIA_STATE_FLUSH__SIZE;

      len += ilo_render_get_flush_len(render) * 3;

      if (ilo_dev_gen(render->dev) >= ILO_GEN(7)) {
         len += GEN6_MI_LOAD_REGISTER_IMM__SIZE * 3 * 2;
         len += GEN7_GPGPU_WALKER__SIZE;
      }
   }

   return len;
}

void
ilo_render_emit_launch_grid_commands(struct ilo_render *render,
                                     const struct ilo_state_vector *vec,
                                     const struct ilo_render_launch_grid_session *session)
{
   const unsigned batch_used = ilo_builder_batch_used(render->builder);
   const uint32_t pcb = render->state.cs.PUSH_CONSTANT_BUFFER;
   const int pcb_size = render->state.cs.PUSH_CONSTANT_BUFFER_size;
   int simd_size;
   bool use_slm;

   ILO_DEV_ASSERT(render->dev, 7, 7.5);

   simd_size = ilo_shader_get_kernel_param(vec->cs, ILO_KERNEL_CS_SIMD_SIZE);
   use_slm = ilo_shader_get_kernel_param(vec->cs, ILO_KERNEL_CS_LOCAL_SIZE);

   ilo_render_emit_flush(render);

   if (ilo_dev_gen(render->dev) >= ILO_GEN(7)) {
      gen7_launch_grid_l3(render, use_slm);
      ilo_render_emit_flush(render);

      gen6_PIPELINE_SELECT(render->builder,
            GEN7_PIPELINE_SELECT_DW0_SELECT_GPGPU);
   } else {
      gen6_PIPELINE_SELECT(render->builder,
            GEN6_PIPELINE_SELECT_DW0_SELECT_MEDIA);
   }

   gen6_state_base_address(render->builder, true);

   gen6_MEDIA_VFE_STATE(render->builder, pcb_size, use_slm);

   if (pcb_size)
      gen6_MEDIA_CURBE_LOAD(render->builder, pcb, pcb_size);

   gen6_MEDIA_INTERFACE_DESCRIPTOR_LOAD(render->builder,
         session->idrt, session->idrt_size);

   gen7_GPGPU_WALKER(render->builder, session->thread_group_offset,
         session->thread_group_dim, session->thread_group_size, simd_size);

   gen6_MEDIA_STATE_FLUSH(render->builder);

   if (ilo_dev_gen(render->dev) >= ILO_GEN(7) && use_slm) {
      ilo_render_emit_flush(render);
      gen7_launch_grid_l3(render, false);
   }

   assert(ilo_builder_batch_used(render->builder) <= batch_used +
         ilo_render_get_launch_grid_commands_len(render, vec));
}
