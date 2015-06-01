/*
 * Copyright © 2014 Intel Corporation
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

#include "brw_vec4.h"
#include "brw_vec4_live_variables.h"
#include "brw_cfg.h"

/** @file brw_vec4_dead_code_eliminate.cpp
 *
 * Dataflow-aware dead code elimination.
 *
 * Walks the instruction list from the bottom, removing instructions that
 * have results that both aren't used in later blocks and haven't been read
 * yet in the tail end of this block.
 */

using namespace brw;

static bool
can_do_writemask(const struct brw_device_info *devinfo,
                 const vec4_instruction *inst)
{
   switch (inst->opcode) {
   case SHADER_OPCODE_GEN4_SCRATCH_READ:
   case VS_OPCODE_PULL_CONSTANT_LOAD:
   case VS_OPCODE_PULL_CONSTANT_LOAD_GEN7:
   case VS_OPCODE_SET_SIMD4X2_HEADER_GEN9:
      return false;
   default:
      /* The MATH instruction on Gen6 only executes in align1 mode, which does
       * not support writemasking.
       */
      if (devinfo->gen == 6 && inst->is_math())
         return false;

      if (inst->is_tex())
         return false;

      return true;
   }
}

bool
vec4_visitor::dead_code_eliminate()
{
   bool progress = false;

   calculate_live_intervals();

   int num_vars = live_intervals->num_vars;
   BITSET_WORD *live = ralloc_array(NULL, BITSET_WORD, BITSET_WORDS(num_vars));
   BITSET_WORD *flag_live = ralloc_array(NULL, BITSET_WORD, 1);

   foreach_block(block, cfg) {
      memcpy(live, live_intervals->block_data[block->num].liveout,
             sizeof(BITSET_WORD) * BITSET_WORDS(num_vars));
      memcpy(flag_live, live_intervals->block_data[block->num].flag_liveout,
             sizeof(BITSET_WORD));

      foreach_inst_in_block_reverse(vec4_instruction, inst, block) {
         if (inst->dst.file == GRF && !inst->has_side_effects()) {
            bool result_live[4] = { false };

            for (unsigned i = 0; i < inst->regs_written; i++) {
               for (int c = 0; c < 4; c++)
                  result_live[c] |= BITSET_TEST(
                     live, var_from_reg(alloc, offset(inst->dst, i), c));
            }

            /* If the instruction can't do writemasking, then it's all or
             * nothing.
             */
            if (!can_do_writemask(devinfo, inst)) {
               bool result = result_live[0] | result_live[1] |
                             result_live[2] | result_live[3];
               result_live[0] = result;
               result_live[1] = result;
               result_live[2] = result;
               result_live[3] = result;
            }

            for (int c = 0; c < 4; c++) {
               if (!result_live[c] && inst->dst.writemask & (1 << c)) {
                  inst->dst.writemask &= ~(1 << c);
                  progress = true;

                  if (inst->dst.writemask == 0) {
                     if (inst->writes_accumulator || inst->writes_flag()) {
                        inst->dst = dst_reg(retype(brw_null_reg(), inst->dst.type));
                     } else {
                        inst->opcode = BRW_OPCODE_NOP;
                        continue;
                     }
                  }
               }
            }
         }

         if (inst->dst.is_null() && inst->writes_flag()) {
            if (!BITSET_TEST(flag_live, 0)) {
               inst->opcode = BRW_OPCODE_NOP;
               progress = true;
               continue;
            }
         }

         if (inst->dst.file == GRF && !inst->predicate) {
            for (unsigned i = 0; i < inst->regs_written; i++) {
               for (int c = 0; c < 4; c++) {
                  if (inst->dst.writemask & (1 << c)) {
                     BITSET_CLEAR(live, var_from_reg(alloc,
                                                     offset(inst->dst, i), c));
                  }
               }
            }
         }

         if (inst->writes_flag()) {
            BITSET_CLEAR(flag_live, 0);
         }

         for (int i = 0; i < 3; i++) {
            if (inst->src[i].file == GRF) {
               for (unsigned j = 0; j < inst->regs_read(i); j++) {
                  for (int c = 0; c < 4; c++) {
                     BITSET_SET(live, var_from_reg(alloc,
                                                   offset(inst->src[i], j), c));
                  }
               }
            }
         }

         if (inst->reads_flag()) {
            BITSET_SET(flag_live, 0);
         }
      }
   }

   ralloc_free(live);
   ralloc_free(flag_live);

   if (progress) {
      foreach_block_and_inst_safe(block, backend_instruction, inst, cfg) {
         if (inst->opcode == BRW_OPCODE_NOP) {
            inst->remove(block);
         }
      }

      invalidate_live_intervals();
   }

   return progress;
}
