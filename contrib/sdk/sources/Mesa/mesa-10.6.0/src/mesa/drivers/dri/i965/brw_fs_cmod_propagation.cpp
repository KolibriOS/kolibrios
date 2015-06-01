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

#include "brw_fs.h"
#include "brw_fs_live_variables.h"
#include "brw_cfg.h"

/** @file brw_fs_cmod_propagation.cpp
 *
 * Implements a pass that propagates the conditional modifier from a CMP x 0.0
 * instruction into the instruction that generated x. For instance, in this
 * sequence
 *
 *    add(8)          g70<1>F    g69<8,8,1>F    4096F
 *    cmp.ge.f0(8)    null       g70<8,8,1>F    0F
 *
 * we can do the comparison as part of the ADD instruction directly:
 *
 *    add.ge.f0(8)    g70<1>F    g69<8,8,1>F    4096F
 *
 * If there had been a use of the flag register and another CMP using g70
 *
 *    add.ge.f0(8)    g70<1>F    g69<8,8,1>F    4096F
 *    (+f0) sel(8)    g71<F>     g72<8,8,1>F    g73<8,8,1>F
 *    cmp.ge.f0(8)    null       g70<8,8,1>F    0F
 *
 * we can recognize that the CMP is generating the flag value that already
 * exists and therefore remove the instruction.
 */

static bool
opt_cmod_propagation_local(bblock_t *block)
{
   bool progress = false;
   int ip = block->end_ip + 1;

   foreach_inst_in_block_reverse_safe(fs_inst, inst, block) {
      ip--;

      if ((inst->opcode != BRW_OPCODE_AND &&
           inst->opcode != BRW_OPCODE_CMP &&
           inst->opcode != BRW_OPCODE_MOV) ||
          inst->predicate != BRW_PREDICATE_NONE ||
          !inst->dst.is_null() ||
          inst->src[0].file != GRF ||
          inst->src[0].abs)
         continue;

      /* Only an AND.NZ can be propagated.  Many AND.Z instructions are
       * generated (for ir_unop_not in fs_visitor::emit_bool_to_cond_code).
       * Propagating those would require inverting the condition on the CMP.
       * This changes both the flag value and the register destination of the
       * CMP.  That result may be used elsewhere, so we can't change its value
       * on a whim.
       */
      if (inst->opcode == BRW_OPCODE_AND &&
          !(inst->src[1].is_one() &&
            inst->conditional_mod == BRW_CONDITIONAL_NZ &&
            !inst->src[0].negate))
         continue;

      if (inst->opcode == BRW_OPCODE_CMP && !inst->src[1].is_zero())
         continue;

      if (inst->opcode == BRW_OPCODE_MOV &&
          inst->conditional_mod != BRW_CONDITIONAL_NZ)
         continue;

      bool read_flag = false;
      foreach_inst_in_block_reverse_starting_from(fs_inst, scan_inst, inst,
                                                  block) {
         if (scan_inst->overwrites_reg(inst->src[0])) {
            if (scan_inst->is_partial_write() ||
                scan_inst->dst.reg_offset != inst->src[0].reg_offset)
               break;

            /* CMP's result is the same regardless of dest type. */
            if (inst->conditional_mod == BRW_CONDITIONAL_NZ &&
                scan_inst->opcode == BRW_OPCODE_CMP &&
                (inst->dst.type == BRW_REGISTER_TYPE_D ||
                 inst->dst.type == BRW_REGISTER_TYPE_UD)) {
               inst->remove(block);
               progress = true;
               break;
            }

            /* If the AND wasn't handled by the previous case, it isn't safe
             * to remove it.
             */
            if (inst->opcode == BRW_OPCODE_AND)
               break;

            /* Comparisons operate differently for ints and floats */
            if (scan_inst->dst.type != inst->dst.type &&
                (scan_inst->dst.type == BRW_REGISTER_TYPE_F ||
                 inst->dst.type == BRW_REGISTER_TYPE_F))
               break;

            /* If the instruction generating inst's source also wrote the
             * flag, and inst is doing a simple .nz comparison, then inst
             * is redundant - the appropriate value is already in the flag
             * register.  Delete inst.
             */
            if (inst->conditional_mod == BRW_CONDITIONAL_NZ &&
                !inst->src[0].negate &&
                scan_inst->writes_flag()) {
               inst->remove(block);
               progress = true;
               break;
            }

            /* Otherwise, try propagating the conditional. */
            enum brw_conditional_mod cond =
               inst->src[0].negate ? brw_swap_cmod(inst->conditional_mod)
                                   : inst->conditional_mod;

            if (scan_inst->can_do_cmod() &&
                ((!read_flag && scan_inst->conditional_mod == BRW_CONDITIONAL_NONE) ||
                 scan_inst->conditional_mod == cond)) {
               scan_inst->conditional_mod = cond;
               inst->remove(block);
               progress = true;
            }
            break;
         }

         if (scan_inst->writes_flag())
            break;

         read_flag = read_flag || scan_inst->reads_flag();
      }
   }

   return progress;
}

bool
fs_visitor::opt_cmod_propagation()
{
   bool progress = false;

   foreach_block_reverse(block, cfg) {
      progress = opt_cmod_propagation_local(block) || progress;
   }

   if (progress)
      invalidate_live_intervals();

   return progress;
}
