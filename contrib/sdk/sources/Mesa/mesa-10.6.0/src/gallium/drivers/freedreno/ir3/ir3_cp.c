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

#include "freedreno_util.h"

#include "ir3.h"

/*
 * Copy Propagate:
 */

/* is it a type preserving mov, with ok flags? */
static bool is_eligible_mov(struct ir3_instruction *instr, bool allow_flags)
{
	if (is_same_type_mov(instr)) {
		struct ir3_register *dst = instr->regs[0];
		struct ir3_register *src = instr->regs[1];
		struct ir3_instruction *src_instr = ssa(src);
		if (dst->flags & (IR3_REG_ADDR | IR3_REG_RELATIV))
			return false;
		if (src->flags & IR3_REG_RELATIV)
			return false;
		if (!allow_flags)
			if (src->flags & (IR3_REG_FABS | IR3_REG_FNEG |
					IR3_REG_SABS | IR3_REG_SNEG | IR3_REG_BNOT))
				return false;
		if (!src_instr)
			return false;
		/* TODO: remove this hack: */
		if (is_meta(src_instr) && (src_instr->opc == OPC_META_FO))
			return false;
		return true;
	}
	return false;
}

static unsigned cp_flags(unsigned flags)
{
	/* only considering these flags (at least for now): */
	flags &= (IR3_REG_CONST | IR3_REG_IMMED |
			IR3_REG_FNEG | IR3_REG_FABS |
			IR3_REG_SNEG | IR3_REG_SABS |
			IR3_REG_BNOT | IR3_REG_RELATIV);
	return flags;
}

static bool valid_flags(struct ir3_instruction *instr, unsigned n,
		unsigned flags)
{
	unsigned valid_flags;
	flags = cp_flags(flags);

	/* clear flags that are 'ok' */
	switch (instr->category) {
	case 1:
		valid_flags = IR3_REG_IMMED | IR3_REG_RELATIV;
		if (flags & ~valid_flags)
			return false;
		break;
	case 5:
		/* no flags allowed */
		if (flags)
			return false;
		break;
	case 6:
		valid_flags = IR3_REG_IMMED;
		if (flags & ~valid_flags)
			return false;
		break;
	case 2:
		valid_flags = ir3_cat2_absneg(instr->opc) |
				IR3_REG_CONST | IR3_REG_RELATIV;

		if (ir3_cat2_int(instr->opc))
			valid_flags |= IR3_REG_IMMED;

		if (flags & ~valid_flags)
			return false;

		if (flags & (IR3_REG_CONST | IR3_REG_IMMED)) {
			unsigned m = (n ^ 1) + 1;
			/* cannot deal w/ const in both srcs:
			 * (note that some cat2 actually only have a single src)
			 */
			if (m < instr->regs_count) {
				struct ir3_register *reg = instr->regs[m];
				if ((flags & IR3_REG_CONST) && (reg->flags & IR3_REG_CONST))
					return false;
				if ((flags & IR3_REG_IMMED) && (reg->flags & IR3_REG_IMMED))
					return false;
			}
			/* cannot be const + ABS|NEG: */
			if (flags & (IR3_REG_FABS | IR3_REG_FNEG |
					IR3_REG_SABS | IR3_REG_SNEG | IR3_REG_BNOT))
				return false;
		}
		break;
	case 3:
		valid_flags = ir3_cat3_absneg(instr->opc) |
				IR3_REG_CONST | IR3_REG_RELATIV;

		if (flags & ~valid_flags)
			return false;

		if (flags & (IR3_REG_CONST | IR3_REG_RELATIV)) {
			/* cannot deal w/ const/relativ in 2nd src: */
			if (n == 1)
				return false;
		}

		if (flags & IR3_REG_CONST) {
			/* cannot be const + ABS|NEG: */
			if (flags & (IR3_REG_FABS | IR3_REG_FNEG |
					IR3_REG_SABS | IR3_REG_SNEG | IR3_REG_BNOT))
				return false;
		}
		break;
	case 4:
		/* seems like blob compiler avoids const as src.. */
		/* TODO double check if this is still the case on a4xx */
		if (flags & IR3_REG_CONST)
			return false;
		if (flags & (IR3_REG_SABS | IR3_REG_SNEG))
			return false;
		break;
	}

	return true;
}

/* propagate register flags from src to dst.. negates need special
 * handling to cancel each other out.
 */
