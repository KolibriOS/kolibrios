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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#pragma once

#include "brw_shader.h"
#include "brw_ir_fs.h"

extern "C" {

#include <sys/types.h>

#include "main/macros.h"
#include "main/shaderobj.h"
#include "main/uniforms.h"
#include "program/prog_parameter.h"
#include "program/prog_print.h"
#include "program/prog_optimize.h"
#include "util/register_allocate.h"
#include "program/hash_table.h"
#include "brw_context.h"
#include "brw_eu.h"
#include "brw_wm.h"
#include "intel_asm_annotation.h"
}
#include "glsl/glsl_types.h"
#include "glsl/ir.h"
#include "glsl/nir/nir.h"
#include "program/sampler.h"

struct bblock_t;
namespace {
   struct acp_entry;
}

namespace brw {
   class fs_live_variables;
}

/**
 * The fragment shader front-end.
 *
 * Translates either GLSL IR or Mesa IR (for ARB_fragment_program) into FS IR.
 */
class fs_visitor : public backend_visitor
{
public:
   const fs_reg reg_null_f;
   const fs_reg reg_null_d;
   const fs_reg reg_null_ud;

   fs_visitor(struct brw_context *brw,
              void *mem_ctx,
              gl_shader_stage stage,
              const void *key,
              struct brw_stage_prog_data *prog_data,
              struct gl_shader_program *shader_prog,
              struct gl_program *prog,
              unsigned dispatch_width);

   ~fs_visitor();

   fs_reg *variable_storage(ir_variable *var);
   fs_reg vgrf(const glsl_type *const type);
   fs_reg vgrf(int num_components);
   void import_uniforms(fs_visitor *v);
   void setup_uniform_clipplane_values();
   void compute_clip_distance();

   void visit(ir_variable *ir);
   void visit(ir_assignment *ir);
   void visit(ir_dereference_variable *ir);
   void visit(ir_dereference_record *ir);
   void visit(ir_dereference_array *ir);
   void visit(ir_expression *ir);
   void visit(ir_texture *ir);
   void visit(ir_if *ir);
   void visit(ir_constant *ir);
   void visit(ir_swizzle *ir);
   void visit(ir_return *ir);
   void visit(ir_loop *ir);
   void visit(ir_loop_jump *ir);
   void visit(ir_discard *ir);
   void visit(ir_call *ir);
   void visit(ir_function *ir);
   void visit(ir_function_signature *ir);
   void visit(ir_emit_vertex *);
   void visit(ir_end_primitive *);

   uint32_t gather_channel(int orig_chan, uint32_t sampler);
   void swizzle_result(ir_texture_opcode op, int dest_components,
                       fs_reg orig_val, uint32_t sampler);

   fs_inst *emit(fs_inst *inst);
   void emit(exec_list list);

   fs_inst *emit(enum opcode opcode);
   fs_inst *emit(enum opcode opcode, const fs_reg &dst);
   fs_inst *emit(enum opcode opcode, const fs_reg &dst, const fs_reg &src0);
   fs_inst *emit(enum opcode opcode, const fs_reg &dst, const fs_reg &src0,
                 const fs_reg &src1);
   fs_inst *emit(enum opcode opcode, const fs_reg &dst,
                 const fs_reg &src0, const fs_reg &src1, const fs_reg &src2);
   fs_inst *emit(enum opcode opcode, const fs_reg &dst,
                 fs_reg src[], int sources);

