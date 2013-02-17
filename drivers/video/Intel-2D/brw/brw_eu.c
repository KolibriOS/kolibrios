/*
 Copyright (C) Intel Corp.  2006.  All Rights Reserved.
 Intel funded Tungsten Graphics (http://www.tungstengraphics.com) to
 develop this 3D driver.

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice (including the
 next paragraph) shall be included in all copies or substantial
 portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 **********************************************************************/
 /*
  * Authors:
  *   Keith Whitwell <keith@tungstengraphics.com>
  */

#include "brw_eu.h"

#include <string.h>
#include <stdlib.h>

/* Returns the corresponding conditional mod for swapping src0 and
 * src1 in e.g. CMP.
 */
uint32_t
brw_swap_cmod(uint32_t cmod)
{
	switch (cmod) {
	case BRW_CONDITIONAL_Z:
	case BRW_CONDITIONAL_NZ:
		return cmod;
	case BRW_CONDITIONAL_G:
		return BRW_CONDITIONAL_LE;
	case BRW_CONDITIONAL_GE:
		return BRW_CONDITIONAL_L;
	case BRW_CONDITIONAL_L:
		return BRW_CONDITIONAL_GE;
	case BRW_CONDITIONAL_LE:
		return BRW_CONDITIONAL_G;
	default:
		return ~0;
	}
}

/* How does predicate control work when execution_size != 8?  Do I
 * need to test/set for 0xffff when execution_size is 16?
 */
void brw_set_predicate_control_flag_value( struct brw_compile *p, unsigned value )
{
	p->current->header.predicate_control = BRW_PREDICATE_NONE;

	if (value != 0xff) {
		if (value != p->flag_value) {
			brw_MOV(p, brw_flag_reg(), brw_imm_uw(value));
			p->flag_value = value;
		}

		p->current->header.predicate_control = BRW_PREDICATE_NORMAL;
	}
}

void brw_set_compression_control(struct brw_compile *p,
				 enum brw_compression compression_control)
{
	p->compressed = (compression_control == BRW_COMPRESSION_COMPRESSED);

	if (p->gen >= 060) {
		/* Since we don't use the 32-wide support in gen6, we translate
		 * the pre-gen6 compression control here.
		 */
		switch (compression_control) {
		case BRW_COMPRESSION_NONE:
			/* This is the "use the first set of bits of dmask/vmask/arf
			 * according to execsize" option.
			 */
			p->current->header.compression_control = GEN6_COMPRESSION_1Q;
			break;
		case BRW_COMPRESSION_2NDHALF:
			/* For 8-wide, this is "use the second set of 8 bits." */
			p->current->header.compression_control = GEN6_COMPRESSION_2Q;
			break;
		case BRW_COMPRESSION_COMPRESSED:
			/* For 16-wide instruction compression, use the first set of 16 bits
			 * since we don't do 32-wide dispatch.
			 */
			p->current->header.compression_control = GEN6_COMPRESSION_1H;
			break;
		default:
			assert(!"not reached");
			p->current->header.compression_control = GEN6_COMPRESSION_1H;
			break;
		}
	} else {
		p->current->header.compression_control = compression_control;
	}
}

void brw_push_insn_state( struct brw_compile *p )
{
	assert(p->current != &p->stack[BRW_EU_MAX_INSN_STACK-1]);
	memcpy(p->current+1, p->current, sizeof(struct brw_instruction));
	p->compressed_stack[p->current - p->stack] = p->compressed;
	p->current++;
}

void brw_pop_insn_state( struct brw_compile *p )
{
	assert(p->current != p->stack);
	p->current--;
	p->compressed = p->compressed_stack[p->current - p->stack];
}

void brw_compile_init(struct brw_compile *p, int gen, void *store)
{
	assert(gen);

	p->gen = gen;
	p->store = store;

	p->nr_insn = 0;
	p->current = p->stack;
	p->compressed = false;
	memset(p->current, 0, sizeof(p->current[0]));

	/* Some defaults?
	*/
	brw_set_mask_control(p, BRW_MASK_ENABLE); /* what does this do? */
	brw_set_saturate(p, 0);
	brw_set_compression_control(p, BRW_COMPRESSION_NONE);
	brw_set_predicate_control_flag_value(p, 0xff);

	p->if_stack_depth = 0;
	p->if_stack_array_size = 0;
	p->if_stack = NULL;
}
