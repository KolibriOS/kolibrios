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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#include "brw_cfg.h"
#include "brw_vec4_live_variables.h"

using namespace brw;

/** @file brw_vec4_live_variables.cpp
 *
 * Support for computing at the basic block level which variables
 * (virtual GRFs in our case) are live at entry and exit.
 *
 * See Muchnick's Advanced Compiler Design and Implementation, section
 * 14.1 (p444).
 */

/**
 * Sets up the use[] and def[] arrays.
 *
 * The basic-block-level live variable analysis needs to know which
 * variables get used before they're completely defined, and which
 * variables are completely defined before they're used.
 *
 * We independently track each channel of a vec4.  This is because we need to
 * be able to recognize a sequence like:
 *
 * ...
 * DP4 tmp.x a b;
 * DP4 tmp.y c d;
 * MUL result.xy tmp.xy e.xy
 * ...
 *
 * as having tmp live only across that sequence (assuming it's used nowhere
 * else), because it's a common pattern.  A more conservative approach that
 * doesn't get tmp marked a deffed in this block will tend to result in
 * spilling.
 */
void
vec4_live_variables::setup_def_use()
{
   int ip = 0;

   foreach_block (block, cfg) {
      assert(ip == block->start_ip);
      if (block->num > 0)
	 assert(cfg->blocks[block->num - 1]->end_ip == ip - 1);

      foreach_inst_in_block(vec4_instruction, inst, block) {
         struct block_data *bd = &block_data[block->num];

	 /* Set use[] for this instruction */
	 for (unsigned int i = 0; i < 3; i++) {
	    if (inst->src[i].file == GRF) {
               for (unsigned j = 0; j < inst->regs_read(i); j++) {
                  for (int c = 0; c < 4; c++) {
                     const unsigned v =
                        var_from_reg(alloc, offset(inst->src[i], j), c);
                     if (!BITSET_TEST(bd->def, v))
                        BITSET_SET(bd->use, v);
                  }
               }
	    }
	 }
         if (inst->reads_flag()) {
            if (!BITSET_TEST(bd->flag_def, 0)) {
               BITSET_SET(bd->flag_use, 0);
            }
         }

	 /* Check for unconditional writes to whole registers. These
	  * are the things that screen off preceding definitions of a
	  * variable, and thus qualify for being in def[].
	  */
	 if (inst->dst.file == GRF && !inst->predicate) {
            for (unsigned i = 0; i < inst->regs_written; i++) {
               for (int c = 0; c < 4; c++) {
                  if (inst->dst.writemask & (1 << c)) {
                     const unsigned v =
                        var_from_reg(alloc, offset(inst->dst, i), c);
                     if (!BITSET_TEST(bd->use, v))
                        BITSET_SET(bd->def, v);
                  }
               }
            }
         }
         if (inst->writes_flag()) {
            if (!BITSET_TEST(bd->flag_use, 0)) {
               BITSET_SET(bd->flag_def, 0);
            }
         }

	 ip++;
      }
   }
}

/**
 * The algorithm incrementally sets bits in liveout and livein,
 * propagating it through control flow.  It will eventually terminate
 * because it only ever adds bits, and stops when no bits are added in
 * a pass.
 */
void
vec4_live_variables::compute_live_variables()
{
   bool cont = true;

   while (cont) {
      cont = false;

      foreach_block (block, cfg) {
         struct block_data *bd = &block_data[block->num];

	 /* Update livein */
	 for (int i = 0; i < bitset_words; i++) {
            BITSET_WORD new_livein = (bd->use[i] |
                                      (bd->liveout[i] &
                                       ~bd->def[i]));
            if (new_livein & ~bd->livein[i]) {
               bd->livein[i] |= new_livein;
               cont = true;
	    }
	 }
         BITSET_WORD new_livein = (bd->flag_use[0] |
                                   (bd->flag_liveout[0] &
                                    ~bd->flag_def[0]));
         if (new_livein & ~bd->flag_livein[0]) {
            bd->flag_livein[0] |= new_livein;
            cont = true;
         }

	 /* Update liveout */
	 foreach_list_typed(bblock_link, child_link, link, &block->children) {
            struct block_data *child_bd = &block_data[child_link->block->num];

	    for (int i = 0; i < bitset_words; i++) {
               BITSET_WORD new_liveout = (child_bd->livein[i] &
                                          ~bd->liveout[i]);
               if (new_liveout) {
                  bd->liveout[i] |= new_liveout;
		  cont = true;
	       }
	    }
            BITSET_WORD new_liveout = (child_bd->flag_livein[0] &
                                       ~bd->flag_liveout[0]);
            if (new_liveout) {
               bd->flag_liveout[0] |= new_liveout;
               cont = true;
            }
	 }
      }
   }
}

