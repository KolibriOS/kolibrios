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

/** @file brw_fs_combine_constants.cpp
 *
 * This file contains the opt_combine_constants() pass that runs after the
 * regular optimization loop. It passes over the instruction list and
 * selectively promotes immediate values to registers by emitting a mov(1)
 * instruction.
 *
 * This is useful on Gen 7 particularly, because a few instructions can be
 * coissued (i.e., issued in the same cycle as another thread on the same EU
 * issues an instruction) under some circumstances, one of which is that they
 * cannot use immediate values.
 */

#include "brw_fs.h"
#include "brw_fs_live_variables.h"
#include "brw_cfg.h"

/* Returns whether an instruction could co-issue if its immediate source were
 * replaced with a GRF source.
 */
static bool
could_coissue(const struct brw_device_info *devinfo, const fs_inst *inst)
{
   if (devinfo->gen != 7)
      return false;

   switch (inst->opcode) {
   case BRW_OPCODE_MOV:
   case BRW_OPCODE_CMP:
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_MUL:
      return true;
   default:
      return false;
   }
}

/**
 * Returns true for instructions that don't support immediate sources.
 */
static bool
must_promote_imm(const struct brw_device_info *devinfo, const fs_inst *inst)
{
   switch (inst->opcode) {
   case SHADER_OPCODE_POW:
      return devinfo->gen < 8;
   case BRW_OPCODE_MAD:
   case BRW_OPCODE_LRP:
      return true;
   default:
      return false;
   }
}

/** A box for putting fs_regs in a linked list. */
struct reg_link {
   DECLARE_RALLOC_CXX_OPERATORS(reg_link)

   reg_link(fs_reg *reg) : reg(reg) {}

   struct exec_node link;
   fs_reg *reg;
};

static struct exec_node *
link(void *mem_ctx, fs_reg *reg)
{
   reg_link *l = new(mem_ctx) reg_link(reg);
   return &l->link;
}

/**
 * Information about an immediate value.
 */
struct imm {
   /** The common ancestor of all blocks using this immediate value. */
   bblock_t *block;

   /**
    * The instruction generating the immediate value, if all uses are contained
    * within a single basic block. Otherwise, NULL.
    */
   fs_inst *inst;

   /**
    * A list of fs_regs that refer to this immediate.  If we promote it, we'll
    * have to patch these up to refer to the new GRF.
    */
   exec_list *uses;

   /** The immediate value.  We currently only handle floats. */
   float val;

   /**
    * The GRF register and subregister number where we've decided to store the
    * constant value.
    */
   uint8_t subreg_offset;
   uint16_t reg;

   /** The number of coissuable instructions using this immediate. */
   uint16_t uses_by_coissue;

   /**
    * Whether this constant is used by an instruction that can't handle an
    * immediate source (and already has to be promoted to a GRF).
    */
   bool must_promote;

   uint16_t first_use_ip;
   uint16_t last_use_ip;
};

/** The working set of information about immediates. */
struct table {
   struct imm *imm;
   int size;
   int len;
};

static struct imm *
find_imm(struct table *table, float val)
{
   assert(signbit(val) == 0);

   for (int i = 0; i < table->len; i++) {
      if (table->imm[i].val == val) {
         return &table->imm[i];
      }
   }
   return NULL;
}

static struct imm *
new_imm(struct table *table, void *mem_ctx)
{
   if (table->len == table->size) {
      table->size *= 2;
      table->imm = reralloc(mem_ctx, table->imm, struct imm, table->size);
   }
   return &table->imm[table->len++];
}

/**
 * Comparator used for sorting an array of imm structures.
 *
 * We sort by basic block number, then last use IP, then first use IP (least
 * to greatest). This sorting causes immediates live in the same area to be
 * allocated to the same register in the hopes that all values will be dead
 * about the same time and the register can be reused.
 */
static int
compare(const void *_a, const void *_b)
{
   const struct imm *a = (const struct imm *)_a,
                    *b = (const struct imm *)_b;

   int block_diff = a->block->num - b->block->num;
   if (block_diff)
      return block_diff;

   int end_diff = a->last_use_ip - b->last_use_ip;
   if (end_diff)
      return end_diff;

   return a->first_use_ip - b->first_use_ip;
}

