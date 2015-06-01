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

/** @file brw_fs_generator.cpp
 *
 * This file supports generating code from the FS LIR to the actual
 * native instructions.
 */

#include "main/macros.h"
#include "brw_context.h"
#include "brw_eu.h"
#include "brw_fs.h"
#include "brw_cfg.h"

static uint32_t brw_file_from_reg(fs_reg *reg)
{
   switch (reg->file) {
   case GRF:
      return BRW_GENERAL_REGISTER_FILE;
   case MRF:
      return BRW_MESSAGE_REGISTER_FILE;
   case IMM:
      return BRW_IMMEDIATE_VALUE;
   default:
      unreachable("not reached");
   }
}

static struct brw_reg
brw_reg_from_fs_reg(fs_reg *reg)
{
   struct brw_reg brw_reg;

   switch (reg->file) {
   case GRF:
   case MRF:
      if (reg->stride == 0) {
         brw_reg = brw_vec1_reg(brw_file_from_reg(reg), reg->reg, 0);
      } else if (reg->width < 8) {
         brw_reg = brw_vec8_reg(brw_file_from_reg(reg), reg->reg, 0);
         brw_reg = stride(brw_reg, reg->width * reg->stride,
                          reg->width, reg->stride);
      } else {
         /* From the Haswell PRM:
          *
          * VertStride must be used to cross GRF register boundaries. This
          * rule implies that elements within a 'Width' cannot cross GRF
          * boundaries.
          *
          * So, for registers with width > 8, we have to use a width of 8
          * and trust the compression state to sort out the exec size.
          */
         brw_reg = brw_vec8_reg(brw_file_from_reg(reg), reg->reg, 0);
         brw_reg = stride(brw_reg, 8 * reg->stride, 8, reg->stride);
      }

      brw_reg = retype(brw_reg, reg->type);
      brw_reg = byte_offset(brw_reg, reg->subreg_offset);
      break;
   case IMM:
      switch (reg->type) {
      case BRW_REGISTER_TYPE_F:
	 brw_reg = brw_imm_f(reg->fixed_hw_reg.dw1.f);
	 break;
      case BRW_REGISTER_TYPE_D:
	 brw_reg = brw_imm_d(reg->fixed_hw_reg.dw1.d);
	 break;
      case BRW_REGISTER_TYPE_UD:
	 brw_reg = brw_imm_ud(reg->fixed_hw_reg.dw1.ud);
	 break;
      case BRW_REGISTER_TYPE_W:
	 brw_reg = brw_imm_w(reg->fixed_hw_reg.dw1.d);
	 break;
      case BRW_REGISTER_TYPE_UW:
	 brw_reg = brw_imm_uw(reg->fixed_hw_reg.dw1.ud);
	 break;
      case BRW_REGISTER_TYPE_VF:
         brw_reg = brw_imm_vf(reg->fixed_hw_reg.dw1.ud);
         break;
      default:
	 unreachable("not reached");
      }
      break;
   case HW_REG:
      assert(reg->type == reg->fixed_hw_reg.type);
      brw_reg = reg->fixed_hw_reg;
      break;
   case BAD_FILE:
      /* Probably unused. */
      brw_reg = brw_null_reg();
      break;
   default:
      unreachable("not reached");
   }
   if (reg->abs)
      brw_reg = brw_abs(brw_reg);
   if (reg->negate)
      brw_reg = negate(brw_reg);

   return brw_reg;
}

fs_generator::fs_generator(struct brw_context *brw,
                           void *mem_ctx,
                           const void *key,
                           struct brw_stage_prog_data *prog_data,
                           struct gl_program *prog,
                           unsigned promoted_constants,
                           bool runtime_check_aads_emit,
                           const char *stage_abbrev)

   : brw(brw), devinfo(brw->intelScreen->devinfo), key(key),
     prog_data(prog_data),
     prog(prog), promoted_constants(promoted_constants),
     runtime_check_aads_emit(runtime_check_aads_emit), debug_flag(false),
     stage_abbrev(stage_abbrev), mem_ctx(mem_ctx)
{
   p = rzalloc(mem_ctx, struct brw_codegen);
   brw_init_codegen(devinfo, p, mem_ctx);
}

fs_generator::~fs_generator()
{
}

class ip_record : public exec_node {
public:
   DECLARE_RALLOC_CXX_OPERATORS(ip_record)

   ip_record(int ip)
   {
      this->ip = ip;
   }

   int ip;
};

bool
fs_generator::patch_discard_jumps_to_fb_writes()
{
   if (devinfo->gen < 6 || this->discard_halt_patches.is_empty())
      return false;

   int scale = brw_jump_scale(p->devinfo);

   /* There is a somewhat strange undocumented requirement of using
    * HALT, according to the simulator.  If some channel has HALTed to
    * a particular UIP, then by the end of the program, every channel
    * must have HALTed to that UIP.  Furthermore, the tracking is a
    * stack, so you can't do the final halt of a UIP after starting
    * halting to a new UIP.
    *
    * Symptoms of not emitting this instruction on actual hardware
    * included GPU hangs and sparkly rendering on the piglit discard
    * tests.
    */
   brw_inst *last_halt = gen6_HALT(p);
   brw_inst_set_uip(p->devinfo, last_halt, 1 * scale);
   brw_inst_set_jip(p->devinfo, last_halt, 1 * scale);

   int ip = p->nr_insn;

   foreach_in_list(ip_record, patch_ip, &discard_halt_patches) {
      brw_inst *patch = &p->store[patch_ip->ip];

      assert(brw_inst_opcode(p->devinfo, patch) == BRW_OPCODE_HALT);
      /* HALT takes a half-instruction distance from the pre-incremented IP. */
      brw_inst_set_uip(p->devinfo, patch, (ip - patch_ip->ip) * scale);
   }

   this->discard_halt_patches.make_empty();
   return true;
}

void
fs_generator::fire_fb_write(fs_inst *inst,
                            struct brw_reg payload,
                            struct brw_reg implied_header,
                            GLuint nr)
{
   uint32_t msg_control;

   brw_wm_prog_data *prog_data = (brw_wm_prog_data*) this->prog_data;

   if (devinfo->gen < 6) {
      brw_push_insn_state(p);
      brw_set_default_exec_size(p, BRW_EXECUTE_8);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
      brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
      brw_MOV(p, offset(payload, 1), brw_vec8_grf(1, 0));
      brw_pop_insn_state(p);
   }

   if (inst->opcode == FS_OPCODE_REP_FB_WRITE)
      msg_control = BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD16_SINGLE_SOURCE_REPLICATED;
   else if (prog_data->dual_src_blend) {
      if (dispatch_width == 8 || !inst->eot)
         msg_control = BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD8_DUAL_SOURCE_SUBSPAN01;
      else
         msg_control = BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD8_DUAL_SOURCE_SUBSPAN23;
   } else if (dispatch_width == 16)
      msg_control = BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD16_SINGLE_SOURCE;
   else
      msg_control = BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD8_SINGLE_SOURCE_SUBSPAN01;

   uint32_t surf_index =
      prog_data->binding_table.render_target_start + inst->target;

   bool last_render_target = inst->eot ||
                             (prog_data->dual_src_blend && dispatch_width == 16);


   brw_fb_WRITE(p,
                dispatch_width,
                payload,
                implied_header,
                msg_control,
                surf_index,
                nr,
                0,
                inst->eot,
                last_render_target,
                inst->header_size != 0);

   brw_mark_surface_used(&prog_data->base, surf_index);
}

void
fs_generator::generate_fb_write(fs_inst *inst, struct brw_reg payload)
{
   brw_wm_prog_data *prog_data = (brw_wm_prog_data*) this->prog_data;
   const brw_wm_prog_key * const key = (brw_wm_prog_key * const) this->key;
   struct brw_reg implied_header;

   if (devinfo->gen < 8 && !devinfo->is_haswell) {
      brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
   }

   if (inst->base_mrf >= 0)
      payload = brw_message_reg(inst->base_mrf);

   /* Header is 2 regs, g0 and g1 are the contents. g0 will be implied
    * move, here's g1.
    */
   if (inst->header_size != 0) {
      brw_push_insn_state(p);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
      brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
      brw_set_default_flag_reg(p, 0, 0);

      /* On HSW, the GPU will use the predicate on SENDC, unless the header is
       * present.
       */
      if (prog_data->uses_kill) {
         struct brw_reg pixel_mask;

         if (devinfo->gen >= 6)
            pixel_mask = retype(brw_vec1_grf(1, 7), BRW_REGISTER_TYPE_UW);
         else
            pixel_mask = retype(brw_vec1_grf(0, 0), BRW_REGISTER_TYPE_UW);

         brw_MOV(p, pixel_mask, brw_flag_reg(0, 1));
      }

      if (devinfo->gen >= 6) {
         brw_push_insn_state(p);
         brw_set_default_exec_size(p, BRW_EXECUTE_16);
	 brw_set_default_compression_control(p, BRW_COMPRESSION_COMPRESSED);
	 brw_MOV(p,
		 retype(payload, BRW_REGISTER_TYPE_UD),
		 retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));
         brw_pop_insn_state(p);

         if (inst->target > 0 && key->replicate_alpha) {
            /* Set "Source0 Alpha Present to RenderTarget" bit in message
             * header.
             */
            brw_OR(p,
		   vec1(retype(payload, BRW_REGISTER_TYPE_UD)),
		   vec1(retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD)),
		   brw_imm_ud(0x1 << 11));
         }

	 if (inst->target > 0) {
	    /* Set the render target index for choosing BLEND_STATE. */
	    brw_MOV(p, retype(vec1(suboffset(payload, 2)),
                              BRW_REGISTER_TYPE_UD),
		    brw_imm_ud(inst->target));
	 }

	 implied_header = brw_null_reg();
      } else {
	 implied_header = retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UW);
      }

      brw_pop_insn_state(p);
   } else {
      implied_header = brw_null_reg();
   }

   if (!runtime_check_aads_emit) {
      fire_fb_write(inst, payload, implied_header, inst->mlen);
   } else {
      /* This can only happen in gen < 6 */
      assert(devinfo->gen < 6);

      struct brw_reg v1_null_ud = vec1(retype(brw_null_reg(), BRW_REGISTER_TYPE_UD));

      /* Check runtime bit to detect if we have to send AA data or not */
      brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
      brw_AND(p,
              v1_null_ud,
              retype(brw_vec1_grf(1, 6), BRW_REGISTER_TYPE_UD),
              brw_imm_ud(1<<26));
      brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);

      int jmp = brw_JMPI(p, brw_imm_ud(0), BRW_PREDICATE_NORMAL) - p->store;
      brw_inst_set_exec_size(p->devinfo, brw_last_inst, BRW_EXECUTE_1);
      {
         /* Don't send AA data */
         fire_fb_write(inst, offset(payload, 1), implied_header, inst->mlen-1);
      }
      brw_land_fwd_jump(p, jmp);
      fire_fb_write(inst, payload, implied_header, inst->mlen);
   }
}

