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

#include "toy_compiler.h"
#include "toy_helpers.h"
#include "toy_legalize.h"
#include "toy_optimize.h"
#include "ilo_shader_internal.h"

struct cs_compile_context {
   struct ilo_shader *shader;
   const struct ilo_shader_variant *variant;

   struct toy_compiler tc;

   int first_free_grf;
   int last_free_grf;

   int num_grf_per_vrf;

   int first_free_mrf;
   int last_free_mrf;
};

/**
 * Compile the shader.
 */
static bool
cs_compile(struct cs_compile_context *ccc)
{
   struct toy_compiler *tc = &ccc->tc;
   struct ilo_shader *sh = ccc->shader;

   toy_compiler_legalize_for_ra(tc);
   toy_compiler_optimize(tc);
   toy_compiler_allocate_registers(tc,
         ccc->first_free_grf,
         ccc->last_free_grf,
         ccc->num_grf_per_vrf);
   toy_compiler_legalize_for_asm(tc);

   if (tc->fail) {
      ilo_err("failed to legalize FS instructions: %s\n", tc->reason);
      return false;
   }

   if (ilo_debug & ILO_DEBUG_CS) {
      ilo_printf("legalized instructions:\n");
      toy_compiler_dump(tc);
      ilo_printf("\n");
   }

   if (true) {
      sh->kernel = toy_compiler_assemble(tc, &sh->kernel_size);
   } else {
      static const uint32_t microcode[] = {
         /* fill in the microcode here */
         0x0, 0x0, 0x0, 0x0,
      };
      const bool swap = true;

      sh->kernel_size = sizeof(microcode);
      sh->kernel = MALLOC(sh->kernel_size);

      if (sh->kernel) {
         const int num_dwords = sizeof(microcode) / 4;
         const uint32_t *src = microcode;
         uint32_t *dst = (uint32_t *) sh->kernel;
         int i;

         for (i = 0; i < num_dwords; i += 4) {
            if (swap) {
               dst[i + 0] = src[i + 3];
               dst[i + 1] = src[i + 2];
               dst[i + 2] = src[i + 1];
               dst[i + 3] = src[i + 0];
            }
            else {
               memcpy(dst, src, 16);
            }
         }
      }
   }

   if (!sh->kernel) {
      ilo_err("failed to compile CS: %s\n", tc->reason);
      return false;
   }

   if (ilo_debug & ILO_DEBUG_CS) {
      ilo_printf("disassembly:\n");
      toy_compiler_disassemble(tc->dev, sh->kernel, sh->kernel_size, false);
      ilo_printf("\n");
   }

   return true;
}

static void
cs_dummy(struct cs_compile_context *ccc)
{
   struct toy_compiler *tc = &ccc->tc;
   struct toy_dst header;
   struct toy_src r0, desc;
   struct toy_inst *inst;

   header = tdst_ud(tdst(TOY_FILE_MRF, ccc->first_free_mrf, 0));
   r0 = tsrc_ud(tsrc(TOY_FILE_GRF, 0, 0));

   inst = tc_MOV(tc, header, r0);
   inst->exec_size = GEN6_EXECSIZE_8;
   inst->mask_ctrl = GEN6_MASKCTRL_NOMASK;

   desc = tsrc_imm_mdesc(tc, true, 1, 0, true,
         GEN6_MSG_TS_RESOURCE_SELECT_NO_DEREF |
         GEN6_MSG_TS_REQUESTER_TYPE_ROOT |
         GEN6_MSG_TS_OPCODE_DEREF);

   tc_SEND(tc, tdst_null(), tsrc_from(header), desc, GEN6_SFID_SPAWNER);
}

static bool
cs_setup(struct cs_compile_context *ccc,
         const struct ilo_shader_state *state,
         const struct ilo_shader_variant *variant)
{
   memset(ccc, 0, sizeof(*ccc));

   ccc->shader = CALLOC_STRUCT(ilo_shader);
   if (!ccc->shader)
      return false;

   ccc->variant = variant;

   toy_compiler_init(&ccc->tc, state->info.dev);

   ccc->tc.templ.access_mode = GEN6_ALIGN_1;
   ccc->tc.templ.qtr_ctrl = GEN6_QTRCTRL_1H;
   ccc->tc.templ.exec_size = GEN6_EXECSIZE_16;
   ccc->tc.rect_linear_width = 8;

   ccc->first_free_grf = 1;
   ccc->last_free_grf = 127;

   /* m0 is reserved for system routines */
   ccc->first_free_mrf = 1;
   ccc->last_free_mrf = 15;

   /* instructions are compressed with GEN6_EXECSIZE_16 */
   ccc->num_grf_per_vrf = 2;

   if (ilo_dev_gen(ccc->tc.dev) >= ILO_GEN(7)) {
      ccc->last_free_grf -= 15;
      ccc->first_free_mrf = ccc->last_free_grf + 1;
      ccc->last_free_mrf = ccc->first_free_mrf + 14;
   }

   ccc->shader->in.start_grf = 1;
   ccc->shader->dispatch_16 = true;

   /* INPUT */
   ccc->shader->bt.const_base = 0;
   ccc->shader->bt.const_count = 1;

   /* a GLOBAL */
   ccc->shader->bt.global_base = 1;
   ccc->shader->bt.global_count = 1;

   ccc->shader->bt.total_count = 2;

   return true;
}

/**
 * Compile the compute shader.
 */
struct ilo_shader *
ilo_shader_compile_cs(const struct ilo_shader_state *state,
                      const struct ilo_shader_variant *variant)
{
   struct cs_compile_context ccc;

   ILO_DEV_ASSERT(state->info.dev, 7, 7.5);

   if (!cs_setup(&ccc, state, variant))
      return NULL;

   cs_dummy(&ccc);

   if (!cs_compile(&ccc)) {
      FREE(ccc.shader);
      ccc.shader = NULL;
   }

   toy_compiler_cleanup(&ccc.tc);

   return ccc.shader;
}