   fs_inst *MOV(const fs_reg &dst, const fs_reg &src);
   fs_inst *NOT(const fs_reg &dst, const fs_reg &src);
   fs_inst *RNDD(const fs_reg &dst, const fs_reg &src);
   fs_inst *RNDE(const fs_reg &dst, const fs_reg &src);
   fs_inst *RNDZ(const fs_reg &dst, const fs_reg &src);
   fs_inst *FRC(const fs_reg &dst, const fs_reg &src);
   fs_inst *ADD(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *MUL(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *MACH(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *MAC(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *SHL(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *SHR(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *ASR(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *AND(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *OR(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *XOR(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *IF(enum brw_predicate predicate);
   fs_inst *IF(const fs_reg &src0, const fs_reg &src1,
               enum brw_conditional_mod condition);
   fs_inst *CMP(fs_reg dst, fs_reg src0, fs_reg src1,
                enum brw_conditional_mod condition);
   fs_inst *LRP(const fs_reg &dst, const fs_reg &a, const fs_reg &y,
                const fs_reg &x);
   fs_inst *DEP_RESOLVE_MOV(int grf);
   fs_inst *BFREV(const fs_reg &dst, const fs_reg &value);
   fs_inst *BFE(const fs_reg &dst, const fs_reg &bits, const fs_reg &offset,
                const fs_reg &value);
   fs_inst *BFI1(const fs_reg &dst, const fs_reg &bits, const fs_reg &offset);
   fs_inst *BFI2(const fs_reg &dst, const fs_reg &bfi1_dst,
                 const fs_reg &insert, const fs_reg &base);
   fs_inst *FBH(const fs_reg &dst, const fs_reg &value);
   fs_inst *FBL(const fs_reg &dst, const fs_reg &value);
   fs_inst *CBIT(const fs_reg &dst, const fs_reg &value);
   fs_inst *MAD(const fs_reg &dst, const fs_reg &c, const fs_reg &b,
                const fs_reg &a);
   fs_inst *ADDC(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *SUBB(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);
   fs_inst *SEL(const fs_reg &dst, const fs_reg &src0, const fs_reg &src1);

   int type_size(const struct glsl_type *type);
   fs_inst *get_instruction_generating_reg(fs_inst *start,
					   fs_inst *end,
					   const fs_reg &reg);

   fs_inst *LOAD_PAYLOAD(const fs_reg &dst, fs_reg *src, int sources,
                         int header_size);

   exec_list VARYING_PULL_CONSTANT_LOAD(const fs_reg &dst,
                                        const fs_reg &surf_index,
                                        const fs_reg &varying_offset,
                                        uint32_t const_offset);

   bool run_fs();
   bool run_vs();
   bool run_cs();
   void optimize();
   void allocate_registers();
   void assign_binding_table_offsets();
   void setup_payload_gen4();
   void setup_payload_gen6();
   void setup_vs_payload();
   void setup_cs_payload();
   void fixup_3src_null_dest();
   void assign_curb_setup();
   void calculate_urb_setup();
   void assign_urb_setup();
   void assign_vs_urb_setup();
   bool assign_regs(bool allow_spilling);
   void assign_regs_trivial();
   void get_used_mrfs(bool *mrf_used);
   void setup_payload_interference(struct ra_graph *g, int payload_reg_count,
                                   int first_payload_node);
   void setup_mrf_hack_interference(struct ra_graph *g,
                                    int first_mrf_hack_node);
   int choose_spill_reg(struct ra_graph *g);
   void spill_reg(int spill_reg);
   void split_virtual_grfs();
   bool compact_virtual_grfs();
   void move_uniform_array_access_to_pull_constants();
   void assign_constant_locations();
   void demote_pull_constants();
   void invalidate_live_intervals();
   void calculate_live_intervals();
   void calculate_register_pressure();
   bool opt_algebraic();
   bool opt_redundant_discard_jumps();
   bool opt_cse();
   bool opt_cse_local(bblock_t *block);
   bool opt_copy_propagate();
   bool try_copy_propagate(fs_inst *inst, int arg, acp_entry *entry);
   bool try_constant_propagate(fs_inst *inst, acp_entry *entry);
   bool opt_copy_propagate_local(void *mem_ctx, bblock_t *block,
                                 exec_list *acp);
   bool opt_register_renaming();
   bool register_coalesce();
   bool compute_to_mrf();
   bool eliminate_find_live_channel();
   bool dead_code_eliminate();
   bool remove_duplicate_mrf_writes();

   bool opt_sampler_eot();
   bool virtual_grf_interferes(int a, int b);
   void schedule_instructions(instruction_scheduler_mode mode);
   void insert_gen4_send_dependency_workarounds();
   void insert_gen4_pre_send_dependency_workarounds(bblock_t *block,
                                                    fs_inst *inst);
   void insert_gen4_post_send_dependency_workarounds(bblock_t *block,
                                                     fs_inst *inst);
   void vfail(const char *msg, va_list args);
   void fail(const char *msg, ...);
   void no16(const char *msg, ...);
   void lower_uniform_pull_constant_loads();
   bool lower_load_payload();
   bool lower_integer_multiplication();
   bool opt_combine_constants();

   void emit_dummy_fs();
   void emit_repclear_shader();
   fs_reg *emit_fragcoord_interpolation(bool pixel_center_integer,
                                        bool origin_upper_left);
   fs_inst *emit_linterp(const fs_reg &attr, const fs_reg &interp,
                         glsl_interp_qualifier interpolation_mode,
                         bool is_centroid, bool is_sample);
   fs_reg *emit_frontfacing_interpolation();
   fs_reg *emit_samplepos_setup();
   fs_reg *emit_sampleid_setup();
   void emit_general_interpolation(fs_reg attr, const char *name,
                                   const glsl_type *type,
                                   glsl_interp_qualifier interpolation_mode,
                                   int location, bool mod_centroid,
                                   bool mod_sample);
   fs_reg *emit_vs_system_value(int location);
   void emit_interpolation_setup_gen4();
   void emit_interpolation_setup_gen6();
   void compute_sample_position(fs_reg dst, fs_reg int_sample_pos);
   fs_reg rescale_texcoord(fs_reg coordinate, int coord_components,
                           bool is_rect, uint32_t sampler, int texunit);
   fs_inst *emit_texture_gen4(ir_texture_opcode op, fs_reg dst,
                              fs_reg coordinate, int coord_components,
                              fs_reg shadow_comp,
                              fs_reg lod, fs_reg lod2, int grad_components,
                              uint32_t sampler);
   fs_inst *emit_texture_gen4_simd16(ir_texture_opcode op, fs_reg dst,
                                     fs_reg coordinate, int vector_elements,
                                     fs_reg shadow_c, fs_reg lod,
                                     uint32_t sampler);
   fs_inst *emit_texture_gen5(ir_texture_opcode op, fs_reg dst,
                              fs_reg coordinate, int coord_components,
                              fs_reg shadow_comp,
                              fs_reg lod, fs_reg lod2, int grad_components,
                              fs_reg sample_index, uint32_t sampler,
                              bool has_offset);
   fs_inst *emit_texture_gen7(ir_texture_opcode op, fs_reg dst,
                              fs_reg coordinate, int coord_components,
                              fs_reg shadow_comp,
                              fs_reg lod, fs_reg lod2, int grad_components,
                              fs_reg sample_index, fs_reg mcs, fs_reg sampler,
                              fs_reg offset_value);
   void emit_texture(ir_texture_opcode op,
                     const glsl_type *dest_type,
                     fs_reg coordinate, int components,
                     fs_reg shadow_c,
                     fs_reg lod, fs_reg dpdy, int grad_components,
                     fs_reg sample_index,
                     fs_reg offset,
                     fs_reg mcs,
                     int gather_component,
                     bool is_cube_array,
                     bool is_rect,
                     uint32_t sampler,
                     fs_reg sampler_reg,
                     int texunit);
   fs_reg emit_mcs_fetch(fs_reg coordinate, int components, fs_reg sampler);
   void emit_gen6_gather_wa(uint8_t wa, fs_reg dst);
   void resolve_source_modifiers(fs_reg *src);
   fs_reg fix_math_operand(fs_reg src);
   fs_inst *emit_math(enum opcode op, fs_reg dst, fs_reg src0);
   fs_inst *emit_math(enum opcode op, fs_reg dst, fs_reg src0, fs_reg src1);
   fs_inst *emit_lrp(const fs_reg &dst, const fs_reg &x, const fs_reg &y,
                     const fs_reg &a);
   void emit_minmax(enum brw_conditional_mod conditionalmod, const fs_reg &dst,
                    const fs_reg &src0, const fs_reg &src1);
   void emit_discard_jump();
   /** Copy any live channel from \p src to the first channel of \p dst. */
   void emit_uniformize(const fs_reg &dst, const fs_reg &src);
   bool try_emit_b2f_of_comparison(ir_expression *ir);
   bool try_emit_saturate(ir_expression *ir);
   bool try_emit_line(ir_expression *ir);
   bool try_emit_mad(ir_expression *ir);
   bool try_replace_with_sel();
   bool try_opt_frontfacing_ternary(ir_if *ir);
   bool opt_peephole_sel();
   bool opt_peephole_predicated_break();
   bool opt_saturate_propagation();
   bool opt_cmod_propagation();
   bool opt_zero_samples();
   void emit_bool_to_cond_code(ir_rvalue *condition);
   void emit_bool_to_cond_code_of_reg(ir_expression *expr, fs_reg op[3]);
   void emit_if_gen6(ir_if *ir);
   void emit_unspill(bblock_t *block, fs_inst *inst, fs_reg reg,
                     uint32_t spill_offset, int count);
   void emit_spill(bblock_t *block, fs_inst *inst, fs_reg reg,
                   uint32_t spill_offset, int count);

   void emit_fragment_program_code();
   void setup_fp_regs();
   fs_reg get_fp_src_reg(const prog_src_register *src);
   fs_reg get_fp_dst_reg(const prog_dst_register *dst);
   void emit_fp_alu1(enum opcode opcode,
                     const struct prog_instruction *fpi,
                     fs_reg dst, fs_reg src);
   void emit_fp_alu2(enum opcode opcode,
                     const struct prog_instruction *fpi,
                     fs_reg dst, fs_reg src0, fs_reg src1);
   void emit_fp_scalar_write(const struct prog_instruction *fpi,
                             fs_reg dst, fs_reg src);
   void emit_fp_scalar_math(enum opcode opcode,
                            const struct prog_instruction *fpi,
                            fs_reg dst, fs_reg src);

   void emit_fp_minmax(const struct prog_instruction *fpi,
                       fs_reg dst, fs_reg src0, fs_reg src1);

   void emit_fp_sop(enum brw_conditional_mod conditional_mod,
                    const struct prog_instruction *fpi,
                    fs_reg dst, fs_reg src0, fs_reg src1, fs_reg one);

   void emit_nir_code();
   void nir_setup_inputs(nir_shader *shader);
   void nir_setup_outputs(nir_shader *shader);
   void nir_setup_uniforms(nir_shader *shader);
   void nir_setup_uniform(nir_variable *var);
   void nir_setup_builtin_uniform(nir_variable *var);
   void nir_emit_system_values(nir_shader *shader);
   void nir_emit_impl(nir_function_impl *impl);
   void nir_emit_cf_list(exec_list *list);
   void nir_emit_if(nir_if *if_stmt);
   void nir_emit_loop(nir_loop *loop);
   void nir_emit_block(nir_block *block);
   void nir_emit_instr(nir_instr *instr);
   void nir_emit_alu(nir_alu_instr *instr);
   void nir_emit_intrinsic(nir_intrinsic_instr *instr);
   void nir_emit_texture(nir_tex_instr *instr);
   void nir_emit_jump(nir_jump_instr *instr);
   fs_reg get_nir_src(nir_src src);
   fs_reg get_nir_dest(nir_dest dest);
   void emit_percomp(fs_inst *inst, unsigned wr_mask);

   bool optimize_frontfacing_ternary(nir_alu_instr *instr,
                                     const fs_reg &result);

   void setup_color_payload(fs_reg *dst, fs_reg color, unsigned components,
                            unsigned exec_size, bool use_2nd_half);
   void emit_alpha_test();
   fs_inst *emit_single_fb_write(fs_reg color1, fs_reg color2,
                                 fs_reg src0_alpha, unsigned components,
                                 unsigned exec_size, bool use_2nd_half = false);
   void emit_fb_writes();
   void emit_urb_writes();
   void emit_cs_terminate();

   void emit_shader_time_begin();
   void emit_shader_time_end();
   fs_inst *SHADER_TIME_ADD(enum shader_time_shader_type type, fs_reg value);

   void emit_untyped_atomic(unsigned atomic_op, unsigned surf_index,
                            fs_reg dst, fs_reg offset, fs_reg src0,
                            fs_reg src1);

   void emit_untyped_surface_read(unsigned surf_index, fs_reg dst,
                                  fs_reg offset);

   void emit_interpolate_expression(ir_expression *ir);

   bool try_rewrite_rhs_to_dst(ir_assignment *ir,
			       fs_reg dst,
			       fs_reg src,
			       fs_inst *pre_rhs_inst,
			       fs_inst *last_rhs_inst);
   void emit_assignment_writes(fs_reg &l, fs_reg &r,
			       const glsl_type *type, bool predicated);
   void resolve_ud_negate(fs_reg *reg);
   void resolve_bool_comparison(ir_rvalue *rvalue, fs_reg *reg);

   fs_reg get_timestamp(fs_inst **out_mov);

   struct brw_reg interp_reg(int location, int channel);
   void setup_uniform_values(ir_variable *ir);
   void setup_builtin_uniform_values(ir_variable *ir);
   int implied_mrf_writes(fs_inst *inst);

   virtual void dump_instructions();
   virtual void dump_instructions(const char *name);
   void dump_instruction(backend_instruction *inst);
   void dump_instruction(backend_instruction *inst, FILE *file);

   void visit_atomic_counter_intrinsic(ir_call *ir);

   const void *const key;
   const struct brw_sampler_prog_key_data *key_tex;

   struct brw_stage_prog_data *prog_data;
   unsigned int sanity_param_count;

   int *param_size;

   int *virtual_grf_start;
   int *virtual_grf_end;
   brw::fs_live_variables *live_intervals;

   int *regs_live_at_ip;

   /** Number of uniform variable components visited. */
   unsigned uniforms;

   /** Total number of direct uniforms we can get from NIR */
   unsigned num_direct_uniforms;

   /** Byte-offset for the next available spot in the scratch space buffer. */
   unsigned last_scratch;

   /**
    * Array mapping UNIFORM register numbers to the pull parameter index,
    * or -1 if this uniform register isn't being uploaded as a pull constant.
    */
   int *pull_constant_loc;

   /**
    * Array mapping UNIFORM register numbers to the push parameter index,
    * or -1 if this uniform register isn't being uploaded as a push constant.
    */
   int *push_constant_loc;

   struct hash_table *variable_ht;
   fs_reg frag_depth;
   fs_reg sample_mask;
   fs_reg outputs[VARYING_SLOT_MAX];
   unsigned output_components[VARYING_SLOT_MAX];
   fs_reg dual_src_output;
   bool do_dual_src;
   int first_non_payload_grf;
   /** Either BRW_MAX_GRF or GEN7_MRF_HACK_START */
   unsigned max_grf;

   fs_reg *fp_temp_regs;
   fs_reg *fp_input_regs;

   fs_reg *nir_locals;
   fs_reg *nir_globals;
   fs_reg nir_inputs;
   fs_reg nir_outputs;
   fs_reg *nir_system_values;

   /** @{ debug annotation info */
   const char *current_annotation;
   const void *base_ir;
   /** @} */

   bool failed;
   char *fail_msg;
   bool simd16_unsupported;
   char *no16_msg;

   /* Result of last visit() method. */
   fs_reg result;

   /** Register numbers for thread payload fields. */
   struct {
      uint8_t source_depth_reg;
      uint8_t source_w_reg;
      uint8_t aa_dest_stencil_reg;
      uint8_t dest_depth_reg;
      uint8_t sample_pos_reg;
      uint8_t sample_mask_in_reg;
      uint8_t barycentric_coord_reg[BRW_WM_BARYCENTRIC_INTERP_MODE_COUNT];

      /** The number of thread payload registers the hardware will supply. */
      uint8_t num_regs;
   } payload;

   bool source_depth_to_render_target;
   bool runtime_check_aads_emit;

   fs_reg pixel_x;
   fs_reg pixel_y;
   fs_reg wpos_w;
   fs_reg pixel_w;
   fs_reg delta_xy[BRW_WM_BARYCENTRIC_INTERP_MODE_COUNT];
   fs_reg shader_start_time;
   fs_reg userplane[MAX_CLIP_PLANES];

   unsigned grf_used;
   bool spilled_any_registers;

   const unsigned dispatch_width; /**< 8 or 16 */

   unsigned promoted_constants;
};

/**
 * The fragment shader code generator.
 *
 * Translates FS IR to actual i965 assembly code.
 */
class fs_generator
{
public:
   fs_generator(struct brw_context *brw,
                void *mem_ctx,
                const void *key,
                struct brw_stage_prog_data *prog_data,
                struct gl_program *fp,
                unsigned promoted_constants,
                bool runtime_check_aads_emit,
                const char *stage_abbrev);
   ~fs_generator();

   void enable_debug(const char *shader_name);
   int generate_code(const cfg_t *cfg, int dispatch_width);
   const unsigned *get_assembly(unsigned int *assembly_size);

private:
   void fire_fb_write(fs_inst *inst,
                      struct brw_reg payload,
                      struct brw_reg implied_header,
                      GLuint nr);
   void generate_fb_write(fs_inst *inst, struct brw_reg payload);
   void generate_urb_write(fs_inst *inst, struct brw_reg payload);
   void generate_cs_terminate(fs_inst *inst, struct brw_reg payload);
   void generate_blorp_fb_write(fs_inst *inst);
   void generate_linterp(fs_inst *inst, struct brw_reg dst,
			 struct brw_reg *src);
   void generate_tex(fs_inst *inst, struct brw_reg dst, struct brw_reg src,
                     struct brw_reg sampler_index);
   void generate_math_gen6(fs_inst *inst,
                           struct brw_reg dst,
                           struct brw_reg src0,
                           struct brw_reg src1);
   void generate_math_gen4(fs_inst *inst,
			   struct brw_reg dst,
			   struct brw_reg src);
   void generate_math_g45(fs_inst *inst,
			  struct brw_reg dst,
			  struct brw_reg src);
   void generate_ddx(enum opcode op, struct brw_reg dst, struct brw_reg src);
   void generate_ddy(enum opcode op, struct brw_reg dst, struct brw_reg src,
                     bool negate_value);
   void generate_scratch_write(fs_inst *inst, struct brw_reg src);
   void generate_scratch_read(fs_inst *inst, struct brw_reg dst);
   void generate_scratch_read_gen7(fs_inst *inst, struct brw_reg dst);
   void generate_uniform_pull_constant_load(fs_inst *inst, struct brw_reg dst,
                                            struct brw_reg index,
                                            struct brw_reg offset);
   void generate_uniform_pull_constant_load_gen7(fs_inst *inst,
                                                 struct brw_reg dst,
                                                 struct brw_reg surf_index,
                                                 struct brw_reg offset);
   void generate_varying_pull_constant_load(fs_inst *inst, struct brw_reg dst,
                                            struct brw_reg index,
                                            struct brw_reg offset);
   void generate_varying_pull_constant_load_gen7(fs_inst *inst,
                                                 struct brw_reg dst,
                                                 struct brw_reg index,
                                                 struct brw_reg offset);
   void generate_mov_dispatch_to_flags(fs_inst *inst);

   void generate_pixel_interpolator_query(fs_inst *inst,
                                          struct brw_reg dst,
                                          struct brw_reg src,
                                          struct brw_reg msg_data,
                                          unsigned msg_type);

   void generate_set_omask(fs_inst *inst,
                           struct brw_reg dst,
                           struct brw_reg sample_mask);

   void generate_set_sample_id(fs_inst *inst,
                               struct brw_reg dst,
                               struct brw_reg src0,
                               struct brw_reg src1);

   void generate_set_simd4x2_offset(fs_inst *inst,
                                    struct brw_reg dst,
                                    struct brw_reg offset);
   void generate_discard_jump(fs_inst *inst);

   void generate_pack_half_2x16_split(fs_inst *inst,
                                      struct brw_reg dst,
                                      struct brw_reg x,
                                      struct brw_reg y);
   void generate_unpack_half_2x16_split(fs_inst *inst,
                                        struct brw_reg dst,
                                        struct brw_reg src);

   void generate_shader_time_add(fs_inst *inst,
                                 struct brw_reg payload,
                                 struct brw_reg offset,
                                 struct brw_reg value);

   bool patch_discard_jumps_to_fb_writes();

   struct brw_context *brw;
   const struct brw_device_info *devinfo;

   struct brw_codegen *p;
   const void * const key;
   struct brw_stage_prog_data * const prog_data;

   const struct gl_program *prog;

   unsigned dispatch_width; /**< 8 or 16 */

   exec_list discard_halt_patches;
   unsigned promoted_constants;
   bool runtime_check_aads_emit;
   bool debug_flag;
   const char *shader_name;
   const char *stage_abbrev;
   void *mem_ctx;
};

bool brw_do_channel_expressions(struct exec_list *instructions);
bool brw_do_vector_splitting(struct exec_list *instructions);
void brw_setup_tex_for_precompile(struct brw_context *brw,
                                  struct brw_sampler_prog_key_data *tex,
                                  struct gl_program *prog);