void
fs_generator::generate_urb_write(fs_inst *inst, struct brw_reg payload)
{
   brw_inst *insn;

   insn = brw_next_insn(p, BRW_OPCODE_SEND);

   brw_set_dest(p, insn, brw_null_reg());
   brw_set_src0(p, insn, payload);
   brw_set_src1(p, insn, brw_imm_d(0));

   brw_inst_set_sfid(p->devinfo, insn, BRW_SFID_URB);
   brw_inst_set_urb_opcode(p->devinfo, insn, GEN8_URB_OPCODE_SIMD8_WRITE);

   brw_inst_set_mlen(p->devinfo, insn, inst->mlen);
   brw_inst_set_rlen(p->devinfo, insn, 0);
   brw_inst_set_eot(p->devinfo, insn, inst->eot);
   brw_inst_set_header_present(p->devinfo, insn, true);
   brw_inst_set_urb_global_offset(p->devinfo, insn, inst->offset);
}

void
fs_generator::generate_cs_terminate(fs_inst *inst, struct brw_reg payload)
{
   struct brw_inst *insn;

   insn = brw_next_insn(p, BRW_OPCODE_SEND);

   brw_set_dest(p, insn, brw_null_reg());
   brw_set_src0(p, insn, payload);
   brw_set_src1(p, insn, brw_imm_d(0));

   /* Terminate a compute shader by sending a message to the thread spawner.
    */
   brw_inst_set_sfid(devinfo, insn, BRW_SFID_THREAD_SPAWNER);
   brw_inst_set_mlen(devinfo, insn, 1);
   brw_inst_set_rlen(devinfo, insn, 0);
   brw_inst_set_eot(devinfo, insn, inst->eot);
   brw_inst_set_header_present(devinfo, insn, false);

   brw_inst_set_ts_opcode(devinfo, insn, 0); /* Dereference resource */
   brw_inst_set_ts_request_type(devinfo, insn, 0); /* Root thread */

   /* Note that even though the thread has a URB resource associated with it,
    * we set the "do not dereference URB" bit, because the URB resource is
    * managed by the fixed-function unit, so it will free it automatically.
    */
   brw_inst_set_ts_resource_select(devinfo, insn, 1); /* Do not dereference URB */

   brw_inst_set_mask_control(devinfo, insn, BRW_MASK_DISABLE);
}

void
fs_generator::generate_blorp_fb_write(fs_inst *inst)
{
   brw_fb_WRITE(p,
                16 /* dispatch_width */,
                brw_message_reg(inst->base_mrf),
                brw_reg_from_fs_reg(&inst->src[0]),
                BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD16_SINGLE_SOURCE,
                inst->target,
                inst->mlen,
                0,
                true,
                true,
                inst->header_size != 0);
}

void
fs_generator::generate_linterp(fs_inst *inst,
			     struct brw_reg dst, struct brw_reg *src)
{
   /* PLN reads:
    *                      /   in SIMD16   \
    *    -----------------------------------
    *   | src1+0 | src1+1 | src1+2 | src1+3 |
    *   |-----------------------------------|
    *   |(x0, x1)|(y0, y1)|(x2, x3)|(y2, y3)|
    *    -----------------------------------
    *
    * but for the LINE/MAC pair, the LINE reads Xs and the MAC reads Ys:
    *
    *    -----------------------------------
    *   | src1+0 | src1+1 | src1+2 | src1+3 |
    *   |-----------------------------------|
    *   |(x0, x1)|(y0, y1)|        |        | in SIMD8
    *   |-----------------------------------|
    *   |(x0, x1)|(x2, x3)|(y0, y1)|(y2, y3)| in SIMD16
    *    -----------------------------------
    *
    * See also: emit_interpolation_setup_gen4().
    */
   struct brw_reg delta_x = src[0];
   struct brw_reg delta_y = offset(src[0], dispatch_width / 8);
   struct brw_reg interp = src[1];

   if (devinfo->has_pln &&
       (devinfo->gen >= 7 || (delta_x.nr & 1) == 0)) {
      brw_PLN(p, dst, interp, delta_x);
   } else {
      brw_LINE(p, brw_null_reg(), interp, delta_x);
      brw_MAC(p, dst, suboffset(interp, 1), delta_y);
   }
}

void
fs_generator::generate_math_gen6(fs_inst *inst,
                                 struct brw_reg dst,
                                 struct brw_reg src0,
                                 struct brw_reg src1)
{
   int op = brw_math_function(inst->opcode);
   bool binop = src1.file != BRW_ARCHITECTURE_REGISTER_FILE;

   if (dispatch_width == 8) {
      gen6_math(p, dst, op, src0, src1);
   } else if (dispatch_width == 16) {
      brw_push_insn_state(p);
      brw_set_default_exec_size(p, BRW_EXECUTE_8);
      brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
      gen6_math(p, firsthalf(dst), op, firsthalf(src0), firsthalf(src1));
      brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
      gen6_math(p, sechalf(dst), op, sechalf(src0),
                binop ? sechalf(src1) : brw_null_reg());
      brw_pop_insn_state(p);
   }
}

void
fs_generator::generate_math_gen4(fs_inst *inst,
			       struct brw_reg dst,
			       struct brw_reg src)
{
   int op = brw_math_function(inst->opcode);

   assert(inst->mlen >= 1);

   if (dispatch_width == 8) {
      gen4_math(p, dst,
                op,
                inst->base_mrf, src,
                BRW_MATH_PRECISION_FULL);
   } else if (dispatch_width == 16) {
      brw_set_default_exec_size(p, BRW_EXECUTE_8);
      brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
      gen4_math(p, firsthalf(dst),
	        op,
	        inst->base_mrf, firsthalf(src),
	        BRW_MATH_PRECISION_FULL);
      brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
      gen4_math(p, sechalf(dst),
	        op,
	        inst->base_mrf + 1, sechalf(src),
	        BRW_MATH_PRECISION_FULL);

      brw_set_default_compression_control(p, BRW_COMPRESSION_COMPRESSED);
   }
}

void
fs_generator::generate_math_g45(fs_inst *inst,
                                struct brw_reg dst,
                                struct brw_reg src)
{
   if (inst->opcode == SHADER_OPCODE_POW ||
       inst->opcode == SHADER_OPCODE_INT_QUOTIENT ||
       inst->opcode == SHADER_OPCODE_INT_REMAINDER) {
      generate_math_gen4(inst, dst, src);
      return;
   }

   int op = brw_math_function(inst->opcode);

   assert(inst->mlen >= 1);

   gen4_math(p, dst,
             op,
             inst->base_mrf, src,
             BRW_MATH_PRECISION_FULL);
}

void
fs_generator::generate_tex(fs_inst *inst, struct brw_reg dst, struct brw_reg src,
                           struct brw_reg sampler_index)
{
   int msg_type = -1;
   int rlen = 4;
   uint32_t simd_mode;
   uint32_t return_format;
   bool is_combined_send = inst->eot;

   switch (dst.type) {
   case BRW_REGISTER_TYPE_D:
      return_format = BRW_SAMPLER_RETURN_FORMAT_SINT32;
      break;
   case BRW_REGISTER_TYPE_UD:
      return_format = BRW_SAMPLER_RETURN_FORMAT_UINT32;
      break;
   default:
      return_format = BRW_SAMPLER_RETURN_FORMAT_FLOAT32;
      break;
   }

   switch (inst->exec_size) {
   case 8:
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD8;
      break;
   case 16:
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
      break;
   default:
      unreachable("Invalid width for texture instruction");
   }

