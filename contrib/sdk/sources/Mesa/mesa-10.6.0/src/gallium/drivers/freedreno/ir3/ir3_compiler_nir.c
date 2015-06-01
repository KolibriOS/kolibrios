/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2015 Rob Clark <robclark@freedesktop.org>
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

#include <stdarg.h>

#include "pipe/p_state.h"
#include "util/u_string.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "tgsi/tgsi_lowering.h"
#include "tgsi/tgsi_strings.h"

#include "nir/tgsi_to_nir.h"
#include "glsl/shader_enums.h"

#include "freedreno_util.h"

#include "ir3_compiler.h"
#include "ir3_shader.h"
#include "ir3_nir.h"

#include "instr-a3xx.h"
#include "ir3.h"


static struct ir3_instruction * create_immed(struct ir3_block *block, uint32_t val);

struct ir3_compile {
	const struct tgsi_token *tokens;
	struct nir_shader *s;

	struct ir3 *ir;
	struct ir3_shader_variant *so;

	/* bitmask of which samplers are integer: */
	uint16_t integer_s;

	struct ir3_block *block;

	/* For fragment shaders, from the hw perspective the only
	 * actual input is r0.xy position register passed to bary.f.
	 * But TGSI doesn't know that, it still declares things as
	 * IN[] registers.  So we do all the input tracking normally
	 * and fix things up after compile_instructions()
	 *
	 * NOTE that frag_pos is the hardware position (possibly it
	 * is actually an index or tag or some such.. it is *not*
	 * values that can be directly used for gl_FragCoord..)
	 */
	struct ir3_instruction *frag_pos, *frag_face, *frag_coord[4];

	/* For vertex shaders, keep track of the system values sources */
	struct ir3_instruction *vertex_id, *basevertex, *instance_id;

	/* mapping from nir_register to defining instruction: */
	struct hash_table *def_ht;

	/* mapping from nir_variable to ir3_array: */
	struct hash_table *var_ht;
	unsigned num_arrays;

	/* a common pattern for indirect addressing is to request the
	 * same address register multiple times.  To avoid generating
	 * duplicate instruction sequences (which our backend does not
	 * try to clean up, since that should be done as the NIR stage)
	 * we cache the address value generated for a given src value:
	 */
	struct hash_table *addr_ht;

	/* for calculating input/output positions/linkages: */
	unsigned next_inloc;

	/* a4xx (at least patchlevel 0) cannot seem to flat-interpolate
	 * so we need to use ldlv.u32 to load the varying directly:
	 */
	bool flat_bypass;

	/* on a3xx, we need to add one to # of array levels:
	 */
	bool levels_add_one;

	/* for looking up which system value is which */
	unsigned sysval_semantics[8];

	/* list of kill instructions: */
	struct ir3_instruction *kill[16];
	unsigned int kill_count;

	/* set if we encounter something we can't handle yet, so we
	 * can bail cleanly and fallback to TGSI compiler f/e
	 */
	bool error;
};


static struct nir_shader *to_nir(const struct tgsi_token *tokens)
{
	struct nir_shader_compiler_options options = {
			.lower_fpow = true,
			.lower_fsat = true,
			.lower_scmp = true,
			.lower_flrp = true,
			.native_integers = true,
	};
	bool progress;

	struct nir_shader *s = tgsi_to_nir(tokens, &options);

	if (fd_mesa_debug & FD_DBG_OPTMSGS) {
		debug_printf("----------------------\n");
		nir_print_shader(s, stdout);
		debug_printf("----------------------\n");
	}

	nir_opt_global_to_local(s);
	nir_convert_to_ssa(s);
	nir_lower_idiv(s);

	do {
		progress = false;

		nir_lower_vars_to_ssa(s);
		nir_lower_alu_to_scalar(s);

		progress |= nir_copy_prop(s);
		progress |= nir_opt_dce(s);
		progress |= nir_opt_cse(s);
		progress |= ir3_nir_lower_if_else(s);
		progress |= nir_opt_algebraic(s);
		progress |= nir_opt_constant_folding(s);

	} while (progress);

	nir_remove_dead_variables(s);
	nir_validate_shader(s);

	if (fd_mesa_debug & FD_DBG_OPTMSGS) {
		debug_printf("----------------------\n");
		nir_print_shader(s, stdout);
		debug_printf("----------------------\n");
	}

	return s;
}

/* TODO nir doesn't lower everything for us yet, but ideally it would: */
static const struct tgsi_token *
lower_tgsi(const struct tgsi_token *tokens, struct ir3_shader_variant *so)
{
	struct tgsi_shader_info info;
	struct tgsi_lowering_config lconfig = {
			.color_two_side = so->key.color_two_side,
			.lower_FRC = true,
	};

	switch (so->type) {
	case SHADER_FRAGMENT:
	case SHADER_COMPUTE:
		lconfig.saturate_s = so->key.fsaturate_s;
		lconfig.saturate_t = so->key.fsaturate_t;
		lconfig.saturate_r = so->key.fsaturate_r;
		break;
	case SHADER_VERTEX:
		lconfig.saturate_s = so->key.vsaturate_s;
		lconfig.saturate_t = so->key.vsaturate_t;
		lconfig.saturate_r = so->key.vsaturate_r;
		break;
	}

	if (!so->shader) {
		/* hack for standalone compiler which does not have
		 * screen/context:
		 */
	} else if (ir3_shader_gpuid(so->shader) >= 400) {
		/* a4xx seems to have *no* sam.p */
		lconfig.lower_TXP = ~0;  /* lower all txp */
	} else {
		/* a3xx just needs to avoid sam.p for 3d tex */
		lconfig.lower_TXP = (1 << TGSI_TEXTURE_3D);
	}

	return tgsi_transform_lowering(&lconfig, tokens, &info);
}

static struct ir3_compile *
compile_init(struct ir3_shader_variant *so,
		const struct tgsi_token *tokens)
{
	struct ir3_compile *ctx = rzalloc(NULL, struct ir3_compile);
	const struct tgsi_token *lowered_tokens;

	if (!so->shader) {
		/* hack for standalone compiler which does not have
		 * screen/context:
		 */
	} else if (ir3_shader_gpuid(so->shader) >= 400) {
		/* need special handling for "flat" */
		ctx->flat_bypass = true;
		ctx->levels_add_one = false;
	} else {
		/* no special handling for "flat" */
		ctx->flat_bypass = false;
		ctx->levels_add_one = true;
	}

	switch (so->type) {
	case SHADER_FRAGMENT:
	case SHADER_COMPUTE:
		ctx->integer_s = so->key.finteger_s;
		break;
	case SHADER_VERTEX:
		ctx->integer_s = so->key.vinteger_s;
		break;
	}

	ctx->ir = so->ir;
	ctx->so = so;
	ctx->next_inloc = 8;
	ctx->def_ht = _mesa_hash_table_create(ctx,
			_mesa_hash_pointer, _mesa_key_pointer_equal);
	ctx->var_ht = _mesa_hash_table_create(ctx,
			_mesa_hash_pointer, _mesa_key_pointer_equal);
	ctx->addr_ht = _mesa_hash_table_create(ctx,
			_mesa_hash_pointer, _mesa_key_pointer_equal);

	lowered_tokens = lower_tgsi(tokens, so);
	if (!lowered_tokens)
		lowered_tokens = tokens;
	ctx->s = to_nir(lowered_tokens);

	if (lowered_tokens != tokens)
		free((void *)lowered_tokens);

	so->first_driver_param = so->first_immediate = ctx->s->num_uniforms;

	/* one (vec4) slot for vertex id base: */
	if (so->type == SHADER_VERTEX)
		so->first_immediate++;

	/* reserve 4 (vec4) slots for ubo base addresses: */
	so->first_immediate += 4;

	return ctx;
}

static void
compile_error(struct ir3_compile *ctx, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	_debug_vprintf(format, ap);
	va_end(ap);
	nir_print_shader(ctx->s, stdout);
	ctx->error = true;
	debug_assert(0);
}

#define compile_assert(ctx, cond) do { \
		if (!(cond)) compile_error((ctx), "failed assert: "#cond"\n"); \
	} while (0)

static void
compile_free(struct ir3_compile *ctx)
{
	ralloc_free(ctx);
}


struct ir3_array {
	unsigned length, aid;
	struct ir3_instruction *arr[];
};

static void
declare_var(struct ir3_compile *ctx, nir_variable *var)
{
	unsigned length = glsl_get_length(var->type) * 4;  /* always vec4, at least with ttn */
	struct ir3_array *arr = ralloc_size(ctx, sizeof(*arr) +
			(length * sizeof(arr->arr[0])));
	arr->length = length;
	arr->aid = ++ctx->num_arrays;
	/* Some shaders end up reading array elements without first writing..
	 * so initialize things to prevent null instr ptrs later:
	 */
	for (unsigned i = 0; i < length; i++)
		arr->arr[i] = create_immed(ctx->block, 0);
	_mesa_hash_table_insert(ctx->var_ht, var, arr);
}

