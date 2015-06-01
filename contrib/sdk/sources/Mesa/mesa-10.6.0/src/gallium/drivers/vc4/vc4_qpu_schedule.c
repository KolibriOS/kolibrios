/*
 * Copyright © 2010 Intel Corporation
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
 * @file vc4_qpu_schedule.c
 *
 * The basic model of the list scheduler is to take a basic block, compute a
 * DAG of the dependencies, and make a list of the DAG heads.  Heuristically
 * pick a DAG head, then put all the children that are now DAG heads into the
 * list of things to schedule.
 *
 * The goal of scheduling here is to pack pairs of operations together in a
 * single QPU instruction.
 */

#include "vc4_qir.h"
#include "vc4_qpu.h"
#include "util/ralloc.h"

static bool debug;

struct schedule_node_child;

struct schedule_node {
        struct simple_node link;
        struct queued_qpu_inst *inst;
        struct schedule_node_child *children;
        uint32_t child_count;
        uint32_t child_array_size;
        uint32_t parent_count;

        /**
         * Minimum number of cycles from scheduling this instruction until the
         * end of the program, based on the slowest dependency chain through
         * the children.
         */
        uint32_t delay;

        /**
         * cycles between this instruction being scheduled and when its result
         * can be consumed.
         */
        uint32_t latency;

        /**
         * Which uniform from uniform_data[] this instruction read, or -1 if
         * not reading a uniform.
         */
        int uniform;
};

struct schedule_node_child {
        struct schedule_node *node;
        bool write_after_read;
};

/* When walking the instructions in reverse, we need to swap before/after in
 * add_dep().
 */
enum direction { F, R };

struct schedule_state {
        struct schedule_node *last_r[6];
        struct schedule_node *last_ra[32];
        struct schedule_node *last_rb[32];
        struct schedule_node *last_sf;
        struct schedule_node *last_vpm_read;
        struct schedule_node *last_tmu_write;
        struct schedule_node *last_tlb;
        struct schedule_node *last_vpm;
        enum direction dir;
};

static void
add_dep(struct schedule_state *state,
        struct schedule_node *before,
        struct schedule_node *after,
        bool write)
{
        bool write_after_read = !write && state->dir == R;

        if (!before || !after)
                return;

        assert(before != after);

        if (state->dir == R) {
                struct schedule_node *t = before;
                before = after;
                after = t;
        }

        for (int i = 0; i < before->child_count; i++) {
                if (before->children[i].node == after &&
                    (before->children[i].write_after_read == write_after_read)) {
                        return;
                }
        }

        if (before->child_array_size <= before->child_count) {
                before->child_array_size = MAX2(before->child_array_size * 2, 16);
                before->children = reralloc(before, before->children,
                                            struct schedule_node_child,
                                            before->child_array_size);
        }

        before->children[before->child_count].node = after;
        before->children[before->child_count].write_after_read =
                write_after_read;
        before->child_count++;
        after->parent_count++;
}

static void
add_read_dep(struct schedule_state *state,
              struct schedule_node *before,
              struct schedule_node *after)
{
        add_dep(state, before, after, false);
}

static void
add_write_dep(struct schedule_state *state,
              struct schedule_node **before,
              struct schedule_node *after)
{
        add_dep(state, *before, after, true);
        *before = after;
}

static bool
qpu_writes_r4(uint64_t inst)
{
        uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);

        switch(sig) {
        case QPU_SIG_COLOR_LOAD:
        case QPU_SIG_LOAD_TMU0:
        case QPU_SIG_LOAD_TMU1:
        case QPU_SIG_ALPHA_MASK_LOAD:
                return true;
        default:
                return false;
        }
}

static void
process_raddr_deps(struct schedule_state *state, struct schedule_node *n,
                   uint32_t raddr, bool is_a)
{
        switch (raddr) {
        case QPU_R_VARY:
                add_write_dep(state, &state->last_r[5], n);
                break;

        case QPU_R_VPM:
                add_write_dep(state, &state->last_vpm_read, n);
                break;

        case QPU_R_UNIF:
        case QPU_R_NOP:
        case QPU_R_ELEM_QPU:
        case QPU_R_XY_PIXEL_COORD:
        case QPU_R_MS_REV_FLAGS:
                break;

        default:
                if (raddr < 32) {
                        if (is_a)
                                add_read_dep(state, state->last_ra[raddr], n);
                        else
                                add_read_dep(state, state->last_rb[raddr], n);
                } else {
                        fprintf(stderr, "unknown raddr %d\n", raddr);
                        abort();
                }
                break;
        }
}

