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

#include "ir3.h"

/*
 * Register Assignment:
 *
 * NOTE: currently only works on a single basic block.. need to think
 * about how multiple basic blocks are going to get scheduled.  But
 * I think I want to re-arrange how blocks work, ie. get rid of the
 * block nesting thing..
 *
 * NOTE: we could do register coalescing (eliminate moves) as part of
 * the RA step.. OTOH I think we need to do scheduling before register
 * assignment.  And if we remove a mov that effects scheduling (unless
 * we leave a placeholder nop, which seems lame), so I'm not really
 * sure how practical this is to do both in a single stage.  But OTOH
 * I'm not really sure a sane way for the CP stage to realize when it
 * cannot remove a mov due to multi-register constraints..
 *
 * NOTE: http://scopesconf.org/scopes-01/paper/session1_2.ps.gz has
 * some ideas to handle array allocation with a more conventional
 * graph coloring algorithm for register assignment, which might be
 * a good alternative to the current algo.  However afaict it cannot
 * handle overlapping arrays, which is a scenario that we have to
 * deal with
 */

struct ir3_ra_ctx {
	struct ir3_block *block;
	enum shader_t type;
	bool frag_coord;
	bool frag_face;
	int cnt;
	bool error;
	struct {
		unsigned base;
		unsigned size;
	} arrays[MAX_ARRAYS];
};

#ifdef DEBUG
#  include "freedreno_util.h"
#  define ra_debug (fd_mesa_debug & FD_DBG_OPTMSGS)
#else
#  define ra_debug 0
#endif

#define ra_dump_list(msg, n) do { \
		if (ra_debug) { \
			debug_printf("-- " msg); \
			ir3_dump_instr_list(n); \
		} \
	} while (0)

#define ra_dump_instr(msg, n) do { \
		if (ra_debug) { \
			debug_printf(">> " msg); \
			ir3_dump_instr_single(n); \
		} \
	} while (0)

#define ra_assert(ctx, x) do { \
		debug_assert(x); \
		if (!(x)) { \
			debug_printf("RA: failed assert: %s\n", #x); \
			(ctx)->error = true; \
		}; \
	} while (0)


/* sorta ugly way to retrofit half-precision support.. rather than
 * passing extra param around, just OR in a high bit.  All the low
 * value arithmetic (ie. +/- offset within a contiguous vec4, etc)
 * will continue to work as long as you don't underflow (and that
 * would go badly anyways).
 */
#define REG_HALF  0x8000

#define REG(n, wm, f) (struct ir3_register){ \
		.flags  = (f), \
		.num    = (n), \
		.wrmask = TGSI_WRITEMASK_ ## wm, \
	}

/* check that the register exists, is a GPR and is not special (a0/p0) */
static struct ir3_register * reg_check(struct ir3_instruction *instr, unsigned n)
{
	if ((n < instr->regs_count) && reg_gpr(instr->regs[n]) &&
			!(instr->regs[n]->flags & IR3_REG_SSA))
		return instr->regs[n];
	return NULL;
}

/* figure out if an unassigned src register points back to the instr we
 * are assigning:
 */
static bool instr_used_by(struct ir3_instruction *instr,
		struct ir3_register *src)
{
	struct ir3_instruction *src_instr = ssa(src);
	unsigned i;
	if (instr == src_instr)
		return true;
	if (src_instr && is_meta(src_instr))
		for (i = 1; i < src_instr->regs_count; i++)
			if (instr_used_by(instr, src_instr->regs[i]))
				return true;

	return false;
}

static bool instr_is_output(struct ir3_instruction *instr)
{
	struct ir3_block *block = instr->block;
	unsigned i;

	for (i = 0; i < block->noutputs; i++)
		if (instr == block->outputs[i])
			return true;

	return false;
}

static void mark_sources(struct ir3_instruction *instr,
		struct ir3_instruction *n, regmask_t *liveregs, regmask_t *written)
{
	unsigned i;

	for (i = 1; i < n->regs_count; i++) {
		struct ir3_register *r = reg_check(n, i);
		if (r)
			regmask_set_if_not(liveregs, r, written);

		/* if any src points back to the instruction(s) in
		 * the block of neighbors that we are assigning then
		 * mark any written (clobbered) registers as live:
		 */
		if (instr_used_by(instr, n->regs[i]))
			regmask_or(liveregs, liveregs, written);
	}

}

/* live means read before written */
static void compute_liveregs(struct ir3_ra_ctx *ctx,
		struct ir3_instruction *instr, regmask_t *liveregs)
{
	struct ir3_block *block = instr->block;
	struct ir3_instruction *n;
	regmask_t written;
	unsigned i;

	regmask_init(&written);

