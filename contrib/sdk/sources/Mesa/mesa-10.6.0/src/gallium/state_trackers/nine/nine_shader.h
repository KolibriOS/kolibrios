/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef _NINE_SHADER_H_
#define _NINE_SHADER_H_

#include "d3d9types.h"
#include "d3d9caps.h"
#include "nine_defines.h"
#include "pipe/p_state.h" /* PIPE_MAX_ATTRIBS */
#include "util/u_memory.h"

struct NineDevice9;

struct nine_lconstf /* NOTE: both pointers should be FREE'd by the user */
{
    struct nine_range *ranges; /* single MALLOC, but next-pointers valid */
    float *data;
};

struct nine_shader_info
{
    unsigned type; /* in, PIPE_SHADER_x */

    uint8_t version; /* (major << 4) | minor */

    const DWORD *byte_code; /* in, pointer to shader tokens */
    DWORD        byte_size; /* out, size of data at byte_code */

    void *cso; /* out, pipe cso for bind_vs,fs_state */

    uint16_t input_map[PIPE_MAX_ATTRIBS]; /* VS input -> NINE_DECLUSAGE_x */
    uint8_t num_inputs; /* there may be unused inputs (NINE_DECLUSAGE_NONE) */

    boolean position_t; /* out, true if VP writes pre-transformed position */
    boolean point_size; /* out, true if VP writes point size */

    uint32_t sampler_ps1xtypes; /* 2 bits per sampler */
    uint16_t sampler_mask; /* out, which samplers are being used */
    uint16_t sampler_mask_shadow; /* in, which samplers use depth compare */
    uint8_t rt_mask; /* out, which render targets are being written */

    unsigned const_i_base; /* in vec4 (16 byte) units */
    unsigned const_b_base; /* in vec4 (16 byte) units */
    unsigned const_used_size;

    unsigned const_float_slots;
    unsigned const_int_slots;
    unsigned const_bool_slots;

    struct nine_lconstf lconstf; /* out, NOTE: members to be free'd by user */
};

static INLINE void
nine_info_mark_const_f_used(struct nine_shader_info *info, int idx)
{
    if (info->const_float_slots < (idx + 1))
        info->const_float_slots = idx + 1;
}
static INLINE void
nine_info_mark_const_i_used(struct nine_shader_info *info, int idx)
{
    if (info->const_int_slots < (idx + 1))
        info->const_int_slots = idx + 1;
}
static INLINE void
nine_info_mark_const_b_used(struct nine_shader_info *info, int idx)
{
    if (info->const_bool_slots < (idx + 1))
        info->const_bool_slots = idx + 1;
}

HRESULT
nine_translate_shader(struct NineDevice9 *device, struct nine_shader_info *);


struct nine_shader_variant
{
    struct nine_shader_variant *next;
    void *cso;
    uint32_t key;
};

static INLINE void *
nine_shader_variant_get(struct nine_shader_variant *list, uint32_t key)
{
    while (list->key != key && list->next)
        list = list->next;
    if (list->key == key)
        return list->cso;
    return NULL;
}

static INLINE boolean
nine_shader_variant_add(struct nine_shader_variant *list,
                        uint32_t key, void *cso)
{
    while (list->next) {
        assert(list->key != key);
        list = list->next;
    }
    list->next = MALLOC_STRUCT(nine_shader_variant);
    if (!list->next)
        return FALSE;
    list->next->next = NULL;
    list->next->key = key;
    list->next->cso = cso;
    return TRUE;
}

static INLINE void
nine_shader_variants_free(struct nine_shader_variant *list)
{
    while (list->next) {
        struct nine_shader_variant *ptr = list->next;
        list->next = ptr->next;
        FREE(ptr);
    }
}

#endif /* _NINE_SHADER_H_ */
