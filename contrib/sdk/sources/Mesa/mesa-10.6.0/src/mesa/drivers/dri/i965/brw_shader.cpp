/*
 * Copyright © 2010 Intel Corporation
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

#include "main/macros.h"
#include "brw_context.h"
#include "brw_vs.h"
#include "brw_gs.h"
#include "brw_fs.h"
#include "brw_cfg.h"
#include "brw_nir.h"
#include "glsl/ir_optimization.h"
#include "glsl/glsl_parser_extras.h"
#include "main/shaderapi.h"

struct brw_compiler *
brw_compiler_create(void *mem_ctx, const struct brw_device_info *devinfo)
{
   struct brw_compiler *compiler = rzalloc(mem_ctx, struct brw_compiler);

   compiler->devinfo = devinfo;

   brw_fs_alloc_reg_sets(compiler);
   brw_vec4_alloc_reg_set(compiler);

   return compiler;
}

struct gl_shader *
brw_new_shader(struct gl_context *ctx, GLuint name, GLuint type)
{
   struct brw_shader *shader;

   shader = rzalloc(NULL, struct brw_shader);
   if (shader) {
      shader->base.Type = type;
      shader->base.Stage = _mesa_shader_enum_to_shader_stage(type);
      shader->base.Name = name;
      _mesa_init_shader(ctx, &shader->base);
   }

   return &shader->base;
}

/**
 * Performs a compile of the shader stages even when we don't know
 * what non-orthogonal state will be set, in the hope that it reflects
 * the eventual NOS used, and thus allows us to produce link failures.
 */
static bool
brw_shader_precompile(struct gl_context *ctx,
                      struct gl_shader_program *sh_prog)
{
   struct gl_shader *vs = sh_prog->_LinkedShaders[MESA_SHADER_VERTEX];
   struct gl_shader *gs = sh_prog->_LinkedShaders[MESA_SHADER_GEOMETRY];
   struct gl_shader *fs = sh_prog->_LinkedShaders[MESA_SHADER_FRAGMENT];
   struct gl_shader *cs = sh_prog->_LinkedShaders[MESA_SHADER_COMPUTE];

   if (fs && !brw_fs_precompile(ctx, sh_prog, fs->Program))
      return false;

   if (gs && !brw_gs_precompile(ctx, sh_prog, gs->Program))
      return false;

   if (vs && !brw_vs_precompile(ctx, sh_prog, vs->Program))
      return false;

   if (cs && !brw_cs_precompile(ctx, sh_prog, cs->Program))
      return false;

   return true;
}

static inline bool
is_scalar_shader_stage(struct brw_context *brw, int stage)
{
   switch (stage) {
   case MESA_SHADER_FRAGMENT:
      return true;
   case MESA_SHADER_VERTEX:
      return brw->scalar_vs;
   default:
      return false;
   }
}

static void
brw_lower_packing_builtins(struct brw_context *brw,
                           gl_shader_stage shader_type,
                           exec_list *ir)
{
   int ops = LOWER_PACK_SNORM_2x16
           | LOWER_UNPACK_SNORM_2x16
           | LOWER_PACK_UNORM_2x16
           | LOWER_UNPACK_UNORM_2x16;

   if (is_scalar_shader_stage(brw, shader_type)) {
      ops |= LOWER_UNPACK_UNORM_4x8
           | LOWER_UNPACK_SNORM_4x8
           | LOWER_PACK_UNORM_4x8
           | LOWER_PACK_SNORM_4x8;
   }

   if (brw->gen >= 7) {
      /* Gen7 introduced the f32to16 and f16to32 instructions, which can be
       * used to execute packHalf2x16 and unpackHalf2x16. For AOS code, no
       * lowering is needed. For SOA code, the Half2x16 ops must be
       * scalarized.
       */
      if (is_scalar_shader_stage(brw, shader_type)) {
         ops |= LOWER_PACK_HALF_2x16_TO_SPLIT
             |  LOWER_UNPACK_HALF_2x16_TO_SPLIT;
      }
   } else {
      ops |= LOWER_PACK_HALF_2x16
          |  LOWER_UNPACK_HALF_2x16;
   }

   lower_packing_builtins(ir, ops);
}

static void
process_glsl_ir(struct brw_context *brw,
                struct gl_shader_program *shader_prog,
                struct gl_shader *shader)
{
   struct gl_context *ctx = &brw->ctx;
   const struct gl_shader_compiler_options *options =
      &ctx->Const.ShaderCompilerOptions[shader->Stage];

   /* Temporary memory context for any new IR. */
   void *mem_ctx = ralloc_context(NULL);

   ralloc_adopt(mem_ctx, shader->ir);

   /* lower_packing_builtins() inserts arithmetic instructions, so it
    * must precede lower_instructions().
    */
   brw_lower_packing_builtins(brw, shader->Stage, shader->ir);
   do_mat_op_to_vec(shader->ir);
   const int bitfield_insert = brw->gen >= 7 ? BITFIELD_INSERT_TO_BFM_BFI : 0;
   lower_instructions(shader->ir,
                      MOD_TO_FLOOR |
                      DIV_TO_MUL_RCP |
                      SUB_TO_ADD_NEG |
                      EXP_TO_EXP2 |
                      LOG_TO_LOG2 |
                      bitfield_insert |
                      LDEXP_TO_ARITH);

