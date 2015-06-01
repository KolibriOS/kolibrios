/*
 * Copyright © 2012 Intel Corporation
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
#include "brw_cfg.h"

/** @file brw_fs_cse.cpp
 *
 * Support for local common subexpression elimination.
 *
 * See Muchnick's Advanced Compiler Design and Implementation, section
 * 13.1 (p378).
 */

namespace {
struct aeb_entry : public exec_node {
   /** The instruction that generates the expression value. */
   fs_inst *generator;

   /** The temporary where the value is stored. */
   fs_reg tmp;
};
}

static bool
is_expression(const fs_visitor *v, const fs_inst *const inst)
{
   switch (inst->opcode) {
   case BRW_OPCODE_MOV:
   case BRW_OPCODE_SEL:
   case BRW_OPCODE_NOT:
   case BRW_OPCODE_AND:
   case BRW_OPCODE_OR:
   case BRW_OPCODE_XOR:
   case BRW_OPCODE_SHR:
   case BRW_OPCODE_SHL:
   case BRW_OPCODE_ASR:
   case BRW_OPCODE_CMP:
   case BRW_OPCODE_CMPN:
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_MUL:
   case BRW_OPCODE_FRC:
   case BRW_OPCODE_RNDU:
   case BRW_OPCODE_RNDD:
   case BRW_OPCODE_RNDE:
   case BRW_OPCODE_RNDZ:
   case BRW_OPCODE_LINE:
   case BRW_OPCODE_PLN:
   case BRW_OPCODE_MAD:
   case BRW_OPCODE_LRP:
   case FS_OPCODE_UNIFORM_PULL_CONSTANT_LOAD:
   case FS_OPCODE_VARYING_PULL_CONSTANT_LOAD_GEN7:
   case FS_OPCODE_VARYING_PULL_CONSTANT_LOAD:
   case FS_OPCODE_CINTERP:
   case FS_OPCODE_LINTERP:
   case SHADER_OPCODE_FIND_LIVE_CHANNEL:
   case SHADER_OPCODE_BROADCAST:
      return true;
   case SHADER_OPCODE_RCP:
   case SHADER_OPCODE_RSQ:
   case SHADER_OPCODE_SQRT:
   case SHADER_OPCODE_EXP2:
   case SHADER_OPCODE_LOG2:
   case SHADER_OPCODE_POW:
   case SHADER_OPCODE_INT_QUOTIENT:
   case SHADER_OPCODE_INT_REMAINDER:
   case SHADER_OPCODE_SIN:
   case SHADER_OPCODE_COS:
      return inst->mlen < 2;
   case SHADER_OPCODE_LOAD_PAYLOAD:
      return !inst->is_copy_payload(v->alloc);
   default:
      return inst->is_send_from_grf() && !inst->has_side_effects();
   }
}

static bool
operands_match(const fs_inst *a, const fs_inst *b, bool *negate)
{
   fs_reg *xs = a->src;
   fs_reg *ys = b->src;

   if (a->opcode == BRW_OPCODE_MAD) {
      return xs[0].equals(ys[0]) &&
             ((xs[1].equals(ys[1]) && xs[2].equals(ys[2])) ||
              (xs[2].equals(ys[1]) && xs[1].equals(ys[2])));
   } else if (a->opcode == BRW_OPCODE_MUL && a->dst.type == BRW_REGISTER_TYPE_F) {
      bool xs0_negate = xs[0].negate;
      bool xs1_negate = xs[1].file == IMM ? xs[1].fixed_hw_reg.dw1.f < 0.0f
                                          : xs[1].negate;
      bool ys0_negate = ys[0].negate;
      bool ys1_negate = ys[1].file == IMM ? ys[1].fixed_hw_reg.dw1.f < 0.0f
                                          : ys[1].negate;
      float xs1_imm = xs[1].fixed_hw_reg.dw1.f;
      float ys1_imm = ys[1].fixed_hw_reg.dw1.f;

      xs[0].negate = false;
      xs[1].negate = false;
      ys[0].negate = false;
      ys[1].negate = false;
      xs[1].fixed_hw_reg.dw1.f = fabsf(xs[1].fixed_hw_reg.dw1.f);
      ys[1].fixed_hw_reg.dw1.f = fabsf(ys[1].fixed_hw_reg.dw1.f);

      bool ret = (xs[0].equals(ys[0]) && xs[1].equals(ys[1])) ||
                 (xs[1].equals(ys[0]) && xs[0].equals(ys[1]));

      xs[0].negate = xs0_negate;
      xs[1].negate = xs[1].file == IMM ? false : xs1_negate;
      ys[0].negate = ys0_negate;
      ys[1].negate = ys[1].file == IMM ? false : ys1_negate;
      xs[1].fixed_hw_reg.dw1.f = xs1_imm;
      ys[1].fixed_hw_reg.dw1.f = ys1_imm;

      *negate = (xs0_negate != xs1_negate) != (ys0_negate != ys1_negate);
      return ret;
   } else if (!a->is_commutative()) {
      bool match = true;
      for (int i = 0; i < a->sources; i++) {
         if (!xs[i].equals(ys[i])) {
            match = false;
            break;
         }
      }
      return match;
   } else {
      return (xs[0].equals(ys[0]) && xs[1].equals(ys[1])) ||
             (xs[1].equals(ys[0]) && xs[0].equals(ys[1]));
   }
}