static struct ir3_array *
get_var(struct ir3_compile *ctx, nir_variable *var)
{
	struct hash_entry *entry = _mesa_hash_table_search(ctx->var_ht, var);
	return entry->data;
}

/* allocate a n element value array (to be populated by caller) and
 * insert in def_ht
 */
static struct ir3_instruction **
__get_dst(struct ir3_compile *ctx, void *key, unsigned n)
{
	struct ir3_instruction **value =
		ralloc_array(ctx->def_ht, struct ir3_instruction *, n);
	_mesa_hash_table_insert(ctx->def_ht, key, value);
	return value;
}

static struct ir3_instruction **
get_dst(struct ir3_compile *ctx, nir_dest *dst, unsigned n)
{
	if (dst->is_ssa) {
		return __get_dst(ctx, &dst->ssa, n);
	} else {
		return __get_dst(ctx, dst->reg.reg, n);
	}
}

static struct ir3_instruction **
get_dst_ssa(struct ir3_compile *ctx, nir_ssa_def *dst, unsigned n)
{
	return __get_dst(ctx, dst, n);
}

static struct ir3_instruction **
get_src(struct ir3_compile *ctx, nir_src *src)
{
	struct hash_entry *entry;
	if (src->is_ssa) {
		entry = _mesa_hash_table_search(ctx->def_ht, src->ssa);
	} else {
		entry = _mesa_hash_table_search(ctx->def_ht, src->reg.reg);
	}
	compile_assert(ctx, entry);
	return entry->data;
}

static struct ir3_instruction *
create_immed(struct ir3_block *block, uint32_t val)
{
	struct ir3_instruction *mov;

	mov = ir3_instr_create(block, 1, 0);
	mov->cat1.src_type = TYPE_U32;
	mov->cat1.dst_type = TYPE_U32;
	ir3_reg_create(mov, 0, 0);
	ir3_reg_create(mov, 0, IR3_REG_IMMED)->uim_val = val;

	return mov;
}

static struct ir3_instruction *
create_addr(struct ir3_block *block, struct ir3_instruction *src)
{
	struct ir3_instruction *instr, *immed;

	/* TODO in at least some cases, the backend could probably be
	 * made clever enough to propagate IR3_REG_HALF..
	 */
	instr = ir3_COV(block, src, TYPE_U32, TYPE_S16);
	instr->regs[0]->flags |= IR3_REG_HALF;

	immed = create_immed(block, 2);
	immed->regs[0]->flags |= IR3_REG_HALF;

	instr = ir3_SHL_B(block, instr, 0, immed, 0);
	instr->regs[0]->flags |= IR3_REG_HALF;
	instr->regs[1]->flags |= IR3_REG_HALF;

	instr = ir3_MOV(block, instr, TYPE_S16);
	instr->regs[0]->flags |= IR3_REG_ADDR | IR3_REG_HALF;
	instr->regs[1]->flags |= IR3_REG_HALF;

	return instr;
}

/* caches addr values to avoid generating multiple cov/shl/mova
 * sequences for each use of a given NIR level src as address
 */
static struct ir3_instruction *
get_addr(struct ir3_compile *ctx, struct ir3_instruction *src)
{
	struct ir3_instruction *addr;
	struct hash_entry *entry;
	entry = _mesa_hash_table_search(ctx->addr_ht, src);
	if (entry)
		return entry->data;

	/* TODO do we need to cache per block? */
	addr = create_addr(ctx->block, src);
	_mesa_hash_table_insert(ctx->addr_ht, src, addr);

	return addr;
}

static struct ir3_instruction *
create_uniform(struct ir3_compile *ctx, unsigned n)
{
	struct ir3_instruction *mov;

	mov = ir3_instr_create(ctx->block, 1, 0);
	/* TODO get types right? */
	mov->cat1.src_type = TYPE_F32;
	mov->cat1.dst_type = TYPE_F32;
	ir3_reg_create(mov, 0, 0);
	ir3_reg_create(mov, n, IR3_REG_CONST);

	return mov;
}

static struct ir3_instruction *
create_uniform_indirect(struct ir3_compile *ctx, unsigned n,
		struct ir3_instruction *address)
{
	struct ir3_instruction *mov;

	mov = ir3_instr_create(ctx->block, 1, 0);
	mov->cat1.src_type = TYPE_U32;
	mov->cat1.dst_type = TYPE_U32;
	ir3_reg_create(mov, 0, 0);
	ir3_reg_create(mov, n, IR3_REG_CONST | IR3_REG_RELATIV);
	mov->address = address;

	array_insert(ctx->ir->indirects, mov);

	return mov;
}

static struct ir3_instruction *
create_collect(struct ir3_block *block, struct ir3_instruction **arr,
		unsigned arrsz)
{
	struct ir3_instruction *collect;

	if (arrsz == 0)
		return NULL;

	collect = ir3_instr_create2(block, -1, OPC_META_FI, 1 + arrsz);
	ir3_reg_create(collect, 0, 0);
	for (unsigned i = 0; i < arrsz; i++)
		ir3_reg_create(collect, 0, IR3_REG_SSA)->instr = arr[i];

	return collect;
}

static struct ir3_instruction *
create_indirect_load(struct ir3_compile *ctx, unsigned arrsz, unsigned n,
		struct ir3_instruction *address, struct ir3_instruction *collect)
{
	struct ir3_block *block = ctx->block;
	struct ir3_instruction *mov;
	struct ir3_register *src;

	mov = ir3_instr_create(block, 1, 0);
	mov->cat1.src_type = TYPE_U32;
	mov->cat1.dst_type = TYPE_U32;
	ir3_reg_create(mov, 0, 0);
	src = ir3_reg_create(mov, 0, IR3_REG_SSA | IR3_REG_RELATIV);
	src->instr = collect;
	src->size  = arrsz;
	src->offset = n;
	mov->address = address;

	array_insert(ctx->ir->indirects, mov);

	return mov;
}

static struct ir3_instruction *
create_indirect_store(struct ir3_compile *ctx, unsigned arrsz, unsigned n,
		struct ir3_instruction *src, struct ir3_instruction *address,
		struct ir3_instruction *collect)
{
	struct ir3_block *block = ctx->block;
	struct ir3_instruction *mov;
	struct ir3_register *dst;

	mov = ir3_instr_create(block, 1, 0);
	mov->cat1.src_type = TYPE_U32;
	mov->cat1.dst_type = TYPE_U32;
	dst = ir3_reg_create(mov, 0, IR3_REG_RELATIV);
	dst->size  = arrsz;
	dst->offset = n;
	ir3_reg_create(mov, 0, IR3_REG_SSA)->instr = src;
	mov->address = address;
	mov->fanin = collect;

	array_insert(ctx->ir->indirects, mov);

	return mov;
}

static struct ir3_instruction *
create_input(struct ir3_block *block, struct ir3_instruction *instr,
		unsigned n)
{
	struct ir3_instruction *in;

	in = ir3_instr_create(block, -1, OPC_META_INPUT);
	in->inout.block = block;
	ir3_reg_create(in, n, 0);
	if (instr)
		ir3_reg_create(in, 0, IR3_REG_SSA)->instr = instr;

	return in;
}

static struct ir3_instruction *
create_frag_input(struct ir3_compile *ctx, unsigned n, bool use_ldlv)
{
	struct ir3_block *block = ctx->block;
	struct ir3_instruction *instr;
	struct ir3_instruction *inloc = create_immed(block, n);

	if (use_ldlv) {
		instr = ir3_LDLV(block, inloc, 0, create_immed(block, 1), 0);
		instr->cat6.type = TYPE_U32;
		instr->cat6.iim_val = 1;
	} else {
		instr = ir3_BARY_F(block, inloc, 0, ctx->frag_pos, 0);
		instr->regs[2]->wrmask = 0x3;
	}

	return instr;
}

static struct ir3_instruction *
create_frag_coord(struct ir3_compile *ctx, unsigned comp)
{
	struct ir3_block *block = ctx->block;
	struct ir3_instruction *instr;

	compile_assert(ctx, !ctx->frag_coord[comp]);

	ctx->frag_coord[comp] = create_input(ctx->block, NULL, 0);

	switch (comp) {
	case 0: /* .x */
	case 1: /* .y */
		/* for frag_coord, we get unsigned values.. we need
		 * to subtract (integer) 8 and divide by 16 (right-
		 * shift by 4) then convert to float:
		 *
		 *    sub.s tmp, src, 8
		 *    shr.b tmp, tmp, 4
		 *    mov.u32f32 dst, tmp
		 *
		 */
		instr = ir3_SUB_S(block, ctx->frag_coord[comp], 0,
				create_immed(block, 8), 0);
		instr = ir3_SHR_B(block, instr, 0,
				create_immed(block, 4), 0);
		instr = ir3_COV(block, instr, TYPE_U32, TYPE_F32);

		return instr;
	case 2: /* .z */
	case 3: /* .w */
	default:
		/* seems that we can use these as-is: */
		return ctx->frag_coord[comp];
	}
}

