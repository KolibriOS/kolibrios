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

/** @file brw_fs_visitor.cpp
 *
 * This file supports generating the FS LIR from the GLSL IR.  The LIR
 * makes it easier to do backend-specific optimizations than doing so
 * in the GLSL IR or in the native code.
 */
#include <sys/types.h>

#include "main/macros.h"
#include "main/shaderobj.h"
#include "program/prog_parameter.h"
#include "program/prog_print.h"
#include "program/prog_optimize.h"
#include "util/register_allocate.h"
#include "program/hash_table.h"
#include "brw_context.h"
#include "brw_eu.h"
#include "brw_wm.h"
#include "brw_cs.h"
#include "brw_vec4.h"
#include "brw_fs.h"
#include "main/uniforms.h"
#include "glsl/glsl_types.h"
#include "glsl/ir_optimization.h"
#include "program/sampler.h"


fs_reg *
fs_visitor::emit_vs_system_value(int location)
{
   fs_reg *reg = new(this->mem_ctx)
      fs_reg(ATTR, VERT_ATTRIB_MAX, BRW_REGISTER_TYPE_D);
   brw_vs_prog_data *vs_prog_data = (brw_vs_prog_data *) prog_data;

   switch (location) {
   case SYSTEM_VALUE_BASE_VERTEX:
      reg->reg_offset = 0;
      vs_prog_data->uses_vertexid = true;
      break;
   case SYSTEM_VALUE_VERTEX_ID:
   case SYSTEM_VALUE_VERTEX_ID_ZERO_BASE:
      reg->reg_offset = 2;
      vs_prog_data->uses_vertexid = true;
      break;
   case SYSTEM_VALUE_INSTANCE_ID:
      reg->reg_offset = 3;
      vs_prog_data->uses_instanceid = true;
      break;
   default:
      unreachable("not reached");
   }

   return reg;
}

void
fs_visitor::visit(ir_variable *ir)
{
   fs_reg *reg = NULL;

   if (variable_storage(ir))
      return;

   if (ir->data.mode == ir_var_shader_in) {
      assert(ir->data.location != -1);
      if (stage == MESA_SHADER_VERTEX) {
         reg = new(this->mem_ctx)
            fs_reg(ATTR, ir->data.location,
                   brw_type_for_base_type(ir->type->get_scalar_type()));
      } else if (ir->data.location == VARYING_SLOT_POS) {
         reg = emit_fragcoord_interpolation(ir->data.pixel_center_integer,
                                            ir->data.origin_upper_left);
      } else if (ir->data.location == VARYING_SLOT_FACE) {
	 reg = emit_frontfacing_interpolation();
      } else {
         reg = new(this->mem_ctx) fs_reg(vgrf(ir->type));
         emit_general_interpolation(*reg, ir->name, ir->type,
                                    (glsl_interp_qualifier) ir->data.interpolation,
                                    ir->data.location, ir->data.centroid,
                                    ir->data.sample);
      }
      assert(reg);
      hash_table_insert(this->variable_ht, reg, ir);
      return;
   } else if (ir->data.mode == ir_var_shader_out) {
      reg = new(this->mem_ctx) fs_reg(vgrf(ir->type));

      if (stage == MESA_SHADER_VERTEX) {
	 int vector_elements =
	    ir->type->is_array() ? ir->type->fields.array->vector_elements
				 : ir->type->vector_elements;

	 for (int i = 0; i < (type_size(ir->type) + 3) / 4; i++) {
	    int output = ir->data.location + i;
	    this->outputs[output] = *reg;
	    this->outputs[output].reg_offset = i * 4;
	    this->output_components[output] = vector_elements;
	 }

      } else if (ir->data.index > 0) {
	 assert(ir->data.location == FRAG_RESULT_DATA0);
	 assert(ir->data.index == 1);
	 this->dual_src_output = *reg;
         this->do_dual_src = true;
      } else if (ir->data.location == FRAG_RESULT_COLOR) {
	 /* Writing gl_FragColor outputs to all color regions. */
         assert(stage == MESA_SHADER_FRAGMENT);
         brw_wm_prog_key *key = (brw_wm_prog_key*) this->key;
	 for (unsigned int i = 0; i < MAX2(key->nr_color_regions, 1); i++) {
	    this->outputs[i] = *reg;
	    this->output_components[i] = 4;
	 }
      } else if (ir->data.location == FRAG_RESULT_DEPTH) {
	 this->frag_depth = *reg;
      } else if (ir->data.location == FRAG_RESULT_SAMPLE_MASK) {
         this->sample_mask = *reg;
      } else {
	 /* gl_FragData or a user-defined FS output */
	 assert(ir->data.location >= FRAG_RESULT_DATA0 &&
		ir->data.location < FRAG_RESULT_DATA0 + BRW_MAX_DRAW_BUFFERS);

	 int vector_elements =
	    ir->type->is_array() ? ir->type->fields.array->vector_elements
				 : ir->type->vector_elements;

	 /* General color output. */
	 for (unsigned int i = 0; i < MAX2(1, ir->type->length); i++) {
	    int output = ir->data.location - FRAG_RESULT_DATA0 + i;
	    this->outputs[output] = offset(*reg, vector_elements * i);
	    this->output_components[output] = vector_elements;
	 }
      }
   } else if (ir->data.mode == ir_var_uniform) {
      int param_index = uniforms;

      /* Thanks to the lower_ubo_reference pass, we will see only
       * ir_binop_ubo_load expressions and not ir_dereference_variable for UBO
       * variables, so no need for them to be in variable_ht.
       *
       * Some uniforms, such as samplers and atomic counters, have no actual
       * storage, so we should ignore them.
       */
      if (ir->is_in_uniform_block() || type_size(ir->type) == 0)
         return;

      if (dispatch_width == 16) {
	 if (!variable_storage(ir)) {
	    fail("Failed to find uniform '%s' in SIMD16\n", ir->name);
	 }
	 return;
      }

      param_size[param_index] = type_size(ir->type);
      if (!strncmp(ir->name, "gl_", 3)) {
	 setup_builtin_uniform_values(ir);
      } else {
	 setup_uniform_values(ir);
      }

      reg = new(this->mem_ctx) fs_reg(UNIFORM, param_index);
      reg->type = brw_type_for_base_type(ir->type);

   } else if (ir->data.mode == ir_var_system_value) {
      switch (ir->data.location) {
      case SYSTEM_VALUE_BASE_VERTEX:
      case SYSTEM_VALUE_VERTEX_ID:
      case SYSTEM_VALUE_VERTEX_ID_ZERO_BASE:
      case SYSTEM_VALUE_INSTANCE_ID:
         reg = emit_vs_system_value(ir->data.location);
         break;
      case SYSTEM_VALUE_SAMPLE_POS:
	 reg = emit_samplepos_setup();
         break;
      case SYSTEM_VALUE_SAMPLE_ID:
	 reg = emit_sampleid_setup();
         break;
      case SYSTEM_VALUE_SAMPLE_MASK_IN:
         assert(devinfo->gen >= 7);
         reg = new(mem_ctx)
            fs_reg(retype(brw_vec8_grf(payload.sample_mask_in_reg, 0),
                          BRW_REGISTER_TYPE_D));
         break;
      }
   }

   if (!reg)
      reg = new(this->mem_ctx) fs_reg(vgrf(ir->type));

   hash_table_insert(this->variable_ht, reg, ir);
}

void
fs_visitor::visit(ir_dereference_variable *ir)
{
   fs_reg *reg = variable_storage(ir->var);

   if (!reg) {
      fail("Failed to find variable storage for %s\n", ir->var->name);
      this->result = fs_reg(reg_null_d);
      return;
   }
   this->result = *reg;
}

void
fs_visitor::visit(ir_dereference_record *ir)
{
   const glsl_type *struct_type = ir->record->type;

   ir->record->accept(this);

   unsigned int off = 0;
   for (unsigned int i = 0; i < struct_type->length; i++) {
      if (strcmp(struct_type->fields.structure[i].name, ir->field) == 0)
	 break;
      off += type_size(struct_type->fields.structure[i].type);
   }
   this->result = offset(this->result, off);
   this->result.type = brw_type_for_base_type(ir->type);
}

void
fs_visitor::visit(ir_dereference_array *ir)
{
   ir_constant *constant_index;
   fs_reg src;
   int element_size = type_size(ir->type);

   constant_index = ir->array_index->as_constant();

   ir->array->accept(this);
   src = this->result;
   src.type = brw_type_for_base_type(ir->type);

   if (constant_index) {
      if (src.file == ATTR) {
         /* Attribute arrays get loaded as one vec4 per element.  In that case
          * offset the source register.
          */
         src.reg += constant_index->value.i[0];
      } else {
         assert(src.file == UNIFORM || src.file == GRF || src.file == HW_REG);
         src = offset(src, constant_index->value.i[0] * element_size);
      }
   } else {
      /* Variable index array dereference.  We attach the variable index
       * component to the reg as a pointer to a register containing the
       * offset.  Currently only uniform arrays are supported in this patch,
       * and that reladdr pointer is resolved by
       * move_uniform_array_access_to_pull_constants().  All other array types
       * are lowered by lower_variable_index_to_cond_assign().
       */
      ir->array_index->accept(this);

      fs_reg index_reg;
      index_reg = vgrf(glsl_type::int_type);
      emit(BRW_OPCODE_MUL, index_reg, this->result, fs_reg(element_size));

      if (src.reladdr) {
         emit(BRW_OPCODE_ADD, index_reg, *src.reladdr, index_reg);
      }

      src.reladdr = ralloc(mem_ctx, fs_reg);
      memcpy(src.reladdr, &index_reg, sizeof(index_reg));
   }
   this->result = src;
}

fs_inst *
fs_visitor::emit_lrp(const fs_reg &dst, const fs_reg &x, const fs_reg &y,
                     const fs_reg &a)
{
   if (devinfo->gen < 6) {
      /* We can't use the LRP instruction.  Emit x*(1-a) + y*a. */
      fs_reg y_times_a           = vgrf(glsl_type::float_type);
      fs_reg one_minus_a         = vgrf(glsl_type::float_type);
      fs_reg x_times_one_minus_a = vgrf(glsl_type::float_type);

      emit(MUL(y_times_a, y, a));

      fs_reg negative_a = a;
      negative_a.negate = !a.negate;
      emit(ADD(one_minus_a, negative_a, fs_reg(1.0f)));
      emit(MUL(x_times_one_minus_a, x, one_minus_a));

      return emit(ADD(dst, x_times_one_minus_a, y_times_a));
   } else {
      /* The LRP instruction actually does op1 * op0 + op2 * (1 - op0), so
       * we need to reorder the operands.
       */
      return emit(LRP(dst, a, y, x));
   }
}

void
fs_visitor::emit_minmax(enum brw_conditional_mod conditionalmod, const fs_reg &dst,
                        const fs_reg &src0, const fs_reg &src1)
{
   assert(conditionalmod == BRW_CONDITIONAL_GE ||
          conditionalmod == BRW_CONDITIONAL_L);

   fs_inst *inst;

   if (devinfo->gen >= 6) {
      inst = emit(BRW_OPCODE_SEL, dst, src0, src1);
      inst->conditional_mod = conditionalmod;
   } else {
      emit(CMP(reg_null_d, src0, src1, conditionalmod));

      inst = emit(BRW_OPCODE_SEL, dst, src0, src1);
      inst->predicate = BRW_PREDICATE_NORMAL;
   }
}

void
fs_visitor::emit_uniformize(const fs_reg &dst, const fs_reg &src)
{
   const fs_reg chan_index = vgrf(glsl_type::uint_type);

   emit(SHADER_OPCODE_FIND_LIVE_CHANNEL, component(chan_index, 0))
      ->force_writemask_all = true;
   emit(SHADER_OPCODE_BROADCAST, component(dst, 0),
        src, component(chan_index, 0))
      ->force_writemask_all = true;
}

bool
fs_visitor::try_emit_saturate(ir_expression *ir)
{
   if (ir->operation != ir_unop_saturate)
      return false;

   ir_rvalue *sat_val = ir->operands[0];

   fs_inst *pre_inst = (fs_inst *) this->instructions.get_tail();

   sat_val->accept(this);
   fs_reg src = this->result;

   fs_inst *last_inst = (fs_inst *) this->instructions.get_tail();

   /* If the last instruction from our accept() generated our
    * src, just set the saturate flag instead of emmitting a separate mov.
    */
   fs_inst *modify = get_instruction_generating_reg(pre_inst, last_inst, src);
   if (modify && modify->regs_written == modify->dst.width / 8 &&
       modify->can_do_saturate()) {
      modify->saturate = true;
      this->result = src;
      return true;
   }

   return false;
}

bool
fs_visitor::try_emit_line(ir_expression *ir)
{
   /* LINE's src0 must be of type float. */
   if (ir->type != glsl_type::float_type)
      return false;

   ir_rvalue *nonmul = ir->operands[1];
   ir_expression *mul = ir->operands[0]->as_expression();

   if (!mul || mul->operation != ir_binop_mul) {
      nonmul = ir->operands[0];
      mul = ir->operands[1]->as_expression();

      if (!mul || mul->operation != ir_binop_mul)
         return false;
   }

   ir_constant *const_add = nonmul->as_constant();
   if (!const_add)
      return false;

   int add_operand_vf = brw_float_to_vf(const_add->value.f[0]);
   if (add_operand_vf == -1)
      return false;

   ir_rvalue *non_const_mul = mul->operands[1];
   ir_constant *const_mul = mul->operands[0]->as_constant();
   if (!const_mul) {
      const_mul = mul->operands[1]->as_constant();

      if (!const_mul)
         return false;

      non_const_mul = mul->operands[0];
   }

   int mul_operand_vf = brw_float_to_vf(const_mul->value.f[0]);
   if (mul_operand_vf == -1)
      return false;

   non_const_mul->accept(this);
   fs_reg src1 = this->result;

   fs_reg src0 = vgrf(ir->type);
   emit(BRW_OPCODE_MOV, src0,
        fs_reg((uint8_t)mul_operand_vf, 0, 0, (uint8_t)add_operand_vf));

   this->result = vgrf(ir->type);
   emit(BRW_OPCODE_LINE, this->result, src0, src1);
   return true;
}

bool
fs_visitor::try_emit_mad(ir_expression *ir)
{
   /* 3-src instructions were introduced in gen6. */
   if (devinfo->gen < 6)
      return false;

   /* MAD can only handle floating-point data. */
   if (ir->type != glsl_type::float_type)
      return false;

   ir_rvalue *nonmul;
   ir_expression *mul;
   bool mul_negate, mul_abs;

   for (int i = 0; i < 2; i++) {
      mul_negate = false;
      mul_abs = false;

      mul = ir->operands[i]->as_expression();
      nonmul = ir->operands[1 - i];

      if (mul && mul->operation == ir_unop_abs) {
         mul = mul->operands[0]->as_expression();
         mul_abs = true;
      } else if (mul && mul->operation == ir_unop_neg) {
         mul = mul->operands[0]->as_expression();
         mul_negate = true;
      }

      if (mul && mul->operation == ir_binop_mul)
         break;
   }

   if (!mul || mul->operation != ir_binop_mul)
      return false;

   nonmul->accept(this);
   fs_reg src0 = this->result;

   mul->operands[0]->accept(this);
   fs_reg src1 = this->result;
   src1.negate ^= mul_negate;
   src1.abs = mul_abs;
   if (mul_abs)
      src1.negate = false;

   mul->operands[1]->accept(this);
   fs_reg src2 = this->result;
   src2.abs = mul_abs;
   if (mul_abs)
      src2.negate = false;

   this->result = vgrf(ir->type);
   emit(BRW_OPCODE_MAD, this->result, src0, src1, src2);

   return true;
}

bool
fs_visitor::try_emit_b2f_of_comparison(ir_expression *ir)
{
   /* On platforms that do not natively generate 0u and ~0u for Boolean
    * results, b2f expressions that look like
    *
    *     f = b2f(expr cmp 0)
    *
    * will generate better code by pretending the expression is
    *
    *     f = ir_triop_csel(0.0, 1.0, expr cmp 0)
    *
    * This is because the last instruction of "expr" can generate the
    * condition code for the "cmp 0".  This avoids having to do the "-(b & 1)"
    * trick to generate 0u or ~0u for the Boolean result.  This means code like
    *
    *     mov(16)         g16<1>F         1F
    *     mul.ge.f0(16)   null            g6<8,8,1>F      g14<8,8,1>F
    *     (+f0) sel(16)   m6<1>F          g16<8,8,1>F     0F
    *
    * will be generated instead of
    *
    *     mul(16)         g2<1>F          g12<8,8,1>F     g4<8,8,1>F
    *     cmp.ge.f0(16)   g2<1>D          g4<8,8,1>F      0F
    *     and(16)         g4<1>D          g2<8,8,1>D      1D
    *     and(16)         m6<1>D          -g4<8,8,1>D     0x3f800000UD
    *
    * When the comparison is != 0.0 using the knowledge that the false case
    * already results in zero would allow better code generation by possibly
    * avoiding a load-immediate instruction.
    */
   ir_expression *cmp = ir->operands[0]->as_expression();
   if (cmp == NULL)
      return false;

   if (cmp->operation == ir_binop_nequal) {
      for (unsigned i = 0; i < 2; i++) {
         ir_constant *c = cmp->operands[i]->as_constant();
         if (c == NULL || !c->is_zero())
            continue;

         ir_expression *expr = cmp->operands[i ^ 1]->as_expression();
         if (expr != NULL) {
            fs_reg op[2];

            for (unsigned j = 0; j < 2; j++) {
               cmp->operands[j]->accept(this);
               op[j] = this->result;

               resolve_ud_negate(&op[j]);
            }

            emit_bool_to_cond_code_of_reg(cmp, op);

            /* In this case we know when the condition is true, op[i ^ 1]
             * contains zero.  Invert the predicate, use op[i ^ 1] as src0,
             * and immediate 1.0f as src1.
             */
            this->result = vgrf(ir->type);
            op[i ^ 1].type = BRW_REGISTER_TYPE_F;

            fs_inst *inst = emit(SEL(this->result, op[i ^ 1], fs_reg(1.0f)));
            inst->predicate = BRW_PREDICATE_NORMAL;
            inst->predicate_inverse = true;
            return true;
         }
      }
   }

   emit_bool_to_cond_code(cmp);

   fs_reg temp = vgrf(ir->type);
   emit(MOV(temp, fs_reg(1.0f)));

   this->result = vgrf(ir->type);
   fs_inst *inst = emit(SEL(this->result, temp, fs_reg(0.0f)));
   inst->predicate = BRW_PREDICATE_NORMAL;

   return true;
}

