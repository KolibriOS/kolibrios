/*
 * Copyright 2012 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *	Tom Stellard <thomas.stellard@amd.com>
 *	Michel Dänzer <michel.daenzer@amd.com>
 *      Christian König <christian.koenig@amd.com>
 */

#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_gather.h"
#include "gallivm/lp_bld_intr.h"
#include "gallivm/lp_bld_logic.h"
#include "gallivm/lp_bld_arit.h"
#include "gallivm/lp_bld_flow.h"
#include "radeon/r600_cs.h"
#include "radeon/radeon_llvm.h"
#include "radeon/radeon_elf_util.h"
#include "radeon/radeon_llvm_emit.h"
#include "util/u_memory.h"
#include "util/u_pstipple.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_util.h"
#include "tgsi/tgsi_dump.h"

#include "si_pipe.h"
#include "si_shader.h"
#include "sid.h"

#include <errno.h>

static const char *scratch_rsrc_dword0_symbol =
	"SCRATCH_RSRC_DWORD0";

static const char *scratch_rsrc_dword1_symbol =
	"SCRATCH_RSRC_DWORD1";

struct si_shader_output_values
{
	LLVMValueRef values[4];
	unsigned name;
	unsigned sid;
};

struct si_shader_context
{
	struct radeon_llvm_context radeon_bld;
	struct si_shader *shader;
	struct si_screen *screen;
	unsigned type; /* TGSI_PROCESSOR_* specifies the type of shader. */
	int param_streamout_config;
	int param_streamout_write_index;
	int param_streamout_offset[4];
	int param_vertex_id;
	int param_instance_id;
	LLVMTargetMachineRef tm;
	LLVMValueRef const_md;
	LLVMValueRef const_resource[SI_NUM_CONST_BUFFERS];
	LLVMValueRef ddxy_lds;
	LLVMValueRef *constants[SI_NUM_CONST_BUFFERS];
	LLVMValueRef resources[SI_NUM_SAMPLER_VIEWS];
	LLVMValueRef samplers[SI_NUM_SAMPLER_STATES];
	LLVMValueRef so_buffers[4];
	LLVMValueRef esgs_ring;
	LLVMValueRef gsvs_ring;
	LLVMValueRef gs_next_vertex;
};

static struct si_shader_context * si_shader_context(
	struct lp_build_tgsi_context * bld_base)
{
	return (struct si_shader_context *)bld_base;
}


#define PERSPECTIVE_BASE 0
#define LINEAR_BASE 9

#define SAMPLE_OFFSET 0
#define CENTER_OFFSET 2
#define CENTROID_OFSET 4

#define USE_SGPR_MAX_SUFFIX_LEN 5
#define CONST_ADDR_SPACE 2
#define LOCAL_ADDR_SPACE 3
#define USER_SGPR_ADDR_SPACE 8


#define SENDMSG_GS 2
#define SENDMSG_GS_DONE 3

#define SENDMSG_GS_OP_NOP      (0 << 4)
#define SENDMSG_GS_OP_CUT      (1 << 4)
#define SENDMSG_GS_OP_EMIT     (2 << 4)
#define SENDMSG_GS_OP_EMIT_CUT (3 << 4)

/**
 * Returns a unique index for a semantic name and index. The index must be
 * less than 64, so that a 64-bit bitmask of used inputs or outputs can be
 * calculated.
 */
unsigned si_shader_io_get_unique_index(unsigned semantic_name, unsigned index)
{
	switch (semantic_name) {
	case TGSI_SEMANTIC_POSITION:
		return 0;
	case TGSI_SEMANTIC_PSIZE:
		return 1;
	case TGSI_SEMANTIC_CLIPDIST:
		assert(index <= 1);
		return 2 + index;
	case TGSI_SEMANTIC_CLIPVERTEX:
		return 4;
	case TGSI_SEMANTIC_COLOR:
		assert(index <= 1);
		return 5 + index;
	case TGSI_SEMANTIC_BCOLOR:
		assert(index <= 1);
		return 7 + index;
	case TGSI_SEMANTIC_FOG:
		return 9;
	case TGSI_SEMANTIC_EDGEFLAG:
		return 10;
	case TGSI_SEMANTIC_GENERIC:
		assert(index <= 63-11);
		return 11 + index;
	default:
		assert(0);
		return 63;
	}
}

/**
 * Given a semantic name and index of a parameter and a mask of used parameters
 * (inputs or outputs), return the index of the parameter in the list of all
 * used parameters.
 *
 * For example, assume this list of parameters:
 *   POSITION, PSIZE, GENERIC0, GENERIC2
 * which has the mask:
 *   11000000000101
 * Then:
 *   querying POSITION returns 0,
 *   querying PSIZE returns 1,
 *   querying GENERIC0 returns 2,
 *   querying GENERIC2 returns 3.
 *
 * Which can be used as an offset to a parameter buffer in units of vec4s.
 */
static int get_param_index(unsigned semantic_name, unsigned index,
			   uint64_t mask)
{
	unsigned unique_index = si_shader_io_get_unique_index(semantic_name, index);
	int i, param_index = 0;

	/* If not present... */
	if (!((1llu << unique_index) & mask))
		return -1;

	for (i = 0; mask; i++) {
		uint64_t bit = 1llu << i;

		if (bit & mask) {
			if (i == unique_index)
				return param_index;

			mask &= ~bit;
			param_index++;
		}
	}

	assert(!"unreachable");
	return -1;
}

/**
 * Get the value of a shader input parameter and extract a bitfield.
 */
static LLVMValueRef unpack_param(struct si_shader_context *si_shader_ctx,
				 unsigned param, unsigned rshift,
				 unsigned bitwidth)
{
	struct gallivm_state *gallivm = &si_shader_ctx->radeon_bld.gallivm;
	LLVMValueRef value = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
					  param);

	if (rshift)
		value = LLVMBuildLShr(gallivm->builder, value,
				      lp_build_const_int32(gallivm, rshift), "");

	if (rshift + bitwidth < 32) {
		unsigned mask = (1 << bitwidth) - 1;
		value = LLVMBuildAnd(gallivm->builder, value,
				     lp_build_const_int32(gallivm, mask), "");
	}

	return value;
}

/**
 * Build an LLVM bytecode indexed load using LLVMBuildGEP + LLVMBuildLoad.
 * It's equivalent to doing a load from &base_ptr[index].
 *
 * \param base_ptr  Where the array starts.
 * \param index     The element index into the array.
 */
static LLVMValueRef build_indexed_load(struct si_shader_context *si_shader_ctx,
				       LLVMValueRef base_ptr, LLVMValueRef index)
{
	struct lp_build_tgsi_context *bld_base = &si_shader_ctx->radeon_bld.soa.bld_base;
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	LLVMValueRef indices[2], pointer;

	indices[0] = bld_base->uint_bld.zero;
	indices[1] = index;

	pointer = LLVMBuildGEP(gallivm->builder, base_ptr, indices, 2, "");
	return LLVMBuildLoad(gallivm->builder, pointer, "");
}

/**
 * Do a load from &base_ptr[index], but also add a flag that it's loading
 * a constant.
 */
static LLVMValueRef build_indexed_load_const(
	struct si_shader_context * si_shader_ctx,
	LLVMValueRef base_ptr, LLVMValueRef index)
{
	LLVMValueRef result = build_indexed_load(si_shader_ctx, base_ptr, index);
	LLVMSetMetadata(result, 1, si_shader_ctx->const_md);
	return result;
}

static LLVMValueRef get_instance_index_for_fetch(
	struct radeon_llvm_context * radeon_bld,
	unsigned divisor)
{
	struct si_shader_context *si_shader_ctx =
		si_shader_context(&radeon_bld->soa.bld_base);
	struct gallivm_state * gallivm = radeon_bld->soa.bld_base.base.gallivm;

	LLVMValueRef result = LLVMGetParam(radeon_bld->main_fn,
					   si_shader_ctx->param_instance_id);

	/* The division must be done before START_INSTANCE is added. */
	if (divisor > 1)
		result = LLVMBuildUDiv(gallivm->builder, result,
				lp_build_const_int32(gallivm, divisor), "");

	return LLVMBuildAdd(gallivm->builder, result, LLVMGetParam(
			radeon_bld->main_fn, SI_PARAM_START_INSTANCE), "");
}

static void declare_input_vs(
	struct radeon_llvm_context *radeon_bld,
	unsigned input_index,
	const struct tgsi_full_declaration *decl)
{
	struct lp_build_context *base = &radeon_bld->soa.bld_base.base;
	struct gallivm_state *gallivm = base->gallivm;
	struct si_shader_context *si_shader_ctx =
		si_shader_context(&radeon_bld->soa.bld_base);
	unsigned divisor = si_shader_ctx->shader->key.vs.instance_divisors[input_index];

	unsigned chan;

	LLVMValueRef t_list_ptr;
	LLVMValueRef t_offset;
	LLVMValueRef t_list;
	LLVMValueRef attribute_offset;
	LLVMValueRef buffer_index;
	LLVMValueRef args[3];
	LLVMTypeRef vec4_type;
	LLVMValueRef input;

	/* Load the T list */
	t_list_ptr = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn, SI_PARAM_VERTEX_BUFFER);

	t_offset = lp_build_const_int32(gallivm, input_index);

	t_list = build_indexed_load_const(si_shader_ctx, t_list_ptr, t_offset);

	/* Build the attribute offset */
	attribute_offset = lp_build_const_int32(gallivm, 0);

	if (divisor) {
		/* Build index from instance ID, start instance and divisor */
		si_shader_ctx->shader->uses_instanceid = true;
		buffer_index = get_instance_index_for_fetch(&si_shader_ctx->radeon_bld, divisor);
	} else {
		/* Load the buffer index for vertices. */
		LLVMValueRef vertex_id = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
						      si_shader_ctx->param_vertex_id);
		LLVMValueRef base_vertex = LLVMGetParam(radeon_bld->main_fn,
							SI_PARAM_BASE_VERTEX);
		buffer_index = LLVMBuildAdd(gallivm->builder, base_vertex, vertex_id, "");
	}

	vec4_type = LLVMVectorType(base->elem_type, 4);
	args[0] = t_list;
	args[1] = attribute_offset;
	args[2] = buffer_index;
	input = build_intrinsic(gallivm->builder,
		"llvm.SI.vs.load.input", vec4_type, args, 3,
		LLVMReadNoneAttribute | LLVMNoUnwindAttribute);

	/* Break up the vec4 into individual components */
	for (chan = 0; chan < 4; chan++) {
		LLVMValueRef llvm_chan = lp_build_const_int32(gallivm, chan);
		/* XXX: Use a helper function for this.  There is one in
 		 * tgsi_llvm.c. */
		si_shader_ctx->radeon_bld.inputs[radeon_llvm_reg_index_soa(input_index, chan)] =
				LLVMBuildExtractElement(gallivm->builder,
				input, llvm_chan, "");
	}
}

static LLVMValueRef fetch_input_gs(
	struct lp_build_tgsi_context *bld_base,
	const struct tgsi_full_src_register *reg,
	enum tgsi_opcode_type type,
	unsigned swizzle)
{
	struct lp_build_context *base = &bld_base->base;
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct si_shader *shader = si_shader_ctx->shader;
	struct lp_build_context *uint =	&si_shader_ctx->radeon_bld.soa.bld_base.uint_bld;
	struct gallivm_state *gallivm = base->gallivm;
	LLVMTypeRef i32 = LLVMInt32TypeInContext(gallivm->context);
	LLVMValueRef vtx_offset;
	LLVMValueRef args[9];
	unsigned vtx_offset_param;
	struct tgsi_shader_info *info = &shader->selector->info;
	unsigned semantic_name = info->input_semantic_name[reg->Register.Index];
	unsigned semantic_index = info->input_semantic_index[reg->Register.Index];

	if (swizzle != ~0 && semantic_name == TGSI_SEMANTIC_PRIMID) {
		if (swizzle == 0)
			return LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
					    SI_PARAM_PRIMITIVE_ID);
		else
			return uint->zero;
	}

	if (!reg->Register.Dimension)
		return NULL;

	if (swizzle == ~0) {
		LLVMValueRef values[TGSI_NUM_CHANNELS];
		unsigned chan;
		for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
			values[chan] = fetch_input_gs(bld_base, reg, type, chan);
		}
		return lp_build_gather_values(bld_base->base.gallivm, values,
					      TGSI_NUM_CHANNELS);
	}

	/* Get the vertex offset parameter */
	vtx_offset_param = reg->Dimension.Index;
	if (vtx_offset_param < 2) {
		vtx_offset_param += SI_PARAM_VTX0_OFFSET;
	} else {
		assert(vtx_offset_param < 6);
		vtx_offset_param += SI_PARAM_VTX2_OFFSET - 2;
	}
	vtx_offset = lp_build_mul_imm(uint,
				      LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
						   vtx_offset_param),
				      4);

	args[0] = si_shader_ctx->esgs_ring;
	args[1] = vtx_offset;
	args[2] = lp_build_const_int32(gallivm,
				       (get_param_index(semantic_name, semantic_index,
							shader->selector->gs_used_inputs) * 4 +
					swizzle) * 256);
	args[3] = uint->zero;
	args[4] = uint->one;  /* OFFEN */
	args[5] = uint->zero; /* IDXEN */
	args[6] = uint->one;  /* GLC */
	args[7] = uint->zero; /* SLC */
	args[8] = uint->zero; /* TFE */

	return LLVMBuildBitCast(gallivm->builder,
				build_intrinsic(gallivm->builder,
						"llvm.SI.buffer.load.dword.i32.i32",
						i32, args, 9,
						LLVMReadOnlyAttribute | LLVMNoUnwindAttribute),
				tgsi2llvmtype(bld_base, type), "");
}

