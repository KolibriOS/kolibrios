/*
 * Copyright © 2014 Broadcom
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

#include "util/u_memory.h"
#include "util/simple_list.h"
#include "util/ralloc.h"

#include "vc4_qir.h"
#include "vc4_qpu.h"

struct qir_op_info {
        const char *name;
        uint8_t ndst, nsrc;
        bool has_side_effects;
        bool multi_instruction;
};

static const struct qir_op_info qir_op_info[] = {
        [QOP_MOV] = { "mov", 1, 1 },
        [QOP_FADD] = { "fadd", 1, 2 },
        [QOP_FSUB] = { "fsub", 1, 2 },
        [QOP_FMUL] = { "fmul", 1, 2 },
        [QOP_MUL24] = { "mul24", 1, 2 },
        [QOP_FMIN] = { "fmin", 1, 2 },
        [QOP_FMAX] = { "fmax", 1, 2 },
        [QOP_FMINABS] = { "fminabs", 1, 2 },
        [QOP_FMAXABS] = { "fmaxabs", 1, 2 },
        [QOP_FTOI] = { "ftoi", 1, 1 },
        [QOP_ITOF] = { "itof", 1, 1 },
        [QOP_ADD] = { "add", 1, 2 },
        [QOP_SUB] = { "sub", 1, 2 },
        [QOP_SHR] = { "shr", 1, 2 },
        [QOP_ASR] = { "asr", 1, 2 },
        [QOP_SHL] = { "shl", 1, 2 },
        [QOP_MIN] = { "min", 1, 2 },
        [QOP_MAX] = { "max", 1, 2 },
        [QOP_AND] = { "and", 1, 2 },
        [QOP_OR] = { "or", 1, 2 },
        [QOP_XOR] = { "xor", 1, 2 },
        [QOP_NOT] = { "not", 1, 1 },

        [QOP_SEL_X_0_NS] = { "fsel_x_0_ns", 1, 1, false, true },
        [QOP_SEL_X_0_NC] = { "fsel_x_0_nc", 1, 1, false, true },
        [QOP_SEL_X_0_ZS] = { "fsel_x_0_zs", 1, 1, false, true },
        [QOP_SEL_X_0_ZC] = { "fsel_x_0_zc", 1, 1, false, true },
        [QOP_SEL_X_Y_NS] = { "fsel_x_y_ns", 1, 2, false, true },
        [QOP_SEL_X_Y_NC] = { "fsel_x_y_nc", 1, 2, false, true },
        [QOP_SEL_X_Y_ZS] = { "fsel_x_y_zs", 1, 2, false, true },
        [QOP_SEL_X_Y_ZC] = { "fsel_x_y_zc", 1, 2, false, true },

        [QOP_RCP] = { "rcp", 1, 1, false, true },
        [QOP_RSQ] = { "rsq", 1, 1, false, true },
        [QOP_EXP2] = { "exp2", 1, 2, false, true },
        [QOP_LOG2] = { "log2", 1, 2, false, true },
        [QOP_PACK_8888_F] = { "pack_8888_f", 1, 1, false, true },
        [QOP_PACK_8A_F] = { "pack_8a_f", 1, 2, false, true },
        [QOP_PACK_8B_F] = { "pack_8b_f", 1, 2, false, true },
        [QOP_PACK_8C_F] = { "pack_8c_f", 1, 2, false, true },
        [QOP_PACK_8D_F] = { "pack_8d_f", 1, 2, false, true },
        [QOP_PACK_SCALED] = { "pack_scaled", 1, 2, false, true },
        [QOP_TLB_DISCARD_SETUP] = { "discard", 0, 1, true },
        [QOP_TLB_STENCIL_SETUP] = { "tlb_stencil_setup", 0, 1, true },
        [QOP_TLB_Z_WRITE] = { "tlb_z", 0, 1, true },
        [QOP_TLB_COLOR_WRITE] = { "tlb_color", 0, 1, true },
        [QOP_TLB_COLOR_READ] = { "tlb_color_read", 1, 0 },
        [QOP_VARY_ADD_C] = { "vary_add_c", 1, 1 },

        [QOP_FRAG_X] = { "frag_x", 1, 0 },
        [QOP_FRAG_Y] = { "frag_y", 1, 0 },
        [QOP_FRAG_Z] = { "frag_z", 1, 0 },
        [QOP_FRAG_W] = { "frag_w", 1, 0 },
        [QOP_FRAG_REV_FLAG] = { "frag_rev_flag", 1, 0 },

        [QOP_TEX_S] = { "tex_s", 0, 2 },
        [QOP_TEX_T] = { "tex_t", 0, 2 },
        [QOP_TEX_R] = { "tex_r", 0, 2 },
        [QOP_TEX_B] = { "tex_b", 0, 2 },
        [QOP_TEX_DIRECT] = { "tex_direct", 0, 2 },
        [QOP_TEX_RESULT] = { "tex_result", 1, 0, true },
        [QOP_R4_UNPACK_A] = { "r4_unpack_a", 1, 1 },
        [QOP_R4_UNPACK_B] = { "r4_unpack_b", 1, 1 },
        [QOP_R4_UNPACK_C] = { "r4_unpack_c", 1, 1 },
        [QOP_R4_UNPACK_D] = { "r4_unpack_d", 1, 1 },
        [QOP_UNPACK_8A_F] = { "unpack_8a_f", 1, 1 },
        [QOP_UNPACK_8B_F] = { "unpack_8b_f", 1, 1 },
        [QOP_UNPACK_8C_F] = { "unpack_8c_f", 1, 1 },
        [QOP_UNPACK_8D_F] = { "unpack_8d_f", 1, 1 },
        [QOP_UNPACK_16A_F] = { "unpack_16a_f", 1, 1 },
        [QOP_UNPACK_16B_F] = { "unpack_16b_f", 1, 1 },
        [QOP_UNPACK_8A_I] = { "unpack_8a_i", 1, 1 },
        [QOP_UNPACK_8B_I] = { "unpack_8b_i", 1, 1 },
        [QOP_UNPACK_8C_I] = { "unpack_8c_i", 1, 1 },
        [QOP_UNPACK_8D_I] = { "unpack_8d_i", 1, 1 },
        [QOP_UNPACK_16A_I] = { "unpack_16a_i", 1, 1 },
        [QOP_UNPACK_16B_I] = { "unpack_16b_i", 1, 1 },
};

static const char *
qir_get_op_name(enum qop qop)
{
        if (qop < ARRAY_SIZE(qir_op_info) && qir_op_info[qop].name)
                return qir_op_info[qop].name;
        else
                return "???";
}

int
qir_get_op_nsrc(enum qop qop)
{
        if (qop < ARRAY_SIZE(qir_op_info) && qir_op_info[qop].name)
                return qir_op_info[qop].nsrc;
        else
                abort();
}

/**
 * Returns whether the instruction has any side effects that must be
 * preserved.
 */