static bool
is_tmu_write(uint32_t waddr)
{
        switch (waddr) {
        case QPU_W_TMU0_S:
        case QPU_W_TMU0_T:
        case QPU_W_TMU0_R:
        case QPU_W_TMU0_B:
        case QPU_W_TMU1_S:
        case QPU_W_TMU1_T:
        case QPU_W_TMU1_R:
        case QPU_W_TMU1_B:
                return true;
        default:
                return false;
        }
}

static bool
reads_uniform(uint64_t inst)
{
        if (QPU_GET_FIELD(inst, QPU_SIG) == QPU_SIG_LOAD_IMM)
                return false;

        return (QPU_GET_FIELD(inst, QPU_RADDR_A) == QPU_R_UNIF ||
                (QPU_GET_FIELD(inst, QPU_RADDR_B) == QPU_R_UNIF &&
                 QPU_GET_FIELD(inst, QPU_SIG) != QPU_SIG_SMALL_IMM) ||
                is_tmu_write(QPU_GET_FIELD(inst, QPU_WADDR_ADD)) ||
                is_tmu_write(QPU_GET_FIELD(inst, QPU_WADDR_MUL)));
}

static void
process_mux_deps(struct schedule_state *state, struct schedule_node *n,
                 uint32_t mux)
{
        if (mux != QPU_MUX_A && mux != QPU_MUX_B)
                add_read_dep(state, state->last_r[mux], n);
}


static void
process_waddr_deps(struct schedule_state *state, struct schedule_node *n,
                   uint32_t waddr, bool is_add)
{
        uint64_t inst = n->inst->inst;
        bool is_a = is_add ^ ((inst & QPU_WS) != 0);

        if (waddr < 32) {
                if (is_a) {
                        add_write_dep(state, &state->last_ra[waddr], n);
                } else {
                        add_write_dep(state, &state->last_rb[waddr], n);
                }
        } else if (is_tmu_write(waddr)) {
                add_write_dep(state, &state->last_tmu_write, n);
        } else if (qpu_waddr_is_tlb(waddr)) {
                add_write_dep(state, &state->last_tlb, n);
        } else {
                switch (waddr) {
                case QPU_W_ACC0:
                case QPU_W_ACC1:
                case QPU_W_ACC2:
                case QPU_W_ACC3:
                case QPU_W_ACC5:
                        add_write_dep(state, &state->last_r[waddr - QPU_W_ACC0],
                                      n);
                        break;

                case QPU_W_VPM:
                        add_write_dep(state, &state->last_vpm, n);
                        break;

                case QPU_W_VPMVCD_SETUP:
                        if (is_a)
                                add_write_dep(state, &state->last_vpm_read, n);
                        else
                                add_write_dep(state, &state->last_vpm, n);
                        break;

                case QPU_W_SFU_RECIP:
                case QPU_W_SFU_RECIPSQRT:
                case QPU_W_SFU_EXP:
                case QPU_W_SFU_LOG:
                        add_write_dep(state, &state->last_r[4], n);
                        break;

                case QPU_W_TLB_STENCIL_SETUP:
                        /* This isn't a TLB operation that does things like
                         * implicitly lock the scoreboard, but it does have to
                         * appear before TLB_Z, and each of the TLB_STENCILs
                         * have to schedule in the same order relative to each
                         * other.
                         */
                        add_write_dep(state, &state->last_tlb, n);
                        break;

                case QPU_W_NOP:
                        break;

                default:
                        fprintf(stderr, "Unknown waddr %d\n", waddr);
                        abort();
                }
        }
}

static void
process_cond_deps(struct schedule_state *state, struct schedule_node *n,
                  uint32_t cond)
{
        switch (cond) {
        case QPU_COND_NEVER:
        case QPU_COND_ALWAYS:
                break;
        default:
                add_read_dep(state, state->last_sf, n);
                break;
        }
}

/**
 * Common code for dependencies that need to be tracked both forward and
 * backward.
 *
 * This is for things like "all reads of r4 have to happen between the r4
 * writes that surround them".
 */
