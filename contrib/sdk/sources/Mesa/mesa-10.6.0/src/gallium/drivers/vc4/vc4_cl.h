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

#ifndef VC4_CL_H
#define VC4_CL_H

#include <stdint.h>

#include "util/u_math.h"
#include "util/macros.h"

#include "vc4_packet.h"

struct vc4_bo;

struct vc4_cl {
        void *base;
        void *next;
        uint32_t size;
        uint32_t reloc_next;
        uint32_t reloc_count;
};

void vc4_init_cl(struct vc4_context *vc4, struct vc4_cl *cl);
void vc4_reset_cl(struct vc4_cl *cl);
void vc4_dump_cl(void *cl, uint32_t size, bool is_render);
uint32_t vc4_gem_hindex(struct vc4_context *vc4, struct vc4_bo *bo);

struct PACKED unaligned_16 { uint16_t x; };
struct PACKED unaligned_32 { uint32_t x; };

static inline void
put_unaligned_32(void *ptr, uint32_t val)
{
        struct unaligned_32 *p = ptr;
        p->x = val;
}

static inline void
put_unaligned_16(void *ptr, uint16_t val)
{
        struct unaligned_16 *p = ptr;
        p->x = val;
}

static inline void
cl_u8(struct vc4_cl *cl, uint8_t n)
{
        assert((cl->next - cl->base) + 1 <= cl->size);

        *(uint8_t *)cl->next = n;
        cl->next++;
}

static inline void
cl_u16(struct vc4_cl *cl, uint16_t n)
{
        assert((cl->next - cl->base) + 2 <= cl->size);

        put_unaligned_16(cl->next, n);
        cl->next += 2;
}

static inline void
cl_u32(struct vc4_cl *cl, uint32_t n)
{
        assert((cl->next - cl->base) + 4 <= cl->size);

        put_unaligned_32(cl->next, n);
        cl->next += 4;
}

static inline void
cl_aligned_u32(struct vc4_cl *cl, uint32_t n)
{
        assert((cl->next - cl->base) + 4 <= cl->size);

        *(uint32_t *)cl->next = n;
        cl->next += 4;
}

static inline void
cl_ptr(struct vc4_cl *cl, void *ptr)
{
        assert((cl->next - cl->base) + sizeof(void *) <= cl->size);

        *(void **)cl->next = ptr;
        cl->next += sizeof(void *);
}

static inline void
cl_f(struct vc4_cl *cl, float f)
{
        cl_u32(cl, fui(f));
}

static inline void
cl_aligned_f(struct vc4_cl *cl, float f)
{
        cl_aligned_u32(cl, fui(f));
}

static inline void
cl_start_reloc(struct vc4_cl *cl, uint32_t n)
{
        assert(n == 1 || n == 2);
        assert(cl->reloc_count == 0);
        cl->reloc_count = n;

        cl_u8(cl, VC4_PACKET_GEM_HANDLES);
        cl->reloc_next = cl->next - cl->base;
        cl_u32(cl, 0); /* Space where hindex will be written. */
        cl_u32(cl, 0); /* Space where hindex will be written. */
}

static inline void
cl_start_shader_reloc(struct vc4_cl *cl, uint32_t n)
{
        assert(cl->reloc_count == 0);
        cl->reloc_count = n;
        cl->reloc_next = cl->next - cl->base;

        /* Space where hindex will be written. */
        cl->next += n * 4;
}

static inline void
cl_reloc_hindex(struct vc4_cl *cl, uint32_t hindex, uint32_t offset)
{
        *(uint32_t *)(cl->base + cl->reloc_next) = hindex;
        cl->reloc_next += 4;

        cl->reloc_count--;

        cl_u32(cl, offset);
}

static inline void
cl_aligned_reloc_hindex(struct vc4_cl *cl, uint32_t hindex, uint32_t offset)
{
        *(uint32_t *)(cl->base + cl->reloc_next) = hindex;
        cl->reloc_next += 4;

        cl->reloc_count--;

        cl_aligned_u32(cl, offset);
}

static inline void
cl_reloc(struct vc4_context *vc4, struct vc4_cl *cl,
         struct vc4_bo *bo, uint32_t offset)
{
        cl_reloc_hindex(cl, vc4_gem_hindex(vc4, bo), offset);
}

static inline void
cl_aligned_reloc(struct vc4_context *vc4, struct vc4_cl *cl,
         struct vc4_bo *bo, uint32_t offset)
{
        cl_aligned_reloc_hindex(cl, vc4_gem_hindex(vc4, bo), offset);
}

void cl_ensure_space(struct vc4_cl *cl, uint32_t size);

#endif /* VC4_CL_H */