   if (devinfo->gen >= 5) {
      switch (inst->opcode) {
      case SHADER_OPCODE_TEX:
	 if (inst->shadow_compare) {
	    msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE_COMPARE;
	 } else {
	    msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE;
	 }
	 break;
      case FS_OPCODE_TXB:
	 if (inst->shadow_compare) {
	    msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE_BIAS_COMPARE;
	 } else {
	    msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE_BIAS;
	 }
	 break;
      case SHADER_OPCODE_TXL:
	 if (inst->shadow_compare) {
	    msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE_LOD_COMPARE;
	 } else {
	    msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE_LOD;
	 }
	 break;
      case SHADER_OPCODE_TXS:
	 msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE_RESINFO;
	 break;
      case SHADER_OPCODE_TXD:
         if (inst->shadow_compare) {
            /* Gen7.5+.  Otherwise, lowered by brw_lower_texture_gradients(). */
            assert(devinfo->gen >= 8 || devinfo->is_haswell);
            msg_type = HSW_SAMPLER_MESSAGE_SAMPLE_DERIV_COMPARE;
         } else {
            msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE_DERIVS;
         }
	 break;
      case SHADER_OPCODE_TXF:
	 msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE_LD;
	 break;
      case SHADER_OPCODE_TXF_CMS:
         if (devinfo->gen >= 7)
            msg_type = GEN7_SAMPLER_MESSAGE_SAMPLE_LD2DMS;
         else
            msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE_LD;
         break;
      case SHADER_OPCODE_TXF_UMS:
         assert(devinfo->gen >= 7);
         msg_type = GEN7_SAMPLER_MESSAGE_SAMPLE_LD2DSS;
         break;
      case SHADER_OPCODE_TXF_MCS:
         assert(devinfo->gen >= 7);
         msg_type = GEN7_SAMPLER_MESSAGE_SAMPLE_LD_MCS;
         break;
      case SHADER_OPCODE_LOD:
         msg_type = GEN5_SAMPLER_MESSAGE_LOD;
         break;
      case SHADER_OPCODE_TG4:
         if (inst->shadow_compare) {
            assert(devinfo->gen >= 7);
            msg_type = GEN7_SAMPLER_MESSAGE_SAMPLE_GATHER4_C;
         } else {
            assert(devinfo->gen >= 6);
            msg_type = GEN7_SAMPLER_MESSAGE_SAMPLE_GATHER4;
         }
         break;
      case SHADER_OPCODE_TG4_OFFSET:
         assert(devinfo->gen >= 7);
         if (inst->shadow_compare) {
            msg_type = GEN7_SAMPLER_MESSAGE_SAMPLE_GATHER4_PO_C;
         } else {
            msg_type = GEN7_SAMPLER_MESSAGE_SAMPLE_GATHER4_PO;
         }
         break;
      default:
	 unreachable("not reached");
      }
   } else {
      switch (inst->opcode) {
      case SHADER_OPCODE_TEX:
	 /* Note that G45 and older determines shadow compare and dispatch width
	  * from message length for most messages.
	  */
         if (dispatch_width == 8) {
            msg_type = BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE;
            if (inst->shadow_compare) {
               assert(inst->mlen == 6);
            } else {
               assert(inst->mlen <= 4);
            }
         } else {
            if (inst->shadow_compare) {
               msg_type = BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_COMPARE;
               assert(inst->mlen == 9);
            } else {
               msg_type = BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE;
               assert(inst->mlen <= 7 && inst->mlen % 2 == 1);
            }
         }
	 break;
      case FS_OPCODE_TXB:
	 if (inst->shadow_compare) {
            assert(dispatch_width == 8);
	    assert(inst->mlen == 6);
	    msg_type = BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_BIAS_COMPARE;
	 } else {
	    assert(inst->mlen == 9);
	    msg_type = BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_BIAS;
	    simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
	 }
	 break;
      case SHADER_OPCODE_TXL:
	 if (inst->shadow_compare) {
            assert(dispatch_width == 8);
	    assert(inst->mlen == 6);
	    msg_type = BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_LOD_COMPARE;
	 } else {
	    assert(inst->mlen == 9);
	    msg_type = BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_LOD;
	    simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
	 }
	 break;
      case SHADER_OPCODE_TXD:
	 /* There is no sample_d_c message; comparisons are done manually */
         assert(dispatch_width == 8);
	 assert(inst->mlen == 7 || inst->mlen == 10);
	 msg_type = BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_GRADIENTS;
	 break;
      case SHADER_OPCODE_TXF:
         assert(inst->mlen <= 9 && inst->mlen % 2 == 1);
	 msg_type = BRW_SAMPLER_MESSAGE_SIMD16_LD;
	 simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
	 break;
      case SHADER_OPCODE_TXS:
	 assert(inst->mlen == 3);
	 msg_type = BRW_SAMPLER_MESSAGE_SIMD16_RESINFO;
	 simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
	 break;
      default:
	 unreachable("not reached");
      }
   }
   assert(msg_type != -1);

   if (simd_mode == BRW_SAMPLER_SIMD_MODE_SIMD16) {
      rlen = 8;
      dst = vec16(dst);
   }

   if (is_combined_send) {
      assert(devinfo->gen >= 9 || devinfo->is_cherryview);
      rlen = 0;
   }

   assert(devinfo->gen < 7 || inst->header_size == 0 ||
          src.file == BRW_GENERAL_REGISTER_FILE);

   assert(sampler_index.type == BRW_REGISTER_TYPE_UD);

   /* Load the message header if present.  If there's a texture offset,
    * we need to set it up explicitly and load the offset bitfield.
    * Otherwise, we can use an implied move from g0 to the first message reg.
    */
   if (inst->header_size != 0) {
      if (devinfo->gen < 6 && !inst->offset) {
         /* Set up an implied move from g0 to the MRF. */
         src = retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UW);
      } else {
         struct brw_reg header_reg;

         if (devinfo->gen >= 7) {
            header_reg = src;
         } else {
            assert(inst->base_mrf != -1);
            header_reg = brw_message_reg(inst->base_mrf);
         }

         brw_push_insn_state(p);
         brw_set_default_exec_size(p, BRW_EXECUTE_8);
         brw_set_default_mask_control(p, BRW_MASK_DISABLE);
         brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
         /* Explicitly set up the message header by copying g0 to the MRF. */
         brw_MOV(p, header_reg, brw_vec8_grf(0, 0));

         if (inst->offset) {
            /* Set the offset bits in DWord 2. */
            brw_MOV(p, get_element_ud(header_reg, 2),
                       brw_imm_ud(inst->offset));
         }

         brw_adjust_sampler_state_pointer(p, header_reg, sampler_index);
         brw_pop_insn_state(p);
      }
   }

   uint32_t base_binding_table_index = (inst->opcode == SHADER_OPCODE_TG4 ||
         inst->opcode == SHADER_OPCODE_TG4_OFFSET)
         ? prog_data->binding_table.gather_texture_start
         : prog_data->binding_table.texture_start;

   if (sampler_index.file == BRW_IMMEDIATE_VALUE) {
      uint32_t sampler = sampler_index.dw1.ud;

      brw_SAMPLE(p,
                 retype(dst, BRW_REGISTER_TYPE_UW),
                 inst->base_mrf,
                 src,
                 sampler + base_binding_table_index,
                 sampler % 16,
                 msg_type,
                 rlen,
                 inst->mlen,
                 inst->header_size != 0,
                 simd_mode,
                 return_format);

      brw_mark_surface_used(prog_data, sampler + base_binding_table_index);
   } else {
      /* Non-const sampler index */
      /* Note: this clobbers `dst` as a temporary before emitting the send */

      struct brw_reg addr = vec1(retype(brw_address_reg(0), BRW_REGISTER_TYPE_UD));
      struct brw_reg temp = vec1(retype(dst, BRW_REGISTER_TYPE_UD));

      struct brw_reg sampler_reg = vec1(retype(sampler_index, BRW_REGISTER_TYPE_UD));

      brw_push_insn_state(p);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_set_default_access_mode(p, BRW_ALIGN_1);

      /* Some care required: `sampler` and `temp` may alias:
       *    addr = sampler & 0xff
       *    temp = (sampler << 8) & 0xf00
       *    addr = addr | temp
       */
      brw_ADD(p, addr, sampler_reg, brw_imm_ud(base_binding_table_index));
      brw_SHL(p, temp, sampler_reg, brw_imm_ud(8u));
      brw_AND(p, temp, temp, brw_imm_ud(0x0f00));
      brw_AND(p, addr, addr, brw_imm_ud(0x0ff));
      brw_OR(p, addr, addr, temp);

      brw_pop_insn_state(p);

      /* dst = send(offset, a0.0 | <descriptor>) */
      brw_inst *insn = brw_send_indirect_message(
         p, BRW_SFID_SAMPLER, dst, src, addr);
      brw_set_sampler_message(p, insn,
                              0 /* surface */,
                              0 /* sampler */,
                              msg_type,
                              rlen,
                              inst->mlen /* mlen */,
                              inst->header_size != 0 /* header */,
                              simd_mode,
                              return_format);

      /* visitor knows more than we do about the surface limit required,
       * so has already done marking.
       */
   }

   if (is_combined_send) {
      brw_inst_set_eot(p->devinfo, brw_last_inst, true);
      brw_inst_set_opcode(p->devinfo, brw_last_inst, BRW_OPCODE_SENDC);
   }
}


/* For OPCODE_DDX and OPCODE_DDY, per channel of output we've got input
 * looking like:
 *
 * arg0: ss0.tl ss0.tr ss0.bl ss0.br ss1.tl ss1.tr ss1.bl ss1.br
 *
 * Ideally, we want to produce:
 *
 *           DDX                     DDY
 * dst: (ss0.tr - ss0.tl)     (ss0.tl - ss0.bl)
 *      (ss0.tr - ss0.tl)     (ss0.tr - ss0.br)
 *      (ss0.br - ss0.bl)     (ss0.tl - ss0.bl)
 *      (ss0.br - ss0.bl)     (ss0.tr - ss0.br)
 *      (ss1.tr - ss1.tl)     (ss1.tl - ss1.bl)
 *      (ss1.tr - ss1.tl)     (ss1.tr - ss1.br)
 *      (ss1.br - ss1.bl)     (ss1.tl - ss1.bl)
 *      (ss1.br - ss1.bl)     (ss1.tr - ss1.br)
 *
 * and add another set of two more subspans if in 16-pixel dispatch mode.
 *
 * For DDX, it ends up being easy: width = 2, horiz=0 gets us the same result
 * for each pair, and vertstride = 2 jumps us 2 elements after processing a
 * pair.  But the ideal approximation may impose a huge performance cost on
 * sample_d.  On at least Haswell, sample_d instruction does some
 * optimizations if the same LOD is used for all pixels in the subspan.
 *
 * For DDY, we need to use ALIGN16 mode since it's capable of doing the
 * appropriate swizzling.
 */
void
fs_generator::generate_ddx(enum opcode opcode,
                           struct brw_reg dst, struct brw_reg src)
{
   unsigned vstride, width;

   if (opcode == FS_OPCODE_DDX_FINE) {
      /* produce accurate derivatives */
      vstride = BRW_VERTICAL_STRIDE_2;
      width = BRW_WIDTH_2;
   } else {
      /* replicate the derivative at the top-left pixel to other pixels */
      vstride = BRW_VERTICAL_STRIDE_4;
      width = BRW_WIDTH_4;
   }