static struct ir3_instruction *
create_frag_face(struct ir3_compile *ctx, unsigned comp)
{
	struct ir3_block *block = ctx->block;
	struct ir3_instruction *instr;

	switch (comp) {
	case 0: /* .x */
		compile_assert(ctx, !ctx->frag_face);

		ctx->frag_face = create_input(block, NULL, 0);

		/* for faceness, we always get -1 or 0 (int).. but TGSI expects
		 * positive vs negative float.. and piglit further seems to
		 * expect -1.0 or 1.0:
		 *
		 *    mul.s tmp, hr0.x, 2
		 *    add.s tmp, tmp, 1
		 *    mov.s32f32, dst, tmp
		 *
		 */
		instr = ir3_MUL_S(block, ctx->frag_face, 0,
				create_immed(block, 2), 0);
		instr = ir3_ADD_S(block, instr, 0,
				create_immed(block, 1), 0);
		instr = ir3_COV(block, instr, TYPE_S32, TYPE_F32);

		return instr;
	case 1: /* .y */
	case 2: /* .z */
		return create_immed(block, fui(0.0));
	default:
	case 3: /* .w */
		return create_immed(block, fui(1.0));
	}
}

/* helper for instructions that produce multiple consecutive scalar
 * outputs which need to have a split/fanout meta instruction inserted
 */
static void
split_dest(struct ir3_block *block, struct ir3_instruction **dst,
		struct ir3_instruction *src)
{
	struct ir3_instruction *prev = NULL;
	for (int i = 0, j = 0; i < 4; i++) {
		struct ir3_instruction *split =
				ir3_instr_create(block, -1, OPC_META_FO);
		ir3_reg_create(split, 0, IR3_REG_SSA);
		ir3_reg_create(split, 0, IR3_REG_SSA)->instr = src;
		split->fo.off = i;

		if (prev) {
			split->cp.left = prev;
			split->cp.left_cnt++;
			prev->cp.right = split;
			prev->cp.right_cnt++;
		}
		prev = split;

		if (src->regs[0]->wrmask & (1 << i))
			dst[j++] = split;
	}
}

/*
 * Adreno uses uint rather than having dedicated bool type,
 * which (potentially) requires some conversion, in particular
 * when using output of an bool instr to int input, or visa
 * versa.
 *
 *         | Adreno  |  NIR  |
 *  -------+---------+-------+-
 *   true  |    1    |  ~0   |
 *   false |    0    |   0   |
 *
 * To convert from an adreno bool (uint) to nir, use:
 *
 *    absneg.s dst, (neg)src
 *
 * To convert back in the other direction:
 *
 *    absneg.s dst, (abs)arc
 *
 * The CP step can clean up the absneg.s that cancel each other
 * out, and with a slight bit of extra cleverness (to recognize
 * the instructions which produce either a 0 or 1) can eliminate
 * the absneg.s's completely when an instruction that wants
 * 0/1 consumes the result.  For example, when a nir 'bcsel'
 * consumes the result of 'feq'.  So we should be able to get by
 * without a boolean resolve step, and without incuring any
 * extra penalty in instruction count.
 */

/* NIR bool -> native (adreno): */
static struct ir3_instruction *
ir3_b2n(struct ir3_block *block, struct ir3_instruction *instr)
{
	return ir3_ABSNEG_S(block, instr, IR3_REG_SABS);
}

/* native (adreno) -> NIR bool: */
static struct ir3_instruction *
ir3_n2b(struct ir3_block *block, struct ir3_instruction *instr)
{
	return ir3_ABSNEG_S(block, instr, IR3_REG_SNEG);
}

/*
 * alu/sfu instructions:
 */