   /* Pre-gen6 HW can only nest if-statements 16 deep.  Beyond this,
    * if-statements need to be flattened.
    */
   if (brw->gen < 6)
      lower_if_to_cond_assign(shader->ir, 16);

   do_lower_texture_projection(shader->ir);
   brw_lower_texture_gradients(brw, shader->ir);
   do_vec_index_to_cond_assign(shader->ir);
   lower_vector_insert(shader->ir, true);
   if (options->NirOptions == NULL)
      brw_do_cubemap_normalize(shader->ir);
   lower_offset_arrays(shader->ir);
   brw_do_lower_unnormalized_offset(shader->ir);
   lower_noise(shader->ir);
   lower_quadop_vector(shader->ir, false);

   bool lowered_variable_indexing =
      lower_variable_index_to_cond_assign(shader->ir,
                                          options->EmitNoIndirectInput,
                                          options->EmitNoIndirectOutput,
                                          options->EmitNoIndirectTemp,
                                          options->EmitNoIndirectUniform);

   if (unlikely(brw->perf_debug && lowered_variable_indexing)) {
      perf_debug("Unsupported form of variable indexing in FS; falling "
                 "back to very inefficient code generation\n");
   }

   lower_ubo_reference(shader, shader->ir);

   bool progress;
   do {
      progress = false;

      if (is_scalar_shader_stage(brw, shader->Stage)) {
         brw_do_channel_expressions(shader->ir);
         brw_do_vector_splitting(shader->ir);
      }

      progress = do_lower_jumps(shader->ir, true, true,
                                true, /* main return */
                                false, /* continue */
                                false /* loops */
                                ) || progress;

      progress = do_common_optimization(shader->ir, true, true,
                                        options, ctx->Const.NativeIntegers) || progress;
   } while (progress);

   if (options->NirOptions != NULL)
      lower_output_reads(shader->ir);

   validate_ir_tree(shader->ir);

   /* Now that we've finished altering the linked IR, reparent any live IR back
    * to the permanent memory context, and free the temporary one (discarding any
    * junk we optimized away).
    */
   reparent_ir(shader->ir, shader->ir);
   ralloc_free(mem_ctx);

   if (ctx->_Shader->Flags & GLSL_DUMP) {
      fprintf(stderr, "\n");
      fprintf(stderr, "GLSL IR for linked %s program %d:\n",
              _mesa_shader_stage_to_string(shader->Stage),
              shader_prog->Name);
      _mesa_print_ir(stderr, shader->ir, NULL);
      fprintf(stderr, "\n");
   }
}

GLboolean
brw_link_shader(struct gl_context *ctx, struct gl_shader_program *shProg)
{
   struct brw_context *brw = brw_context(ctx);
   unsigned int stage;

   for (stage = 0; stage < ARRAY_SIZE(shProg->_LinkedShaders); stage++) {
      struct gl_shader *shader = shProg->_LinkedShaders[stage];
      const struct gl_shader_compiler_options *options =
         &ctx->Const.ShaderCompilerOptions[stage];

      if (!shader)
	 continue;

      struct gl_program *prog =
	 ctx->Driver.NewProgram(ctx, _mesa_shader_stage_to_program(stage),
                                shader->Name);
      if (!prog)
	return false;
      prog->Parameters = _mesa_new_parameter_list();

      _mesa_copy_linked_program_data((gl_shader_stage) stage, shProg, prog);

      process_glsl_ir(brw, shProg, shader);

      /* Make a pass over the IR to add state references for any built-in
       * uniforms that are used.  This has to be done now (during linking).
       * Code generation doesn't happen until the first time this shader is
       * used for rendering.  Waiting until then to generate the parameters is
       * too late.  At that point, the values for the built-in uniforms won't
       * get sent to the shader.
       */
      foreach_in_list(ir_instruction, node, shader->ir) {
	 ir_variable *var = node->as_variable();

	 if ((var == NULL) || (var->data.mode != ir_var_uniform)
	     || (strncmp(var->name, "gl_", 3) != 0))
	    continue;

	 const ir_state_slot *const slots = var->get_state_slots();
	 assert(slots != NULL);

	 for (unsigned int i = 0; i < var->get_num_state_slots(); i++) {
	    _mesa_add_state_reference(prog->Parameters,
				      (gl_state_index *) slots[i].tokens);
	 }
      }

      do_set_program_inouts(shader->ir, prog, shader->Stage);

      prog->SamplersUsed = shader->active_samplers;
      prog->ShadowSamplers = shader->shadow_samplers;
      _mesa_update_shader_textures_used(shProg, prog);

      _mesa_reference_program(ctx, &shader->Program, prog);

      brw_add_texrect_params(prog);

      if (options->NirOptions)
         prog->nir = brw_create_nir(brw, shProg, prog, (gl_shader_stage) stage);

      _mesa_reference_program(ctx, &prog, NULL);
   }

   if ((ctx->_Shader->Flags & GLSL_DUMP) && shProg->Name != 0) {
      for (unsigned i = 0; i < shProg->NumShaders; i++) {
         const struct gl_shader *sh = shProg->Shaders[i];
         if (!sh)
            continue;

         fprintf(stderr, "GLSL %s shader %d source for linked program %d:\n",
                 _mesa_shader_stage_to_string(sh->Stage),
                 i, shProg->Name);
         fprintf(stderr, "%s", sh->Source);
         fprintf(stderr, "\n");
      }
   }

   if (brw->precompile && !brw_shader_precompile(ctx, shProg))
      return false;

   return true;
}


