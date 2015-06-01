/*
 * Copyright © 2014 Intel Corporation
 * Copyright © 2015 Red Hat
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
 *    Jason Ekstrand (jason@jlekstrand.net)
 *    Rob Clark (robclark@freedesktop.org)
 *
 */

#include "ir3_nir.h"
#include "glsl/nir/nir_builder.h"

/* Based on nir_opt_peephole_select, and hacked up to more aggressively
 * flatten anything that can be flattened
 *
 * This *might* be something that other drivers could use.  On the other
 * hand, I think most other hw has predicated instructions or similar
 * to select which side of if/else writes back result (and therefore
 * not having to assign unique registers to both sides of the if/else.
 * (And hopefully those drivers don't also have crazy scheduling reqs
 * and can more easily do this in their backend.)
 *
 * TODO eventually when we have proper flow control in the backend:
 *
 *  + Probably weight differently normal ALUs vs SFUs (cos/rcp/exp)
 *    since executing extra SFUs for the branch-not-taken path will
 *    generally be much more expensive.
 *
 *    Possibly what constitutes an ALU vs SFU differs between hw
 *    backends.. but that seems doubtful.
 *
 *  + Account for texture fetch and memory accesses (incl UBOs)
 *    since these will be more expensive..
 *
 *  + When if-condition is const (or uniform) or we have some way
 *    to know that all threads in the warp take the same branch
 *    then we should prefer to not flatten the if/else..
 */

struct lower_state {
	nir_builder b;
	void *mem_ctx;
	bool progress;
};

static bool
valid_dest(nir_block *block, nir_dest *dest)
{
	/* It must be SSA */
	if (!dest->is_ssa)
		return false;

	/* We only lower blocks that do not contain other blocks
	 * (so this is run iteratively in a loop).  Therefore if
	 * we get this far, it should not have any if_uses:
	 */
	assert(list_empty(&dest->ssa.if_uses));

	/* The only uses of this definition must be phi's in the
	 * successor or in the current block
	 */
	nir_foreach_use(&dest->ssa, use) {
		nir_instr *dest_instr = use->parent_instr;
		if (dest_instr->block == block)
			continue;
		if ((dest_instr->type == nir_instr_type_phi) &&
				(dest_instr->block == block->successors[0]))
			continue;
		return false;
	}

	return true;
}

static bool
block_check_for_allowed_instrs(nir_block *block)
{
	nir_foreach_instr(block, instr) {
		switch (instr->type) {
		case nir_instr_type_intrinsic: {
			nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
			const nir_intrinsic_info *info =
					&nir_intrinsic_infos[intr->intrinsic];

			switch (intr->intrinsic) {
			case nir_intrinsic_discard_if:
				/* to simplify things, we want discard_if src in ssa: */
				if (!intr->src[0].is_ssa)
					return false;
				/* fallthrough */
			case nir_intrinsic_discard:
				/* discard/discard_if can be reordered, but only
				 * with some special care
				 */
				break;
			case nir_intrinsic_store_output:
				/* TODO technically, if both if and else store
				 * the same output, we can hoist that out to
				 * the end of the block w/ a phi..
				 * In practice, the tgsi shaders we already get
				 * do this for us, so I think we don't need to
				 */
			default:
				if (!(info->flags & NIR_INTRINSIC_CAN_REORDER))
					return false;
			}

			break;
		}

		case nir_instr_type_tex: {
			nir_tex_instr *tex = nir_instr_as_tex(instr);
			if (!valid_dest(block, &tex->dest))
				return false;
			break;
		}
		case nir_instr_type_phi: {
			nir_phi_instr *phi = nir_instr_as_phi(instr);
			if (!valid_dest(block, &phi->dest))
				return false;
			break;
		}
		case nir_instr_type_alu: {
			nir_alu_instr *alu = nir_instr_as_alu(instr);
			if (!valid_dest(block, &alu->dest.dest))
				return false;
			break;
		}

		case nir_instr_type_load_const:
		case nir_instr_type_ssa_undef:
			break; /* always ssa dest */

		default:
			return false;
		}
	}

	return true;
}

/* flatten an then or else block: */
static void
flatten_block(nir_builder *bld, nir_block *if_block, nir_block *prev_block,
		nir_ssa_def *condition, bool invert)
{
	nir_foreach_instr_safe(if_block, instr) {
		if (instr->type == nir_instr_type_intrinsic) {
			nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
			if ((intr->intrinsic == nir_intrinsic_discard) ||
					(intr->intrinsic == nir_intrinsic_discard_if)) {
				nir_ssa_def *discard_cond;

				nir_builder_insert_after_instr(bld,
						nir_block_last_instr(prev_block));

				if (invert) {
					condition = nir_inot(bld, condition);
					invert = false;
				}

				if (intr->intrinsic == nir_intrinsic_discard) {
					discard_cond = condition;
				} else {
					assert(intr->src[0].is_ssa);
					/* discard_if gets re-written w/ src and'd: */
					discard_cond = nir_iand(bld, condition, intr->src[0].ssa);
				}

				nir_intrinsic_instr *discard_if =
						nir_intrinsic_instr_create(bld->shader,
								nir_intrinsic_discard_if);
				discard_if->src[0] = nir_src_for_ssa(discard_cond);

				nir_instr_insert_after(nir_block_last_instr(prev_block),
						&discard_if->instr);
				nir_instr_remove(instr);
				instr = NULL;
			}
		}
		/* if not an handled specially, just move to prev block: */
		if (instr) {
			/* NOTE: exec_node_remove() is safe here (vs nir_instr_remove()
			 * since we are re-adding the instructin back in to the prev
			 * block (so no dangling SSA uses)
			 */
			exec_node_remove(&instr->node);
			instr->block = prev_block;
			exec_list_push_tail(&prev_block->instr_list, &instr->node);
		}
	}
}

