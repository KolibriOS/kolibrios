/* -*- c++ -*- */
/*
 * Copyright © 2011-2015 Intel Corporation
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

#ifndef BRW_IR_VEC4_H
#define BRW_IR_VEC4_H

#include "brw_shader.h"
#include "brw_context.h"

namespace brw {

class dst_reg;

class src_reg : public backend_reg
{
public:
   DECLARE_RALLOC_CXX_OPERATORS(src_reg)

   void init();

   src_reg(register_file file, int reg, const glsl_type *type);
   src_reg();
   src_reg(float f);
   src_reg(uint32_t u);
   src_reg(int32_t i);
   src_reg(uint8_t vf[4]);
   src_reg(uint8_t vf0, uint8_t vf1, uint8_t vf2, uint8_t vf3);
   src_reg(struct brw_reg reg);

   bool equals(const src_reg &r) const;

   src_reg(class vec4_visitor *v, const struct glsl_type *type);
   src_reg(class vec4_visitor *v, const struct glsl_type *type, int size);

   explicit src_reg(const dst_reg &reg);

   unsigned swizzle; /**< BRW_SWIZZLE_XYZW macros from brw_reg.h. */

   src_reg *reladdr;
};

static inline src_reg
retype(src_reg reg, enum brw_reg_type type)
{
   reg.fixed_hw_reg.type = reg.type = type;
   return reg;
}

static inline src_reg
offset(src_reg reg, unsigned delta)
{
   assert(delta == 0 || (reg.file != HW_REG && reg.file != IMM));
   reg.reg_offset += delta;
   return reg;
}

/**
 * Reswizzle a given source register.
 * \sa brw_swizzle().
 */
static inline src_reg
swizzle(src_reg reg, unsigned swizzle)
{
   assert(reg.file != HW_REG);
   reg.swizzle = brw_compose_swizzle(swizzle, reg.swizzle);
   return reg;
}

static inline src_reg
negate(src_reg reg)
{
   assert(reg.file != HW_REG && reg.file != IMM);
   reg.negate = !reg.negate;
   return reg;
}

static inline bool
is_uniform(const src_reg &reg)
{
   return (reg.file == IMM || reg.file == UNIFORM || reg.is_null()) &&
          (!reg.reladdr || is_uniform(*reg.reladdr));
}

class dst_reg : public backend_reg
{
public:
   DECLARE_RALLOC_CXX_OPERATORS(dst_reg)

   void init();

   dst_reg();
   dst_reg(register_file file, int reg);
   dst_reg(register_file file, int reg, const glsl_type *type,
           unsigned writemask);
   dst_reg(struct brw_reg reg);
   dst_reg(class vec4_visitor *v, const struct glsl_type *type);

   explicit dst_reg(const src_reg &reg);

   bool equals(const dst_reg &r) const;

   unsigned writemask; /**< Bitfield of WRITEMASK_[XYZW] */

   src_reg *reladdr;
};

static inline dst_reg
retype(dst_reg reg, enum brw_reg_type type)
{
   reg.fixed_hw_reg.type = reg.type = type;
   return reg;
}

static inline dst_reg
offset(dst_reg reg, unsigned delta)
{
   assert(delta == 0 || (reg.file != HW_REG && reg.file != IMM));
   reg.reg_offset += delta;
   return reg;
}

static inline dst_reg
writemask(dst_reg reg, unsigned mask)
{
   assert(reg.file != HW_REG && reg.file != IMM);
   assert((reg.writemask & mask) != 0);
   reg.writemask &= mask;
   return reg;
}

class vec4_instruction : public backend_instruction {
public:
   DECLARE_RALLOC_CXX_OPERATORS(vec4_instruction)

   vec4_instruction(enum opcode opcode,
                    const dst_reg &dst = dst_reg(),
                    const src_reg &src0 = src_reg(),
                    const src_reg &src1 = src_reg(),
                    const src_reg &src2 = src_reg());

   struct brw_reg get_dst(void);
   struct brw_reg get_src(const struct brw_vue_prog_data *prog_data, int i);

   dst_reg dst;
   src_reg src[3];

   enum brw_urb_write_flags urb_write_flags;

   unsigned sol_binding; /**< gen6: SOL binding table index */
   bool sol_final_write; /**< gen6: send commit message */
   unsigned sol_vertex; /**< gen6: used for setting dst index in SVB header */

   bool is_send_from_grf();
   unsigned regs_read(unsigned arg) const;
   bool can_reswizzle(int dst_writemask, int swizzle, int swizzle_mask);
   void reswizzle(int dst_writemask, int swizzle);
   bool can_do_source_mods(const struct brw_device_info *devinfo);

   bool reads_flag()
   {
      return predicate || opcode == VS_OPCODE_UNPACK_FLAGS_SIMD4X2;
   }

   bool writes_flag()
   {
      return (conditional_mod && (opcode != BRW_OPCODE_SEL &&
                                  opcode != BRW_OPCODE_IF &&
                                  opcode != BRW_OPCODE_WHILE));
   }
};

} /* namespace brw */

#endif
