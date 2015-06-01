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

#ifndef ILO_BUFFER_H
#define ILO_BUFFER_H

#include "intel_winsys.h"

#include "ilo_core.h"
#include "ilo_dev.h"

struct ilo_buffer {
   unsigned bo_size;

   struct intel_bo *bo;
};

static inline void
ilo_buffer_init(struct ilo_buffer *buf, const struct ilo_dev *dev,
                unsigned size, uint32_t bind, uint32_t flags)
{
   buf->bo_size = size;

   /*
    * From the Sandy Bridge PRM, volume 1 part 1, page 118:
    *
    *     "For buffers, which have no inherent "height," padding requirements
    *      are different. A buffer must be padded to the next multiple of 256
    *      array elements, with an additional 16 bytes added beyond that to
    *      account for the L1 cache line."
    */
   if (bind & PIPE_BIND_SAMPLER_VIEW)
      buf->bo_size = align(buf->bo_size, 256) + 16;

   if ((bind & PIPE_BIND_VERTEX_BUFFER) && ilo_dev_gen(dev) < ILO_GEN(7.5)) {
      /*
       * As noted in ilo_format_translate(), we treat some 3-component formats
       * as 4-component formats to work around hardware limitations.  Imagine
       * the case where the vertex buffer holds a single
       * PIPE_FORMAT_R16G16B16_FLOAT vertex, and buf->bo_size is 6.  The
       * hardware would fail to fetch it at boundary check because the vertex
       * buffer is expected to hold a PIPE_FORMAT_R16G16B16A16_FLOAT vertex
       * and that takes at least 8 bytes.
       *
       * For the workaround to work, we should add 2 to the bo size.  But that
       * would waste a page when the bo size is already page aligned.  Let's
       * round it to page size for now and revisit this when needed.
       */
      buf->bo_size = align(buf->bo_size, 4096);
   }
}

static inline void
ilo_buffer_cleanup(struct ilo_buffer *buf)
{
   intel_bo_unref(buf->bo);
}

static inline void
ilo_buffer_set_bo(struct ilo_buffer *buf, struct intel_bo *bo)
{
   intel_bo_unref(buf->bo);
   buf->bo = intel_bo_ref(bo);
}

#endif /* ILO_BUFFER_H */