static int
pack_pixel_offset(float x)
{
   /* Clamp upper end of the range to +7/16. See explanation in non-constant
    * offset case below. */
   int n = MIN2((int)(x * 16), 7);
   return n & 0xf;
}

void
fs_visitor::emit_interpolate_expression(ir_expression *ir)
{
   /* in SIMD16 mode, the pixel interpolator returns coords interleaved
    * 8 channels at a time, same as the barycentric coords presented in
    * the FS payload. this requires a bit of extra work to support.
    */
   no16("interpolate_at_* not yet supported in SIMD16 mode.");

   assert(stage == MESA_SHADER_FRAGMENT);
   brw_wm_prog_key *key = (brw_wm_prog_key*) this->key;

   ir_dereference * deref = ir->operands[0]->as_dereference();
   ir_swizzle * swiz = NULL;
   if (!deref) {
      /* the api does not allow a swizzle here, but the varying packing code
       * may have pushed one into here.
       */
      swiz = ir->operands[0]->as_swizzle();
      assert(swiz);
      deref = swiz->val->as_dereference();
   }
   assert(deref);
   ir_variable * var = deref->variable_referenced();
   assert(var);

   /* 1. collect interpolation factors */

   fs_reg dst_xy = vgrf(glsl_type::get_instance(ir->type->base_type, 2, 1));

   /* for most messages, we need one reg of ignored data; the hardware requires mlen==1
    * even when there is no payload. in the per-slot offset case, we'll replace this with
    * the proper source data. */
   fs_reg src = vgrf(glsl_type::float_type);
   int mlen = 1;     /* one reg unless overriden */
   int reg_width = dispatch_width / 8;
   fs_inst *inst;

   switch (ir->operation) {
   case ir_unop_interpolate_at_centroid:
      inst = emit(FS_OPCODE_INTERPOLATE_AT_CENTROID, dst_xy, src, fs_reg(0u));
      break;

   case ir_binop_interpolate_at_sample: {
      ir_constant *sample_num = ir->operands[1]->as_constant();
      assert(sample_num || !"nonconstant sample number should have been lowered.");

      unsigned msg_data = sample_num->value.i[0] << 4;
      inst = emit(FS_OPCODE_INTERPOLATE_AT_SAMPLE, dst_xy, src, fs_reg(msg_data));
      break;
   }

   case ir_binop_interpolate_at_offset: {
      ir_constant *const_offset = ir->operands[1]->as_constant();
      if (const_offset) {
         unsigned msg_data = pack_pixel_offset(const_offset->value.f[0]) |
                            (pack_pixel_offset(const_offset->value.f[1]) << 4);
         inst = emit(FS_OPCODE_INTERPOLATE_AT_SHARED_OFFSET, dst_xy, src,
                     fs_reg(msg_data));
      } else {
         /* pack the operands: hw wants offsets as 4 bit signed ints */
         ir->operands[1]->accept(this);
         src = vgrf(glsl_type::ivec2_type);
         fs_reg src2 = src;
         for (int i = 0; i < 2; i++) {
            fs_reg temp = vgrf(glsl_type::float_type);
            emit(MUL(temp, this->result, fs_reg(16.0f)));
            emit(MOV(src2, temp));  /* float to int */

            /* Clamp the upper end of the range to +7/16. ARB_gpu_shader5 requires
             * that we support a maximum offset of +0.5, which isn't representable
             * in a S0.4 value -- if we didn't clamp it, we'd end up with -8/16,
             * which is the opposite of what the shader author wanted.
             *
             * This is legal due to ARB_gpu_shader5's quantization rules:
             *
             * "Not all values of <offset> may be supported; x and y offsets may
             * be rounded to fixed-point values with the number of fraction bits
             * given by the implementation-dependent constant
             * FRAGMENT_INTERPOLATION_OFFSET_BITS"
             */

            fs_inst *inst = emit(BRW_OPCODE_SEL, src2, src2, fs_reg(7));
            inst->conditional_mod = BRW_CONDITIONAL_L; /* min(src2, 7) */

            src2 = offset(src2, 1);
            this->result = offset(this->result, 1);
         }

         mlen = 2 * reg_width;
         inst = emit(FS_OPCODE_INTERPOLATE_AT_PER_SLOT_OFFSET, dst_xy, src,
                     fs_reg(0u));
      }
      break;
   }

   default:
      unreachable("not reached");
   }

   inst->mlen = mlen;
   inst->regs_written = 2 * reg_width; /* 2 floats per slot returned */
   inst->pi_noperspective = var->determine_interpolation_mode(key->flat_shade) ==
         INTERP_QUALIFIER_NOPERSPECTIVE;

   /* 2. emit linterp */

   fs_reg res = vgrf(ir->type);
   this->result = res;

   for (int i = 0; i < ir->type->vector_elements; i++) {
      int ch = swiz ? ((*(int *)&swiz->mask) >> 2*i) & 3 : i;
      emit(FS_OPCODE_LINTERP, res, dst_xy,
           fs_reg(interp_reg(var->data.location, ch)));
      res = offset(res, 1);
   }
}

void
fs_visitor::visit(ir_expression *ir)
{
   unsigned int operand;
   fs_reg op[3], temp;
   fs_inst *inst;
   struct brw_wm_prog_key *fs_key = (struct brw_wm_prog_key *) this->key;

   assert(ir->get_num_operands() <= 3);

   if (try_emit_saturate(ir))
      return;

   /* Deal with the real oddball stuff first */
   switch (ir->operation) {
   case ir_binop_add:
      if (devinfo->gen <= 5 && try_emit_line(ir))
         return;
      if (try_emit_mad(ir))
         return;
      break;

   case ir_triop_csel:
      ir->operands[1]->accept(this);
      op[1] = this->result;
      ir->operands[2]->accept(this);
      op[2] = this->result;

      emit_bool_to_cond_code(ir->operands[0]);

      this->result = vgrf(ir->type);
      inst = emit(SEL(this->result, op[1], op[2]));
      inst->predicate = BRW_PREDICATE_NORMAL;
      return;

   case ir_unop_b2f:
      if (devinfo->gen <= 5 && try_emit_b2f_of_comparison(ir))
         return;
      break;

   case ir_unop_interpolate_at_centroid:
   case ir_binop_interpolate_at_offset:
   case ir_binop_interpolate_at_sample:
      emit_interpolate_expression(ir);
      return;

   default:
      break;
   }

   for (operand = 0; operand < ir->get_num_operands(); operand++) {
      ir->operands[operand]->accept(this);
      if (this->result.file == BAD_FILE) {
	 fail("Failed to get tree for expression operand:\n");
	 ir->operands[operand]->fprint(stderr);
         fprintf(stderr, "\n");
      }
      assert(this->result.file == GRF ||
             this->result.file == UNIFORM || this->result.file == ATTR);
      op[operand] = this->result;

      /* Matrix expression operands should have been broken down to vector
       * operations already.
       */
      assert(!ir->operands[operand]->type->is_matrix());
      /* And then those vector operands should have been broken down to scalar.
       */
      assert(!ir->operands[operand]->type->is_vector());
   }

   /* Storage for our result.  If our result goes into an assignment, it will
    * just get copy-propagated out, so no worries.
    */
   this->result = vgrf(ir->type);

   switch (ir->operation) {
   case ir_unop_logic_not:
      emit(NOT(this->result, op[0]));
      break;
   case ir_unop_neg:
      op[0].negate = !op[0].negate;
      emit(MOV(this->result, op[0]));
      break;
   case ir_unop_abs:
      op[0].abs = true;
      op[0].negate = false;
      emit(MOV(this->result, op[0]));
      break;
   case ir_unop_sign:
      if (ir->type->is_float()) {
         /* AND(val, 0x80000000) gives the sign bit.
          *
          * Predicated OR ORs 1.0 (0x3f800000) with the sign bit if val is not
          * zero.
          */
         emit(CMP(reg_null_f, op[0], fs_reg(0.0f), BRW_CONDITIONAL_NZ));

         op[0].type = BRW_REGISTER_TYPE_UD;
         this->result.type = BRW_REGISTER_TYPE_UD;
         emit(AND(this->result, op[0], fs_reg(0x80000000u)));

         inst = emit(OR(this->result, this->result, fs_reg(0x3f800000u)));
         inst->predicate = BRW_PREDICATE_NORMAL;

         this->result.type = BRW_REGISTER_TYPE_F;
      } else {
         /*  ASR(val, 31) -> negative val generates 0xffffffff (signed -1).
          *               -> non-negative val generates 0x00000000.
          *  Predicated OR sets 1 if val is positive.
          */
         emit(CMP(reg_null_d, op[0], fs_reg(0), BRW_CONDITIONAL_G));

         emit(ASR(this->result, op[0], fs_reg(31)));

         inst = emit(OR(this->result, this->result, fs_reg(1)));
         inst->predicate = BRW_PREDICATE_NORMAL;
      }
      break;
   case ir_unop_rcp:
      emit_math(SHADER_OPCODE_RCP, this->result, op[0]);
      break;

   case ir_unop_exp2:
      emit_math(SHADER_OPCODE_EXP2, this->result, op[0]);
      break;
   case ir_unop_log2:
      emit_math(SHADER_OPCODE_LOG2, this->result, op[0]);
      break;
   case ir_unop_exp:
   case ir_unop_log:
      unreachable("not reached: should be handled by ir_explog_to_explog2");
   case ir_unop_sin:
      emit_math(SHADER_OPCODE_SIN, this->result, op[0]);
      break;
   case ir_unop_cos:
      emit_math(SHADER_OPCODE_COS, this->result, op[0]);
      break;

   case ir_unop_dFdx:
      /* Select one of the two opcodes based on the glHint value. */
      if (fs_key->high_quality_derivatives)
         emit(FS_OPCODE_DDX_FINE, this->result, op[0]);
      else
         emit(FS_OPCODE_DDX_COARSE, this->result, op[0]);
      break;

   case ir_unop_dFdx_coarse:
      emit(FS_OPCODE_DDX_COARSE, this->result, op[0]);
      break;

   case ir_unop_dFdx_fine:
      emit(FS_OPCODE_DDX_FINE, this->result, op[0]);
      break;

   case ir_unop_dFdy:
      /* Select one of the two opcodes based on the glHint value. */
      if (fs_key->high_quality_derivatives)
         emit(FS_OPCODE_DDY_FINE, result, op[0], fs_reg(fs_key->render_to_fbo));
      else
         emit(FS_OPCODE_DDY_COARSE, result, op[0], fs_reg(fs_key->render_to_fbo));
      break;

   case ir_unop_dFdy_coarse:
      emit(FS_OPCODE_DDY_COARSE, result, op[0], fs_reg(fs_key->render_to_fbo));
      break;

   case ir_unop_dFdy_fine:
      emit(FS_OPCODE_DDY_FINE, result, op[0], fs_reg(fs_key->render_to_fbo));
      break;

   case ir_binop_add:
      emit(ADD(this->result, op[0], op[1]));
      break;
   case ir_binop_sub:
      unreachable("not reached: should be handled by ir_sub_to_add_neg");

   case ir_binop_mul:
      emit(MUL(this->result, op[0], op[1]));
      break;
   case ir_binop_imul_high: {
      if (devinfo->gen >= 7)
         no16("SIMD16 explicit accumulator operands unsupported\n");

      struct brw_reg acc = retype(brw_acc_reg(dispatch_width),
                                  this->result.type);

      fs_inst *mul = emit(MUL(acc, op[0], op[1]));
      emit(MACH(this->result, op[0], op[1]));

      /* Until Gen8, integer multiplies read 32-bits from one source, and
       * 16-bits from the other, and relying on the MACH instruction to
       * generate the high bits of the result.
       *
       * On Gen8, the multiply instruction does a full 32x32-bit multiply,
       * but in order to do a 64x64-bit multiply we have to simulate the
       * previous behavior and then use a MACH instruction.
       *
       * FINISHME: Don't use source modifiers on src1.
       */
      if (devinfo->gen >= 8) {
         assert(mul->src[1].type == BRW_REGISTER_TYPE_D ||
                mul->src[1].type == BRW_REGISTER_TYPE_UD);
         if (mul->src[1].type == BRW_REGISTER_TYPE_D) {
            mul->src[1].type = BRW_REGISTER_TYPE_W;
            mul->src[1].stride = 2;
         } else {
            mul->src[1].type = BRW_REGISTER_TYPE_UW;
            mul->src[1].stride = 2;
         }
      }

      break;
   }
   case ir_binop_div:
      /* Floating point should be lowered by DIV_TO_MUL_RCP in the compiler. */
      assert(ir->type->is_integer());
      emit_math(SHADER_OPCODE_INT_QUOTIENT, this->result, op[0], op[1]);
      break;
   case ir_binop_carry: {
      if (devinfo->gen >= 7)
         no16("SIMD16 explicit accumulator operands unsupported\n");

      struct brw_reg acc = retype(brw_acc_reg(dispatch_width),
                                  BRW_REGISTER_TYPE_UD);

      emit(ADDC(reg_null_ud, op[0], op[1]));
      emit(MOV(this->result, fs_reg(acc)));
      break;
   }
   case ir_binop_borrow: {
      if (devinfo->gen >= 7)
         no16("SIMD16 explicit accumulator operands unsupported\n");

      struct brw_reg acc = retype(brw_acc_reg(dispatch_width),
                                  BRW_REGISTER_TYPE_UD);

      emit(SUBB(reg_null_ud, op[0], op[1]));
      emit(MOV(this->result, fs_reg(acc)));
      break;
   }
   case ir_binop_mod:
      /* Floating point should be lowered by MOD_TO_FLOOR in the compiler. */
      assert(ir->type->is_integer());
      emit_math(SHADER_OPCODE_INT_REMAINDER, this->result, op[0], op[1]);
      break;

   case ir_binop_less:
   case ir_binop_greater:
   case ir_binop_lequal:
   case ir_binop_gequal:
   case ir_binop_equal:
   case ir_binop_all_equal:
   case ir_binop_nequal:
   case ir_binop_any_nequal:
      if (devinfo->gen <= 5) {
         resolve_bool_comparison(ir->operands[0], &op[0]);
         resolve_bool_comparison(ir->operands[1], &op[1]);
      }

      emit(CMP(this->result, op[0], op[1],
               brw_conditional_for_comparison(ir->operation)));
      break;

   case ir_binop_logic_xor:
      emit(XOR(this->result, op[0], op[1]));
      break;

   case ir_binop_logic_or:
      emit(OR(this->result, op[0], op[1]));
      break;

   case ir_binop_logic_and:
      emit(AND(this->result, op[0], op[1]));
      break;

   case ir_binop_dot:
   case ir_unop_any:
      unreachable("not reached: should be handled by brw_fs_channel_expressions");

   case ir_unop_noise:
      unreachable("not reached: should be handled by lower_noise");

   case ir_quadop_vector:
      unreachable("not reached: should be handled by lower_quadop_vector");

   case ir_binop_vector_extract:
      unreachable("not reached: should be handled by lower_vec_index_to_cond_assign()");

   case ir_triop_vector_insert:
      unreachable("not reached: should be handled by lower_vector_insert()");

   case ir_binop_ldexp:
      unreachable("not reached: should be handled by ldexp_to_arith()");

   case ir_unop_sqrt:
      emit_math(SHADER_OPCODE_SQRT, this->result, op[0]);
      break;

   case ir_unop_rsq:
      emit_math(SHADER_OPCODE_RSQ, this->result, op[0]);
      break;

   case ir_unop_bitcast_i2f:
   case ir_unop_bitcast_u2f:
      op[0].type = BRW_REGISTER_TYPE_F;
      this->result = op[0];
      break;
   case ir_unop_i2u:
   case ir_unop_bitcast_f2u:
      op[0].type = BRW_REGISTER_TYPE_UD;
      this->result = op[0];
      break;
   case ir_unop_u2i:
   case ir_unop_bitcast_f2i:
      op[0].type = BRW_REGISTER_TYPE_D;
      this->result = op[0];
      break;
   case ir_unop_i2f:
   case ir_unop_u2f:
   case ir_unop_f2i:
   case ir_unop_f2u:
      emit(MOV(this->result, op[0]));
      break;

   case ir_unop_b2i:
      emit(AND(this->result, op[0], fs_reg(1)));
      break;
   case ir_unop_b2f:
      if (devinfo->gen <= 5) {
         resolve_bool_comparison(ir->operands[0], &op[0]);
      }
      op[0].type = BRW_REGISTER_TYPE_D;
      this->result.type = BRW_REGISTER_TYPE_D;
      emit(AND(this->result, op[0], fs_reg(0x3f800000u)));
      this->result.type = BRW_REGISTER_TYPE_F;
      break;

   case ir_unop_f2b:
      emit(CMP(this->result, op[0], fs_reg(0.0f), BRW_CONDITIONAL_NZ));
      break;
   case ir_unop_i2b:
      emit(CMP(this->result, op[0], fs_reg(0), BRW_CONDITIONAL_NZ));
      break;

   case ir_unop_trunc:
      emit(RNDZ(this->result, op[0]));
      break;
   case ir_unop_ceil: {
         fs_reg tmp = vgrf(ir->type);
         op[0].negate = !op[0].negate;
         emit(RNDD(tmp, op[0]));
         tmp.negate = true;
         emit(MOV(this->result, tmp));
      }
      break;
   case ir_unop_floor:
      emit(RNDD(this->result, op[0]));
      break;
   case ir_unop_fract:
      emit(FRC(this->result, op[0]));
      break;
   case ir_unop_round_even:
      emit(RNDE(this->result, op[0]));
      break;

   case ir_binop_min:
   case ir_binop_max:
      resolve_ud_negate(&op[0]);
      resolve_ud_negate(&op[1]);
      emit_minmax(ir->operation == ir_binop_min ?
                  BRW_CONDITIONAL_L : BRW_CONDITIONAL_GE,
                  this->result, op[0], op[1]);
      break;
   case ir_unop_pack_snorm_2x16:
   case ir_unop_pack_snorm_4x8:
   case ir_unop_pack_unorm_2x16:
   case ir_unop_pack_unorm_4x8:
   case ir_unop_unpack_snorm_2x16:
   case ir_unop_unpack_snorm_4x8:
   case ir_unop_unpack_unorm_2x16:
   case ir_unop_unpack_unorm_4x8:
   case ir_unop_unpack_half_2x16:
   case ir_unop_pack_half_2x16:
      unreachable("not reached: should be handled by lower_packing_builtins");
   case ir_unop_unpack_half_2x16_split_x:
      emit(FS_OPCODE_UNPACK_HALF_2x16_SPLIT_X, this->result, op[0]);
      break;
   case ir_unop_unpack_half_2x16_split_y:
      emit(FS_OPCODE_UNPACK_HALF_2x16_SPLIT_Y, this->result, op[0]);
      break;
   case ir_binop_pow:
      emit_math(SHADER_OPCODE_POW, this->result, op[0], op[1]);
      break;

   case ir_unop_bitfield_reverse:
      emit(BFREV(this->result, op[0]));
      break;
   case ir_unop_bit_count:
      emit(CBIT(this->result, op[0]));
      break;
   case ir_unop_find_msb:
      temp = vgrf(glsl_type::uint_type);
      emit(FBH(temp, op[0]));

      /* FBH counts from the MSB side, while GLSL's findMSB() wants the count
       * from the LSB side. If FBH didn't return an error (0xFFFFFFFF), then
       * subtract the result from 31 to convert the MSB count into an LSB count.
       */

      /* FBH only supports UD type for dst, so use a MOV to convert UD to D. */
      emit(MOV(this->result, temp));
      emit(CMP(reg_null_d, this->result, fs_reg(-1), BRW_CONDITIONAL_NZ));

      temp.negate = true;
      inst = emit(ADD(this->result, temp, fs_reg(31)));
      inst->predicate = BRW_PREDICATE_NORMAL;
      break;
   case ir_unop_find_lsb:
      emit(FBL(this->result, op[0]));
      break;
   case ir_unop_saturate:
      inst = emit(MOV(this->result, op[0]));
      inst->saturate = true;
      break;
   case ir_triop_bitfield_extract:
      /* Note that the instruction's argument order is reversed from GLSL
       * and the IR.
       */
      emit(BFE(this->result, op[2], op[1], op[0]));
      break;
   case ir_binop_bfm:
      emit(BFI1(this->result, op[0], op[1]));
      break;
   case ir_triop_bfi:
      emit(BFI2(this->result, op[0], op[1], op[2]));
      break;
   case ir_quadop_bitfield_insert:
      unreachable("not reached: should be handled by "
              "lower_instructions::bitfield_insert_to_bfm_bfi");

   case ir_unop_bit_not:
      emit(NOT(this->result, op[0]));
      break;
   case ir_binop_bit_and:
      emit(AND(this->result, op[0], op[1]));
      break;
   case ir_binop_bit_xor:
      emit(XOR(this->result, op[0], op[1]));
      break;
   case ir_binop_bit_or:
      emit(OR(this->result, op[0], op[1]));
      break;

   case ir_binop_lshift:
      emit(SHL(this->result, op[0], op[1]));
      break;

   case ir_binop_rshift:
      if (ir->type->base_type == GLSL_TYPE_INT)
	 emit(ASR(this->result, op[0], op[1]));
      else
	 emit(SHR(this->result, op[0], op[1]));
      break;
   case ir_binop_pack_half_2x16_split:
      emit(FS_OPCODE_PACK_HALF_2x16_SPLIT, this->result, op[0], op[1]);
      break;
   case ir_binop_ubo_load: {
      /* This IR node takes a constant uniform block and a constant or
       * variable byte offset within the block and loads a vector from that.
       */
      ir_constant *const_uniform_block = ir->operands[0]->as_constant();
      ir_constant *const_offset = ir->operands[1]->as_constant();
      fs_reg surf_index;

      if (const_uniform_block) {
         /* The block index is a constant, so just emit the binding table entry
          * as an immediate.
          */
         surf_index = fs_reg(stage_prog_data->binding_table.ubo_start +
                                 const_uniform_block->value.u[0]);
      } else {
         /* The block index is not a constant. Evaluate the index expression
          * per-channel and add the base UBO index; we have to select a value
          * from any live channel.
          */
         surf_index = vgrf(glsl_type::uint_type);
         emit(ADD(surf_index, op[0],
                  fs_reg(stage_prog_data->binding_table.ubo_start)));
         emit_uniformize(surf_index, surf_index);

         /* Assume this may touch any UBO. It would be nice to provide
          * a tighter bound, but the array information is already lowered away.
          */
         brw_mark_surface_used(prog_data,
                               stage_prog_data->binding_table.ubo_start +
                               shader_prog->NumUniformBlocks - 1);
      }

      if (const_offset) {
         fs_reg packed_consts = vgrf(glsl_type::float_type);
         packed_consts.type = result.type;

         fs_reg const_offset_reg = fs_reg(const_offset->value.u[0] & ~15);
         emit(new(mem_ctx) fs_inst(FS_OPCODE_UNIFORM_PULL_CONSTANT_LOAD, 8,
                                   packed_consts, surf_index, const_offset_reg));

         for (int i = 0; i < ir->type->vector_elements; i++) {
            packed_consts.set_smear(const_offset->value.u[0] % 16 / 4 + i);

            /* The std140 packing rules don't allow vectors to cross 16-byte
             * boundaries, and a reg is 32 bytes.
             */
            assert(packed_consts.subreg_offset < 32);

            /* UBO bools are any nonzero value.  We consider bools to be
             * values with the low bit set to 1.  Convert them using CMP.
             */
            if (ir->type->base_type == GLSL_TYPE_BOOL) {
               emit(CMP(result, packed_consts, fs_reg(0u), BRW_CONDITIONAL_NZ));
            } else {
               emit(MOV(result, packed_consts));
            }

            result = offset(result, 1);
         }
      } else {
         /* Turn the byte offset into a dword offset. */
         fs_reg base_offset = vgrf(glsl_type::int_type);
         emit(SHR(base_offset, op[1], fs_reg(2)));

         for (int i = 0; i < ir->type->vector_elements; i++) {
            emit(VARYING_PULL_CONSTANT_LOAD(result, surf_index,
                                            base_offset, i));

            if (ir->type->base_type == GLSL_TYPE_BOOL)
               emit(CMP(result, result, fs_reg(0), BRW_CONDITIONAL_NZ));

            result = offset(result, 1);
         }
      }

      result.reg_offset = 0;
      break;
   }

   case ir_triop_fma:
      /* Note that the instruction's argument order is reversed from GLSL
       * and the IR.
       */
      emit(MAD(this->result, op[2], op[1], op[0]));
      break;

   case ir_triop_lrp:
      emit_lrp(this->result, op[0], op[1], op[2]);
      break;

   case ir_triop_csel:
   case ir_unop_interpolate_at_centroid:
   case ir_binop_interpolate_at_offset:
   case ir_binop_interpolate_at_sample:
      unreachable("already handled above");
      break;

   case ir_unop_d2f:
   case ir_unop_f2d:
   case ir_unop_d2i:
   case ir_unop_i2d:
   case ir_unop_d2u:
   case ir_unop_u2d:
   case ir_unop_d2b:
   case ir_unop_pack_double_2x32:
   case ir_unop_unpack_double_2x32:
   case ir_unop_frexp_sig:
   case ir_unop_frexp_exp:
      unreachable("fp64 todo");
      break;
   }
}