static bool
instructions_match(fs_inst *a, fs_inst *b, bool *negate)
{
   return a->opcode == b->opcode &&
          a->saturate == b->saturate &&
          a->predicate == b->predicate &&
          a->predicate_inverse == b->predicate_inverse &&
          a->conditional_mod == b->conditional_mod &&
          a->dst.type == b->dst.type &&
          a->sources == b->sources &&
          (a->is_tex() ? (a->offset == b->offset &&
                          a->mlen == b->mlen &&
                          a->regs_written == b->regs_written &&
                          a->base_mrf == b->base_mrf &&
                          a->eot == b->eot &&
                          a->header_size == b->header_size &&
                          a->shadow_compare == b->shadow_compare)
                       : true) &&
          operands_match(a, b, negate);
}

static fs_inst *
create_copy_instr(fs_visitor *v, fs_inst *inst, fs_reg src, bool negate)
{
   int written = inst->regs_written;
   int dst_width = inst->dst.width / 8;
   fs_inst *copy;

   if (written > dst_width) {
      fs_reg *payload;
      int sources, header_size;
      if (inst->opcode == SHADER_OPCODE_LOAD_PAYLOAD) {
         sources = inst->sources;
         header_size = inst->header_size;
      } else {
         assert(written % dst_width == 0);
         sources = written / dst_width;
         header_size = 0;
      }

      assert(src.file == GRF);
      payload = ralloc_array(v->mem_ctx, fs_reg, sources);
      for (int i = 0; i < header_size; i++) {
         payload[i] = src;
         payload[i].width = 8;
         src.reg_offset++;
      }
      for (int i = header_size; i < sources; i++) {
         payload[i] = src;
         src = offset(src, 1);
      }
      copy = v->LOAD_PAYLOAD(inst->dst, payload, sources, header_size);
   } else {
      copy = v->MOV(inst->dst, src);
      copy->force_writemask_all = inst->force_writemask_all;
      copy->src[0].negate = negate;
   }
   assert(copy->regs_written == written);

   return copy;
}

bool
fs_visitor::opt_cse_local(bblock_t *block)
{
   bool progress = false;
   exec_list aeb;

   void *cse_ctx = ralloc_context(NULL);

   int ip = block->start_ip;
   foreach_inst_in_block(fs_inst, inst, block) {
      /* Skip some cases. */
      if (is_expression(this, inst) && !inst->is_partial_write() &&
          (inst->dst.file != HW_REG || inst->dst.is_null()))
      {
         bool found = false;
         bool negate = false;

         foreach_in_list_use_after(aeb_entry, entry, &aeb) {
            /* Match current instruction's expression against those in AEB. */
            if (!(entry->generator->dst.is_null() && !inst->dst.is_null()) &&
                instructions_match(inst, entry->generator, &negate)) {
               found = true;
               progress = true;
               break;
            }
         }

         if (!found) {
            if (inst->opcode != BRW_OPCODE_MOV ||
                (inst->opcode == BRW_OPCODE_MOV &&
                 inst->src[0].file == IMM &&
                 inst->src[0].type == BRW_REGISTER_TYPE_VF)) {
               /* Our first sighting of this expression.  Create an entry. */
               aeb_entry *entry = ralloc(cse_ctx, aeb_entry);
               entry->tmp = reg_undef;
               entry->generator = inst;
               aeb.push_tail(entry);
            }
         } else {
            /* This is at least our second sighting of this expression.
             * If we don't have a temporary already, make one.
             */
            bool no_existing_temp = entry->tmp.file == BAD_FILE;
            if (no_existing_temp && !entry->generator->dst.is_null()) {
               int written = entry->generator->regs_written;
               assert((written * 8) % entry->generator->dst.width == 0);

               entry->tmp = fs_reg(GRF, alloc.allocate(written),
                                   entry->generator->dst.type,
                                   entry->generator->dst.width);

               fs_inst *copy = create_copy_instr(this, entry->generator,
                                                 entry->tmp, false);
               entry->generator->insert_after(block, copy);

               entry->generator->dst = entry->tmp;
            }

            /* dest <- temp */
            if (!inst->dst.is_null()) {
               assert(inst->regs_written == entry->generator->regs_written);
               assert(inst->dst.width == entry->generator->dst.width);
               assert(inst->dst.type == entry->tmp.type);

               fs_inst *copy = create_copy_instr(this, inst,
                                                 entry->tmp, negate);
               inst->insert_before(block, copy);
            }

            /* Set our iterator so that next time through the loop inst->next
             * will get the instruction in the basic block after the one we've
             * removed.
             */
            fs_inst *prev = (fs_inst *)inst->prev;

            inst->remove(block);
            inst = prev;
         }
      }

      foreach_in_list_safe(aeb_entry, entry, &aeb) {
         /* Kill all AEB entries that write a different value to or read from
          * the flag register if we just wrote it.
          */
         if (inst->writes_flag()) {
            bool negate; /* dummy */
            if (entry->generator->reads_flag() ||
                (entry->generator->writes_flag() &&
                 !instructions_match(inst, entry->generator, &negate))) {
               entry->remove();
               ralloc_free(entry);
               continue;
            }
         }

         for (int i = 0; i < entry->generator->sources; i++) {
            fs_reg *src_reg = &entry->generator->src[i];

            /* Kill all AEB entries that use the destination we just
             * overwrote.
             */
            if (inst->overwrites_reg(entry->generator->src[i])) {
               entry->remove();
               ralloc_free(entry);
               break;
            }

            /* Kill any AEB entries using registers that don't get reused any
             * more -- a sure sign they'll fail operands_match().
             */
            if (src_reg->file == GRF && virtual_grf_end[src_reg->reg] < ip) {
               entry->remove();
               ralloc_free(entry);
               break;
            }
         }
      }

      ip++;
   }

   ralloc_free(cse_ctx);

   return progress;
}

bool
fs_visitor::opt_cse()
{
   bool progress = false;

   calculate_live_intervals();

   foreach_block (block, cfg) {
      progress = opt_cse_local(block) || progress;
   }

   if (progress)
      invalidate_live_intervals();

   return progress;
}
