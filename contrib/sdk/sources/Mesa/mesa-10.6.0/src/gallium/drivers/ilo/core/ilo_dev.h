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

#ifndef ILO_DEV_H
#define ILO_DEV_H

#include "ilo_core.h"

#define ILO_GEN(gen) ((int) (gen * 100))

#define ILO_DEV_ASSERT(dev, min_gen, max_gen) \
   ilo_dev_assert(dev, ILO_GEN(min_gen), ILO_GEN(max_gen))

struct intel_winsys;

struct ilo_dev {
   struct intel_winsys *winsys;

   /* these mirror intel_winsys_info */
   int devid;
   size_t aperture_total;
   size_t aperture_mappable;
   bool has_llc;
   bool has_address_swizzling;
   bool has_logical_context;
   bool has_ppgtt;
   bool has_timestamp;
   bool has_gen7_sol_reset;

   /* use ilo_dev_gen() to access */
   int gen_opaque;

   int gt;
   int eu_count;
   int thread_count;
   int urb_size;
};

bool
ilo_dev_init(struct ilo_dev *dev, struct intel_winsys *winsys);

void
ilo_dev_cleanup(struct ilo_dev *dev);

static inline int
ilo_dev_gen(const struct ilo_dev *dev)
{
   return dev->gen_opaque;
}

static inline void
ilo_dev_assert(const struct ilo_dev *dev, int min_opqaue, int max_opqaue)
{
   assert(dev->gen_opaque >= min_opqaue && dev->gen_opaque <= max_opqaue);
}

#endif /* ILO_DEV_H */