void
fs_visitor::emit_assignment_writes(fs_reg &l, fs_reg &r,
				   const glsl_type *type, bool predicated)
{
   switch (type->base_type) {
   case GLSL_TYPE_FLOAT:
   case GLSL_TYPE_UINT:
   case GLSL_TYPE_INT:
   case GLSL_TYPE_BOOL:
      for (unsigned int i = 0; i < type->components(); i++) {
	 l.type = brw_type_for_base_type(type);
	 r.type = brw_type_for_base_type(type);

	 if (predicated || !l.equals(r)) {
	    fs_inst *inst = emit(MOV(l, r));
	    inst->predicate = predicated ? BRW_PREDICATE_NORMAL : BRW_PREDICATE_NONE;
	 }

	 l = offset(l, 1);
	 r = offset(r, 1);
      }
      break;
   case GLSL_TYPE_ARRAY:
      for (unsigned int i = 0; i < type->length; i++) {
	 emit_assignment_writes(l, r, type->fields.array, predicated);
      }
      break;

   case GLSL_TYPE_STRUCT:
      for (unsigned int i = 0; i < type->length; i++) {
	 emit_assignment_writes(l, r, type->fields.structure[i].type,
				predicated);
      }
      break;

   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_IMAGE:
   case GLSL_TYPE_ATOMIC_UINT:
      break;

   case GLSL_TYPE_DOUBLE:
   case GLSL_TYPE_VOID:
   case GLSL_TYPE_ERROR:
   case GLSL_TYPE_INTERFACE:
      unreachable("not reached");
   }
}

/* If the RHS processing resulted in an instruction generating a
 * temporary value, and it would be easy to rewrite the instruction to
 * generate its result right into the LHS instead, do so.  This ends
 * up reliably removing instructions where it can be tricky to do so
 * later without real UD chain information.
 */
bool
fs_visitor::try_rewrite_rhs_to_dst(ir_assignment *ir,
                                   fs_reg dst,
                                   fs_reg src,
                                   fs_inst *pre_rhs_inst,
                                   fs_inst *last_rhs_inst)
{
   /* Only attempt if we're doing a direct assignment. */
   if (ir->condition ||
       !(ir->lhs->type->is_scalar() ||
        (ir->lhs->type->is_vector() &&
         ir->write_mask == (1 << ir->lhs->type->vector_elements) - 1)))
      return false;

   /* Make sure the last instruction generated our source reg. */
   fs_inst *modify = get_instruction_generating_reg(pre_rhs_inst,
						    last_rhs_inst,
						    src);
   if (!modify)
      return false;

   /* If last_rhs_inst wrote a different number of components than our LHS,
    * we can't safely rewrite it.
    */
   if (alloc.sizes[dst.reg] != modify->regs_written)
      return false;

   /* Success!  Rewrite the instruction. */
   modify->dst = dst;

   return true;
}

void
fs_visitor::visit(ir_assignment *ir)
{
   fs_reg l, r;
   fs_inst *inst;

   /* FINISHME: arrays on the lhs */
   ir->lhs->accept(this);
   l = this->result;

   fs_inst *pre_rhs_inst = (fs_inst *) this->instructions.get_tail();

   ir->rhs->accept(this);
   r = this->result;

   fs_inst *last_rhs_inst = (fs_inst *) this->instructions.get_tail();

   assert(l.file != BAD_FILE);
   assert(r.file != BAD_FILE);

   if (try_rewrite_rhs_to_dst(ir, l, r, pre_rhs_inst, last_rhs_inst))
      return;

   if (ir->condition) {
      emit_bool_to_cond_code(ir->condition);
   }

   if (ir->lhs->type->is_scalar() ||
       ir->lhs->type->is_vector()) {
      for (int i = 0; i < ir->lhs->type->vector_elements; i++) {
	 if (ir->write_mask & (1 << i)) {
	    inst = emit(MOV(l, r));
	    if (ir->condition)
	       inst->predicate = BRW_PREDICATE_NORMAL;
	    r = offset(r, 1);
	 }
	 l = offset(l, 1);
      }
   } else {
      emit_assignment_writes(l, r, ir->lhs->type, ir->condition != NULL);
   }
}

fs_inst *
fs_visitor::emit_texture_gen4(ir_texture_opcode op, fs_reg dst,
                              fs_reg coordinate, int coord_components,
                              fs_reg shadow_c,
                              fs_reg lod, fs_reg dPdy, int grad_components,
                              uint32_t sampler)
{
   int mlen;
   int base_mrf = 1;
   bool simd16 = false;
   fs_reg orig_dst;

   /* g0 header. */
   mlen = 1;

   if (shadow_c.file != BAD_FILE) {
      for (int i = 0; i < coord_components; i++) {
	 emit(MOV(fs_reg(MRF, base_mrf + mlen + i), coordinate));
	 coordinate = offset(coordinate, 1);
      }

      /* gen4's SIMD8 sampler always has the slots for u,v,r present.
       * the unused slots must be zeroed.
       */
      for (int i = coord_components; i < 3; i++) {
         emit(MOV(fs_reg(MRF, base_mrf + mlen + i), fs_reg(0.0f)));
      }
      mlen += 3;

      if (op == ir_tex) {
	 /* There's no plain shadow compare message, so we use shadow
	  * compare with a bias of 0.0.
	  */
	 emit(MOV(fs_reg(MRF, base_mrf + mlen), fs_reg(0.0f)));
	 mlen++;
      } else if (op == ir_txb || op == ir_txl) {
	 emit(MOV(fs_reg(MRF, base_mrf + mlen), lod));
	 mlen++;
      } else {
         unreachable("Should not get here.");
      }

      emit(MOV(fs_reg(MRF, base_mrf + mlen), shadow_c));
      mlen++;
   } else if (op == ir_tex) {
      for (int i = 0; i < coord_components; i++) {
	 emit(MOV(fs_reg(MRF, base_mrf + mlen + i), coordinate));
	 coordinate = offset(coordinate, 1);
      }
      /* zero the others. */
      for (int i = coord_components; i<3; i++) {
         emit(MOV(fs_reg(MRF, base_mrf + mlen + i), fs_reg(0.0f)));
      }
      /* gen4's SIMD8 sampler always has the slots for u,v,r present. */
      mlen += 3;
   } else if (op == ir_txd) {
      fs_reg &dPdx = lod;

      for (int i = 0; i < coord_components; i++) {
	 emit(MOV(fs_reg(MRF, base_mrf + mlen + i), coordinate));
	 coordinate = offset(coordinate, 1);
      }
      /* the slots for u and v are always present, but r is optional */
      mlen += MAX2(coord_components, 2);

      /*  P   = u, v, r
       * dPdx = dudx, dvdx, drdx
       * dPdy = dudy, dvdy, drdy
       *
       * 1-arg: Does not exist.
       *
       * 2-arg: dudx   dvdx   dudy   dvdy
       *        dPdx.x dPdx.y dPdy.x dPdy.y
       *        m4     m5     m6     m7
       *
       * 3-arg: dudx   dvdx   drdx   dudy   dvdy   drdy
       *        dPdx.x dPdx.y dPdx.z dPdy.x dPdy.y dPdy.z
       *        m5     m6     m7     m8     m9     m10
       */
      for (int i = 0; i < grad_components; i++) {
	 emit(MOV(fs_reg(MRF, base_mrf + mlen), dPdx));
	 dPdx = offset(dPdx, 1);
      }
      mlen += MAX2(grad_components, 2);

      for (int i = 0; i < grad_components; i++) {
	 emit(MOV(fs_reg(MRF, base_mrf + mlen), dPdy));
	 dPdy = offset(dPdy, 1);
      }
      mlen += MAX2(grad_components, 2);
   } else if (op == ir_txs) {
      /* There's no SIMD8 resinfo message on Gen4.  Use SIMD16 instead. */
      simd16 = true;
      emit(MOV(fs_reg(MRF, base_mrf + mlen, BRW_REGISTER_TYPE_UD), lod));
      mlen += 2;
   } else {
      /* Oh joy.  gen4 doesn't have SIMD8 non-shadow-compare bias/lod
       * instructions.  We'll need to do SIMD16 here.
       */
      simd16 = true;
      assert(op == ir_txb || op == ir_txl || op == ir_txf);

      for (int i = 0; i < coord_components; i++) {
	 emit(MOV(fs_reg(MRF, base_mrf + mlen + i * 2, coordinate.type),
                  coordinate));
	 coordinate = offset(coordinate, 1);
      }

      /* Initialize the rest of u/v/r with 0.0.  Empirically, this seems to
       * be necessary for TXF (ld), but seems wise to do for all messages.
       */
      for (int i = coord_components; i < 3; i++) {
	 emit(MOV(fs_reg(MRF, base_mrf + mlen + i * 2), fs_reg(0.0f)));
      }

      /* lod/bias appears after u/v/r. */
      mlen += 6;

      emit(MOV(fs_reg(MRF, base_mrf + mlen, lod.type), lod));
      mlen++;

      /* The unused upper half. */
      mlen++;
   }

   if (simd16) {
      /* Now, since we're doing simd16, the return is 2 interleaved
       * vec4s where the odd-indexed ones are junk. We'll need to move
       * this weirdness around to the expected layout.
       */
      orig_dst = dst;
      dst = fs_reg(GRF, alloc.allocate(8), orig_dst.type);
   }

   enum opcode opcode;
   switch (op) {
   case ir_tex: opcode = SHADER_OPCODE_TEX; break;
   case ir_txb: opcode = FS_OPCODE_TXB; break;
   case ir_txl: opcode = SHADER_OPCODE_TXL; break;
   case ir_txd: opcode = SHADER_OPCODE_TXD; break;
   case ir_txs: opcode = SHADER_OPCODE_TXS; break;
   case ir_txf: opcode = SHADER_OPCODE_TXF; break;
   default:
      unreachable("not reached");
   }

   fs_inst *inst = emit(opcode, dst, reg_undef, fs_reg(sampler));
   inst->base_mrf = base_mrf;
   inst->mlen = mlen;
   inst->header_size = 1;
   inst->regs_written = simd16 ? 8 : 4;

   if (simd16) {
      for (int i = 0; i < 4; i++) {
	 emit(MOV(orig_dst, dst));
	 orig_dst = offset(orig_dst, 1);
	 dst = offset(dst, 2);
      }
   }

   return inst;
}