   struct brw_reg src0 = brw_reg(src.file, src.nr, 1,
                                 src.negate, src.abs,
				 BRW_REGISTER_TYPE_F,
				 vstride,
				 width,
				 BRW_HORIZONTAL_STRIDE_0,
				 BRW_SWIZZLE_XYZW, WRITEMASK_XYZW);
   struct brw_reg src1 = brw_reg(src.file, src.nr, 0,
                                 src.negate, src.abs,
				 BRW_REGISTER_TYPE_F,
				 vstride,
				 width,
				 BRW_HORIZONTAL_STRIDE_0,
				 BRW_SWIZZLE_XYZW, WRITEMASK_XYZW);
   brw_ADD(p, dst, src0, negate(src1));
}

/* The negate_value boolean is used to negate the derivative computation for
 * FBOs, since they place the origin at the upper left instead of the lower
 * left.
 */
void
fs_generator::generate_ddy(enum opcode opcode,
                           struct brw_reg dst, struct brw_reg src,
                           bool negate_value)
{
   if (opcode == FS_OPCODE_DDY_FINE) {
      /* From the Ivy Bridge PRM, volume 4 part 3, section 3.3.9 (Register
       * Region Restrictions):
       *
       *     In Align16 access mode, SIMD16 is not allowed for DW operations
       *     and SIMD8 is not allowed for DF operations.
       *
       * In this context, "DW operations" means "operations acting on 32-bit
       * values", so it includes operations on floats.
       *
       * Gen4 has a similar restriction.  From the i965 PRM, section 11.5.3
       * (Instruction Compression -> Rules and Restrictions):
       *
       *     A compressed instruction must be in Align1 access mode. Align16
       *     mode instructions cannot be compressed.
       *
       * Similar text exists in the g45 PRM.
       *
       * On these platforms, if we're building a SIMD16 shader, we need to
       * manually unroll to a pair of SIMD8 instructions.
       */
      bool unroll_to_simd8 =
         (dispatch_width == 16 &&
          (devinfo->gen == 4 || (devinfo->gen == 7 && !devinfo->is_haswell)));

      /* produce accurate derivatives */
      struct brw_reg src0 = brw_reg(src.file, src.nr, 0,
                                    src.negate, src.abs,
                                    BRW_REGISTER_TYPE_F,
                                    BRW_VERTICAL_STRIDE_4,
                                    BRW_WIDTH_4,
                                    BRW_HORIZONTAL_STRIDE_1,
                                    BRW_SWIZZLE_XYXY, WRITEMASK_XYZW);
      struct brw_reg src1 = brw_reg(src.file, src.nr, 0,
                                    src.negate, src.abs,
                                    BRW_REGISTER_TYPE_F,
                                    BRW_VERTICAL_STRIDE_4,
                                    BRW_WIDTH_4,
                                    BRW_HORIZONTAL_STRIDE_1,
                                    BRW_SWIZZLE_ZWZW, WRITEMASK_XYZW);
      brw_push_insn_state(p);
      brw_set_default_access_mode(p, BRW_ALIGN_16);
      if (unroll_to_simd8) {
         brw_set_default_exec_size(p, BRW_EXECUTE_8);
         brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
         if (negate_value) {
            brw_ADD(p, firsthalf(dst), firsthalf(src1), negate(firsthalf(src0)));
            brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
            brw_ADD(p, sechalf(dst), sechalf(src1), negate(sechalf(src0)));
         } else {
            brw_ADD(p, firsthalf(dst), firsthalf(src0), negate(firsthalf(src1)));
            brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
            brw_ADD(p, sechalf(dst), sechalf(src0), negate(sechalf(src1)));
         }
      } else {
         if (negate_value)
            brw_ADD(p, dst, src1, negate(src0));
         else
            brw_ADD(p, dst, src0, negate(src1));
      }
      brw_pop_insn_state(p);
   } else {
      /* replicate the derivative at the top-left pixel to other pixels */
      struct brw_reg src0 = brw_reg(src.file, src.nr, 0,
                                    src.negate, src.abs,
                                    BRW_REGISTER_TYPE_F,
                                    BRW_VERTICAL_STRIDE_4,
                                    BRW_WIDTH_4,
                                    BRW_HORIZONTAL_STRIDE_0,
                                    BRW_SWIZZLE_XYZW, WRITEMASK_XYZW);
      struct brw_reg src1 = brw_reg(src.file, src.nr, 2,
                                    src.negate, src.abs,
                                    BRW_REGISTER_TYPE_F,
                                    BRW_VERTICAL_STRIDE_4,
                                    BRW_WIDTH_4,
                                    BRW_HORIZONTAL_STRIDE_0,
                                    BRW_SWIZZLE_XYZW, WRITEMASK_XYZW);
      if (negate_value)
         brw_ADD(p, dst, src1, negate(src0));
      else
         brw_ADD(p, dst, src0, negate(src1));
   }
}

void
fs_generator::generate_discard_jump(fs_inst *inst)
{
   assert(devinfo->gen >= 6);

   /* This HALT will be patched up at FB write time to point UIP at the end of
    * the program, and at brw_uip_jip() JIP will be set to the end of the
    * current block (or the program).
    */
   this->discard_halt_patches.push_tail(new(mem_ctx) ip_record(p->nr_insn));

   brw_push_insn_state(p);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   gen6_HALT(p);
   brw_pop_insn_state(p);
}

void
fs_generator::generate_scratch_write(fs_inst *inst, struct brw_reg src)
{
   assert(inst->mlen != 0);

   brw_MOV(p,
	   brw_uvec_mrf(inst->exec_size, (inst->base_mrf + 1), 0),
	   retype(src, BRW_REGISTER_TYPE_UD));
   brw_oword_block_write_scratch(p, brw_message_reg(inst->base_mrf),
                                 inst->exec_size / 8, inst->offset);
}

void
fs_generator::generate_scratch_read(fs_inst *inst, struct brw_reg dst)
{
   assert(inst->mlen != 0);

   brw_oword_block_read_scratch(p, dst, brw_message_reg(inst->base_mrf),
                                inst->exec_size / 8, inst->offset);
}

void
fs_generator::generate_scratch_read_gen7(fs_inst *inst, struct brw_reg dst)
{
   gen7_block_read_scratch(p, dst, inst->exec_size / 8, inst->offset);
}

void
fs_generator::generate_uniform_pull_constant_load(fs_inst *inst,
                                                  struct brw_reg dst,
                                                  struct brw_reg index,
                                                  struct brw_reg offset)
{
   assert(inst->mlen != 0);

   assert(index.file == BRW_IMMEDIATE_VALUE &&
	  index.type == BRW_REGISTER_TYPE_UD);
   uint32_t surf_index = index.dw1.ud;

   assert(offset.file == BRW_IMMEDIATE_VALUE &&
	  offset.type == BRW_REGISTER_TYPE_UD);
   uint32_t read_offset = offset.dw1.ud;

   brw_oword_block_read(p, dst, brw_message_reg(inst->base_mrf),
			read_offset, surf_index);

   brw_mark_surface_used(prog_data, surf_index);
}

void
fs_generator::generate_uniform_pull_constant_load_gen7(fs_inst *inst,
                                                       struct brw_reg dst,
                                                       struct brw_reg index,
                                                       struct brw_reg offset)
{
   assert(inst->mlen == 0);
   assert(index.type == BRW_REGISTER_TYPE_UD);

   assert(offset.file == BRW_GENERAL_REGISTER_FILE);
   /* Reference just the dword we need, to avoid angering validate_reg(). */
   offset = brw_vec1_grf(offset.nr, 0);

   /* We use the SIMD4x2 mode because we want to end up with 4 components in
    * the destination loaded consecutively from the same offset (which appears
    * in the first component, and the rest are ignored).
    */
   dst.width = BRW_WIDTH_4;

   struct brw_reg src = offset;
   bool header_present = false;
   int mlen = 1;

   if (devinfo->gen >= 9) {
      /* Skylake requires a message header in order to use SIMD4x2 mode. */
      src = retype(brw_vec4_grf(offset.nr - 1, 0), BRW_REGISTER_TYPE_UD);
      mlen = 2;
      header_present = true;

      brw_push_insn_state(p);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_set_default_exec_size(p, BRW_EXECUTE_8);
      brw_MOV(p, vec8(src), retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));
      brw_set_default_access_mode(p, BRW_ALIGN_1);

      brw_MOV(p, get_element_ud(src, 2),
              brw_imm_ud(GEN9_SAMPLER_SIMD_MODE_EXTENSION_SIMD4X2));
      brw_pop_insn_state(p);
   }

   if (index.file == BRW_IMMEDIATE_VALUE) {

      uint32_t surf_index = index.dw1.ud;

      brw_push_insn_state(p);
      brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_inst *send = brw_next_insn(p, BRW_OPCODE_SEND);
      brw_pop_insn_state(p);

      brw_set_dest(p, send, dst);
      brw_set_src0(p, send, src);
      brw_set_sampler_message(p, send,
                              surf_index,
                              0, /* LD message ignores sampler unit */
                              GEN5_SAMPLER_MESSAGE_SAMPLE_LD,
                              1, /* rlen */
                              mlen,
                              header_present,
                              BRW_SAMPLER_SIMD_MODE_SIMD4X2,
                              0);

      brw_mark_surface_used(prog_data, surf_index);

   } else {

      struct brw_reg addr = vec1(retype(brw_address_reg(0), BRW_REGISTER_TYPE_UD));

      brw_push_insn_state(p);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_set_default_access_mode(p, BRW_ALIGN_1);

      /* a0.0 = surf_index & 0xff */
      brw_inst *insn_and = brw_next_insn(p, BRW_OPCODE_AND);
      brw_inst_set_exec_size(p->devinfo, insn_and, BRW_EXECUTE_1);
      brw_set_dest(p, insn_and, addr);
      brw_set_src0(p, insn_and, vec1(retype(index, BRW_REGISTER_TYPE_UD)));
      brw_set_src1(p, insn_and, brw_imm_ud(0x0ff));

      /* dst = send(payload, a0.0 | <descriptor>) */
      brw_inst *insn = brw_send_indirect_message(
         p, BRW_SFID_SAMPLER, dst, src, addr);
      brw_set_sampler_message(p, insn,
                              0,
                              0, /* LD message ignores sampler unit */
                              GEN5_SAMPLER_MESSAGE_SAMPLE_LD,
                              1, /* rlen */
                              mlen,
                              header_present,
                              BRW_SAMPLER_SIMD_MODE_SIMD4X2,
                              0);

      brw_pop_insn_state(p);

      /* visitor knows more than we do about the surface limit required,
       * so has already done marking.
       */

   }
}

