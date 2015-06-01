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
 * @file vc4_opt_algebraic.c
 *
 * This is the optimization pass for miscellaneous changes to instructions
 * where we can simplify the operation by some knowledge about the specific
 * operations.
 *
 * Mostly this will be a matter of turning things into MOVs so that they can
 * later be copy-propagated out.
 */

#include "vc4_qir.h"
#include "util/hash_table.h"
#include "util/u_math.h"

static inline uint32_t
index_hash(const void *key)
{
        return (uintptr_t)key;
}

static inline bool
index_compare(const void *a, const void *b)
{
        return a == b;
}

static void
add_uniform(struct hash_table *ht, struct qreg reg)
{
        struct hash_entry *entry;
        void *key = (void *)(uintptr_t)reg.index;

        entry = _mesa_hash_table_search(ht, key);
        if (entry) {
                entry->data++;
        } else {
                _mesa_hash_table_insert(ht, key, (void *)(uintptr_t)1);
        }
}

static void
remove_uniform(struct hash_table *ht, struct qreg reg)
{
        struct hash_entry *entry;
        void *key = (void *)(uintptr_t)reg.index;

        entry = _mesa_hash_table_search(ht, key);
        assert(entry);
        entry->data--;
        if (entry->data == NULL)
                _mesa_hash_table_remove(ht, entry);
}

static bool
is_lowerable_uniform(struct qinst *inst, int i)
{
        if (inst->src[i].file != QFILE_UNIF)
                return false;
        if (qir_is_tex(inst))
                return i != 1;
        return true;
}

void
qir_lower_uniforms(struct vc4_compile *c)
{
        struct simple_node *node;
        struct hash_table *ht =
                _mesa_hash_table_create(c, index_hash, index_compare);

        /* Walk the instruction list, finding which instructions have more
         * than one uniform referenced, and add those uniform values to the
         * ht.
         */
        foreach(node, &c->instructions) {
                struct qinst *inst = (struct qinst *)node;
                uint32_t nsrc = qir_get_op_nsrc(inst->op);

                uint32_t count = 0;
                for (int i = 0; i < nsrc; i++) {
                        if (inst->src[i].file == QFILE_UNIF)
                                count++;
                }

                if (count <= 1)
                        continue;

                for (int i = 0; i < nsrc; i++) {
                        if (is_lowerable_uniform(inst, i))
                                add_uniform(ht, inst->src[i]);
                }
        }

        while (ht->entries) {
                /* Find the most commonly used uniform in instructions that
                 * need a uniform lowered.
                 */
                uint32_t max_count = 0;
                uint32_t max_index = 0;
                struct hash_entry *entry;
                hash_table_foreach(ht, entry) {
                        uint32_t count = (uintptr_t)entry->data;
                        uint32_t index = (uintptr_t)entry->key;
                        if (count > max_count) {
                                max_count = count;
                                max_index = index;
                        }
                }

                /* Now, find the instructions using this uniform and make them
                 * reference a temp instead.
                 */
                struct qreg temp = qir_get_temp(c);
                struct qreg unif = { QFILE_UNIF, max_index };
                struct qinst *mov = qir_inst(QOP_MOV, temp, unif, c->undef);
                insert_at_head(&c->instructions, &mov->link);
                c->defs[temp.index] = mov;
                foreach(node, &c->instructions) {
                        struct qinst *inst = (struct qinst *)node;
                        uint32_t nsrc = qir_get_op_nsrc(inst->op);

                        uint32_t count = 0;
                        for (int i = 0; i < nsrc; i++) {
                                if (inst->src[i].file == QFILE_UNIF)
                                        count++;
                        }

                        if (count <= 1)
                                continue;

                        for (int i = 0; i < nsrc; i++) {
                                if (is_lowerable_uniform(inst, i) &&
                                    inst->src[i].index == max_index) {
                                        inst->src[i] = temp;
                                        remove_uniform(ht, unif);
                                        count--;
                                }
                        }

                        /* If the instruction doesn't need lowering any more,
                         * then drop it from the list.
                         */
                        if (count <= 1) {
                                for (int i = 0; i < nsrc; i++) {
                                        if (is_lowerable_uniform(inst, i))
                                                remove_uniform(ht, inst->src[i]);
                                }
                        }
                }
        }

        _mesa_hash_table_destroy(ht, NULL);
}