fs_inst *
fs_visitor::emit_texture_gen4_simd16(ir_texture_opcode op, fs_reg dst,
                                     fs_reg coordinate, int vector_elements,
                                     fs_reg shadow_c, fs_reg lod,
                                     uint32_t sampler)
{
   fs_reg message(MRF, 2, BRW_REGISTER_TYPE_F, dispatch_width);
   bool has_lod = op == ir_txl || op == ir_txb || op == ir_txf;

   if (has_lod && shadow_c.file != BAD_FILE)
      no16("TXB and TXL with shadow comparison unsupported in SIMD16.");

   if (op == ir_txd)
      no16("textureGrad unsupported in SIMD16.");

   /* Copy the coordinates. */
   for (int i = 0; i < vector_elements; i++) {
      emit(MOV(retype(offset(message, i), coordinate.type), coordinate));
      coordinate = offset(coordinate, 1);
   }

   fs_reg msg_end = offset(message, vector_elements);

   /* Messages other than sample and ld require all three components */
   if (has_lod || shadow_c.file != BAD_FILE) {
      for (int i = vector_elements; i < 3; i++) {
         emit(MOV(offset(message, i), fs_reg(0.0f)));
      }
   }

   if (has_lod) {
      fs_reg msg_lod = retype(offset(message, 3), op == ir_txf ?
                              BRW_REGISTER_TYPE_UD : BRW_REGISTER_TYPE_F);
      emit(MOV(msg_lod, lod));
      msg_end = offset(msg_lod, 1);
   }

   if (shadow_c.file != BAD_FILE) {
      fs_reg msg_ref = offset(message, 3 + has_lod);
      emit(MOV(msg_ref, shadow_c));
      msg_end = offset(msg_ref, 1);
   }

   enum opcode opcode;
   switch (op) {
   case ir_tex: opcode = SHADER_OPCODE_TEX; break;
   case ir_txb: opcode = FS_OPCODE_TXB;     break;
   case ir_txd: opcode = SHADER_OPCODE_TXD; break;
   case ir_txl: opcode = SHADER_OPCODE_TXL; break;
   case ir_txs: opcode = SHADER_OPCODE_TXS; break;
   case ir_txf: opcode = SHADER_OPCODE_TXF; break;
   default: unreachable("not reached");
   }

   fs_inst *inst = emit(opcode, dst, reg_undef, fs_reg(sampler));
   inst->base_mrf = message.reg - 1;
   inst->mlen = msg_end.reg - inst->base_mrf;
   inst->header_size = 1;
   inst->regs_written = 8;

   return inst;
}

/* gen5's sampler has slots for u, v, r, array index, then optional
 * parameters like shadow comparitor or LOD bias.  If optional
 * parameters aren't present, those base slots are optional and don't
 * need to be included in the message.
 *
 * We don't fill in the unnecessary slots regardless, which may look
 * surprising in the disassembly.
 */
fs_inst *
fs_visitor::emit_texture_gen5(ir_texture_opcode op, fs_reg dst,
                              fs_reg coordinate, int vector_elements,
                              fs_reg shadow_c,
                              fs_reg lod, fs_reg lod2, int grad_components,
                              fs_reg sample_index, uint32_t sampler,
                              bool has_offset)
{
   int reg_width = dispatch_width / 8;
   unsigned header_size = 0;

   fs_reg message(MRF, 2, BRW_REGISTER_TYPE_F, dispatch_width);
   fs_reg msg_coords = message;

   if (has_offset) {
      /* The offsets set up by the ir_texture visitor are in the
       * m1 header, so we can't go headerless.
       */
      header_size = 1;
      message.reg--;
   }

   for (int i = 0; i < vector_elements; i++) {
      emit(MOV(retype(offset(msg_coords, i), coordinate.type), coordinate));
      coordinate = offset(coordinate, 1);
   }
   fs_reg msg_end = offset(msg_coords, vector_elements);
   fs_reg msg_lod = offset(msg_coords, 4);

   if (shadow_c.file != BAD_FILE) {
      fs_reg msg_shadow = msg_lod;
      emit(MOV(msg_shadow, shadow_c));
      msg_lod = offset(msg_shadow, 1);
      msg_end = msg_lod;
   }

   enum opcode opcode;
   switch (op) {
   case ir_tex:
      opcode = SHADER_OPCODE_TEX;
      break;
   case ir_txb:
      emit(MOV(msg_lod, lod));
      msg_end = offset(msg_lod, 1);

      opcode = FS_OPCODE_TXB;
      break;
   case ir_txl:
      emit(MOV(msg_lod, lod));
      msg_end = offset(msg_lod, 1);

      opcode = SHADER_OPCODE_TXL;
      break;
   case ir_txd: {
      /**
       *  P   =  u,    v,    r
       * dPdx = dudx, dvdx, drdx
       * dPdy = dudy, dvdy, drdy
       *
       * Load up these values:
       * - dudx   dudy   dvdx   dvdy   drdx   drdy
       * - dPdx.x dPdy.x dPdx.y dPdy.y dPdx.z dPdy.z
       */
      msg_end = msg_lod;
      for (int i = 0; i < grad_components; i++) {
         emit(MOV(msg_end, lod));
         lod = offset(lod, 1);
         msg_end = offset(msg_end, 1);

         emit(MOV(msg_end, lod2));
         lod2 = offset(lod2, 1);
         msg_end = offset(msg_end, 1);
      }

      opcode = SHADER_OPCODE_TXD;
      break;
   }
   case ir_txs:
      msg_lod = retype(msg_end, BRW_REGISTER_TYPE_UD);
      emit(MOV(msg_lod, lod));
      msg_end = offset(msg_lod, 1);

      opcode = SHADER_OPCODE_TXS;
      break;
   case ir_query_levels:
      msg_lod = msg_end;
      emit(MOV(retype(msg_lod, BRW_REGISTER_TYPE_UD), fs_reg(0u)));
      msg_end = offset(msg_lod, 1);

      opcode = SHADER_OPCODE_TXS;
      break;
   case ir_txf:
      msg_lod = offset(msg_coords, 3);
      emit(MOV(retype(msg_lod, BRW_REGISTER_TYPE_UD), lod));
      msg_end = offset(msg_lod, 1);

      opcode = SHADER_OPCODE_TXF;
      break;
   case ir_txf_ms:
      msg_lod = offset(msg_coords, 3);
      /* lod */
      emit(MOV(retype(msg_lod, BRW_REGISTER_TYPE_UD), fs_reg(0u)));
      /* sample index */
      emit(MOV(retype(offset(msg_lod, 1), BRW_REGISTER_TYPE_UD), sample_index));
      msg_end = offset(msg_lod, 2);

      opcode = SHADER_OPCODE_TXF_CMS;
      break;
   case ir_lod:
      opcode = SHADER_OPCODE_LOD;
      break;
   case ir_tg4:
      opcode = SHADER_OPCODE_TG4;
      break;
   default:
      unreachable("not reached");
   }

   fs_inst *inst = emit(opcode, dst, reg_undef, fs_reg(sampler));
   inst->base_mrf = message.reg;
   inst->mlen = msg_end.reg - message.reg;
   inst->header_size = header_size;
   inst->regs_written = 4 * reg_width;

   if (inst->mlen > MAX_SAMPLER_MESSAGE_SIZE) {
      fail("Message length >" STRINGIFY(MAX_SAMPLER_MESSAGE_SIZE)
           " disallowed by hardware\n");
   }

   return inst;
}

static bool
is_high_sampler(const struct brw_device_info *devinfo, fs_reg sampler)
{
   if (devinfo->gen < 8 && !devinfo->is_haswell)
      return false;

   return sampler.file != IMM || sampler.fixed_hw_reg.dw1.ud >= 16;
}

fs_inst *
fs_visitor::emit_texture_gen7(ir_texture_opcode op, fs_reg dst,
                              fs_reg coordinate, int coord_components,
                              fs_reg shadow_c,
                              fs_reg lod, fs_reg lod2, int grad_components,
                              fs_reg sample_index, fs_reg mcs, fs_reg sampler,
                              fs_reg offset_value)
{
   int reg_width = dispatch_width / 8;
   unsigned header_size = 0;

   fs_reg *sources = ralloc_array(mem_ctx, fs_reg, MAX_SAMPLER_MESSAGE_SIZE);
   for (int i = 0; i < MAX_SAMPLER_MESSAGE_SIZE; i++) {
      sources[i] = vgrf(glsl_type::float_type);
   }
   int length = 0;

   if (op == ir_tg4 || offset_value.file != BAD_FILE ||
       is_high_sampler(devinfo, sampler)) {
      /* For general texture offsets (no txf workaround), we need a header to
       * put them in.  Note that for SIMD16 we're making space for two actual
       * hardware registers here, so the emit will have to fix up for this.
       *
       * * ir4_tg4 needs to place its channel select in the header,
       * for interaction with ARB_texture_swizzle
       *
       * The sampler index is only 4-bits, so for larger sampler numbers we
       * need to offset the Sampler State Pointer in the header.
       */
      header_size = 1;
      sources[0] = fs_reg(GRF, alloc.allocate(1), BRW_REGISTER_TYPE_UD);
      length++;
   }

   if (shadow_c.file != BAD_FILE) {
      emit(MOV(sources[length], shadow_c));
      length++;
   }

   bool has_nonconstant_offset =
      offset_value.file != BAD_FILE && offset_value.file != IMM;
   bool coordinate_done = false;

   /* The sampler can only meaningfully compute LOD for fragment shader
    * messages. For all other stages, we change the opcode to ir_txl and
    * hardcode the LOD to 0.
    */
   if (stage != MESA_SHADER_FRAGMENT && op == ir_tex) {
      op = ir_txl;
      lod = fs_reg(0.0f);
   }

   /* Set up the LOD info */
   switch (op) {
   case ir_tex:
   case ir_lod:
      break;
   case ir_txb:
      emit(MOV(sources[length], lod));
      length++;
      break;
   case ir_txl:
      emit(MOV(sources[length], lod));
      length++;
      break;
   case ir_txd: {
      no16("Gen7 does not support sample_d/sample_d_c in SIMD16 mode.");

      /* Load dPdx and the coordinate together:
       * [hdr], [ref], x, dPdx.x, dPdy.x, y, dPdx.y, dPdy.y, z, dPdx.z, dPdy.z
       */
      for (int i = 0; i < coord_components; i++) {
	 emit(MOV(sources[length], coordinate));
	 coordinate = offset(coordinate, 1);
	 length++;

         /* For cube map array, the coordinate is (u,v,r,ai) but there are
          * only derivatives for (u, v, r).
          */
         if (i < grad_components) {
            emit(MOV(sources[length], lod));
            lod = offset(lod, 1);
            length++;

            emit(MOV(sources[length], lod2));
            lod2 = offset(lod2, 1);
            length++;
         }
      }

      coordinate_done = true;
      break;
   }
   case ir_txs:
      emit(MOV(retype(sources[length], BRW_REGISTER_TYPE_UD), lod));
      length++;
      break;
   case ir_query_levels:
      emit(MOV(retype(sources[length], BRW_REGISTER_TYPE_UD), fs_reg(0u)));
      length++;
      break;
   case ir_txf:
      /* Unfortunately, the parameters for LD are intermixed: u, lod, v, r.
       * On Gen9 they are u, v, lod, r
       */

      emit(MOV(retype(sources[length], BRW_REGISTER_TYPE_D), coordinate));
      coordinate = offset(coordinate, 1);
      length++;

      if (devinfo->gen >= 9) {
         if (coord_components >= 2) {
            emit(MOV(retype(sources[length], BRW_REGISTER_TYPE_D), coordinate));
            coordinate = offset(coordinate, 1);
         }
         length++;
      }

      emit(MOV(retype(sources[length], BRW_REGISTER_TYPE_D), lod));
      length++;

      for (int i = devinfo->gen >= 9 ? 2 : 1; i < coord_components; i++) {
	 emit(MOV(retype(sources[length], BRW_REGISTER_TYPE_D), coordinate));
	 coordinate = offset(coordinate, 1);
	 length++;
      }

      coordinate_done = true;
      break;
   case ir_txf_ms:
      emit(MOV(retype(sources[length], BRW_REGISTER_TYPE_UD), sample_index));
      length++;

      /* data from the multisample control surface */
      emit(MOV(retype(sources[length], BRW_REGISTER_TYPE_UD), mcs));
      length++;

      /* there is no offsetting for this message; just copy in the integer
       * texture coordinates
       */
      for (int i = 0; i < coord_components; i++) {
         emit(MOV(retype(sources[length], BRW_REGISTER_TYPE_D), coordinate));
         coordinate = offset(coordinate, 1);
         length++;
      }

      coordinate_done = true;
      break;
   case ir_tg4:
      if (has_nonconstant_offset) {
         if (shadow_c.file != BAD_FILE)
            no16("Gen7 does not support gather4_po_c in SIMD16 mode.");

         /* More crazy intermixing */
         for (int i = 0; i < 2; i++) { /* u, v */
            emit(MOV(sources[length], coordinate));
            coordinate = offset(coordinate, 1);
            length++;
         }

         for (int i = 0; i < 2; i++) { /* offu, offv */
            emit(MOV(retype(sources[length], BRW_REGISTER_TYPE_D), offset_value));
            offset_value = offset(offset_value, 1);
            length++;
         }

         if (coord_components == 3) { /* r if present */
            emit(MOV(sources[length], coordinate));
            coordinate = offset(coordinate, 1);
            length++;
         }

         coordinate_done = true;
      }
      break;
   }

   /* Set up the coordinate (except for cases where it was done above) */
   if (!coordinate_done) {
      for (int i = 0; i < coord_components; i++) {
         emit(MOV(sources[length], coordinate));
         coordinate = offset(coordinate, 1);
         length++;
      }
   }

   int mlen;
   if (reg_width == 2)
      mlen = length * reg_width - header_size;
   else
      mlen = length * reg_width;

   fs_reg src_payload = fs_reg(GRF, alloc.allocate(mlen),
                               BRW_REGISTER_TYPE_F, dispatch_width);
   emit(LOAD_PAYLOAD(src_payload, sources, length, header_size));

   /* Generate the SEND */
   enum opcode opcode;
   switch (op) {
   case ir_tex: opcode = SHADER_OPCODE_TEX; break;
   case ir_txb: opcode = FS_OPCODE_TXB; break;
   case ir_txl: opcode = SHADER_OPCODE_TXL; break;
   case ir_txd: opcode = SHADER_OPCODE_TXD; break;
   case ir_txf: opcode = SHADER_OPCODE_TXF; break;
   case ir_txf_ms: opcode = SHADER_OPCODE_TXF_CMS; break;
   case ir_txs: opcode = SHADER_OPCODE_TXS; break;
   case ir_query_levels: opcode = SHADER_OPCODE_TXS; break;
   case ir_lod: opcode = SHADER_OPCODE_LOD; break;
   case ir_tg4:
      if (has_nonconstant_offset)
         opcode = SHADER_OPCODE_TG4_OFFSET;
      else
         opcode = SHADER_OPCODE_TG4;
      break;
   default:
      unreachable("not reached");
   }
   fs_inst *inst = emit(opcode, dst, src_payload, sampler);
   inst->base_mrf = -1;
   inst->mlen = mlen;
   inst->header_size = header_size;
   inst->regs_written = 4 * reg_width;

   if (inst->mlen > MAX_SAMPLER_MESSAGE_SIZE) {
      fail("Message length >" STRINGIFY(MAX_SAMPLER_MESSAGE_SIZE)
           " disallowed by hardware\n");
   }

   return inst;
}

fs_reg
fs_visitor::rescale_texcoord(fs_reg coordinate, int coord_components,
                             bool is_rect, uint32_t sampler, int texunit)
{
   fs_inst *inst = NULL;
   bool needs_gl_clamp = true;
   fs_reg scale_x, scale_y;

   /* The 965 requires the EU to do the normalization of GL rectangle
    * texture coordinates.  We use the program parameter state
    * tracking to get the scaling factor.
    */
   if (is_rect &&
       (devinfo->gen < 6 ||
        (devinfo->gen >= 6 && (key_tex->gl_clamp_mask[0] & (1 << sampler) ||
                               key_tex->gl_clamp_mask[1] & (1 << sampler))))) {
      struct gl_program_parameter_list *params = prog->Parameters;
      int tokens[STATE_LENGTH] = {
	 STATE_INTERNAL,
	 STATE_TEXRECT_SCALE,
	 texunit,
	 0,
	 0
      };

      no16("rectangle scale uniform setup not supported on SIMD16\n");
      if (dispatch_width == 16) {
	 return coordinate;
      }

      GLuint index = _mesa_add_state_reference(params,
					       (gl_state_index *)tokens);
      /* Try to find existing copies of the texrect scale uniforms. */
      for (unsigned i = 0; i < uniforms; i++) {
         if (stage_prog_data->param[i] ==
             &prog->Parameters->ParameterValues[index][0]) {
            scale_x = fs_reg(UNIFORM, i);
            scale_y = fs_reg(UNIFORM, i + 1);
            break;
         }
      }

      /* If we didn't already set them up, do so now. */
      if (scale_x.file == BAD_FILE) {
         scale_x = fs_reg(UNIFORM, uniforms);
         scale_y = fs_reg(UNIFORM, uniforms + 1);

         stage_prog_data->param[uniforms++] =
            &prog->Parameters->ParameterValues[index][0];
         stage_prog_data->param[uniforms++] =
            &prog->Parameters->ParameterValues[index][1];
      }
   }

   /* The 965 requires the EU to do the normalization of GL rectangle
    * texture coordinates.  We use the program parameter state
    * tracking to get the scaling factor.
    */
   if (devinfo->gen < 6 && is_rect) {
      fs_reg dst = fs_reg(GRF, alloc.allocate(coord_components));
      fs_reg src = coordinate;
      coordinate = dst;

      emit(MUL(dst, src, scale_x));
      dst = offset(dst, 1);
      src = offset(src, 1);
      emit(MUL(dst, src, scale_y));
   } else if (is_rect) {
      /* On gen6+, the sampler handles the rectangle coordinates
       * natively, without needing rescaling.  But that means we have
       * to do GL_CLAMP clamping at the [0, width], [0, height] scale,
       * not [0, 1] like the default case below.
       */
      needs_gl_clamp = false;

      for (int i = 0; i < 2; i++) {
	 if (key_tex->gl_clamp_mask[i] & (1 << sampler)) {
	    fs_reg chan = coordinate;
	    chan = offset(chan, i);

	    inst = emit(BRW_OPCODE_SEL, chan, chan, fs_reg(0.0f));
	    inst->conditional_mod = BRW_CONDITIONAL_GE;

	    /* Our parameter comes in as 1.0/width or 1.0/height,
	     * because that's what people normally want for doing
	     * texture rectangle handling.  We need width or height
	     * for clamping, but we don't care enough to make a new
	     * parameter type, so just invert back.
	     */
	    fs_reg limit = vgrf(glsl_type::float_type);
	    emit(MOV(limit, i == 0 ? scale_x : scale_y));
	    emit(SHADER_OPCODE_RCP, limit, limit);

	    inst = emit(BRW_OPCODE_SEL, chan, chan, limit);
	    inst->conditional_mod = BRW_CONDITIONAL_L;
	 }
      }
   }

   if (coord_components > 0 && needs_gl_clamp) {
      for (int i = 0; i < MIN2(coord_components, 3); i++) {
	 if (key_tex->gl_clamp_mask[i] & (1 << sampler)) {
	    fs_reg chan = coordinate;
	    chan = offset(chan, i);

	    fs_inst *inst = emit(MOV(chan, chan));
	    inst->saturate = true;
	 }
      }
   }
   return coordinate;
}

