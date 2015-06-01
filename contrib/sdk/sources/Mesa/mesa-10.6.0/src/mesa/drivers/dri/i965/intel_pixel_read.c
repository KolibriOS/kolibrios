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

#include "main/glheader.h"
#include "main/enums.h"
#include "main/mtypes.h"
#include "main/macros.h"
#include "main/fbobject.h"
#include "main/image.h"
#include "main/bufferobj.h"
#include "main/readpix.h"
#include "main/state.h"
#include "main/glformats.h"
#include "drivers/common/meta.h"

#include "brw_context.h"
#include "intel_screen.h"
#include "intel_batchbuffer.h"
#include "intel_blit.h"
#include "intel_buffers.h"
#include "intel_fbo.h"
#include "intel_mipmap_tree.h"
#include "intel_pixel.h"
#include "intel_buffer_objects.h"
#include "intel_tiled_memcpy.h"

#define FILE_DEBUG_FLAG DEBUG_PIXEL

/**
 * \brief A fast path for glReadPixels
 *
 * This fast path is taken when the source format is BGRA, RGBA,
 * A or L and when the texture memory is X- or Y-tiled.  It downloads
 * the source data by directly mapping the memory without a GTT fence.
 * This then needs to be de-tiled on the CPU before presenting the data to
 * the user in the linear fasion.
 *
 * This is a performance win over the conventional texture download path.
 * In the conventional texture download path, the texture is either mapped
 * through the GTT or copied to a linear buffer with the blitter before
 * handing off to a software path.  This allows us to avoid round-tripping
 * through the GPU (in the case where we would be blitting) and do only a
 * single copy operation.
 */
static bool
intel_readpixels_tiled_memcpy(struct gl_context * ctx,
                              GLint xoffset, GLint yoffset,
                              GLsizei width, GLsizei height,
                              GLenum format, GLenum type,
                              GLvoid * pixels,
                              const struct gl_pixelstore_attrib *pack)
{
   struct brw_context *brw = brw_context(ctx);
   struct gl_renderbuffer *rb = ctx->ReadBuffer->_ColorReadBuffer;

   /* This path supports reading from color buffers only */
   if (rb == NULL)
      return false;

   struct intel_renderbuffer *irb = intel_renderbuffer(rb);
   int dst_pitch;

   /* The miptree's buffer. */
   drm_intel_bo *bo;

   int error = 0;

   uint32_t cpp;
   mem_copy_fn mem_copy = NULL;

   /* This fastpath is restricted to specific renderbuffer types:
    * a 2D BGRA, RGBA, L8 or A8 texture. It could be generalized to support
    * more types.
    */
   if (!brw->has_llc ||
       !(type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_INT_8_8_8_8_REV) ||
       pixels == NULL ||
       _mesa_is_bufferobj(pack->BufferObj) ||
       pack->Alignment > 4 ||
       pack->SkipPixels > 0 ||
       pack->SkipRows > 0 ||
       (pack->RowLength != 0 && pack->RowLength != width) ||
       pack->SwapBytes ||
       pack->LsbFirst ||
       pack->Invert)
      return false;

   /* This renderbuffer can come from a texture.  In this case, we impose
    * some of the same restrictions we have for textures and adjust for
    * miplevels.
    */
   if (rb->TexImage) {
      if (rb->TexImage->TexObject->Target != GL_TEXTURE_2D &&
          rb->TexImage->TexObject->Target != GL_TEXTURE_RECTANGLE)
         return false;

      int level = rb->TexImage->Level + rb->TexImage->TexObject->MinLevel;

      /* Adjust x and y offset based on miplevel */
      xoffset += irb->mt->level[level].level_x;
      yoffset += irb->mt->level[level].level_y;
   }

   /* It is possible that the renderbuffer (or underlying texture) is
    * multisampled.  Since ReadPixels from a multisampled buffer requires a
    * multisample resolve, we can't handle this here
    */
   if (rb->NumSamples > 1)
      return false;

   /* We can't handle copying from RGBX or BGRX because the tiled_memcpy
    * function doesn't set the last channel to 1.
    */
   if (rb->Format == MESA_FORMAT_B8G8R8X8_UNORM ||
       rb->Format == MESA_FORMAT_R8G8B8X8_UNORM)
      return false;

   if (!intel_get_memcpy(rb->Format, format, type, &mem_copy, &cpp,
                         INTEL_DOWNLOAD))
      return false;

   if (!irb->mt ||
       (irb->mt->tiling != I915_TILING_X &&
       irb->mt->tiling != I915_TILING_Y)) {
      /* The algorithm is written only for X- or Y-tiled memory. */
      return false;
   }

   /* Since we are going to read raw data to the miptree, we need to resolve
    * any pending fast color clears before we start.
    */
   intel_miptree_resolve_color(brw, irb->mt);

   bo = irb->mt->bo;

   if (drm_intel_bo_references(brw->batch.bo, bo)) {
      perf_debug("Flushing before mapping a referenced bo.\n");
      intel_batchbuffer_flush(brw);
   }

   error = brw_bo_map(brw, bo, false /* write enable */, "miptree");
   if (error) {
      DBG("%s: failed to map bo\n", __func__);
      return false;
   }

   dst_pitch = _mesa_image_row_stride(pack, width, format, type);

   /* For a window-system renderbuffer, the buffer is actually flipped
    * vertically, so we need to handle that.  Since the detiling function
    * can only really work in the forwards direction, we have to be a
    * little creative.  First, we compute the Y-offset of the first row of
    * the renderbuffer (in renderbuffer coordinates).  We then match that
    * with the last row of the client's data.  Finally, we give
    * tiled_to_linear a negative pitch so that it walks through the
    * client's data backwards as it walks through the renderbufer forwards.
    */
   if (rb->Name == 0) {
      yoffset = rb->Height - yoffset - height;
      pixels += (ptrdiff_t) (height - 1) * dst_pitch;
      dst_pitch = -dst_pitch;
   }

   /* We postponed printing this message until having committed to executing
    * the function.
    */
   DBG("%s: x,y=(%d,%d) (w,h)=(%d,%d) format=0x%x type=0x%x "
       "mesa_format=0x%x tiling=%d "
       "pack=(alignment=%d row_length=%d skip_pixels=%d skip_rows=%d)\n",
       __func__, xoffset, yoffset, width, height,
       format, type, rb->Format, irb->mt->tiling,
       pack->Alignment, pack->RowLength, pack->SkipPixels,
       pack->SkipRows);

   tiled_to_linear(
      xoffset * cpp, (xoffset + width) * cpp,
      yoffset, yoffset + height,
      pixels - (ptrdiff_t) yoffset * dst_pitch - (ptrdiff_t) xoffset * cpp,
      bo->virtual,
      dst_pitch, irb->mt->pitch,
      brw->has_swizzling,
      irb->mt->tiling,
      mem_copy
   );

   drm_intel_bo_unmap(bo);
   return true;
}