static void declare_input_fs(
	struct radeon_llvm_context *radeon_bld,
	unsigned input_index,
	const struct tgsi_full_declaration *decl)
{
	struct lp_build_context *base = &radeon_bld->soa.bld_base.base;
	struct si_shader_context *si_shader_ctx =
		si_shader_context(&radeon_bld->soa.bld_base);
	struct si_shader *shader = si_shader_ctx->shader;
	struct lp_build_context *uint =	&radeon_bld->soa.bld_base.uint_bld;
	struct gallivm_state *gallivm = base->gallivm;
	LLVMTypeRef input_type = LLVMFloatTypeInContext(gallivm->context);
	LLVMValueRef main_fn = radeon_bld->main_fn;

	LLVMValueRef interp_param;
	const char * intr_name;

	/* This value is:
	 * [15:0] NewPrimMask (Bit mask for each quad.  It is set it the
	 *                     quad begins a new primitive.  Bit 0 always needs
	 *                     to be unset)
	 * [32:16] ParamOffset
	 *
	 */
	LLVMValueRef params = LLVMGetParam(main_fn, SI_PARAM_PRIM_MASK);
	LLVMValueRef attr_number;

	unsigned chan;

	if (decl->Semantic.Name == TGSI_SEMANTIC_POSITION) {
		for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
			unsigned soa_index =
				radeon_llvm_reg_index_soa(input_index, chan);
			radeon_bld->inputs[soa_index] =
				LLVMGetParam(main_fn, SI_PARAM_POS_X_FLOAT + chan);

			if (chan == 3)
				/* RCP for fragcoord.w */
				radeon_bld->inputs[soa_index] =
					LLVMBuildFDiv(gallivm->builder,
						      lp_build_const_float(gallivm, 1.0f),
						      radeon_bld->inputs[soa_index],
						      "");
		}
		return;
	}

	if (decl->Semantic.Name == TGSI_SEMANTIC_FACE) {
		radeon_bld->inputs[radeon_llvm_reg_index_soa(input_index, 0)] =
			LLVMGetParam(main_fn, SI_PARAM_FRONT_FACE);
		radeon_bld->inputs[radeon_llvm_reg_index_soa(input_index, 1)] =
		radeon_bld->inputs[radeon_llvm_reg_index_soa(input_index, 2)] =
			lp_build_const_float(gallivm, 0.0f);
		radeon_bld->inputs[radeon_llvm_reg_index_soa(input_index, 3)] =
			lp_build_const_float(gallivm, 1.0f);

		return;
	}

	shader->ps_input_param_offset[input_index] = shader->nparam++;
	attr_number = lp_build_const_int32(gallivm,
					   shader->ps_input_param_offset[input_index]);

	switch (decl->Interp.Interpolate) {
	case TGSI_INTERPOLATE_CONSTANT:
		interp_param = 0;
		break;
	case TGSI_INTERPOLATE_LINEAR:
		if (decl->Interp.Location == TGSI_INTERPOLATE_LOC_SAMPLE)
			interp_param = LLVMGetParam(main_fn, SI_PARAM_LINEAR_SAMPLE);
		else if (decl->Interp.Location == TGSI_INTERPOLATE_LOC_CENTROID)
			interp_param = LLVMGetParam(main_fn, SI_PARAM_LINEAR_CENTROID);
		else
			interp_param = LLVMGetParam(main_fn, SI_PARAM_LINEAR_CENTER);
		break;
	case TGSI_INTERPOLATE_COLOR:
	case TGSI_INTERPOLATE_PERSPECTIVE:
		if (decl->Interp.Location == TGSI_INTERPOLATE_LOC_SAMPLE)
			interp_param = LLVMGetParam(main_fn, SI_PARAM_PERSP_SAMPLE);
		else if (decl->Interp.Location == TGSI_INTERPOLATE_LOC_CENTROID)
			interp_param = LLVMGetParam(main_fn, SI_PARAM_PERSP_CENTROID);
		else
			interp_param = LLVMGetParam(main_fn, SI_PARAM_PERSP_CENTER);
		break;
	default:
		fprintf(stderr, "Warning: Unhandled interpolation mode.\n");
		return;
	}

	/* fs.constant returns the param from the middle vertex, so it's not
	 * really useful for flat shading. It's meant to be used for custom
	 * interpolation (but the intrinsic can't fetch from the other two
	 * vertices).
	 *
	 * Luckily, it doesn't matter, because we rely on the FLAT_SHADE state
	 * to do the right thing. The only reason we use fs.constant is that
	 * fs.interp cannot be used on integers, because they can be equal
	 * to NaN.
	 */
	intr_name = interp_param ? "llvm.SI.fs.interp" : "llvm.SI.fs.constant";

	if (decl->Semantic.Name == TGSI_SEMANTIC_COLOR &&
	    si_shader_ctx->shader->key.ps.color_two_side) {
		LLVMValueRef args[4];
		LLVMValueRef face, is_face_positive;
		LLVMValueRef back_attr_number =
			lp_build_const_int32(gallivm,
					     shader->ps_input_param_offset[input_index] + 1);

		face = LLVMGetParam(main_fn, SI_PARAM_FRONT_FACE);

		is_face_positive = LLVMBuildFCmp(gallivm->builder,
						 LLVMRealOGT, face,
						 lp_build_const_float(gallivm, 0.0f),
						 "");

		args[2] = params;
		args[3] = interp_param;
		for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
			LLVMValueRef llvm_chan = lp_build_const_int32(gallivm, chan);
			unsigned soa_index = radeon_llvm_reg_index_soa(input_index, chan);
			LLVMValueRef front, back;

			args[0] = llvm_chan;
			args[1] = attr_number;
			front = build_intrinsic(gallivm->builder, intr_name,
						input_type, args, args[3] ? 4 : 3,
						LLVMReadNoneAttribute | LLVMNoUnwindAttribute);

			args[1] = back_attr_number;
			back = build_intrinsic(gallivm->builder, intr_name,
					       input_type, args, args[3] ? 4 : 3,
					       LLVMReadNoneAttribute | LLVMNoUnwindAttribute);

			radeon_bld->inputs[soa_index] =
				LLVMBuildSelect(gallivm->builder,
						is_face_positive,
						front,
						back,
						"");
		}

		shader->nparam++;
	} else if (decl->Semantic.Name == TGSI_SEMANTIC_FOG) {
		LLVMValueRef args[4];

		args[0] = uint->zero;
		args[1] = attr_number;
		args[2] = params;
		args[3] = interp_param;
		radeon_bld->inputs[radeon_llvm_reg_index_soa(input_index, 0)] =
			build_intrinsic(gallivm->builder, intr_name,
					input_type, args, args[3] ? 4 : 3,
					LLVMReadNoneAttribute | LLVMNoUnwindAttribute);
		radeon_bld->inputs[radeon_llvm_reg_index_soa(input_index, 1)] =
		radeon_bld->inputs[radeon_llvm_reg_index_soa(input_index, 2)] =
			lp_build_const_float(gallivm, 0.0f);
		radeon_bld->inputs[radeon_llvm_reg_index_soa(input_index, 3)] =
			lp_build_const_float(gallivm, 1.0f);
	} else {
		for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
			LLVMValueRef args[4];
			LLVMValueRef llvm_chan = lp_build_const_int32(gallivm, chan);
			unsigned soa_index = radeon_llvm_reg_index_soa(input_index, chan);
			args[0] = llvm_chan;
			args[1] = attr_number;
			args[2] = params;
			args[3] = interp_param;
			radeon_bld->inputs[soa_index] =
				build_intrinsic(gallivm->builder, intr_name,
						input_type, args, args[3] ? 4 : 3,
						LLVMReadNoneAttribute | LLVMNoUnwindAttribute);
		}
	}
}

static LLVMValueRef get_sample_id(struct radeon_llvm_context *radeon_bld)
{
	return unpack_param(si_shader_context(&radeon_bld->soa.bld_base),
			    SI_PARAM_ANCILLARY, 8, 4);
}

/**
 * Load a dword from a constant buffer.
 */
static LLVMValueRef buffer_load_const(LLVMBuilderRef builder, LLVMValueRef resource,
				      LLVMValueRef offset, LLVMTypeRef return_type)
{
	LLVMValueRef args[2] = {resource, offset};

	return build_intrinsic(builder, "llvm.SI.load.const", return_type, args, 2,
			       LLVMReadNoneAttribute | LLVMNoUnwindAttribute);
}

static void declare_system_value(
	struct radeon_llvm_context * radeon_bld,
	unsigned index,
	const struct tgsi_full_declaration *decl)
{
	struct si_shader_context *si_shader_ctx =
		si_shader_context(&radeon_bld->soa.bld_base);
	struct lp_build_context *uint_bld = &radeon_bld->soa.bld_base.uint_bld;
	struct gallivm_state *gallivm = &radeon_bld->gallivm;
	LLVMValueRef value = 0;

	switch (decl->Semantic.Name) {
	case TGSI_SEMANTIC_INSTANCEID:
		value = LLVMGetParam(radeon_bld->main_fn,
				     si_shader_ctx->param_instance_id);
		break;

	case TGSI_SEMANTIC_VERTEXID:
		value = LLVMBuildAdd(gallivm->builder,
				     LLVMGetParam(radeon_bld->main_fn,
						  si_shader_ctx->param_vertex_id),
				     LLVMGetParam(radeon_bld->main_fn,
						  SI_PARAM_BASE_VERTEX), "");
		break;

	case TGSI_SEMANTIC_VERTEXID_NOBASE:
		value = LLVMGetParam(radeon_bld->main_fn,
				     si_shader_ctx->param_vertex_id);
		break;

	case TGSI_SEMANTIC_BASEVERTEX:
		value = LLVMGetParam(radeon_bld->main_fn,
				     SI_PARAM_BASE_VERTEX);
		break;

	case TGSI_SEMANTIC_SAMPLEID:
		value = get_sample_id(radeon_bld);
		break;

	case TGSI_SEMANTIC_SAMPLEPOS:
	{
		LLVMBuilderRef builder = gallivm->builder;
		LLVMValueRef desc = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn, SI_PARAM_CONST);
		LLVMValueRef buf_index = lp_build_const_int32(gallivm, SI_DRIVER_STATE_CONST_BUF);
		LLVMValueRef resource = build_indexed_load_const(si_shader_ctx, desc, buf_index);

		/* offset = sample_id * 8  (8 = 2 floats containing samplepos.xy) */
		LLVMValueRef offset0 = lp_build_mul_imm(uint_bld, get_sample_id(radeon_bld), 8);
		LLVMValueRef offset1 = LLVMBuildAdd(builder, offset0, lp_build_const_int32(gallivm, 4), "");

		LLVMValueRef pos[4] = {
			buffer_load_const(builder, resource, offset0, radeon_bld->soa.bld_base.base.elem_type),
			buffer_load_const(builder, resource, offset1, radeon_bld->soa.bld_base.base.elem_type),
			lp_build_const_float(gallivm, 0),
			lp_build_const_float(gallivm, 0)
		};
		value = lp_build_gather_values(gallivm, pos, 4);
		break;
	}

	case TGSI_SEMANTIC_SAMPLEMASK:
		/* Smoothing isn't MSAA in GL, but it's MSAA in hardware.
		 * Therefore, force gl_SampleMaskIn to 1 for GL. */
		if (si_shader_ctx->shader->key.ps.poly_line_smoothing)
			value = uint_bld->one;
		else
			value = LLVMGetParam(radeon_bld->main_fn, SI_PARAM_SAMPLE_COVERAGE);
		break;

	default:
		assert(!"unknown system value");
		return;
	}

	radeon_bld->system_values[index] = value;
}

static LLVMValueRef fetch_constant(
	struct lp_build_tgsi_context * bld_base,
	const struct tgsi_full_src_register *reg,
	enum tgsi_opcode_type type,
	unsigned swizzle)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct lp_build_context * base = &bld_base->base;
	const struct tgsi_ind_register *ireg = &reg->Indirect;
	unsigned buf, idx;

	LLVMValueRef addr;
	LLVMValueRef result;

	if (swizzle == LP_CHAN_ALL) {
		unsigned chan;
		LLVMValueRef values[4];
		for (chan = 0; chan < TGSI_NUM_CHANNELS; ++chan)
			values[chan] = fetch_constant(bld_base, reg, type, chan);

		return lp_build_gather_values(bld_base->base.gallivm, values, 4);
	}

	buf = reg->Register.Dimension ? reg->Dimension.Index : 0;
	idx = reg->Register.Index * 4 + swizzle;

	if (!reg->Register.Indirect)
		return bitcast(bld_base, type, si_shader_ctx->constants[buf][idx]);

	addr = si_shader_ctx->radeon_bld.soa.addr[ireg->Index][ireg->Swizzle];
	addr = LLVMBuildLoad(base->gallivm->builder, addr, "load addr reg");
	addr = lp_build_mul_imm(&bld_base->uint_bld, addr, 16);
	addr = lp_build_add(&bld_base->uint_bld, addr,
			    lp_build_const_int32(base->gallivm, idx * 4));

	result = buffer_load_const(base->gallivm->builder, si_shader_ctx->const_resource[buf],
			    addr, base->elem_type);

	return bitcast(bld_base, type, result);
}

/* Initialize arguments for the shader export intrinsic */
static void si_llvm_init_export_args(struct lp_build_tgsi_context *bld_base,
				     LLVMValueRef *values,
				     unsigned target,
				     LLVMValueRef *args)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct lp_build_context *uint =
				&si_shader_ctx->radeon_bld.soa.bld_base.uint_bld;
	struct lp_build_context *base = &bld_base->base;
	unsigned compressed = 0;
	unsigned chan;

	if (si_shader_ctx->type == TGSI_PROCESSOR_FRAGMENT) {
		int cbuf = target - V_008DFC_SQ_EXP_MRT;

		if (cbuf >= 0 && cbuf < 8) {
			compressed = (si_shader_ctx->shader->key.ps.export_16bpc >> cbuf) & 0x1;

			if (compressed)
				si_shader_ctx->shader->spi_shader_col_format |=
					V_028714_SPI_SHADER_FP16_ABGR << (4 * cbuf);
			else
				si_shader_ctx->shader->spi_shader_col_format |=
					V_028714_SPI_SHADER_32_ABGR << (4 * cbuf);

			si_shader_ctx->shader->cb_shader_mask |= 0xf << (4 * cbuf);
		}
	}

	if (compressed) {
		/* Pixel shader needs to pack output values before export */
		for (chan = 0; chan < 2; chan++ ) {
			args[0] = values[2 * chan];
			args[1] = values[2 * chan + 1];
			args[chan + 5] =
				build_intrinsic(base->gallivm->builder,
						"llvm.SI.packf16",
						LLVMInt32TypeInContext(base->gallivm->context),
						args, 2,
						LLVMReadNoneAttribute | LLVMNoUnwindAttribute);
			args[chan + 7] = args[chan + 5] =
				LLVMBuildBitCast(base->gallivm->builder,
						 args[chan + 5],
						 LLVMFloatTypeInContext(base->gallivm->context),
						 "");
		}

		/* Set COMPR flag */
		args[4] = uint->one;
	} else {
		for (chan = 0; chan < 4; chan++ )
			/* +5 because the first output value will be
			 * the 6th argument to the intrinsic. */
			args[chan + 5] = values[chan];

		/* Clear COMPR flag */
		args[4] = uint->zero;
	}

	/* XXX: This controls which components of the output
	 * registers actually get exported. (e.g bit 0 means export
	 * X component, bit 1 means export Y component, etc.)  I'm
	 * hard coding this to 0xf for now.  In the future, we might
	 * want to do something else. */
	args[0] = lp_build_const_int32(base->gallivm, 0xf);

	/* Specify whether the EXEC mask represents the valid mask */
	args[1] = uint->zero;

	/* Specify whether this is the last export */
	args[2] = uint->zero;

	/* Specify the target we are exporting */
	args[3] = lp_build_const_int32(base->gallivm, target);

	/* XXX: We probably need to keep track of the output
	 * values, so we know what we are passing to the next
	 * stage. */
}

/* Load from output pointers and initialize arguments for the shader export intrinsic */
static void si_llvm_init_export_args_load(struct lp_build_tgsi_context *bld_base,
					  LLVMValueRef *out_ptr,
					  unsigned target,
					  LLVMValueRef *args)
{
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	LLVMValueRef values[4];
	int i;

	for (i = 0; i < 4; i++)
		values[i] = LLVMBuildLoad(gallivm->builder, out_ptr[i], "");

	si_llvm_init_export_args(bld_base, values, target, args);
}