static void
calculate_deps(struct schedule_state *state, struct schedule_node *n)
{
        uint64_t inst = n->inst->inst;
        uint32_t add_op = QPU_GET_FIELD(inst, QPU_OP_ADD);
        uint32_t mul_op = QPU_GET_FIELD(inst, QPU_OP_MUL);
        uint32_t waddr_add = QPU_GET_FIELD(inst, QPU_WADDR_ADD);
        uint32_t waddr_mul = QPU_GET_FIELD(inst, QPU_WADDR_MUL);
        uint32_t raddr_a = QPU_GET_FIELD(inst, QPU_RADDR_A);
        uint32_t raddr_b = QPU_GET_FIELD(inst, QPU_RADDR_B);
        uint32_t add_a = QPU_GET_FIELD(inst, QPU_ADD_A);
        uint32_t add_b = QPU_GET_FIELD(inst, QPU_ADD_B);
        uint32_t mul_a = QPU_GET_FIELD(inst, QPU_MUL_A);
        uint32_t mul_b = QPU_GET_FIELD(inst, QPU_MUL_B);
        uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);

        if (sig != QPU_SIG_LOAD_IMM) {
                process_raddr_deps(state, n, raddr_a, true);
                if (sig != QPU_SIG_SMALL_IMM)
                        process_raddr_deps(state, n, raddr_b, false);
        }

        if (add_op != QPU_A_NOP) {
                process_mux_deps(state, n, add_a);
                process_mux_deps(state, n, add_b);
        }
        if (mul_op != QPU_M_NOP) {
                process_mux_deps(state, n, mul_a);
                process_mux_deps(state, n, mul_b);
        }

        process_waddr_deps(state, n, waddr_add, true);
        process_waddr_deps(state, n, waddr_mul, false);
        if (qpu_writes_r4(inst))
                add_write_dep(state, &state->last_r[4], n);

        switch (sig) {
        case QPU_SIG_SW_BREAKPOINT:
        case QPU_SIG_NONE:
        case QPU_SIG_THREAD_SWITCH:
        case QPU_SIG_LAST_THREAD_SWITCH:
        case QPU_SIG_SMALL_IMM:
        case QPU_SIG_LOAD_IMM:
                break;

        case QPU_SIG_LOAD_TMU0:
        case QPU_SIG_LOAD_TMU1:
                /* TMU loads are coming from a FIFO, so ordering is important.
                 */
                add_write_dep(state, &state->last_tmu_write, n);
                break;

        case QPU_SIG_COLOR_LOAD:
                add_read_dep(state, state->last_tlb, n);
                break;

        case QPU_SIG_PROG_END:
        case QPU_SIG_WAIT_FOR_SCOREBOARD:
        case QPU_SIG_SCOREBOARD_UNLOCK:
        case QPU_SIG_COVERAGE_LOAD:
        case QPU_SIG_COLOR_LOAD_END:
        case QPU_SIG_ALPHA_MASK_LOAD:
        case QPU_SIG_BRANCH:
                fprintf(stderr, "Unhandled signal bits %d\n", sig);
                abort();
        }

        process_cond_deps(state, n, QPU_GET_FIELD(inst, QPU_COND_ADD));
        process_cond_deps(state, n, QPU_GET_FIELD(inst, QPU_COND_ADD));
        if (inst & QPU_SF)
                add_write_dep(state, &state->last_sf, n);
}

static void
calculate_forward_deps(struct vc4_compile *c, struct simple_node *schedule_list)
{
        struct simple_node *node;
        struct schedule_state state;

        memset(&state, 0, sizeof(state));
        state.dir = F;

        foreach(node, schedule_list)
                calculate_deps(&state, (struct schedule_node *)node);
}

static void
calculate_reverse_deps(struct vc4_compile *c, struct simple_node *schedule_list)
{
        struct simple_node *node;
        struct schedule_state state;

        memset(&state, 0, sizeof(state));
        state.dir = R;

        for (node = schedule_list->prev; schedule_list != node; node = node->prev) {
                calculate_deps(&state, (struct schedule_node *)node);
        }
}

struct choose_scoreboard {
        int tick;
        int last_sfu_write_tick;
        uint32_t last_waddr_a, last_waddr_b;
};