	for (n = instr->next; n; n = n->next) {
		struct ir3_register *r;

		if (is_meta(n))
			continue;

		/* check first src's read: */
		mark_sources(instr, n, liveregs, &written);

		/* for instructions that write to an array, we need to
		 * capture the dependency on the array elements:
		 */
		if (n->fanin)
			mark_sources(instr, n->fanin, liveregs, &written);

		/* meta-instructions don't actually get scheduled,
		 * so don't let it's write confuse us.. what we
		 * really care about is when the src to the meta
		 * instr was written:
		 */
		if (is_meta(n))
			continue;

		/* then dst written (if assigned already): */
		r = reg_check(n, 0);
		if (r) {
			/* if an instruction *is* an output, then it is live */
			if (!instr_is_output(n))
				regmask_set(&written, r);
		}

	}

	/* be sure to account for output registers too: */
	for (i = 0; i < block->noutputs; i++) {
		struct ir3_register *r;
		if (!block->outputs[i])
			continue;
		r = reg_check(block->outputs[i], 0);
		if (r)
			regmask_set_if_not(liveregs, r, &written);
	}

	/* if instruction is output, we need a reg that isn't written
	 * before the end.. equiv to the instr_used_by() check above
	 * in the loop body
	 * TODO maybe should follow fanin/fanout?
	 */
	if (instr_is_output(instr))
		regmask_or(liveregs, liveregs, &written);
}

static int find_available(regmask_t *liveregs, int size, bool half)
{
	unsigned i;
	unsigned f = half ? IR3_REG_HALF : 0;
	for (i = 0; i < MAX_REG - size; i++) {
		if (!regmask_get(liveregs, &REG(i, X, f))) {
			unsigned start = i++;
			for (; (i < MAX_REG) && ((i - start) < size); i++)
				if (regmask_get(liveregs, &REG(i, X, f)))
					break;
			if ((i - start) >= size)
				return start;
		}
	}
	assert(0);
	return -1;
}

static int alloc_block(struct ir3_ra_ctx *ctx,
		struct ir3_instruction *instr, int size)
{
	struct ir3_register *dst = instr->regs[0];
	struct ir3_instruction *n;
	regmask_t liveregs;
	unsigned name;

	/* should only ever be called w/ head of neighbor list: */
	debug_assert(!instr->cp.left);

	regmask_init(&liveregs);

	for (n = instr; n; n = n->cp.right)
		compute_liveregs(ctx, n, &liveregs);

	/* because we do assignment on fanout nodes for wrmask!=0x1, we
	 * need to handle this special case, where the fanout nodes all
	 * appear after one or more of the consumers of the src node:
	 *
	 *   0098:009: sam _, r2.x
	 *   0028:010: mul.f r3.z, r4.x, c13.x
	 *   ; we start assigning here for '0098:009: sam'.. but
	 *   ; would miss the usage at '0028:010: mul.f'
	 *   0101:009: _meta:fo _, _[0098:009: sam], off=2
	 */
	if (is_meta(instr) && (instr->opc == OPC_META_FO))
		compute_liveregs(ctx, instr->regs[1]->instr, &liveregs);

	name = find_available(&liveregs, size,
			!!(dst->flags & IR3_REG_HALF));

	if (dst->flags & IR3_REG_HALF)
		name |= REG_HALF;

	return name;
}

static type_t half_type(type_t type)
{
	switch (type) {
	case TYPE_F32: return TYPE_F16;
	case TYPE_U32: return TYPE_U16;
	case TYPE_S32: return TYPE_S16;
	/* instructions may already be fixed up: */
	case TYPE_F16:
	case TYPE_U16:
	case TYPE_S16:
		return type;
	default:
		assert(0);
		return ~0;
	}
}

/* some instructions need fix-up if dst register is half precision: */
static void fixup_half_instr_dst(struct ir3_instruction *instr)
{
	switch (instr->category) {
	case 1: /* move instructions */
		instr->cat1.dst_type = half_type(instr->cat1.dst_type);
		break;
	case 3:
		switch (instr->opc) {
		case OPC_MAD_F32:
			instr->opc = OPC_MAD_F16;
			break;
		case OPC_SEL_B32:
			instr->opc = OPC_SEL_B16;
			break;
		case OPC_SEL_S32:
			instr->opc = OPC_SEL_S16;
			break;
		case OPC_SEL_F32:
			instr->opc = OPC_SEL_F16;
			break;
		case OPC_SAD_S32:
			instr->opc = OPC_SAD_S16;
			break;
		/* instructions may already be fixed up: */
		case OPC_MAD_F16:
		case OPC_SEL_B16:
		case OPC_SEL_S16:
		case OPC_SEL_F16:
		case OPC_SAD_S16:
			break;
		default:
			assert(0);
			break;
		}
		break;
	case 5:
		instr->cat5.type = half_type(instr->cat5.type);
		break;
	}
}
/* some instructions need fix-up if src register is half precision: */
static void fixup_half_instr_src(struct ir3_instruction *instr)
{
	switch (instr->category) {
	case 1: /* move instructions */
		instr->cat1.src_type = half_type(instr->cat1.src_type);
		break;
	}
}