static void si_alpha_test(struct lp_build_tgsi_context *bld_base,
			  LLVMValueRef alpha_ptr)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct gallivm_state *gallivm = bld_base->base.gallivm;

	if (si_shader_ctx->shader->key.ps.alpha_func != PIPE_FUNC_NEVER) {
		LLVMValueRef alpha_ref = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
				SI_PARAM_ALPHA_REF);

		LLVMValueRef alpha_pass =
			lp_build_cmp(&bld_base->base,
				     si_shader_ctx->shader->key.ps.alpha_func,
				     LLVMBuildLoad(gallivm->builder, alpha_ptr, ""),
				     alpha_ref);
		LLVMValueRef arg =
			lp_build_select(&bld_base->base,
					alpha_pass,
					lp_build_const_float(gallivm, 1.0f),
					lp_build_const_float(gallivm, -1.0f));

		build_intrinsic(gallivm->builder,
				"llvm.AMDGPU.kill",
				LLVMVoidTypeInContext(gallivm->context),
				&arg, 1, 0);
	} else {
		build_intrinsic(gallivm->builder,
				"llvm.AMDGPU.kilp",
				LLVMVoidTypeInContext(gallivm->context),
				NULL, 0, 0);
	}

	si_shader_ctx->shader->db_shader_control |= S_02880C_KILL_ENABLE(1);
}

static void si_scale_alpha_by_sample_mask(struct lp_build_tgsi_context *bld_base,
					  LLVMValueRef alpha_ptr)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	LLVMValueRef coverage, alpha;

	/* alpha = alpha * popcount(coverage) / SI_NUM_SMOOTH_AA_SAMPLES */
	coverage = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
				SI_PARAM_SAMPLE_COVERAGE);
	coverage = bitcast(bld_base, TGSI_TYPE_SIGNED, coverage);

	coverage = build_intrinsic(gallivm->builder, "llvm.ctpop.i32",
				   bld_base->int_bld.elem_type,
				   &coverage, 1, LLVMReadNoneAttribute);

	coverage = LLVMBuildUIToFP(gallivm->builder, coverage,
				   bld_base->base.elem_type, "");

	coverage = LLVMBuildFMul(gallivm->builder, coverage,
				 lp_build_const_float(gallivm,
					1.0 / SI_NUM_SMOOTH_AA_SAMPLES), "");

	alpha = LLVMBuildLoad(gallivm->builder, alpha_ptr, "");
	alpha = LLVMBuildFMul(gallivm->builder, alpha, coverage, "");
	LLVMBuildStore(gallivm->builder, alpha, alpha_ptr);
}

static void si_llvm_emit_clipvertex(struct lp_build_tgsi_context * bld_base,
				    LLVMValueRef (*pos)[9], LLVMValueRef *out_elts)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct lp_build_context *base = &bld_base->base;
	struct lp_build_context *uint = &si_shader_ctx->radeon_bld.soa.bld_base.uint_bld;
	unsigned reg_index;
	unsigned chan;
	unsigned const_chan;
	LLVMValueRef base_elt;
	LLVMValueRef ptr = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn, SI_PARAM_CONST);
	LLVMValueRef constbuf_index = lp_build_const_int32(base->gallivm, SI_DRIVER_STATE_CONST_BUF);
	LLVMValueRef const_resource = build_indexed_load_const(si_shader_ctx, ptr, constbuf_index);

	for (reg_index = 0; reg_index < 2; reg_index ++) {
		LLVMValueRef *args = pos[2 + reg_index];

		args[5] =
		args[6] =
		args[7] =
		args[8] = lp_build_const_float(base->gallivm, 0.0f);

		/* Compute dot products of position and user clip plane vectors */
		for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
			for (const_chan = 0; const_chan < TGSI_NUM_CHANNELS; const_chan++) {
				args[1] = lp_build_const_int32(base->gallivm,
							       ((reg_index * 4 + chan) * 4 +
								const_chan) * 4);
				base_elt = buffer_load_const(base->gallivm->builder, const_resource,
						      args[1], base->elem_type);
				args[5 + chan] =
					lp_build_add(base, args[5 + chan],
						     lp_build_mul(base, base_elt,
								  out_elts[const_chan]));
			}
		}

		args[0] = lp_build_const_int32(base->gallivm, 0xf);
		args[1] = uint->zero;
		args[2] = uint->zero;
		args[3] = lp_build_const_int32(base->gallivm,
					       V_008DFC_SQ_EXP_POS + 2 + reg_index);
		args[4] = uint->zero;
	}
}

static void si_dump_streamout(struct pipe_stream_output_info *so)
{
	unsigned i;

	if (so->num_outputs)
		fprintf(stderr, "STREAMOUT\n");

	for (i = 0; i < so->num_outputs; i++) {
		unsigned mask = ((1 << so->output[i].num_components) - 1) <<
				so->output[i].start_component;
		fprintf(stderr, "  %i: BUF%i[%i..%i] <- OUT[%i].%s%s%s%s\n",
			i, so->output[i].output_buffer,
			so->output[i].dst_offset, so->output[i].dst_offset + so->output[i].num_components - 1,
			so->output[i].register_index,
			mask & 1 ? "x" : "",
		        mask & 2 ? "y" : "",
		        mask & 4 ? "z" : "",
		        mask & 8 ? "w" : "");
	}
}

/* TBUFFER_STORE_FORMAT_{X,XY,XYZ,XYZW} <- the suffix is selected by num_channels=1..4.
 * The type of vdata must be one of i32 (num_channels=1), v2i32 (num_channels=2),
 * or v4i32 (num_channels=3,4). */
static void build_tbuffer_store(struct si_shader_context *shader,
				LLVMValueRef rsrc,
				LLVMValueRef vdata,
				unsigned num_channels,
				LLVMValueRef vaddr,
				LLVMValueRef soffset,
				unsigned inst_offset,
				unsigned dfmt,
				unsigned nfmt,
				unsigned offen,
				unsigned idxen,
				unsigned glc,
				unsigned slc,
				unsigned tfe)
{
	struct gallivm_state *gallivm = &shader->radeon_bld.gallivm;
	LLVMTypeRef i32 = LLVMInt32TypeInContext(gallivm->context);
	LLVMValueRef args[] = {
		rsrc,
		vdata,
		LLVMConstInt(i32, num_channels, 0),
		vaddr,
		soffset,
		LLVMConstInt(i32, inst_offset, 0),
		LLVMConstInt(i32, dfmt, 0),
		LLVMConstInt(i32, nfmt, 0),
		LLVMConstInt(i32, offen, 0),
		LLVMConstInt(i32, idxen, 0),
		LLVMConstInt(i32, glc, 0),
		LLVMConstInt(i32, slc, 0),
		LLVMConstInt(i32, tfe, 0)
	};

	/* The instruction offset field has 12 bits */
	assert(offen || inst_offset < (1 << 12));

	/* The intrinsic is overloaded, we need to add a type suffix for overloading to work. */
	unsigned func = CLAMP(num_channels, 1, 3) - 1;
	const char *types[] = {"i32", "v2i32", "v4i32"};
	char name[256];
	snprintf(name, sizeof(name), "llvm.SI.tbuffer.store.%s", types[func]);

	lp_build_intrinsic(gallivm->builder, name,
			   LLVMVoidTypeInContext(gallivm->context),
			   args, Elements(args));
}

static void build_streamout_store(struct si_shader_context *shader,
				  LLVMValueRef rsrc,
				  LLVMValueRef vdata,
				  unsigned num_channels,
				  LLVMValueRef vaddr,
				  LLVMValueRef soffset,
				  unsigned inst_offset)
{
	static unsigned dfmt[] = {
		V_008F0C_BUF_DATA_FORMAT_32,
		V_008F0C_BUF_DATA_FORMAT_32_32,
		V_008F0C_BUF_DATA_FORMAT_32_32_32,
		V_008F0C_BUF_DATA_FORMAT_32_32_32_32
	};
	assert(num_channels >= 1 && num_channels <= 4);

	build_tbuffer_store(shader, rsrc, vdata, num_channels, vaddr, soffset,
			    inst_offset, dfmt[num_channels-1],
			    V_008F0C_BUF_NUM_FORMAT_UINT, 1, 0, 1, 1, 0);
}

/* On SI, the vertex shader is responsible for writing streamout data
 * to buffers. */
static void si_llvm_emit_streamout(struct si_shader_context *shader,
				   struct si_shader_output_values *outputs,
				   unsigned noutput)
{
	struct pipe_stream_output_info *so = &shader->shader->selector->so;
	struct gallivm_state *gallivm = &shader->radeon_bld.gallivm;
	LLVMBuilderRef builder = gallivm->builder;
	int i, j;
	struct lp_build_if_state if_ctx;

	LLVMTypeRef i32 = LLVMInt32TypeInContext(gallivm->context);

	/* Get bits [22:16], i.e. (so_param >> 16) & 127; */
	LLVMValueRef so_vtx_count =
		unpack_param(shader, shader->param_streamout_config, 16, 7);

	LLVMValueRef tid = build_intrinsic(builder, "llvm.SI.tid", i32,
					   NULL, 0, LLVMReadNoneAttribute);

	/* can_emit = tid < so_vtx_count; */
	LLVMValueRef can_emit =
		LLVMBuildICmp(builder, LLVMIntULT, tid, so_vtx_count, "");

	/* Emit the streamout code conditionally. This actually avoids
	 * out-of-bounds buffer access. The hw tells us via the SGPR
	 * (so_vtx_count) which threads are allowed to emit streamout data. */
	lp_build_if(&if_ctx, gallivm, can_emit);
	{
		/* The buffer offset is computed as follows:
		 *   ByteOffset = streamout_offset[buffer_id]*4 +
		 *                (streamout_write_index + thread_id)*stride[buffer_id] +
		 *                attrib_offset
                 */

		LLVMValueRef so_write_index =
			LLVMGetParam(shader->radeon_bld.main_fn,
				     shader->param_streamout_write_index);

		/* Compute (streamout_write_index + thread_id). */
		so_write_index = LLVMBuildAdd(builder, so_write_index, tid, "");

		/* Compute the write offset for each enabled buffer. */
		LLVMValueRef so_write_offset[4] = {};
		for (i = 0; i < 4; i++) {
			if (!so->stride[i])
				continue;

			LLVMValueRef so_offset = LLVMGetParam(shader->radeon_bld.main_fn,
							      shader->param_streamout_offset[i]);
			so_offset = LLVMBuildMul(builder, so_offset, LLVMConstInt(i32, 4, 0), "");

			so_write_offset[i] = LLVMBuildMul(builder, so_write_index,
							  LLVMConstInt(i32, so->stride[i]*4, 0), "");
			so_write_offset[i] = LLVMBuildAdd(builder, so_write_offset[i], so_offset, "");
		}

		/* Write streamout data. */
		for (i = 0; i < so->num_outputs; i++) {
			unsigned buf_idx = so->output[i].output_buffer;
			unsigned reg = so->output[i].register_index;
			unsigned start = so->output[i].start_component;
			unsigned num_comps = so->output[i].num_components;
			LLVMValueRef out[4];

			assert(num_comps && num_comps <= 4);
			if (!num_comps || num_comps > 4)
				continue;

			if (reg >= noutput)
				continue;

			/* Load the output as int. */
			for (j = 0; j < num_comps; j++) {
				out[j] = LLVMBuildBitCast(builder,
							  outputs[reg].values[start+j],
						i32, "");
			}

			/* Pack the output. */
			LLVMValueRef vdata = NULL;

			switch (num_comps) {
			case 1: /* as i32 */
				vdata = out[0];
				break;
			case 2: /* as v2i32 */
			case 3: /* as v4i32 (aligned to 4) */
			case 4: /* as v4i32 */
				vdata = LLVMGetUndef(LLVMVectorType(i32, util_next_power_of_two(num_comps)));
				for (j = 0; j < num_comps; j++) {
					vdata = LLVMBuildInsertElement(builder, vdata, out[j],
								       LLVMConstInt(i32, j, 0), "");
				}
				break;
			}

			build_streamout_store(shader, shader->so_buffers[buf_idx],
					      vdata, num_comps,
					      so_write_offset[buf_idx],
					      LLVMConstInt(i32, 0, 0),
					      so->output[i].dst_offset*4);
		}
	}
	lp_build_endif(&if_ctx);
}


/* Generate export instructions for hardware VS shader stage */
static void si_llvm_export_vs(struct lp_build_tgsi_context *bld_base,
			      struct si_shader_output_values *outputs,
			      unsigned noutput)
{
	struct si_shader_context * si_shader_ctx = si_shader_context(bld_base);
	struct si_shader * shader = si_shader_ctx->shader;
	struct lp_build_context * base = &bld_base->base;
	struct lp_build_context * uint =
				&si_shader_ctx->radeon_bld.soa.bld_base.uint_bld;
	LLVMValueRef args[9];
	LLVMValueRef pos_args[4][9] = { { 0 } };
	LLVMValueRef psize_value = NULL, edgeflag_value = NULL, layer_value = NULL;
	unsigned semantic_name, semantic_index;
	unsigned target;
	unsigned param_count = 0;
	unsigned pos_idx;
	int i;

	if (outputs && si_shader_ctx->shader->selector->so.num_outputs) {
		si_llvm_emit_streamout(si_shader_ctx, outputs, noutput);
	}

	for (i = 0; i < noutput; i++) {
		semantic_name = outputs[i].name;
		semantic_index = outputs[i].sid;

handle_semantic:
		/* Select the correct target */
		switch(semantic_name) {
		case TGSI_SEMANTIC_PSIZE:
			psize_value = outputs[i].values[0];
			continue;
		case TGSI_SEMANTIC_EDGEFLAG:
			edgeflag_value = outputs[i].values[0];
			continue;
		case TGSI_SEMANTIC_LAYER:
			layer_value = outputs[i].values[0];
			continue;
		case TGSI_SEMANTIC_POSITION:
			target = V_008DFC_SQ_EXP_POS;
			break;
		case TGSI_SEMANTIC_COLOR:
		case TGSI_SEMANTIC_BCOLOR:
			target = V_008DFC_SQ_EXP_PARAM + param_count;
			shader->vs_output_param_offset[i] = param_count;
			param_count++;
			break;
		case TGSI_SEMANTIC_CLIPDIST:
			target = V_008DFC_SQ_EXP_POS + 2 + semantic_index;
			break;
		case TGSI_SEMANTIC_CLIPVERTEX:
			si_llvm_emit_clipvertex(bld_base, pos_args, outputs[i].values);
			continue;
		case TGSI_SEMANTIC_PRIMID:
		case TGSI_SEMANTIC_FOG:
		case TGSI_SEMANTIC_GENERIC:
			target = V_008DFC_SQ_EXP_PARAM + param_count;
			shader->vs_output_param_offset[i] = param_count;
			param_count++;
			break;
		default:
			target = 0;
			fprintf(stderr,
				"Warning: SI unhandled vs output type:%d\n",
				semantic_name);
		}

		si_llvm_init_export_args(bld_base, outputs[i].values, target, args);

		if (target >= V_008DFC_SQ_EXP_POS &&
		    target <= (V_008DFC_SQ_EXP_POS + 3)) {
			memcpy(pos_args[target - V_008DFC_SQ_EXP_POS],
			       args, sizeof(args));
		} else {
			lp_build_intrinsic(base->gallivm->builder,
					   "llvm.SI.export",
					   LLVMVoidTypeInContext(base->gallivm->context),
					   args, 9);
		}

		if (semantic_name == TGSI_SEMANTIC_CLIPDIST) {
			semantic_name = TGSI_SEMANTIC_GENERIC;
			goto handle_semantic;
		}
	}

	/* We need to add the position output manually if it's missing. */
	if (!pos_args[0][0]) {
		pos_args[0][0] = lp_build_const_int32(base->gallivm, 0xf); /* writemask */
		pos_args[0][1] = uint->zero; /* EXEC mask */
		pos_args[0][2] = uint->zero; /* last export? */
		pos_args[0][3] = lp_build_const_int32(base->gallivm, V_008DFC_SQ_EXP_POS);
		pos_args[0][4] = uint->zero; /* COMPR flag */
		pos_args[0][5] = base->zero; /* X */
		pos_args[0][6] = base->zero; /* Y */
		pos_args[0][7] = base->zero; /* Z */
		pos_args[0][8] = base->one;  /* W */
	}

	/* Write the misc vector (point size, edgeflag, layer, viewport). */
	if (shader->selector->info.writes_psize ||
	    shader->selector->info.writes_edgeflag ||
	    shader->selector->info.writes_layer) {
		pos_args[1][0] = lp_build_const_int32(base->gallivm, /* writemask */
						      shader->selector->info.writes_psize |
						      (shader->selector->info.writes_edgeflag << 1) |
						      (shader->selector->info.writes_layer << 2));
		pos_args[1][1] = uint->zero; /* EXEC mask */
		pos_args[1][2] = uint->zero; /* last export? */
		pos_args[1][3] = lp_build_const_int32(base->gallivm, V_008DFC_SQ_EXP_POS + 1);
		pos_args[1][4] = uint->zero; /* COMPR flag */
		pos_args[1][5] = base->zero; /* X */
		pos_args[1][6] = base->zero; /* Y */
		pos_args[1][7] = base->zero; /* Z */
		pos_args[1][8] = base->zero; /* W */

		if (shader->selector->info.writes_psize)
			pos_args[1][5] = psize_value;

		if (shader->selector->info.writes_edgeflag) {
			/* The output is a float, but the hw expects an integer
			 * with the first bit containing the edge flag. */
			edgeflag_value = LLVMBuildFPToUI(base->gallivm->builder,
							 edgeflag_value,
							 bld_base->uint_bld.elem_type, "");
			edgeflag_value = lp_build_min(&bld_base->int_bld,
						      edgeflag_value,
						      bld_base->int_bld.one);

			/* The LLVM intrinsic expects a float. */
			pos_args[1][6] = LLVMBuildBitCast(base->gallivm->builder,
							  edgeflag_value,
							  base->elem_type, "");
		}

		if (shader->selector->info.writes_layer)
			pos_args[1][7] = layer_value;
	}

	for (i = 0; i < 4; i++)
		if (pos_args[i][0])
			shader->nr_pos_exports++;

	pos_idx = 0;
	for (i = 0; i < 4; i++) {
		if (!pos_args[i][0])
			continue;

		/* Specify the target we are exporting */
		pos_args[i][3] = lp_build_const_int32(base->gallivm, V_008DFC_SQ_EXP_POS + pos_idx++);

		if (pos_idx == shader->nr_pos_exports)
			/* Specify that this is the last export */
			pos_args[i][2] = uint->one;

		lp_build_intrinsic(base->gallivm->builder,
				   "llvm.SI.export",
				   LLVMVoidTypeInContext(base->gallivm->context),
				   pos_args[i], 9);
	}
}

