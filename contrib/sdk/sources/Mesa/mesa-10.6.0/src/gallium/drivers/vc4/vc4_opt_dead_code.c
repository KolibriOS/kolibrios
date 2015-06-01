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
 * @file vc4_opt_dead_code.c
 *
 * This is a simmple dead code eliminator for QIR with no control flow.
 *
 * It walks from the bottom of the instruction list, removing instructions
 * with a destination that is never used, and marking the sources of non-dead
 * instructions as used.
 */

#include "vc4_qir.h"

static bool debug;

static void
dce(struct vc4_compile *c, struct qinst *inst)
{
        if (debug) {
                fprintf(stderr, "Removing: ");
                qir_dump_inst(c, inst);
                fprintf(stderr, "\n");
        }
        assert(!inst->sf);
        qir_remove_instruction(c, inst);
}

static bool
has_nonremovable_reads(struct vc4_compile *c, struct qinst *inst)
{
        for (int i = 0; i < qir_get_op_nsrc(inst->op); i++) {
                if (inst->src[i].file == QFILE_VPM) {
                        uint32_t attr = inst->src[i].index / 4;
                        uint32_t offset = (inst->src[i].index % 4) * 4;

                        if (c->vattr_sizes[attr] != offset + 4)
                                return true;

                        /* Can't get rid of the last VPM read, or the
                         * simulator (at least) throws an error.
                         */
                        uint32_t total_size = 0;
                        for (uint32_t i = 0; i < ARRAY_SIZE(c->vattr_sizes); i++)
                                total_size += c->vattr_sizes[i];
                        if (total_size == 4)
                                return true;
                }

                if (inst->src[i].file == QFILE_VARY &&
                    c->input_semantics[inst->src[i].index].semantic == 0xff) {
                        return true;
                }
        }

        return false;
}

bool
qir_opt_dead_code(struct vc4_compile *c)
{
        bool progress = false;
        bool *used = calloc(c->num_temps, sizeof(bool));
        bool sf_used = false;
        /* Whether we're eliminating texture setup currently. */
        bool dce_tex = false;

        struct simple_node *node, *t;
        for (node = c->instructions.prev, t = node->prev;
             &c->instructions != node;
             node = t, t = t->prev) {
                struct qinst *inst = (struct qinst *)node;

                if (inst->dst.file == QFILE_TEMP &&
                    !used[inst->dst.index] &&
                    !inst->sf &&
                    (!qir_has_side_effects(c, inst) ||
                     inst->op == QOP_TEX_RESULT) &&
                    !has_nonremovable_reads(c, inst)) {
                        if (inst->op == QOP_TEX_RESULT) {
                                dce_tex = true;
                                c->num_texture_samples--;
                        }

                        for (int i = 0; i < qir_get_op_nsrc(inst->op); i++) {
                                if (inst->src[i].file != QFILE_VPM)
                                        continue;
                                uint32_t attr = inst->src[i].index / 4;
                                uint32_t offset = (inst->src[i].index % 4) * 4;

                                if (c->vattr_sizes[attr] == offset + 4) {
                                        c->num_inputs--;
                                        c->vattr_sizes[attr] -= 4;
                                }
                        }

                        dce(c, inst);
                        progress = true;
                        continue;
                }

                if (qir_depends_on_flags(inst))
                        sf_used = true;
                if (inst->sf) {
                        if (!sf_used) {
                                if (debug) {
                                        fprintf(stderr, "Removing SF on: ");
                                        qir_dump_inst(c, inst);
                                        fprintf(stderr, "\n");
                                }

                                inst->sf = false;
                                progress = true;
                        }
                        sf_used = false;
                }

                if (inst->op == QOP_TEX_RESULT)
                        dce_tex = false;

                if (dce_tex && (inst->op == QOP_TEX_S ||
                                inst->op == QOP_TEX_T ||
                                inst->op == QOP_TEX_R ||
                                inst->op == QOP_TEX_B ||
                                inst->op == QOP_TEX_DIRECT)) {
                        dce(c, inst);
                        progress = true;
                        continue;
                }

                for (int i = 0; i < qir_get_op_nsrc(inst->op); i++) {
                        if (inst->src[i].file == QFILE_TEMP)
                                used[inst->src[i].index] = true;
                }
        }

        free(used);

        return progress;
}
