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

#include <inttypes.h>

#include "vc4_context.h"
#include "vc4_qir.h"
#include "vc4_qpu.h"
#include "util/ralloc.h"

static void
vc4_dump_program(struct vc4_compile *c)
{
        fprintf(stderr, "%s prog %d/%d QPU:\n",
                qir_get_stage_name(c->stage),
                c->program_id, c->variant_id);

        for (int i = 0; i < c->qpu_inst_count; i++) {
                fprintf(stderr, "0x%016"PRIx64" ", c->qpu_insts[i]);
                vc4_qpu_disasm(&c->qpu_insts[i], 1);
                fprintf(stderr, "\n");
        }
}

static void
queue(struct vc4_compile *c, uint64_t inst)
{
        struct queued_qpu_inst *q = rzalloc(c, struct queued_qpu_inst);
        q->inst = inst;
        insert_at_tail(&c->qpu_inst_list, &q->link);
}

static uint64_t *
last_inst(struct vc4_compile *c)
{
        struct queued_qpu_inst *q =
                (struct queued_qpu_inst *)last_elem(&c->qpu_inst_list);
        return &q->inst;
}

static void
set_last_cond_add(struct vc4_compile *c, uint32_t cond)
{
        *last_inst(c) = qpu_set_cond_add(*last_inst(c), cond);
}

/**
 * Some special registers can be read from either file, which lets us resolve
 * raddr conflicts without extra MOVs.
 */
static bool
swap_file(struct qpu_reg *src)
{
        switch (src->addr) {
        case QPU_R_UNIF:
        case QPU_R_VARY:
                if (src->mux == QPU_MUX_SMALL_IMM) {
                        return false;
                } else {
                        if (src->mux == QPU_MUX_A)
                                src->mux = QPU_MUX_B;
                        else
                                src->mux = QPU_MUX_A;
                        return true;
                }

        default:
                return false;
        }
}

/**
 * This is used to resolve the fact that we might register-allocate two
 * different operands of an instruction to the same physical register file
 * even though instructions have only one field for the register file source
 * address.
 *
 * In that case, we need to move one to a temporary that can be used in the
 * instruction, instead.  We reserve ra31/rb31 for this purpose.
 */
static void
fixup_raddr_conflict(struct vc4_compile *c,
                     struct qpu_reg dst,
                     struct qpu_reg *src0, struct qpu_reg *src1)
{
        uint32_t mux0 = src0->mux == QPU_MUX_SMALL_IMM ? QPU_MUX_B : src0->mux;
        uint32_t mux1 = src1->mux == QPU_MUX_SMALL_IMM ? QPU_MUX_B : src1->mux;

        if (mux0 <= QPU_MUX_R5 ||
            mux0 != mux1 ||
            (src0->addr == src1->addr &&
             src0->mux == src1->mux)) {
                return;
        }

        if (swap_file(src0) || swap_file(src1))
                return;

        if (mux0 == QPU_MUX_A) {
                queue(c, qpu_a_MOV(qpu_rb(31), *src1));
                *src1 = qpu_rb(31);
        } else {
                queue(c, qpu_a_MOV(qpu_ra(31), *src1));
                *src1 = qpu_ra(31);
        }
}

