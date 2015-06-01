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

#include "ilo_screen.h"
#include "ilo_resource.h"

/*
 * From the Ivy Bridge PRM, volume 1 part 1, page 105:
 *
 *     "In addition to restrictions on maximum height, width, and depth,
 *      surfaces are also restricted to a maximum size in bytes. This
 *      maximum is 2 GB for all products and all surface types."
 */
static const size_t ilo_max_resource_size = 1u << 31;

static const char *
resource_get_bo_name(const struct pipe_resource *templ)
{
   static const char *target_names[PIPE_MAX_TEXTURE_TYPES] = {
      [PIPE_BUFFER] = "buf",
      [PIPE_TEXTURE_1D] = "tex-1d",
      [PIPE_TEXTURE_2D] = "tex-2d",
      [PIPE_TEXTURE_3D] = "tex-3d",
      [PIPE_TEXTURE_CUBE] = "tex-cube",
      [PIPE_TEXTURE_RECT] = "tex-rect",
      [PIPE_TEXTURE_1D_ARRAY] = "tex-1d-array",
      [PIPE_TEXTURE_2D_ARRAY] = "tex-2d-array",
      [PIPE_TEXTURE_CUBE_ARRAY] = "tex-cube-array",
   };
   const char *name = target_names[templ->target];

   if (templ->target == PIPE_BUFFER) {
      switch (templ->bind) {
      case PIPE_BIND_VERTEX_BUFFER:
         name = "buf-vb";
         break;
      case PIPE_BIND_INDEX_BUFFER:
         name = "buf-ib";
         break;
      case PIPE_BIND_CONSTANT_BUFFER:
         name = "buf-cb";
         break;
      case PIPE_BIND_STREAM_OUTPUT:
         name = "buf-so";
         break;
      default:
         break;
      }
   }

   return name;
}

static bool
resource_get_cpu_init(const struct pipe_resource *templ)
{
   return (templ->bind & (PIPE_BIND_DEPTH_STENCIL |
                          PIPE_BIND_RENDER_TARGET |
                          PIPE_BIND_STREAM_OUTPUT)) ? false : true;
}

static enum gen_surface_tiling
winsys_to_surface_tiling(enum intel_tiling_mode tiling)
{
   switch (tiling) {
   case INTEL_TILING_NONE:
      return GEN6_TILING_NONE;
   case INTEL_TILING_X:
      return GEN6_TILING_X;
   case INTEL_TILING_Y:
      return GEN6_TILING_Y;
   default:
      assert(!"unknown tiling");
      return GEN6_TILING_NONE;
   }
}

static inline enum intel_tiling_mode
surface_to_winsys_tiling(enum gen_surface_tiling tiling)
{
   switch (tiling) {
   case GEN6_TILING_NONE:
      return INTEL_TILING_NONE;
   case GEN6_TILING_X:
      return INTEL_TILING_X;
   case GEN6_TILING_Y:
      return INTEL_TILING_Y;
   default:
      assert(!"unknown tiling");
      return GEN6_TILING_NONE;
   }
}

static void
tex_free_slices(struct ilo_texture *tex)
{
   FREE(tex->slices[0]);
}

static bool
tex_alloc_slices(struct ilo_texture *tex)
{
   const struct pipe_resource *templ = &tex->base;
   struct ilo_texture_slice *slices;
   int depth, lv;

   /* sum the depths of all levels */
   depth = 0;
   for (lv = 0; lv <= templ->last_level; lv++)
      depth += u_minify(templ->depth0, lv);

   /*
    * There are (depth * tex->base.array_size) slices in total.  Either depth
    * is one (non-3D) or templ->array_size is one (non-array), but it does
    * not matter.
    */
   slices = CALLOC(depth * templ->array_size, sizeof(*slices));
   if (!slices)
      return false;

   tex->slices[0] = slices;

   /* point to the respective positions in the buffer */
   for (lv = 1; lv <= templ->last_level; lv++) {
      tex->slices[lv] = tex->slices[lv - 1] +
         u_minify(templ->depth0, lv - 1) * templ->array_size;
   }

   return true;
}