bool
qir_has_side_effects(struct vc4_compile *c, struct qinst *inst)
{
        return qir_op_info[inst->op].has_side_effects;
}

bool
qir_has_side_effect_reads(struct vc4_compile *c, struct qinst *inst)
{
        /* We can dead-code eliminate varyings, because we only tell the VS
         * about the live ones at the end.  But we have to preserve the
         * point/line coordinates reads, because they're generated by
         * fixed-function hardware.
         */
        for (int i = 0; i < qir_get_op_nsrc(inst->op); i++) {
                if (inst->src[i].file == QFILE_VARY &&
                    c->input_semantics[inst->src[i].index].semantic == 0xff) {
                        return true;
                }

                if (inst->src[i].file == QFILE_VPM)
                        return true;
        }

        if (inst->dst.file == QFILE_VPM)
                return true;

        return false;
}

bool
qir_is_multi_instruction(struct qinst *inst)
{
        return qir_op_info[inst->op].multi_instruction;
}

bool
qir_is_tex(struct qinst *inst)
{
        return inst->op >= QOP_TEX_S && inst->op <= QOP_TEX_DIRECT;
}

bool
qir_depends_on_flags(struct qinst *inst)
{
        switch (inst->op) {
        case QOP_SEL_X_0_NS:
        case QOP_SEL_X_0_NC:
        case QOP_SEL_X_0_ZS:
        case QOP_SEL_X_0_ZC:
        case QOP_SEL_X_Y_NS:
        case QOP_SEL_X_Y_NC:
        case QOP_SEL_X_Y_ZS:
        case QOP_SEL_X_Y_ZC:
                return true;
        default:
                return false;
        }
}