static void si_llvm_emit_es_epilogue(struct lp_build_tgsi_context * bld_base)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	struct si_shader *es = si_shader_ctx->shader;
	struct tgsi_shader_info *info = &es->selector->info;
	LLVMTypeRef i32 = LLVMInt32TypeInContext(gallivm->context);
	LLVMValueRef soffset = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
					    SI_PARAM_ES2GS_OFFSET);
	unsigned chan;
	int i;

	for (i = 0; i < info->num_outputs; i++) {
		LLVMValueRef *out_ptr =
			si_shader_ctx->radeon_bld.soa.outputs[i];
		int param_index = get_param_index(info->output_semantic_name[i],
						  info->output_semantic_index[i],
						  es->key.vs.gs_used_inputs);

		if (param_index < 0)
			continue;

		for (chan = 0; chan < 4; chan++) {
			LLVMValueRef out_val = LLVMBuildLoad(gallivm->builder, out_ptr[chan], "");
			out_val = LLVMBuildBitCast(gallivm->builder, out_val, i32, "");

			build_tbuffer_store(si_shader_ctx,
					    si_shader_ctx->esgs_ring,
					    out_val, 1,
					    LLVMGetUndef(i32), soffset,
					    (4 * param_index + chan) * 4,
					    V_008F0C_BUF_DATA_FORMAT_32,
					    V_008F0C_BUF_NUM_FORMAT_UINT,
					    0, 0, 1, 1, 0);
		}
	}
}

static void si_llvm_emit_gs_epilogue(struct lp_build_tgsi_context *bld_base)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	LLVMValueRef args[2];

	args[0] = lp_build_const_int32(gallivm,	SENDMSG_GS_OP_NOP | SENDMSG_GS_DONE);
	args[1] = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn, SI_PARAM_GS_WAVE_ID);
	build_intrinsic(gallivm->builder, "llvm.SI.sendmsg",
			LLVMVoidTypeInContext(gallivm->context), args, 2,
			LLVMNoUnwindAttribute);
}

static void si_llvm_emit_vs_epilogue(struct lp_build_tgsi_context * bld_base)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	struct tgsi_shader_info *info = &si_shader_ctx->shader->selector->info;
	struct si_shader_output_values *outputs = NULL;
	int i,j;

	outputs = MALLOC(info->num_outputs * sizeof(outputs[0]));

	for (i = 0; i < info->num_outputs; i++) {
		outputs[i].name = info->output_semantic_name[i];
		outputs[i].sid = info->output_semantic_index[i];

		for (j = 0; j < 4; j++)
			outputs[i].values[j] =
				LLVMBuildLoad(gallivm->builder,
					      si_shader_ctx->radeon_bld.soa.outputs[i][j],
					      "");
	}

	si_llvm_export_vs(bld_base, outputs, info->num_outputs);
	FREE(outputs);
}

static void si_llvm_emit_fs_epilogue(struct lp_build_tgsi_context * bld_base)
{
	struct si_shader_context * si_shader_ctx = si_shader_context(bld_base);
	struct si_shader * shader = si_shader_ctx->shader;
	struct lp_build_context * base = &bld_base->base;
	struct lp_build_context * uint = &bld_base->uint_bld;
	struct tgsi_shader_info *info = &shader->selector->info;
	LLVMValueRef args[9];
	LLVMValueRef last_args[9] = { 0 };
	int depth_index = -1, stencil_index = -1, samplemask_index = -1;
	int i;

	for (i = 0; i < info->num_outputs; i++) {
		unsigned semantic_name = info->output_semantic_name[i];
		unsigned semantic_index = info->output_semantic_index[i];
		unsigned target;
		LLVMValueRef alpha_ptr;

		/* Select the correct target */
		switch (semantic_name) {
		case TGSI_SEMANTIC_POSITION:
			depth_index = i;
			continue;
		case TGSI_SEMANTIC_STENCIL:
			stencil_index = i;
			continue;
		case TGSI_SEMANTIC_SAMPLEMASK:
			samplemask_index = i;
			continue;
		case TGSI_SEMANTIC_COLOR:
			target = V_008DFC_SQ_EXP_MRT + semantic_index;
			alpha_ptr = si_shader_ctx->radeon_bld.soa.outputs[i][3];

			if (si_shader_ctx->shader->key.ps.alpha_to_one)
				LLVMBuildStore(base->gallivm->builder,
					       base->one, alpha_ptr);

			if (semantic_index == 0 &&
			    si_shader_ctx->shader->key.ps.alpha_func != PIPE_FUNC_ALWAYS)
				si_alpha_test(bld_base, alpha_ptr);

			if (si_shader_ctx->shader->key.ps.poly_line_smoothing)
				si_scale_alpha_by_sample_mask(bld_base, alpha_ptr);
			break;
		default:
			target = 0;
			fprintf(stderr,
				"Warning: SI unhandled fs output type:%d\n",
				semantic_name);
		}

		si_llvm_init_export_args_load(bld_base,
					      si_shader_ctx->radeon_bld.soa.outputs[i],
					      target, args);

		if (semantic_name == TGSI_SEMANTIC_COLOR) {
			/* If there is an export instruction waiting to be emitted, do so now. */
			if (last_args[0]) {
				lp_build_intrinsic(base->gallivm->builder,
						   "llvm.SI.export",
						   LLVMVoidTypeInContext(base->gallivm->context),
						   last_args, 9);
			}

			/* This instruction will be emitted at the end of the shader. */
			memcpy(last_args, args, sizeof(args));

			/* Handle FS_COLOR0_WRITES_ALL_CBUFS. */
			if (shader->selector->info.properties[TGSI_PROPERTY_FS_COLOR0_WRITES_ALL_CBUFS] &&
			    semantic_index == 0 &&
			    si_shader_ctx->shader->key.ps.last_cbuf > 0) {
				for (int c = 1; c <= si_shader_ctx->shader->key.ps.last_cbuf; c++) {
					si_llvm_init_export_args_load(bld_base,
								      si_shader_ctx->radeon_bld.soa.outputs[i],
								      V_008DFC_SQ_EXP_MRT + c, args);
					lp_build_intrinsic(base->gallivm->builder,
							   "llvm.SI.export",
							   LLVMVoidTypeInContext(base->gallivm->context),
							   args, 9);
				}
			}
		} else {
			lp_build_intrinsic(base->gallivm->builder,
					   "llvm.SI.export",
					   LLVMVoidTypeInContext(base->gallivm->context),
					   args, 9);
		}
	}

	if (depth_index >= 0 || stencil_index >= 0 || samplemask_index >= 0) {
		LLVMValueRef out_ptr;
		unsigned mask = 0;

		/* Specify the target we are exporting */
		args[3] = lp_build_const_int32(base->gallivm, V_008DFC_SQ_EXP_MRTZ);

		args[5] = base->zero; /* R, depth */
		args[6] = base->zero; /* G, stencil test value[0:7], stencil op value[8:15] */
		args[7] = base->zero; /* B, sample mask */
		args[8] = base->zero; /* A, alpha to mask */

		if (depth_index >= 0) {
			out_ptr = si_shader_ctx->radeon_bld.soa.outputs[depth_index][2];
			args[5] = LLVMBuildLoad(base->gallivm->builder, out_ptr, "");
			mask |= 0x1;
			si_shader_ctx->shader->db_shader_control |= S_02880C_Z_EXPORT_ENABLE(1);
		}

		if (stencil_index >= 0) {
			out_ptr = si_shader_ctx->radeon_bld.soa.outputs[stencil_index][1];
			args[6] = LLVMBuildLoad(base->gallivm->builder, out_ptr, "");
			mask |= 0x2;
			si_shader_ctx->shader->db_shader_control |=
				S_02880C_STENCIL_TEST_VAL_EXPORT_ENABLE(1);
		}

		if (samplemask_index >= 0) {
			out_ptr = si_shader_ctx->radeon_bld.soa.outputs[samplemask_index][0];
			args[7] = LLVMBuildLoad(base->gallivm->builder, out_ptr, "");
			mask |= 0x4;
			si_shader_ctx->shader->db_shader_control |= S_02880C_MASK_EXPORT_ENABLE(1);
		}

		/* SI (except OLAND) has a bug that it only looks
		 * at the X writemask component. */
		if (si_shader_ctx->screen->b.chip_class == SI &&
		    si_shader_ctx->screen->b.family != CHIP_OLAND)
			mask |= 0x1;

		if (samplemask_index >= 0)
			si_shader_ctx->shader->spi_shader_z_format = V_028710_SPI_SHADER_32_ABGR;
		else if (stencil_index >= 0)
			si_shader_ctx->shader->spi_shader_z_format = V_028710_SPI_SHADER_32_GR;
		else
			si_shader_ctx->shader->spi_shader_z_format = V_028710_SPI_SHADER_32_R;

		/* Specify which components to enable */
		args[0] = lp_build_const_int32(base->gallivm, mask);

		args[1] =
		args[2] =
		args[4] = uint->zero;

		if (last_args[0])
			lp_build_intrinsic(base->gallivm->builder,
					   "llvm.SI.export",
					   LLVMVoidTypeInContext(base->gallivm->context),
					   args, 9);
		else
			memcpy(last_args, args, sizeof(args));
	}

	if (!last_args[0]) {
		/* Specify which components to enable */
		last_args[0] = lp_build_const_int32(base->gallivm, 0x0);

		/* Specify the target we are exporting */
		last_args[3] = lp_build_const_int32(base->gallivm, V_008DFC_SQ_EXP_MRT);

		/* Set COMPR flag to zero to export data as 32-bit */
		last_args[4] = uint->zero;

		/* dummy bits */
		last_args[5]= uint->zero;
		last_args[6]= uint->zero;
		last_args[7]= uint->zero;
		last_args[8]= uint->zero;
	}

	/* Specify whether the EXEC mask represents the valid mask */
	last_args[1] = uint->one;

	/* Specify that this is the last export */
	last_args[2] = lp_build_const_int32(base->gallivm, 1);

	lp_build_intrinsic(base->gallivm->builder,
			   "llvm.SI.export",
			   LLVMVoidTypeInContext(base->gallivm->context),
			   last_args, 9);
}

static void build_tex_intrinsic(const struct lp_build_tgsi_action * action,
				struct lp_build_tgsi_context * bld_base,
				struct lp_build_emit_data * emit_data);

static bool tgsi_is_shadow_sampler(unsigned target)
{
	return target == TGSI_TEXTURE_SHADOW1D ||
	       target == TGSI_TEXTURE_SHADOW1D_ARRAY ||
	       target == TGSI_TEXTURE_SHADOW2D ||
	       target == TGSI_TEXTURE_SHADOW2D_ARRAY ||
	       target == TGSI_TEXTURE_SHADOWCUBE ||
	       target == TGSI_TEXTURE_SHADOWCUBE_ARRAY ||
	       target == TGSI_TEXTURE_SHADOWRECT;
}

static const struct lp_build_tgsi_action tex_action;

