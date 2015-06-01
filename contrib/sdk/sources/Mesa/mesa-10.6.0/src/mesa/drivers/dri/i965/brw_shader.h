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

#include <stdint.h>
#include "brw_reg.h"
#include "brw_defines.h"
#include "main/compiler.h"
#include "glsl/ir.h"

#ifdef __cplusplus
#include "brw_ir_allocator.h"
#endif

#pragma once

#define MAX_SAMPLER_MESSAGE_SIZE 11
#define MAX_VGRF_SIZE 16

struct brw_compiler {
   const struct brw_device_info *devinfo;

   struct {
      struct ra_regs *regs;

      /**
       * Array of the ra classes for the unaligned contiguous register
       * block sizes used.
       */
      int *classes;

      /**
       * Mapping for register-allocated objects in *regs to the first
       * GRF for that object.
       */
      uint8_t *ra_reg_to_grf;
   } vec4_reg_set;

   struct {
      struct ra_regs *regs;

      /**
       * Array of the ra classes for the unaligned contiguous register
       * block sizes used, indexed by register size.
       */
      int classes[16];

      /**
       * Mapping from classes to ra_reg ranges.  Each of the per-size
       * classes corresponds to a range of ra_reg nodes.  This array stores
       * those ranges in the form of first ra_reg in each class and the
       * total number of ra_reg elements in the last array element.  This
       * way the range of the i'th class is given by:
       * [ class_to_ra_reg_range[i], class_to_ra_reg_range[i+1] )
       */
      int class_to_ra_reg_range[17];

      /**
       * Mapping for register-allocated objects in *regs to the first
       * GRF for that object.
       */
      uint8_t *ra_reg_to_grf;

      /**
       * ra class for the aligned pairs we use for PLN, which doesn't
       * appear in *classes.
       */
      int aligned_pairs_class;
   } fs_reg_sets[2];
};

enum PACKED register_file {
   BAD_FILE,
   GRF,
   MRF,
   IMM,
   HW_REG, /* a struct brw_reg */
   ATTR,
   UNIFORM, /* prog_data->params[reg] */
};

struct backend_reg
{
#ifdef __cplusplus
   bool is_zero() const;
   bool is_one() const;
   bool is_negative_one() const;
   bool is_null() const;
   bool is_accumulator() const;
   bool in_range(const backend_reg &r, unsigned n) const;
#endif

   enum register_file file; /**< Register file: GRF, MRF, IMM. */
   enum brw_reg_type type;  /**< Register type: BRW_REGISTER_TYPE_* */

   /**
    * Register number.
    *
    * For GRF, it's a virtual register number until register allocation.
    *
    * For MRF, it's the hardware register.
    */
   uint16_t reg;

   /**
    * Offset within the virtual register.
    *
    * In the scalar backend, this is in units of a float per pixel for pre-
    * register allocation registers (i.e., one register in SIMD8 mode and two
    * registers in SIMD16 mode).
    *
    * For uniforms, this is in units of 1 float.
    */
   uint16_t reg_offset;

   struct brw_reg fixed_hw_reg;

   bool negate;
   bool abs;
};

struct cfg_t;
struct bblock_t;

#ifdef __cplusplus
struct backend_instruction : public exec_node {
   bool is_3src() const;
   bool is_tex() const;
   bool is_math() const;
   bool is_control_flow() const;
   bool is_commutative() const;
   bool can_do_source_mods() const;
   bool can_do_saturate() const;
   bool can_do_cmod() const;
   bool reads_accumulator_implicitly() const;
   bool writes_accumulator_implicitly(const struct brw_device_info *devinfo) const;

   void remove(bblock_t *block);
   void insert_after(bblock_t *block, backend_instruction *inst);
   void insert_before(bblock_t *block, backend_instruction *inst);
   void insert_before(bblock_t *block, exec_list *list);

   /**
    * True if the instruction has side effects other than writing to
    * its destination registers.  You are expected not to reorder or
    * optimize these out unless you know what you are doing.
    */
   bool has_side_effects() const;
#else
struct backend_instruction {
   struct exec_node link;
#endif
   /** @{
    * Annotation for the generated IR.  One of the two can be set.
    */
   const void *ir;
   const char *annotation;
   /** @} */