void
fs_generator::generate_varying_pull_constant_load(fs_inst *inst,
                                                  struct brw_reg dst,
                                                  struct brw_reg index,
                                                  struct brw_reg offset)
{
   assert(devinfo->gen < 7); /* Should use the gen7 variant. */
   assert(inst->header_size != 0);
   assert(inst->mlen);

   assert(index.file == BRW_IMMEDIATE_VALUE &&
	  index.type == BRW_REGISTER_TYPE_UD);
   uint32_t surf_index = index.dw1.ud;

   uint32_t simd_mode, rlen, msg_type;
   if (dispatch_width == 16) {
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
      rlen = 8;
   } else {
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD8;
      rlen = 4;
   }

   if (devinfo->gen >= 5)
      msg_type = GEN5_SAMPLER_MESSAGE_SAMPLE_LD;
   else {
      /* We always use the SIMD16 message so that we only have to load U, and
       * not V or R.
       */
      msg_type = BRW_SAMPLER_MESSAGE_SIMD16_LD;
      assert(inst->mlen == 3);
      assert(inst->regs_written == 8);
      rlen = 8;
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
   }

   struct brw_reg offset_mrf = retype(brw_message_reg(inst->base_mrf + 1),
                                      BRW_REGISTER_TYPE_D);
   brw_MOV(p, offset_mrf, offset);

   struct brw_reg header = brw_vec8_grf(0, 0);
   gen6_resolve_implied_move(p, &header, inst->base_mrf);

   brw_inst *send = brw_next_insn(p, BRW_OPCODE_SEND);
   brw_inst_set_qtr_control(p->devinfo, send, BRW_COMPRESSION_NONE);
   brw_set_dest(p, send, retype(dst, BRW_REGISTER_TYPE_UW));
   brw_set_src0(p, send, header);
   if (devinfo->gen < 6)
      brw_inst_set_base_mrf(p->devinfo, send, inst->base_mrf);

   /* Our surface is set up as floats, regardless of what actual data is
    * stored in it.
    */
   uint32_t return_format = BRW_SAMPLER_RETURN_FORMAT_FLOAT32;
   brw_set_sampler_message(p, send,
                           surf_index,
                           0, /* sampler (unused) */
                           msg_type,
                           rlen,
                           inst->mlen,
                           inst->header_size != 0,
                           simd_mode,
                           return_format);

   brw_mark_surface_used(prog_data, surf_index);
}

void
fs_generator::generate_varying_pull_constant_load_gen7(fs_inst *inst,
                                                       struct brw_reg dst,
                                                       struct brw_reg index,
                                                       struct brw_reg offset)
{
   assert(devinfo->gen >= 7);
   /* Varying-offset pull constant loads are treated as a normal expression on
    * gen7, so the fact that it's a send message is hidden at the IR level.
    */
   assert(inst->header_size == 0);
   assert(!inst->mlen);
   assert(index.type == BRW_REGISTER_TYPE_UD);

   uint32_t simd_mode, rlen, mlen;
   if (dispatch_width == 16) {
      mlen = 2;
      rlen = 8;
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
   } else {
      mlen = 1;
      rlen = 4;
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD8;
   }

   if (index.file == BRW_IMMEDIATE_VALUE) {

      uint32_t surf_index = index.dw1.ud;

      brw_inst *send = brw_next_insn(p, BRW_OPCODE_SEND);
      brw_set_dest(p, send, retype(dst, BRW_REGISTER_TYPE_UW));
      brw_set_src0(p, send, offset);
      brw_set_sampler_message(p, send,
                              surf_index,
                              0, /* LD message ignores sampler unit */
                              GEN5_SAMPLER_MESSAGE_SAMPLE_LD,
                              rlen,
                              mlen,
                              false, /* no header */
                              simd_mode,
                              0);

      brw_mark_surface_used(prog_data, surf_index);

   } else {

      struct brw_reg addr = vec1(retype(brw_address_reg(0), BRW_REGISTER_TYPE_UD));

      brw_push_insn_state(p);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_set_default_access_mode(p, BRW_ALIGN_1);

      /* a0.0 = surf_index & 0xff */
      brw_inst *insn_and = brw_next_insn(p, BRW_OPCODE_AND);
      brw_inst_set_exec_size(p->devinfo, insn_and, BRW_EXECUTE_1);
      brw_set_dest(p, insn_and, addr);
      brw_set_src0(p, insn_and, vec1(retype(index, BRW_REGISTER_TYPE_UD)));
      brw_set_src1(p, insn_and, brw_imm_ud(0x0ff));

      brw_pop_insn_state(p);

      /* dst = send(offset, a0.0 | <descriptor>) */
      brw_inst *insn = brw_send_indirect_message(
         p, BRW_SFID_SAMPLER, retype(dst, BRW_REGISTER_TYPE_UW),
         offset, addr);
      brw_set_sampler_message(p, insn,
                              0 /* surface */,
                              0 /* sampler */,
                              GEN5_SAMPLER_MESSAGE_SAMPLE_LD,
                              rlen /* rlen */,
                              mlen /* mlen */,
                              false /* header */,
                              simd_mode,
                              0);

      /* visitor knows more than we do about the surface limit required,
       * so has already done marking.
       */
   }
}

/**
 * Cause the current pixel/sample mask (from R1.7 bits 15:0) to be transferred
 * into the flags register (f0.0).
 *
 * Used only on Gen6 and above.
 */
void
fs_generator::generate_mov_dispatch_to_flags(fs_inst *inst)
{
   struct brw_reg flags = brw_flag_reg(0, inst->flag_subreg);
   struct brw_reg dispatch_mask;

   if (devinfo->gen >= 6)
      dispatch_mask = retype(brw_vec1_grf(1, 7), BRW_REGISTER_TYPE_UW);
   else
      dispatch_mask = retype(brw_vec1_grf(0, 0), BRW_REGISTER_TYPE_UW);

   brw_push_insn_state(p);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, flags, dispatch_mask);
   brw_pop_insn_state(p);
}

void
fs_generator::generate_pixel_interpolator_query(fs_inst *inst,
                                                struct brw_reg dst,
                                                struct brw_reg src,
                                                struct brw_reg msg_data,
                                                unsigned msg_type)
{
   assert(msg_data.file == BRW_IMMEDIATE_VALUE &&
          msg_data.type == BRW_REGISTER_TYPE_UD);

   brw_pixel_interpolator_query(p,
         retype(dst, BRW_REGISTER_TYPE_UW),
         src,
         inst->pi_noperspective,
         msg_type,
         msg_data.dw1.ud,
         inst->mlen,
         inst->regs_written);
}


/**
 * Sets the first word of a vgrf for gen7+ simd4x2 uniform pull constant
 * sampler LD messages.
 *
 * We don't want to bake it into the send message's code generation because
 * that means we don't get a chance to schedule the instructions.
 */
void
fs_generator::generate_set_simd4x2_offset(fs_inst *inst,
                                          struct brw_reg dst,
                                          struct brw_reg value)
{
   assert(value.file == BRW_IMMEDIATE_VALUE);

   brw_push_insn_state(p);
   brw_set_default_exec_size(p, BRW_EXECUTE_8);
   brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, retype(brw_vec1_reg(dst.file, dst.nr, 0), value.type), value);
   brw_pop_insn_state(p);
}

/* Sets vstride=16, width=8, hstride=2 or vstride=0, width=1, hstride=0
 * (when mask is passed as a uniform) of register mask before moving it
 * to register dst.
 */
void
fs_generator::generate_set_omask(fs_inst *inst,
                                 struct brw_reg dst,
                                 struct brw_reg mask)
{
   bool stride_8_8_1 =
    (mask.vstride == BRW_VERTICAL_STRIDE_8 &&
     mask.width == BRW_WIDTH_8 &&
     mask.hstride == BRW_HORIZONTAL_STRIDE_1);

   bool stride_0_1_0 = has_scalar_region(mask);

   assert(stride_8_8_1 || stride_0_1_0);
   assert(dst.type == BRW_REGISTER_TYPE_UW);

   brw_push_insn_state(p);
   brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);

   if (stride_8_8_1) {
      brw_MOV(p, dst, retype(stride(mask, 16, 8, 2), dst.type));
   } else if (stride_0_1_0) {
      brw_MOV(p, dst, retype(mask, dst.type));
   }
   brw_pop_insn_state(p);
}

/* Sets vstride=1, width=4, hstride=0 of register src1 during
 * the ADD instruction.
 */
void
fs_generator::generate_set_sample_id(fs_inst *inst,
                                     struct brw_reg dst,
                                     struct brw_reg src0,
                                     struct brw_reg src1)
{
   assert(dst.type == BRW_REGISTER_TYPE_D ||
          dst.type == BRW_REGISTER_TYPE_UD);
   assert(src0.type == BRW_REGISTER_TYPE_D ||
          src0.type == BRW_REGISTER_TYPE_UD);

   brw_push_insn_state(p);
   brw_set_default_exec_size(p, BRW_EXECUTE_8);
   brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   struct brw_reg reg = retype(stride(src1, 1, 4, 0), BRW_REGISTER_TYPE_UW);
   if (dispatch_width == 8) {
      brw_ADD(p, dst, src0, reg);
   } else if (dispatch_width == 16) {
      brw_ADD(p, firsthalf(dst), firsthalf(src0), reg);
      brw_ADD(p, sechalf(dst), sechalf(src0), suboffset(reg, 2));
   }
   brw_pop_insn_state(p);
}