static bool
tex_create_bo(struct ilo_texture *tex)
{
   struct ilo_screen *is = ilo_screen(tex->base.screen);
   const char *name = resource_get_bo_name(&tex->base);
   const bool cpu_init = resource_get_cpu_init(&tex->base);
   struct intel_bo *bo;

   bo = intel_winsys_alloc_bo(is->dev.winsys, name,
         tex->image.bo_stride * tex->image.bo_height, cpu_init);

   /* set the tiling for transfer and export */
   if (bo && (tex->image.tiling == GEN6_TILING_X ||
              tex->image.tiling == GEN6_TILING_Y)) {
      const enum intel_tiling_mode tiling =
         surface_to_winsys_tiling(tex->image.tiling);

      if (intel_bo_set_tiling(bo, tiling, tex->image.bo_stride)) {
         intel_bo_unref(bo);
         bo = NULL;
      }
   }
   if (!bo)
      return false;

   ilo_image_set_bo(&tex->image, bo);
   intel_bo_unref(bo);

   return true;
}

static bool
tex_create_separate_stencil(struct ilo_texture *tex)
{
   struct pipe_resource templ = tex->base;
   struct pipe_resource *s8;

   /*
    * Unless PIPE_BIND_DEPTH_STENCIL is set, the resource may have other
    * tilings.  But that should be fine since it will never be bound as the
    * stencil buffer, and our transfer code can handle all tilings.
    */
   templ.format = PIPE_FORMAT_S8_UINT;

   /* no stencil texturing */
   templ.bind &= ~PIPE_BIND_SAMPLER_VIEW;

   s8 = tex->base.screen->resource_create(tex->base.screen, &templ);
   if (!s8)
      return false;

   tex->separate_s8 = ilo_texture(s8);

   assert(tex->separate_s8->image.format == PIPE_FORMAT_S8_UINT);

   return true;
}

static bool
tex_create_hiz(struct ilo_texture *tex)
{
   const struct pipe_resource *templ = &tex->base;
   struct ilo_screen *is = ilo_screen(tex->base.screen);
   struct intel_bo *bo;

   bo = intel_winsys_alloc_bo(is->dev.winsys, "hiz texture",
         tex->image.aux.bo_stride * tex->image.aux.bo_height, false);
   if (!bo)
      return false;

   ilo_image_set_aux_bo(&tex->image, bo);

   if (tex->imported) {
      unsigned lv;

      for (lv = 0; lv <= templ->last_level; lv++) {
         if (tex->image.aux.enables & (1 << lv)) {
            const unsigned num_slices = (templ->target == PIPE_TEXTURE_3D) ?
               u_minify(templ->depth0, lv) : templ->array_size;
            /* this will trigger HiZ resolves */
            const unsigned flags = ILO_TEXTURE_CPU_WRITE;

            ilo_texture_set_slice_flags(tex, lv, 0, num_slices, flags, flags);
         }
      }
   }

   return true;
}

static bool
tex_create_mcs(struct ilo_texture *tex)
{
   struct ilo_screen *is = ilo_screen(tex->base.screen);
   struct intel_bo *bo;

   assert(tex->image.aux.enables == (1 << (tex->base.last_level + 1)) - 1);

   bo = intel_winsys_alloc_bo(is->dev.winsys, "mcs texture",
         tex->image.aux.bo_stride * tex->image.aux.bo_height, false);
   if (!bo)
      return false;

   ilo_image_set_aux_bo(&tex->image, bo);

   return true;
}

static void
tex_destroy(struct ilo_texture *tex)
{
   if (tex->separate_s8)
      tex_destroy(tex->separate_s8);

   ilo_image_cleanup(&tex->image);

   tex_free_slices(tex);
   FREE(tex);
}