static void combine_flags(unsigned *dstflags, unsigned srcflags)
{
	/* if what we are combining into already has (abs) flags,
	 * we can drop (neg) from src:
	 */
	if (*dstflags & IR3_REG_FABS)
		srcflags &= ~IR3_REG_FNEG;
	if (*dstflags & IR3_REG_SABS)
		srcflags &= ~IR3_REG_SNEG;

	if (srcflags & IR3_REG_FABS)
		*dstflags |= IR3_REG_FABS;
	if (srcflags & IR3_REG_SABS)
		*dstflags |= IR3_REG_SABS;
	if (srcflags & IR3_REG_FNEG)
		*dstflags ^= IR3_REG_FNEG;
	if (srcflags & IR3_REG_SNEG)
		*dstflags ^= IR3_REG_SNEG;
	if (srcflags & IR3_REG_BNOT)
		*dstflags ^= IR3_REG_BNOT;
}

static struct ir3_instruction * instr_cp(struct ir3_instruction *instr, unsigned *flags);

/* the "plain" MAD's (ie. the ones that don't shift first src prior to
 * multiply) can swap their first two srcs if src[0] is !CONST and
 * src[1] is CONST:
 */
static bool is_valid_mad(struct ir3_instruction *instr)
{
	return (instr->category == 3) && is_mad(instr->opc);
}

/**
 * Handle cp for a given src register.  This additionally handles
 * the cases of collapsing immedate/const (which replace the src
 * register with a non-ssa src) or collapsing mov's from relative
 * src (which needs to also fixup the address src reference by the
 * instruction).
 */
static void
reg_cp(struct ir3_instruction *instr, struct ir3_register *reg, unsigned n)
{
	unsigned src_flags = 0, new_flags;
	struct ir3_instruction *src_instr;

	if (is_meta(instr)) {
		/* meta instructions cannot fold up register
		 * flags.. they are usually src for texture
		 * fetch, etc, where we cannot specify abs/neg
		 */
		reg->instr = instr_cp(reg->instr, NULL);
		return;
	}

	src_instr = instr_cp(reg->instr, &src_flags);

	new_flags = reg->flags;
	combine_flags(&new_flags, src_flags);

	reg->flags = new_flags;
	reg->instr = src_instr;

	if (!valid_flags(instr, n, reg->flags)) {
		/* insert an absneg.f */
		if (reg->flags & (IR3_REG_SNEG | IR3_REG_SABS | IR3_REG_BNOT)) {
			debug_assert(!(reg->flags & (IR3_REG_FNEG | IR3_REG_FABS)));
			reg->instr = ir3_ABSNEG_S(instr->block,
					reg->instr, cp_flags(src_flags));
		} else {
			debug_assert(!(reg->flags & (IR3_REG_SNEG | IR3_REG_SABS | IR3_REG_BNOT)));
			reg->instr = ir3_ABSNEG_F(instr->block,
					reg->instr, cp_flags(src_flags));
		}
		reg->flags &= ~cp_flags(src_flags);
		debug_assert(valid_flags(instr, n, reg->flags));
		/* send it through instr_cp() again since
		 * the absneg src might be a mov from const
		 * that could be cleaned up:
		 */
		reg->instr = instr_cp(reg->instr, NULL);
		return;
	}

	if (is_same_type_mov(reg->instr)) {
		struct ir3_register *src_reg = reg->instr->regs[1];
		unsigned new_flags = src_reg->flags;

		combine_flags(&new_flags, reg->flags);

		if (!valid_flags(instr, n, new_flags)) {
			/* special case for "normal" mad instructions, we can
			 * try swapping the first two args if that fits better.
			 */
			if ((n == 1) && is_valid_mad(instr) &&
					!(instr->regs[0 + 1]->flags & (IR3_REG_CONST | IR3_REG_RELATIV)) &&
					valid_flags(instr, 0, new_flags)) {
				/* swap src[0] and src[1]: */
				struct ir3_register *tmp;
				tmp = instr->regs[0 + 1];
				instr->regs[0 + 1] = instr->regs[1 + 1];
				instr->regs[1 + 1] = tmp;
				n = 0;
			} else {
				return;
			}
		}

		/* Here we handle the special case of mov from
		 * CONST and/or RELATIV.  These need to be handled
		 * specially, because in the case of move from CONST
		 * there is no src ir3_instruction so we need to
		 * replace the ir3_register.  And in the case of
		 * RELATIV we need to handle the address register
		 * dependency.
		 */
		if (src_reg->flags & IR3_REG_CONST) {
			/* an instruction cannot reference two different
			 * address registers:
			 */
			if ((src_reg->flags & IR3_REG_RELATIV) &&
					conflicts(instr->address, reg->instr->address))
				return;

			src_reg->flags = new_flags;
			instr->regs[n+1] = src_reg;

			if (src_reg->flags & IR3_REG_RELATIV)
				instr->address = reg->instr->address;

			return;
		}

		if ((src_reg->flags & IR3_REG_RELATIV) &&
				!conflicts(instr->address, reg->instr->address)) {
			src_reg->flags = new_flags;
			instr->regs[n+1] = src_reg;
			instr->address = reg->instr->address;

			return;
		}

		/* NOTE: seems we can only do immed integers, so don't
		 * need to care about float.  But we do need to handle
		 * abs/neg *before* checking that the immediate requires
		 * few enough bits to encode:
		 *
		 * TODO: do we need to do something to avoid accidentally
		 * catching a float immed?
		 */
		if (src_reg->flags & IR3_REG_IMMED) {
			int32_t iim_val = src_reg->iim_val;

			debug_assert((instr->category == 1) ||
					(instr->category == 6) ||
					((instr->category == 2) &&
						ir3_cat2_int(instr->opc)));

			if (new_flags & IR3_REG_SABS)
				iim_val = abs(iim_val);

			if (new_flags & IR3_REG_SNEG)
				iim_val = -iim_val;

			if (new_flags & IR3_REG_BNOT)
				iim_val = ~iim_val;

			if (!(iim_val & ~0x3ff)) {
				new_flags &= ~(IR3_REG_SABS | IR3_REG_SNEG | IR3_REG_BNOT);
				src_reg->flags = new_flags;
				src_reg->iim_val = iim_val;
				instr->regs[n+1] = src_reg;
			}

			return;
		}
	}
}