static bool
reads_too_soon_after_write(struct choose_scoreboard *scoreboard, uint64_t inst)
{
        uint32_t raddr_a = QPU_GET_FIELD(inst, QPU_RADDR_A);
        uint32_t raddr_b = QPU_GET_FIELD(inst, QPU_RADDR_B);
        uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);
        uint32_t src_muxes[] = {
                QPU_GET_FIELD(inst, QPU_ADD_A),
                QPU_GET_FIELD(inst, QPU_ADD_B),
                QPU_GET_FIELD(inst, QPU_MUL_A),
                QPU_GET_FIELD(inst, QPU_MUL_B),
        };
        for (int i = 0; i < ARRAY_SIZE(src_muxes); i++) {
                if ((src_muxes[i] == QPU_MUX_A &&
                     raddr_a < 32 &&
                     scoreboard->last_waddr_a == raddr_a) ||
                    (src_muxes[i] == QPU_MUX_B &&
                     sig != QPU_SIG_SMALL_IMM &&
                     raddr_b < 32 &&
                     scoreboard->last_waddr_b == raddr_b)) {
                        return true;
                }

                if (src_muxes[i] == QPU_MUX_R4) {
                        if (scoreboard->tick -
                            scoreboard->last_sfu_write_tick <= 2) {
                                return true;
                        }
                }
        }

        return false;
}

static bool
pixel_scoreboard_too_soon(struct choose_scoreboard *scoreboard, uint64_t inst)
{
        return (scoreboard->tick < 2 && qpu_inst_is_tlb(inst));
}

static int
get_instruction_priority(uint64_t inst)
{
        uint32_t waddr_add = QPU_GET_FIELD(inst, QPU_WADDR_ADD);
        uint32_t waddr_mul = QPU_GET_FIELD(inst, QPU_WADDR_MUL);
        uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);
        uint32_t baseline_score;
        uint32_t next_score = 0;

        /* Schedule TLB operations as late as possible, to get more
         * parallelism between shaders.
         */
        if (qpu_inst_is_tlb(inst))
                return next_score;
        next_score++;

        /* Schedule texture read results collection late to hide latency. */
        if (sig == QPU_SIG_LOAD_TMU0 || sig == QPU_SIG_LOAD_TMU1)
                return next_score;
        next_score++;

        /* Default score for things that aren't otherwise special. */
        baseline_score = next_score;
        next_score++;

        /* Schedule texture read setup early to hide their latency better. */
        if (is_tmu_write(waddr_add) || is_tmu_write(waddr_mul))
                return next_score;
        next_score++;

        return baseline_score;
}

static struct schedule_node *
choose_instruction_to_schedule(struct choose_scoreboard *scoreboard,
                               struct simple_node *schedule_list,
                               struct schedule_node *prev_inst)
{
        struct schedule_node *chosen = NULL;
        struct simple_node *node;
        int chosen_prio = 0;

        foreach(node, schedule_list) {
                struct schedule_node *n = (struct schedule_node *)node;
                uint64_t inst = n->inst->inst;

                /* "An instruction must not read from a location in physical
                 *  regfile A or B that was written to by the previous
                 *  instruction."
                 */
                if (reads_too_soon_after_write(scoreboard, inst))
                        continue;

                /* "A scoreboard wait must not occur in the first two
                 *  instructions of a fragment shader. This is either the
                 *  explicit Wait for Scoreboard signal or an implicit wait
                 *  with the first tile-buffer read or write instruction."
                 */
                if (pixel_scoreboard_too_soon(scoreboard, inst))
                        continue;

                /* If we're trying to pair with another instruction, check
                 * that they're compatible.
                 */
                if (prev_inst) {
                        if (prev_inst->uniform != -1 && n->uniform != -1)
                                continue;

                        inst = qpu_merge_inst(prev_inst->inst->inst, inst);
                        if (!inst)
                                continue;
                }

                int prio = get_instruction_priority(inst);

                /* Found a valid instruction.  If nothing better comes along,
                 * this one works.
                 */
                if (!chosen) {
                        chosen = n;
                        chosen_prio = prio;
                        continue;
                }

                if (prio > chosen_prio) {
                        chosen = n;
                        chosen_prio = prio;
                } else if (prio < chosen_prio) {
                        continue;
                }

                if (n->delay > chosen->delay) {
                        chosen = n;
                        chosen_prio = prio;
                } else if (n->delay < chosen->delay) {
                        continue;
                }
        }

        return chosen;
}

static void
update_scoreboard_for_chosen(struct choose_scoreboard *scoreboard,
                             uint64_t inst)
{
        uint32_t waddr_add = QPU_GET_FIELD(inst, QPU_WADDR_ADD);
        uint32_t waddr_mul = QPU_GET_FIELD(inst, QPU_WADDR_MUL);

        if (!(inst & QPU_WS)) {
                scoreboard->last_waddr_a = waddr_add;
                scoreboard->last_waddr_b = waddr_mul;
        } else {
                scoreboard->last_waddr_b = waddr_add;
                scoreboard->last_waddr_a = waddr_mul;
        }

        if ((waddr_add >= QPU_W_SFU_RECIP && waddr_add <= QPU_W_SFU_LOG) ||
            (waddr_mul >= QPU_W_SFU_RECIP && waddr_mul <= QPU_W_SFU_LOG)) {
                scoreboard->last_sfu_write_tick = scoreboard->tick;
        }
}