void
intelReadPixels(struct gl_context * ctx,
                GLint x, GLint y, GLsizei width, GLsizei height,
                GLenum format, GLenum type,
                const struct gl_pixelstore_attrib *pack, GLvoid * pixels)
{
   bool ok;

   struct brw_context *brw = brw_context(ctx);
   bool dirty;

   DBG("%s\n", __func__);

   if (_mesa_is_bufferobj(pack->BufferObj)) {
      if (_mesa_meta_pbo_GetTexSubImage(ctx, 2, NULL, x, y, 0, width, height, 1,
                                        format, type, pixels, pack)) {
         /* _mesa_meta_pbo_GetTexSubImage() implements PBO transfers by
          * binding the user-provided BO as a fake framebuffer and rendering
          * to it.  This breaks the invariant of the GL that nothing is able
          * to render to a BO, causing nondeterministic corruption issues
          * because the render cache is not coherent with a number of other
          * caches that the BO could potentially be bound to afterwards.
          *
          * This could be solved in the same way that we guarantee texture
          * coherency after a texture is attached to a framebuffer and
          * rendered to, but that would involve checking *all* BOs bound to
          * the pipeline for the case we need to emit a cache flush due to
          * previous rendering to any of them -- Including vertex, index,
          * uniform, atomic counter, shader image, transform feedback,
          * indirect draw buffers, etc.
          *
          * That would increase the per-draw call overhead even though it's
          * very unlikely that any of the BOs bound to the pipeline has been
          * rendered to via a PBO at any point, so it seems better to just
          * flush here unconditionally.
          */
         intel_batchbuffer_emit_mi_flush(brw);
         return;
      }

      perf_debug("%s: fallback to CPU mapping in PBO case\n", __func__);
   }

   ok = intel_readpixels_tiled_memcpy(ctx, x, y, width, height,
                                      format, type, pixels, pack);
   if(ok)
      return;

   /* glReadPixels() wont dirty the front buffer, so reset the dirty
    * flag after calling intel_prepare_render(). */
   dirty = brw->front_buffer_dirty;
   intel_prepare_render(brw);
   brw->front_buffer_dirty = dirty;

   /* Update Mesa state before calling _mesa_readpixels().
    * XXX this may not be needed since ReadPixels no longer uses the
    * span code.
    */

   if (ctx->NewState)
      _mesa_update_state(ctx);

   _mesa_readpixels(ctx, x, y, width, height, format, type, pack, pixels);

   /* There's an intel_prepare_render() call in intelSpanRenderStart(). */
   brw->front_buffer_dirty = dirty;
}