static void tex_fetch_args(
	struct lp_build_tgsi_context * bld_base,
	struct lp_build_emit_data * emit_data)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	const struct tgsi_full_instruction * inst = emit_data->inst;
	unsigned opcode = inst->Instruction.Opcode;
	unsigned target = inst->Texture.Texture;
	LLVMValueRef coords[5];
	LLVMValueRef address[16];
	int ref_pos;
	unsigned num_coords = tgsi_util_get_texture_coord_dim(target, &ref_pos);
	unsigned count = 0;
	unsigned chan;
	unsigned sampler_src = emit_data->inst->Instruction.NumSrcRegs - 1;
	unsigned sampler_index = emit_data->inst->Src[sampler_src].Register.Index;
	bool has_offset = HAVE_LLVM >= 0x0305 ? inst->Texture.NumOffsets > 0 : false;

	if (target == TGSI_TEXTURE_BUFFER) {
		LLVMTypeRef i128 = LLVMIntTypeInContext(gallivm->context, 128);
		LLVMTypeRef v2i128 = LLVMVectorType(i128, 2);
		LLVMTypeRef i8 = LLVMInt8TypeInContext(gallivm->context);
		LLVMTypeRef v16i8 = LLVMVectorType(i8, 16);

		/* Bitcast and truncate v8i32 to v16i8. */
		LLVMValueRef res = si_shader_ctx->resources[sampler_index];
		res = LLVMBuildBitCast(gallivm->builder, res, v2i128, "");
		res = LLVMBuildExtractElement(gallivm->builder, res, bld_base->uint_bld.one, "");
		res = LLVMBuildBitCast(gallivm->builder, res, v16i8, "");

		emit_data->dst_type = LLVMVectorType(bld_base->base.elem_type, 4);
		emit_data->args[0] = res;
		emit_data->args[1] = bld_base->uint_bld.zero;
		emit_data->args[2] = lp_build_emit_fetch(bld_base, emit_data->inst, 0, 0);
		emit_data->arg_count = 3;
		return;
	}

	/* Fetch and project texture coordinates */
	coords[3] = lp_build_emit_fetch(bld_base, emit_data->inst, 0, TGSI_CHAN_W);
	for (chan = 0; chan < 3; chan++ ) {
		coords[chan] = lp_build_emit_fetch(bld_base,
						   emit_data->inst, 0,
						   chan);
		if (opcode == TGSI_OPCODE_TXP)
			coords[chan] = lp_build_emit_llvm_binary(bld_base,
								 TGSI_OPCODE_DIV,
								 coords[chan],
								 coords[3]);
	}

	if (opcode == TGSI_OPCODE_TXP)
		coords[3] = bld_base->base.one;

	/* Pack offsets. */
	if (has_offset && opcode != TGSI_OPCODE_TXF) {
		/* The offsets are six-bit signed integers packed like this:
		 *   X=[5:0], Y=[13:8], and Z=[21:16].
		 */
		LLVMValueRef offset[3], pack;

		assert(inst->Texture.NumOffsets == 1);

		for (chan = 0; chan < 3; chan++) {
			offset[chan] = lp_build_emit_fetch_texoffset(bld_base,
								     emit_data->inst, 0, chan);
			offset[chan] = LLVMBuildAnd(gallivm->builder, offset[chan],
						    lp_build_const_int32(gallivm, 0x3f), "");
			if (chan)
				offset[chan] = LLVMBuildShl(gallivm->builder, offset[chan],
							    lp_build_const_int32(gallivm, chan*8), "");
		}

		pack = LLVMBuildOr(gallivm->builder, offset[0], offset[1], "");
		pack = LLVMBuildOr(gallivm->builder, pack, offset[2], "");
		address[count++] = pack;
	}

	/* Pack LOD bias value */
	if (opcode == TGSI_OPCODE_TXB)
		address[count++] = coords[3];
	if (opcode == TGSI_OPCODE_TXB2)
		address[count++] = lp_build_emit_fetch(bld_base, inst, 1, 0);

	/* Pack depth comparison value */
	if (tgsi_is_shadow_sampler(target) && opcode != TGSI_OPCODE_LODQ) {
		if (target == TGSI_TEXTURE_SHADOWCUBE_ARRAY) {
			address[count++] = lp_build_emit_fetch(bld_base, inst, 1, 0);
		} else {
			assert(ref_pos >= 0);
			address[count++] = coords[ref_pos];
		}
	}

	if (target == TGSI_TEXTURE_CUBE ||
	    target == TGSI_TEXTURE_CUBE_ARRAY ||
	    target == TGSI_TEXTURE_SHADOWCUBE ||
	    target == TGSI_TEXTURE_SHADOWCUBE_ARRAY)
		radeon_llvm_emit_prepare_cube_coords(bld_base, emit_data, coords);

	/* Pack user derivatives */
	if (opcode == TGSI_OPCODE_TXD) {
		int num_deriv_channels, param;

		switch (target) {
		case TGSI_TEXTURE_3D:
			num_deriv_channels = 3;
			break;
		case TGSI_TEXTURE_2D:
		case TGSI_TEXTURE_SHADOW2D:
		case TGSI_TEXTURE_RECT:
		case TGSI_TEXTURE_SHADOWRECT:
		case TGSI_TEXTURE_2D_ARRAY:
		case TGSI_TEXTURE_SHADOW2D_ARRAY:
		case TGSI_TEXTURE_CUBE:
		case TGSI_TEXTURE_SHADOWCUBE:
		case TGSI_TEXTURE_CUBE_ARRAY:
		case TGSI_TEXTURE_SHADOWCUBE_ARRAY:
			num_deriv_channels = 2;
			break;
		case TGSI_TEXTURE_1D:
		case TGSI_TEXTURE_SHADOW1D:
		case TGSI_TEXTURE_1D_ARRAY:
		case TGSI_TEXTURE_SHADOW1D_ARRAY:
			num_deriv_channels = 1;
			break;
		default:
			assert(0); /* no other targets are valid here */
		}

		for (param = 1; param <= 2; param++)
			for (chan = 0; chan < num_deriv_channels; chan++)
				address[count++] = lp_build_emit_fetch(bld_base, inst, param, chan);
	}

	/* Pack texture coordinates */
	address[count++] = coords[0];
	if (num_coords > 1)
		address[count++] = coords[1];
	if (num_coords > 2)
		address[count++] = coords[2];

	/* Pack LOD or sample index */
	if (opcode == TGSI_OPCODE_TXL || opcode == TGSI_OPCODE_TXF)
		address[count++] = coords[3];
	else if (opcode == TGSI_OPCODE_TXL2)
		address[count++] = lp_build_emit_fetch(bld_base, inst, 1, 0);

	if (count > 16) {
		assert(!"Cannot handle more than 16 texture address parameters");
		count = 16;
	}

	for (chan = 0; chan < count; chan++ ) {
		address[chan] = LLVMBuildBitCast(gallivm->builder,
						 address[chan],
						 LLVMInt32TypeInContext(gallivm->context),
						 "");
	}

	/* Adjust the sample index according to FMASK.
	 *
	 * For uncompressed MSAA surfaces, FMASK should return 0x76543210,
	 * which is the identity mapping. Each nibble says which physical sample
	 * should be fetched to get that sample.
	 *
	 * For example, 0x11111100 means there are only 2 samples stored and
	 * the second sample covers 3/4 of the pixel. When reading samples 0
	 * and 1, return physical sample 0 (determined by the first two 0s
	 * in FMASK), otherwise return physical sample 1.
	 *
	 * The sample index should be adjusted as follows:
	 *   sample_index = (fmask >> (sample_index * 4)) & 0xF;
	 */
	if (target == TGSI_TEXTURE_2D_MSAA ||
	    target == TGSI_TEXTURE_2D_ARRAY_MSAA) {
		struct lp_build_context *uint_bld = &bld_base->uint_bld;
		struct lp_build_emit_data txf_emit_data = *emit_data;
		LLVMValueRef txf_address[4];
		unsigned txf_count = count;
		struct tgsi_full_instruction inst = {};

		memcpy(txf_address, address, sizeof(txf_address));

		if (target == TGSI_TEXTURE_2D_MSAA) {
			txf_address[2] = bld_base->uint_bld.zero;
		}
		txf_address[3] = bld_base->uint_bld.zero;

		/* Pad to a power-of-two size. */
		while (txf_count < util_next_power_of_two(txf_count))
			txf_address[txf_count++] = LLVMGetUndef(LLVMInt32TypeInContext(gallivm->context));

		/* Read FMASK using TXF. */
		inst.Instruction.Opcode = TGSI_OPCODE_TXF;
		inst.Texture.Texture = target == TGSI_TEXTURE_2D_MSAA ? TGSI_TEXTURE_2D : TGSI_TEXTURE_2D_ARRAY;
		txf_emit_data.inst = &inst;
		txf_emit_data.chan = 0;
		txf_emit_data.dst_type = LLVMVectorType(
			LLVMInt32TypeInContext(gallivm->context), 4);
		txf_emit_data.args[0] = lp_build_gather_values(gallivm, txf_address, txf_count);
		txf_emit_data.args[1] = si_shader_ctx->resources[SI_FMASK_TEX_OFFSET + sampler_index];
		txf_emit_data.args[2] = lp_build_const_int32(gallivm, inst.Texture.Texture);
		txf_emit_data.arg_count = 3;

		build_tex_intrinsic(&tex_action, bld_base, &txf_emit_data);

		/* Initialize some constants. */
		LLVMValueRef four = LLVMConstInt(uint_bld->elem_type, 4, 0);
		LLVMValueRef F = LLVMConstInt(uint_bld->elem_type, 0xF, 0);

		/* Apply the formula. */
		LLVMValueRef fmask =
			LLVMBuildExtractElement(gallivm->builder,
						txf_emit_data.output[0],
						uint_bld->zero, "");

		unsigned sample_chan = target == TGSI_TEXTURE_2D_MSAA ? 2 : 3;

		LLVMValueRef sample_index4 =
			LLVMBuildMul(gallivm->builder, address[sample_chan], four, "");

		LLVMValueRef shifted_fmask =
			LLVMBuildLShr(gallivm->builder, fmask, sample_index4, "");

		LLVMValueRef final_sample =
			LLVMBuildAnd(gallivm->builder, shifted_fmask, F, "");

		/* Don't rewrite the sample index if WORD1.DATA_FORMAT of the FMASK
		 * resource descriptor is 0 (invalid),
		 */
		LLVMValueRef fmask_desc =
			LLVMBuildBitCast(gallivm->builder,
					 si_shader_ctx->resources[SI_FMASK_TEX_OFFSET + sampler_index],
					 LLVMVectorType(uint_bld->elem_type, 8), "");

		LLVMValueRef fmask_word1 =
			LLVMBuildExtractElement(gallivm->builder, fmask_desc,
						uint_bld->one, "");

		LLVMValueRef word1_is_nonzero =
			LLVMBuildICmp(gallivm->builder, LLVMIntNE,
				      fmask_word1, uint_bld->zero, "");

		/* Replace the MSAA sample index. */
		address[sample_chan] =
			LLVMBuildSelect(gallivm->builder, word1_is_nonzero,
					final_sample, address[sample_chan], "");
	}

	/* Resource */
	emit_data->args[1] = si_shader_ctx->resources[sampler_index];

	if (opcode == TGSI_OPCODE_TXF) {
		/* add tex offsets */
		if (inst->Texture.NumOffsets) {
			struct lp_build_context *uint_bld = &bld_base->uint_bld;
			struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
			const struct tgsi_texture_offset * off = inst->TexOffsets;

			assert(inst->Texture.NumOffsets == 1);

			switch (target) {
			case TGSI_TEXTURE_3D:
				address[2] = lp_build_add(uint_bld, address[2],
						bld->immediates[off->Index][off->SwizzleZ]);
				/* fall through */
			case TGSI_TEXTURE_2D:
			case TGSI_TEXTURE_SHADOW2D:
			case TGSI_TEXTURE_RECT:
			case TGSI_TEXTURE_SHADOWRECT:
			case TGSI_TEXTURE_2D_ARRAY:
			case TGSI_TEXTURE_SHADOW2D_ARRAY:
				address[1] =
					lp_build_add(uint_bld, address[1],
						bld->immediates[off->Index][off->SwizzleY]);
				/* fall through */
			case TGSI_TEXTURE_1D:
			case TGSI_TEXTURE_SHADOW1D:
			case TGSI_TEXTURE_1D_ARRAY:
			case TGSI_TEXTURE_SHADOW1D_ARRAY:
				address[0] =
					lp_build_add(uint_bld, address[0],
						bld->immediates[off->Index][off->SwizzleX]);
				break;
				/* texture offsets do not apply to other texture targets */
			}
		}

		emit_data->args[2] = lp_build_const_int32(gallivm, target);
		emit_data->arg_count = 3;

		emit_data->dst_type = LLVMVectorType(
			LLVMInt32TypeInContext(gallivm->context),
			4);
	} else if (opcode == TGSI_OPCODE_TG4 ||
		   opcode == TGSI_OPCODE_LODQ ||
		   has_offset) {
		unsigned is_array = target == TGSI_TEXTURE_1D_ARRAY ||
				    target == TGSI_TEXTURE_SHADOW1D_ARRAY ||
				    target == TGSI_TEXTURE_2D_ARRAY ||
				    target == TGSI_TEXTURE_SHADOW2D_ARRAY ||
				    target == TGSI_TEXTURE_CUBE_ARRAY ||
				    target == TGSI_TEXTURE_SHADOWCUBE_ARRAY;
		unsigned is_rect = target == TGSI_TEXTURE_RECT;
		unsigned dmask = 0xf;

		if (opcode == TGSI_OPCODE_TG4) {
			unsigned gather_comp = 0;

			/* DMASK was repurposed for GATHER4. 4 components are always
			 * returned and DMASK works like a swizzle - it selects
			 * the component to fetch. The only valid DMASK values are
			 * 1=red, 2=green, 4=blue, 8=alpha. (e.g. 1 returns
			 * (red,red,red,red) etc.) The ISA document doesn't mention
			 * this.
			 */

			/* Get the component index from src1.x for Gather4. */
			if (!tgsi_is_shadow_sampler(target)) {
				LLVMValueRef (*imms)[4] = lp_soa_context(bld_base)->immediates;
				LLVMValueRef comp_imm;
				struct tgsi_src_register src1 = inst->Src[1].Register;

				assert(src1.File == TGSI_FILE_IMMEDIATE);

				comp_imm = imms[src1.Index][src1.SwizzleX];
				gather_comp = LLVMConstIntGetZExtValue(comp_imm);
				gather_comp = CLAMP(gather_comp, 0, 3);
			}

			dmask = 1 << gather_comp;
		}

		emit_data->args[2] = si_shader_ctx->samplers[sampler_index];
		emit_data->args[3] = lp_build_const_int32(gallivm, dmask);
		emit_data->args[4] = lp_build_const_int32(gallivm, is_rect); /* unorm */
		emit_data->args[5] = lp_build_const_int32(gallivm, 0); /* r128 */
		emit_data->args[6] = lp_build_const_int32(gallivm, is_array); /* da */
		emit_data->args[7] = lp_build_const_int32(gallivm, 0); /* glc */
		emit_data->args[8] = lp_build_const_int32(gallivm, 0); /* slc */
		emit_data->args[9] = lp_build_const_int32(gallivm, 0); /* tfe */
		emit_data->args[10] = lp_build_const_int32(gallivm, 0); /* lwe */

		emit_data->arg_count = 11;

		emit_data->dst_type = LLVMVectorType(
			LLVMFloatTypeInContext(gallivm->context),
			4);
	} else {
		emit_data->args[2] = si_shader_ctx->samplers[sampler_index];
		emit_data->args[3] = lp_build_const_int32(gallivm, target);
		emit_data->arg_count = 4;

		emit_data->dst_type = LLVMVectorType(
			LLVMFloatTypeInContext(gallivm->context),
			4);
	}

	/* The fetch opcode has been converted to a 2D array fetch.
	 * This simplifies the LLVM backend. */
	if (target == TGSI_TEXTURE_CUBE_ARRAY)
		target = TGSI_TEXTURE_2D_ARRAY;
	else if (target == TGSI_TEXTURE_SHADOWCUBE_ARRAY)
		target = TGSI_TEXTURE_SHADOW2D_ARRAY;

	/* Pad to power of two vector */
	while (count < util_next_power_of_two(count))
		address[count++] = LLVMGetUndef(LLVMInt32TypeInContext(gallivm->context));

	emit_data->args[0] = lp_build_gather_values(gallivm, address, count);
}