static void reg_assign(struct ir3_instruction *instr,
		unsigned r, unsigned name)
{
	struct ir3_register *reg = instr->regs[r];

	reg->flags &= ~IR3_REG_SSA;
	reg->num = name & ~REG_HALF;

	if (name & REG_HALF) {
		reg->flags |= IR3_REG_HALF;
		/* if dst reg being assigned, patch up the instr: */
		if (reg == instr->regs[0])
			fixup_half_instr_dst(instr);
		else
			fixup_half_instr_src(instr);
	}
}

static void instr_assign(struct ir3_ra_ctx *ctx,
		struct ir3_instruction *instr, unsigned name);

static void instr_assign_src(struct ir3_ra_ctx *ctx,
		struct ir3_instruction *instr, unsigned r, unsigned name)
{
	struct ir3_register *reg = instr->regs[r];

	if (reg->flags & IR3_REG_RELATIV)
		name += reg->offset;

	reg_assign(instr, r, name);

	if (is_meta(instr)) {
		switch (instr->opc) {
		case OPC_META_INPUT:
			/* shader-input does not have a src, only block input: */
			debug_assert(instr->regs_count == 2);
			instr_assign(ctx, instr, name);
			return;
		case OPC_META_FO:
			instr_assign(ctx, instr, name + instr->fo.off);
			return;
		case OPC_META_FI:
			instr_assign(ctx, instr, name - (r - 1));
			return;
		default:
			break;
		}
	}
}

static void instr_assign_srcs(struct ir3_ra_ctx *ctx,
		struct ir3_instruction *instr, unsigned name)
{
	struct ir3_instruction *n, *src;

	for (n = instr->next; n && !ctx->error; n = n->next) {
		foreach_ssa_src_n(src, i, n) {
			unsigned r = i + 1;

			/* skip address / etc (non real sources): */
			if (r >= n->regs_count)
				continue;

			if (src == instr)
				instr_assign_src(ctx, n, r, name);
		}
	}
}

static void instr_assign(struct ir3_ra_ctx *ctx,
		struct ir3_instruction *instr, unsigned name)
{
	struct ir3_register *reg = instr->regs[0];

	if (reg->flags & IR3_REG_RELATIV)
		return;

	/* check if already assigned: */
	if (!(reg->flags & IR3_REG_SSA)) {
		/* ... and if so, sanity check: */
		ra_assert(ctx, reg->num == (name & ~REG_HALF));
		return;
	}

	/* rename this instructions dst register: */
	reg_assign(instr, 0, name);

	/* and rename any subsequent use of result of this instr: */
	instr_assign_srcs(ctx, instr, name);

	/* To simplify the neighbor logic, and to "avoid" dealing with
	 * instructions which write more than one output, we actually
	 * do register assignment for instructions that produce multiple
	 * outputs on the fanout nodes and propagate up the assignment
	 * to the actual instruction:
	 */
	if (is_meta(instr) && (instr->opc == OPC_META_FO)) {
		struct ir3_instruction *src;

		debug_assert(name >= instr->fo.off);

		foreach_ssa_src(src, instr)
			instr_assign(ctx, src, name - instr->fo.off);
	}
}

/* check neighbor list to see if it is already partially (or completely)
 * assigned, in which case register block is already allocated and we
 * just need to complete the assignment:
 */
static int check_partial_assignment(struct ir3_ra_ctx *ctx,
		struct ir3_instruction *instr)
{
	struct ir3_instruction *n;
	int off = 0;

	debug_assert(!instr->cp.left);

	for (n = instr; n; n = n->cp.right) {
		struct ir3_register *dst = n->regs[0];
		if ((n->depth != DEPTH_UNUSED) &&
				!(dst->flags & IR3_REG_SSA)) {
			int name = dst->num - off;
			debug_assert(name >= 0);
			return name;
		}
		off++;
	}

	return -1;
}

/* allocate register name(s) for a list of neighboring instructions;
 * instr should point to leftmost neighbor (head of list)
 */
static void instr_alloc_and_assign(struct ir3_ra_ctx *ctx,
		struct ir3_instruction *instr)
{
	struct ir3_instruction *n;
	struct ir3_register *dst;
	int name;

	debug_assert(!instr->cp.left);

	if (instr->regs_count == 0)
		return;

	dst = instr->regs[0];