/* Sample from the MCS surface attached to this multisample texture. */
fs_reg
fs_visitor::emit_mcs_fetch(fs_reg coordinate, int components, fs_reg sampler)
{
   int reg_width = dispatch_width / 8;
   fs_reg payload = fs_reg(GRF, alloc.allocate(components * reg_width),
                           BRW_REGISTER_TYPE_F, dispatch_width);
   fs_reg dest = vgrf(glsl_type::uvec4_type);
   fs_reg *sources = ralloc_array(mem_ctx, fs_reg, components);

   /* parameters are: u, v, r; missing parameters are treated as zero */
   for (int i = 0; i < components; i++) {
      sources[i] = vgrf(glsl_type::float_type);
      emit(MOV(retype(sources[i], BRW_REGISTER_TYPE_D), coordinate));
      coordinate = offset(coordinate, 1);
   }

   emit(LOAD_PAYLOAD(payload, sources, components, 0));

   fs_inst *inst = emit(SHADER_OPCODE_TXF_MCS, dest, payload, sampler);
   inst->base_mrf = -1;
   inst->mlen = components * reg_width;
   inst->header_size = 0;
   inst->regs_written = 4 * reg_width; /* we only care about one reg of
                                        * response, but the sampler always
                                        * writes 4/8
                                        */

   return dest;
}

void
fs_visitor::emit_texture(ir_texture_opcode op,
                         const glsl_type *dest_type,
                         fs_reg coordinate, int coord_components,
                         fs_reg shadow_c,
                         fs_reg lod, fs_reg lod2, int grad_components,
                         fs_reg sample_index,
                         fs_reg offset_value,
                         fs_reg mcs,
                         int gather_component,
                         bool is_cube_array,
                         bool is_rect,
                         uint32_t sampler,
                         fs_reg sampler_reg, int texunit)
{
   fs_inst *inst = NULL;

   if (op == ir_tg4) {
      /* When tg4 is used with the degenerate ZERO/ONE swizzles, don't bother
       * emitting anything other than setting up the constant result.
       */
      int swiz = GET_SWZ(key_tex->swizzles[sampler], gather_component);
      if (swiz == SWIZZLE_ZERO || swiz == SWIZZLE_ONE) {

         fs_reg res = vgrf(glsl_type::vec4_type);
         this->result = res;

         for (int i=0; i<4; i++) {
            emit(MOV(res, fs_reg(swiz == SWIZZLE_ZERO ? 0.0f : 1.0f)));
            res = offset(res, 1);
         }
         return;
      }
   }

   if (coordinate.file != BAD_FILE) {
      /* FINISHME: Texture coordinate rescaling doesn't work with non-constant
       * samplers.  This should only be a problem with GL_CLAMP on Gen7.
       */
      coordinate = rescale_texcoord(coordinate, coord_components, is_rect,
                                    sampler, texunit);
   }

   /* Writemasking doesn't eliminate channels on SIMD8 texture
    * samples, so don't worry about them.
    */
   fs_reg dst = vgrf(glsl_type::get_instance(dest_type->base_type, 4, 1));

   if (devinfo->gen >= 7) {
      inst = emit_texture_gen7(op, dst, coordinate, coord_components,
                               shadow_c, lod, lod2, grad_components,
                               sample_index, mcs, sampler_reg,
                               offset_value);
   } else if (devinfo->gen >= 5) {
      inst = emit_texture_gen5(op, dst, coordinate, coord_components,
                               shadow_c, lod, lod2, grad_components,
                               sample_index, sampler,
                               offset_value.file != BAD_FILE);
   } else if (dispatch_width == 16) {
      inst = emit_texture_gen4_simd16(op, dst, coordinate, coord_components,
                                      shadow_c, lod, sampler);
   } else {
      inst = emit_texture_gen4(op, dst, coordinate, coord_components,
                               shadow_c, lod, lod2, grad_components,
                               sampler);
   }

   if (shadow_c.file != BAD_FILE)
      inst->shadow_compare = true;

   if (offset_value.file == IMM)
      inst->offset = offset_value.fixed_hw_reg.dw1.ud;

   if (op == ir_tg4) {
      inst->offset |=
         gather_channel(gather_component, sampler) << 16; /* M0.2:16-17 */

      if (devinfo->gen == 6)
         emit_gen6_gather_wa(key_tex->gen6_gather_wa[sampler], dst);
   }

   /* fixup #layers for cube map arrays */
   if (op == ir_txs && is_cube_array) {
      fs_reg depth = offset(dst, 2);
      fs_reg fixed_depth = vgrf(glsl_type::int_type);
      emit_math(SHADER_OPCODE_INT_QUOTIENT, fixed_depth, depth, fs_reg(6));

      fs_reg *fixed_payload = ralloc_array(mem_ctx, fs_reg, inst->regs_written);
      int components = inst->regs_written / (dst.width / 8);
      for (int i = 0; i < components; i++) {
         if (i == 2) {
            fixed_payload[i] = fixed_depth;
         } else {
            fixed_payload[i] = offset(dst, i);
         }
      }
      emit(LOAD_PAYLOAD(dst, fixed_payload, components, 0));
   }

   swizzle_result(op, dest_type->vector_elements, dst, sampler);
}

void
fs_visitor::visit(ir_texture *ir)
{
   uint32_t sampler =
      _mesa_get_sampler_uniform_value(ir->sampler, shader_prog, prog);

   ir_rvalue *nonconst_sampler_index =
      _mesa_get_sampler_array_nonconst_index(ir->sampler);

   /* Handle non-constant sampler array indexing */
   fs_reg sampler_reg;
   if (nonconst_sampler_index) {
      /* The highest sampler which may be used by this operation is
       * the last element of the array. Mark it here, because the generator
       * doesn't have enough information to determine the bound.
       */
      uint32_t array_size = ir->sampler->as_dereference_array()
         ->array->type->array_size();

      uint32_t max_used = sampler + array_size - 1;
      if (ir->op == ir_tg4 && devinfo->gen < 8) {
         max_used += stage_prog_data->binding_table.gather_texture_start;
      } else {
         max_used += stage_prog_data->binding_table.texture_start;
      }

      brw_mark_surface_used(prog_data, max_used);

      /* Emit code to evaluate the actual indexing expression */
      nonconst_sampler_index->accept(this);
      fs_reg temp = vgrf(glsl_type::uint_type);
      emit(ADD(temp, this->result, fs_reg(sampler)));
      emit_uniformize(temp, temp);

      sampler_reg = temp;
   } else {
      /* Single sampler, or constant array index; the indexing expression
       * is just an immediate.
       */
      sampler_reg = fs_reg(sampler);
   }

   /* FINISHME: We're failing to recompile our programs when the sampler is
    * updated.  This only matters for the texture rectangle scale parameters
    * (pre-gen6, or gen6+ with GL_CLAMP).
    */
   int texunit = prog->SamplerUnits[sampler];

   /* Should be lowered by do_lower_texture_projection */
   assert(!ir->projector);

   /* Should be lowered */
   assert(!ir->offset || !ir->offset->type->is_array());

   /* Generate code to compute all the subexpression trees.  This has to be
    * done before loading any values into MRFs for the sampler message since
    * generating these values may involve SEND messages that need the MRFs.
    */
   fs_reg coordinate;
   int coord_components = 0;
   if (ir->coordinate) {
      coord_components = ir->coordinate->type->vector_elements;
      ir->coordinate->accept(this);
      coordinate = this->result;
   }

   fs_reg shadow_comparitor;
   if (ir->shadow_comparitor) {
      ir->shadow_comparitor->accept(this);
      shadow_comparitor = this->result;
   }

   fs_reg offset_value;
   if (ir->offset) {
      ir_constant *const_offset = ir->offset->as_constant();
      if (const_offset) {
         /* Store the header bitfield in an IMM register.  This allows us to
          * use offset_value.file to distinguish between no offset, a constant
          * offset, and a non-constant offset.
          */
         offset_value =
            fs_reg(brw_texture_offset(const_offset->value.i,
                                      const_offset->type->vector_elements));
      } else {
         ir->offset->accept(this);
         offset_value = this->result;
      }
   }

   fs_reg lod, lod2, sample_index, mcs;
   int grad_components = 0;
   switch (ir->op) {
   case ir_tex:
   case ir_lod:
   case ir_tg4:
   case ir_query_levels:
      break;
   case ir_txb:
      ir->lod_info.bias->accept(this);
      lod = this->result;
      break;
   case ir_txd:
      ir->lod_info.grad.dPdx->accept(this);
      lod = this->result;

      ir->lod_info.grad.dPdy->accept(this);
      lod2 = this->result;

      grad_components = ir->lod_info.grad.dPdx->type->vector_elements;
      break;
   case ir_txf:
   case ir_txl:
   case ir_txs:
      ir->lod_info.lod->accept(this);
      lod = this->result;
      break;
   case ir_txf_ms:
      ir->lod_info.sample_index->accept(this);
      sample_index = this->result;

      if (devinfo->gen >= 7 &&
          key_tex->compressed_multisample_layout_mask & (1 << sampler)) {
         mcs = emit_mcs_fetch(coordinate, ir->coordinate->type->vector_elements,
                              sampler_reg);
      } else {
         mcs = fs_reg(0u);
      }
      break;
   default:
      unreachable("Unrecognized texture opcode");
   };

   int gather_component = 0;
   if (ir->op == ir_tg4)
      gather_component = ir->lod_info.component->as_constant()->value.i[0];

   bool is_rect =
      ir->sampler->type->sampler_dimensionality == GLSL_SAMPLER_DIM_RECT;

   bool is_cube_array =
      ir->sampler->type->sampler_dimensionality == GLSL_SAMPLER_DIM_CUBE &&
      ir->sampler->type->sampler_array;

   emit_texture(ir->op, ir->type, coordinate, coord_components,
                shadow_comparitor, lod, lod2, grad_components,
                sample_index, offset_value, mcs,
                gather_component, is_cube_array, is_rect, sampler,
                sampler_reg, texunit);
}

/**
 * Apply workarounds for Gen6 gather with UINT/SINT
 */
void
fs_visitor::emit_gen6_gather_wa(uint8_t wa, fs_reg dst)
{
   if (!wa)
      return;

   int width = (wa & WA_8BIT) ? 8 : 16;

   for (int i = 0; i < 4; i++) {
      fs_reg dst_f = retype(dst, BRW_REGISTER_TYPE_F);
      /* Convert from UNORM to UINT */
      emit(MUL(dst_f, dst_f, fs_reg((float)((1 << width) - 1))));
      emit(MOV(dst, dst_f));

      if (wa & WA_SIGN) {
         /* Reinterpret the UINT value as a signed INT value by
          * shifting the sign bit into place, then shifting back
          * preserving sign.
          */
         emit(SHL(dst, dst, fs_reg(32 - width)));
         emit(ASR(dst, dst, fs_reg(32 - width)));
      }

      dst = offset(dst, 1);
   }
}

/**
 * Set up the gather channel based on the swizzle, for gather4.
 */
uint32_t
fs_visitor::gather_channel(int orig_chan, uint32_t sampler)
{
   int swiz = GET_SWZ(key_tex->swizzles[sampler], orig_chan);
   switch (swiz) {
      case SWIZZLE_X: return 0;
      case SWIZZLE_Y:
         /* gather4 sampler is broken for green channel on RG32F --
          * we must ask for blue instead.
          */
         if (key_tex->gather_channel_quirk_mask & (1 << sampler))
            return 2;
         return 1;
      case SWIZZLE_Z: return 2;
      case SWIZZLE_W: return 3;
      default:
         unreachable("Not reached"); /* zero, one swizzles handled already */
   }
}

/**
 * Swizzle the result of a texture result.  This is necessary for
 * EXT_texture_swizzle as well as DEPTH_TEXTURE_MODE for shadow comparisons.
 */
void
fs_visitor::swizzle_result(ir_texture_opcode op, int dest_components,
                           fs_reg orig_val, uint32_t sampler)
{
   if (op == ir_query_levels) {
      /* # levels is in .w */
      this->result = offset(orig_val, 3);
      return;
   }

   this->result = orig_val;

   /* txs,lod don't actually sample the texture, so swizzling the result
    * makes no sense.
    */
   if (op == ir_txs || op == ir_lod || op == ir_tg4)
      return;

   if (dest_components == 1) {
      /* Ignore DEPTH_TEXTURE_MODE swizzling. */
   } else if (key_tex->swizzles[sampler] != SWIZZLE_NOOP) {
      fs_reg swizzled_result = vgrf(glsl_type::vec4_type);
      swizzled_result.type = orig_val.type;

      for (int i = 0; i < 4; i++) {
	 int swiz = GET_SWZ(key_tex->swizzles[sampler], i);
	 fs_reg l = swizzled_result;
	 l = offset(l, i);

	 if (swiz == SWIZZLE_ZERO) {
	    emit(MOV(l, fs_reg(0.0f)));
	 } else if (swiz == SWIZZLE_ONE) {
	    emit(MOV(l, fs_reg(1.0f)));
	 } else {
            emit(MOV(l, offset(orig_val,
                               GET_SWZ(key_tex->swizzles[sampler], i))));
	 }
      }
      this->result = swizzled_result;
   }
}

void
fs_visitor::visit(ir_swizzle *ir)
{
   ir->val->accept(this);
   fs_reg val = this->result;

   if (ir->type->vector_elements == 1) {
      this->result = offset(this->result, ir->mask.x);
      return;
   }

   fs_reg result = vgrf(ir->type);
   this->result = result;

   for (unsigned int i = 0; i < ir->type->vector_elements; i++) {
      fs_reg channel = val;
      int swiz = 0;

      switch (i) {
      case 0:
	 swiz = ir->mask.x;
	 break;
      case 1:
	 swiz = ir->mask.y;
	 break;
      case 2:
	 swiz = ir->mask.z;
	 break;
      case 3:
	 swiz = ir->mask.w;
	 break;
      }

      emit(MOV(result, offset(channel, swiz)));
      result = offset(result, 1);
   }
}

void
fs_visitor::visit(ir_discard *ir)
{
   /* We track our discarded pixels in f0.1.  By predicating on it, we can
    * update just the flag bits that aren't yet discarded.  If there's no
    * condition, we emit a CMP of g0 != g0, so all currently executing
    * channels will get turned off.
    */
   fs_inst *cmp;
   if (ir->condition) {
      emit_bool_to_cond_code(ir->condition);
      cmp = (fs_inst *) this->instructions.get_tail();
      cmp->conditional_mod = brw_negate_cmod(cmp->conditional_mod);
   } else {
      fs_reg some_reg = fs_reg(retype(brw_vec8_grf(0, 0),
                                      BRW_REGISTER_TYPE_UW));
      cmp = emit(CMP(reg_null_f, some_reg, some_reg, BRW_CONDITIONAL_NZ));
   }
   cmp->predicate = BRW_PREDICATE_NORMAL;
   cmp->flag_subreg = 1;

   if (devinfo->gen >= 6) {
      emit_discard_jump();
   }
}

void
fs_visitor::visit(ir_constant *ir)
{
   /* Set this->result to reg at the bottom of the function because some code
    * paths will cause this visitor to be applied to other fields.  This will
    * cause the value stored in this->result to be modified.
    *
    * Make reg constant so that it doesn't get accidentally modified along the
    * way.  Yes, I actually had this problem. :(
    */
   const fs_reg reg = vgrf(ir->type);
   fs_reg dst_reg = reg;

   if (ir->type->is_array()) {
      const unsigned size = type_size(ir->type->fields.array);

      for (unsigned i = 0; i < ir->type->length; i++) {
	 ir->array_elements[i]->accept(this);
	 fs_reg src_reg = this->result;

	 dst_reg.type = src_reg.type;
	 for (unsigned j = 0; j < size; j++) {
	    emit(MOV(dst_reg, src_reg));
	    src_reg = offset(src_reg, 1);
	    dst_reg = offset(dst_reg, 1);
	 }
      }
   } else if (ir->type->is_record()) {
      foreach_in_list(ir_constant, field, &ir->components) {
	 const unsigned size = type_size(field->type);

	 field->accept(this);
	 fs_reg src_reg = this->result;

	 dst_reg.type = src_reg.type;
	 for (unsigned j = 0; j < size; j++) {
	    emit(MOV(dst_reg, src_reg));
	    src_reg = offset(src_reg, 1);
	    dst_reg = offset(dst_reg, 1);
	 }
      }
   } else {
      const unsigned size = type_size(ir->type);

      for (unsigned i = 0; i < size; i++) {
	 switch (ir->type->base_type) {
	 case GLSL_TYPE_FLOAT:
	    emit(MOV(dst_reg, fs_reg(ir->value.f[i])));
	    break;
	 case GLSL_TYPE_UINT:
	    emit(MOV(dst_reg, fs_reg(ir->value.u[i])));
	    break;
	 case GLSL_TYPE_INT:
	    emit(MOV(dst_reg, fs_reg(ir->value.i[i])));
	    break;
	 case GLSL_TYPE_BOOL:
            emit(MOV(dst_reg, fs_reg(ir->value.b[i] != 0 ? ~0 : 0)));
	    break;
	 default:
	    unreachable("Non-float/uint/int/bool constant");
	 }
	 dst_reg = offset(dst_reg, 1);
      }
   }

   this->result = reg;
}

void
fs_visitor::emit_bool_to_cond_code(ir_rvalue *ir)
{
   ir_expression *expr = ir->as_expression();

   if (!expr || expr->operation == ir_binop_ubo_load) {
      ir->accept(this);

      fs_inst *inst = emit(AND(reg_null_d, this->result, fs_reg(1)));
      inst->conditional_mod = BRW_CONDITIONAL_NZ;
      return;
   }

   fs_reg op[3];

   assert(expr->get_num_operands() <= 3);
   for (unsigned int i = 0; i < expr->get_num_operands(); i++) {
      assert(expr->operands[i]->type->is_scalar());

      expr->operands[i]->accept(this);
      op[i] = this->result;

      resolve_ud_negate(&op[i]);
   }

   emit_bool_to_cond_code_of_reg(expr, op);
}