static void build_tex_intrinsic(const struct lp_build_tgsi_action * action,
				struct lp_build_tgsi_context * bld_base,
				struct lp_build_emit_data * emit_data)
{
	struct lp_build_context * base = &bld_base->base;
	unsigned opcode = emit_data->inst->Instruction.Opcode;
	unsigned target = emit_data->inst->Texture.Texture;
	char intr_name[127];
	bool has_offset = HAVE_LLVM >= 0x0305 ?
				emit_data->inst->Texture.NumOffsets > 0 : false;

	if (target == TGSI_TEXTURE_BUFFER) {
		emit_data->output[emit_data->chan] = build_intrinsic(
			base->gallivm->builder,
			"llvm.SI.vs.load.input", emit_data->dst_type,
			emit_data->args, emit_data->arg_count,
			LLVMReadNoneAttribute | LLVMNoUnwindAttribute);
		return;
	}

	if (opcode == TGSI_OPCODE_TG4 ||
	    opcode == TGSI_OPCODE_LODQ ||
	    (opcode != TGSI_OPCODE_TXF && has_offset)) {
		bool is_shadow = tgsi_is_shadow_sampler(target);
		const char *name = "llvm.SI.image.sample";
		const char *infix = "";

		switch (opcode) {
		case TGSI_OPCODE_TEX:
		case TGSI_OPCODE_TEX2:
		case TGSI_OPCODE_TXP:
			break;
		case TGSI_OPCODE_TXB:
		case TGSI_OPCODE_TXB2:
			infix = ".b";
			break;
		case TGSI_OPCODE_TXL:
		case TGSI_OPCODE_TXL2:
			infix = ".l";
			break;
		case TGSI_OPCODE_TXD:
			infix = ".d";
			break;
		case TGSI_OPCODE_TG4:
			name = "llvm.SI.gather4";
			break;
		case TGSI_OPCODE_LODQ:
			name = "llvm.SI.getlod";
			is_shadow = false;
			has_offset = false;
			break;
		default:
			assert(0);
			return;
		}

		/* Add the type and suffixes .c, .o if needed. */
		sprintf(intr_name, "%s%s%s%s.v%ui32", name,
			is_shadow ? ".c" : "", infix, has_offset ? ".o" : "",
			LLVMGetVectorSize(LLVMTypeOf(emit_data->args[0])));

		emit_data->output[emit_data->chan] = build_intrinsic(
			base->gallivm->builder, intr_name, emit_data->dst_type,
			emit_data->args, emit_data->arg_count,
			LLVMReadNoneAttribute | LLVMNoUnwindAttribute);
	} else {
		LLVMTypeRef i8, v16i8, v32i8;
		const char *name;

		switch (opcode) {
		case TGSI_OPCODE_TEX:
		case TGSI_OPCODE_TEX2:
		case TGSI_OPCODE_TXP:
			name = "llvm.SI.sample";
			break;
		case TGSI_OPCODE_TXB:
		case TGSI_OPCODE_TXB2:
			name = "llvm.SI.sampleb";
			break;
		case TGSI_OPCODE_TXD:
			name = "llvm.SI.sampled";
			break;
		case TGSI_OPCODE_TXF:
			name = "llvm.SI.imageload";
			break;
		case TGSI_OPCODE_TXL:
		case TGSI_OPCODE_TXL2:
			name = "llvm.SI.samplel";
			break;
		default:
			assert(0);
			return;
		}

		i8 = LLVMInt8TypeInContext(base->gallivm->context);
		v16i8 = LLVMVectorType(i8, 16);
		v32i8 = LLVMVectorType(i8, 32);

		emit_data->args[1] = LLVMBuildBitCast(base->gallivm->builder,
						emit_data->args[1], v32i8, "");
		if (opcode != TGSI_OPCODE_TXF) {
			emit_data->args[2] = LLVMBuildBitCast(base->gallivm->builder,
						emit_data->args[2], v16i8, "");
		}

		sprintf(intr_name, "%s.v%ui32", name,
			LLVMGetVectorSize(LLVMTypeOf(emit_data->args[0])));

		emit_data->output[emit_data->chan] = build_intrinsic(
			base->gallivm->builder, intr_name, emit_data->dst_type,
			emit_data->args, emit_data->arg_count,
			LLVMReadNoneAttribute | LLVMNoUnwindAttribute);
	}
}

static void txq_fetch_args(
	struct lp_build_tgsi_context * bld_base,
	struct lp_build_emit_data * emit_data)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	const struct tgsi_full_instruction *inst = emit_data->inst;
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	unsigned target = inst->Texture.Texture;

	if (target == TGSI_TEXTURE_BUFFER) {
		LLVMTypeRef i32 = LLVMInt32TypeInContext(gallivm->context);
		LLVMTypeRef v8i32 = LLVMVectorType(i32, 8);

		/* Read the size from the buffer descriptor directly. */
		LLVMValueRef size = si_shader_ctx->resources[inst->Src[1].Register.Index];
		size = LLVMBuildBitCast(gallivm->builder, size, v8i32, "");
		size = LLVMBuildExtractElement(gallivm->builder, size,
					      lp_build_const_int32(gallivm, 6), "");
		emit_data->args[0] = size;
		return;
	}

	/* Mip level */
	emit_data->args[0] = lp_build_emit_fetch(bld_base, inst, 0, TGSI_CHAN_X);

	/* Resource */
	emit_data->args[1] = si_shader_ctx->resources[inst->Src[1].Register.Index];

	/* Texture target */
	if (target == TGSI_TEXTURE_CUBE_ARRAY ||
	    target == TGSI_TEXTURE_SHADOWCUBE_ARRAY)
		target = TGSI_TEXTURE_2D_ARRAY;

	emit_data->args[2] = lp_build_const_int32(bld_base->base.gallivm,
						  target);

	emit_data->arg_count = 3;

	emit_data->dst_type = LLVMVectorType(
		LLVMInt32TypeInContext(bld_base->base.gallivm->context),
		4);
}

static void build_txq_intrinsic(const struct lp_build_tgsi_action * action,
				struct lp_build_tgsi_context * bld_base,
				struct lp_build_emit_data * emit_data)
{
	unsigned target = emit_data->inst->Texture.Texture;

	if (target == TGSI_TEXTURE_BUFFER) {
		/* Just return the buffer size. */
		emit_data->output[emit_data->chan] = emit_data->args[0];
		return;
	}

	build_tgsi_intrinsic_nomem(action, bld_base, emit_data);

	/* Divide the number of layers by 6 to get the number of cubes. */
	if (target == TGSI_TEXTURE_CUBE_ARRAY ||
	    target == TGSI_TEXTURE_SHADOWCUBE_ARRAY) {
		LLVMBuilderRef builder = bld_base->base.gallivm->builder;
		LLVMValueRef two = lp_build_const_int32(bld_base->base.gallivm, 2);
		LLVMValueRef six = lp_build_const_int32(bld_base->base.gallivm, 6);

		LLVMValueRef v4 = emit_data->output[emit_data->chan];
		LLVMValueRef z = LLVMBuildExtractElement(builder, v4, two, "");
		z = LLVMBuildSDiv(builder, z, six, "");

		emit_data->output[emit_data->chan] =
			LLVMBuildInsertElement(builder, v4, z, two, "");
	}
}

static void si_llvm_emit_ddxy(
	const struct lp_build_tgsi_action * action,
	struct lp_build_tgsi_context * bld_base,
	struct lp_build_emit_data * emit_data)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	struct lp_build_context * base = &bld_base->base;
	const struct tgsi_full_instruction *inst = emit_data->inst;
	unsigned opcode = inst->Instruction.Opcode;
	LLVMValueRef indices[2];
	LLVMValueRef store_ptr, load_ptr0, load_ptr1;
	LLVMValueRef tl, trbl, result[4];
	LLVMTypeRef i32;
	unsigned swizzle[4];
	unsigned c;

	i32 = LLVMInt32TypeInContext(gallivm->context);

	indices[0] = bld_base->uint_bld.zero;
	indices[1] = build_intrinsic(gallivm->builder, "llvm.SI.tid", i32,
				     NULL, 0, LLVMReadNoneAttribute);
	store_ptr = LLVMBuildGEP(gallivm->builder, si_shader_ctx->ddxy_lds,
				 indices, 2, "");

	indices[1] = LLVMBuildAnd(gallivm->builder, indices[1],
				  lp_build_const_int32(gallivm, 0xfffffffc), "");
	load_ptr0 = LLVMBuildGEP(gallivm->builder, si_shader_ctx->ddxy_lds,
				 indices, 2, "");

	indices[1] = LLVMBuildAdd(gallivm->builder, indices[1],
				  lp_build_const_int32(gallivm,
						       opcode == TGSI_OPCODE_DDX ? 1 : 2),
				  "");
	load_ptr1 = LLVMBuildGEP(gallivm->builder, si_shader_ctx->ddxy_lds,
				 indices, 2, "");

	for (c = 0; c < 4; ++c) {
		unsigned i;

		swizzle[c] = tgsi_util_get_full_src_register_swizzle(&inst->Src[0], c);
		for (i = 0; i < c; ++i) {
			if (swizzle[i] == swizzle[c]) {
				result[c] = result[i];
				break;
			}
		}
		if (i != c)
			continue;

		LLVMBuildStore(gallivm->builder,
			       LLVMBuildBitCast(gallivm->builder,
						lp_build_emit_fetch(bld_base, inst, 0, c),
						i32, ""),
			       store_ptr);

		tl = LLVMBuildLoad(gallivm->builder, load_ptr0, "");
		tl = LLVMBuildBitCast(gallivm->builder, tl, base->elem_type, "");

		trbl = LLVMBuildLoad(gallivm->builder, load_ptr1, "");
		trbl = LLVMBuildBitCast(gallivm->builder, trbl,	base->elem_type, "");

		result[c] = LLVMBuildFSub(gallivm->builder, trbl, tl, "");
	}

	emit_data->output[0] = lp_build_gather_values(gallivm, result, 4);
}

/* Emit one vertex from the geometry shader */
static void si_llvm_emit_vertex(
	const struct lp_build_tgsi_action *action,
	struct lp_build_tgsi_context *bld_base,
	struct lp_build_emit_data *emit_data)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct lp_build_context *uint = &bld_base->uint_bld;
	struct si_shader *shader = si_shader_ctx->shader;
	struct tgsi_shader_info *info = &shader->selector->info;
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	LLVMTypeRef i32 = LLVMInt32TypeInContext(gallivm->context);
	LLVMValueRef soffset = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
					    SI_PARAM_GS2VS_OFFSET);
	LLVMValueRef gs_next_vertex;
	LLVMValueRef can_emit, kill;
	LLVMValueRef args[2];
	unsigned chan;
	int i;

	/* Write vertex attribute values to GSVS ring */
	gs_next_vertex = LLVMBuildLoad(gallivm->builder, si_shader_ctx->gs_next_vertex, "");

	/* If this thread has already emitted the declared maximum number of
	 * vertices, kill it: excessive vertex emissions are not supposed to
	 * have any effect, and GS threads have no externally observable
	 * effects other than emitting vertices.
	 */
	can_emit = LLVMBuildICmp(gallivm->builder, LLVMIntULE, gs_next_vertex,
				 lp_build_const_int32(gallivm,
						      shader->selector->gs_max_out_vertices), "");
	kill = lp_build_select(&bld_base->base, can_emit,
			       lp_build_const_float(gallivm, 1.0f),
			       lp_build_const_float(gallivm, -1.0f));
	build_intrinsic(gallivm->builder, "llvm.AMDGPU.kill",
			LLVMVoidTypeInContext(gallivm->context), &kill, 1, 0);

	for (i = 0; i < info->num_outputs; i++) {
		LLVMValueRef *out_ptr =
			si_shader_ctx->radeon_bld.soa.outputs[i];

		for (chan = 0; chan < 4; chan++) {
			LLVMValueRef out_val = LLVMBuildLoad(gallivm->builder, out_ptr[chan], "");
			LLVMValueRef voffset =
				lp_build_const_int32(gallivm, (i * 4 + chan) *
						     shader->selector->gs_max_out_vertices);

			voffset = lp_build_add(uint, voffset, gs_next_vertex);
			voffset = lp_build_mul_imm(uint, voffset, 4);

			out_val = LLVMBuildBitCast(gallivm->builder, out_val, i32, "");

			build_tbuffer_store(si_shader_ctx,
					    si_shader_ctx->gsvs_ring,
					    out_val, 1,
					    voffset, soffset, 0,
					    V_008F0C_BUF_DATA_FORMAT_32,
					    V_008F0C_BUF_NUM_FORMAT_UINT,
					    1, 0, 1, 1, 0);
		}
	}
	gs_next_vertex = lp_build_add(uint, gs_next_vertex,
				      lp_build_const_int32(gallivm, 1));
	LLVMBuildStore(gallivm->builder, gs_next_vertex, si_shader_ctx->gs_next_vertex);

	/* Signal vertex emission */
	args[0] = lp_build_const_int32(gallivm, SENDMSG_GS_OP_EMIT | SENDMSG_GS);
	args[1] = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn, SI_PARAM_GS_WAVE_ID);
	build_intrinsic(gallivm->builder, "llvm.SI.sendmsg",
			LLVMVoidTypeInContext(gallivm->context), args, 2,
			LLVMNoUnwindAttribute);
}

/* Cut one primitive from the geometry shader */
static void si_llvm_emit_primitive(
	const struct lp_build_tgsi_action *action,
	struct lp_build_tgsi_context *bld_base,
	struct lp_build_emit_data *emit_data)
{
	struct si_shader_context *si_shader_ctx = si_shader_context(bld_base);
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	LLVMValueRef args[2];

	/* Signal primitive cut */
	args[0] = lp_build_const_int32(gallivm,	SENDMSG_GS_OP_CUT | SENDMSG_GS);
	args[1] = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn, SI_PARAM_GS_WAVE_ID);
	build_intrinsic(gallivm->builder, "llvm.SI.sendmsg",
			LLVMVoidTypeInContext(gallivm->context), args, 2,
			LLVMNoUnwindAttribute);
}

static const struct lp_build_tgsi_action tex_action = {
	.fetch_args = tex_fetch_args,
	.emit = build_tex_intrinsic,
};

static const struct lp_build_tgsi_action txq_action = {
	.fetch_args = txq_fetch_args,
	.emit = build_txq_intrinsic,
	.intr_name = "llvm.SI.resinfo"
};

static void create_meta_data(struct si_shader_context *si_shader_ctx)
{
	struct gallivm_state *gallivm = si_shader_ctx->radeon_bld.soa.bld_base.base.gallivm;
	LLVMValueRef args[3];

	args[0] = LLVMMDStringInContext(gallivm->context, "const", 5);
	args[1] = 0;
	args[2] = lp_build_const_int32(gallivm, 1);

	si_shader_ctx->const_md = LLVMMDNodeInContext(gallivm->context, args, 3);
}

static LLVMTypeRef const_array(LLVMTypeRef elem_type, int num_elements)
{
	return LLVMPointerType(LLVMArrayType(elem_type, num_elements),
			       CONST_ADDR_SPACE);
}