static bool
tex_alloc_bos(struct ilo_texture *tex)
{
   struct ilo_screen *is = ilo_screen(tex->base.screen);

   if (!tex->imported && !tex_create_bo(tex))
      return false;

   /* allocate separate stencil resource */
   if (tex->image.separate_stencil && !tex_create_separate_stencil(tex))
      return false;

   switch (tex->image.aux.type) {
   case ILO_IMAGE_AUX_HIZ:
      if (!tex_create_hiz(tex)) {
         /* Separate Stencil Buffer requires HiZ to be enabled */
         if (ilo_dev_gen(&is->dev) == ILO_GEN(6) &&
             tex->image.separate_stencil)
            return false;
      }
      break;
   case ILO_IMAGE_AUX_MCS:
      if (!tex_create_mcs(tex))
         return false;
      break;
   default:
      break;
   }

   return true;
}

static bool
tex_import_handle(struct ilo_texture *tex,
                  const struct winsys_handle *handle)
{
   struct ilo_screen *is = ilo_screen(tex->base.screen);
   const struct pipe_resource *templ = &tex->base;
   const char *name = resource_get_bo_name(&tex->base);
   enum intel_tiling_mode tiling;
   unsigned long pitch;
   struct intel_bo *bo;

   bo = intel_winsys_import_handle(is->dev.winsys, name, handle,
         tex->image.bo_height, &tiling, &pitch);
   if (!bo)
      return false;

   if (!ilo_image_init_for_imported(&tex->image, &is->dev, templ,
            winsys_to_surface_tiling(tiling), pitch)) {
      ilo_err("failed to import handle for texture\n");
      intel_bo_unref(bo);
      return false;
   }

   ilo_image_set_bo(&tex->image, bo);
   intel_bo_unref(bo);

   tex->imported = true;

   return true;
}

static bool
tex_init_image(struct ilo_texture *tex,
               const struct winsys_handle *handle)
{
   struct ilo_screen *is = ilo_screen(tex->base.screen);
   const struct pipe_resource *templ = &tex->base;
   struct ilo_image *img = &tex->image;

   if (handle) {
      if (!tex_import_handle(tex, handle))
         return false;
   } else {
      ilo_image_init(img, &is->dev, templ);
   }

   if (img->bo_height > ilo_max_resource_size / img->bo_stride)
      return false;

   if (templ->flags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT) {
      /* require on-the-fly tiling/untiling or format conversion */
      if (img->tiling == GEN8_TILING_W || img->separate_stencil ||
          img->format != templ->format)
         return false;
   }

   if (!tex_alloc_slices(tex))
      return false;

   return true;
}

static struct pipe_resource *
tex_create(struct pipe_screen *screen,
           const struct pipe_resource *templ,
           const struct winsys_handle *handle)
{
   struct ilo_texture *tex;

   tex = CALLOC_STRUCT(ilo_texture);
   if (!tex)
      return NULL;

   tex->base = *templ;
   tex->base.screen = screen;
   pipe_reference_init(&tex->base.reference, 1);

   if (!tex_init_image(tex, handle)) {
      FREE(tex);
      return NULL;
   }

   if (!tex_alloc_bos(tex)) {
      tex_destroy(tex);
      return NULL;
   }

   return &tex->base;
}

static bool
tex_get_handle(struct ilo_texture *tex, struct winsys_handle *handle)
{
   struct ilo_screen *is = ilo_screen(tex->base.screen);
   enum intel_tiling_mode tiling;
   int err;

   /* must match what tex_create_bo() sets */
   if (tex->image.tiling == GEN8_TILING_W)
      tiling = INTEL_TILING_NONE;
   else
      tiling = surface_to_winsys_tiling(tex->image.tiling);

   err = intel_winsys_export_handle(is->dev.winsys, tex->image.bo, tiling,
         tex->image.bo_stride, tex->image.bo_height, handle);

   return !err;
}