bool
fs_visitor::opt_combine_constants()
{
   void *const_ctx = ralloc_context(NULL);

   struct table table;
   table.size = 8;
   table.len = 0;
   table.imm = ralloc_array(const_ctx, struct imm, table.size);

   cfg->calculate_idom();
   unsigned ip = -1;

   /* Make a pass through all instructions and count the number of times each
    * constant is used by coissueable instructions or instructions that cannot
    * take immediate arguments.
    */
   foreach_block_and_inst(block, fs_inst, inst, cfg) {
      ip++;

      if (!could_coissue(devinfo, inst) && !must_promote_imm(devinfo, inst))
         continue;

      for (int i = 0; i < inst->sources; i++) {
         if (inst->src[i].file != IMM ||
             inst->src[i].type != BRW_REGISTER_TYPE_F)
            continue;

         float val = fabsf(inst->src[i].fixed_hw_reg.dw1.f);
         struct imm *imm = find_imm(&table, val);

         if (imm) {
            bblock_t *intersection = cfg_t::intersect(block, imm->block);
            if (intersection != imm->block)
               imm->inst = NULL;
            imm->block = intersection;
            imm->uses->push_tail(link(const_ctx, &inst->src[i]));
            imm->uses_by_coissue += could_coissue(devinfo, inst);
            imm->must_promote = imm->must_promote || must_promote_imm(devinfo, inst);
            imm->last_use_ip = ip;
         } else {
            imm = new_imm(&table, const_ctx);
            imm->block = block;
            imm->inst = inst;
            imm->uses = new(const_ctx) exec_list();
            imm->uses->push_tail(link(const_ctx, &inst->src[i]));
            imm->val = val;
            imm->uses_by_coissue = could_coissue(devinfo, inst);
            imm->must_promote = must_promote_imm(devinfo, inst);
            imm->first_use_ip = ip;
            imm->last_use_ip = ip;
         }
      }
   }

   /* Remove constants from the table that don't have enough uses to make them
    * profitable to store in a register.
    */
   for (int i = 0; i < table.len;) {
      struct imm *imm = &table.imm[i];

      if (!imm->must_promote && imm->uses_by_coissue < 4) {
         table.imm[i] = table.imm[table.len - 1];
         table.len--;
         continue;
      }
      i++;
   }
   if (table.len == 0) {
      ralloc_free(const_ctx);
      return false;
   }
   if (cfg->num_blocks != 1)
      qsort(table.imm, table.len, sizeof(struct imm), compare);


   /* Insert MOVs to load the constant values into GRFs. */
   fs_reg reg(GRF, alloc.allocate(dispatch_width / 8));
   reg.stride = 0;
   for (int i = 0; i < table.len; i++) {
      struct imm *imm = &table.imm[i];

      fs_inst *mov = MOV(reg, fs_reg(imm->val));
      mov->force_writemask_all = true;
      if (imm->inst) {
         imm->inst->insert_before(imm->block, mov);
      } else {
         backend_instruction *inst = imm->block->last_non_control_flow_inst();
         inst->insert_after(imm->block, mov);
      }
      imm->reg = reg.reg;
      imm->subreg_offset = reg.subreg_offset;

      reg.subreg_offset += sizeof(float);
      if ((unsigned)reg.subreg_offset == dispatch_width * sizeof(float)) {
         reg.reg = alloc.allocate(dispatch_width / 8);
         reg.subreg_offset = 0;
      }
   }
   promoted_constants = table.len;

   /* Rewrite the immediate sources to refer to the new GRFs. */
   for (int i = 0; i < table.len; i++) {
      foreach_list_typed(reg_link, link, link, table.imm[i].uses) {
         fs_reg *reg = link->reg;
         reg->file = GRF;
         reg->reg = table.imm[i].reg;
         reg->subreg_offset = table.imm[i].subreg_offset;
         reg->stride = 0;
         reg->negate = signbit(reg->fixed_hw_reg.dw1.f) !=
                               signbit(table.imm[i].val);
         assert(fabsf(reg->fixed_hw_reg.dw1.f) == table.imm[i].val);
      }
   }

   ralloc_free(const_ctx);
   invalidate_live_intervals();

   return true;
}