static void create_function(struct si_shader_context *si_shader_ctx)
{
	struct lp_build_tgsi_context *bld_base = &si_shader_ctx->radeon_bld.soa.bld_base;
	struct gallivm_state *gallivm = bld_base->base.gallivm;
	struct si_shader *shader = si_shader_ctx->shader;
	LLVMTypeRef params[SI_NUM_PARAMS], f32, i8, i32, v2i32, v3i32, v16i8, v4i32, v8i32;
	unsigned i, last_array_pointer, last_sgpr, num_params;

	i8 = LLVMInt8TypeInContext(gallivm->context);
	i32 = LLVMInt32TypeInContext(gallivm->context);
	f32 = LLVMFloatTypeInContext(gallivm->context);
	v2i32 = LLVMVectorType(i32, 2);
	v3i32 = LLVMVectorType(i32, 3);
	v4i32 = LLVMVectorType(i32, 4);
	v8i32 = LLVMVectorType(i32, 8);
	v16i8 = LLVMVectorType(i8, 16);

	params[SI_PARAM_RW_BUFFERS] = const_array(v16i8, SI_NUM_RW_BUFFERS);
	params[SI_PARAM_CONST] = const_array(v16i8, SI_NUM_CONST_BUFFERS);
	params[SI_PARAM_SAMPLER] = const_array(v4i32, SI_NUM_SAMPLER_STATES);
	params[SI_PARAM_RESOURCE] = const_array(v8i32, SI_NUM_SAMPLER_VIEWS);
	last_array_pointer = SI_PARAM_RESOURCE;

	switch (si_shader_ctx->type) {
	case TGSI_PROCESSOR_VERTEX:
		params[SI_PARAM_VERTEX_BUFFER] = const_array(v16i8, SI_NUM_VERTEX_BUFFERS);
		last_array_pointer = SI_PARAM_VERTEX_BUFFER;
		params[SI_PARAM_BASE_VERTEX] = i32;
		params[SI_PARAM_START_INSTANCE] = i32;
		num_params = SI_PARAM_START_INSTANCE+1;

		if (shader->key.vs.as_es) {
			params[SI_PARAM_ES2GS_OFFSET] = i32;
			num_params++;
		} else {
			if (shader->is_gs_copy_shader) {
				last_array_pointer = SI_PARAM_CONST;
				num_params = SI_PARAM_CONST+1;
			}

			/* The locations of the other parameters are assigned dynamically. */

			/* Streamout SGPRs. */
			if (shader->selector->so.num_outputs) {
				params[si_shader_ctx->param_streamout_config = num_params++] = i32;
				params[si_shader_ctx->param_streamout_write_index = num_params++] = i32;
			}
			/* A streamout buffer offset is loaded if the stride is non-zero. */
			for (i = 0; i < 4; i++) {
				if (!shader->selector->so.stride[i])
					continue;

				params[si_shader_ctx->param_streamout_offset[i] = num_params++] = i32;
			}
		}

		last_sgpr = num_params-1;

		/* VGPRs */
		params[si_shader_ctx->param_vertex_id = num_params++] = i32;
		params[num_params++] = i32; /* unused*/
		params[num_params++] = i32; /* unused */
		params[si_shader_ctx->param_instance_id = num_params++] = i32;
		break;

	case TGSI_PROCESSOR_GEOMETRY:
		params[SI_PARAM_GS2VS_OFFSET] = i32;
		params[SI_PARAM_GS_WAVE_ID] = i32;
		last_sgpr = SI_PARAM_GS_WAVE_ID;

		/* VGPRs */
		params[SI_PARAM_VTX0_OFFSET] = i32;
		params[SI_PARAM_VTX1_OFFSET] = i32;
		params[SI_PARAM_PRIMITIVE_ID] = i32;
		params[SI_PARAM_VTX2_OFFSET] = i32;
		params[SI_PARAM_VTX3_OFFSET] = i32;
		params[SI_PARAM_VTX4_OFFSET] = i32;
		params[SI_PARAM_VTX5_OFFSET] = i32;
		params[SI_PARAM_GS_INSTANCE_ID] = i32;
		num_params = SI_PARAM_GS_INSTANCE_ID+1;
		break;

	case TGSI_PROCESSOR_FRAGMENT:
		params[SI_PARAM_ALPHA_REF] = f32;
		params[SI_PARAM_PRIM_MASK] = i32;
		last_sgpr = SI_PARAM_PRIM_MASK;
		params[SI_PARAM_PERSP_SAMPLE] = v2i32;
		params[SI_PARAM_PERSP_CENTER] = v2i32;
		params[SI_PARAM_PERSP_CENTROID] = v2i32;
		params[SI_PARAM_PERSP_PULL_MODEL] = v3i32;
		params[SI_PARAM_LINEAR_SAMPLE] = v2i32;
		params[SI_PARAM_LINEAR_CENTER] = v2i32;
		params[SI_PARAM_LINEAR_CENTROID] = v2i32;
		params[SI_PARAM_LINE_STIPPLE_TEX] = f32;
		params[SI_PARAM_POS_X_FLOAT] = f32;
		params[SI_PARAM_POS_Y_FLOAT] = f32;
		params[SI_PARAM_POS_Z_FLOAT] = f32;
		params[SI_PARAM_POS_W_FLOAT] = f32;
		params[SI_PARAM_FRONT_FACE] = f32;
		params[SI_PARAM_ANCILLARY] = i32;
		params[SI_PARAM_SAMPLE_COVERAGE] = f32;
		params[SI_PARAM_POS_FIXED_PT] = f32;
		num_params = SI_PARAM_POS_FIXED_PT+1;
		break;

	default:
		assert(0 && "unimplemented shader");
		return;
	}

	assert(num_params <= Elements(params));
	radeon_llvm_create_func(&si_shader_ctx->radeon_bld, params, num_params);
	radeon_llvm_shader_type(si_shader_ctx->radeon_bld.main_fn, si_shader_ctx->type);

	if (shader->dx10_clamp_mode)
		LLVMAddTargetDependentFunctionAttr(si_shader_ctx->radeon_bld.main_fn,
						   "enable-no-nans-fp-math", "true");

	for (i = 0; i <= last_sgpr; ++i) {
		LLVMValueRef P = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn, i);

		/* We tell llvm that array inputs are passed by value to allow Sinking pass
		 * to move load. Inputs are constant so this is fine. */
		if (i <= last_array_pointer)
			LLVMAddAttribute(P, LLVMByValAttribute);
		else
			LLVMAddAttribute(P, LLVMInRegAttribute);
	}

	if (bld_base->info &&
	    (bld_base->info->opcode_count[TGSI_OPCODE_DDX] > 0 ||
	     bld_base->info->opcode_count[TGSI_OPCODE_DDY] > 0))
		si_shader_ctx->ddxy_lds =
			LLVMAddGlobalInAddressSpace(gallivm->module,
						    LLVMArrayType(i32, 64),
						    "ddxy_lds",
						    LOCAL_ADDR_SPACE);
}

static void preload_constants(struct si_shader_context *si_shader_ctx)
{
	struct lp_build_tgsi_context * bld_base = &si_shader_ctx->radeon_bld.soa.bld_base;
	struct gallivm_state * gallivm = bld_base->base.gallivm;
	const struct tgsi_shader_info * info = bld_base->info;
	unsigned buf;
	LLVMValueRef ptr = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn, SI_PARAM_CONST);

	for (buf = 0; buf < SI_NUM_CONST_BUFFERS; buf++) {
		unsigned i, num_const = info->const_file_max[buf] + 1;

		if (num_const == 0)
			continue;

		/* Allocate space for the constant values */
		si_shader_ctx->constants[buf] = CALLOC(num_const * 4, sizeof(LLVMValueRef));

		/* Load the resource descriptor */
		si_shader_ctx->const_resource[buf] =
			build_indexed_load_const(si_shader_ctx, ptr, lp_build_const_int32(gallivm, buf));

		/* Load the constants, we rely on the code sinking to do the rest */
		for (i = 0; i < num_const * 4; ++i) {
			si_shader_ctx->constants[buf][i] =
				buffer_load_const(gallivm->builder,
					si_shader_ctx->const_resource[buf],
					lp_build_const_int32(gallivm, i * 4),
					bld_base->base.elem_type);
		}
	}
}

static void preload_samplers(struct si_shader_context *si_shader_ctx)
{
	struct lp_build_tgsi_context * bld_base = &si_shader_ctx->radeon_bld.soa.bld_base;
	struct gallivm_state * gallivm = bld_base->base.gallivm;
	const struct tgsi_shader_info * info = bld_base->info;

	unsigned i, num_samplers = info->file_max[TGSI_FILE_SAMPLER] + 1;

	LLVMValueRef res_ptr, samp_ptr;
	LLVMValueRef offset;

	if (num_samplers == 0)
		return;

	res_ptr = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn, SI_PARAM_RESOURCE);
	samp_ptr = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn, SI_PARAM_SAMPLER);

	/* Load the resources and samplers, we rely on the code sinking to do the rest */
	for (i = 0; i < num_samplers; ++i) {
		/* Resource */
		offset = lp_build_const_int32(gallivm, i);
		si_shader_ctx->resources[i] = build_indexed_load_const(si_shader_ctx, res_ptr, offset);

		/* Sampler */
		offset = lp_build_const_int32(gallivm, i);
		si_shader_ctx->samplers[i] = build_indexed_load_const(si_shader_ctx, samp_ptr, offset);

		/* FMASK resource */
		if (info->is_msaa_sampler[i]) {
			offset = lp_build_const_int32(gallivm, SI_FMASK_TEX_OFFSET + i);
			si_shader_ctx->resources[SI_FMASK_TEX_OFFSET + i] =
				build_indexed_load_const(si_shader_ctx, res_ptr, offset);
		}
	}
}

static void preload_streamout_buffers(struct si_shader_context *si_shader_ctx)
{
	struct lp_build_tgsi_context * bld_base = &si_shader_ctx->radeon_bld.soa.bld_base;
	struct gallivm_state * gallivm = bld_base->base.gallivm;
	unsigned i;

	if (si_shader_ctx->type != TGSI_PROCESSOR_VERTEX ||
	    si_shader_ctx->shader->key.vs.as_es ||
	    !si_shader_ctx->shader->selector->so.num_outputs)
		return;

	LLVMValueRef buf_ptr = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
					    SI_PARAM_RW_BUFFERS);

	/* Load the resources, we rely on the code sinking to do the rest */
	for (i = 0; i < 4; ++i) {
		if (si_shader_ctx->shader->selector->so.stride[i]) {
			LLVMValueRef offset = lp_build_const_int32(gallivm,
								   SI_SO_BUF_OFFSET + i);

			si_shader_ctx->so_buffers[i] = build_indexed_load_const(si_shader_ctx, buf_ptr, offset);
		}
	}
}

/**
 * Load ESGS and GSVS ring buffer resource descriptors and save the variables
 * for later use.
 */
static void preload_ring_buffers(struct si_shader_context *si_shader_ctx)
{
	struct gallivm_state *gallivm =
		si_shader_ctx->radeon_bld.soa.bld_base.base.gallivm;

	LLVMValueRef buf_ptr = LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
					    SI_PARAM_RW_BUFFERS);

	if ((si_shader_ctx->type == TGSI_PROCESSOR_VERTEX &&
	     si_shader_ctx->shader->key.vs.as_es) ||
	    si_shader_ctx->type == TGSI_PROCESSOR_GEOMETRY) {
		LLVMValueRef offset = lp_build_const_int32(gallivm, SI_RING_ESGS);

		si_shader_ctx->esgs_ring =
			build_indexed_load_const(si_shader_ctx, buf_ptr, offset);
	}

	if (si_shader_ctx->type == TGSI_PROCESSOR_GEOMETRY ||
	    si_shader_ctx->shader->is_gs_copy_shader) {
		LLVMValueRef offset = lp_build_const_int32(gallivm, SI_RING_GSVS);

		si_shader_ctx->gsvs_ring =
			build_indexed_load_const(si_shader_ctx, buf_ptr, offset);
	}
}

void si_shader_binary_read_config(const struct si_screen *sscreen,
				struct si_shader *shader,
				unsigned symbol_offset)
{
	unsigned i;
	const unsigned char *config =
		radeon_shader_binary_config_start(&shader->binary,
						symbol_offset);

	/* XXX: We may be able to emit some of these values directly rather than
	 * extracting fields to be emitted later.
	 */

	for (i = 0; i < shader->binary.config_size_per_symbol; i+= 8) {
		unsigned reg = util_le32_to_cpu(*(uint32_t*)(config + i));
		unsigned value = util_le32_to_cpu(*(uint32_t*)(config + i + 4));
		switch (reg) {
		case R_00B028_SPI_SHADER_PGM_RSRC1_PS:
		case R_00B128_SPI_SHADER_PGM_RSRC1_VS:
		case R_00B228_SPI_SHADER_PGM_RSRC1_GS:
		case R_00B848_COMPUTE_PGM_RSRC1:
			shader->num_sgprs = MAX2(shader->num_sgprs, (G_00B028_SGPRS(value) + 1) * 8);
			shader->num_vgprs = MAX2(shader->num_vgprs, (G_00B028_VGPRS(value) + 1) * 4);
			shader->float_mode =  G_00B028_FLOAT_MODE(value);
			break;
		case R_00B02C_SPI_SHADER_PGM_RSRC2_PS:
			shader->lds_size = MAX2(shader->lds_size, G_00B02C_EXTRA_LDS_SIZE(value));
			break;
		case R_00B84C_COMPUTE_PGM_RSRC2:
			shader->lds_size = MAX2(shader->lds_size, G_00B84C_LDS_SIZE(value));
			break;
		case R_0286CC_SPI_PS_INPUT_ENA:
			shader->spi_ps_input_ena = value;
			break;
		case R_0286E8_SPI_TMPRING_SIZE:
		case R_00B860_COMPUTE_TMPRING_SIZE:
			/* WAVESIZE is in units of 256 dwords. */
			shader->scratch_bytes_per_wave =
				G_00B860_WAVESIZE(value) * 256 * 4 * 1;
			break;
		default:
			fprintf(stderr, "Warning: Compiler emitted unknown "
				"config register: 0x%x\n", reg);
			break;
		}
	}
}

void si_shader_apply_scratch_relocs(struct si_context *sctx,
			struct si_shader *shader,
			uint64_t scratch_va)
{
	unsigned i;
	uint32_t scratch_rsrc_dword0 = scratch_va & 0xffffffff;
	uint32_t scratch_rsrc_dword1 =
		S_008F04_BASE_ADDRESS_HI(scratch_va >> 32)
		|  S_008F04_STRIDE(shader->scratch_bytes_per_wave / 64);

	for (i = 0 ; i < shader->binary.reloc_count; i++) {
		const struct radeon_shader_reloc *reloc =
					&shader->binary.relocs[i];
		if (!strcmp(scratch_rsrc_dword0_symbol, reloc->name)) {
			util_memcpy_cpu_to_le32(shader->binary.code + reloc->offset,
			&scratch_rsrc_dword0, 4);
		} else if (!strcmp(scratch_rsrc_dword1_symbol, reloc->name)) {
			util_memcpy_cpu_to_le32(shader->binary.code + reloc->offset,
			&scratch_rsrc_dword1, 4);
		}
	}
}

