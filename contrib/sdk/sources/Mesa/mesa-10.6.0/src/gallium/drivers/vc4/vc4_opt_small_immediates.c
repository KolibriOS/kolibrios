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

/**
 * @file vc4_opt_small_immediates.c
 *
 * Turns references to small constant uniform values into small immediates
 * fields.
 */

#include "vc4_qir.h"
#include "vc4_qpu.h"

static bool debug;

bool
qir_opt_small_immediates(struct vc4_compile *c)
{
        bool progress = false;
        struct simple_node *node;

        foreach(node, &c->instructions) {
                struct qinst *inst = (struct qinst *)node;

                /* The small immediate value sits in the raddr B field, so we
                 * can't have 2 small immediates in one instruction (unless
                 * they're the same value, but that should be optimized away
                 * elsewhere).
                 */
                bool uses_small_imm = false;
                for (int i = 0; i < qir_get_op_nsrc(inst->op); i++) {
                        if (inst->src[i].file == QFILE_SMALL_IMM)
                                uses_small_imm = true;
                }
                if (uses_small_imm)
                        continue;

                for (int i = 0; i < qir_get_op_nsrc(inst->op); i++) {
                        struct qreg src = qir_follow_movs(c, inst->src[i]);

                        if (src.file != QFILE_UNIF ||
                            c->uniform_contents[src.index] !=
                            QUNIFORM_CONSTANT) {
                                continue;
                        }

                        if (i == 1 &&
                            (inst->op == QOP_TEX_S ||
                             inst->op == QOP_TEX_T ||
                             inst->op == QOP_TEX_R ||
                             inst->op == QOP_TEX_B)) {
                                /* No turning the implicit uniform read into
                                 * an immediate.
                                 */
                                continue;
                        }

                        if (qir_src_needs_a_file(inst))
                                continue;

                        uint32_t imm = c->uniform_data[src.index];
                        uint32_t small_imm = qpu_encode_small_immediate(imm);
                        if (small_imm == ~0)
                                continue;

                        if (debug) {
                                fprintf(stderr, "opt_small_immediate() from: ");
                                qir_dump_inst(c, inst);
                                fprintf(stderr, "\n");
                        }
                        inst->src[i].file = QFILE_SMALL_IMM;
                        inst->src[i].index = imm;
                        if (debug) {
                                fprintf(stderr, "to: ");
                                qir_dump_inst(c, inst);
                                fprintf(stderr, "\n");
                        }
                        progress = true;
                        break;
                }
        }

        return progress;
}
