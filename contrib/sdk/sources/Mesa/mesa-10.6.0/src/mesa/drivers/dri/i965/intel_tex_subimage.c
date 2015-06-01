
/**************************************************************************
 *
 * Copyright 2003 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "main/bufferobj.h"
#include "main/image.h"
#include "main/macros.h"
#include "main/mtypes.h"
#include "main/pbo.h"
#include "main/texobj.h"
#include "main/texstore.h"
#include "main/texcompress.h"
#include "main/enums.h"
#include "drivers/common/meta.h"

#include "brw_context.h"
#include "intel_batchbuffer.h"
#include "intel_tex.h"
#include "intel_mipmap_tree.h"
#include "intel_blit.h"
#include "intel_tiled_memcpy.h"

#define FILE_DEBUG_FLAG DEBUG_TEXTURE

/**
 * \brief A fast path for glTexImage and glTexSubImage.
 *
 * \param for_glTexImage Was this called from glTexImage or glTexSubImage?
 *
 * This fast path is taken when the texture format is BGRA, RGBA,
 * A or L and when the texture memory is X- or Y-tiled.  It uploads
 * the texture data by mapping the texture memory without a GTT fence, thus
 * acquiring a tiled view of the memory, and then copying sucessive
 * spans within each tile.
 *
 * This is a performance win over the conventional texture upload path because
 * it avoids the performance penalty of writing through the write-combine
 * buffer. In the conventional texture upload path,
 * texstore.c:store_texsubimage(), the texture memory is mapped through a GTT
 * fence, thus acquiring a linear view of the memory, then each row in the
 * image is memcpy'd. In this fast path, we replace each row's copy with
 * a sequence of copies over each linear span in tile.
 *
 * One use case is Google Chrome's paint rectangles.  Chrome (as
 * of version 21) renders each page as a tiling of 256x256 GL_BGRA textures.
 * Each page's content is initially uploaded with glTexImage2D and damaged
 * regions are updated with glTexSubImage2D. On some workloads, the
 * performance gain of this fastpath on Sandybridge is over 5x.
 */