static void
emit_alu(struct ir3_compile *ctx, nir_alu_instr *alu)
{
	const nir_op_info *info = &nir_op_infos[alu->op];
	struct ir3_instruction **dst, *src[info->num_inputs];
	struct ir3_block *b = ctx->block;

	dst = get_dst(ctx, &alu->dest.dest, MAX2(info->output_size, 1));

	/* Vectors are special in that they have non-scalarized writemasks,
	 * and just take the first swizzle channel for each argument in
	 * order into each writemask channel.
	 */
	if ((alu->op == nir_op_vec2) ||
			(alu->op == nir_op_vec3) ||
			(alu->op == nir_op_vec4)) {

		for (int i = 0; i < info->num_inputs; i++) {
			nir_alu_src *asrc = &alu->src[i];

			compile_assert(ctx, !asrc->abs);
			compile_assert(ctx, !asrc->negate);

			src[i] = get_src(ctx, &asrc->src)[asrc->swizzle[0]];
			if (!src[i])
				src[i] = create_immed(ctx->block, 0);
			dst[i] = ir3_MOV(b, src[i], TYPE_U32);
		}

		return;
	}

	/* General case: We can just grab the one used channel per src. */
	for (int i = 0; i < info->num_inputs; i++) {
		unsigned chan = ffs(alu->dest.write_mask) - 1;
		nir_alu_src *asrc = &alu->src[i];

		compile_assert(ctx, !asrc->abs);
		compile_assert(ctx, !asrc->negate);

		src[i] = get_src(ctx, &asrc->src)[asrc->swizzle[chan]];

		compile_assert(ctx, src[i]);
	}

	switch (alu->op) {
	case nir_op_f2i:
		dst[0] = ir3_COV(b, src[0], TYPE_F32, TYPE_S32);
		break;
	case nir_op_f2u:
		dst[0] = ir3_COV(b, src[0], TYPE_F32, TYPE_U32);
		break;
	case nir_op_i2f:
		dst[0] = ir3_COV(b, src[0], TYPE_S32, TYPE_F32);
		break;
	case nir_op_u2f:
		dst[0] = ir3_COV(b, src[0], TYPE_U32, TYPE_F32);
		break;
	case nir_op_imov:
		dst[0] = ir3_MOV(b, src[0], TYPE_S32);
		break;
	case nir_op_fmov:
		dst[0] = ir3_MOV(b, src[0], TYPE_F32);
		break;
	case nir_op_f2b:
		dst[0] = ir3_CMPS_F(b, src[0], 0, create_immed(b, fui(0.0)), 0);
		dst[0]->cat2.condition = IR3_COND_NE;
		dst[0] = ir3_n2b(b, dst[0]);
		break;
	case nir_op_b2f:
		dst[0] = ir3_COV(b, ir3_b2n(b, src[0]), TYPE_U32, TYPE_F32);
		break;
	case nir_op_b2i:
		dst[0] = ir3_b2n(b, src[0]);
		break;
	case nir_op_i2b:
		dst[0] = ir3_CMPS_S(b, src[0], 0, create_immed(b, 0), 0);
		dst[0]->cat2.condition = IR3_COND_NE;
		dst[0] = ir3_n2b(b, dst[0]);
		break;

	case nir_op_fneg:
		dst[0] = ir3_ABSNEG_F(b, src[0], IR3_REG_FNEG);
		break;
	case nir_op_fabs:
		dst[0] = ir3_ABSNEG_F(b, src[0], IR3_REG_FABS);
		break;
	case nir_op_fmax:
		dst[0] = ir3_MAX_F(b, src[0], 0, src[1], 0);
		break;
	case nir_op_fmin:
		dst[0] = ir3_MIN_F(b, src[0], 0, src[1], 0);
		break;
	case nir_op_fmul:
		dst[0] = ir3_MUL_F(b, src[0], 0, src[1], 0);
		break;
	case nir_op_fadd:
		dst[0] = ir3_ADD_F(b, src[0], 0, src[1], 0);
		break;
	case nir_op_fsub:
		dst[0] = ir3_ADD_F(b, src[0], 0, src[1], IR3_REG_FNEG);
		break;
	case nir_op_ffma:
		dst[0] = ir3_MAD_F32(b, src[0], 0, src[1], 0, src[2], 0);
		break;
	case nir_op_fddx:
		dst[0] = ir3_DSX(b, src[0], 0);
		dst[0]->cat5.type = TYPE_F32;
		break;
	case nir_op_fddy:
		dst[0] = ir3_DSY(b, src[0], 0);
		dst[0]->cat5.type = TYPE_F32;
		break;
		break;
	case nir_op_flt:
		dst[0] = ir3_CMPS_F(b, src[0], 0, src[1], 0);
		dst[0]->cat2.condition = IR3_COND_LT;
		dst[0] = ir3_n2b(b, dst[0]);
		break;
	case nir_op_fge:
		dst[0] = ir3_CMPS_F(b, src[0], 0, src[1], 0);
		dst[0]->cat2.condition = IR3_COND_GE;
		dst[0] = ir3_n2b(b, dst[0]);
		break;
	case nir_op_feq:
		dst[0] = ir3_CMPS_F(b, src[0], 0, src[1], 0);
		dst[0]->cat2.condition = IR3_COND_EQ;
		dst[0] = ir3_n2b(b, dst[0]);
		break;
	case nir_op_fne:
		dst[0] = ir3_CMPS_F(b, src[0], 0, src[1], 0);
		dst[0]->cat2.condition = IR3_COND_NE;
		dst[0] = ir3_n2b(b, dst[0]);
		break;
	case nir_op_fceil:
		dst[0] = ir3_CEIL_F(b, src[0], 0);
		break;
	case nir_op_ffloor:
		dst[0] = ir3_FLOOR_F(b, src[0], 0);
		break;
	case nir_op_ftrunc:
		dst[0] = ir3_TRUNC_F(b, src[0], 0);
		break;
	case nir_op_fround_even:
		dst[0] = ir3_RNDNE_F(b, src[0], 0);
		break;
	case nir_op_fsign:
		dst[0] = ir3_SIGN_F(b, src[0], 0);
		break;

	case nir_op_fsin:
		dst[0] = ir3_SIN(b, src[0], 0);
		break;
	case nir_op_fcos:
		dst[0] = ir3_COS(b, src[0], 0);
		break;
	case nir_op_frsq:
		dst[0] = ir3_RSQ(b, src[0], 0);
		break;
	case nir_op_frcp:
		dst[0] = ir3_RCP(b, src[0], 0);
		break;
	case nir_op_flog2:
		dst[0] = ir3_LOG2(b, src[0], 0);
		break;
	case nir_op_fexp2:
		dst[0] = ir3_EXP2(b, src[0], 0);
		break;
	case nir_op_fsqrt:
		dst[0] = ir3_SQRT(b, src[0], 0);
		break;

	case nir_op_iabs:
		dst[0] = ir3_ABSNEG_S(b, src[0], IR3_REG_SABS);
		break;
	case nir_op_iadd:
		dst[0] = ir3_ADD_U(b, src[0], 0, src[1], 0);
		break;
	case nir_op_iand:
		dst[0] = ir3_AND_B(b, src[0], 0, src[1], 0);
		break;
	case nir_op_imax:
		dst[0] = ir3_MAX_S(b, src[0], 0, src[1], 0);
		break;
	case nir_op_imin:
		dst[0] = ir3_MIN_S(b, src[0], 0, src[1], 0);
		break;
	case nir_op_imul:
		/*
		 * dst = (al * bl) + (ah * bl << 16) + (al * bh << 16)
		 *   mull.u tmp0, a, b           ; mul low, i.e. al * bl
		 *   madsh.m16 tmp1, a, b, tmp0  ; mul-add shift high mix, i.e. ah * bl << 16
		 *   madsh.m16 dst, b, a, tmp1   ; i.e. al * bh << 16
		 */
		dst[0] = ir3_MADSH_M16(b, src[1], 0, src[0], 0,
					ir3_MADSH_M16(b, src[0], 0, src[1], 0,
						ir3_MULL_U(b, src[0], 0, src[1], 0), 0), 0);
		break;
	case nir_op_ineg:
		dst[0] = ir3_ABSNEG_S(b, src[0], IR3_REG_SNEG);
		break;
	case nir_op_inot:
		dst[0] = ir3_NOT_B(b, src[0], 0);
		break;
	case nir_op_ior:
		dst[0] = ir3_OR_B(b, src[0], 0, src[1], 0);
		break;
	case nir_op_ishl:
		dst[0] = ir3_SHL_B(b, src[0], 0, src[1], 0);
		break;
	case nir_op_ishr:
		dst[0] = ir3_ASHR_B(b, src[0], 0, src[1], 0);
		break;
	case nir_op_isign: {
		/* maybe this would be sane to lower in nir.. */
		struct ir3_instruction *neg, *pos;

		neg = ir3_CMPS_S(b, src[0], 0, create_immed(b, 0), 0);
		neg->cat2.condition = IR3_COND_LT;

		pos = ir3_CMPS_S(b, src[0], 0, create_immed(b, 0), 0);
		pos->cat2.condition = IR3_COND_GT;

		dst[0] = ir3_SUB_U(b, pos, 0, neg, 0);

		break;
	}
	case nir_op_isub:
		dst[0] = ir3_SUB_U(b, src[0], 0, src[1], 0);
		break;
	case nir_op_ixor:
		dst[0] = ir3_XOR_B(b, src[0], 0, src[1], 0);
		break;
	case nir_op_ushr:
		dst[0] = ir3_SHR_B(b, src[0], 0, src[1], 0);
		break;
	case nir_op_ilt:
		dst[0] = ir3_CMPS_S(b, src[0], 0, src[1], 0);
		dst[0]->cat2.condition = IR3_COND_LT;
		dst[0] = ir3_n2b(b, dst[0]);
		break;
	case nir_op_ige:
		dst[0] = ir3_CMPS_S(b, src[0], 0, src[1], 0);
		dst[0]->cat2.condition = IR3_COND_GE;
		dst[0] = ir3_n2b(b, dst[0]);
		break;
	case nir_op_ieq:
		dst[0] = ir3_CMPS_S(b, src[0], 0, src[1], 0);
		dst[0]->cat2.condition = IR3_COND_EQ;
		dst[0] = ir3_n2b(b, dst[0]);
		break;
	case nir_op_ine:
		dst[0] = ir3_CMPS_S(b, src[0], 0, src[1], 0);
		dst[0]->cat2.condition = IR3_COND_NE;
		dst[0] = ir3_n2b(b, dst[0]);
		break;
	case nir_op_ult:
		dst[0] = ir3_CMPS_U(b, src[0], 0, src[1], 0);
		dst[0]->cat2.condition = IR3_COND_LT;
		dst[0] = ir3_n2b(b, dst[0]);
		break;
	case nir_op_uge:
		dst[0] = ir3_CMPS_U(b, src[0], 0, src[1], 0);
		dst[0]->cat2.condition = IR3_COND_GE;
		dst[0] = ir3_n2b(b, dst[0]);
		break;

	case nir_op_bcsel:
		dst[0] = ir3_SEL_B32(b, src[1], 0, ir3_b2n(b, src[0]), 0, src[2], 0);
		break;

	default:
		compile_error(ctx, "Unhandled ALU op: %s\n",
				nir_op_infos[alu->op].name);
		break;
	}
}

/* handles direct/indirect UBO reads: */
static void
emit_intrinsic_load_ubo(struct ir3_compile *ctx, nir_intrinsic_instr *intr,
		struct ir3_instruction **dst)
{
	struct ir3_block *b = ctx->block;
	struct ir3_instruction *addr, *src0, *src1;
	/* UBO addresses are the first driver params: */
	unsigned ubo = regid(ctx->so->first_driver_param, 0);
	unsigned off = intr->const_index[0];

	/* First src is ubo index, which could either be an immed or not: */
	src0 = get_src(ctx, &intr->src[0])[0];
	if (is_same_type_mov(src0) &&
			(src0->regs[1]->flags & IR3_REG_IMMED)) {
		addr = create_uniform(ctx, ubo + src0->regs[1]->iim_val);
	} else {
		addr = create_uniform_indirect(ctx, ubo, get_addr(ctx, src0));
	}

	if (intr->intrinsic == nir_intrinsic_load_ubo_indirect) {
		/* For load_ubo_indirect, second src is indirect offset: */
		src1 = get_src(ctx, &intr->src[1])[0];

		/* and add offset to addr: */
		addr = ir3_ADD_S(b, addr, 0, src1, 0);
	}

	/* if offset is to large to encode in the ldg, split it out: */
	if ((off + (intr->num_components * 4)) > 1024) {
		/* split out the minimal amount to improve the odds that
		 * cp can fit the immediate in the add.s instruction:
		 */
		unsigned off2 = off + (intr->num_components * 4) - 1024;
		addr = ir3_ADD_S(b, addr, 0, create_immed(b, off2), 0);
		off -= off2;
	}

	for (int i = 0; i < intr->num_components; i++) {
		struct ir3_instruction *load =
				ir3_LDG(b, addr, 0, create_immed(b, 1), 0);
		load->cat6.type = TYPE_U32;
		load->cat6.offset = off + i * 4;    /* byte offset */
		dst[i] = load;
	}
}

/* handles array reads: */
static void
emit_intrinisic_load_var(struct ir3_compile *ctx, nir_intrinsic_instr *intr,
		struct ir3_instruction **dst)
{
	nir_deref_var *dvar = intr->variables[0];
	nir_deref_array *darr = nir_deref_as_array(dvar->deref.child);
	struct ir3_array *arr = get_var(ctx, dvar->var);

	compile_assert(ctx, dvar->deref.child &&
		(dvar->deref.child->deref_type == nir_deref_type_array));

	switch (darr->deref_array_type) {
	case nir_deref_array_type_direct:
		/* direct access does not require anything special: */
		for (int i = 0; i < intr->num_components; i++) {
			unsigned n = darr->base_offset * 4 + i;
			compile_assert(ctx, n < arr->length);
			dst[i] = arr->arr[n];
		}
		break;
	case nir_deref_array_type_indirect: {
		/* for indirect, we need to collect all the array elements: */
		struct ir3_instruction *collect =
				create_collect(ctx->block, arr->arr, arr->length);
		struct ir3_instruction *addr =
				get_addr(ctx, get_src(ctx, &darr->indirect)[0]);
		for (int i = 0; i < intr->num_components; i++) {
			unsigned n = darr->base_offset * 4 + i;
			compile_assert(ctx, n < arr->length);
			dst[i] = create_indirect_load(ctx, arr->length, n, addr, collect);
		}
		break;
	}
	default:
		compile_error(ctx, "Unhandled load deref type: %u\n",
				darr->deref_array_type);
		break;
	}
}