static bool
lower_if_else_block(nir_block *block, void *void_state)
{
	struct lower_state *state = void_state;

	/* If the block is empty, then it certainly doesn't have any phi nodes,
	 * so we can skip it.  This also ensures that we do an early skip on the
	 * end block of the function which isn't actually attached to the CFG.
	 */
	if (exec_list_is_empty(&block->instr_list))
		return true;

	if (nir_cf_node_is_first(&block->cf_node))
		return true;

	nir_cf_node *prev_node = nir_cf_node_prev(&block->cf_node);
	if (prev_node->type != nir_cf_node_if)
		return true;

	nir_if *if_stmt = nir_cf_node_as_if(prev_node);
	nir_cf_node *then_node = nir_if_first_then_node(if_stmt);
	nir_cf_node *else_node = nir_if_first_else_node(if_stmt);

	/* We can only have one block in each side ... */
	if (nir_if_last_then_node(if_stmt) != then_node ||
			nir_if_last_else_node(if_stmt) != else_node)
		return true;

	nir_block *then_block = nir_cf_node_as_block(then_node);
	nir_block *else_block = nir_cf_node_as_block(else_node);

	/* ... and those blocks must only contain "allowed" instructions. */
	if (!block_check_for_allowed_instrs(then_block) ||
			!block_check_for_allowed_instrs(else_block))
		return true;

	/* condition should be ssa too, which simplifies flatten_block: */
	if (!if_stmt->condition.is_ssa)
		return true;

	/* At this point, we know that the previous CFG node is an if-then
	 * statement containing only moves to phi nodes in this block.  We can
	 * just remove that entire CF node and replace all of the phi nodes with
	 * selects.
	 */

	nir_block *prev_block = nir_cf_node_as_block(nir_cf_node_prev(prev_node));
	assert(prev_block->cf_node.type == nir_cf_node_block);

	/* First, we move the remaining instructions from the blocks to the
	 * block before.  There are a few things that need handling specially
	 * like discard/discard_if.
	 */
	flatten_block(&state->b, then_block, prev_block,
			if_stmt->condition.ssa, false);
	flatten_block(&state->b, else_block, prev_block,
			if_stmt->condition.ssa, true);

	nir_foreach_instr_safe(block, instr) {
		if (instr->type != nir_instr_type_phi)
			break;

		nir_phi_instr *phi = nir_instr_as_phi(instr);
		nir_alu_instr *sel = nir_alu_instr_create(state->mem_ctx, nir_op_bcsel);
		nir_src_copy(&sel->src[0].src, &if_stmt->condition, state->mem_ctx);
		/* Splat the condition to all channels */
		memset(sel->src[0].swizzle, 0, sizeof sel->src[0].swizzle);

		assert(exec_list_length(&phi->srcs) == 2);
		nir_foreach_phi_src(phi, src) {
			assert(src->pred == then_block || src->pred == else_block);
			assert(src->src.is_ssa);

			unsigned idx = src->pred == then_block ? 1 : 2;
			nir_src_copy(&sel->src[idx].src, &src->src, state->mem_ctx);
		}

		nir_ssa_dest_init(&sel->instr, &sel->dest.dest,
				phi->dest.ssa.num_components, phi->dest.ssa.name);
		sel->dest.write_mask = (1 << phi->dest.ssa.num_components) - 1;

		nir_ssa_def_rewrite_uses(&phi->dest.ssa,
				nir_src_for_ssa(&sel->dest.dest.ssa),
				state->mem_ctx);

		nir_instr_insert_before(&phi->instr, &sel->instr);
		nir_instr_remove(&phi->instr);
	}

	nir_cf_node_remove(&if_stmt->cf_node);
	state->progress = true;

	return true;
}

static bool
lower_if_else_impl(nir_function_impl *impl)
{
	struct lower_state state;

	state.mem_ctx = ralloc_parent(impl);
	state.progress = false;
	nir_builder_init(&state.b, impl);

	nir_foreach_block(impl, lower_if_else_block, &state);

	if (state.progress)
		nir_metadata_preserve(impl, nir_metadata_none);

	return state.progress;
}

bool
ir3_nir_lower_if_else(nir_shader *shader)
{
	bool progress = false;

	nir_foreach_overload(shader, overload) {
		if (overload->impl)
			progress |= lower_if_else_impl(overload->impl);
	}

	return progress;
}