	/* For indirect dst, take the register assignment from the
	 * fanin and propagate it forward.
	 */
	if (dst->flags & IR3_REG_RELATIV) {
		/* NOTE can be grouped, if for example outputs:
		 * for now disable cp if indirect writes
		 */
		instr_alloc_and_assign(ctx, instr->fanin);

		dst->num += instr->fanin->regs[0]->num;
		dst->flags &= ~IR3_REG_SSA;

		instr_assign_srcs(ctx, instr, instr->fanin->regs[0]->num);

		return;
	}

	/* for instructions w/ fanouts, do the actual register assignment
	 * on the group of fanout neighbor nodes and propagate the reg
	 * name back up to the texture instruction.
	 */
	if (dst->wrmask != 0x1)
		return;

	name = check_partial_assignment(ctx, instr);

	/* allocate register(s): */
	if (name >= 0) {
		/* already partially assigned, just finish the job */
	} else if (reg_gpr(dst)) {
		int size;
		/* number of consecutive registers to assign: */
		size = ir3_neighbor_count(instr);
		if (dst->wrmask != 0x1)
			size = MAX2(size, ffs(~dst->wrmask) - 1);
		name = alloc_block(ctx, instr, size);
	} else if (dst->flags & IR3_REG_ADDR) {
		debug_assert(!instr->cp.right);
		dst->flags &= ~IR3_REG_ADDR;
		name = regid(REG_A0, 0) | REG_HALF;
	} else {
		debug_assert(!instr->cp.right);
		/* predicate register (p0).. etc */
		name = regid(REG_P0, 0);
		debug_assert(dst->num == name);
	}

	ra_assert(ctx, name >= 0);

	for (n = instr; n && !ctx->error; n = n->cp.right) {
		instr_assign(ctx, n, name);
		name++;
	}
}

static void instr_assign_array(struct ir3_ra_ctx *ctx,
		struct ir3_instruction *instr)
{
	struct ir3_instruction *src;
	int name, aid = instr->fi.aid;

	if (ctx->arrays[aid].base == ~0) {
		int size = instr->regs_count - 1;
		ctx->arrays[aid].base = alloc_block(ctx, instr, size);
		ctx->arrays[aid].size = size;
	}

	name = ctx->arrays[aid].base;

	foreach_ssa_src_n(src, i, instr) {
		unsigned r = i + 1;

		/* skip address / etc (non real sources): */
		if (r >= instr->regs_count)
			break;

		instr_assign(ctx, src, name);
		name++;
	}

}

static int block_ra(struct ir3_ra_ctx *ctx, struct ir3_block *block)
{
	struct ir3_instruction *n;

	/* frag shader inputs get pre-assigned, since we have some
	 * constraints/unknowns about setup for some of these regs:
	 */
	if ((ctx->type == SHADER_FRAGMENT) && !block->parent) {
		unsigned i = 0, j;
		if (ctx->frag_face && (i < block->ninputs) && block->inputs[i]) {
			/* if we have frag_face, it gets hr0.x */
			instr_assign(ctx, block->inputs[i], REG_HALF | 0);
			i += 4;
		}
		for (j = 0; i < block->ninputs; i++, j++)
			if (block->inputs[i])
				instr_assign(ctx, block->inputs[i], j);
	}

	ra_dump_list("-------\n", block->head);

	/* first pass, assign arrays: */
	for (n = block->head; n && !ctx->error; n = n->next) {
		if (is_meta(n) && (n->opc == OPC_META_FI) && n->fi.aid) {
			debug_assert(!n->cp.left);  /* don't think this should happen */
			ra_dump_instr("ASSIGN ARRAY: ", n);
			instr_assign_array(ctx, n);
			ra_dump_list("-------\n", block->head);
		}
	}

	for (n = block->head; n && !ctx->error; n = n->next) {
		ra_dump_instr("ASSIGN: ", n);
		instr_alloc_and_assign(ctx, ir3_neighbor_first(n));
		ra_dump_list("-------\n", block->head);
	}

	return ctx->error ? -1 : 0;
}

int ir3_block_ra(struct ir3_block *block, enum shader_t type,
		bool frag_coord, bool frag_face)
{
	struct ir3_instruction *n;
	struct ir3_ra_ctx ctx = {
			.block = block,
			.type = type,
			.frag_coord = frag_coord,
			.frag_face = frag_face,
	};
	int ret;

	memset(&ctx.arrays, ~0, sizeof(ctx.arrays));

	/* mark dst registers w/ SSA flag so we can see which
	 * have been assigned so far:
	 * NOTE: we really should set SSA flag consistently on
	 * every dst register in the frontend.
	 */
	for (n = block->head; n; n = n->next)
		if (n->regs_count > 0)
			n->regs[0]->flags |= IR3_REG_SSA;

	ir3_clear_mark(block->shader);
	ret = block_ra(&ctx, block);

	return ret;
}