vec4_live_variables::vec4_live_variables(const simple_allocator &alloc,
                                         cfg_t *cfg)
   : alloc(alloc), cfg(cfg)
{
   mem_ctx = ralloc_context(NULL);

   num_vars = alloc.total_size * 4;
   block_data = rzalloc_array(mem_ctx, struct block_data, cfg->num_blocks);

   bitset_words = BITSET_WORDS(num_vars);
   for (int i = 0; i < cfg->num_blocks; i++) {
      block_data[i].def = rzalloc_array(mem_ctx, BITSET_WORD, bitset_words);
      block_data[i].use = rzalloc_array(mem_ctx, BITSET_WORD, bitset_words);
      block_data[i].livein = rzalloc_array(mem_ctx, BITSET_WORD, bitset_words);
      block_data[i].liveout = rzalloc_array(mem_ctx, BITSET_WORD, bitset_words);

      block_data[i].flag_def[0] = 0;
      block_data[i].flag_use[0] = 0;
      block_data[i].flag_livein[0] = 0;
      block_data[i].flag_liveout[0] = 0;
   }

   setup_def_use();
   compute_live_variables();
}

vec4_live_variables::~vec4_live_variables()
{
   ralloc_free(mem_ctx);
}

#define MAX_INSTRUCTION (1 << 30)

/**
 * Computes a conservative start/end of the live intervals for each virtual GRF.
 *
 * We could expose per-channel live intervals to the consumer based on the
 * information we computed in vec4_live_variables, except that our only
 * current user is virtual_grf_interferes().  So we instead union the
 * per-channel ranges into a per-vgrf range for virtual_grf_start[] and
 * virtual_grf_end[].
 *
 * We could potentially have virtual_grf_interferes() do the test per-channel,
 * which would let some interesting register allocation occur (particularly on
 * code-generated GLSL sequences from the Cg compiler which does register
 * allocation at the GLSL level and thus reuses components of the variable
 * with distinct lifetimes).  But right now the complexity of doing so doesn't
 * seem worth it, since having virtual_grf_interferes() be cheap is important
 * for register allocation performance.
 */
void
vec4_visitor::calculate_live_intervals()
{
   if (this->live_intervals)
      return;

   int *start = ralloc_array(mem_ctx, int, this->alloc.total_size * 4);
   int *end = ralloc_array(mem_ctx, int, this->alloc.total_size * 4);
   ralloc_free(this->virtual_grf_start);
   ralloc_free(this->virtual_grf_end);
   this->virtual_grf_start = start;
   this->virtual_grf_end = end;

   for (unsigned i = 0; i < this->alloc.total_size * 4; i++) {
      start[i] = MAX_INSTRUCTION;
      end[i] = -1;
   }

   /* Start by setting up the intervals with no knowledge of control
    * flow.
    */
   int ip = 0;
   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      for (unsigned int i = 0; i < 3; i++) {
	 if (inst->src[i].file == GRF) {
            for (unsigned j = 0; j < inst->regs_read(i); j++) {
               for (int c = 0; c < 4; c++) {
                  const unsigned v =
                     var_from_reg(alloc, offset(inst->src[i], j), c);
                  start[v] = MIN2(start[v], ip);
                  end[v] = ip;
               }
            }
	 }
      }

      if (inst->dst.file == GRF) {
         for (unsigned i = 0; i < inst->regs_written; i++) {
            for (int c = 0; c < 4; c++) {
               if (inst->dst.writemask & (1 << c)) {
                  const unsigned v =
                     var_from_reg(alloc, offset(inst->dst, i), c);
                  start[v] = MIN2(start[v], ip);
                  end[v] = ip;
               }
            }
         }
      }

      ip++;
   }

   /* Now, extend those intervals using our analysis of control flow.
    *
    * The control flow-aware analysis was done at a channel level, while at
    * this point we're distilling it down to vgrfs.
    */
   this->live_intervals = new(mem_ctx) vec4_live_variables(alloc, cfg);

   foreach_block (block, cfg) {
      struct block_data *bd = &live_intervals->block_data[block->num];

      for (int i = 0; i < live_intervals->num_vars; i++) {
	 if (BITSET_TEST(bd->livein, i)) {
	    start[i] = MIN2(start[i], block->start_ip);
	    end[i] = MAX2(end[i], block->start_ip);
	 }

	 if (BITSET_TEST(bd->liveout, i)) {
	    start[i] = MIN2(start[i], block->end_ip);
	    end[i] = MAX2(end[i], block->end_ip);
	 }
      }
   }
}

void
vec4_visitor::invalidate_live_intervals()
{
   ralloc_free(live_intervals);
   live_intervals = NULL;
}

int
vec4_visitor::var_range_start(unsigned v, unsigned n) const
{
   int start = INT_MAX;

   for (unsigned i = 0; i < n; i++)
      start = MIN2(start, virtual_grf_start[v + i]);

   return start;
}

int
vec4_visitor::var_range_end(unsigned v, unsigned n) const
{
   int end = INT_MIN;

   for (unsigned i = 0; i < n; i++)
      end = MAX2(end, virtual_grf_end[v + i]);

   return end;
}

bool
vec4_visitor::virtual_grf_interferes(int a, int b)
{
   return !((var_range_end(4 * alloc.offsets[a], 4 * alloc.sizes[a]) <=
             var_range_start(4 * alloc.offsets[b], 4 * alloc.sizes[b])) ||
            (var_range_end(4 * alloc.offsets[b], 4 * alloc.sizes[b]) <=
             var_range_start(4 * alloc.offsets[a], 4 * alloc.sizes[a])));
}