int si_shader_binary_read(struct si_screen *sscreen,
			struct si_shader *shader,
			const struct radeon_shader_binary *binary)
{

	unsigned i;
	unsigned code_size;
	unsigned char *ptr;
	bool dump  = r600_can_dump_shader(&sscreen->b,
		shader->selector ? shader->selector->tokens : NULL);

	si_shader_binary_read_config(sscreen, shader, 0);

	if (dump) {
		if (!binary->disassembled) {
			fprintf(stderr, "SI CODE:\n");
			for (i = 0; i < binary->code_size; i+=4 ) {
				fprintf(stderr, "@0x%x: %02x%02x%02x%02x\n", i, binary->code[i + 3],
				binary->code[i + 2], binary->code[i + 1],
				binary->code[i]);
			}
		}

		fprintf(stderr, "*** SHADER STATS ***\n"
			"SGPRS: %d\nVGPRS: %d\nCode Size: %d bytes\nLDS: %d blocks\n"
			"Scratch: %d bytes per wave\n********************\n",
			shader->num_sgprs, shader->num_vgprs, binary->code_size,
			shader->lds_size, shader->scratch_bytes_per_wave);
	}

	/* copy new shader */
	code_size = binary->code_size + binary->rodata_size;
	r600_resource_reference(&shader->bo, NULL);
	shader->bo = si_resource_create_custom(&sscreen->b.b, PIPE_USAGE_IMMUTABLE,
					       code_size);
	if (shader->bo == NULL) {
		return -ENOMEM;
	}


	ptr = sscreen->b.ws->buffer_map(shader->bo->cs_buf, NULL, PIPE_TRANSFER_READ_WRITE);
	util_memcpy_cpu_to_le32(ptr, binary->code, binary->code_size);
	if (binary->rodata_size > 0) {
		ptr += binary->code_size;
		util_memcpy_cpu_to_le32(ptr, binary->rodata, binary->rodata_size);
	}

	sscreen->b.ws->buffer_unmap(shader->bo->cs_buf);

	return 0;
}

int si_compile_llvm(struct si_screen *sscreen, struct si_shader *shader,
		    LLVMTargetMachineRef tm, LLVMModuleRef mod)
{
	int r = 0;
	bool dump = r600_can_dump_shader(&sscreen->b,
			shader->selector ? shader->selector->tokens : NULL);
	r = radeon_llvm_compile(mod, &shader->binary,
		r600_get_llvm_processor_name(sscreen->b.family), dump, tm);

	if (r) {
		return r;
	}
	r = si_shader_binary_read(sscreen, shader, &shader->binary);

	FREE(shader->binary.config);
	FREE(shader->binary.rodata);
	FREE(shader->binary.global_symbol_offsets);
	if (shader->scratch_bytes_per_wave == 0) {
		FREE(shader->binary.code);
		FREE(shader->binary.relocs);
		memset(&shader->binary, 0, sizeof(shader->binary));
	}
	return r;
}

/* Generate code for the hardware VS shader stage to go with a geometry shader */
static int si_generate_gs_copy_shader(struct si_screen *sscreen,
				      struct si_shader_context *si_shader_ctx,
				      struct si_shader *gs, bool dump)
{
	struct gallivm_state *gallivm = &si_shader_ctx->radeon_bld.gallivm;
	struct lp_build_tgsi_context *bld_base = &si_shader_ctx->radeon_bld.soa.bld_base;
	struct lp_build_context *base = &bld_base->base;
	struct lp_build_context *uint = &bld_base->uint_bld;
	struct si_shader *shader = si_shader_ctx->shader;
	struct si_shader_output_values *outputs;
	struct tgsi_shader_info *gsinfo = &gs->selector->info;
	LLVMValueRef args[9];
	int i, r;

	outputs = MALLOC(gsinfo->num_outputs * sizeof(outputs[0]));

	si_shader_ctx->type = TGSI_PROCESSOR_VERTEX;
	shader->is_gs_copy_shader = true;

	radeon_llvm_context_init(&si_shader_ctx->radeon_bld);

	create_meta_data(si_shader_ctx);
	create_function(si_shader_ctx);
	preload_streamout_buffers(si_shader_ctx);
	preload_ring_buffers(si_shader_ctx);

	args[0] = si_shader_ctx->gsvs_ring;
	args[1] = lp_build_mul_imm(uint,
				   LLVMGetParam(si_shader_ctx->radeon_bld.main_fn,
						si_shader_ctx->param_vertex_id),
				   4);
	args[3] = uint->zero;
	args[4] = uint->one;  /* OFFEN */
	args[5] = uint->zero; /* IDXEN */
	args[6] = uint->one;  /* GLC */
	args[7] = uint->one;  /* SLC */
	args[8] = uint->zero; /* TFE */

	/* Fetch vertex data from GSVS ring */
	for (i = 0; i < gsinfo->num_outputs; ++i) {
		unsigned chan;

		outputs[i].name = gsinfo->output_semantic_name[i];
		outputs[i].sid = gsinfo->output_semantic_index[i];

		for (chan = 0; chan < 4; chan++) {
			args[2] = lp_build_const_int32(gallivm,
						       (i * 4 + chan) *
						       gs->selector->gs_max_out_vertices * 16 * 4);

			outputs[i].values[chan] =
				LLVMBuildBitCast(gallivm->builder,
						 build_intrinsic(gallivm->builder,
								 "llvm.SI.buffer.load.dword.i32.i32",
								 LLVMInt32TypeInContext(gallivm->context),
								 args, 9,
								 LLVMReadOnlyAttribute | LLVMNoUnwindAttribute),
						 base->elem_type, "");
		}
	}

	si_llvm_export_vs(bld_base, outputs, gsinfo->num_outputs);

	radeon_llvm_finalize_module(&si_shader_ctx->radeon_bld);

	if (dump)
		fprintf(stderr, "Copy Vertex Shader for Geometry Shader:\n\n");

	r = si_compile_llvm(sscreen, si_shader_ctx->shader,
			    si_shader_ctx->tm, bld_base->base.gallivm->module);

	radeon_llvm_dispose(&si_shader_ctx->radeon_bld);

	FREE(outputs);
	return r;
}

static void si_dump_key(unsigned shader, union si_shader_key *key)
{
	int i;

	fprintf(stderr, "SHADER KEY\n");

	switch (shader) {
	case PIPE_SHADER_VERTEX:
		fprintf(stderr, "  instance_divisors = {");
		for (i = 0; i < Elements(key->vs.instance_divisors); i++)
			fprintf(stderr, !i ? "%u" : ", %u",
				key->vs.instance_divisors[i]);
		fprintf(stderr, "}\n");

		if (key->vs.as_es)
			fprintf(stderr, "  gs_used_inputs = 0x%"PRIx64"\n",
				key->vs.gs_used_inputs);
		fprintf(stderr, "  as_es = %u\n", key->vs.as_es);
		break;

	case PIPE_SHADER_GEOMETRY:
		break;

	case PIPE_SHADER_FRAGMENT:
		fprintf(stderr, "  export_16bpc = 0x%X\n", key->ps.export_16bpc);
		fprintf(stderr, "  last_cbuf = %u\n", key->ps.last_cbuf);
		fprintf(stderr, "  color_two_side = %u\n", key->ps.color_two_side);
		fprintf(stderr, "  alpha_func = %u\n", key->ps.alpha_func);
		fprintf(stderr, "  alpha_to_one = %u\n", key->ps.alpha_to_one);
		fprintf(stderr, "  poly_stipple = %u\n", key->ps.poly_stipple);
		break;

	default:
		assert(0);
	}
}

int si_shader_create(struct si_screen *sscreen, LLVMTargetMachineRef tm,
		     struct si_shader *shader)
{
	struct si_shader_selector *sel = shader->selector;
	struct tgsi_token *tokens = sel->tokens;
	struct si_shader_context si_shader_ctx;
	struct lp_build_tgsi_context * bld_base;
	struct tgsi_shader_info stipple_shader_info;
	LLVMModuleRef mod;
	int r = 0;
	bool poly_stipple = sel->type == PIPE_SHADER_FRAGMENT &&
			    shader->key.ps.poly_stipple;
	bool dump = r600_can_dump_shader(&sscreen->b, sel->tokens);

	if (poly_stipple) {
		tokens = util_pstipple_create_fragment_shader(tokens, NULL,
						SI_POLY_STIPPLE_SAMPLER);
		tgsi_scan_shader(tokens, &stipple_shader_info);
	}

	/* Dump TGSI code before doing TGSI->LLVM conversion in case the
	 * conversion fails. */
	if (dump) {
		si_dump_key(sel->type, &shader->key);
		tgsi_dump(tokens, 0);
		si_dump_streamout(&sel->so);
	}

	assert(shader->nparam == 0);

	memset(&si_shader_ctx, 0, sizeof(si_shader_ctx));
	radeon_llvm_context_init(&si_shader_ctx.radeon_bld);
	bld_base = &si_shader_ctx.radeon_bld.soa.bld_base;

	if (sel->type != PIPE_SHADER_COMPUTE)
		shader->dx10_clamp_mode = true;

	if (sel->info.uses_kill)
		shader->db_shader_control |= S_02880C_KILL_ENABLE(1);

	shader->uses_instanceid = sel->info.uses_instanceid;
	bld_base->info = poly_stipple ? &stipple_shader_info : &sel->info;
	bld_base->emit_fetch_funcs[TGSI_FILE_CONSTANT] = fetch_constant;

	bld_base->op_actions[TGSI_OPCODE_TEX] = tex_action;
	bld_base->op_actions[TGSI_OPCODE_TEX2] = tex_action;
	bld_base->op_actions[TGSI_OPCODE_TXB] = tex_action;
	bld_base->op_actions[TGSI_OPCODE_TXB2] = tex_action;
	bld_base->op_actions[TGSI_OPCODE_TXD] = tex_action;
	bld_base->op_actions[TGSI_OPCODE_TXF] = tex_action;
	bld_base->op_actions[TGSI_OPCODE_TXL] = tex_action;
	bld_base->op_actions[TGSI_OPCODE_TXL2] = tex_action;
	bld_base->op_actions[TGSI_OPCODE_TXP] = tex_action;
	bld_base->op_actions[TGSI_OPCODE_TXQ] = txq_action;
	bld_base->op_actions[TGSI_OPCODE_TG4] = tex_action;
	bld_base->op_actions[TGSI_OPCODE_LODQ] = tex_action;

	bld_base->op_actions[TGSI_OPCODE_DDX].emit = si_llvm_emit_ddxy;
	bld_base->op_actions[TGSI_OPCODE_DDY].emit = si_llvm_emit_ddxy;

	bld_base->op_actions[TGSI_OPCODE_EMIT].emit = si_llvm_emit_vertex;
	bld_base->op_actions[TGSI_OPCODE_ENDPRIM].emit = si_llvm_emit_primitive;

	if (HAVE_LLVM >= 0x0306) {
		bld_base->op_actions[TGSI_OPCODE_MAX].emit = build_tgsi_intrinsic_nomem;
		bld_base->op_actions[TGSI_OPCODE_MAX].intr_name = "llvm.maxnum.f32";
		bld_base->op_actions[TGSI_OPCODE_MIN].emit = build_tgsi_intrinsic_nomem;
		bld_base->op_actions[TGSI_OPCODE_MIN].intr_name = "llvm.minnum.f32";
	}

	si_shader_ctx.radeon_bld.load_system_value = declare_system_value;
	si_shader_ctx.shader = shader;
	si_shader_ctx.type = tgsi_get_processor_type(tokens);
	si_shader_ctx.screen = sscreen;
	si_shader_ctx.tm = tm;

	switch (si_shader_ctx.type) {
	case TGSI_PROCESSOR_VERTEX:
		si_shader_ctx.radeon_bld.load_input = declare_input_vs;
		if (shader->key.vs.as_es) {
			bld_base->emit_epilogue = si_llvm_emit_es_epilogue;
		} else {
			bld_base->emit_epilogue = si_llvm_emit_vs_epilogue;
		}
		break;
	case TGSI_PROCESSOR_GEOMETRY:
		bld_base->emit_fetch_funcs[TGSI_FILE_INPUT] = fetch_input_gs;
		bld_base->emit_epilogue = si_llvm_emit_gs_epilogue;
		break;
	case TGSI_PROCESSOR_FRAGMENT:
		si_shader_ctx.radeon_bld.load_input = declare_input_fs;
		bld_base->emit_epilogue = si_llvm_emit_fs_epilogue;

		switch (sel->info.properties[TGSI_PROPERTY_FS_DEPTH_LAYOUT]) {
		case TGSI_FS_DEPTH_LAYOUT_GREATER:
			shader->db_shader_control |=
				S_02880C_CONSERVATIVE_Z_EXPORT(V_02880C_EXPORT_GREATER_THAN_Z);
			break;
		case TGSI_FS_DEPTH_LAYOUT_LESS:
			shader->db_shader_control |=
				S_02880C_CONSERVATIVE_Z_EXPORT(V_02880C_EXPORT_LESS_THAN_Z);
			break;
		}
		break;
	default:
		assert(!"Unsupported shader type");
		return -1;
	}

	create_meta_data(&si_shader_ctx);
	create_function(&si_shader_ctx);
	preload_constants(&si_shader_ctx);
	preload_samplers(&si_shader_ctx);
	preload_streamout_buffers(&si_shader_ctx);
	preload_ring_buffers(&si_shader_ctx);

	if (si_shader_ctx.type == TGSI_PROCESSOR_GEOMETRY) {
		si_shader_ctx.gs_next_vertex =
			lp_build_alloca(bld_base->base.gallivm,
					bld_base->uint_bld.elem_type, "");
	}

	if (!lp_build_tgsi_llvm(bld_base, tokens)) {
		fprintf(stderr, "Failed to translate shader from TGSI to LLVM\n");
		goto out;
	}

	radeon_llvm_finalize_module(&si_shader_ctx.radeon_bld);

	mod = bld_base->base.gallivm->module;
	r = si_compile_llvm(sscreen, shader, tm, mod);
	if (r) {
		fprintf(stderr, "LLVM failed to compile shader\n");
		goto out;
	}

	radeon_llvm_dispose(&si_shader_ctx.radeon_bld);

	if (si_shader_ctx.type == TGSI_PROCESSOR_GEOMETRY) {
		shader->gs_copy_shader = CALLOC_STRUCT(si_shader);
		shader->gs_copy_shader->selector = shader->selector;
		shader->gs_copy_shader->key = shader->key;
		si_shader_ctx.shader = shader->gs_copy_shader;
		if ((r = si_generate_gs_copy_shader(sscreen, &si_shader_ctx,
						    shader, dump))) {
			free(shader->gs_copy_shader);
			shader->gs_copy_shader = NULL;
			goto out;
		}
	}

out:
	for (int i = 0; i < SI_NUM_CONST_BUFFERS; i++)
		FREE(si_shader_ctx.constants[i]);
	if (poly_stipple)
		tgsi_free_tokens(tokens);
	return r;
}

void si_shader_destroy(struct pipe_context *ctx, struct si_shader *shader)
{
	if (shader->gs_copy_shader)
		si_shader_destroy(ctx, shader->gs_copy_shader);

	if (shader->scratch_bo)
		r600_resource_reference(&shader->scratch_bo, NULL);

	r600_resource_reference(&shader->bo, NULL);

	FREE(shader->binary.code);
	FREE(shader->binary.relocs);
}
