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

/** @file vc4_fence.c
 *
 * Seqno-based fence management.
 *
 * We have two mechanisms for waiting in our kernel API: You can wait on a BO
 * to have all rendering to from any process to be completed, or wait on a
 * seqno for that particular seqno to be passed.  The fence API we're
 * implementing is based on waiting for all rendering in the context to have
 * completed (with no reference to what other processes might be doing with
 * the same BOs), so we can just use the seqno of the last rendering we'd
 * fired off as our fence marker.
 */

#include "util/u_inlines.h"

#include "vc4_screen.h"
#include "vc4_bufmgr.h"

struct vc4_fence {
        struct pipe_reference reference;
        uint64_t seqno;
};

static void
vc4_fence_reference(struct pipe_screen *pscreen,
                    struct pipe_fence_handle **pp,
                    struct pipe_fence_handle *pf)
{
        struct vc4_fence **p = (struct vc4_fence **)pp;
        struct vc4_fence *f = (struct vc4_fence *)pf;
        struct vc4_fence *old = *p;

        if (pipe_reference(&(*p)->reference, &f->reference)) {
                free(old);
        }
        *p = f;
}

static boolean
vc4_fence_signalled(struct pipe_screen *pscreen,
                    struct pipe_fence_handle *pf)
{
        struct vc4_screen *screen = vc4_screen(pscreen);
        struct vc4_fence *f = (struct vc4_fence *)pf;

        return vc4_wait_seqno(screen, f->seqno, 0);
}

static boolean
vc4_fence_finish(struct pipe_screen *pscreen,
                 struct pipe_fence_handle *pf,
                 uint64_t timeout_ns)
{
        struct vc4_screen *screen = vc4_screen(pscreen);
        struct vc4_fence *f = (struct vc4_fence *)pf;

        return vc4_wait_seqno(screen, f->seqno, timeout_ns);
}

struct vc4_fence *
vc4_fence_create(struct vc4_screen *screen, uint64_t seqno)
{
        struct vc4_fence *f = calloc(1, sizeof(*f));

        if (!f)
                return NULL;

        pipe_reference_init(&f->reference, 1);
        f->seqno = seqno;

        return f;
}

void
vc4_fence_init(struct vc4_screen *screen)
{
        screen->base.fence_reference = vc4_fence_reference;
        screen->base.fence_signalled = vc4_fence_signalled;
        screen->base.fence_finish = vc4_fence_finish;
}