enum brw_reg_type
brw_type_for_base_type(const struct glsl_type *type)
{
   switch (type->base_type) {
   case GLSL_TYPE_FLOAT:
      return BRW_REGISTER_TYPE_F;
   case GLSL_TYPE_INT:
   case GLSL_TYPE_BOOL:
      return BRW_REGISTER_TYPE_D;
   case GLSL_TYPE_UINT:
      return BRW_REGISTER_TYPE_UD;
   case GLSL_TYPE_ARRAY:
      return brw_type_for_base_type(type->fields.array);
   case GLSL_TYPE_STRUCT:
   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_ATOMIC_UINT:
      /* These should be overridden with the type of the member when
       * dereferenced into.  BRW_REGISTER_TYPE_UD seems like a likely
       * way to trip up if we don't.
       */
      return BRW_REGISTER_TYPE_UD;
   case GLSL_TYPE_IMAGE:
      return BRW_REGISTER_TYPE_UD;
   case GLSL_TYPE_VOID:
   case GLSL_TYPE_ERROR:
   case GLSL_TYPE_INTERFACE:
   case GLSL_TYPE_DOUBLE:
      unreachable("not reached");
   }

   return BRW_REGISTER_TYPE_F;
}

enum brw_conditional_mod
brw_conditional_for_comparison(unsigned int op)
{
   switch (op) {
   case ir_binop_less:
      return BRW_CONDITIONAL_L;
   case ir_binop_greater:
      return BRW_CONDITIONAL_G;
   case ir_binop_lequal:
      return BRW_CONDITIONAL_LE;
   case ir_binop_gequal:
      return BRW_CONDITIONAL_GE;
   case ir_binop_equal:
   case ir_binop_all_equal: /* same as equal for scalars */
      return BRW_CONDITIONAL_Z;
   case ir_binop_nequal:
   case ir_binop_any_nequal: /* same as nequal for scalars */
      return BRW_CONDITIONAL_NZ;
   default:
      unreachable("not reached: bad operation for comparison");
   }
}

uint32_t
brw_math_function(enum opcode op)
{
   switch (op) {
   case SHADER_OPCODE_RCP:
      return BRW_MATH_FUNCTION_INV;
   case SHADER_OPCODE_RSQ:
      return BRW_MATH_FUNCTION_RSQ;
   case SHADER_OPCODE_SQRT:
      return BRW_MATH_FUNCTION_SQRT;
   case SHADER_OPCODE_EXP2:
      return BRW_MATH_FUNCTION_EXP;
   case SHADER_OPCODE_LOG2:
      return BRW_MATH_FUNCTION_LOG;
   case SHADER_OPCODE_POW:
      return BRW_MATH_FUNCTION_POW;
   case SHADER_OPCODE_SIN:
      return BRW_MATH_FUNCTION_SIN;
   case SHADER_OPCODE_COS:
      return BRW_MATH_FUNCTION_COS;
   case SHADER_OPCODE_INT_QUOTIENT:
      return BRW_MATH_FUNCTION_INT_DIV_QUOTIENT;
   case SHADER_OPCODE_INT_REMAINDER:
      return BRW_MATH_FUNCTION_INT_DIV_REMAINDER;
   default:
      unreachable("not reached: unknown math function");
   }
}

uint32_t
brw_texture_offset(int *offsets, unsigned num_components)
{
   if (!offsets) return 0;  /* nonconstant offset; caller will handle it. */

   /* Combine all three offsets into a single unsigned dword:
    *
    *    bits 11:8 - U Offset (X component)
    *    bits  7:4 - V Offset (Y component)
    *    bits  3:0 - R Offset (Z component)
    */
   unsigned offset_bits = 0;
   for (unsigned i = 0; i < num_components; i++) {
      const unsigned shift = 4 * (2 - i);
      offset_bits |= (offsets[i] << shift) & (0xF << shift);
   }
   return offset_bits;
}

