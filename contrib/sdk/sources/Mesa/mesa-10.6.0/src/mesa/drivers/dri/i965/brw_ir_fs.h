/* -*- c++ -*- */
/*
 * Copyright © 2010-2015 Intel Corporation
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

#ifndef BRW_IR_FS_H
#define BRW_IR_FS_H

#include "brw_shader.h"

class fs_inst;

class fs_reg : public backend_reg {
public:
   DECLARE_RALLOC_CXX_OPERATORS(fs_reg)

   void init();

   fs_reg();
   explicit fs_reg(float f);
   explicit fs_reg(int32_t i);
   explicit fs_reg(uint32_t u);
   explicit fs_reg(uint8_t vf[4]);
   explicit fs_reg(uint8_t vf0, uint8_t vf1, uint8_t vf2, uint8_t vf3);
   fs_reg(struct brw_reg fixed_hw_reg);
   fs_reg(enum register_file file, int reg);
   fs_reg(enum register_file file, int reg, enum brw_reg_type type);
   fs_reg(enum register_file file, int reg, enum brw_reg_type type, uint8_t width);

   bool equals(const fs_reg &r) const;
   bool is_contiguous() const;

   /** Smear a channel of the reg to all channels. */
   fs_reg &set_smear(unsigned subreg);

   /**
    * Offset in bytes from the start of the register.  Values up to a
    * backend_reg::reg_offset unit are valid.
    */
   int subreg_offset;

   fs_reg *reladdr;

   /**
    * The register width.  This indicates how many hardware values are
    * represented by each virtual value.  Valid values are 1, 8, or 16.
    * For immediate values, this is 1.  Most of the rest of the time, it
    * will be equal to the dispatch width.
    */
   uint8_t width;

   /** Register region horizontal stride */
   uint8_t stride;
};

static inline fs_reg
negate(fs_reg reg)
{
   assert(reg.file != HW_REG && reg.file != IMM);
   reg.negate = !reg.negate;
   return reg;
}

static inline fs_reg
retype(fs_reg reg, enum brw_reg_type type)
{
   reg.fixed_hw_reg.type = reg.type = type;
   return reg;
}

static inline fs_reg
byte_offset(fs_reg reg, unsigned delta)
{
   switch (reg.file) {
   case BAD_FILE:
      break;
   case GRF:
   case ATTR:
      reg.reg_offset += delta / 32;
      break;
   case MRF:
      reg.reg += delta / 32;
      break;
   default:
      assert(delta == 0);
   }
   reg.subreg_offset += delta % 32;
   return reg;
}

static inline fs_reg
horiz_offset(fs_reg reg, unsigned delta)
{
   switch (reg.file) {
   case BAD_FILE:
   case UNIFORM:
   case IMM:
      /* These only have a single component that is implicitly splatted.  A
       * horizontal offset should be a harmless no-op.
       */
      break;
   case GRF:
   case MRF:
   case ATTR:
      return byte_offset(reg, delta * reg.stride * type_sz(reg.type));
   default:
      assert(delta == 0);
   }
   return reg;
}

static inline fs_reg
offset(fs_reg reg, unsigned delta)
{
   switch (reg.file) {
   case BAD_FILE:
      break;
   case GRF:
   case MRF:
   case ATTR:
      return byte_offset(reg,
                         delta * MAX2(reg.width * reg.stride, 1) *
                         type_sz(reg.type));
   case UNIFORM:
      reg.reg_offset += delta;
      break;
   default:
      assert(delta == 0);
   }
   return reg;
}

static inline fs_reg
component(fs_reg reg, unsigned idx)
{
   assert(reg.subreg_offset == 0);
   assert(idx < reg.width);
   reg.subreg_offset = idx * type_sz(reg.type);
   reg.width = 1;
   reg.stride = 0;
   return reg;
}

static inline bool
is_uniform(const fs_reg &reg)
{
   return (reg.width == 1 || reg.stride == 0 || reg.is_null()) &&
          (!reg.reladdr || is_uniform(*reg.reladdr));
}

/**
 * Get either of the 8-component halves of a 16-component register.
 *
 * Note: this also works if \c reg represents a SIMD16 pair of registers.
 */
static inline fs_reg
half(fs_reg reg, unsigned idx)
{
   assert(idx < 2);

   switch (reg.file) {
   case BAD_FILE:
   case UNIFORM:
   case IMM:
      return reg;

   case GRF:
   case MRF:
      assert(reg.width == 16);
      reg.width = 8;
      return horiz_offset(reg, 8 * idx);

   case ATTR:
   case HW_REG:
   default:
      unreachable("Cannot take half of this register type");
   }
   return reg;
}

static const fs_reg reg_undef;

class fs_inst : public backend_instruction {
   fs_inst &operator=(const fs_inst &);

   void init(enum opcode opcode, uint8_t exec_width, const fs_reg &dst,
             const fs_reg *src, unsigned sources);

public:
   DECLARE_RALLOC_CXX_OPERATORS(fs_inst)

   fs_inst();
   fs_inst(enum opcode opcode, uint8_t exec_size);
   fs_inst(enum opcode opcode, const fs_reg &dst);
   fs_inst(enum opcode opcode, uint8_t exec_size, const fs_reg &dst,
           const fs_reg &src0);
   fs_inst(enum opcode opcode, const fs_reg &dst, const fs_reg &src0);
   fs_inst(enum opcode opcode, uint8_t exec_size, const fs_reg &dst,
           const fs_reg &src0, const fs_reg &src1);
   fs_inst(enum opcode opcode, const fs_reg &dst, const fs_reg &src0,
           const fs_reg &src1);
   fs_inst(enum opcode opcode, uint8_t exec_size, const fs_reg &dst,
           const fs_reg &src0, const fs_reg &src1, const fs_reg &src2);
   fs_inst(enum opcode opcode, const fs_reg &dst, const fs_reg &src0,
           const fs_reg &src1, const fs_reg &src2);
   fs_inst(enum opcode opcode, const fs_reg &dst, const fs_reg src[],
           unsigned sources);
   fs_inst(enum opcode opcode, uint8_t exec_size, const fs_reg &dst,
           const fs_reg src[], unsigned sources);
   fs_inst(const fs_inst &that);
   ~fs_inst();

   void resize_sources(uint8_t num_sources);

   bool equals(fs_inst *inst) const;
   bool overwrites_reg(const fs_reg &reg) const;
   bool is_send_from_grf() const;
   bool is_partial_write() const;
   bool is_copy_payload(const brw::simple_allocator &grf_alloc) const;
   int regs_read(int arg) const;
   bool can_do_source_mods(const struct brw_device_info *devinfo);
   bool has_side_effects() const;

   bool reads_flag() const;
   bool writes_flag() const;

   fs_reg dst;
   fs_reg *src;

   uint8_t sources; /**< Number of fs_reg sources. */

   /**
    * Execution size of the instruction.  This is used by the generator to
    * generate the correct binary for the given fs_inst.  Current valid
    * values are 1, 8, 16.
    */
   uint8_t exec_size;

   bool eot:1;
   bool force_uncompressed:1;
   bool force_sechalf:1;
   bool pi_noperspective:1;   /**< Pixel interpolator noperspective flag */
};

/**
 * Set second-half quarter control on \p inst.
 */
static inline fs_inst *
set_sechalf(fs_inst *inst)
{
   inst->force_sechalf = true;
   return inst;
}

#endif