/* handles array writes: */
static void
emit_intrinisic_store_var(struct ir3_compile *ctx, nir_intrinsic_instr *intr)
{
	nir_deref_var *dvar = intr->variables[0];
	nir_deref_array *darr = nir_deref_as_array(dvar->deref.child);
	struct ir3_array *arr = get_var(ctx, dvar->var);
	struct ir3_instruction **src;

	compile_assert(ctx, dvar->deref.child &&
		(dvar->deref.child->deref_type == nir_deref_type_array));

	src = get_src(ctx, &intr->src[0]);

	switch (darr->deref_array_type) {
	case nir_deref_array_type_direct:
		/* direct access does not require anything special: */
		for (int i = 0; i < intr->num_components; i++) {
			unsigned n = darr->base_offset * 4 + i;
			compile_assert(ctx, n < arr->length);
			arr->arr[n] = src[i];
		}
		break;
	case nir_deref_array_type_indirect: {
		/* for indirect, create indirect-store and fan that out: */
		struct ir3_instruction *collect =
				create_collect(ctx->block, arr->arr, arr->length);
		struct ir3_instruction *addr =
				get_addr(ctx, get_src(ctx, &darr->indirect)[0]);
		for (int i = 0; i < intr->num_components; i++) {
			struct ir3_instruction *store;
			unsigned n = darr->base_offset * 4 + i;
			compile_assert(ctx, n < arr->length);

			store = create_indirect_store(ctx, arr->length,
					n, src[i], addr, collect);

			store->fanin->fi.aid = arr->aid;

			/* TODO: probably split this out to be used for
			 * store_output_indirect? or move this into
			 * create_indirect_store()?
			 */
			for (int j = i; j < arr->length; j += 4) {
				struct ir3_instruction *split;

				split = ir3_instr_create(ctx->block, -1, OPC_META_FO);
				split->fo.off = j;
				ir3_reg_create(split, 0, 0);
				ir3_reg_create(split, 0, IR3_REG_SSA)->instr = store;

				arr->arr[j] = split;
			}
		}
		break;
	}
	default:
		compile_error(ctx, "Unhandled store deref type: %u\n",
				darr->deref_array_type);
		break;
	}
}

static void add_sysval_input(struct ir3_compile *ctx, unsigned name,
		struct ir3_instruction *instr)
{
	struct ir3_shader_variant *so = ctx->so;
	unsigned r = regid(so->inputs_count, 0);
	unsigned n = so->inputs_count++;

	so->inputs[n].semantic = ir3_semantic_name(name, 0);
	so->inputs[n].compmask = 1;
	so->inputs[n].regid = r;
	so->inputs[n].interpolate = TGSI_INTERPOLATE_CONSTANT;
	so->total_in++;

	ctx->block->ninputs = MAX2(ctx->block->ninputs, r + 1);
	ctx->block->inputs[r] = instr;
}

static void
emit_intrinisic(struct ir3_compile *ctx, nir_intrinsic_instr *intr)
{
	const nir_intrinsic_info *info = &nir_intrinsic_infos[intr->intrinsic];
	struct ir3_instruction **dst, **src;
	struct ir3_block *b = ctx->block;
	unsigned idx = intr->const_index[0];

	if (info->has_dest) {
		dst = get_dst(ctx, &intr->dest, intr->num_components);
	}

	switch (intr->intrinsic) {
	case nir_intrinsic_load_uniform:
		compile_assert(ctx, intr->const_index[1] == 1);
		for (int i = 0; i < intr->num_components; i++) {
			unsigned n = idx * 4 + i;
			dst[i] = create_uniform(ctx, n);
		}
		break;
	case nir_intrinsic_load_uniform_indirect:
		compile_assert(ctx, intr->const_index[1] == 1);
		src = get_src(ctx, &intr->src[0]);
		for (int i = 0; i < intr->num_components; i++) {
			unsigned n = idx * 4 + i;
			dst[i] = create_uniform_indirect(ctx, n,
					get_addr(ctx, src[0]));
		}
		break;
	case nir_intrinsic_load_ubo:
	case nir_intrinsic_load_ubo_indirect:
		emit_intrinsic_load_ubo(ctx, intr, dst);
		break;
	case nir_intrinsic_load_input:
		compile_assert(ctx, intr->const_index[1] == 1);
		for (int i = 0; i < intr->num_components; i++) {
			unsigned n = idx * 4 + i;
			dst[i] = b->inputs[n];
		}
		break;
	case nir_intrinsic_load_input_indirect:
		compile_assert(ctx, intr->const_index[1] == 1);
		src = get_src(ctx, &intr->src[0]);
		struct ir3_instruction *collect =
				create_collect(b, b->inputs, b->ninputs);
		struct ir3_instruction *addr = get_addr(ctx, src[0]);
		for (int i = 0; i < intr->num_components; i++) {
			unsigned n = idx * 4 + i;
			dst[i] = create_indirect_load(ctx, b->ninputs, n, addr, collect);
		}
		break;
	case nir_intrinsic_load_var:
		emit_intrinisic_load_var(ctx, intr, dst);
		break;
	case nir_intrinsic_store_var:
		emit_intrinisic_store_var(ctx, intr);
		break;
	case nir_intrinsic_store_output:
		compile_assert(ctx, intr->const_index[1] == 1);
		src = get_src(ctx, &intr->src[0]);
		for (int i = 0; i < intr->num_components; i++) {
			unsigned n = idx * 4 + i;
			b->outputs[n] = src[i];
		}
		break;
	case nir_intrinsic_load_base_vertex:
		if (!ctx->basevertex) {
			/* first four vec4 sysval's reserved for UBOs: */
			unsigned r = regid(ctx->so->first_driver_param + 4, 0);
			ctx->basevertex = create_uniform(ctx, r);
			add_sysval_input(ctx, TGSI_SEMANTIC_BASEVERTEX,
					ctx->basevertex);
		}
		dst[0] = ctx->basevertex;
		break;
	case nir_intrinsic_load_vertex_id_zero_base:
		if (!ctx->vertex_id) {
			ctx->vertex_id = create_input(ctx->block, NULL, 0);
			add_sysval_input(ctx, TGSI_SEMANTIC_VERTEXID_NOBASE,
					ctx->vertex_id);
		}
		dst[0] = ctx->vertex_id;
		break;
	case nir_intrinsic_load_instance_id:
		if (!ctx->instance_id) {
			ctx->instance_id = create_input(ctx->block, NULL, 0);
			add_sysval_input(ctx, TGSI_SEMANTIC_INSTANCEID,
					ctx->instance_id);
		}
		dst[0] = ctx->instance_id;
		break;
	case nir_intrinsic_discard_if:
	case nir_intrinsic_discard: {
		struct ir3_instruction *cond, *kill;

		if (intr->intrinsic == nir_intrinsic_discard_if) {
			/* conditional discard: */
			src = get_src(ctx, &intr->src[0]);
			cond = ir3_b2n(b, src[0]);
		} else {
			/* unconditional discard: */
			cond = create_immed(b, 1);
		}

		cond = ir3_CMPS_S(b, cond, 0, create_immed(b, 0), 0);
		cond->cat2.condition = IR3_COND_NE;

		/* condition always goes in predicate register: */
		cond->regs[0]->num = regid(REG_P0, 0);

		kill = ir3_KILL(b, cond, 0);

		ctx->kill[ctx->kill_count++] = kill;
		ctx->so->has_kill = true;

		break;
	}
	default:
		compile_error(ctx, "Unhandled intrinsic type: %s\n",
				nir_intrinsic_infos[intr->intrinsic].name);
		break;
	}
}

static void
emit_load_const(struct ir3_compile *ctx, nir_load_const_instr *instr)
{
	struct ir3_instruction **dst = get_dst_ssa(ctx, &instr->def,
			instr->def.num_components);
	for (int i = 0; i < instr->def.num_components; i++)
		dst[i] = create_immed(ctx->block, instr->value.u[i]);
}

static void
emit_undef(struct ir3_compile *ctx, nir_ssa_undef_instr *undef)
{
	struct ir3_instruction **dst = get_dst_ssa(ctx, &undef->def,
			undef->def.num_components);
	/* backend doesn't want undefined instructions, so just plug
	 * in 0.0..
	 */
	for (int i = 0; i < undef->def.num_components; i++)
		dst[i] = create_immed(ctx->block, fui(0.0));
}

/*
 * texture fetch/sample instructions:
 */