const char *
brw_instruction_name(enum opcode op)
{
   switch (op) {
   case BRW_OPCODE_MOV ... BRW_OPCODE_NOP:
      assert(opcode_descs[op].name);
      return opcode_descs[op].name;
   case FS_OPCODE_FB_WRITE:
      return "fb_write";
   case FS_OPCODE_BLORP_FB_WRITE:
      return "blorp_fb_write";
   case FS_OPCODE_REP_FB_WRITE:
      return "rep_fb_write";

   case SHADER_OPCODE_RCP:
      return "rcp";
   case SHADER_OPCODE_RSQ:
      return "rsq";
   case SHADER_OPCODE_SQRT:
      return "sqrt";
   case SHADER_OPCODE_EXP2:
      return "exp2";
   case SHADER_OPCODE_LOG2:
      return "log2";
   case SHADER_OPCODE_POW:
      return "pow";
   case SHADER_OPCODE_INT_QUOTIENT:
      return "int_quot";
   case SHADER_OPCODE_INT_REMAINDER:
      return "int_rem";
   case SHADER_OPCODE_SIN:
      return "sin";
   case SHADER_OPCODE_COS:
      return "cos";

   case SHADER_OPCODE_TEX:
      return "tex";
   case SHADER_OPCODE_TXD:
      return "txd";
   case SHADER_OPCODE_TXF:
      return "txf";
   case SHADER_OPCODE_TXL:
      return "txl";
   case SHADER_OPCODE_TXS:
      return "txs";
   case FS_OPCODE_TXB:
      return "txb";
   case SHADER_OPCODE_TXF_CMS:
      return "txf_cms";
   case SHADER_OPCODE_TXF_UMS:
      return "txf_ums";
   case SHADER_OPCODE_TXF_MCS:
      return "txf_mcs";
   case SHADER_OPCODE_LOD:
      return "lod";
   case SHADER_OPCODE_TG4:
      return "tg4";
   case SHADER_OPCODE_TG4_OFFSET:
      return "tg4_offset";
   case SHADER_OPCODE_SHADER_TIME_ADD:
      return "shader_time_add";

   case SHADER_OPCODE_UNTYPED_ATOMIC:
      return "untyped_atomic";
   case SHADER_OPCODE_UNTYPED_SURFACE_READ:
      return "untyped_surface_read";
   case SHADER_OPCODE_UNTYPED_SURFACE_WRITE:
      return "untyped_surface_write";
   case SHADER_OPCODE_TYPED_ATOMIC:
      return "typed_atomic";
   case SHADER_OPCODE_TYPED_SURFACE_READ:
      return "typed_surface_read";
   case SHADER_OPCODE_TYPED_SURFACE_WRITE:
      return "typed_surface_write";
   case SHADER_OPCODE_MEMORY_FENCE:
      return "memory_fence";

   case SHADER_OPCODE_LOAD_PAYLOAD:
      return "load_payload";

   case SHADER_OPCODE_GEN4_SCRATCH_READ:
      return "gen4_scratch_read";
   case SHADER_OPCODE_GEN4_SCRATCH_WRITE:
      return "gen4_scratch_write";
   case SHADER_OPCODE_GEN7_SCRATCH_READ:
      return "gen7_scratch_read";
   case SHADER_OPCODE_URB_WRITE_SIMD8:
      return "gen8_urb_write_simd8";

   case SHADER_OPCODE_FIND_LIVE_CHANNEL:
      return "find_live_channel";
   case SHADER_OPCODE_BROADCAST:
      return "broadcast";

   case VEC4_OPCODE_MOV_BYTES:
      return "mov_bytes";
   case VEC4_OPCODE_PACK_BYTES:
      return "pack_bytes";
   case VEC4_OPCODE_UNPACK_UNIFORM:
      return "unpack_uniform";

   case FS_OPCODE_DDX_COARSE:
      return "ddx_coarse";
   case FS_OPCODE_DDX_FINE:
      return "ddx_fine";
   case FS_OPCODE_DDY_COARSE:
      return "ddy_coarse";
   case FS_OPCODE_DDY_FINE:
      return "ddy_fine";

   case FS_OPCODE_CINTERP:
      return "cinterp";
   case FS_OPCODE_LINTERP:
      return "linterp";

   case FS_OPCODE_PIXEL_X:
      return "pixel_x";
   case FS_OPCODE_PIXEL_Y:
      return "pixel_y";

   case FS_OPCODE_UNIFORM_PULL_CONSTANT_LOAD:
      return "uniform_pull_const";
   case FS_OPCODE_UNIFORM_PULL_CONSTANT_LOAD_GEN7:
      return "uniform_pull_const_gen7";
   case FS_OPCODE_VARYING_PULL_CONSTANT_LOAD:
      return "varying_pull_const";
   case FS_OPCODE_VARYING_PULL_CONSTANT_LOAD_GEN7:
      return "varying_pull_const_gen7";

   case FS_OPCODE_MOV_DISPATCH_TO_FLAGS:
      return "mov_dispatch_to_flags";
   case FS_OPCODE_DISCARD_JUMP:
      return "discard_jump";

   case FS_OPCODE_SET_OMASK:
      return "set_omask";
   case FS_OPCODE_SET_SAMPLE_ID:
      return "set_sample_id";
   case FS_OPCODE_SET_SIMD4X2_OFFSET:
      return "set_simd4x2_offset";

   case FS_OPCODE_PACK_HALF_2x16_SPLIT:
      return "pack_half_2x16_split";
   case FS_OPCODE_UNPACK_HALF_2x16_SPLIT_X:
      return "unpack_half_2x16_split_x";
   case FS_OPCODE_UNPACK_HALF_2x16_SPLIT_Y:
      return "unpack_half_2x16_split_y";

   case FS_OPCODE_PLACEHOLDER_HALT:
      return "placeholder_halt";

   case FS_OPCODE_INTERPOLATE_AT_CENTROID:
      return "interp_centroid";
   case FS_OPCODE_INTERPOLATE_AT_SAMPLE:
      return "interp_sample";
   case FS_OPCODE_INTERPOLATE_AT_SHARED_OFFSET:
      return "interp_shared_offset";
   case FS_OPCODE_INTERPOLATE_AT_PER_SLOT_OFFSET:
      return "interp_per_slot_offset";

   case VS_OPCODE_URB_WRITE:
      return "vs_urb_write";
   case VS_OPCODE_PULL_CONSTANT_LOAD:
      return "pull_constant_load";
   case VS_OPCODE_PULL_CONSTANT_LOAD_GEN7:
      return "pull_constant_load_gen7";

   case VS_OPCODE_SET_SIMD4X2_HEADER_GEN9:
      return "set_simd4x2_header_gen9";

   case VS_OPCODE_UNPACK_FLAGS_SIMD4X2:
      return "unpack_flags_simd4x2";

   case GS_OPCODE_URB_WRITE:
      return "gs_urb_write";
   case GS_OPCODE_URB_WRITE_ALLOCATE:
      return "gs_urb_write_allocate";
   case GS_OPCODE_THREAD_END:
      return "gs_thread_end";
   case GS_OPCODE_SET_WRITE_OFFSET:
      return "set_write_offset";
   case GS_OPCODE_SET_VERTEX_COUNT:
      return "set_vertex_count";
   case GS_OPCODE_SET_DWORD_2:
      return "set_dword_2";
   case GS_OPCODE_PREPARE_CHANNEL_MASKS:
      return "prepare_channel_masks";
   case GS_OPCODE_SET_CHANNEL_MASKS:
      return "set_channel_masks";
   case GS_OPCODE_GET_INSTANCE_ID:
      return "get_instance_id";
   case GS_OPCODE_FF_SYNC:
      return "ff_sync";
   case GS_OPCODE_SET_PRIMITIVE_ID:
      return "set_primitive_id";
   case GS_OPCODE_SVB_WRITE:
      return "gs_svb_write";
   case GS_OPCODE_SVB_SET_DST_INDEX:
      return "gs_svb_set_dst_index";
   case GS_OPCODE_FF_SYNC_SET_PRIMITIVES:
      return "gs_ff_sync_set_primitives";
   case CS_OPCODE_CS_TERMINATE:
      return "cs_terminate";
   }

   unreachable("not reached");
}

