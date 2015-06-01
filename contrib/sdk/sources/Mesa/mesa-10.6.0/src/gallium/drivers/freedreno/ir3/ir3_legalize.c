/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "pipe/p_shader_tokens.h"
#include "util/u_math.h"

#include "freedreno_util.h"

#include "ir3.h"

/*
 * Legalize:
 *
 * We currently require that scheduling ensures that we have enough nop's
 * in all the right places.  The legalize step mostly handles fixing up
 * instruction flags ((ss)/(sy)/(ei)), and collapses sequences of nop's
 * into fewer nop's w/ rpt flag.
 */

struct ir3_legalize_ctx {
	struct ir3_block *block;
	bool has_samp;
	int max_bary;
};

static void legalize(struct ir3_legalize_ctx *ctx)
{
	struct ir3_block *block = ctx->block;
	struct ir3_instruction *n;
	struct ir3 *shader = block->shader;
	struct ir3_instruction *end =
			ir3_instr_create(block, 0, OPC_END);
	struct ir3_instruction *last_input = NULL;
	struct ir3_instruction *last_rel = NULL;
	regmask_t needs_ss_war;       /* write after read */
	regmask_t needs_ss;
	regmask_t needs_sy;

	regmask_init(&needs_ss_war);
	regmask_init(&needs_ss);
	regmask_init(&needs_sy);

	shader->instrs_count = 0;

	for (n = block->head; n; n = n->next) {
		struct ir3_register *reg;
		unsigned i;

		if (is_meta(n))
			continue;

		if (is_input(n)) {
			struct ir3_register *inloc = n->regs[1];
			assert(inloc->flags & IR3_REG_IMMED);
			ctx->max_bary = MAX2(ctx->max_bary, inloc->iim_val);
		}

		/* NOTE: consider dst register too.. it could happen that
		 * texture sample instruction (for example) writes some
		 * components which are unused.  A subsequent instruction
		 * that writes the same register can race w/ the sam instr
		 * resulting in undefined results:
		 */
		for (i = 0; i < n->regs_count; i++) {
			reg = n->regs[i];

			if (reg_gpr(reg)) {

				/* TODO: we probably only need (ss) for alu
				 * instr consuming sfu result.. need to make
				 * some tests for both this and (sy)..
				 */
				if (regmask_get(&needs_ss, reg)) {
					n->flags |= IR3_INSTR_SS;
					regmask_init(&needs_ss);
				}

				if (regmask_get(&needs_sy, reg)) {
					n->flags |= IR3_INSTR_SY;
					regmask_init(&needs_sy);
				}
			}

			/* TODO: is it valid to have address reg loaded from a
			 * relative src (ie. mova a0, c<a0.x+4>)?  If so, the
			 * last_rel check below should be moved ahead of this:
			 */
			if (reg->flags & IR3_REG_RELATIV)
				last_rel = n;
		}

		if (n->regs_count > 0) {
			reg = n->regs[0];
			if (regmask_get(&needs_ss_war, reg)) {
				n->flags |= IR3_INSTR_SS;
				regmask_init(&needs_ss_war); // ??? I assume?
			}

			if (last_rel && (reg->num == regid(REG_A0, 0))) {
				last_rel->flags |= IR3_INSTR_UL;
				last_rel = NULL;
			}
		}

		/* cat5+ does not have an (ss) bit, if needed we need to
		 * insert a nop to carry the sync flag.  Would be kinda
		 * clever if we were aware of this during scheduling, but
		 * this should be a pretty rare case:
		 */
		if ((n->flags & IR3_INSTR_SS) && (n->category >= 5)) {
			struct ir3_instruction *nop;
			nop = ir3_instr_create(block, 0, OPC_NOP);
			nop->flags |= IR3_INSTR_SS;
			n->flags &= ~IR3_INSTR_SS;
		}

		/* need to be able to set (ss) on first instruction: */
		if ((shader->instrs_count == 0) && (n->category >= 5))
			ir3_instr_create(block, 0, OPC_NOP);

		if (is_nop(n) && shader->instrs_count) {
			struct ir3_instruction *last =
					shader->instrs[shader->instrs_count-1];
			if (is_nop(last) && (last->repeat < 5)) {
				last->repeat++;
				last->flags |= n->flags;
				continue;
			}
		}

		shader->instrs[shader->instrs_count++] = n;

		if (is_sfu(n))
			regmask_set(&needs_ss, n->regs[0]);

		if (is_tex(n)) {
			/* this ends up being the # of samp instructions.. but that
			 * is ok, everything else only cares whether it is zero or
			 * not.  We do this here, rather than when we encounter a
			 * SAMP decl, because (especially in binning pass shader)
			 * the samp instruction(s) could get eliminated if the
			 * result is not used.
			 */
			ctx->has_samp = true;
			regmask_set(&needs_sy, n->regs[0]);
		} else if (is_mem(n)) {
			regmask_set(&needs_sy, n->regs[0]);
		}

		/* both tex/sfu appear to not always immediately consume
		 * their src register(s):
		 */
		if (is_tex(n) || is_sfu(n) || is_mem(n)) {
			foreach_src(reg, n) {
				if (reg_gpr(reg))
					regmask_set(&needs_ss_war, reg);
			}
		}

		if (is_input(n))
			last_input = n;
	}

	if (last_input) {
		/* special hack.. if using ldlv to bypass interpolation,
		 * we need to insert a dummy bary.f on which we can set
		 * the (ei) flag:
		 */
		if (is_mem(last_input) && (last_input->opc == OPC_LDLV)) {
			int i, cnt;

			/* note that ir3_instr_create() inserts into
			 * shader->instrs[] and increments the count..
			 * so we need to bump up the cnt initially (to
			 * avoid it clobbering the last real instr) and
			 * restore it after.
			 */
			cnt = ++shader->instrs_count;

			/* inserting instructions would be a bit nicer if list.. */
			for (i = cnt - 2; i >= 0; i--) {
				if (shader->instrs[i] == last_input) {

					/* (ss)bary.f (ei)r63.x, 0, r0.x */
					last_input = ir3_instr_create(block, 2, OPC_BARY_F);
					last_input->flags |= IR3_INSTR_SS;
					ir3_reg_create(last_input, regid(63, 0), 0);
					ir3_reg_create(last_input, 0, IR3_REG_IMMED)->iim_val = 0;
					ir3_reg_create(last_input, regid(0, 0), 0);

					shader->instrs[i + 1] = last_input;

					break;
				}
				shader->instrs[i + 1] = shader->instrs[i];
			}

			shader->instrs_count = cnt;
		}
		last_input->regs[0]->flags |= IR3_REG_EI;
	}

	if (last_rel)
		last_rel->flags |= IR3_INSTR_UL;

	shader->instrs[shader->instrs_count++] = end;

	shader->instrs[0]->flags |= IR3_INSTR_SS | IR3_INSTR_SY;
}

void ir3_block_legalize(struct ir3_block *block,
		bool *has_samp, int *max_bary)
{
	struct ir3_legalize_ctx ctx = {
			.block = block,
			.max_bary = -1,
	};

	legalize(&ctx);

	*has_samp = ctx.has_samp;
	*max_bary = ctx.max_bary;
}