static void
tex_info(nir_tex_instr *tex, unsigned *flagsp, unsigned *coordsp)
{
	unsigned coords, flags = 0;

	/* note: would use tex->coord_components.. except txs.. also,
	 * since array index goes after shadow ref, we don't want to
	 * count it:
	 */
	switch (tex->sampler_dim) {
	case GLSL_SAMPLER_DIM_1D:
	case GLSL_SAMPLER_DIM_BUF:
		coords = 1;
		break;
	case GLSL_SAMPLER_DIM_2D:
	case GLSL_SAMPLER_DIM_RECT:
	case GLSL_SAMPLER_DIM_EXTERNAL:
	case GLSL_SAMPLER_DIM_MS:
		coords = 2;
		break;
	case GLSL_SAMPLER_DIM_3D:
	case GLSL_SAMPLER_DIM_CUBE:
		coords = 3;
		flags |= IR3_INSTR_3D;
		break;
	}

	if (tex->is_shadow)
		flags |= IR3_INSTR_S;

	if (tex->is_array)
		flags |= IR3_INSTR_A;

	*flagsp = flags;
	*coordsp = coords;
}

static void
emit_tex(struct ir3_compile *ctx, nir_tex_instr *tex)
{
	struct ir3_block *b = ctx->block;
	struct ir3_instruction **dst, *sam, *src0[12], *src1[4];
	struct ir3_instruction **coord, *lod, *compare, *proj, **off, **ddx, **ddy;
	bool has_bias = false, has_lod = false, has_proj = false, has_off = false;
	unsigned i, coords, flags;
	unsigned nsrc0 = 0, nsrc1 = 0;
	type_t type;
	opc_t opc;

	/* TODO: might just be one component for gathers? */
	dst = get_dst(ctx, &tex->dest, 4);

	for (unsigned i = 0; i < tex->num_srcs; i++) {
		switch (tex->src[i].src_type) {
		case nir_tex_src_coord:
			coord = get_src(ctx, &tex->src[i].src);
			break;
		case nir_tex_src_bias:
			lod = get_src(ctx, &tex->src[i].src)[0];
			has_bias = true;
			break;
		case nir_tex_src_lod:
			lod = get_src(ctx, &tex->src[i].src)[0];
			has_lod = true;
			break;
		case nir_tex_src_comparitor: /* shadow comparator */
			compare = get_src(ctx, &tex->src[i].src)[0];
			break;
		case nir_tex_src_projector:
			proj = get_src(ctx, &tex->src[i].src)[0];
			has_proj = true;
			break;
		case nir_tex_src_offset:
			off = get_src(ctx, &tex->src[i].src);
			has_off = true;
			break;
		case nir_tex_src_ddx:
			ddx = get_src(ctx, &tex->src[i].src);
			break;
		case nir_tex_src_ddy:
			ddy = get_src(ctx, &tex->src[i].src);
			break;
		default:
			compile_error(ctx, "Unhandled NIR tex serc type: %d\n",
					tex->src[i].src_type);
			return;
		}
	}

	switch (tex->op) {
	case nir_texop_tex:      opc = OPC_SAM;      break;
	case nir_texop_txb:      opc = OPC_SAMB;     break;
	case nir_texop_txl:      opc = OPC_SAML;     break;
	case nir_texop_txd:      opc = OPC_SAMGQ;    break;
	case nir_texop_txf:      opc = OPC_ISAML;    break;
	case nir_texop_txf_ms:
	case nir_texop_txs:
	case nir_texop_lod:
	case nir_texop_tg4:
	case nir_texop_query_levels:
		compile_error(ctx, "Unhandled NIR tex type: %d\n", tex->op);
		return;
	}

	tex_info(tex, &flags, &coords);

	/* scale up integer coords for TXF based on the LOD */
	if (opc == OPC_ISAML) {
		assert(has_lod);
		for (i = 0; i < coords; i++)
			coord[i] = ir3_SHL_B(b, coord[i], 0, lod, 0);
	}
	/*
	 * lay out the first argument in the proper order:
	 *  - actual coordinates first
	 *  - shadow reference
	 *  - array index
	 *  - projection w
	 *  - starting at offset 4, dpdx.xy, dpdy.xy
	 *
	 * bias/lod go into the second arg
	 */

	/* insert tex coords: */
	for (i = 0; i < coords; i++)
		src0[nsrc0++] = coord[i];

	if (coords == 1) {
		/* hw doesn't do 1d, so we treat it as 2d with
		 * height of 1, and patch up the y coord.
		 * TODO: y coord should be (int)0 in some cases..
		 */
		src0[nsrc0++] = create_immed(b, fui(0.5));
	}

	if (tex->is_shadow)
		src0[nsrc0++] = compare;

	if (tex->is_array)
		src0[nsrc0++] = coord[coords];

	if (has_proj) {
		src0[nsrc0++] = proj;
		flags |= IR3_INSTR_P;
	}

	/* pad to 4, then ddx/ddy: */
	if (tex->op == nir_texop_txd) {
		while (nsrc0 < 4)
			src0[nsrc0++] = create_immed(b, fui(0.0));
		for (i = 0; i < coords; i++)
			src0[nsrc0++] = ddx[i];
		if (coords < 2)
			src0[nsrc0++] = create_immed(b, fui(0.0));
		for (i = 0; i < coords; i++)
			src0[nsrc0++] = ddy[i];
		if (coords < 2)
			src0[nsrc0++] = create_immed(b, fui(0.0));
	}

	/*
	 * second argument (if applicable):
	 *  - offsets
	 *  - lod
	 *  - bias
	 */
	if (has_off | has_lod | has_bias) {
		if (has_off) {
			for (i = 0; i < coords; i++)
				src1[nsrc1++] = off[i];
			if (coords < 2)
				src1[nsrc1++] = create_immed(b, fui(0.0));
			flags |= IR3_INSTR_O;
		}

		if (has_lod | has_bias)
			src1[nsrc1++] = lod;
	}

	switch (tex->dest_type) {
	case nir_type_invalid:
	case nir_type_float:
		type = TYPE_F32;
		break;
	case nir_type_int:
		type = TYPE_S32;
		break;
	case nir_type_unsigned:
	case nir_type_bool:
		type = TYPE_U32;
		break;
	}

	sam = ir3_SAM(b, opc, type, TGSI_WRITEMASK_XYZW,
			flags, tex->sampler_index, tex->sampler_index,
			create_collect(b, src0, nsrc0),
			create_collect(b, src1, nsrc1));

	split_dest(b, dst, sam);
}

static void
emit_tex_query_levels(struct ir3_compile *ctx, nir_tex_instr *tex)
{
	struct ir3_block *b = ctx->block;
	struct ir3_instruction **dst, *sam;

	dst = get_dst(ctx, &tex->dest, 1);

	sam = ir3_SAM(b, OPC_GETINFO, TYPE_U32, TGSI_WRITEMASK_Z, 0,
			tex->sampler_index, tex->sampler_index, NULL, NULL);

	/* even though there is only one component, since it ends
	 * up in .z rather than .x, we need a split_dest()
	 */
	split_dest(b, dst, sam);

	/* The # of levels comes from getinfo.z. We need to add 1 to it, since
	 * the value in TEX_CONST_0 is zero-based.
	 */
	if (ctx->levels_add_one)
		dst[0] = ir3_ADD_U(b, dst[0], 0, create_immed(b, 1), 0);
}

static void
emit_tex_txs(struct ir3_compile *ctx, nir_tex_instr *tex)
{
	struct ir3_block *b = ctx->block;
	struct ir3_instruction **dst, *sam, *lod;
	unsigned flags, coords;

	tex_info(tex, &flags, &coords);

	dst = get_dst(ctx, &tex->dest, 4);

	compile_assert(ctx, tex->num_srcs == 1);
	compile_assert(ctx, tex->src[0].src_type == nir_tex_src_lod);

	lod = get_src(ctx, &tex->src[0].src)[0];

	sam = ir3_SAM(b, OPC_GETSIZE, TYPE_U32, TGSI_WRITEMASK_XYZW, flags,
			tex->sampler_index, tex->sampler_index, lod, NULL);

	split_dest(b, dst, sam);

	/* Array size actually ends up in .w rather than .z. This doesn't
	 * matter for miplevel 0, but for higher mips the value in z is
	 * minified whereas w stays. Also, the value in TEX_CONST_3_DEPTH is
	 * returned, which means that we have to add 1 to it for arrays.
	 */
	if (tex->is_array) {
		if (ctx->levels_add_one) {
			dst[coords] = ir3_ADD_U(b, dst[3], 0, create_immed(b, 1), 0);
		} else {
			dst[coords] = ir3_MOV(b, dst[3], TYPE_U32);
		}
	}
}

static void
emit_instr(struct ir3_compile *ctx, nir_instr *instr)
{
	switch (instr->type) {
	case nir_instr_type_alu:
		emit_alu(ctx, nir_instr_as_alu(instr));
		break;
	case nir_instr_type_intrinsic:
		emit_intrinisic(ctx, nir_instr_as_intrinsic(instr));
		break;
	case nir_instr_type_load_const:
		emit_load_const(ctx, nir_instr_as_load_const(instr));
		break;
	case nir_instr_type_ssa_undef:
		emit_undef(ctx, nir_instr_as_ssa_undef(instr));
		break;
	case nir_instr_type_tex: {
		nir_tex_instr *tex = nir_instr_as_tex(instr);
		/* couple tex instructions get special-cased:
		 */
		switch (tex->op) {
		case nir_texop_txs:
			emit_tex_txs(ctx, tex);
			break;
		case nir_texop_query_levels:
			emit_tex_query_levels(ctx, tex);
			break;
		default:
			emit_tex(ctx, tex);
			break;
		}
		break;
	}
	case nir_instr_type_call:
	case nir_instr_type_jump:
	case nir_instr_type_phi:
	case nir_instr_type_parallel_copy:
		compile_error(ctx, "Unhandled NIR instruction type: %d\n", instr->type);
		break;
	}
}