bool
brw_saturate_immediate(enum brw_reg_type type, struct brw_reg *reg)
{
   union {
      unsigned ud;
      int d;
      float f;
   } imm = { reg->dw1.ud }, sat_imm = { 0 };

   switch (type) {
   case BRW_REGISTER_TYPE_UD:
   case BRW_REGISTER_TYPE_D:
   case BRW_REGISTER_TYPE_UQ:
   case BRW_REGISTER_TYPE_Q:
      /* Nothing to do. */
      return false;
   case BRW_REGISTER_TYPE_UW:
      sat_imm.ud = CLAMP(imm.ud, 0, USHRT_MAX);
      break;
   case BRW_REGISTER_TYPE_W:
      sat_imm.d = CLAMP(imm.d, SHRT_MIN, SHRT_MAX);
      break;
   case BRW_REGISTER_TYPE_F:
      sat_imm.f = CLAMP(imm.f, 0.0f, 1.0f);
      break;
   case BRW_REGISTER_TYPE_UB:
   case BRW_REGISTER_TYPE_B:
      unreachable("no UB/B immediates");
   case BRW_REGISTER_TYPE_V:
   case BRW_REGISTER_TYPE_UV:
   case BRW_REGISTER_TYPE_VF:
      unreachable("unimplemented: saturate vector immediate");
   case BRW_REGISTER_TYPE_DF:
   case BRW_REGISTER_TYPE_HF:
      unreachable("unimplemented: saturate DF/HF immediate");
   }

   if (imm.ud != sat_imm.ud) {
      reg->dw1.ud = sat_imm.ud;
      return true;
   }
   return false;
}

bool
brw_negate_immediate(enum brw_reg_type type, struct brw_reg *reg)
{
   switch (type) {
   case BRW_REGISTER_TYPE_D:
   case BRW_REGISTER_TYPE_UD:
      reg->dw1.d = -reg->dw1.d;
      return true;
   case BRW_REGISTER_TYPE_W:
   case BRW_REGISTER_TYPE_UW:
      reg->dw1.d = -(int16_t)reg->dw1.ud;
      return true;
   case BRW_REGISTER_TYPE_F:
      reg->dw1.f = -reg->dw1.f;
      return true;
   case BRW_REGISTER_TYPE_VF:
      reg->dw1.ud ^= 0x80808080;
      return true;
   case BRW_REGISTER_TYPE_UB:
   case BRW_REGISTER_TYPE_B:
      unreachable("no UB/B immediates");
   case BRW_REGISTER_TYPE_UV:
   case BRW_REGISTER_TYPE_V:
      assert(!"unimplemented: negate UV/V immediate");
   case BRW_REGISTER_TYPE_UQ:
   case BRW_REGISTER_TYPE_Q:
      assert(!"unimplemented: negate UQ/Q immediate");
   case BRW_REGISTER_TYPE_DF:
   case BRW_REGISTER_TYPE_HF:
      assert(!"unimplemented: negate DF/HF immediate");
   }

   return false;
}