void
fs_generator::generate_pack_half_2x16_split(fs_inst *inst,
                                            struct brw_reg dst,
                                            struct brw_reg x,
                                            struct brw_reg y)
{
   assert(devinfo->gen >= 7);
   assert(dst.type == BRW_REGISTER_TYPE_UD);
   assert(x.type == BRW_REGISTER_TYPE_F);
   assert(y.type == BRW_REGISTER_TYPE_F);

   /* From the Ivybridge PRM, Vol4, Part3, Section 6.27 f32to16:
    *
    *   Because this instruction does not have a 16-bit floating-point type,
    *   the destination data type must be Word (W).
    *
    *   The destination must be DWord-aligned and specify a horizontal stride
    *   (HorzStride) of 2. The 16-bit result is stored in the lower word of
    *   each destination channel and the upper word is not modified.
    */
   struct brw_reg dst_w = spread(retype(dst, BRW_REGISTER_TYPE_W), 2);

   /* Give each 32-bit channel of dst the form below, where "." means
    * unchanged.
    *   0x....hhhh
    */
   brw_F32TO16(p, dst_w, y);

   /* Now the form:
    *   0xhhhh0000
    */
   brw_SHL(p, dst, dst, brw_imm_ud(16u));

   /* And, finally the form of packHalf2x16's output:
    *   0xhhhhllll
    */
   brw_F32TO16(p, dst_w, x);
}

void
fs_generator::generate_unpack_half_2x16_split(fs_inst *inst,
                                              struct brw_reg dst,
                                              struct brw_reg src)
{
   assert(devinfo->gen >= 7);
   assert(dst.type == BRW_REGISTER_TYPE_F);
   assert(src.type == BRW_REGISTER_TYPE_UD);

   /* From the Ivybridge PRM, Vol4, Part3, Section 6.26 f16to32:
    *
    *   Because this instruction does not have a 16-bit floating-point type,
    *   the source data type must be Word (W). The destination type must be
    *   F (Float).
    */
   struct brw_reg src_w = spread(retype(src, BRW_REGISTER_TYPE_W), 2);

   /* Each channel of src has the form of unpackHalf2x16's input: 0xhhhhllll.
    * For the Y case, we wish to access only the upper word; therefore
    * a 16-bit subregister offset is needed.
    */
   assert(inst->opcode == FS_OPCODE_UNPACK_HALF_2x16_SPLIT_X ||
          inst->opcode == FS_OPCODE_UNPACK_HALF_2x16_SPLIT_Y);
   if (inst->opcode == FS_OPCODE_UNPACK_HALF_2x16_SPLIT_Y)
      src_w.subnr += 2;

   brw_F16TO32(p, dst, src_w);
}

void
fs_generator::generate_shader_time_add(fs_inst *inst,
                                       struct brw_reg payload,
                                       struct brw_reg offset,
                                       struct brw_reg value)
{
   assert(devinfo->gen >= 7);
   brw_push_insn_state(p);
   brw_set_default_mask_control(p, true);

   assert(payload.file == BRW_GENERAL_REGISTER_FILE);
   struct brw_reg payload_offset = retype(brw_vec1_grf(payload.nr, 0),
                                          offset.type);
   struct brw_reg payload_value = retype(brw_vec1_grf(payload.nr + 1, 0),
                                         value.type);

   assert(offset.file == BRW_IMMEDIATE_VALUE);
   if (value.file == BRW_GENERAL_REGISTER_FILE) {
      value.width = BRW_WIDTH_1;
      value.hstride = BRW_HORIZONTAL_STRIDE_0;
      value.vstride = BRW_VERTICAL_STRIDE_0;
   } else {
      assert(value.file == BRW_IMMEDIATE_VALUE);
   }

   /* Trying to deal with setup of the params from the IR is crazy in the FS8
    * case, and we don't really care about squeezing every bit of performance
    * out of this path, so we just emit the MOVs from here.
    */
   brw_MOV(p, payload_offset, offset);
   brw_MOV(p, payload_value, value);
   brw_shader_time_add(p, payload,
                       prog_data->binding_table.shader_time_start);
   brw_pop_insn_state(p);

   brw_mark_surface_used(prog_data,
                         prog_data->binding_table.shader_time_start);
}

void
fs_generator::enable_debug(const char *shader_name)
{
   debug_flag = true;
   this->shader_name = shader_name;
}