/**
 * Given an SSA src (instruction), return the one with extraneous
 * mov's removed, ie, for (to copy NIR syntax):
 *
 *   vec1 ssa1 = fadd <something>, <somethingelse>
 *   vec1 ssa2 = fabs ssa1
 *   vec1 ssa3 = fneg ssa1
 *
 * then calling instr_cp(ssa3, &flags) would return ssa1 with
 * (IR3_REG_ABS | IR3_REG_NEGATE) in flags.  If flags is NULL,
 * then disallow eliminating copies which would require flag
 * propagation (for example, we cannot propagate abs/neg into
 * an output).
 */
static struct ir3_instruction *
instr_cp(struct ir3_instruction *instr, unsigned *flags)
{
	struct ir3_register *reg;

	/* stay within the block.. don't try to operate across
	 * basic block boundaries or we'll have problems when
	 * dealing with multiple basic blocks:
	 */
	if (is_meta(instr) && (instr->opc == OPC_META_INPUT))
		return instr;

	if (is_eligible_mov(instr, !!flags)) {
		struct ir3_register *reg = instr->regs[1];
		struct ir3_instruction *src_instr = ssa(reg);
		if (flags)
			combine_flags(flags, reg->flags);
		return instr_cp(src_instr, flags);
	}

	/* Check termination condition before walking children (rather
	 * than before checking eligible-mov).  A mov instruction may
	 * appear as ssa-src for multiple other instructions, and we
	 * want to consider it for removal for each, rather than just
	 * the first one.  (But regardless of how many places it shows
	 * up as a src, we only need to recursively walk the children
	 * once.)
	 */
	if (ir3_instr_check_mark(instr))
		return instr;

	/* walk down the graph from each src: */
	foreach_src_n(reg, n, instr) {
		if (!(reg->flags & IR3_REG_SSA))
			continue;

		reg_cp(instr, reg, n);
	}

	if (instr->address)
		instr->address = instr_cp(instr->address, NULL);

	return instr;
}

static void block_cp(struct ir3_block *block)
{
	unsigned i;

	for (i = 0; i < block->noutputs; i++) {
		if (block->outputs[i]) {
			struct ir3_instruction *out =
					instr_cp(block->outputs[i], NULL);

			block->outputs[i] = out;
		}
	}
}

void ir3_block_cp(struct ir3_block *block)
{
	ir3_clear_mark(block->shader);
	block_cp(block);
}
