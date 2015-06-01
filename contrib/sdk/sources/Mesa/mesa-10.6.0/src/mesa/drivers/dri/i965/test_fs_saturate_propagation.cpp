/*
 * Copyright © 2015 Intel Corporation
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

#include <gtest/gtest.h>
#include "brw_fs.h"
#include "brw_cfg.h"
#include "program/program.h"

class saturate_propagation_test : public ::testing::Test {
   virtual void SetUp();

public:
   struct brw_context *brw;
   struct brw_device_info *devinfo;
   struct gl_context *ctx;
   struct brw_wm_prog_data *prog_data;
   struct gl_shader_program *shader_prog;
   struct brw_fragment_program *fp;
   fs_visitor *v;
};

class saturate_propagation_fs_visitor : public fs_visitor
{
public:
   saturate_propagation_fs_visitor(struct brw_context *brw,
                                   struct brw_wm_prog_data *prog_data,
                                   struct gl_shader_program *shader_prog)
      : fs_visitor(brw, NULL, MESA_SHADER_FRAGMENT, NULL, &prog_data->base,
                   shader_prog, (struct gl_program *) NULL, 8) {}
};


void saturate_propagation_test::SetUp()
{
   brw = (struct brw_context *)calloc(1, sizeof(*brw));
   devinfo = (struct brw_device_info *)calloc(1, sizeof(*brw));
   brw->intelScreen = (struct intel_screen *)calloc(1, sizeof(*brw->intelScreen));
   brw->intelScreen->devinfo = devinfo;
   ctx = &brw->ctx;

   fp = ralloc(NULL, struct brw_fragment_program);
   prog_data = ralloc(NULL, struct brw_wm_prog_data);
   shader_prog = ralloc(NULL, struct gl_shader_program);

   v = new saturate_propagation_fs_visitor(brw, prog_data, shader_prog);

   _mesa_init_fragment_program(ctx, &fp->program, GL_FRAGMENT_SHADER, 0);

   brw->gen = devinfo->gen = 4;
}

static fs_inst *
instruction(bblock_t *block, int num)
{
   fs_inst *inst = (fs_inst *)block->start();
   for (int i = 0; i < num; i++) {
      inst = (fs_inst *)inst->next;
   }
   return inst;
}

static bool
saturate_propagation(fs_visitor *v)
{
   const bool print = false;

   if (print) {
      fprintf(stderr, "= Before =\n");
      v->cfg->dump(v);
   }

   bool ret = v->opt_saturate_propagation();

   if (print) {
      fprintf(stderr, "\n= After =\n");
      v->cfg->dump(v);
   }

   return ret;
}

TEST_F(saturate_propagation_test, basic)
{
   fs_reg dst0 = v->vgrf(glsl_type::float_type);
   fs_reg dst1 = v->vgrf(glsl_type::float_type);
   fs_reg src0 = v->vgrf(glsl_type::float_type);
   fs_reg src1 = v->vgrf(glsl_type::float_type);
   v->emit(BRW_OPCODE_ADD, dst0, src0, src1);
   v->emit(BRW_OPCODE_MOV, dst1, dst0)
      ->saturate = true;

   /* = Before =
    *
    * 0: add(8)        dst0  src0  src1
    * 1: mov.sat(8)    dst1  dst0
    *
    * = After =
    * 0: add.sat(8)    dst0  src0  src1
    * 1: mov(8)        dst1  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, other_non_saturated_use)
{
   fs_reg dst0 = v->vgrf(glsl_type::float_type);
   fs_reg dst1 = v->vgrf(glsl_type::float_type);
   fs_reg dst2 = v->vgrf(glsl_type::float_type);
   fs_reg src0 = v->vgrf(glsl_type::float_type);
   fs_reg src1 = v->vgrf(glsl_type::float_type);
   v->emit(BRW_OPCODE_ADD, dst0, src0, src1);
   v->emit(BRW_OPCODE_MOV, dst1, dst0)
      ->saturate = true;
   v->emit(BRW_OPCODE_ADD, dst2, dst0, src0);

   /* = Before =
    *
    * 0: add(8)        dst0  src0  src1
    * 1: mov.sat(8)    dst1  dst0
    * 2: add(8)        dst2  dst0  src0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 2)->opcode);
}

TEST_F(saturate_propagation_test, predicated_instruction)
{
   fs_reg dst0 = v->vgrf(glsl_type::float_type);
   fs_reg dst1 = v->vgrf(glsl_type::float_type);
   fs_reg src0 = v->vgrf(glsl_type::float_type);
   fs_reg src1 = v->vgrf(glsl_type::float_type);
   v->emit(BRW_OPCODE_ADD, dst0, src0, src1)
      ->predicate = BRW_PREDICATE_NORMAL;
   v->emit(BRW_OPCODE_MOV, dst1, dst0)
      ->saturate = true;

   /* = Before =
    *
    * 0: (+f0) add(8)  dst0  src0  src1
    * 1: mov.sat(8)    dst1  dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, neg_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_type::float_type);
   fs_reg dst1 = v->vgrf(glsl_type::float_type);
   fs_reg src0 = v->vgrf(glsl_type::float_type);
   fs_reg src1 = v->vgrf(glsl_type::float_type);
   v->emit(BRW_OPCODE_ADD, dst0, src0, src1);
   dst0.negate = true;
   v->emit(BRW_OPCODE_MOV, dst1, dst0)
      ->saturate = true;

   /* = Before =
    *
    * 0: add(8)        dst0  src0  src1
    * 1: mov.sat(8)    dst1  -dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, abs_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_type::float_type);
   fs_reg dst1 = v->vgrf(glsl_type::float_type);
   fs_reg src0 = v->vgrf(glsl_type::float_type);
   fs_reg src1 = v->vgrf(glsl_type::float_type);
   v->emit(BRW_OPCODE_ADD, dst0, src0, src1);
   dst0.abs = true;
   v->emit(BRW_OPCODE_MOV, dst1, dst0)
      ->saturate = true;

   /* = Before =
    *
    * 0: add(8)        dst0  src0  src1
    * 1: mov.sat(8)    dst1  (abs)dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, producer_saturates)
{
   fs_reg dst0 = v->vgrf(glsl_type::float_type);
   fs_reg dst1 = v->vgrf(glsl_type::float_type);
   fs_reg dst2 = v->vgrf(glsl_type::float_type);
   fs_reg src0 = v->vgrf(glsl_type::float_type);
   fs_reg src1 = v->vgrf(glsl_type::float_type);
   v->emit(BRW_OPCODE_ADD, dst0, src0, src1)
      ->saturate = true;
   v->emit(BRW_OPCODE_MOV, dst1, dst0)
      ->saturate = true;
   v->emit(BRW_OPCODE_MOV, dst2, dst0);

   /* = Before =
    *
    * 0: add.sat(8)    dst0  src0  src1
    * 1: mov.sat(8)    dst1  dst0
    * 2: mov(8)        dst2  dst0
    *
    * = After =
    * 0: add.sat(8)    dst0  src0  src1
    * 1: mov(8)        dst1  dst0
    * 2: mov(8)        dst2  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, intervening_saturating_copy)
{
   fs_reg dst0 = v->vgrf(glsl_type::float_type);
   fs_reg dst1 = v->vgrf(glsl_type::float_type);
   fs_reg dst2 = v->vgrf(glsl_type::float_type);
   fs_reg src0 = v->vgrf(glsl_type::float_type);
   fs_reg src1 = v->vgrf(glsl_type::float_type);
   v->emit(BRW_OPCODE_ADD, dst0, src0, src1);
   v->emit(BRW_OPCODE_MOV, dst1, dst0)
      ->saturate = true;
   v->emit(BRW_OPCODE_MOV, dst2, dst0)
      ->saturate = true;

   /* = Before =
    *
    * 0: add(8)    dst0  src0  src1
    * 1: mov.sat(8)    dst1  dst0
    * 2: mov.sat(8)    dst2  dst0
    *
    * = After =
    * 0: add.sat(8)    dst0  src0  src1
    * 1: mov(8)        dst1  dst0
    * 2: mov(8)        dst2  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 2)->opcode);
   EXPECT_FALSE(instruction(block0, 2)->saturate);
}

TEST_F(saturate_propagation_test, intervening_dest_write)
{
   fs_reg dst0 = v->vgrf(glsl_type::vec4_type);
   fs_reg dst1 = v->vgrf(glsl_type::float_type);
   fs_reg src0 = v->vgrf(glsl_type::float_type);
   fs_reg src1 = v->vgrf(glsl_type::float_type);
   fs_reg src2 = v->vgrf(glsl_type::vec2_type);
   v->emit(BRW_OPCODE_ADD, offset(dst0, 2), src0, src1);
   v->emit(SHADER_OPCODE_TEX, dst0, src2)
      ->regs_written = 4;
   v->emit(BRW_OPCODE_MOV, dst1, offset(dst0, 2))
      ->saturate = true;

   /* = Before =
    *
    * 0: add(8)        dst0+2  src0    src1
    * 1: tex(8) rlen 4 dst0+0  src2
    * 2: mov.sat(8)    dst1    dst0+2
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(SHADER_OPCODE_TEX, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 2)->opcode);
   EXPECT_TRUE(instruction(block0, 2)->saturate);
}

TEST_F(saturate_propagation_test, mul_neg_mov_sat_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_type::float_type);
   fs_reg dst1 = v->vgrf(glsl_type::float_type);
   fs_reg dst2 = v->vgrf(glsl_type::float_type);
   fs_reg src0 = v->vgrf(glsl_type::float_type);
   fs_reg src1 = v->vgrf(glsl_type::float_type);
   v->emit(BRW_OPCODE_MUL, dst0, src0, src1);
   dst0.negate = true;
   v->emit(BRW_OPCODE_MOV, dst1, dst0)
      ->saturate = true;
   dst0.negate = false;
   v->emit(BRW_OPCODE_MOV, dst2, dst0)
      ->saturate = true;

   /* = Before =
    *
    * 0: mul(8)        dst0  src0  src1
    * 1: mov.sat(8)    dst1  -dst0
    * 2: mov.sat(8)    dst2  dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MUL, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_FALSE(instruction(block0, 0)->src[1].negate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
   EXPECT_TRUE(instruction(block0, 1)->src[0].negate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 2)->opcode);
   EXPECT_TRUE(instruction(block0, 2)->saturate);
}
