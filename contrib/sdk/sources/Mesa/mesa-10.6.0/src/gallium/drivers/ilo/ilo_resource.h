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

#ifndef ILO_RESOURCE_H
#define ILO_RESOURCE_H

#include "core/intel_winsys.h"
#include "core/ilo_buffer.h"
#include "core/ilo_image.h"

#include "ilo_common.h"
#include "ilo_screen.h"

enum ilo_texture_flags {
   /*
    * Possible writers of a texture.  There can be at most one writer at any
    * time.
    *
    * Wine set in resolve flags (in ilo_blit_resolve_slices()), they indicate
    * the new writer.  When set in slice flags (ilo_texture_slice::flags),
    * they indicate the writer since last resolve.
    */
   ILO_TEXTURE_RENDER_WRITE   = 1 << 0,
   ILO_TEXTURE_BLT_WRITE      = 1 << 1,
   ILO_TEXTURE_CPU_WRITE      = 1 << 2,

   /*
    * Possible readers of a texture.  There may be multiple readers at any
    * time.
    *
    * When set in resolve flags, they indicate the new readers.  They are
    * never set in slice flags.
    */
   ILO_TEXTURE_RENDER_READ    = 1 << 3,
   ILO_TEXTURE_BLT_READ       = 1 << 4,
   ILO_TEXTURE_CPU_READ       = 1 << 5,

   /*
    * Set when the texture is cleared.
    *
    * When set in resolve flags, the new writer will clear.  When set in slice
    * flags, the slice has been cleared to ilo_texture_slice::clear_value.
    */
   ILO_TEXTURE_CLEAR          = 1 << 6,
};

/**
 * A 3D image slice, cube face, or array layer.
 */
struct ilo_texture_slice {
   unsigned flags;

   /*
    * Slice clear value.  It is served for two purposes
    *
    *  - the clear value used in commands such as 3DSTATE_CLEAR_PARAMS
    *  - the clear value when ILO_TEXTURE_CLEAR is set
    *
    * Since commands such as 3DSTATE_CLEAR_PARAMS expect a single clear value
    * for all slices, ilo_blit_resolve_slices() will silently make all slices
    * to have the same clear value.
    */
   uint32_t clear_value;
};

struct ilo_texture {
   struct pipe_resource base;

   bool imported;

   struct ilo_image image;

   /* XXX thread-safety */
   struct ilo_texture_slice *slices[PIPE_MAX_TEXTURE_LEVELS];

   struct ilo_texture *separate_s8;
};

struct ilo_buffer_resource {
   struct pipe_resource base;

   struct ilo_buffer buffer;
};

static inline struct ilo_buffer *
ilo_buffer(struct pipe_resource *res)
{
   return (res && res->target == PIPE_BUFFER) ?
      &((struct ilo_buffer_resource *) res)->buffer : NULL;
}

static inline struct ilo_texture *
ilo_texture(struct pipe_resource *res)
{
   return (struct ilo_texture *)
      ((res && res->target != PIPE_BUFFER) ? res : NULL);
}

void
ilo_init_resource_functions(struct ilo_screen *is);

bool
ilo_resource_rename_bo(struct pipe_resource *res);

/**
 * Return the bo of the resource.
 */
static inline struct intel_bo *
ilo_resource_get_bo(struct pipe_resource *res)
{
   return (res->target == PIPE_BUFFER) ?
      ilo_buffer(res)->bo : ilo_texture(res)->image.bo;
}

static inline struct ilo_texture_slice *
ilo_texture_get_slice(const struct ilo_texture *tex,
                      unsigned level, unsigned slice)
{
   assert(level <= tex->base.last_level);
   assert(slice < ((tex->base.target == PIPE_TEXTURE_3D) ?
         u_minify(tex->base.depth0, level) : tex->base.array_size));

   return &tex->slices[level][slice];
}

static inline void
ilo_texture_set_slice_flags(struct ilo_texture *tex, unsigned level,
                            unsigned first_slice, unsigned num_slices,
                            unsigned mask, unsigned value)
{
   const struct ilo_texture_slice *last =
      ilo_texture_get_slice(tex, level, first_slice + num_slices - 1);
   struct ilo_texture_slice *slice =
      ilo_texture_get_slice(tex, level, first_slice);

   while (slice <= last) {
      slice->flags = (slice->flags & ~mask) | (value & mask);
      slice++;
   }
}

static inline void
ilo_texture_set_slice_clear_value(struct ilo_texture *tex, unsigned level,
                                  unsigned first_slice, unsigned num_slices,
                                  uint32_t clear_value)
{
   const struct ilo_texture_slice *last =
      ilo_texture_get_slice(tex, level, first_slice + num_slices - 1);
   struct ilo_texture_slice *slice =
      ilo_texture_get_slice(tex, level, first_slice);

   while (slice <= last) {
      slice->clear_value = clear_value;
      slice++;
   }
}

#endif /* ILO_RESOURCE_H */