int
fs_generator::generate_code(const cfg_t *cfg, int dispatch_width)
{
   /* align to 64 byte boundary. */
   while (p->next_insn_offset % 64)
      brw_NOP(p);

   this->dispatch_width = dispatch_width;
   if (dispatch_width == 16)
      brw_set_default_compression_control(p, BRW_COMPRESSION_COMPRESSED);

   int start_offset = p->next_insn_offset;
   int spill_count = 0, fill_count = 0;
   int loop_count = 0;

   struct annotation_info annotation;
   memset(&annotation, 0, sizeof(annotation));

   foreach_block_and_inst (block, fs_inst, inst, cfg) {
      struct brw_reg src[3], dst;
      unsigned int last_insn_offset = p->next_insn_offset;
      bool multiple_instructions_emitted = false;

      if (unlikely(debug_flag))
         annotate(p->devinfo, &annotation, cfg, inst, p->next_insn_offset);

      for (unsigned int i = 0; i < inst->sources; i++) {
	 src[i] = brw_reg_from_fs_reg(&inst->src[i]);

	 /* The accumulator result appears to get used for the
	  * conditional modifier generation.  When negating a UD
	  * value, there is a 33rd bit generated for the sign in the
	  * accumulator value, so now you can't check, for example,
	  * equality with a 32-bit value.  See piglit fs-op-neg-uvec4.
	  */
	 assert(!inst->conditional_mod ||
		inst->src[i].type != BRW_REGISTER_TYPE_UD ||
		!inst->src[i].negate);
      }
      dst = brw_reg_from_fs_reg(&inst->dst);

      brw_set_default_predicate_control(p, inst->predicate);
      brw_set_default_predicate_inverse(p, inst->predicate_inverse);
      brw_set_default_flag_reg(p, 0, inst->flag_subreg);
      brw_set_default_saturate(p, inst->saturate);
      brw_set_default_mask_control(p, inst->force_writemask_all);
      brw_set_default_acc_write_control(p, inst->writes_accumulator);
      brw_set_default_exec_size(p, cvt(inst->exec_size) - 1);

      switch (inst->exec_size) {
      case 1:
      case 2:
      case 4:
         assert(inst->force_writemask_all);
         brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
         break;
      case 8:
         if (inst->force_sechalf) {
            brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
         } else {
            brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
         }
         break;
      case 16:
      case 32:
         /* If the instruction writes to more than one register, it needs to
          * be a "compressed" instruction on Gen <= 5.
          */
         if (inst->exec_size * inst->dst.stride * type_sz(inst->dst.type) > 32)
            brw_set_default_compression_control(p, BRW_COMPRESSION_COMPRESSED);
         else
            brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
         break;
      default:
         unreachable("Invalid instruction width");
      }

      switch (inst->opcode) {
      case BRW_OPCODE_MOV:
	 brw_MOV(p, dst, src[0]);
	 break;
      case BRW_OPCODE_ADD:
	 brw_ADD(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_MUL:
	 brw_MUL(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_AVG:
	 brw_AVG(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_MACH:
	 brw_MACH(p, dst, src[0], src[1]);
	 break;

      case BRW_OPCODE_LINE:
         brw_LINE(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_MAD:
         assert(devinfo->gen >= 6);
	 brw_set_default_access_mode(p, BRW_ALIGN_16);
         if (dispatch_width == 16 && !devinfo->supports_simd16_3src) {
            brw_set_default_exec_size(p, BRW_EXECUTE_8);
	    brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
            brw_inst *f = brw_MAD(p, firsthalf(dst), firsthalf(src[0]), firsthalf(src[1]), firsthalf(src[2]));
	    brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
            brw_inst *s = brw_MAD(p, sechalf(dst), sechalf(src[0]), sechalf(src[1]), sechalf(src[2]));
	    brw_set_default_compression_control(p, BRW_COMPRESSION_COMPRESSED);

            if (inst->conditional_mod) {
               brw_inst_set_cond_modifier(p->devinfo, f, inst->conditional_mod);
               brw_inst_set_cond_modifier(p->devinfo, s, inst->conditional_mod);
               multiple_instructions_emitted = true;
            }
	 } else {
	    brw_MAD(p, dst, src[0], src[1], src[2]);
	 }
	 brw_set_default_access_mode(p, BRW_ALIGN_1);
	 break;

      case BRW_OPCODE_LRP:
         assert(devinfo->gen >= 6);
	 brw_set_default_access_mode(p, BRW_ALIGN_16);
         if (dispatch_width == 16 && !devinfo->supports_simd16_3src) {
            brw_set_default_exec_size(p, BRW_EXECUTE_8);
	    brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
            brw_inst *f = brw_LRP(p, firsthalf(dst), firsthalf(src[0]), firsthalf(src[1]), firsthalf(src[2]));
	    brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
            brw_inst *s = brw_LRP(p, sechalf(dst), sechalf(src[0]), sechalf(src[1]), sechalf(src[2]));
	    brw_set_default_compression_control(p, BRW_COMPRESSION_COMPRESSED);

            if (inst->conditional_mod) {
               brw_inst_set_cond_modifier(p->devinfo, f, inst->conditional_mod);
               brw_inst_set_cond_modifier(p->devinfo, s, inst->conditional_mod);
               multiple_instructions_emitted = true;
            }
	 } else {
	    brw_LRP(p, dst, src[0], src[1], src[2]);
	 }
	 brw_set_default_access_mode(p, BRW_ALIGN_1);
	 break;

      case BRW_OPCODE_FRC:
	 brw_FRC(p, dst, src[0]);
	 break;
      case BRW_OPCODE_RNDD:
	 brw_RNDD(p, dst, src[0]);
	 break;
      case BRW_OPCODE_RNDE:
	 brw_RNDE(p, dst, src[0]);
	 break;
      case BRW_OPCODE_RNDZ:
	 brw_RNDZ(p, dst, src[0]);
	 break;

      case BRW_OPCODE_AND:
	 brw_AND(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_OR:
	 brw_OR(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_XOR:
	 brw_XOR(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_NOT:
	 brw_NOT(p, dst, src[0]);
	 break;
      case BRW_OPCODE_ASR:
	 brw_ASR(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_SHR:
	 brw_SHR(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_SHL:
	 brw_SHL(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_F32TO16:
         assert(devinfo->gen >= 7);
         brw_F32TO16(p, dst, src[0]);
         break;
      case BRW_OPCODE_F16TO32:
         assert(devinfo->gen >= 7);
         brw_F16TO32(p, dst, src[0]);
         break;
      case BRW_OPCODE_CMP:
         /* The Ivybridge/BayTrail WaCMPInstFlagDepClearedEarly workaround says
          * that when the destination is a GRF that the dependency-clear bit on
          * the flag register is cleared early.
          *
          * Suggested workarounds are to disable coissuing CMP instructions
          * or to split CMP(16) instructions into two CMP(8) instructions.
          *
          * We choose to split into CMP(8) instructions since disabling
          * coissuing would affect CMP instructions not otherwise affected by
          * the errata.
          */
         if (dispatch_width == 16 && devinfo->gen == 7 && !devinfo->is_haswell) {
            if (dst.file == BRW_GENERAL_REGISTER_FILE) {
               brw_set_default_exec_size(p, BRW_EXECUTE_8);
               brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
               brw_CMP(p, firsthalf(dst), inst->conditional_mod,
                          firsthalf(src[0]), firsthalf(src[1]));
               brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
               brw_CMP(p, sechalf(dst), inst->conditional_mod,
                          sechalf(src[0]), sechalf(src[1]));
               brw_set_default_compression_control(p, BRW_COMPRESSION_COMPRESSED);

               multiple_instructions_emitted = true;
            } else if (dst.file == BRW_ARCHITECTURE_REGISTER_FILE) {
               /* For unknown reasons, the aforementioned workaround is not
                * sufficient. Overriding the type when the destination is the
                * null register is necessary but not sufficient by itself.
                */
               assert(dst.nr == BRW_ARF_NULL);
               dst.type = BRW_REGISTER_TYPE_D;
               brw_CMP(p, dst, inst->conditional_mod, src[0], src[1]);
            } else {
               unreachable("not reached");
            }
         } else {
            brw_CMP(p, dst, inst->conditional_mod, src[0], src[1]);
         }
	 break;
      case BRW_OPCODE_SEL:
	 brw_SEL(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_BFREV:
         assert(devinfo->gen >= 7);
         /* BFREV only supports UD type for src and dst. */
         brw_BFREV(p, retype(dst, BRW_REGISTER_TYPE_UD),
                      retype(src[0], BRW_REGISTER_TYPE_UD));
         break;
      case BRW_OPCODE_FBH:
         assert(devinfo->gen >= 7);
         /* FBH only supports UD type for dst. */
         brw_FBH(p, retype(dst, BRW_REGISTER_TYPE_UD), src[0]);
         break;
      case BRW_OPCODE_FBL:
         assert(devinfo->gen >= 7);
         /* FBL only supports UD type for dst. */
         brw_FBL(p, retype(dst, BRW_REGISTER_TYPE_UD), src[0]);
         break;
      case BRW_OPCODE_CBIT:
         assert(devinfo->gen >= 7);
         /* CBIT only supports UD type for dst. */
         brw_CBIT(p, retype(dst, BRW_REGISTER_TYPE_UD), src[0]);
         break;
      case BRW_OPCODE_ADDC:
         assert(devinfo->gen >= 7);
         brw_ADDC(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_SUBB:
         assert(devinfo->gen >= 7);
         brw_SUBB(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_MAC:
         brw_MAC(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_BFE:
         assert(devinfo->gen >= 7);
         brw_set_default_access_mode(p, BRW_ALIGN_16);
         if (dispatch_width == 16 && !devinfo->supports_simd16_3src) {
            brw_set_default_exec_size(p, BRW_EXECUTE_8);
            brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
            brw_BFE(p, firsthalf(dst), firsthalf(src[0]), firsthalf(src[1]), firsthalf(src[2]));
            brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
            brw_BFE(p, sechalf(dst), sechalf(src[0]), sechalf(src[1]), sechalf(src[2]));
            brw_set_default_compression_control(p, BRW_COMPRESSION_COMPRESSED);
         } else {
            brw_BFE(p, dst, src[0], src[1], src[2]);
         }
         brw_set_default_access_mode(p, BRW_ALIGN_1);
         break;

      case BRW_OPCODE_BFI1:
         assert(devinfo->gen >= 7);
         /* The Haswell WaForceSIMD8ForBFIInstruction workaround says that we
          * should
          *
          *    "Force BFI instructions to be executed always in SIMD8."
          */
         if (dispatch_width == 16 && devinfo->is_haswell) {
            brw_set_default_exec_size(p, BRW_EXECUTE_8);
            brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
            brw_BFI1(p, firsthalf(dst), firsthalf(src[0]), firsthalf(src[1]));
            brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
            brw_BFI1(p, sechalf(dst), sechalf(src[0]), sechalf(src[1]));
            brw_set_default_compression_control(p, BRW_COMPRESSION_COMPRESSED);
         } else {
            brw_BFI1(p, dst, src[0], src[1]);
         }
         break;
      case BRW_OPCODE_BFI2:
         assert(devinfo->gen >= 7);
         brw_set_default_access_mode(p, BRW_ALIGN_16);
         /* The Haswell WaForceSIMD8ForBFIInstruction workaround says that we
          * should
          *
          *    "Force BFI instructions to be executed always in SIMD8."
          *
          * Otherwise we would be able to emit compressed instructions like we
          * do for the other three-source instructions.
          */
         if (dispatch_width == 16 &&
             (devinfo->is_haswell || !devinfo->supports_simd16_3src)) {
            brw_set_default_exec_size(p, BRW_EXECUTE_8);
            brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
            brw_BFI2(p, firsthalf(dst), firsthalf(src[0]), firsthalf(src[1]), firsthalf(src[2]));
            brw_set_default_compression_control(p, BRW_COMPRESSION_2NDHALF);
            brw_BFI2(p, sechalf(dst), sechalf(src[0]), sechalf(src[1]), sechalf(src[2]));
            brw_set_default_compression_control(p, BRW_COMPRESSION_COMPRESSED);
         } else {
            brw_BFI2(p, dst, src[0], src[1], src[2]);
         }
         brw_set_default_access_mode(p, BRW_ALIGN_1);
         break;

      case BRW_OPCODE_IF:
	 if (inst->src[0].file != BAD_FILE) {
	    /* The instruction has an embedded compare (only allowed on gen6) */
	    assert(devinfo->gen == 6);
	    gen6_IF(p, inst->conditional_mod, src[0], src[1]);
	 } else {
	    brw_IF(p, dispatch_width == 16 ? BRW_EXECUTE_16 : BRW_EXECUTE_8);
	 }
	 break;

      case BRW_OPCODE_ELSE:
	 brw_ELSE(p);
	 break;
      case BRW_OPCODE_ENDIF:
	 brw_ENDIF(p);
	 break;

      case BRW_OPCODE_DO:
	 brw_DO(p, BRW_EXECUTE_8);
	 break;

      case BRW_OPCODE_BREAK:
	 brw_BREAK(p);
	 brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
	 break;
      case BRW_OPCODE_CONTINUE:
         brw_CONT(p);
	 brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
	 break;

      case BRW_OPCODE_WHILE:
	 brw_WHILE(p);
         loop_count++;
	 break;

      case SHADER_OPCODE_RCP:
      case SHADER_OPCODE_RSQ:
      case SHADER_OPCODE_SQRT:
      case SHADER_OPCODE_EXP2:
      case SHADER_OPCODE_LOG2:
      case SHADER_OPCODE_SIN:
      case SHADER_OPCODE_COS:
         assert(devinfo->gen < 6 || inst->mlen == 0);
         assert(inst->conditional_mod == BRW_CONDITIONAL_NONE);
	 if (devinfo->gen >= 7) {
            gen6_math(p, dst, brw_math_function(inst->opcode), src[0],
                      brw_null_reg());
	 } else if (devinfo->gen == 6) {
	    generate_math_gen6(inst, dst, src[0], brw_null_reg());
	 } else if (devinfo->gen == 5 || devinfo->is_g4x) {
	    generate_math_g45(inst, dst, src[0]);
	 } else {
	    generate_math_gen4(inst, dst, src[0]);
	 }
	 break;
      case SHADER_OPCODE_INT_QUOTIENT:
      case SHADER_OPCODE_INT_REMAINDER:
      case SHADER_OPCODE_POW:
         assert(devinfo->gen < 6 || inst->mlen == 0);
         assert(inst->conditional_mod == BRW_CONDITIONAL_NONE);
	 if (devinfo->gen >= 7 && inst->opcode == SHADER_OPCODE_POW) {
            gen6_math(p, dst, brw_math_function(inst->opcode), src[0], src[1]);
	 } else if (devinfo->gen >= 6) {
	    generate_math_gen6(inst, dst, src[0], src[1]);
	 } else {
	    generate_math_gen4(inst, dst, src[0]);
	 }
	 break;
      case FS_OPCODE_CINTERP:
	 brw_MOV(p, dst, src[0]);
	 break;
      case FS_OPCODE_LINTERP:
	 generate_linterp(inst, dst, src);
	 break;
      case FS_OPCODE_PIXEL_X:
         assert(src[0].type == BRW_REGISTER_TYPE_UW);
         src[0].subnr = 0 * type_sz(src[0].type);
         brw_MOV(p, dst, stride(src[0], 8, 4, 1));
         break;
      case FS_OPCODE_PIXEL_Y:
         assert(src[0].type == BRW_REGISTER_TYPE_UW);
         src[0].subnr = 4 * type_sz(src[0].type);
         brw_MOV(p, dst, stride(src[0], 8, 4, 1));
         break;
      case SHADER_OPCODE_TEX:
      case FS_OPCODE_TXB:
      case SHADER_OPCODE_TXD:
      case SHADER_OPCODE_TXF:
      case SHADER_OPCODE_TXF_CMS:
      case SHADER_OPCODE_TXF_UMS:
      case SHADER_OPCODE_TXF_MCS:
      case SHADER_OPCODE_TXL:
      case SHADER_OPCODE_TXS:
      case SHADER_OPCODE_LOD:
      case SHADER_OPCODE_TG4:
      case SHADER_OPCODE_TG4_OFFSET:
	 generate_tex(inst, dst, src[0], src[1]);
	 break;
      case FS_OPCODE_DDX_COARSE:
      case FS_OPCODE_DDX_FINE:
         generate_ddx(inst->opcode, dst, src[0]);
         break;
      case FS_OPCODE_DDY_COARSE:
      case FS_OPCODE_DDY_FINE:
         assert(src[1].file == BRW_IMMEDIATE_VALUE);
         generate_ddy(inst->opcode, dst, src[0], src[1].dw1.ud);
	 break;

      case SHADER_OPCODE_GEN4_SCRATCH_WRITE:
	 generate_scratch_write(inst, src[0]);
         spill_count++;
	 break;

      case SHADER_OPCODE_GEN4_SCRATCH_READ:
	 generate_scratch_read(inst, dst);
         fill_count++;
	 break;

      case SHADER_OPCODE_GEN7_SCRATCH_READ:
	 generate_scratch_read_gen7(inst, dst);
         fill_count++;
	 break;

      case SHADER_OPCODE_URB_WRITE_SIMD8:
	 generate_urb_write(inst, src[0]);
	 break;

      case FS_OPCODE_UNIFORM_PULL_CONSTANT_LOAD:
	 generate_uniform_pull_constant_load(inst, dst, src[0], src[1]);
	 break;

      case FS_OPCODE_UNIFORM_PULL_CONSTANT_LOAD_GEN7:
	 generate_uniform_pull_constant_load_gen7(inst, dst, src[0], src[1]);
	 break;

      case FS_OPCODE_VARYING_PULL_CONSTANT_LOAD:
	 generate_varying_pull_constant_load(inst, dst, src[0], src[1]);
	 break;

      case FS_OPCODE_VARYING_PULL_CONSTANT_LOAD_GEN7:
	 generate_varying_pull_constant_load_gen7(inst, dst, src[0], src[1]);
	 break;

      case FS_OPCODE_REP_FB_WRITE:
      case FS_OPCODE_FB_WRITE:
	 generate_fb_write(inst, src[0]);
	 break;

      case FS_OPCODE_BLORP_FB_WRITE:
	 generate_blorp_fb_write(inst);
	 break;

      case FS_OPCODE_MOV_DISPATCH_TO_FLAGS:
         generate_mov_dispatch_to_flags(inst);
         break;

      case FS_OPCODE_DISCARD_JUMP:
         generate_discard_jump(inst);
         break;

      case SHADER_OPCODE_SHADER_TIME_ADD:
         generate_shader_time_add(inst, src[0], src[1], src[2]);
         break;

      case SHADER_OPCODE_UNTYPED_ATOMIC:
         assert(src[1].file == BRW_IMMEDIATE_VALUE &&
                src[2].file == BRW_IMMEDIATE_VALUE);
         brw_untyped_atomic(p, dst, src[0], src[1], src[2].dw1.ud,
                            inst->mlen, !inst->dst.is_null());
         brw_mark_surface_used(prog_data, src[1].dw1.ud);
         break;

      case SHADER_OPCODE_UNTYPED_SURFACE_READ:
         assert(src[1].file == BRW_IMMEDIATE_VALUE &&
                src[2].file == BRW_IMMEDIATE_VALUE);
         brw_untyped_surface_read(p, dst, src[0], src[1],
                                  inst->mlen, src[2].dw1.ud);
         brw_mark_surface_used(prog_data, src[1].dw1.ud);
         break;

      case SHADER_OPCODE_UNTYPED_SURFACE_WRITE:
         assert(src[2].file == BRW_IMMEDIATE_VALUE);
         brw_untyped_surface_write(p, src[0], src[1],
                                   inst->mlen, src[2].dw1.ud);
         break;

      case SHADER_OPCODE_TYPED_ATOMIC:
         assert(src[2].file == BRW_IMMEDIATE_VALUE);
         brw_typed_atomic(p, dst, src[0], src[1],
                          src[2].dw1.ud, inst->mlen, !inst->dst.is_null());
         break;

      case SHADER_OPCODE_TYPED_SURFACE_READ:
         assert(src[2].file == BRW_IMMEDIATE_VALUE);
         brw_typed_surface_read(p, dst, src[0], src[1],
                                inst->mlen, src[2].dw1.ud);
         break;

      case SHADER_OPCODE_TYPED_SURFACE_WRITE:
         assert(src[2].file == BRW_IMMEDIATE_VALUE);
         brw_typed_surface_write(p, src[0], src[1], inst->mlen, src[2].dw1.ud);
         break;

      case SHADER_OPCODE_MEMORY_FENCE:
         brw_memory_fence(p, dst);
         break;

      case FS_OPCODE_SET_SIMD4X2_OFFSET:
         generate_set_simd4x2_offset(inst, dst, src[0]);
         break;

      case SHADER_OPCODE_FIND_LIVE_CHANNEL:
         brw_find_live_channel(p, dst);
         break;

      case SHADER_OPCODE_BROADCAST:
         brw_broadcast(p, dst, src[0], src[1]);
         break;

      case FS_OPCODE_SET_OMASK:
         generate_set_omask(inst, dst, src[0]);
         break;

      case FS_OPCODE_SET_SAMPLE_ID:
         generate_set_sample_id(inst, dst, src[0], src[1]);
         break;

      case FS_OPCODE_PACK_HALF_2x16_SPLIT:
          generate_pack_half_2x16_split(inst, dst, src[0], src[1]);
          break;

      case FS_OPCODE_UNPACK_HALF_2x16_SPLIT_X:
      case FS_OPCODE_UNPACK_HALF_2x16_SPLIT_Y:
         generate_unpack_half_2x16_split(inst, dst, src[0]);
         break;

      case FS_OPCODE_PLACEHOLDER_HALT:
         /* This is the place where the final HALT needs to be inserted if
          * we've emitted any discards.  If not, this will emit no code.
          */
         if (!patch_discard_jumps_to_fb_writes()) {
            if (unlikely(debug_flag)) {
               annotation.ann_count--;
            }
         }
         break;

      case FS_OPCODE_INTERPOLATE_AT_CENTROID:
         generate_pixel_interpolator_query(inst, dst, src[0], src[1],
                                           GEN7_PIXEL_INTERPOLATOR_LOC_CENTROID);
         break;

      case FS_OPCODE_INTERPOLATE_AT_SAMPLE:
         generate_pixel_interpolator_query(inst, dst, src[0], src[1],
                                           GEN7_PIXEL_INTERPOLATOR_LOC_SAMPLE);
         break;

      case FS_OPCODE_INTERPOLATE_AT_SHARED_OFFSET:
         generate_pixel_interpolator_query(inst, dst, src[0], src[1],
                                           GEN7_PIXEL_INTERPOLATOR_LOC_SHARED_OFFSET);
         break;

      case FS_OPCODE_INTERPOLATE_AT_PER_SLOT_OFFSET:
         generate_pixel_interpolator_query(inst, dst, src[0], src[1],
                                           GEN7_PIXEL_INTERPOLATOR_LOC_PER_SLOT_OFFSET);
         break;

      case CS_OPCODE_CS_TERMINATE:
         generate_cs_terminate(inst, src[0]);
         break;

      default:
         unreachable("Unsupported opcode");

      case SHADER_OPCODE_LOAD_PAYLOAD:
         unreachable("Should be lowered by lower_load_payload()");
      }

      if (multiple_instructions_emitted)
         continue;

      if (inst->no_dd_clear || inst->no_dd_check || inst->conditional_mod) {
         assert(p->next_insn_offset == last_insn_offset + 16 ||
                !"conditional_mod, no_dd_check, or no_dd_clear set for IR "
                 "emitting more than 1 instruction");

         brw_inst *last = &p->store[last_insn_offset / 16];

         if (inst->conditional_mod)
            brw_inst_set_cond_modifier(p->devinfo, last, inst->conditional_mod);
         brw_inst_set_no_dd_clear(p->devinfo, last, inst->no_dd_clear);
         brw_inst_set_no_dd_check(p->devinfo, last, inst->no_dd_check);
      }
   }

   brw_set_uip_jip(p);
   annotation_finalize(&annotation, p->next_insn_offset);

   int before_size = p->next_insn_offset - start_offset;
   brw_compact_instructions(p, start_offset, annotation.ann_count,
                            annotation.ann);
   int after_size = p->next_insn_offset - start_offset;

   if (unlikely(debug_flag)) {
      fprintf(stderr, "Native code for %s\n"
              "SIMD%d shader: %d instructions. %d loops. %d:%d spills:fills. Promoted %u constants. Compacted %d to %d"
              " bytes (%.0f%%)\n",
              shader_name, dispatch_width, before_size / 16, loop_count,
              spill_count, fill_count, promoted_constants, before_size, after_size,
              100.0f * (before_size - after_size) / before_size);

      dump_assembly(p->store, annotation.ann_count, annotation.ann,
                    p->devinfo, prog);
      ralloc_free(annotation.ann);
   }

   static GLuint msg_id = 0;
   _mesa_gl_debug(&brw->ctx, &msg_id,
                  MESA_DEBUG_SOURCE_SHADER_COMPILER,
                  MESA_DEBUG_TYPE_OTHER,
                  MESA_DEBUG_SEVERITY_NOTIFICATION,
                  "%s SIMD%d shader: %d inst, %d loops, %d:%d spills:fills, "
                  "Promoted %u constants, compacted %d to %d bytes.\n",
                  stage_abbrev, dispatch_width, before_size / 16, loop_count,
                  spill_count, fill_count, promoted_constants, before_size, after_size);

   return start_offset;
}

const unsigned *
fs_generator::get_assembly(unsigned int *assembly_size)
{
   return brw_get_program(p, assembly_size);
}