void
fs_visitor::emit_bool_to_cond_code_of_reg(ir_expression *expr, fs_reg op[3])
{
   fs_inst *inst;

   switch (expr->operation) {
   case ir_unop_logic_not:
      inst = emit(AND(reg_null_d, op[0], fs_reg(1)));
      inst->conditional_mod = BRW_CONDITIONAL_Z;
      break;

   case ir_binop_logic_xor:
      if (devinfo->gen <= 5) {
         fs_reg temp = vgrf(expr->type);
         emit(XOR(temp, op[0], op[1]));
         inst = emit(AND(reg_null_d, temp, fs_reg(1)));
      } else {
         inst = emit(XOR(reg_null_d, op[0], op[1]));
      }
      inst->conditional_mod = BRW_CONDITIONAL_NZ;
      break;

   case ir_binop_logic_or:
      if (devinfo->gen <= 5) {
         fs_reg temp = vgrf(expr->type);
         emit(OR(temp, op[0], op[1]));
         inst = emit(AND(reg_null_d, temp, fs_reg(1)));
      } else {
         inst = emit(OR(reg_null_d, op[0], op[1]));
      }
      inst->conditional_mod = BRW_CONDITIONAL_NZ;
      break;

   case ir_binop_logic_and:
      if (devinfo->gen <= 5) {
         fs_reg temp = vgrf(expr->type);
         emit(AND(temp, op[0], op[1]));
         inst = emit(AND(reg_null_d, temp, fs_reg(1)));
      } else {
         inst = emit(AND(reg_null_d, op[0], op[1]));
      }
      inst->conditional_mod = BRW_CONDITIONAL_NZ;
      break;

   case ir_unop_f2b:
      if (devinfo->gen >= 6) {
         emit(CMP(reg_null_d, op[0], fs_reg(0.0f), BRW_CONDITIONAL_NZ));
      } else {
         inst = emit(MOV(reg_null_f, op[0]));
         inst->conditional_mod = BRW_CONDITIONAL_NZ;
      }
      break;

   case ir_unop_i2b:
      if (devinfo->gen >= 6) {
         emit(CMP(reg_null_d, op[0], fs_reg(0), BRW_CONDITIONAL_NZ));
      } else {
         inst = emit(MOV(reg_null_d, op[0]));
         inst->conditional_mod = BRW_CONDITIONAL_NZ;
      }
      break;

   case ir_binop_greater:
   case ir_binop_gequal:
   case ir_binop_less:
   case ir_binop_lequal:
   case ir_binop_equal:
   case ir_binop_all_equal:
   case ir_binop_nequal:
   case ir_binop_any_nequal:
      if (devinfo->gen <= 5) {
         resolve_bool_comparison(expr->operands[0], &op[0]);
         resolve_bool_comparison(expr->operands[1], &op[1]);
      }

      emit(CMP(reg_null_d, op[0], op[1],
               brw_conditional_for_comparison(expr->operation)));
      break;

   case ir_triop_csel: {
      /* Expand the boolean condition into the flag register. */
      inst = emit(MOV(reg_null_d, op[0]));
      inst->conditional_mod = BRW_CONDITIONAL_NZ;

      /* Select which boolean to return. */
      fs_reg temp = vgrf(expr->operands[1]->type);
      inst = emit(SEL(temp, op[1], op[2]));
      inst->predicate = BRW_PREDICATE_NORMAL;

      /* Expand the result to a condition code. */
      inst = emit(MOV(reg_null_d, temp));
      inst->conditional_mod = BRW_CONDITIONAL_NZ;
      break;
   }

   default:
      unreachable("not reached");
   }
}

/**
 * Emit a gen6 IF statement with the comparison folded into the IF
 * instruction.
 */
void
fs_visitor::emit_if_gen6(ir_if *ir)
{
   ir_expression *expr = ir->condition->as_expression();

   if (expr && expr->operation != ir_binop_ubo_load) {
      fs_reg op[3];
      fs_inst *inst;
      fs_reg temp;

      assert(expr->get_num_operands() <= 3);
      for (unsigned int i = 0; i < expr->get_num_operands(); i++) {
	 assert(expr->operands[i]->type->is_scalar());

	 expr->operands[i]->accept(this);
	 op[i] = this->result;
      }

      switch (expr->operation) {
      case ir_unop_logic_not:
         emit(IF(op[0], fs_reg(0), BRW_CONDITIONAL_Z));
         return;

      case ir_binop_logic_xor:
         emit(IF(op[0], op[1], BRW_CONDITIONAL_NZ));
         return;

      case ir_binop_logic_or:
         temp = vgrf(glsl_type::bool_type);
         emit(OR(temp, op[0], op[1]));
         emit(IF(temp, fs_reg(0), BRW_CONDITIONAL_NZ));
         return;

      case ir_binop_logic_and:
         temp = vgrf(glsl_type::bool_type);
         emit(AND(temp, op[0], op[1]));
         emit(IF(temp, fs_reg(0), BRW_CONDITIONAL_NZ));
         return;

      case ir_unop_f2b:
	 inst = emit(BRW_OPCODE_IF, reg_null_f, op[0], fs_reg(0));
	 inst->conditional_mod = BRW_CONDITIONAL_NZ;
	 return;

      case ir_unop_i2b:
	 emit(IF(op[0], fs_reg(0), BRW_CONDITIONAL_NZ));
	 return;

      case ir_binop_greater:
      case ir_binop_gequal:
      case ir_binop_less:
      case ir_binop_lequal:
      case ir_binop_equal:
      case ir_binop_all_equal:
      case ir_binop_nequal:
      case ir_binop_any_nequal:
         if (devinfo->gen <= 5) {
            resolve_bool_comparison(expr->operands[0], &op[0]);
            resolve_bool_comparison(expr->operands[1], &op[1]);
         }

	 emit(IF(op[0], op[1],
                 brw_conditional_for_comparison(expr->operation)));
	 return;

      case ir_triop_csel: {
         /* Expand the boolean condition into the flag register. */
         fs_inst *inst = emit(MOV(reg_null_d, op[0]));
         inst->conditional_mod = BRW_CONDITIONAL_NZ;

         /* Select which boolean to use as the result. */
         fs_reg temp = vgrf(expr->operands[1]->type);
         inst = emit(SEL(temp, op[1], op[2]));
         inst->predicate = BRW_PREDICATE_NORMAL;

	 emit(IF(temp, fs_reg(0), BRW_CONDITIONAL_NZ));
	 return;
      }

      default:
	 unreachable("not reached");
      }
   }

   ir->condition->accept(this);
   emit(IF(this->result, fs_reg(0), BRW_CONDITIONAL_NZ));
}

bool
fs_visitor::try_opt_frontfacing_ternary(ir_if *ir)
{
   ir_dereference_variable *deref = ir->condition->as_dereference_variable();
   if (!deref || strcmp(deref->var->name, "gl_FrontFacing") != 0)
      return false;

   if (ir->then_instructions.length() != 1 ||
       ir->else_instructions.length() != 1)
      return false;

   ir_assignment *then_assign =
         ((ir_instruction *)ir->then_instructions.head)->as_assignment();
   ir_assignment *else_assign =
         ((ir_instruction *)ir->else_instructions.head)->as_assignment();

   if (!then_assign || then_assign->condition ||
       !else_assign || else_assign->condition ||
       then_assign->write_mask != else_assign->write_mask ||
       !then_assign->lhs->equals(else_assign->lhs))
      return false;

   ir_constant *then_rhs = then_assign->rhs->as_constant();
   ir_constant *else_rhs = else_assign->rhs->as_constant();

   if (!then_rhs || !else_rhs)
      return false;

   if (then_rhs->type->base_type != GLSL_TYPE_FLOAT)
      return false;

   if ((then_rhs->is_one() && else_rhs->is_negative_one()) ||
       (else_rhs->is_one() && then_rhs->is_negative_one())) {
      then_assign->lhs->accept(this);
      fs_reg dst = this->result;
      dst.type = BRW_REGISTER_TYPE_D;
      fs_reg tmp = vgrf(glsl_type::int_type);

      if (devinfo->gen >= 6) {
         /* Bit 15 of g0.0 is 0 if the polygon is front facing. */
         fs_reg g0 = fs_reg(retype(brw_vec1_grf(0, 0), BRW_REGISTER_TYPE_W));

         /* For (gl_FrontFacing ? 1.0 : -1.0), emit:
          *
          *    or(8)  tmp.1<2>W  g0.0<0,1,0>W  0x00003f80W
          *    and(8) dst<1>D    tmp<8,8,1>D   0xbf800000D
          *
          * and negate g0.0<0,1,0>W for (gl_FrontFacing ? -1.0 : 1.0).
          */

         if (then_rhs->is_negative_one()) {
            assert(else_rhs->is_one());
            g0.negate = true;
         }

         tmp.type = BRW_REGISTER_TYPE_W;
         tmp.subreg_offset = 2;
         tmp.stride = 2;

         fs_inst *or_inst = emit(OR(tmp, g0, fs_reg(0x3f80)));
         or_inst->src[1].type = BRW_REGISTER_TYPE_UW;

         tmp.type = BRW_REGISTER_TYPE_D;
         tmp.subreg_offset = 0;
         tmp.stride = 1;
      } else {
         /* Bit 31 of g1.6 is 0 if the polygon is front facing. */
         fs_reg g1_6 = fs_reg(retype(brw_vec1_grf(1, 6), BRW_REGISTER_TYPE_D));

         /* For (gl_FrontFacing ? 1.0 : -1.0), emit:
          *
          *    or(8)  tmp<1>D  g1.6<0,1,0>D  0x3f800000D
          *    and(8) dst<1>D  tmp<8,8,1>D   0xbf800000D
          *
          * and negate g1.6<0,1,0>D for (gl_FrontFacing ? -1.0 : 1.0).
          */

         if (then_rhs->is_negative_one()) {
            assert(else_rhs->is_one());
            g1_6.negate = true;
         }

         emit(OR(tmp, g1_6, fs_reg(0x3f800000)));
      }
      emit(AND(dst, tmp, fs_reg(0xbf800000)));
      return true;
   }

   return false;
}

/**
 * Try to replace IF/MOV/ELSE/MOV/ENDIF with SEL.
 *
 * Many GLSL shaders contain the following pattern:
 *
 *    x = condition ? foo : bar
 *
 * The compiler emits an ir_if tree for this, since each subexpression might be
 * a complex tree that could have side-effects or short-circuit logic.
 *
 * However, the common case is to simply select one of two constants or
 * variable values---which is exactly what SEL is for.  In this case, the
 * assembly looks like:
 *
 *    (+f0) IF
 *    MOV dst src0
 *    ELSE
 *    MOV dst src1
 *    ENDIF
 *
 * which can be easily translated into:
 *
 *    (+f0) SEL dst src0 src1
 *
 * If src0 is an immediate value, we promote it to a temporary GRF.
 */
bool
fs_visitor::try_replace_with_sel()
{
   fs_inst *endif_inst = (fs_inst *) instructions.get_tail();
   assert(endif_inst->opcode == BRW_OPCODE_ENDIF);

   /* Pattern match in reverse: IF, MOV, ELSE, MOV, ENDIF. */
   int opcodes[] = {
      BRW_OPCODE_IF, BRW_OPCODE_MOV, BRW_OPCODE_ELSE, BRW_OPCODE_MOV,
   };

   fs_inst *match = (fs_inst *) endif_inst->prev;
   for (int i = 0; i < 4; i++) {
      if (match->is_head_sentinel() || match->opcode != opcodes[4-i-1])
         return false;
      match = (fs_inst *) match->prev;
   }

   /* The opcodes match; it looks like the right sequence of instructions. */
   fs_inst *else_mov = (fs_inst *) endif_inst->prev;
   fs_inst *then_mov = (fs_inst *) else_mov->prev->prev;
   fs_inst *if_inst = (fs_inst *) then_mov->prev;

   /* Check that the MOVs are the right form. */
   if (then_mov->dst.equals(else_mov->dst) &&
       !then_mov->is_partial_write() &&
       !else_mov->is_partial_write()) {

      /* Remove the matched instructions; we'll emit a SEL to replace them. */
      while (!if_inst->next->is_tail_sentinel())
         if_inst->next->exec_node::remove();
      if_inst->exec_node::remove();

      /* Only the last source register can be a constant, so if the MOV in
       * the "then" clause uses a constant, we need to put it in a temporary.
       */
      fs_reg src0(then_mov->src[0]);
      if (src0.file == IMM) {
         src0 = vgrf(glsl_type::float_type);
         src0.type = then_mov->src[0].type;
         emit(MOV(src0, then_mov->src[0]));
      }

      fs_inst *sel;
      if (if_inst->conditional_mod) {
         /* Sandybridge-specific IF with embedded comparison */
         emit(CMP(reg_null_d, if_inst->src[0], if_inst->src[1],
                  if_inst->conditional_mod));
         sel = emit(BRW_OPCODE_SEL, then_mov->dst, src0, else_mov->src[0]);
         sel->predicate = BRW_PREDICATE_NORMAL;
      } else {
         /* Separate CMP and IF instructions */
         sel = emit(BRW_OPCODE_SEL, then_mov->dst, src0, else_mov->src[0]);
         sel->predicate = if_inst->predicate;
         sel->predicate_inverse = if_inst->predicate_inverse;
      }

      return true;
   }

   return false;
}

void
fs_visitor::visit(ir_if *ir)
{
   if (try_opt_frontfacing_ternary(ir))
      return;

   /* Don't point the annotation at the if statement, because then it plus
    * the then and else blocks get printed.
    */
   this->base_ir = ir->condition;

   if (devinfo->gen == 6) {
      emit_if_gen6(ir);
   } else {
      emit_bool_to_cond_code(ir->condition);

      emit(IF(BRW_PREDICATE_NORMAL));
   }

   foreach_in_list(ir_instruction, ir_, &ir->then_instructions) {
      this->base_ir = ir_;
      ir_->accept(this);
   }

   if (!ir->else_instructions.is_empty()) {
      emit(BRW_OPCODE_ELSE);

      foreach_in_list(ir_instruction, ir_, &ir->else_instructions) {
	 this->base_ir = ir_;
	 ir_->accept(this);
      }
   }

   emit(BRW_OPCODE_ENDIF);

   if (!try_replace_with_sel() && devinfo->gen < 6) {
      no16("Can't support (non-uniform) control flow on SIMD16\n");
   }
}

void
fs_visitor::visit(ir_loop *ir)
{
   if (devinfo->gen < 6) {
      no16("Can't support (non-uniform) control flow on SIMD16\n");
   }

   this->base_ir = NULL;
   emit(BRW_OPCODE_DO);

   foreach_in_list(ir_instruction, ir_, &ir->body_instructions) {
      this->base_ir = ir_;
      ir_->accept(this);
   }

   this->base_ir = NULL;
   emit(BRW_OPCODE_WHILE);
}

void
fs_visitor::visit(ir_loop_jump *ir)
{
   switch (ir->mode) {
   case ir_loop_jump::jump_break:
      emit(BRW_OPCODE_BREAK);
      break;
   case ir_loop_jump::jump_continue:
      emit(BRW_OPCODE_CONTINUE);
      break;
   }
}

void
fs_visitor::visit_atomic_counter_intrinsic(ir_call *ir)
{
   ir_dereference *deref = static_cast<ir_dereference *>(
      ir->actual_parameters.get_head());
   ir_variable *location = deref->variable_referenced();
   unsigned surf_index = (stage_prog_data->binding_table.abo_start +
                          location->data.binding);

   /* Calculate the surface offset */
   fs_reg offset = vgrf(glsl_type::uint_type);
   ir_dereference_array *deref_array = deref->as_dereference_array();

   if (deref_array) {
      deref_array->array_index->accept(this);

      fs_reg tmp = vgrf(glsl_type::uint_type);
      emit(MUL(tmp, this->result, fs_reg(ATOMIC_COUNTER_SIZE)));
      emit(ADD(offset, tmp, fs_reg(location->data.atomic.offset)));
   } else {
      offset = fs_reg(location->data.atomic.offset);
   }

   /* Emit the appropriate machine instruction */
   const char *callee = ir->callee->function_name();
   ir->return_deref->accept(this);
   fs_reg dst = this->result;

   if (!strcmp("__intrinsic_atomic_read", callee)) {
      emit_untyped_surface_read(surf_index, dst, offset);

   } else if (!strcmp("__intrinsic_atomic_increment", callee)) {
      emit_untyped_atomic(BRW_AOP_INC, surf_index, dst, offset,
                          fs_reg(), fs_reg());

   } else if (!strcmp("__intrinsic_atomic_predecrement", callee)) {
      emit_untyped_atomic(BRW_AOP_PREDEC, surf_index, dst, offset,
                          fs_reg(), fs_reg());
   }
}

void
fs_visitor::visit(ir_call *ir)
{
   const char *callee = ir->callee->function_name();

   if (!strcmp("__intrinsic_atomic_read", callee) ||
       !strcmp("__intrinsic_atomic_increment", callee) ||
       !strcmp("__intrinsic_atomic_predecrement", callee)) {
      visit_atomic_counter_intrinsic(ir);
   } else {
      unreachable("Unsupported intrinsic.");
   }
}

void
fs_visitor::visit(ir_return *)
{
   unreachable("FINISHME");
}

void
fs_visitor::visit(ir_function *ir)
{
   /* Ignore function bodies other than main() -- we shouldn't see calls to
    * them since they should all be inlined before we get to ir_to_mesa.
    */
   if (strcmp(ir->name, "main") == 0) {
      const ir_function_signature *sig;
      exec_list empty;

      sig = ir->matching_signature(NULL, &empty, false);

      assert(sig);

      foreach_in_list(ir_instruction, ir_, &sig->body) {
	 this->base_ir = ir_;
	 ir_->accept(this);
      }
   }
}

void
fs_visitor::visit(ir_function_signature *)
{
   unreachable("not reached");
}

void
fs_visitor::visit(ir_emit_vertex *)
{
   unreachable("not reached");
}

void
fs_visitor::visit(ir_end_primitive *)
{
   unreachable("not reached");
}

