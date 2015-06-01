/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2012-2013 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#ifndef ILO_FENCE_H
#define ILO_FENCE_H

#include "intel_winsys.h"

#include "ilo_core.h"
#include "ilo_dev.h"

struct ilo_fence {
   struct intel_bo *seq_bo;
};

static inline void
ilo_fence_init(struct ilo_fence *fence, const struct ilo_dev *dev)
{
   /* no-op */
}

static inline void
ilo_fence_cleanup(struct ilo_fence *fence)
{
   intel_bo_unref(fence->seq_bo);
}

/**
 * Set the sequence bo for waiting.  The fence is considered signaled when
 * there is no sequence bo.
 */
static inline void
ilo_fence_set_seq_bo(struct ilo_fence *fence, struct intel_bo *seq_bo)
{
   intel_bo_unref(fence->seq_bo);
   fence->seq_bo = intel_bo_ref(seq_bo);
}

/**
 * Wait for the fence to be signaled or until \p timeout nanoseconds has
 * passed.  It will wait indefinitely when \p timeout is negative.
 */
static inline bool
ilo_fence_wait(struct ilo_fence *fence, int64_t timeout)
{
   return (!fence->seq_bo || intel_bo_wait(fence->seq_bo, timeout) == 0);
}

#endif /* ILO_FENCE_H */