bool
qir_src_needs_a_file(struct qinst *inst)
{
        switch (inst->op) {
        case QOP_UNPACK_8A_F:
        case QOP_UNPACK_8B_F:
        case QOP_UNPACK_8C_F:
        case QOP_UNPACK_8D_F:
        case QOP_UNPACK_16A_F:
        case QOP_UNPACK_16B_F:
        case QOP_UNPACK_8A_I:
        case QOP_UNPACK_8B_I:
        case QOP_UNPACK_8C_I:
        case QOP_UNPACK_8D_I:
        case QOP_UNPACK_16A_I:
        case QOP_UNPACK_16B_I:
                return true;
        default:
                return false;
        }
}

bool
qir_writes_r4(struct qinst *inst)
{
        switch (inst->op) {
        case QOP_TEX_RESULT:
        case QOP_TLB_COLOR_READ:
        case QOP_RCP:
        case QOP_RSQ:
        case QOP_EXP2:
        case QOP_LOG2:
                return true;
        default:
                return false;
        }
}

bool
qir_reads_r4(struct qinst *inst)
{
        switch (inst->op) {
        case QOP_R4_UNPACK_A:
        case QOP_R4_UNPACK_B:
        case QOP_R4_UNPACK_C:
        case QOP_R4_UNPACK_D:
                return true;
        default:
                return false;
        }
}

static void
qir_print_reg(struct vc4_compile *c, struct qreg reg, bool write)
{
        static const char *files[] = {
                [QFILE_TEMP] = "t",
                [QFILE_VARY] = "v",
                [QFILE_UNIF] = "u",
        };

        if (reg.file == QFILE_NULL) {
                fprintf(stderr, "null");
        } else if (reg.file == QFILE_SMALL_IMM) {
                if ((int)reg.index >= -16 && (int)reg.index <= 15)
                        fprintf(stderr, "%d", reg.index);
                else
                        fprintf(stderr, "%f", uif(reg.index));
        } else if (reg.file == QFILE_VPM) {
                if (write) {
                        fprintf(stderr, "vpm");
                } else {
                        fprintf(stderr, "vpm%d.%d",
                                reg.index / 4, reg.index % 4);
                }
        } else {
                fprintf(stderr, "%s%d", files[reg.file], reg.index);
        }

        if (reg.file == QFILE_UNIF &&
            c->uniform_contents[reg.index] == QUNIFORM_CONSTANT) {
                fprintf(stderr, " (0x%08x / %f)",
                        c->uniform_data[reg.index],
                        uif(c->uniform_data[reg.index]));
        }
}

void
qir_dump_inst(struct vc4_compile *c, struct qinst *inst)
{
        fprintf(stderr, "%s%s ",
                qir_get_op_name(inst->op),
                inst->sf ? ".sf" : "");

        qir_print_reg(c, inst->dst, true);
        for (int i = 0; i < qir_get_op_nsrc(inst->op); i++) {
                fprintf(stderr, ", ");
                qir_print_reg(c, inst->src[i], false);
        }
}

void
qir_dump(struct vc4_compile *c)
{
        struct simple_node *node;

        foreach(node, &c->instructions) {
                struct qinst *inst = (struct qinst *)node;
                qir_dump_inst(c, inst);
                fprintf(stderr, "\n");
        }
}

struct qreg
qir_get_temp(struct vc4_compile *c)
{
        struct qreg reg;

        reg.file = QFILE_TEMP;
        reg.index = c->num_temps++;

        if (c->num_temps > c->defs_array_size) {
                uint32_t old_size = c->defs_array_size;
                c->defs_array_size = MAX2(old_size * 2, 16);
                c->defs = reralloc(c, c->defs, struct qinst *,
                                   c->defs_array_size);
                memset(&c->defs[old_size], 0,
                       sizeof(c->defs[0]) * (c->defs_array_size - old_size));
        }

        return reg;
}

struct qinst *
qir_inst(enum qop op, struct qreg dst, struct qreg src0, struct qreg src1)
{
        struct qinst *inst = CALLOC_STRUCT(qinst);

        inst->op = op;
        inst->dst = dst;
        inst->src = calloc(2, sizeof(inst->src[0]));
        inst->src[0] = src0;
        inst->src[1] = src1;

        return inst;
}

struct qinst *
qir_inst4(enum qop op, struct qreg dst,
          struct qreg a,
          struct qreg b,
          struct qreg c,
          struct qreg d)
{
        struct qinst *inst = CALLOC_STRUCT(qinst);

        inst->op = op;
        inst->dst = dst;
        inst->src = calloc(4, sizeof(*inst->src));
        inst->src[0] = a;
        inst->src[1] = b;
        inst->src[2] = c;
        inst->src[3] = d;

        return inst;
}