void
fs_visitor::emit_untyped_atomic(unsigned atomic_op, unsigned surf_index,
                                fs_reg dst, fs_reg offset, fs_reg src0,
                                fs_reg src1)
{
   int reg_width = dispatch_width / 8;
   int length = 0;

   fs_reg *sources = ralloc_array(mem_ctx, fs_reg, 4);

   sources[0] = fs_reg(GRF, alloc.allocate(1), BRW_REGISTER_TYPE_UD);
   /* Initialize the sample mask in the message header. */
   emit(MOV(sources[0], fs_reg(0u)))
      ->force_writemask_all = true;

   if (stage == MESA_SHADER_FRAGMENT) {
      if (((brw_wm_prog_data*)this->prog_data)->uses_kill) {
         emit(MOV(component(sources[0], 7), brw_flag_reg(0, 1)))
            ->force_writemask_all = true;
      } else {
         emit(MOV(component(sources[0], 7),
                  retype(brw_vec1_grf(1, 7), BRW_REGISTER_TYPE_UD)))
            ->force_writemask_all = true;
      }
   } else {
      /* The execution mask is part of the side-band information sent together with
       * the message payload to the data port. It's implicitly ANDed with the sample
       * mask sent in the header to compute the actual set of channels that execute
       * the atomic operation.
       */
      assert(stage == MESA_SHADER_VERTEX || stage == MESA_SHADER_COMPUTE);
      emit(MOV(component(sources[0], 7),
               fs_reg(0xffffu)))->force_writemask_all = true;
   }
   length++;

   /* Set the atomic operation offset. */
   sources[1] = vgrf(glsl_type::uint_type);
   emit(MOV(sources[1], offset));
   length++;

   /* Set the atomic operation arguments. */
   if (src0.file != BAD_FILE) {
      sources[length] = vgrf(glsl_type::uint_type);
      emit(MOV(sources[length], src0));
      length++;
   }

   if (src1.file != BAD_FILE) {
      sources[length] = vgrf(glsl_type::uint_type);
      emit(MOV(sources[length], src1));
      length++;
   }

   int mlen = 1 + (length - 1) * reg_width;
   fs_reg src_payload = fs_reg(GRF, alloc.allocate(mlen),
                               BRW_REGISTER_TYPE_UD, dispatch_width);
   emit(LOAD_PAYLOAD(src_payload, sources, length, 1));

   /* Emit the instruction. */
   fs_inst *inst = emit(SHADER_OPCODE_UNTYPED_ATOMIC, dst, src_payload,
                        fs_reg(surf_index), fs_reg(atomic_op));
   inst->mlen = mlen;
}

void
fs_visitor::emit_untyped_surface_read(unsigned surf_index, fs_reg dst,
                                      fs_reg offset)
{
   int reg_width = dispatch_width / 8;

   fs_reg *sources = ralloc_array(mem_ctx, fs_reg, 2);

   sources[0] = fs_reg(GRF, alloc.allocate(1), BRW_REGISTER_TYPE_UD);
   /* Initialize the sample mask in the message header. */
   emit(MOV(sources[0], fs_reg(0u)))
      ->force_writemask_all = true;

   if (stage == MESA_SHADER_FRAGMENT) {
      if (((brw_wm_prog_data*)this->prog_data)->uses_kill) {
         emit(MOV(component(sources[0], 7), brw_flag_reg(0, 1)))
            ->force_writemask_all = true;
      } else {
         emit(MOV(component(sources[0], 7),
                  retype(brw_vec1_grf(1, 7), BRW_REGISTER_TYPE_UD)))
            ->force_writemask_all = true;
      }
   } else {
      /* The execution mask is part of the side-band information sent together with
       * the message payload to the data port. It's implicitly ANDed with the sample
       * mask sent in the header to compute the actual set of channels that execute
       * the atomic operation.
       */
      assert(stage == MESA_SHADER_VERTEX || stage == MESA_SHADER_COMPUTE);
      emit(MOV(component(sources[0], 7),
               fs_reg(0xffffu)))->force_writemask_all = true;
   }

   /* Set the surface read offset. */
   sources[1] = vgrf(glsl_type::uint_type);
   emit(MOV(sources[1], offset));

   int mlen = 1 + reg_width;
   fs_reg src_payload = fs_reg(GRF, alloc.allocate(mlen),
                               BRW_REGISTER_TYPE_UD, dispatch_width);
   fs_inst *inst = emit(LOAD_PAYLOAD(src_payload, sources, 2, 1));

   /* Emit the instruction. */
   inst = emit(SHADER_OPCODE_UNTYPED_SURFACE_READ, dst, src_payload,
               fs_reg(surf_index), fs_reg(1));
   inst->mlen = mlen;
}

fs_inst *
fs_visitor::emit(fs_inst *inst)
{
   if (dispatch_width == 16 && inst->exec_size == 8)
      inst->force_uncompressed = true;

   inst->annotation = this->current_annotation;
   inst->ir = this->base_ir;

   this->instructions.push_tail(inst);

   return inst;
}

void
fs_visitor::emit(exec_list list)
{
   foreach_in_list_safe(fs_inst, inst, &list) {
      inst->exec_node::remove();
      emit(inst);
   }
}

/** Emits a dummy fragment shader consisting of magenta for bringup purposes. */
void
fs_visitor::emit_dummy_fs()
{
   int reg_width = dispatch_width / 8;

   /* Everyone's favorite color. */
   const float color[4] = { 1.0, 0.0, 1.0, 0.0 };
   for (int i = 0; i < 4; i++) {
      emit(MOV(fs_reg(MRF, 2 + i * reg_width, BRW_REGISTER_TYPE_F,
                      dispatch_width), fs_reg(color[i])));
   }

   fs_inst *write;
   write = emit(FS_OPCODE_FB_WRITE);
   write->eot = true;
   if (devinfo->gen >= 6) {
      write->base_mrf = 2;
      write->mlen = 4 * reg_width;
   } else {
      write->header_size = 2;
      write->base_mrf = 0;
      write->mlen = 2 + 4 * reg_width;
   }

   /* Tell the SF we don't have any inputs.  Gen4-5 require at least one
    * varying to avoid GPU hangs, so set that.
    */
   brw_wm_prog_data *wm_prog_data = (brw_wm_prog_data *) this->prog_data;
   wm_prog_data->num_varying_inputs = devinfo->gen < 6 ? 1 : 0;
   memset(wm_prog_data->urb_setup, -1,
          sizeof(wm_prog_data->urb_setup[0]) * VARYING_SLOT_MAX);

   /* We don't have any uniforms. */
   stage_prog_data->nr_params = 0;
   stage_prog_data->nr_pull_params = 0;
   stage_prog_data->curb_read_length = 0;
   stage_prog_data->dispatch_grf_start_reg = 2;
   wm_prog_data->dispatch_grf_start_reg_16 = 2;
   grf_used = 1; /* Gen4-5 don't allow zero GRF blocks */

   calculate_cfg();
}

/* The register location here is relative to the start of the URB
 * data.  It will get adjusted to be a real location before
 * generate_code() time.
 */
struct brw_reg
fs_visitor::interp_reg(int location, int channel)
{
   assert(stage == MESA_SHADER_FRAGMENT);
   brw_wm_prog_data *prog_data = (brw_wm_prog_data*) this->prog_data;
   int regnr = prog_data->urb_setup[location] * 2 + channel / 2;
   int stride = (channel & 1) * 4;

   assert(prog_data->urb_setup[location] != -1);

   return brw_vec1_grf(regnr, stride);
}

/** Emits the interpolation for the varying inputs. */
void
fs_visitor::emit_interpolation_setup_gen4()
{
   struct brw_reg g1_uw = retype(brw_vec1_grf(1, 0), BRW_REGISTER_TYPE_UW);

   this->current_annotation = "compute pixel centers";
   this->pixel_x = vgrf(glsl_type::uint_type);
   this->pixel_y = vgrf(glsl_type::uint_type);
   this->pixel_x.type = BRW_REGISTER_TYPE_UW;
   this->pixel_y.type = BRW_REGISTER_TYPE_UW;
   emit(ADD(this->pixel_x,
            fs_reg(stride(suboffset(g1_uw, 4), 2, 4, 0)),
            fs_reg(brw_imm_v(0x10101010))));
   emit(ADD(this->pixel_y,
            fs_reg(stride(suboffset(g1_uw, 5), 2, 4, 0)),
            fs_reg(brw_imm_v(0x11001100))));

   this->current_annotation = "compute pixel deltas from v0";

   this->delta_xy[BRW_WM_PERSPECTIVE_PIXEL_BARYCENTRIC] =
      vgrf(glsl_type::vec2_type);
   const fs_reg &delta_xy = this->delta_xy[BRW_WM_PERSPECTIVE_PIXEL_BARYCENTRIC];
   const fs_reg xstart(negate(brw_vec1_grf(1, 0)));
   const fs_reg ystart(negate(brw_vec1_grf(1, 1)));

   if (devinfo->has_pln && dispatch_width == 16) {
      emit(ADD(half(offset(delta_xy, 0), 0), half(this->pixel_x, 0), xstart));
      emit(ADD(half(offset(delta_xy, 0), 1), half(this->pixel_y, 0), ystart));
      emit(ADD(half(offset(delta_xy, 1), 0), half(this->pixel_x, 1), xstart))
         ->force_sechalf = true;
      emit(ADD(half(offset(delta_xy, 1), 1), half(this->pixel_y, 1), ystart))
         ->force_sechalf = true;
   } else {
      emit(ADD(offset(delta_xy, 0), this->pixel_x, xstart));
      emit(ADD(offset(delta_xy, 1), this->pixel_y, ystart));
   }

   this->current_annotation = "compute pos.w and 1/pos.w";
   /* Compute wpos.w.  It's always in our setup, since it's needed to
    * interpolate the other attributes.
    */
   this->wpos_w = vgrf(glsl_type::float_type);
   emit(FS_OPCODE_LINTERP, wpos_w, delta_xy, interp_reg(VARYING_SLOT_POS, 3));
   /* Compute the pixel 1/W value from wpos.w. */
   this->pixel_w = vgrf(glsl_type::float_type);
   emit_math(SHADER_OPCODE_RCP, this->pixel_w, wpos_w);
   this->current_annotation = NULL;
}

/** Emits the interpolation for the varying inputs. */
void
fs_visitor::emit_interpolation_setup_gen6()
{
   struct brw_reg g1_uw = retype(brw_vec1_grf(1, 0), BRW_REGISTER_TYPE_UW);

   this->current_annotation = "compute pixel centers";
   if (brw->gen >= 8 || dispatch_width == 8) {
      /* The "Register Region Restrictions" page says for BDW (and newer,
       * presumably):
       *
       *     "When destination spans two registers, the source may be one or
       *      two registers. The destination elements must be evenly split
       *      between the two registers."
       *
       * Thus we can do a single add(16) in SIMD8 or an add(32) in SIMD16 to
       * compute our pixel centers.
       */
      fs_reg int_pixel_xy(GRF, alloc.allocate(dispatch_width / 8),
                          BRW_REGISTER_TYPE_UW, dispatch_width * 2);
      emit(ADD(int_pixel_xy,
               fs_reg(stride(suboffset(g1_uw, 4), 1, 4, 0)),
               fs_reg(brw_imm_v(0x11001010))))
         ->force_writemask_all = true;

      this->pixel_x = vgrf(glsl_type::float_type);
      this->pixel_y = vgrf(glsl_type::float_type);
      emit(FS_OPCODE_PIXEL_X, this->pixel_x, int_pixel_xy);
      emit(FS_OPCODE_PIXEL_Y, this->pixel_y, int_pixel_xy);
   } else {
      /* The "Register Region Restrictions" page says for SNB, IVB, HSW:
       *
       *     "When destination spans two registers, the source MUST span two
       *      registers."
       *
       * Since the GRF source of the ADD will only read a single register, we
       * must do two separate ADDs in SIMD16.
       */
      fs_reg int_pixel_x = vgrf(glsl_type::uint_type);
      fs_reg int_pixel_y = vgrf(glsl_type::uint_type);
      int_pixel_x.type = BRW_REGISTER_TYPE_UW;
      int_pixel_y.type = BRW_REGISTER_TYPE_UW;
      emit(ADD(int_pixel_x,
               fs_reg(stride(suboffset(g1_uw, 4), 2, 4, 0)),
               fs_reg(brw_imm_v(0x10101010))));
      emit(ADD(int_pixel_y,
               fs_reg(stride(suboffset(g1_uw, 5), 2, 4, 0)),
               fs_reg(brw_imm_v(0x11001100))));

      /* As of gen6, we can no longer mix float and int sources.  We have
       * to turn the integer pixel centers into floats for their actual
       * use.
       */
      this->pixel_x = vgrf(glsl_type::float_type);
      this->pixel_y = vgrf(glsl_type::float_type);
      emit(MOV(this->pixel_x, int_pixel_x));
      emit(MOV(this->pixel_y, int_pixel_y));
   }

   this->current_annotation = "compute pos.w";
   this->pixel_w = fs_reg(brw_vec8_grf(payload.source_w_reg, 0));
   this->wpos_w = vgrf(glsl_type::float_type);
   emit_math(SHADER_OPCODE_RCP, this->wpos_w, this->pixel_w);

   for (int i = 0; i < BRW_WM_BARYCENTRIC_INTERP_MODE_COUNT; ++i) {
      uint8_t reg = payload.barycentric_coord_reg[i];
      this->delta_xy[i] = fs_reg(brw_vec16_grf(reg, 0));
   }

   this->current_annotation = NULL;
}

void
fs_visitor::setup_color_payload(fs_reg *dst, fs_reg color, unsigned components,
                                unsigned exec_size, bool use_2nd_half)
{
   brw_wm_prog_key *key = (brw_wm_prog_key*) this->key;
   fs_inst *inst;

   if (key->clamp_fragment_color) {
      fs_reg tmp = vgrf(glsl_type::vec4_type);
      assert(color.type == BRW_REGISTER_TYPE_F);
      for (unsigned i = 0; i < components; i++) {
         inst = emit(MOV(offset(tmp, i), offset(color, i)));
         inst->saturate = true;
      }
      color = tmp;
   }

   if (exec_size < dispatch_width) {
      unsigned half_idx = use_2nd_half ? 1 : 0;
      for (unsigned i = 0; i < components; i++)
         dst[i] = half(offset(color, i), half_idx);
   } else {
      for (unsigned i = 0; i < components; i++)
         dst[i] = offset(color, i);
   }
}

static enum brw_conditional_mod
cond_for_alpha_func(GLenum func)
{
   switch(func) {
      case GL_GREATER:
         return BRW_CONDITIONAL_G;
      case GL_GEQUAL:
         return BRW_CONDITIONAL_GE;
      case GL_LESS:
         return BRW_CONDITIONAL_L;
      case GL_LEQUAL:
         return BRW_CONDITIONAL_LE;
      case GL_EQUAL:
         return BRW_CONDITIONAL_EQ;
      case GL_NOTEQUAL:
         return BRW_CONDITIONAL_NEQ;
      default:
         unreachable("Not reached");
   }
}

/**
 * Alpha test support for when we compile it into the shader instead
 * of using the normal fixed-function alpha test.
 */
void
fs_visitor::emit_alpha_test()
{
   assert(stage == MESA_SHADER_FRAGMENT);
   brw_wm_prog_key *key = (brw_wm_prog_key*) this->key;
   this->current_annotation = "Alpha test";

   fs_inst *cmp;
   if (key->alpha_test_func == GL_ALWAYS)
      return;

   if (key->alpha_test_func == GL_NEVER) {
      /* f0.1 = 0 */
      fs_reg some_reg = fs_reg(retype(brw_vec8_grf(0, 0),
                                      BRW_REGISTER_TYPE_UW));
      cmp = emit(CMP(reg_null_f, some_reg, some_reg,
                     BRW_CONDITIONAL_NEQ));
   } else {
      /* RT0 alpha */
      fs_reg color = offset(outputs[0], 3);

      /* f0.1 &= func(color, ref) */
      cmp = emit(CMP(reg_null_f, color, fs_reg(key->alpha_test_ref),
                     cond_for_alpha_func(key->alpha_test_func)));
   }
   cmp->predicate = BRW_PREDICATE_NORMAL;
   cmp->flag_subreg = 1;
}

fs_inst *
fs_visitor::emit_single_fb_write(fs_reg color0, fs_reg color1,
                                 fs_reg src0_alpha, unsigned components,
                                 unsigned exec_size, bool use_2nd_half)
{
   assert(stage == MESA_SHADER_FRAGMENT);
   brw_wm_prog_data *prog_data = (brw_wm_prog_data*) this->prog_data;
   brw_wm_prog_key *key = (brw_wm_prog_key*) this->key;

   this->current_annotation = "FB write header";
   int header_size = 2, payload_header_size;

   /* We can potentially have a message length of up to 15, so we have to set
    * base_mrf to either 0 or 1 in order to fit in m0..m15.
    */
   fs_reg *sources = ralloc_array(mem_ctx, fs_reg, 15);
   int length = 0;

   /* From the Sandy Bridge PRM, volume 4, page 198:
    *
    *     "Dispatched Pixel Enables. One bit per pixel indicating
    *      which pixels were originally enabled when the thread was
    *      dispatched. This field is only required for the end-of-
    *      thread message and on all dual-source messages."
    */
   if (devinfo->gen >= 6 &&
       (devinfo->is_haswell || devinfo->gen >= 8 || !prog_data->uses_kill) &&
       color1.file == BAD_FILE &&
       key->nr_color_regions == 1) {
      header_size = 0;
   }

   if (header_size != 0) {
      assert(header_size == 2);
      /* Allocate 2 registers for a header */
      length += 2;
   }

   if (payload.aa_dest_stencil_reg) {
      sources[length] = fs_reg(GRF, alloc.allocate(1));
      emit(MOV(sources[length],
               fs_reg(brw_vec8_grf(payload.aa_dest_stencil_reg, 0))));
      length++;
   }

   prog_data->uses_omask =
      prog->OutputsWritten & BITFIELD64_BIT(FRAG_RESULT_SAMPLE_MASK);
   if (prog_data->uses_omask) {
      this->current_annotation = "FB write oMask";
      assert(this->sample_mask.file != BAD_FILE);
      /* Hand over gl_SampleMask. Only lower 16 bits are relevant.  Since
       * it's unsinged single words, one vgrf is always 16-wide.
       */
      sources[length] = fs_reg(GRF, alloc.allocate(1),
                               BRW_REGISTER_TYPE_UW, 16);
      emit(FS_OPCODE_SET_OMASK, sources[length], this->sample_mask);
      length++;
   }

   payload_header_size = length;

   if (color0.file == BAD_FILE) {
      /* Even if there's no color buffers enabled, we still need to send
       * alpha out the pipeline to our null renderbuffer to support
       * alpha-testing, alpha-to-coverage, and so on.
       */
      if (this->outputs[0].file != BAD_FILE)
         setup_color_payload(&sources[length + 3], offset(this->outputs[0], 3),
                             1, exec_size, false);
      length += 4;
   } else if (color1.file == BAD_FILE) {
      if (src0_alpha.file != BAD_FILE) {
         setup_color_payload(&sources[length], src0_alpha, 1, exec_size, false);
         length++;
      }

      setup_color_payload(&sources[length], color0, components,
                          exec_size, use_2nd_half);
      length += 4;
   } else {
      setup_color_payload(&sources[length], color0, components,
                          exec_size, use_2nd_half);
      length += 4;
      setup_color_payload(&sources[length], color1, components,
                          exec_size, use_2nd_half);
      length += 4;
   }

   if (source_depth_to_render_target) {
      if (devinfo->gen == 6) {
	 /* For outputting oDepth on gen6, SIMD8 writes have to be
	  * used.  This would require SIMD8 moves of each half to
	  * message regs, kind of like pre-gen5 SIMD16 FB writes.
	  * Just bail on doing so for now.
	  */
	 no16("Missing support for simd16 depth writes on gen6\n");
      }

      if (prog->OutputsWritten & BITFIELD64_BIT(FRAG_RESULT_DEPTH)) {
	 /* Hand over gl_FragDepth. */
	 assert(this->frag_depth.file != BAD_FILE);
         sources[length] = this->frag_depth;
      } else {
	 /* Pass through the payload depth. */
         sources[length] = fs_reg(brw_vec8_grf(payload.source_depth_reg, 0));
      }
      length++;
   }

   if (payload.dest_depth_reg)
      sources[length++] = fs_reg(brw_vec8_grf(payload.dest_depth_reg, 0));

   fs_inst *load;
   fs_inst *write;
   if (devinfo->gen >= 7) {
      /* Send from the GRF */
      fs_reg payload = fs_reg(GRF, -1, BRW_REGISTER_TYPE_F, exec_size);
      load = emit(LOAD_PAYLOAD(payload, sources, length, payload_header_size));
      payload.reg = alloc.allocate(load->regs_written);
      load->dst = payload;
      write = emit(FS_OPCODE_FB_WRITE, reg_undef, payload);
      write->base_mrf = -1;
   } else {
      /* Send from the MRF */
      load = emit(LOAD_PAYLOAD(fs_reg(MRF, 1, BRW_REGISTER_TYPE_F, exec_size),
                               sources, length, payload_header_size));

      /* On pre-SNB, we have to interlace the color values.  LOAD_PAYLOAD
       * will do this for us if we just give it a COMPR4 destination.
       */
      if (brw->gen < 6 && exec_size == 16)
         load->dst.reg |= BRW_MRF_COMPR4;

      write = emit(FS_OPCODE_FB_WRITE);
      write->exec_size = exec_size;
      write->base_mrf = 1;
   }

   write->mlen = load->regs_written;
   write->header_size = header_size;
   if (prog_data->uses_kill) {
      write->predicate = BRW_PREDICATE_NORMAL;
      write->flag_subreg = 1;
   }
   return write;
}