static void
dump_state(struct simple_node *schedule_list)
{
        struct simple_node *node;

        uint32_t i = 0;
        foreach(node, schedule_list) {
                struct schedule_node *n = (struct schedule_node *)node;

                fprintf(stderr, "%3d: ", i++);
                vc4_qpu_disasm(&n->inst->inst, 1);
                fprintf(stderr, "\n");

                for (int i = 0; i < n->child_count; i++) {
                        struct schedule_node *child = n->children[i].node;
                        if (!child)
                                continue;

                        fprintf(stderr, "   - ");
                        vc4_qpu_disasm(&child->inst->inst, 1);
                        fprintf(stderr, " (%d parents, %c)\n",
                                child->parent_count,
                                n->children[i].write_after_read ? 'w' : 'r');
                }
        }
}

/** Recursive computation of the delay member of a node. */
static void
compute_delay(struct schedule_node *n)
{
        if (!n->child_count) {
                n->delay = 1;
        } else {
                for (int i = 0; i < n->child_count; i++) {
                        if (!n->children[i].node->delay)
                                compute_delay(n->children[i].node);
                        n->delay = MAX2(n->delay,
                                        n->children[i].node->delay + n->latency);
                }
        }
}

static void
mark_instruction_scheduled(struct simple_node *schedule_list,
                           struct schedule_node *node,
                           bool war_only)
{
        if (!node)
                return;

        for (int i = node->child_count - 1; i >= 0; i--) {
                struct schedule_node *child =
                        node->children[i].node;

                if (!child)
                        continue;

                if (war_only && !node->children[i].write_after_read)
                        continue;

                child->parent_count--;
                if (child->parent_count == 0)
                        insert_at_head(schedule_list, &child->link);

                node->children[i].node = NULL;
        }
}

static void
schedule_instructions(struct vc4_compile *c, struct simple_node *schedule_list)
{
        struct simple_node *node, *t;
        struct choose_scoreboard scoreboard;

        /* We reorder the uniforms as we schedule instructions, so save the
         * old data off and replace it.
         */
        uint32_t *uniform_data = c->uniform_data;
        enum quniform_contents *uniform_contents = c->uniform_contents;
        c->uniform_contents = ralloc_array(c, enum quniform_contents,
                                           c->num_uniforms);
        c->uniform_data = ralloc_array(c, uint32_t, c->num_uniforms);
        c->uniform_array_size = c->num_uniforms;
        uint32_t next_uniform = 0;

        memset(&scoreboard, 0, sizeof(scoreboard));
        scoreboard.last_waddr_a = ~0;
        scoreboard.last_waddr_b = ~0;
        scoreboard.last_sfu_write_tick = -10;

        if (debug) {
                fprintf(stderr, "initial deps:\n");
                dump_state(schedule_list);
                fprintf(stderr, "\n");
        }

        /* Remove non-DAG heads from the list. */
        foreach_s(node, t, schedule_list) {
                struct schedule_node *n = (struct schedule_node *)node;

                if (n->parent_count != 0)
                        remove_from_list(&n->link);
        }

        while (!is_empty_list(schedule_list)) {
                struct schedule_node *chosen =
                        choose_instruction_to_schedule(&scoreboard,
                                                       schedule_list,
                                                       NULL);
                struct schedule_node *merge = NULL;

                /* If there are no valid instructions to schedule, drop a NOP
                 * in.
                 */
                uint64_t inst = chosen ? chosen->inst->inst : qpu_NOP();

                if (debug) {
                        fprintf(stderr, "current list:\n");
                        dump_state(schedule_list);
                        fprintf(stderr, "chose: ");
                        vc4_qpu_disasm(&inst, 1);
                        fprintf(stderr, "\n");
                }

                /* Schedule this instruction onto the QPU list. Also try to
                 * find an instruction to pair with it.
                 */
                if (chosen) {
                        remove_from_list(&chosen->link);
                        mark_instruction_scheduled(schedule_list, chosen, true);
                        if (chosen->uniform != -1) {
                                c->uniform_data[next_uniform] =
                                        uniform_data[chosen->uniform];
                                c->uniform_contents[next_uniform] =
                                        uniform_contents[chosen->uniform];
                                next_uniform++;
                        }

                        merge = choose_instruction_to_schedule(&scoreboard,
                                                               schedule_list,
                                                               chosen);
                        if (merge) {
                                remove_from_list(&merge->link);
                                inst = qpu_merge_inst(inst, merge->inst->inst);
                                assert(inst != 0);
                                if (merge->uniform != -1) {
                                        c->uniform_data[next_uniform] =
                                                uniform_data[merge->uniform];
                                        c->uniform_contents[next_uniform] =
                                                uniform_contents[merge->uniform];
                                        next_uniform++;
                                }

                                if (debug) {
                                        fprintf(stderr, "merging: ");
                                        vc4_qpu_disasm(&merge->inst->inst, 1);
                                        fprintf(stderr, "\n");
                                        fprintf(stderr, "resulting in: ");
                                        vc4_qpu_disasm(&inst, 1);
                                        fprintf(stderr, "\n");
                                }
                        }
                }

                if (debug) {
                        fprintf(stderr, "\n");
                }

                qpu_serialize_one_inst(c, inst);

                update_scoreboard_for_chosen(&scoreboard, inst);

                /* Now that we've scheduled a new instruction, some of its
                 * children can be promoted to the list of instructions ready to
                 * be scheduled.  Update the children's unblocked time for this
                 * DAG edge as we do so.
                 */
                mark_instruction_scheduled(schedule_list, chosen, false);
                mark_instruction_scheduled(schedule_list, merge, false);

                scoreboard.tick++;
        }

        assert(next_uniform == c->num_uniforms);
}