bool
intel_texsubimage_tiled_memcpy(struct gl_context * ctx,
                               GLuint dims,
                               struct gl_texture_image *texImage,
                               GLint xoffset, GLint yoffset, GLint zoffset,
                               GLsizei width, GLsizei height, GLsizei depth,
                               GLenum format, GLenum type,
                               const GLvoid *pixels,
                               const struct gl_pixelstore_attrib *packing,
                               bool for_glTexImage)
{
   struct brw_context *brw = brw_context(ctx);
   struct intel_texture_image *image = intel_texture_image(texImage);
   int src_pitch;

   /* The miptree's buffer. */
   drm_intel_bo *bo;

   int error = 0;

   uint32_t cpp;
   mem_copy_fn mem_copy = NULL;

   /* This fastpath is restricted to specific texture types:
    * a 2D BGRA, RGBA, L8 or A8 texture. It could be generalized to support
    * more types.
    *
    * FINISHME: The restrictions below on packing alignment and packing row
    * length are likely unneeded now because we calculate the source stride
    * with _mesa_image_row_stride. However, before removing the restrictions
    * we need tests.
    */
   if (!brw->has_llc ||
       !(type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_INT_8_8_8_8_REV) ||
       !(texImage->TexObject->Target == GL_TEXTURE_2D ||
         texImage->TexObject->Target == GL_TEXTURE_RECTANGLE) ||
       pixels == NULL ||
       _mesa_is_bufferobj(packing->BufferObj) ||
       packing->Alignment > 4 ||
       packing->SkipPixels > 0 ||
       packing->SkipRows > 0 ||
       (packing->RowLength != 0 && packing->RowLength != width) ||
       packing->SwapBytes ||
       packing->LsbFirst ||
       packing->Invert)
      return false;

   if (!intel_get_memcpy(texImage->TexFormat, format, type, &mem_copy, &cpp,
                         INTEL_UPLOAD))
      return false;

   /* If this is a nontrivial texture view, let another path handle it instead. */
   if (texImage->TexObject->MinLayer)
      return false;

   if (for_glTexImage)
      ctx->Driver.AllocTextureImageBuffer(ctx, texImage);

   if (!image->mt ||
       (image->mt->tiling != I915_TILING_X &&
       image->mt->tiling != I915_TILING_Y)) {
      /* The algorithm is written only for X- or Y-tiled memory. */
      return false;
   }

   /* Since we are going to write raw data to the miptree, we need to resolve
    * any pending fast color clears before we start.
    */
   intel_miptree_resolve_color(brw, image->mt);

   bo = image->mt->bo;

   if (drm_intel_bo_references(brw->batch.bo, bo)) {
      perf_debug("Flushing before mapping a referenced bo.\n");
      intel_batchbuffer_flush(brw);
   }

   error = brw_bo_map(brw, bo, true /* write enable */, "miptree");
   if (error || bo->virtual == NULL) {
      DBG("%s: failed to map bo\n", __func__);
      return false;
   }

   src_pitch = _mesa_image_row_stride(packing, width, format, type);

   /* We postponed printing this message until having committed to executing
    * the function.
    */
   DBG("%s: level=%d offset=(%d,%d) (w,h)=(%d,%d) format=0x%x type=0x%x "
       "mesa_format=0x%x tiling=%d "
       "packing=(alignment=%d row_length=%d skip_pixels=%d skip_rows=%d) "
       "for_glTexImage=%d\n",
       __func__, texImage->Level, xoffset, yoffset, width, height,
       format, type, texImage->TexFormat, image->mt->tiling,
       packing->Alignment, packing->RowLength, packing->SkipPixels,
       packing->SkipRows, for_glTexImage);

   int level = texImage->Level + texImage->TexObject->MinLevel;

   /* Adjust x and y offset based on miplevel */
   xoffset += image->mt->level[level].level_x;
   yoffset += image->mt->level[level].level_y;

   linear_to_tiled(
      xoffset * cpp, (xoffset + width) * cpp,
      yoffset, yoffset + height,
      bo->virtual,
      pixels - (ptrdiff_t) yoffset * src_pitch - (ptrdiff_t) xoffset * cpp,
      image->mt->pitch, src_pitch,
      brw->has_swizzling,
      image->mt->tiling,
      mem_copy
   );

   drm_intel_bo_unmap(bo);
   return true;
}

static void
intelTexSubImage(struct gl_context * ctx,
                 GLuint dims,
                 struct gl_texture_image *texImage,
                 GLint xoffset, GLint yoffset, GLint zoffset,
                 GLsizei width, GLsizei height, GLsizei depth,
                 GLenum format, GLenum type,
                 const GLvoid * pixels,
                 const struct gl_pixelstore_attrib *packing)
{
   struct intel_texture_image *intelImage = intel_texture_image(texImage);
   bool ok;

   bool tex_busy = intelImage->mt && drm_intel_bo_busy(intelImage->mt->bo);

   DBG("%s mesa_format %s target %s format %s type %s level %d %dx%dx%d\n",
       __func__, _mesa_get_format_name(texImage->TexFormat),
       _mesa_lookup_enum_by_nr(texImage->TexObject->Target),
       _mesa_lookup_enum_by_nr(format), _mesa_lookup_enum_by_nr(type),
       texImage->Level, texImage->Width, texImage->Height, texImage->Depth);

   ok = _mesa_meta_pbo_TexSubImage(ctx, dims, texImage,
                                   xoffset, yoffset, zoffset,
                                   width, height, depth, format, type,
                                   pixels, false, tex_busy, packing);
   if (ok)
      return;

   ok = intel_texsubimage_tiled_memcpy(ctx, dims, texImage,
                                       xoffset, yoffset, zoffset,
                                       width, height, depth,
                                       format, type, pixels, packing,
                                       false /*for_glTexImage*/);
   if (ok)
     return;

   _mesa_store_texsubimage(ctx, dims, texImage,
                           xoffset, yoffset, zoffset,
                           width, height, depth,
                           format, type, pixels, packing);
}

void
intelInitTextureSubImageFuncs(struct dd_function_table *functions)
{
   functions->TexSubImage = intelTexSubImage;
}