static void
emit_block(struct ir3_compile *ctx, nir_block *block)
{
	nir_foreach_instr(block, instr) {
		emit_instr(ctx, instr);
		if (ctx->error)
			return;
	}
}

static void
emit_function(struct ir3_compile *ctx, nir_function_impl *impl)
{
	foreach_list_typed(nir_cf_node, node, node, &impl->body) {
		switch (node->type) {
		case nir_cf_node_block:
			emit_block(ctx, nir_cf_node_as_block(node));
			break;
		case nir_cf_node_if:
		case nir_cf_node_loop:
		case nir_cf_node_function:
			compile_error(ctx, "TODO\n");
			break;
		}
		if (ctx->error)
			return;
	}
}

static void
setup_input(struct ir3_compile *ctx, nir_variable *in)
{
	struct ir3_shader_variant *so = ctx->so;
	unsigned array_len = MAX2(glsl_get_length(in->type), 1);
	unsigned ncomp = glsl_get_components(in->type);
	/* XXX: map loc slots to semantics */
	unsigned semantic_name = in->data.location;
	unsigned semantic_index = in->data.index;
	unsigned n = in->data.driver_location;

	DBG("; in: %u:%u, len=%ux%u, loc=%u\n",
			semantic_name, semantic_index, array_len,
			ncomp, n);

	so->inputs[n].semantic =
			ir3_semantic_name(semantic_name, semantic_index);
	so->inputs[n].compmask = (1 << ncomp) - 1;
	so->inputs[n].inloc = ctx->next_inloc;
	so->inputs[n].interpolate = 0;
	so->inputs_count = MAX2(so->inputs_count, n + 1);

	/* the fdN_program_emit() code expects tgsi consts here, so map
	 * things back to tgsi for now:
	 */
	switch (in->data.interpolation) {
	case INTERP_QUALIFIER_FLAT:
		so->inputs[n].interpolate = TGSI_INTERPOLATE_CONSTANT;
		break;
	case INTERP_QUALIFIER_NOPERSPECTIVE:
		so->inputs[n].interpolate = TGSI_INTERPOLATE_LINEAR;
		break;
	case INTERP_QUALIFIER_SMOOTH:
		so->inputs[n].interpolate = TGSI_INTERPOLATE_PERSPECTIVE;
		break;
	}

	for (int i = 0; i < ncomp; i++) {
		struct ir3_instruction *instr = NULL;
		unsigned idx = (n * 4) + i;

		if (ctx->so->type == SHADER_FRAGMENT) {
			if (semantic_name == TGSI_SEMANTIC_POSITION) {
				so->inputs[n].bary = false;
				so->frag_coord = true;
				instr = create_frag_coord(ctx, i);
			} else if (semantic_name == TGSI_SEMANTIC_FACE) {
				so->inputs[n].bary = false;
				so->frag_face = true;
				instr = create_frag_face(ctx, i);
			} else {
				bool use_ldlv = false;

				/* with NIR, we need to infer TGSI_INTERPOLATE_COLOR
				 * from the semantic name:
				 */
				if ((in->data.interpolation == INTERP_QUALIFIER_NONE) &&
						((semantic_name == TGSI_SEMANTIC_COLOR) ||
							(semantic_name == TGSI_SEMANTIC_BCOLOR)))
					so->inputs[n].interpolate = TGSI_INTERPOLATE_COLOR;

				if (ctx->flat_bypass) {
					/* with NIR, we need to infer TGSI_INTERPOLATE_COLOR
					 * from the semantic name:
					 */
					switch (so->inputs[n].interpolate) {
					case TGSI_INTERPOLATE_COLOR:
						if (!ctx->so->key.rasterflat)
							break;
						/* fallthrough */
					case TGSI_INTERPOLATE_CONSTANT:
						use_ldlv = true;
						break;
					}
				}

				so->inputs[n].bary = true;

				instr = create_frag_input(ctx,
						so->inputs[n].inloc + i - 8, use_ldlv);
			}
		} else {
			instr = create_input(ctx->block, NULL, idx);
		}

		ctx->block->inputs[idx] = instr;
	}

	if (so->inputs[n].bary || (ctx->so->type == SHADER_VERTEX)) {
		ctx->next_inloc += ncomp;
		so->total_in += ncomp;
	}
}

static void
setup_output(struct ir3_compile *ctx, nir_variable *out)
{
	struct ir3_shader_variant *so = ctx->so;
	unsigned array_len = MAX2(glsl_get_length(out->type), 1);
	unsigned ncomp = glsl_get_components(out->type);
	/* XXX: map loc slots to semantics */
	unsigned semantic_name = out->data.location;
	unsigned semantic_index = out->data.index;
	unsigned n = out->data.driver_location;
	unsigned comp = 0;

	DBG("; out: %u:%u, len=%ux%u, loc=%u\n",
			semantic_name, semantic_index, array_len,
			ncomp, n);

	if (ctx->so->type == SHADER_VERTEX) {
		switch (semantic_name) {
		case TGSI_SEMANTIC_POSITION:
			so->writes_pos = true;
			break;
		case TGSI_SEMANTIC_PSIZE:
			so->writes_psize = true;
			break;
		case TGSI_SEMANTIC_COLOR:
		case TGSI_SEMANTIC_BCOLOR:
		case TGSI_SEMANTIC_GENERIC:
		case TGSI_SEMANTIC_FOG:
		case TGSI_SEMANTIC_TEXCOORD:
			break;
		default:
			compile_error(ctx, "unknown VS semantic name: %s\n",
					tgsi_semantic_names[semantic_name]);
		}
	} else {
		switch (semantic_name) {
		case TGSI_SEMANTIC_POSITION:
			comp = 2;  /* tgsi will write to .z component */
			so->writes_pos = true;
			break;
		case TGSI_SEMANTIC_COLOR:
			break;
		default:
			compile_error(ctx, "unknown FS semantic name: %s\n",
					tgsi_semantic_names[semantic_name]);
		}
	}

	compile_assert(ctx, n < ARRAY_SIZE(so->outputs));

	so->outputs[n].semantic =
			ir3_semantic_name(semantic_name, semantic_index);
	so->outputs[n].regid = regid(n, comp);
	so->outputs_count = MAX2(so->outputs_count, n + 1);

	for (int i = 0; i < ncomp; i++) {
		unsigned idx = (n * 4) + i;

		ctx->block->outputs[idx] = create_immed(ctx->block, fui(0.0));
	}
}

static void
emit_instructions(struct ir3_compile *ctx)
{
	unsigned ninputs  = exec_list_length(&ctx->s->inputs) * 4;
	unsigned noutputs = exec_list_length(&ctx->s->outputs) * 4;

	/* we need to allocate big enough outputs array so that
	 * we can stuff the kill's at the end.  Likewise for vtx
	 * shaders, we need to leave room for sysvals:
	 */
	if (ctx->so->type == SHADER_FRAGMENT) {
		noutputs += ARRAY_SIZE(ctx->kill);
	} else if (ctx->so->type == SHADER_VERTEX) {
		ninputs += 8;
	}

	ctx->block = ir3_block_create(ctx->ir, 0, ninputs, noutputs);

	if (ctx->so->type == SHADER_FRAGMENT) {
		ctx->block->noutputs -= ARRAY_SIZE(ctx->kill);
	} else if (ctx->so->type == SHADER_VERTEX) {
		ctx->block->ninputs -= 8;
	}

	/* for fragment shader, we have a single input register (usually
	 * r0.xy) which is used as the base for bary.f varying fetch instrs:
	 */
	if (ctx->so->type == SHADER_FRAGMENT) {
		// TODO maybe a helper for fi since we need it a few places..
		struct ir3_instruction *instr;
		instr = ir3_instr_create(ctx->block, -1, OPC_META_FI);
		ir3_reg_create(instr, 0, 0);
		ir3_reg_create(instr, 0, IR3_REG_SSA);    /* r0.x */
		ir3_reg_create(instr, 0, IR3_REG_SSA);    /* r0.y */
		ctx->frag_pos = instr;
	}

	/* Setup inputs: */
	foreach_list_typed(nir_variable, var, node, &ctx->s->inputs) {
		setup_input(ctx, var);
	}

	/* Setup outputs: */
	foreach_list_typed(nir_variable, var, node, &ctx->s->outputs) {
		setup_output(ctx, var);
	}

	/* Setup variables (which should only be arrays): */
	foreach_list_typed(nir_variable, var, node, &ctx->s->globals) {
		declare_var(ctx, var);
	}

	/* Find the main function and emit the body: */
	nir_foreach_overload(ctx->s, overload) {
		compile_assert(ctx, strcmp(overload->function->name, "main") == 0);
		compile_assert(ctx, overload->impl);
		emit_function(ctx, overload->impl);
		if (ctx->error)
			return;
	}
}