static uint32_t waddr_latency(uint32_t waddr)
{
        if (waddr < 32)
                return 2;

        /* Some huge number, really. */
        if (waddr >= QPU_W_TMU0_S && waddr <= QPU_W_TMU1_B)
                return 10;

        switch(waddr) {
        case QPU_W_SFU_RECIP:
        case QPU_W_SFU_RECIPSQRT:
        case QPU_W_SFU_EXP:
        case QPU_W_SFU_LOG:
                return 3;
        default:
                return 1;
        }
}

static uint32_t
instruction_latency(uint64_t inst)
{
        return MAX2(waddr_latency(QPU_GET_FIELD(inst, QPU_WADDR_ADD)),
                    waddr_latency(QPU_GET_FIELD(inst, QPU_WADDR_MUL)));
}

void
qpu_schedule_instructions(struct vc4_compile *c)
{
        void *mem_ctx = ralloc_context(NULL);
        struct simple_node schedule_list;
        struct simple_node *node;

        make_empty_list(&schedule_list);

        if (debug) {
                fprintf(stderr, "Pre-schedule instructions\n");
                foreach(node, &c->qpu_inst_list) {
                        struct queued_qpu_inst *q =
                                (struct queued_qpu_inst *)node;
                        vc4_qpu_disasm(&q->inst, 1);
                        fprintf(stderr, "\n");
                }
                fprintf(stderr, "\n");
        }

        /* Wrap each instruction in a scheduler structure. */
        uint32_t next_uniform = 0;
        while (!is_empty_list(&c->qpu_inst_list)) {
                struct queued_qpu_inst *inst =
                        (struct queued_qpu_inst *)c->qpu_inst_list.next;
                struct schedule_node *n = rzalloc(mem_ctx, struct schedule_node);

                n->inst = inst;
                n->latency = instruction_latency(inst->inst);

                if (reads_uniform(inst->inst)) {
                        n->uniform = next_uniform++;
                } else {
                        n->uniform = -1;
                }
                remove_from_list(&inst->link);
                insert_at_tail(&schedule_list, &n->link);
        }
        assert(next_uniform == c->num_uniforms);

        calculate_forward_deps(c, &schedule_list);
        calculate_reverse_deps(c, &schedule_list);

        foreach(node, &schedule_list) {
                struct schedule_node *n = (struct schedule_node *)node;
                compute_delay(n);
        }

        schedule_instructions(c, &schedule_list);

        if (debug) {
                fprintf(stderr, "Post-schedule instructions\n");
                vc4_qpu_disasm(c->qpu_insts, c->qpu_inst_count);
                fprintf(stderr, "\n");
        }

        ralloc_free(mem_ctx);
}