void
fs_visitor::emit_fb_writes()
{
   assert(stage == MESA_SHADER_FRAGMENT);
   brw_wm_prog_data *prog_data = (brw_wm_prog_data*) this->prog_data;
   brw_wm_prog_key *key = (brw_wm_prog_key*) this->key;

   fs_inst *inst = NULL;
   if (do_dual_src) {
      this->current_annotation = ralloc_asprintf(this->mem_ctx,
						 "FB dual-source write");
      inst = emit_single_fb_write(this->outputs[0], this->dual_src_output,
                                  reg_undef, 4, 8);
      inst->target = 0;

      /* SIMD16 dual source blending requires to send two SIMD8 dual source
       * messages, where each message contains color data for 8 pixels. Color
       * data for the first group of pixels is stored in the "lower" half of
       * the color registers, so in SIMD16, the previous message did:
       * m + 0: r0
       * m + 1: g0
       * m + 2: b0
       * m + 3: a0
       *
       * Here goes the second message, which packs color data for the
       * remaining 8 pixels. Color data for these pixels is stored in the
       * "upper" half of the color registers, so we need to do:
       * m + 0: r1
       * m + 1: g1
       * m + 2: b1
       * m + 3: a1
       */
      if (dispatch_width == 16) {
         inst = emit_single_fb_write(this->outputs[0], this->dual_src_output,
                                     reg_undef, 4, 8, true);
         inst->target = 0;
      }

      prog_data->dual_src_blend = true;
   } else {
      for (int target = 0; target < key->nr_color_regions; target++) {
         /* Skip over outputs that weren't written. */
         if (this->outputs[target].file == BAD_FILE)
            continue;

         this->current_annotation = ralloc_asprintf(this->mem_ctx,
                                                    "FB write target %d",
                                                    target);
         fs_reg src0_alpha;
         if (devinfo->gen >= 6 && key->replicate_alpha && target != 0)
            src0_alpha = offset(outputs[0], 3);

         inst = emit_single_fb_write(this->outputs[target], reg_undef,
                                     src0_alpha,
                                     this->output_components[target],
                                     dispatch_width);
         inst->target = target;
      }
   }

   if (inst == NULL) {
      /* Even if there's no color buffers enabled, we still need to send
       * alpha out the pipeline to our null renderbuffer to support
       * alpha-testing, alpha-to-coverage, and so on.
       */
      inst = emit_single_fb_write(reg_undef, reg_undef, reg_undef, 0,
                                  dispatch_width);
      inst->target = 0;
   }

   inst->eot = true;
   this->current_annotation = NULL;
}

void
fs_visitor::setup_uniform_clipplane_values()
{
   gl_clip_plane *clip_planes = brw_select_clip_planes(ctx);
   const struct brw_vue_prog_key *key =
      (const struct brw_vue_prog_key *) this->key;

   for (int i = 0; i < key->nr_userclip_plane_consts; i++) {
      this->userplane[i] = fs_reg(UNIFORM, uniforms);
      for (int j = 0; j < 4; ++j) {
         stage_prog_data->param[uniforms + j] =
            (gl_constant_value *) &clip_planes[i][j];
      }
      uniforms += 4;
   }
}

void fs_visitor::compute_clip_distance()
{
   struct brw_vue_prog_data *vue_prog_data =
      (struct brw_vue_prog_data *) prog_data;
   const struct brw_vue_prog_key *key =
      (const struct brw_vue_prog_key *) this->key;

   /* From the GLSL 1.30 spec, section 7.1 (Vertex Shader Special Variables):
    *
    *     "If a linked set of shaders forming the vertex stage contains no
    *     static write to gl_ClipVertex or gl_ClipDistance, but the
    *     application has requested clipping against user clip planes through
    *     the API, then the coordinate written to gl_Position is used for
    *     comparison against the user clip planes."
    *
    * This function is only called if the shader didn't write to
    * gl_ClipDistance.  Accordingly, we use gl_ClipVertex to perform clipping
    * if the user wrote to it; otherwise we use gl_Position.
    */

   gl_varying_slot clip_vertex = VARYING_SLOT_CLIP_VERTEX;
   if (!(vue_prog_data->vue_map.slots_valid & VARYING_BIT_CLIP_VERTEX))
      clip_vertex = VARYING_SLOT_POS;

   /* If the clip vertex isn't written, skip this.  Typically this means
    * the GS will set up clipping. */
   if (outputs[clip_vertex].file == BAD_FILE)
      return;

   setup_uniform_clipplane_values();

   current_annotation = "user clip distances";

   this->outputs[VARYING_SLOT_CLIP_DIST0] = vgrf(glsl_type::vec4_type);
   this->outputs[VARYING_SLOT_CLIP_DIST1] = vgrf(glsl_type::vec4_type);

   for (int i = 0; i < key->nr_userclip_plane_consts; i++) {
      fs_reg u = userplane[i];
      fs_reg output = outputs[VARYING_SLOT_CLIP_DIST0 + i / 4];
      output.reg_offset = i & 3;

      emit(MUL(output, outputs[clip_vertex], u));
      for (int j = 1; j < 4; j++) {
         u.reg = userplane[i].reg + j;
         emit(MAD(output, output, offset(outputs[clip_vertex], j), u));
      }
   }
}

void
fs_visitor::emit_urb_writes()
{
   int slot, urb_offset, length;
   struct brw_vs_prog_data *vs_prog_data =
      (struct brw_vs_prog_data *) prog_data;
   const struct brw_vs_prog_key *key =
      (const struct brw_vs_prog_key *) this->key;
   const GLbitfield64 psiz_mask =
      VARYING_BIT_LAYER | VARYING_BIT_VIEWPORT | VARYING_BIT_PSIZ;
   const struct brw_vue_map *vue_map = &vs_prog_data->base.vue_map;
   bool flush;
   fs_reg sources[8];

   /* Lower legacy ff and ClipVertex clipping to clip distances */
   if (key->base.userclip_active && !prog->UsesClipDistanceOut)
      compute_clip_distance();

   /* If we don't have any valid slots to write, just do a minimal urb write
    * send to terminate the shader. */
   if (vue_map->slots_valid == 0) {

      fs_reg payload = fs_reg(GRF, alloc.allocate(1), BRW_REGISTER_TYPE_UD);
      fs_inst *inst = emit(MOV(payload, fs_reg(retype(brw_vec8_grf(1, 0),
                                                      BRW_REGISTER_TYPE_UD))));
      inst->force_writemask_all = true;

      inst = emit(SHADER_OPCODE_URB_WRITE_SIMD8, reg_undef, payload);
      inst->eot = true;
      inst->mlen = 1;
      inst->offset = 1;
      return;
   }

   length = 0;
   urb_offset = 0;
   flush = false;
   for (slot = 0; slot < vue_map->num_slots; slot++) {
      fs_reg reg, src, zero;

      int varying = vue_map->slot_to_varying[slot];
      switch (varying) {
      case VARYING_SLOT_PSIZ:

         /* The point size varying slot is the vue header and is always in the
          * vue map.  But often none of the special varyings that live there
          * are written and in that case we can skip writing to the vue
          * header, provided the corresponding state properly clamps the
          * values further down the pipeline. */
         if ((vue_map->slots_valid & psiz_mask) == 0) {
            assert(length == 0);
            urb_offset++;
            break;
         }

         zero = fs_reg(GRF, alloc.allocate(1), BRW_REGISTER_TYPE_UD);
         emit(MOV(zero, fs_reg(0u)));

         sources[length++] = zero;
         if (vue_map->slots_valid & VARYING_BIT_LAYER)
            sources[length++] = this->outputs[VARYING_SLOT_LAYER];
         else
            sources[length++] = zero;

         if (vue_map->slots_valid & VARYING_BIT_VIEWPORT)
            sources[length++] = this->outputs[VARYING_SLOT_VIEWPORT];
         else
            sources[length++] = zero;

         if (vue_map->slots_valid & VARYING_BIT_PSIZ)
            sources[length++] = this->outputs[VARYING_SLOT_PSIZ];
         else
            sources[length++] = zero;
         break;

      case BRW_VARYING_SLOT_NDC:
      case VARYING_SLOT_EDGE:
         unreachable("unexpected scalar vs output");
         break;

      case BRW_VARYING_SLOT_PAD:
         break;

      default:
         /* gl_Position is always in the vue map, but isn't always written by
          * the shader.  Other varyings (clip distances) get added to the vue
          * map but don't always get written.  In those cases, the
          * corresponding this->output[] slot will be invalid we and can skip
          * the urb write for the varying.  If we've already queued up a vue
          * slot for writing we flush a mlen 5 urb write, otherwise we just
          * advance the urb_offset.
          */
         if (this->outputs[varying].file == BAD_FILE) {
            if (length > 0)
               flush = true;
            else
               urb_offset++;
            break;
         }

         if ((varying == VARYING_SLOT_COL0 ||
              varying == VARYING_SLOT_COL1 ||
              varying == VARYING_SLOT_BFC0 ||
              varying == VARYING_SLOT_BFC1) &&
             key->clamp_vertex_color) {
            /* We need to clamp these guys, so do a saturating MOV into a
             * temp register and use that for the payload.
             */
            for (int i = 0; i < 4; i++) {
               reg = fs_reg(GRF, alloc.allocate(1), outputs[varying].type);
               src = offset(this->outputs[varying], i);
               fs_inst *inst = emit(MOV(reg, src));
               inst->saturate = true;
               sources[length++] = reg;
            }
         } else {
            for (int i = 0; i < 4; i++)
               sources[length++] = offset(this->outputs[varying], i);
         }
         break;
      }

      current_annotation = "URB write";

      /* If we've queued up 8 registers of payload (2 VUE slots), if this is
       * the last slot or if we need to flush (see BAD_FILE varying case
       * above), emit a URB write send now to flush out the data.
       */
      int last = slot == vue_map->num_slots - 1;
      if (length == 8 || last)
         flush = true;
      if (flush) {
         fs_reg *payload_sources = ralloc_array(mem_ctx, fs_reg, length + 1);
         fs_reg payload = fs_reg(GRF, alloc.allocate(length + 1),
                                 BRW_REGISTER_TYPE_F, dispatch_width);

         /* We need WE_all on the MOV for the message header (the URB handles)
          * so do a MOV to a dummy register and set force_writemask_all on the
          * MOV.  LOAD_PAYLOAD will preserve that.
          */
         fs_reg dummy = fs_reg(GRF, alloc.allocate(1),
                               BRW_REGISTER_TYPE_UD);
         fs_inst *inst = emit(MOV(dummy, fs_reg(retype(brw_vec8_grf(1, 0),
                                                       BRW_REGISTER_TYPE_UD))));
         inst->force_writemask_all = true;
         payload_sources[0] = dummy;

         memcpy(&payload_sources[1], sources, length * sizeof sources[0]);
         emit(LOAD_PAYLOAD(payload, payload_sources, length + 1, 1));

         inst = emit(SHADER_OPCODE_URB_WRITE_SIMD8, reg_undef, payload);
         inst->eot = last;
         inst->mlen = length + 1;
         inst->offset = urb_offset;
         urb_offset = slot + 1;
         length = 0;
         flush = false;
      }
   }
}

void
fs_visitor::resolve_ud_negate(fs_reg *reg)
{
   if (reg->type != BRW_REGISTER_TYPE_UD ||
       !reg->negate)
      return;

   fs_reg temp = vgrf(glsl_type::uint_type);
   emit(MOV(temp, *reg));
   *reg = temp;
}

void
fs_visitor::emit_cs_terminate()
{
   assert(brw->gen >= 7);

   /* We are getting the thread ID from the compute shader header */
   assert(stage == MESA_SHADER_COMPUTE);

   /* We can't directly send from g0, since sends with EOT have to use
    * g112-127. So, copy it to a virtual register, The register allocator will
    * make sure it uses the appropriate register range.
    */
   struct brw_reg g0 = retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD);
   fs_reg payload = fs_reg(GRF, alloc.allocate(1), BRW_REGISTER_TYPE_UD);
   fs_inst *inst = emit(MOV(payload, g0));
   inst->force_writemask_all = true;

   /* Send a message to the thread spawner to terminate the thread. */
   inst = emit(CS_OPCODE_CS_TERMINATE, reg_undef, payload);
   inst->eot = true;
}

/**
 * Resolve the result of a Gen4-5 CMP instruction to a proper boolean.
 *
 * CMP on Gen4-5 only sets the LSB of the result; the rest are undefined.
 * If we need a proper boolean value, we have to fix it up to be 0 or ~0.
 */
void
fs_visitor::resolve_bool_comparison(ir_rvalue *rvalue, fs_reg *reg)
{
   assert(devinfo->gen <= 5);

   if (rvalue->type != glsl_type::bool_type)
      return;

   fs_reg and_result = vgrf(glsl_type::bool_type);
   fs_reg neg_result = vgrf(glsl_type::bool_type);
   emit(AND(and_result, *reg, fs_reg(1)));
   emit(MOV(neg_result, negate(and_result)));
   *reg = neg_result;
}

fs_visitor::fs_visitor(struct brw_context *brw,
                       void *mem_ctx,
                       gl_shader_stage stage,
                       const void *key,
                       struct brw_stage_prog_data *prog_data,
                       struct gl_shader_program *shader_prog,
                       struct gl_program *prog,
                       unsigned dispatch_width)
   : backend_visitor(brw, shader_prog, prog, prog_data, stage),
     reg_null_f(retype(brw_null_vec(dispatch_width), BRW_REGISTER_TYPE_F)),
     reg_null_d(retype(brw_null_vec(dispatch_width), BRW_REGISTER_TYPE_D)),
     reg_null_ud(retype(brw_null_vec(dispatch_width), BRW_REGISTER_TYPE_UD)),
     key(key), prog_data(prog_data),
     dispatch_width(dispatch_width), promoted_constants(0)
{
   this->mem_ctx = mem_ctx;

   switch (stage) {
   case MESA_SHADER_FRAGMENT:
      key_tex = &((const brw_wm_prog_key *) key)->tex;
      break;
   case MESA_SHADER_VERTEX:
   case MESA_SHADER_GEOMETRY:
      key_tex = &((const brw_vue_prog_key *) key)->tex;
      break;
   case MESA_SHADER_COMPUTE:
      key_tex = &((const brw_cs_prog_key*) key)->tex;
      break;
   default:
      unreachable("unhandled shader stage");
   }

   this->failed = false;
   this->simd16_unsupported = false;
   this->no16_msg = NULL;
   this->variable_ht = hash_table_ctor(0,
                                       hash_table_pointer_hash,
                                       hash_table_pointer_compare);

   this->nir_locals = NULL;
   this->nir_globals = NULL;

   memset(&this->payload, 0, sizeof(this->payload));
   memset(this->outputs, 0, sizeof(this->outputs));
   memset(this->output_components, 0, sizeof(this->output_components));
   this->source_depth_to_render_target = false;
   this->runtime_check_aads_emit = false;
   this->first_non_payload_grf = 0;
   this->max_grf = devinfo->gen >= 7 ? GEN7_MRF_HACK_START : BRW_MAX_GRF;

   this->current_annotation = NULL;
   this->base_ir = NULL;

   this->virtual_grf_start = NULL;
   this->virtual_grf_end = NULL;
   this->live_intervals = NULL;
   this->regs_live_at_ip = NULL;

   this->uniforms = 0;
   this->last_scratch = 0;
   this->pull_constant_loc = NULL;
   this->push_constant_loc = NULL;

   this->spilled_any_registers = false;
   this->do_dual_src = false;

   if (dispatch_width == 8)
      this->param_size = rzalloc_array(mem_ctx, int, stage_prog_data->nr_params);
}

fs_visitor::~fs_visitor()
{
   hash_table_dtor(this->variable_ht);
}
