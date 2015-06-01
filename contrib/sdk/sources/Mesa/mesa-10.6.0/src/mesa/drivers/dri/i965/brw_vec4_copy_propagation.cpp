/*
 * Copyright © 2011 Intel Corporation
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

/**
 * @file brw_vec4_copy_propagation.cpp
 *
 * Implements tracking of values copied between registers, and
 * optimizations based on that: copy propagation and constant
 * propagation.
 */

#include "brw_vec4.h"
#include "brw_cfg.h"
extern "C" {
#include "main/macros.h"
}

namespace brw {

struct copy_entry {
   src_reg *value[4];
   int saturatemask;
};

static bool
is_direct_copy(vec4_instruction *inst)
{
   return (inst->opcode == BRW_OPCODE_MOV &&
	   !inst->predicate &&
	   inst->dst.file == GRF &&
	   !inst->dst.reladdr &&
	   !inst->src[0].reladdr &&
	   (inst->dst.type == inst->src[0].type ||
            (inst->dst.type == BRW_REGISTER_TYPE_F &&
             inst->src[0].type == BRW_REGISTER_TYPE_VF)));
}

static bool
is_dominated_by_previous_instruction(vec4_instruction *inst)
{
   return (inst->opcode != BRW_OPCODE_DO &&
	   inst->opcode != BRW_OPCODE_WHILE &&
	   inst->opcode != BRW_OPCODE_ELSE &&
	   inst->opcode != BRW_OPCODE_ENDIF);
}

static bool
is_channel_updated(vec4_instruction *inst, src_reg *values[4], int ch)
{
   const src_reg *src = values[ch];

   /* consider GRF only */
   assert(inst->dst.file == GRF);
   if (!src || src->file != GRF)
      return false;

   return (src->in_range(inst->dst, inst->regs_written) &&
           inst->dst.writemask & (1 << BRW_GET_SWZ(src->swizzle, ch)));
}

static unsigned
swizzle_vf_imm(unsigned vf4, unsigned swizzle)
{
   union {
      unsigned vf4;
      uint8_t vf[4];
   } v = { vf4 }, ret;

   ret.vf[0] = v.vf[BRW_GET_SWZ(swizzle, 0)];
   ret.vf[1] = v.vf[BRW_GET_SWZ(swizzle, 1)];
   ret.vf[2] = v.vf[BRW_GET_SWZ(swizzle, 2)];
   ret.vf[3] = v.vf[BRW_GET_SWZ(swizzle, 3)];

   return ret.vf4;
}

static bool
is_logic_op(enum opcode opcode)
{
   return (opcode == BRW_OPCODE_AND ||
           opcode == BRW_OPCODE_OR  ||
           opcode == BRW_OPCODE_XOR ||
           opcode == BRW_OPCODE_NOT);
}

static bool
try_constant_propagate(const struct brw_device_info *devinfo,
                       vec4_instruction *inst,
                       int arg, struct copy_entry *entry)
{
   /* For constant propagation, we only handle the same constant
    * across all 4 channels.  Some day, we should handle the 8-bit
    * float vector format, which would let us constant propagate
    * vectors better.
    */
   src_reg value = *entry->value[0];
   for (int i = 1; i < 4; i++) {
      if (!value.equals(*entry->value[i]))
	 return false;
   }

   if (value.file != IMM)
      return false;

   if (value.type == BRW_REGISTER_TYPE_VF) {
      /* The result of bit-casting the component values of a vector float
       * cannot in general be represented as an immediate.
       */
      if (inst->src[arg].type != BRW_REGISTER_TYPE_F)
         return false;
   } else {
      value.type = inst->src[arg].type;
   }

   if (inst->src[arg].abs) {
      if ((devinfo->gen >= 8 && is_logic_op(inst->opcode)) ||
          !brw_abs_immediate(value.type, &value.fixed_hw_reg)) {
         return false;
      }
   }

   if (inst->src[arg].negate) {
      if ((devinfo->gen >= 8 && is_logic_op(inst->opcode)) ||
          !brw_negate_immediate(value.type, &value.fixed_hw_reg)) {
         return false;
      }
   }

   if (value.type == BRW_REGISTER_TYPE_VF)
      value.fixed_hw_reg.dw1.ud = swizzle_vf_imm(value.fixed_hw_reg.dw1.ud,
                                                 inst->src[arg].swizzle);

   switch (inst->opcode) {
   case BRW_OPCODE_MOV:
   case SHADER_OPCODE_BROADCAST:
      inst->src[arg] = value;
      return true;

   case SHADER_OPCODE_POW:
   case SHADER_OPCODE_INT_QUOTIENT:
   case SHADER_OPCODE_INT_REMAINDER:
      if (devinfo->gen < 8)
         break;
      /* fallthrough */
   case BRW_OPCODE_DP2:
   case BRW_OPCODE_DP3:
   case BRW_OPCODE_DP4:
   case BRW_OPCODE_DPH:
   case BRW_OPCODE_BFI1:
   case BRW_OPCODE_ASR:
   case BRW_OPCODE_SHL:
   case BRW_OPCODE_SHR:
   case BRW_OPCODE_SUBB:
      if (arg == 1) {
         inst->src[arg] = value;
         return true;
      }
      break;

   case BRW_OPCODE_MACH:
   case BRW_OPCODE_MUL:
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_OR:
   case BRW_OPCODE_AND:
   case BRW_OPCODE_XOR:
   case BRW_OPCODE_ADDC:
      if (arg == 1) {
	 inst->src[arg] = value;
	 return true;
      } else if (arg == 0 && inst->src[1].file != IMM) {
	 /* Fit this constant in by commuting the operands.  Exception: we
	  * can't do this for 32-bit integer MUL/MACH because it's asymmetric.
	  */
	 if ((inst->opcode == BRW_OPCODE_MUL ||
              inst->opcode == BRW_OPCODE_MACH) &&
	     (inst->src[1].type == BRW_REGISTER_TYPE_D ||
	      inst->src[1].type == BRW_REGISTER_TYPE_UD))
	    break;
	 inst->src[0] = inst->src[1];
	 inst->src[1] = value;
	 return true;
      }
      break;

   case BRW_OPCODE_CMP:
      if (arg == 1) {
	 inst->src[arg] = value;
	 return true;
      } else if (arg == 0 && inst->src[1].file != IMM) {
	 enum brw_conditional_mod new_cmod;

	 new_cmod = brw_swap_cmod(inst->conditional_mod);
	 if (new_cmod != BRW_CONDITIONAL_NONE) {
	    /* Fit this constant in by swapping the operands and
	     * flipping the test.
	     */
	    inst->src[0] = inst->src[1];
	    inst->src[1] = value;
	    inst->conditional_mod = new_cmod;
	    return true;
	 }
      }
      break;

   case BRW_OPCODE_SEL:
      if (arg == 1) {
	 inst->src[arg] = value;
	 return true;
      } else if (arg == 0 && inst->src[1].file != IMM) {
	 inst->src[0] = inst->src[1];
	 inst->src[1] = value;

	 /* If this was predicated, flipping operands means
	  * we also need to flip the predicate.
	  */
	 if (inst->conditional_mod == BRW_CONDITIONAL_NONE) {
	    inst->predicate_inverse = !inst->predicate_inverse;
	 }
	 return true;
      }
      break;

   default:
      break;
   }

   return false;
}

static bool
try_copy_propagate(const struct brw_device_info *devinfo,
                   vec4_instruction *inst,
                   int arg, struct copy_entry *entry)
{
   /* For constant propagation, we only handle the same constant
    * across all 4 channels.  Some day, we should handle the 8-bit
    * float vector format, which would let us constant propagate
    * vectors better.
    */
   src_reg value = *entry->value[0];
   for (int i = 1; i < 4; i++) {
      /* This is equals() except we don't care about the swizzle. */
      if (value.file != entry->value[i]->file ||
	  value.reg != entry->value[i]->reg ||
	  value.reg_offset != entry->value[i]->reg_offset ||
	  value.type != entry->value[i]->type ||
	  value.negate != entry->value[i]->negate ||
	  value.abs != entry->value[i]->abs) {
	 return false;
      }
   }

   /* Compute the swizzle of the original register by swizzling the
    * component loaded from each value according to the swizzle of
    * operand we're going to change.
    */
   int s[4];
   for (int i = 0; i < 4; i++) {
      s[i] = BRW_GET_SWZ(entry->value[i]->swizzle, i);
   }
   value.swizzle = brw_compose_swizzle(inst->src[arg].swizzle,
                                       BRW_SWIZZLE4(s[0], s[1], s[2], s[3]));

   if (value.file != UNIFORM &&
       value.file != GRF &&
       value.file != ATTR)
      return false;

   if (devinfo->gen >= 8 && (value.negate || value.abs) &&
       is_logic_op(inst->opcode)) {
      return false;
   }

   if (inst->src[arg].abs) {
      value.negate = false;
      value.abs = true;
   }
   if (inst->src[arg].negate)
      value.negate = !value.negate;

   bool has_source_modifiers = value.negate || value.abs;

   /* gen6 math and gen7+ SENDs from GRFs ignore source modifiers on
    * instructions.
    */
   if ((has_source_modifiers || value.file == UNIFORM ||
        value.swizzle != BRW_SWIZZLE_XYZW) && !inst->can_do_source_mods(devinfo))
      return false;

   if (has_source_modifiers && value.type != inst->src[arg].type)
      return false;

   if (has_source_modifiers &&
       inst->opcode == SHADER_OPCODE_GEN4_SCRATCH_WRITE)
      return false;

   if (inst->is_3src() && value.file == UNIFORM)
      return false;

   if (inst->is_send_from_grf())
      return false;

   /* we can't generally copy-propagate UD negations becuse we
    * end up accessing the resulting values as signed integers
    * instead. See also resolve_ud_negate().
    */
   if (value.negate &&
       value.type == BRW_REGISTER_TYPE_UD)
      return false;

   /* Don't report progress if this is a noop. */
   if (value.equals(inst->src[arg]))
      return false;

   const unsigned dst_saturate_mask = inst->dst.writemask &
      brw_apply_swizzle_to_mask(inst->src[arg].swizzle, entry->saturatemask);

   if (dst_saturate_mask) {
      /* We either saturate all or nothing. */
      if (dst_saturate_mask != inst->dst.writemask)
         return false;

      /* Limit saturate propagation only to SEL with src1 bounded within 0.0
       * and 1.0, otherwise skip copy propagate altogether.
       */
      switch(inst->opcode) {
      case BRW_OPCODE_SEL:
         if (arg != 0 ||
             inst->src[0].type != BRW_REGISTER_TYPE_F ||
             inst->src[1].file != IMM ||
             inst->src[1].type != BRW_REGISTER_TYPE_F ||
             inst->src[1].fixed_hw_reg.dw1.f < 0.0 ||
             inst->src[1].fixed_hw_reg.dw1.f > 1.0) {
            return false;
         }
         if (!inst->saturate)
            inst->saturate = true;
         break;
      default:
         return false;
      }
   }

   value.type = inst->src[arg].type;
   inst->src[arg] = value;
   return true;
}

bool
vec4_visitor::opt_copy_propagation(bool do_constant_prop)
{
   bool progress = false;
   struct copy_entry entries[alloc.total_size];

   memset(&entries, 0, sizeof(entries));

   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      /* This pass only works on basic blocks.  If there's flow
       * control, throw out all our information and start from
       * scratch.
       *
       * This should really be fixed by using a structure like in
       * src/glsl/opt_copy_propagation.cpp to track available copies.
       */
      if (!is_dominated_by_previous_instruction(inst)) {
	 memset(&entries, 0, sizeof(entries));
	 continue;
      }

      /* For each source arg, see if each component comes from a copy
       * from the same type file (IMM, GRF, UNIFORM), and try
       * optimizing out access to the copy result
       */
      for (int i = 2; i >= 0; i--) {
	 /* Copied values end up in GRFs, and we don't track reladdr
	  * accesses.
	  */
	 if (inst->src[i].file != GRF ||
	     inst->src[i].reladdr)
	    continue;

         /* We only handle single-register copies. */
         if (inst->regs_read(i) != 1)
            continue;

	 int reg = (alloc.offsets[inst->src[i].reg] +
		    inst->src[i].reg_offset);

	 /* Find the regs that each swizzle component came from.
	  */
         struct copy_entry entry;
         memset(&entry, 0, sizeof(copy_entry));
	 int c;
	 for (c = 0; c < 4; c++) {
            int channel = BRW_GET_SWZ(inst->src[i].swizzle, c);
            entry.value[c] = entries[reg].value[channel];

	    /* If there's no available copy for this channel, bail.
	     * We could be more aggressive here -- some channels might
	     * not get used based on the destination writemask.
	     */
	    if (!entry.value[c])
	       break;

            entry.saturatemask |=
               (entries[reg].saturatemask & (1 << channel) ? 1 : 0) << c;

	    /* We'll only be able to copy propagate if the sources are
	     * all from the same file -- there's no ability to swizzle
	     * 0 or 1 constants in with source registers like in i915.
	     */
	    if (c > 0 && entry.value[c - 1]->file != entry.value[c]->file)
	       break;
	 }

	 if (c != 4)
	    continue;

         if (do_constant_prop && try_constant_propagate(devinfo, inst, i, &entry))
            progress = true;

	 if (try_copy_propagate(devinfo, inst, i, &entry))
	    progress = true;
      }

      /* Track available source registers. */
      if (inst->dst.file == GRF) {
	 const int reg =
	    alloc.offsets[inst->dst.reg] + inst->dst.reg_offset;

	 /* Update our destination's current channel values.  For a direct copy,
	  * the value is the newly propagated source.  Otherwise, we don't know
	  * the new value, so clear it.
	  */
	 bool direct_copy = is_direct_copy(inst);
         entries[reg].saturatemask &= ~inst->dst.writemask;
	 for (int i = 0; i < 4; i++) {
	    if (inst->dst.writemask & (1 << i)) {
               entries[reg].value[i] = direct_copy ? &inst->src[0] : NULL;
               entries[reg].saturatemask |=
                  inst->saturate && direct_copy ? 1 << i : 0;
            }
	 }

	 /* Clear the records for any registers whose current value came from
	  * our destination's updated channels, as the two are no longer equal.
	  */
	 if (inst->dst.reladdr)
	    memset(&entries, 0, sizeof(entries));
	 else {
	    for (unsigned i = 0; i < alloc.total_size; i++) {
	       for (int j = 0; j < 4; j++) {
		  if (is_channel_updated(inst, entries[i].value, j)) {
		     entries[i].value[j] = NULL;
		     entries[i].saturatemask &= ~(1 << j);
                  }
	       }
	    }
	 }
      }
   }

   if (progress)
      invalidate_live_intervals();

   return progress;
}

} /* namespace brw */