static bool
buf_create_bo(struct ilo_buffer_resource *buf)
{
   struct ilo_screen *is = ilo_screen(buf->base.screen);
   const char *name = resource_get_bo_name(&buf->base);
   const bool cpu_init = resource_get_cpu_init(&buf->base);
   struct intel_bo *bo;

   bo = intel_winsys_alloc_bo(is->dev.winsys, name,
         buf->buffer.bo_size, cpu_init);
   if (!bo)
      return false;

   ilo_buffer_set_bo(&buf->buffer, bo);
   intel_bo_unref(bo);

   return true;
}

static void
buf_destroy(struct ilo_buffer_resource *buf)
{
   ilo_buffer_cleanup(&buf->buffer);
   FREE(buf);
}

static struct pipe_resource *
buf_create(struct pipe_screen *screen, const struct pipe_resource *templ)
{
   const struct ilo_screen *is = ilo_screen(screen);
   struct ilo_buffer_resource *buf;

   buf = CALLOC_STRUCT(ilo_buffer_resource);
   if (!buf)
      return NULL;

   buf->base = *templ;
   buf->base.screen = screen;
   pipe_reference_init(&buf->base.reference, 1);

   ilo_buffer_init(&buf->buffer, &is->dev,
         templ->width0, templ->bind, templ->flags);

   if (buf->buffer.bo_size < templ->width0 ||
       buf->buffer.bo_size > ilo_max_resource_size ||
       !buf_create_bo(buf)) {
      FREE(buf);
      return NULL;
   }

   return &buf->base;
}

static boolean
ilo_can_create_resource(struct pipe_screen *screen,
                        const struct pipe_resource *templ)
{
   struct ilo_image img;

   if (templ->target == PIPE_BUFFER)
      return (templ->width0 <= ilo_max_resource_size);

   memset(&img, 0, sizeof(img));
   ilo_image_init(&img, &ilo_screen(screen)->dev, templ);

   return (img.bo_height <= ilo_max_resource_size / img.bo_stride);
}

static struct pipe_resource *
ilo_resource_create(struct pipe_screen *screen,
                    const struct pipe_resource *templ)
{
   if (templ->target == PIPE_BUFFER)
      return buf_create(screen, templ);
   else
      return tex_create(screen, templ, NULL);
}

static struct pipe_resource *
ilo_resource_from_handle(struct pipe_screen *screen,
                         const struct pipe_resource *templ,
                         struct winsys_handle *handle)
{
   if (templ->target == PIPE_BUFFER)
      return NULL;
   else
      return tex_create(screen, templ, handle);
}

static boolean
ilo_resource_get_handle(struct pipe_screen *screen,
                        struct pipe_resource *res,
                        struct winsys_handle *handle)
{
   if (res->target == PIPE_BUFFER)
      return false;
   else
      return tex_get_handle(ilo_texture(res), handle);

}

static void
ilo_resource_destroy(struct pipe_screen *screen,
                     struct pipe_resource *res)
{
   if (res->target == PIPE_BUFFER)
      buf_destroy((struct ilo_buffer_resource *) res);
   else
      tex_destroy(ilo_texture(res));
}

/**
 * Initialize resource-related functions.
 */
void
ilo_init_resource_functions(struct ilo_screen *is)
{
   is->base.can_create_resource = ilo_can_create_resource;
   is->base.resource_create = ilo_resource_create;
   is->base.resource_from_handle = ilo_resource_from_handle;
   is->base.resource_get_handle = ilo_resource_get_handle;
   is->base.resource_destroy = ilo_resource_destroy;
}

bool
ilo_resource_rename_bo(struct pipe_resource *res)
{
   if (res->target == PIPE_BUFFER) {
      return buf_create_bo((struct ilo_buffer_resource *) res);
   } else {
      struct ilo_texture *tex = ilo_texture(res);

      /* an imported texture cannot be renamed */
      if (tex->imported)
         return false;

      return tex_create_bo(tex);
   }
}