void
vc4_generate_code(struct vc4_context *vc4, struct vc4_compile *c)
{
        struct qpu_reg *temp_registers = vc4_register_allocate(vc4, c);
        bool discard = false;
        uint32_t inputs_remaining = c->num_inputs;
        uint32_t vpm_read_fifo_count = 0;
        uint32_t vpm_read_offset = 0;
        int last_vpm_read_index = -1;
        /* Map from the QIR ops enum order to QPU unpack bits. */
        static const uint32_t unpack_map[] = {
                QPU_UNPACK_8A,
                QPU_UNPACK_8B,
                QPU_UNPACK_8C,
                QPU_UNPACK_8D,
                QPU_UNPACK_16A_TO_F32,
                QPU_UNPACK_16B_TO_F32,
        };

        make_empty_list(&c->qpu_inst_list);

        switch (c->stage) {
        case QSTAGE_VERT:
        case QSTAGE_COORD:
                /* There's a 4-entry FIFO for VPMVCD reads, each of which can
                 * load up to 16 dwords (4 vec4s) per vertex.
                 */
                while (inputs_remaining) {
                        uint32_t num_entries = MIN2(inputs_remaining, 16);
                        queue(c, qpu_load_imm_ui(qpu_vrsetup(),
                                                 vpm_read_offset |
                                                 0x00001a00 |
                                                 ((num_entries & 0xf) << 20)));
                        inputs_remaining -= num_entries;
                        vpm_read_offset += num_entries;
                        vpm_read_fifo_count++;
                }
                assert(vpm_read_fifo_count <= 4);

                queue(c, qpu_load_imm_ui(qpu_vwsetup(), 0x00001a00));
                break;
        case QSTAGE_FRAG:
                break;
        }

        struct simple_node *node;
        foreach(node, &c->instructions) {
                struct qinst *qinst = (struct qinst *)node;

#if 0
                fprintf(stderr, "translating qinst to qpu: ");
                qir_dump_inst(qinst);
                fprintf(stderr, "\n");
#endif

                static const struct {
                        uint32_t op;
                        bool is_mul;
                } translate[] = {
#define A(name) [QOP_##name] = {QPU_A_##name, false}
#define M(name) [QOP_##name] = {QPU_M_##name, true}
                        A(FADD),
                        A(FSUB),
                        A(FMIN),
                        A(FMAX),
                        A(FMINABS),
                        A(FMAXABS),
                        A(FTOI),
                        A(ITOF),
                        A(ADD),
                        A(SUB),
                        A(SHL),
                        A(SHR),
                        A(ASR),
                        A(MIN),
                        A(MAX),
                        A(AND),
                        A(OR),
                        A(XOR),
                        A(NOT),

                        M(FMUL),
                        M(MUL24),
                };

                struct qpu_reg src[4];
                for (int i = 0; i < qir_get_op_nsrc(qinst->op); i++) {
                        int index = qinst->src[i].index;
                        switch (qinst->src[i].file) {
                        case QFILE_NULL:
                                src[i] = qpu_rn(0);
                                break;
                        case QFILE_TEMP:
                                src[i] = temp_registers[index];
                                break;
                        case QFILE_UNIF:
                                src[i] = qpu_unif();
                                break;
                        case QFILE_VARY:
                                src[i] = qpu_vary();
                                break;
                        case QFILE_SMALL_IMM:
                                src[i].mux = QPU_MUX_SMALL_IMM;
                                src[i].addr = qpu_encode_small_immediate(qinst->src[i].index);
                                /* This should only have returned a valid
                                 * small immediate field, not ~0 for failure.
                                 */
                                assert(src[i].addr <= 47);
                                break;
                        case QFILE_VPM:
                                assert((int)qinst->src[i].index >=
                                       last_vpm_read_index);
                                last_vpm_read_index = qinst->src[i].index;
                                src[i] = qpu_ra(QPU_R_VPM);
                                break;
                        }
                }

                struct qpu_reg dst;
                switch (qinst->dst.file) {
                case QFILE_NULL:
                        dst = qpu_ra(QPU_W_NOP);
                        break;
                case QFILE_TEMP:
                        dst = temp_registers[qinst->dst.index];
                        break;
                case QFILE_VPM:
                        dst = qpu_ra(QPU_W_VPM);
                        break;
                case QFILE_VARY:
                case QFILE_UNIF:
                case QFILE_SMALL_IMM:
                        assert(!"not reached");
                        break;
                }

                switch (qinst->op) {
                case QOP_MOV:
                        /* Skip emitting the MOV if it's a no-op. */
                        if (dst.mux == QPU_MUX_A || dst.mux == QPU_MUX_B ||
                            dst.mux != src[0].mux || dst.addr != src[0].addr) {
                                queue(c, qpu_a_MOV(dst, src[0]));
                        }
                        break;

                case QOP_SEL_X_0_ZS:
                case QOP_SEL_X_0_ZC:
                case QOP_SEL_X_0_NS:
                case QOP_SEL_X_0_NC:
                        queue(c, qpu_a_MOV(dst, src[0]));
                        set_last_cond_add(c, qinst->op - QOP_SEL_X_0_ZS +
                                          QPU_COND_ZS);

                        queue(c, qpu_a_XOR(dst, qpu_r0(), qpu_r0()));
                        set_last_cond_add(c, ((qinst->op - QOP_SEL_X_0_ZS) ^
                                              1) + QPU_COND_ZS);
                        break;

                case QOP_SEL_X_Y_ZS:
                case QOP_SEL_X_Y_ZC:
                case QOP_SEL_X_Y_NS:
                case QOP_SEL_X_Y_NC:
                        queue(c, qpu_a_MOV(dst, src[0]));
                        set_last_cond_add(c, qinst->op - QOP_SEL_X_Y_ZS +
                                          QPU_COND_ZS);

                        queue(c, qpu_a_MOV(dst, src[1]));
                        set_last_cond_add(c, ((qinst->op - QOP_SEL_X_Y_ZS) ^
                                              1) + QPU_COND_ZS);

                        break;

                case QOP_RCP:
                case QOP_RSQ:
                case QOP_EXP2:
                case QOP_LOG2:
                        switch (qinst->op) {
                        case QOP_RCP:
                                queue(c, qpu_a_MOV(qpu_rb(QPU_W_SFU_RECIP),
                                                   src[0]));
                                break;
                        case QOP_RSQ:
                                queue(c, qpu_a_MOV(qpu_rb(QPU_W_SFU_RECIPSQRT),
                                                   src[0]));
                                break;
                        case QOP_EXP2:
                                queue(c, qpu_a_MOV(qpu_rb(QPU_W_SFU_EXP),
                                                   src[0]));
                                break;
                        case QOP_LOG2:
                                queue(c, qpu_a_MOV(qpu_rb(QPU_W_SFU_LOG),
                                                   src[0]));
                                break;
                        default:
                                abort();
                        }

                        queue(c, qpu_a_MOV(dst, qpu_r4()));

                        break;

                case QOP_PACK_8888_F:
                        queue(c, qpu_m_MOV(dst, src[0]));
                        *last_inst(c) |= QPU_PM;
                        *last_inst(c) |= QPU_SET_FIELD(QPU_PACK_MUL_8888,
                                                       QPU_PACK);
                        break;

                case QOP_PACK_8A_F:
                case QOP_PACK_8B_F:
                case QOP_PACK_8C_F:
                case QOP_PACK_8D_F:
                        /* If dst doesn't happen to already contain src[0],
                         * then we have to move it in.
                         */
                        if (qinst->src[0].file != QFILE_NULL &&
                            (src[0].mux != dst.mux || src[0].addr != dst.addr)) {
                                /* Don't overwrite src1 while setting up
                                 * the dst!
                                 */
                                if (dst.mux == src[1].mux &&
                                    dst.addr == src[1].addr) {
                                        queue(c, qpu_m_MOV(qpu_rb(31), src[1]));
                                        src[1] = qpu_rb(31);
                                }

                                queue(c, qpu_m_MOV(dst, src[0]));
                        }

                        queue(c, qpu_m_MOV(dst, src[1]));
                        *last_inst(c) |= QPU_PM;
                        *last_inst(c) |= QPU_SET_FIELD(QPU_PACK_MUL_8A +
                                                       qinst->op - QOP_PACK_8A_F,
                                                       QPU_PACK);
                        break;

                case QOP_FRAG_X:
                        queue(c, qpu_a_ITOF(dst,
                                            qpu_ra(QPU_R_XY_PIXEL_COORD)));
                        break;

                case QOP_FRAG_Y:
                        queue(c, qpu_a_ITOF(dst,
                                            qpu_rb(QPU_R_XY_PIXEL_COORD)));
                        break;

                case QOP_FRAG_REV_FLAG:
                        queue(c, qpu_a_ITOF(dst,
                                            qpu_rb(QPU_R_MS_REV_FLAGS)));
                        break;

                case QOP_FRAG_Z:
                case QOP_FRAG_W:
                        /* QOP_FRAG_Z/W don't emit instructions, just allocate
                         * the register to the Z/W payload.
                         */
                        break;

                case QOP_TLB_DISCARD_SETUP:
                        discard = true;
                        queue(c, qpu_a_MOV(src[0], src[0]));
                        *last_inst(c) |= QPU_SF;
                        break;

                case QOP_TLB_STENCIL_SETUP:
                        queue(c, qpu_a_MOV(qpu_ra(QPU_W_TLB_STENCIL_SETUP), src[0]));
                        break;

                case QOP_TLB_Z_WRITE:
                        queue(c, qpu_a_MOV(qpu_ra(QPU_W_TLB_Z), src[0]));
                        if (discard) {
                                set_last_cond_add(c, QPU_COND_ZS);
                        }
                        break;

                case QOP_TLB_COLOR_READ:
                        queue(c, qpu_NOP());
                        *last_inst(c) = qpu_set_sig(*last_inst(c),
                                                    QPU_SIG_COLOR_LOAD);

                        break;

                case QOP_TLB_COLOR_WRITE:
                        queue(c, qpu_a_MOV(qpu_tlbc(), src[0]));
                        if (discard) {
                                set_last_cond_add(c, QPU_COND_ZS);
                        }
                        break;

                case QOP_VARY_ADD_C:
                        queue(c, qpu_a_FADD(dst, src[0], qpu_r5()));
                        break;

                case QOP_PACK_SCALED: {
                        uint64_t a = (qpu_a_MOV(dst, src[0]) |
                                      QPU_SET_FIELD(QPU_PACK_A_16A,
                                                    QPU_PACK));
                        uint64_t b = (qpu_a_MOV(dst, src[1]) |
                                      QPU_SET_FIELD(QPU_PACK_A_16B,
                                                    QPU_PACK));

                        if (dst.mux == src[1].mux && dst.addr == src[1].addr) {
                                queue(c, b);
                                queue(c, a);
                        } else {
                                queue(c, a);
                                queue(c, b);
                        }
                        break;
                }

                case QOP_TEX_S:
                case QOP_TEX_T:
                case QOP_TEX_R:
                case QOP_TEX_B:
                        queue(c, qpu_a_MOV(qpu_rb(QPU_W_TMU0_S +
                                                  (qinst->op - QOP_TEX_S)),
                                           src[0]));
                        break;

                case QOP_TEX_DIRECT:
                        fixup_raddr_conflict(c, dst, &src[0], &src[1]);
                        queue(c, qpu_a_ADD(qpu_rb(QPU_W_TMU0_S), src[0], src[1]));
                        break;

                case QOP_TEX_RESULT:
                        queue(c, qpu_NOP());
                        *last_inst(c) = qpu_set_sig(*last_inst(c),
                                                    QPU_SIG_LOAD_TMU0);

                        break;

                case QOP_R4_UNPACK_A:
                case QOP_R4_UNPACK_B:
                case QOP_R4_UNPACK_C:
                case QOP_R4_UNPACK_D:
                        assert(src[0].mux == QPU_MUX_R4);
                        queue(c, qpu_a_MOV(dst, src[0]));
                        *last_inst(c) |= QPU_PM;
                        *last_inst(c) |= QPU_SET_FIELD(QPU_UNPACK_8A +
                                                       (qinst->op -
                                                        QOP_R4_UNPACK_A),
                                                       QPU_UNPACK);

                        break;

                case QOP_UNPACK_8A_F:
                case QOP_UNPACK_8B_F:
                case QOP_UNPACK_8C_F:
                case QOP_UNPACK_8D_F:
                case QOP_UNPACK_16A_F:
                case QOP_UNPACK_16B_F: {
                        assert(src[0].mux == QPU_MUX_A);

                        /* Since we're setting the pack bits, if the
                         * destination is in A it would get re-packed.
                         */
                        queue(c, qpu_a_FMAX((dst.mux == QPU_MUX_A ?
                                             qpu_rb(31) : dst),
                                            src[0], src[0]));
                        *last_inst(c) |= QPU_SET_FIELD(unpack_map[qinst->op -
                                                                  QOP_UNPACK_8A_F],
                                                       QPU_UNPACK);

                        if (dst.mux == QPU_MUX_A) {
                                queue(c, qpu_a_MOV(dst, qpu_rb(31)));
                        }
                }
                        break;

                case QOP_UNPACK_8A_I:
                case QOP_UNPACK_8B_I:
                case QOP_UNPACK_8C_I:
                case QOP_UNPACK_8D_I:
                case QOP_UNPACK_16A_I:
                case QOP_UNPACK_16B_I: {
                        assert(src[0].mux == QPU_MUX_A);

                        /* Since we're setting the pack bits, if the
                         * destination is in A it would get re-packed.
                         */
                        queue(c, qpu_a_MOV((dst.mux == QPU_MUX_A ?
                                            qpu_rb(31) : dst), src[0]));
                        *last_inst(c) |= QPU_SET_FIELD(unpack_map[qinst->op -
                                                                  QOP_UNPACK_8A_I],
                                                       QPU_UNPACK);

                        if (dst.mux == QPU_MUX_A) {
                                queue(c, qpu_a_MOV(dst, qpu_rb(31)));
                        }
                }
                        break;

                default:
                        assert(qinst->op < ARRAY_SIZE(translate));
                        assert(translate[qinst->op].op != 0); /* NOPs */

                        /* If we have only one source, put it in the second
                         * argument slot as well so that we don't take up
                         * another raddr just to get unused data.
                         */
                        if (qir_get_op_nsrc(qinst->op) == 1)
                                src[1] = src[0];

                        fixup_raddr_conflict(c, dst, &src[0], &src[1]);

                        if (translate[qinst->op].is_mul) {
                                queue(c, qpu_m_alu2(translate[qinst->op].op,
                                                    dst,
                                                    src[0], src[1]));
                        } else {
                                queue(c, qpu_a_alu2(translate[qinst->op].op,
                                                    dst,
                                                    src[0], src[1]));
                        }

                        break;
                }

                if (qinst->sf) {
                        assert(!qir_is_multi_instruction(qinst));
                        *last_inst(c) |= QPU_SF;
                }
        }

        qpu_schedule_instructions(c);

        /* thread end can't have VPM write or read */
        if (QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_WADDR_ADD) == QPU_W_VPM ||
            QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_WADDR_MUL) == QPU_W_VPM ||
            QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_RADDR_A) == QPU_R_VPM ||
            QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_RADDR_B) == QPU_R_VPM) {
                qpu_serialize_one_inst(c, qpu_NOP());
        }

        /* thread end can't have uniform read */
        if (QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_RADDR_A) == QPU_R_UNIF ||
            QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_RADDR_B) == QPU_R_UNIF) {
                qpu_serialize_one_inst(c, qpu_NOP());
        }

        /* thread end can't have TLB operations */
        if (qpu_inst_is_tlb(c->qpu_insts[c->qpu_inst_count - 1]))
                qpu_serialize_one_inst(c, qpu_NOP());

        c->qpu_insts[c->qpu_inst_count - 1] =
                qpu_set_sig(c->qpu_insts[c->qpu_inst_count - 1],
                            QPU_SIG_PROG_END);
        qpu_serialize_one_inst(c, qpu_NOP());
        qpu_serialize_one_inst(c, qpu_NOP());

        switch (c->stage) {
        case QSTAGE_VERT:
        case QSTAGE_COORD:
                break;
        case QSTAGE_FRAG:
                c->qpu_insts[c->qpu_inst_count - 1] =
                        qpu_set_sig(c->qpu_insts[c->qpu_inst_count - 1],
                                    QPU_SIG_SCOREBOARD_UNLOCK);
                break;
        }

        if (vc4_debug & VC4_DEBUG_QPU)
                vc4_dump_program(c);

        vc4_qpu_validate(c->qpu_insts, c->qpu_inst_count);

        free(temp_registers);
}