   uint32_t offset; /**< spill/unspill offset or texture offset bitfield */
   uint8_t mlen; /**< SEND message length */
   int8_t base_mrf; /**< First MRF in the SEND message, if mlen is nonzero. */
   uint8_t target; /**< MRT target. */
   uint8_t regs_written; /**< Number of registers written by the instruction. */

   enum opcode opcode; /* BRW_OPCODE_* or FS_OPCODE_* */
   enum brw_conditional_mod conditional_mod; /**< BRW_CONDITIONAL_* */
   enum brw_predicate predicate;
   bool predicate_inverse:1;
   bool writes_accumulator:1; /**< instruction implicitly writes accumulator */
   bool force_writemask_all:1;
   bool no_dd_clear:1;
   bool no_dd_check:1;
   bool saturate:1;
   bool shadow_compare:1;

   /* Chooses which flag subregister (f0.0 or f0.1) is used for conditional
    * mod and predication.
    */
   unsigned flag_subreg:1;

   /** The number of hardware registers used for a message header. */
   uint8_t header_size;
};

#ifdef __cplusplus

enum instruction_scheduler_mode {
   SCHEDULE_PRE,
   SCHEDULE_PRE_NON_LIFO,
   SCHEDULE_PRE_LIFO,
   SCHEDULE_POST,
};

class backend_visitor : public ir_visitor {
protected:

   backend_visitor(struct brw_context *brw,
                   struct gl_shader_program *shader_prog,
                   struct gl_program *prog,
                   struct brw_stage_prog_data *stage_prog_data,
                   gl_shader_stage stage);

public:

   struct brw_context * const brw;
   const struct brw_device_info * const devinfo;
   struct gl_context * const ctx;
   struct brw_shader * const shader;
   struct gl_shader_program * const shader_prog;
   struct gl_program * const prog;
   struct brw_stage_prog_data * const stage_prog_data;

   /** ralloc context for temporary data used during compile */
   void *mem_ctx;

   /**
    * List of either fs_inst or vec4_instruction (inheriting from
    * backend_instruction)
    */
   exec_list instructions;

   cfg_t *cfg;

   gl_shader_stage stage;
   bool debug_enabled;
   const char *stage_name;
   const char *stage_abbrev;

   brw::simple_allocator alloc;

   virtual void dump_instruction(backend_instruction *inst) = 0;
   virtual void dump_instruction(backend_instruction *inst, FILE *file) = 0;
   virtual void dump_instructions();
   virtual void dump_instructions(const char *name);

   void calculate_cfg();
   void invalidate_cfg();

   void assign_common_binding_table_offsets(uint32_t next_binding_table_offset);

   virtual void invalidate_live_intervals() = 0;
};

uint32_t brw_texture_offset(int *offsets, unsigned num_components);

#endif /* __cplusplus */

enum brw_reg_type brw_type_for_base_type(const struct glsl_type *type);
enum brw_conditional_mod brw_conditional_for_comparison(unsigned int op);
uint32_t brw_math_function(enum opcode op);
const char *brw_instruction_name(enum opcode op);
bool brw_saturate_immediate(enum brw_reg_type type, struct brw_reg *reg);
bool brw_negate_immediate(enum brw_reg_type type, struct brw_reg *reg);
bool brw_abs_immediate(enum brw_reg_type type, struct brw_reg *reg);

#ifdef __cplusplus
extern "C" {
#endif

struct brw_compiler *
brw_compiler_create(void *mem_ctx, const struct brw_device_info *devinfo);

bool brw_vs_precompile(struct gl_context *ctx,
                       struct gl_shader_program *shader_prog,
                       struct gl_program *prog);
bool brw_gs_precompile(struct gl_context *ctx,
                       struct gl_shader_program *shader_prog,
                       struct gl_program *prog);
bool brw_fs_precompile(struct gl_context *ctx,
                       struct gl_shader_program *shader_prog,
                       struct gl_program *prog);
bool brw_cs_precompile(struct gl_context *ctx,
                       struct gl_shader_program *shader_prog,
                       struct gl_program *prog);

#ifdef __cplusplus
}
#endif