/* from NIR perspective, we actually have inputs.  But most of the "inputs"
 * for a fragment shader are just bary.f instructions.  The *actual* inputs
 * from the hw perspective are the frag_pos and optionally frag_coord and
 * frag_face.
 */
static void
fixup_frag_inputs(struct ir3_compile *ctx)
{
	struct ir3_shader_variant *so = ctx->so;
	struct ir3_block *block = ctx->block;
	struct ir3_instruction **inputs;
	struct ir3_instruction *instr;
	int n, regid = 0;

	block->ninputs = 0;

	n  = 4;  /* always have frag_pos */
	n += COND(so->frag_face, 4);
	n += COND(so->frag_coord, 4);

	inputs = ir3_alloc(ctx->ir, n * (sizeof(struct ir3_instruction *)));

	if (so->frag_face) {
		/* this ultimately gets assigned to hr0.x so doesn't conflict
		 * with frag_coord/frag_pos..
		 */
		inputs[block->ninputs++] = ctx->frag_face;
		ctx->frag_face->regs[0]->num = 0;

		/* remaining channels not used, but let's avoid confusing
		 * other parts that expect inputs to come in groups of vec4
		 */
		inputs[block->ninputs++] = NULL;
		inputs[block->ninputs++] = NULL;
		inputs[block->ninputs++] = NULL;
	}

	/* since we don't know where to set the regid for frag_coord,
	 * we have to use r0.x for it.  But we don't want to *always*
	 * use r1.x for frag_pos as that could increase the register
	 * footprint on simple shaders:
	 */
	if (so->frag_coord) {
		ctx->frag_coord[0]->regs[0]->num = regid++;
		ctx->frag_coord[1]->regs[0]->num = regid++;
		ctx->frag_coord[2]->regs[0]->num = regid++;
		ctx->frag_coord[3]->regs[0]->num = regid++;

		inputs[block->ninputs++] = ctx->frag_coord[0];
		inputs[block->ninputs++] = ctx->frag_coord[1];
		inputs[block->ninputs++] = ctx->frag_coord[2];
		inputs[block->ninputs++] = ctx->frag_coord[3];
	}

	/* we always have frag_pos: */
	so->pos_regid = regid;

	/* r0.x */
	instr = create_input(block, NULL, block->ninputs);
	instr->regs[0]->num = regid++;
	inputs[block->ninputs++] = instr;
	ctx->frag_pos->regs[1]->instr = instr;

	/* r0.y */
	instr = create_input(block, NULL, block->ninputs);
	instr->regs[0]->num = regid++;
	inputs[block->ninputs++] = instr;
	ctx->frag_pos->regs[2]->instr = instr;

	block->inputs = inputs;
}

static void
compile_dump(struct ir3_compile *ctx)
{
	const char *name = (ctx->so->type == SHADER_VERTEX) ? "vert" : "frag";
	static unsigned n = 0;
	char fname[16];
	FILE *f;
	snprintf(fname, sizeof(fname), "%s-%04u.dot", name, n++);
	f = fopen(fname, "w");
	if (!f)
		return;
	ir3_block_depth(ctx->block);
	ir3_dump(ctx->ir, name, ctx->block, f);
	fclose(f);
}

int
ir3_compile_shader_nir(struct ir3_shader_variant *so,
		const struct tgsi_token *tokens, struct ir3_shader_key key)
{
	struct ir3_compile *ctx;
	struct ir3_block *block;
	struct ir3_instruction **inputs;
	unsigned i, j, actual_in;
	int ret = 0, max_bary;

	assert(!so->ir);

	so->ir = ir3_create();

	assert(so->ir);

	ctx = compile_init(so, tokens);
	if (!ctx) {
		DBG("INIT failed!");
		ret = -1;
		goto out;
	}

	emit_instructions(ctx);

	if (ctx->error) {
		DBG("EMIT failed!");
		ret = -1;
		goto out;
	}

	block = ctx->block;
	so->ir->block = block;

	/* keep track of the inputs from TGSI perspective.. */
	inputs = block->inputs;

	/* but fixup actual inputs for frag shader: */
	if (so->type == SHADER_FRAGMENT)
		fixup_frag_inputs(ctx);

	/* at this point, for binning pass, throw away unneeded outputs: */
	if (key.binning_pass) {
		for (i = 0, j = 0; i < so->outputs_count; i++) {
			unsigned name = sem2name(so->outputs[i].semantic);
			unsigned idx = sem2idx(so->outputs[i].semantic);

			/* throw away everything but first position/psize */
			if ((idx == 0) && ((name == TGSI_SEMANTIC_POSITION) ||
					(name == TGSI_SEMANTIC_PSIZE))) {
				if (i != j) {
					so->outputs[j] = so->outputs[i];
					block->outputs[(j*4)+0] = block->outputs[(i*4)+0];
					block->outputs[(j*4)+1] = block->outputs[(i*4)+1];
					block->outputs[(j*4)+2] = block->outputs[(i*4)+2];
					block->outputs[(j*4)+3] = block->outputs[(i*4)+3];
				}
				j++;
			}
		}
		so->outputs_count = j;
		block->noutputs = j * 4;
	}

	/* if we want half-precision outputs, mark the output registers
	 * as half:
	 */
	if (key.half_precision) {
		for (i = 0; i < block->noutputs; i++) {
			if (!block->outputs[i])
				continue;
			block->outputs[i]->regs[0]->flags |= IR3_REG_HALF;
		}
	}

	/* at this point, we want the kill's in the outputs array too,
	 * so that they get scheduled (since they have no dst).. we've
	 * already ensured that the array is big enough in push_block():
	 */
	if (so->type == SHADER_FRAGMENT) {
		for (i = 0; i < ctx->kill_count; i++)
			block->outputs[block->noutputs++] = ctx->kill[i];
	}

	if (fd_mesa_debug & FD_DBG_OPTDUMP)
		compile_dump(ctx);

	if (fd_mesa_debug & FD_DBG_OPTMSGS) {
		printf("BEFORE CP:\n");
		ir3_dump_instr_list(block->head);
	}

	ir3_block_depth(block);

	ir3_block_cp(block);

	if (fd_mesa_debug & FD_DBG_OPTMSGS) {
		printf("BEFORE GROUPING:\n");
		ir3_dump_instr_list(block->head);
	}

	/* Group left/right neighbors, inserting mov's where needed to
	 * solve conflicts:
	 */
	ir3_block_group(block);

	if (fd_mesa_debug & FD_DBG_OPTDUMP)
		compile_dump(ctx);

	ir3_block_depth(block);

	if (fd_mesa_debug & FD_DBG_OPTMSGS) {
		printf("AFTER DEPTH:\n");
		ir3_dump_instr_list(block->head);
	}

	ret = ir3_block_sched(block);
	if (ret) {
		DBG("SCHED failed!");
		goto out;
	}

	if (fd_mesa_debug & FD_DBG_OPTMSGS) {
		printf("AFTER SCHED:\n");
		ir3_dump_instr_list(block->head);
	}

	ret = ir3_block_ra(block, so->type, so->frag_coord, so->frag_face);
	if (ret) {
		DBG("RA failed!");
		goto out;
	}

	if (fd_mesa_debug & FD_DBG_OPTMSGS) {
		printf("AFTER RA:\n");
		ir3_dump_instr_list(block->head);
	}

	ir3_block_legalize(block, &so->has_samp, &max_bary);

	/* fixup input/outputs: */
	for (i = 0; i < so->outputs_count; i++) {
		so->outputs[i].regid = block->outputs[i*4]->regs[0]->num;
		/* preserve hack for depth output.. tgsi writes depth to .z,
		 * but what we give the hw is the scalar register:
		 */
		if ((so->type == SHADER_FRAGMENT) &&
			(sem2name(so->outputs[i].semantic) == TGSI_SEMANTIC_POSITION))
			so->outputs[i].regid += 2;
	}

	/* Note that some or all channels of an input may be unused: */
	actual_in = 0;
	for (i = 0; i < so->inputs_count; i++) {
		unsigned j, regid = ~0, compmask = 0;
		so->inputs[i].ncomp = 0;
		for (j = 0; j < 4; j++) {
			struct ir3_instruction *in = inputs[(i*4) + j];
			if (in) {
				compmask |= (1 << j);
				regid = in->regs[0]->num - j;
				actual_in++;
				so->inputs[i].ncomp++;
			}
		}
		so->inputs[i].regid = regid;
		so->inputs[i].compmask = compmask;
	}

	/* fragment shader always gets full vec4's even if it doesn't
	 * fetch all components, but vertex shader we need to update
	 * with the actual number of components fetch, otherwise thing
	 * will hang due to mismaptch between VFD_DECODE's and
	 * TOTALATTRTOVS
	 */
	if (so->type == SHADER_VERTEX)
		so->total_in = actual_in;
	else
		so->total_in = align(max_bary + 1, 4);

out:
	if (ret) {
		ir3_destroy(so->ir);
		so->ir = NULL;
	}
	compile_free(ctx);

	return ret;
}