bool
brw_abs_immediate(enum brw_reg_type type, struct brw_reg *reg)
{
   switch (type) {
   case BRW_REGISTER_TYPE_D:
      reg->dw1.d = abs(reg->dw1.d);
      return true;
   case BRW_REGISTER_TYPE_W:
      reg->dw1.d = abs((int16_t)reg->dw1.ud);
      return true;
   case BRW_REGISTER_TYPE_F:
      reg->dw1.f = fabsf(reg->dw1.f);
      return true;
   case BRW_REGISTER_TYPE_VF:
      reg->dw1.ud &= ~0x80808080;
      return true;
   case BRW_REGISTER_TYPE_UB:
   case BRW_REGISTER_TYPE_B:
      unreachable("no UB/B immediates");
   case BRW_REGISTER_TYPE_UQ:
   case BRW_REGISTER_TYPE_UD:
   case BRW_REGISTER_TYPE_UW:
   case BRW_REGISTER_TYPE_UV:
      /* Presumably the absolute value modifier on an unsigned source is a
       * nop, but it would be nice to confirm.
       */
      assert(!"unimplemented: abs unsigned immediate");
   case BRW_REGISTER_TYPE_V:
      assert(!"unimplemented: abs V immediate");
   case BRW_REGISTER_TYPE_Q:
      assert(!"unimplemented: abs Q immediate");
   case BRW_REGISTER_TYPE_DF:
   case BRW_REGISTER_TYPE_HF:
      assert(!"unimplemented: abs DF/HF immediate");
   }

   return false;
}

backend_visitor::backend_visitor(struct brw_context *brw,
                                 struct gl_shader_program *shader_prog,
                                 struct gl_program *prog,
                                 struct brw_stage_prog_data *stage_prog_data,
                                 gl_shader_stage stage)
   : brw(brw),
     devinfo(brw->intelScreen->devinfo),
     ctx(&brw->ctx),
     shader(shader_prog ?
        (struct brw_shader *)shader_prog->_LinkedShaders[stage] : NULL),
     shader_prog(shader_prog),
     prog(prog),
     stage_prog_data(stage_prog_data),
     cfg(NULL),
     stage(stage)
{
   debug_enabled = INTEL_DEBUG & intel_debug_flag_for_shader_stage(stage);
   stage_name = _mesa_shader_stage_to_string(stage);
   stage_abbrev = _mesa_shader_stage_to_abbrev(stage);
}

bool
backend_reg::is_zero() const
{
   if (file != IMM)
      return false;

   return fixed_hw_reg.dw1.d == 0;
}

bool
backend_reg::is_one() const
{
   if (file != IMM)
      return false;

   return type == BRW_REGISTER_TYPE_F
          ? fixed_hw_reg.dw1.f == 1.0
          : fixed_hw_reg.dw1.d == 1;
}

bool
backend_reg::is_negative_one() const
{
   if (file != IMM)
      return false;

   switch (type) {
   case BRW_REGISTER_TYPE_F:
      return fixed_hw_reg.dw1.f == -1.0;
   case BRW_REGISTER_TYPE_D:
      return fixed_hw_reg.dw1.d == -1;
   default:
      return false;
   }
}

bool
backend_reg::is_null() const
{
   return file == HW_REG &&
          fixed_hw_reg.file == BRW_ARCHITECTURE_REGISTER_FILE &&
          fixed_hw_reg.nr == BRW_ARF_NULL;
}


bool
backend_reg::is_accumulator() const
{
   return file == HW_REG &&
          fixed_hw_reg.file == BRW_ARCHITECTURE_REGISTER_FILE &&
          fixed_hw_reg.nr == BRW_ARF_ACCUMULATOR;
}

bool
backend_reg::in_range(const backend_reg &r, unsigned n) const
{
   return (file == r.file &&
           reg == r.reg &&
           reg_offset >= r.reg_offset &&
           reg_offset < r.reg_offset + n);
}

bool
backend_instruction::is_commutative() const
{
   switch (opcode) {
   case BRW_OPCODE_AND:
   case BRW_OPCODE_OR:
   case BRW_OPCODE_XOR:
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_MUL:
      return true;
   case BRW_OPCODE_SEL:
      /* MIN and MAX are commutative. */
      if (conditional_mod == BRW_CONDITIONAL_GE ||
          conditional_mod == BRW_CONDITIONAL_L) {
         return true;
      }
      /* fallthrough */
   default:
      return false;
   }
}

bool
backend_instruction::is_3src() const
{
   return opcode < ARRAY_SIZE(opcode_descs) && opcode_descs[opcode].nsrc == 3;
}

bool
backend_instruction::is_tex() const
{
   return (opcode == SHADER_OPCODE_TEX ||
           opcode == FS_OPCODE_TXB ||
           opcode == SHADER_OPCODE_TXD ||
           opcode == SHADER_OPCODE_TXF ||
           opcode == SHADER_OPCODE_TXF_CMS ||
           opcode == SHADER_OPCODE_TXF_UMS ||
           opcode == SHADER_OPCODE_TXF_MCS ||
           opcode == SHADER_OPCODE_TXL ||
           opcode == SHADER_OPCODE_TXS ||
           opcode == SHADER_OPCODE_LOD ||
           opcode == SHADER_OPCODE_TG4 ||
           opcode == SHADER_OPCODE_TG4_OFFSET);
}

bool
backend_instruction::is_math() const
{
   return (opcode == SHADER_OPCODE_RCP ||
           opcode == SHADER_OPCODE_RSQ ||
           opcode == SHADER_OPCODE_SQRT ||
           opcode == SHADER_OPCODE_EXP2 ||
           opcode == SHADER_OPCODE_LOG2 ||
           opcode == SHADER_OPCODE_SIN ||
           opcode == SHADER_OPCODE_COS ||
           opcode == SHADER_OPCODE_INT_QUOTIENT ||
           opcode == SHADER_OPCODE_INT_REMAINDER ||
           opcode == SHADER_OPCODE_POW);
}