void
qir_emit(struct vc4_compile *c, struct qinst *inst)
{
        if (inst->dst.file == QFILE_TEMP)
                c->defs[inst->dst.index] = inst;

        insert_at_tail(&c->instructions, &inst->link);
}

bool
qir_reg_equals(struct qreg a, struct qreg b)
{
        return a.file == b.file && a.index == b.index;
}

struct vc4_compile *
qir_compile_init(void)
{
        struct vc4_compile *c = rzalloc(NULL, struct vc4_compile);

        make_empty_list(&c->instructions);

        c->output_position_index = -1;
        c->output_clipvertex_index = -1;
        c->output_color_index = -1;
        c->output_point_size_index = -1;

        c->def_ht = _mesa_hash_table_create(c, _mesa_hash_pointer,
                                            _mesa_key_pointer_equal);

        return c;
}

void
qir_remove_instruction(struct vc4_compile *c, struct qinst *qinst)
{
        if (qinst->dst.file == QFILE_TEMP)
                c->defs[qinst->dst.index] = NULL;

        remove_from_list(&qinst->link);
        free(qinst->src);
        free(qinst);
}

struct qreg
qir_follow_movs(struct vc4_compile *c, struct qreg reg)
{
        while (reg.file == QFILE_TEMP && c->defs[reg.index]->op == QOP_MOV)
                reg = c->defs[reg.index]->src[0];

        return reg;
}

void
qir_compile_destroy(struct vc4_compile *c)
{
        while (!is_empty_list(&c->instructions)) {
                struct qinst *qinst =
                        (struct qinst *)first_elem(&c->instructions);
                qir_remove_instruction(c, qinst);
        }

        ralloc_free(c);
}

const char *
qir_get_stage_name(enum qstage stage)
{
        static const char *names[] = {
                [QSTAGE_FRAG] = "FS",
                [QSTAGE_VERT] = "VS",
                [QSTAGE_COORD] = "CS",
        };

        return names[stage];
}

struct qreg
qir_uniform(struct vc4_compile *c,
            enum quniform_contents contents,
            uint32_t data)
{
        for (int i = 0; i < c->num_uniforms; i++) {
                if (c->uniform_contents[i] == contents &&
                    c->uniform_data[i] == data) {
                        return (struct qreg) { QFILE_UNIF, i };
                }
        }

        uint32_t uniform = c->num_uniforms++;
        struct qreg u = { QFILE_UNIF, uniform };

        if (uniform >= c->uniform_array_size) {
                c->uniform_array_size = MAX2(MAX2(16, uniform + 1),
                                             c->uniform_array_size * 2);

                c->uniform_data = reralloc(c, c->uniform_data,
                                           uint32_t,
                                           c->uniform_array_size);
                c->uniform_contents = reralloc(c, c->uniform_contents,
                                               enum quniform_contents,
                                               c->uniform_array_size);
        }

        c->uniform_contents[uniform] = contents;
        c->uniform_data[uniform] = data;

        return u;
}

void
qir_SF(struct vc4_compile *c, struct qreg src)
{
        struct qinst *last_inst = NULL;
        if (!is_empty_list(&c->instructions))
                last_inst = (struct qinst *)c->instructions.prev;

        if (!last_inst ||
            last_inst->dst.file != src.file ||
            last_inst->dst.index != src.index ||
            qir_is_multi_instruction(last_inst)) {
                src = qir_MOV(c, src);
                last_inst = (struct qinst *)c->instructions.prev;
        }
        last_inst->sf = true;
}

#define OPTPASS(func)                                                   \
        do {                                                            \
                bool stage_progress = func(c);                          \
                if (stage_progress) {                                   \
                        progress = true;                                \
                        if (print_opt_debug) {                          \
                                fprintf(stderr,                         \
                                        "QIR opt pass %2d: %s progress\n", \
                                        pass, #func);                   \
                        }                                               \
                }                                                       \
        } while (0)

void
qir_optimize(struct vc4_compile *c)
{
        bool print_opt_debug = false;
        int pass = 1;

        while (true) {
                bool progress = false;

                OPTPASS(qir_opt_algebraic);
                OPTPASS(qir_opt_cse);
                OPTPASS(qir_opt_constant_folding);
                OPTPASS(qir_opt_copy_propagation);
                OPTPASS(qir_opt_dead_code);
                OPTPASS(qir_opt_small_immediates);
                OPTPASS(qir_opt_vpm_writes);

                if (!progress)
                        break;

                pass++;
        }
}