bool
backend_instruction::is_control_flow() const
{
   switch (opcode) {
   case BRW_OPCODE_DO:
   case BRW_OPCODE_WHILE:
   case BRW_OPCODE_IF:
   case BRW_OPCODE_ELSE:
   case BRW_OPCODE_ENDIF:
   case BRW_OPCODE_BREAK:
   case BRW_OPCODE_CONTINUE:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::can_do_source_mods() const
{
   switch (opcode) {
   case BRW_OPCODE_ADDC:
   case BRW_OPCODE_BFE:
   case BRW_OPCODE_BFI1:
   case BRW_OPCODE_BFI2:
   case BRW_OPCODE_BFREV:
   case BRW_OPCODE_CBIT:
   case BRW_OPCODE_FBH:
   case BRW_OPCODE_FBL:
   case BRW_OPCODE_SUBB:
      return false;
   default:
      return true;
   }
}

bool
backend_instruction::can_do_saturate() const
{
   switch (opcode) {
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_ASR:
   case BRW_OPCODE_AVG:
   case BRW_OPCODE_DP2:
   case BRW_OPCODE_DP3:
   case BRW_OPCODE_DP4:
   case BRW_OPCODE_DPH:
   case BRW_OPCODE_F16TO32:
   case BRW_OPCODE_F32TO16:
   case BRW_OPCODE_LINE:
   case BRW_OPCODE_LRP:
   case BRW_OPCODE_MAC:
   case BRW_OPCODE_MACH:
   case BRW_OPCODE_MAD:
   case BRW_OPCODE_MATH:
   case BRW_OPCODE_MOV:
   case BRW_OPCODE_MUL:
   case BRW_OPCODE_PLN:
   case BRW_OPCODE_RNDD:
   case BRW_OPCODE_RNDE:
   case BRW_OPCODE_RNDU:
   case BRW_OPCODE_RNDZ:
   case BRW_OPCODE_SEL:
   case BRW_OPCODE_SHL:
   case BRW_OPCODE_SHR:
   case FS_OPCODE_LINTERP:
   case SHADER_OPCODE_COS:
   case SHADER_OPCODE_EXP2:
   case SHADER_OPCODE_LOG2:
   case SHADER_OPCODE_POW:
   case SHADER_OPCODE_RCP:
   case SHADER_OPCODE_RSQ:
   case SHADER_OPCODE_SIN:
   case SHADER_OPCODE_SQRT:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::can_do_cmod() const
{
   switch (opcode) {
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_ADDC:
   case BRW_OPCODE_AND:
   case BRW_OPCODE_ASR:
   case BRW_OPCODE_AVG:
   case BRW_OPCODE_CMP:
   case BRW_OPCODE_CMPN:
   case BRW_OPCODE_DP2:
   case BRW_OPCODE_DP3:
   case BRW_OPCODE_DP4:
   case BRW_OPCODE_DPH:
   case BRW_OPCODE_F16TO32:
   case BRW_OPCODE_F32TO16:
   case BRW_OPCODE_FRC:
   case BRW_OPCODE_LINE:
   case BRW_OPCODE_LRP:
   case BRW_OPCODE_LZD:
   case BRW_OPCODE_MAC:
   case BRW_OPCODE_MACH:
   case BRW_OPCODE_MAD:
   case BRW_OPCODE_MOV:
   case BRW_OPCODE_MUL:
   case BRW_OPCODE_NOT:
   case BRW_OPCODE_OR:
   case BRW_OPCODE_PLN:
   case BRW_OPCODE_RNDD:
   case BRW_OPCODE_RNDE:
   case BRW_OPCODE_RNDU:
   case BRW_OPCODE_RNDZ:
   case BRW_OPCODE_SAD2:
   case BRW_OPCODE_SADA2:
   case BRW_OPCODE_SHL:
   case BRW_OPCODE_SHR:
   case BRW_OPCODE_SUBB:
   case BRW_OPCODE_XOR:
   case FS_OPCODE_CINTERP:
   case FS_OPCODE_LINTERP:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::reads_accumulator_implicitly() const
{
   switch (opcode) {
   case BRW_OPCODE_MAC:
   case BRW_OPCODE_MACH:
   case BRW_OPCODE_SADA2:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::writes_accumulator_implicitly(const struct brw_device_info *devinfo) const
{
   return writes_accumulator ||
          (devinfo->gen < 6 &&
           ((opcode >= BRW_OPCODE_ADD && opcode < BRW_OPCODE_NOP) ||
            (opcode >= FS_OPCODE_DDX_COARSE && opcode <= FS_OPCODE_LINTERP &&
             opcode != FS_OPCODE_CINTERP)));
}

bool
backend_instruction::has_side_effects() const
{
   switch (opcode) {
   case SHADER_OPCODE_UNTYPED_ATOMIC:
   case SHADER_OPCODE_GEN4_SCRATCH_WRITE:
   case SHADER_OPCODE_UNTYPED_SURFACE_WRITE:
   case SHADER_OPCODE_TYPED_ATOMIC:
   case SHADER_OPCODE_TYPED_SURFACE_WRITE:
   case SHADER_OPCODE_MEMORY_FENCE:
   case SHADER_OPCODE_URB_WRITE_SIMD8:
   case FS_OPCODE_FB_WRITE:
      return true;
   default:
      return false;
   }
}

#ifndef NDEBUG
static bool
inst_is_in_block(const bblock_t *block, const backend_instruction *inst)
{
   bool found = false;
   foreach_inst_in_block (backend_instruction, i, block) {
      if (inst == i) {
         found = true;
      }
   }
   return found;
}
#endif

static void
adjust_later_block_ips(bblock_t *start_block, int ip_adjustment)
{
   for (bblock_t *block_iter = start_block->next();
        !block_iter->link.is_tail_sentinel();
        block_iter = block_iter->next()) {
      block_iter->start_ip += ip_adjustment;
      block_iter->end_ip += ip_adjustment;
   }
}

void
backend_instruction::insert_after(bblock_t *block, backend_instruction *inst)
{
   if (!this->is_head_sentinel())
      assert(inst_is_in_block(block, this) || !"Instruction not in block");

   block->end_ip++;

   adjust_later_block_ips(block, 1);

   exec_node::insert_after(inst);
}

void
backend_instruction::insert_before(bblock_t *block, backend_instruction *inst)
{
   if (!this->is_tail_sentinel())
      assert(inst_is_in_block(block, this) || !"Instruction not in block");

   block->end_ip++;

   adjust_later_block_ips(block, 1);

   exec_node::insert_before(inst);
}

void
backend_instruction::insert_before(bblock_t *block, exec_list *list)
{
   assert(inst_is_in_block(block, this) || !"Instruction not in block");

   unsigned num_inst = list->length();

   block->end_ip += num_inst;

   adjust_later_block_ips(block, num_inst);

   exec_node::insert_before(list);
}

void
backend_instruction::remove(bblock_t *block)
{
   assert(inst_is_in_block(block, this) || !"Instruction not in block");

   adjust_later_block_ips(block, -1);

   if (block->start_ip == block->end_ip) {
      block->cfg->remove_block(block);
   } else {
      block->end_ip--;
   }

   exec_node::remove();
}

void
backend_visitor::dump_instructions()
{
   dump_instructions(NULL);
}

void
backend_visitor::dump_instructions(const char *name)
{
   FILE *file = stderr;
   if (name && geteuid() != 0) {
      file = fopen(name, "w");
      if (!file)
         file = stderr;
   }

   if (cfg) {
      int ip = 0;
      foreach_block_and_inst(block, backend_instruction, inst, cfg) {
         fprintf(file, "%4d: ", ip++);
         dump_instruction(inst, file);
      }
   } else {
      int ip = 0;
      foreach_in_list(backend_instruction, inst, &instructions) {
         fprintf(file, "%4d: ", ip++);
         dump_instruction(inst, file);
      }
   }

   if (file != stderr) {
      fclose(file);
   }
}

void
backend_visitor::calculate_cfg()
{
   if (this->cfg)
      return;
   cfg = new(mem_ctx) cfg_t(&this->instructions);
}

void
backend_visitor::invalidate_cfg()
{
   ralloc_free(this->cfg);
   this->cfg = NULL;
}

/**
 * Sets up the starting offsets for the groups of binding table entries
 * commong to all pipeline stages.
 *
 * Unused groups are initialized to 0xd0d0d0d0 to make it obvious that they're
 * unused but also make sure that addition of small offsets to them will
 * trigger some of our asserts that surface indices are < BRW_MAX_SURFACES.
 */
void
backend_visitor::assign_common_binding_table_offsets(uint32_t next_binding_table_offset)
{
   int num_textures = _mesa_fls(prog->SamplersUsed);

   stage_prog_data->binding_table.texture_start = next_binding_table_offset;
   next_binding_table_offset += num_textures;

   if (shader) {
      stage_prog_data->binding_table.ubo_start = next_binding_table_offset;
      next_binding_table_offset += shader->base.NumUniformBlocks;
   } else {
      stage_prog_data->binding_table.ubo_start = 0xd0d0d0d0;
   }

   if (INTEL_DEBUG & DEBUG_SHADER_TIME) {
      stage_prog_data->binding_table.shader_time_start = next_binding_table_offset;
      next_binding_table_offset++;
   } else {
      stage_prog_data->binding_table.shader_time_start = 0xd0d0d0d0;
   }

   if (prog->UsesGather) {
      if (devinfo->gen >= 8) {
         stage_prog_data->binding_table.gather_texture_start =
            stage_prog_data->binding_table.texture_start;
      } else {
         stage_prog_data->binding_table.gather_texture_start = next_binding_table_offset;
         next_binding_table_offset += num_textures;
      }
   } else {
      stage_prog_data->binding_table.gather_texture_start = 0xd0d0d0d0;
   }

   if (shader_prog && shader_prog->NumAtomicBuffers) {
      stage_prog_data->binding_table.abo_start = next_binding_table_offset;
      next_binding_table_offset += shader_prog->NumAtomicBuffers;
   } else {
      stage_prog_data->binding_table.abo_start = 0xd0d0d0d0;
   }

   if (shader && shader->base.NumImages) {
      stage_prog_data->binding_table.image_start = next_binding_table_offset;
      next_binding_table_offset += shader->base.NumImages;
   } else {
      stage_prog_data->binding_table.image_start = 0xd0d0d0d0;
   }

   /* This may or may not be used depending on how the compile goes. */
   stage_prog_data->binding_table.pull_constants_start = next_binding_table_offset;
   next_binding_table_offset++;

   assert(next_binding_table_offset <= BRW_MAX_SURFACES);

   /* prog_data->base.binding_table.size will be set by brw_mark_surface_used. */
}
