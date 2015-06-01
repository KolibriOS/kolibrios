/*
 * Copyright © 2012 Intel Corporation
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

#include "main/blend.h"
#include "main/mtypes.h"
#include "main/samplerobj.h"
#include "main/texformat.h"
#include "main/teximage.h"
#include "program/prog_parameter.h"

#include "intel_mipmap_tree.h"
#include "intel_batchbuffer.h"
#include "intel_tex.h"
#include "intel_fbo.h"
#include "intel_buffer_objects.h"

#include "brw_context.h"
#include "brw_state.h"
#include "brw_defines.h"
#include "brw_wm.h"

/**
 * Convert an swizzle enumeration (i.e. SWIZZLE_X) to one of the Gen7.5+
 * "Shader Channel Select" enumerations (i.e. HSW_SCS_RED).  The mappings are
 *
 * SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_ZERO, SWIZZLE_ONE
 *         0          1          2          3             4            5
 *         4          5          6          7             0            1
 *   SCS_RED, SCS_GREEN,  SCS_BLUE, SCS_ALPHA,     SCS_ZERO,     SCS_ONE
 *
 * which is simply adding 4 then modding by 8 (or anding with 7).
 */
static unsigned
swizzle_to_scs(unsigned swizzle)
{
   return (swizzle + 4) & 7;
}

static uint32_t
surface_tiling_mode(uint32_t tiling)
{
   switch (tiling) {
   case I915_TILING_X:
      return GEN8_SURFACE_TILING_X;
   case I915_TILING_Y:
      return GEN8_SURFACE_TILING_Y;
   default:
      return GEN8_SURFACE_TILING_NONE;
   }
}

static unsigned
vertical_alignment(const struct intel_mipmap_tree *mt)
{
   switch (mt->align_h) {
   case 4:
      return GEN8_SURFACE_VALIGN_4;
   case 8:
      return GEN8_SURFACE_VALIGN_8;
   case 16:
      return GEN8_SURFACE_VALIGN_16;
   default:
      unreachable("Unsupported vertical surface alignment.");
   }
}

static unsigned
horizontal_alignment(const struct intel_mipmap_tree *mt)
{
   switch (mt->align_w) {
   case 4:
      return GEN8_SURFACE_HALIGN_4;
   case 8:
      return GEN8_SURFACE_HALIGN_8;
   case 16:
      return GEN8_SURFACE_HALIGN_16;
   default:
      unreachable("Unsupported horizontal surface alignment.");
   }
}

static uint32_t *
allocate_surface_state(struct brw_context *brw, uint32_t *out_offset, int index)
{
   int dwords = brw->gen >= 9 ? 16 : 13;
   uint32_t *surf = __brw_state_batch(brw, AUB_TRACE_SURFACE_STATE,
                                      dwords * 4, 64, index, out_offset);
   memset(surf, 0, dwords * 4);
   return surf;
}

static void
gen8_emit_buffer_surface_state(struct brw_context *brw,
                               uint32_t *out_offset,
                               drm_intel_bo *bo,
                               unsigned buffer_offset,
                               unsigned surface_format,
                               unsigned buffer_size,
                               unsigned pitch,
                               bool rw)
{
   const unsigned mocs = brw->gen >= 9 ? SKL_MOCS_WB : BDW_MOCS_WB;
   uint32_t *surf = allocate_surface_state(brw, out_offset, -1);

   surf[0] = BRW_SURFACE_BUFFER << BRW_SURFACE_TYPE_SHIFT |
             surface_format << BRW_SURFACE_FORMAT_SHIFT |
             BRW_SURFACE_RC_READ_WRITE;
   surf[1] = SET_FIELD(mocs, GEN8_SURFACE_MOCS);

   surf[2] = SET_FIELD((buffer_size - 1) & 0x7f, GEN7_SURFACE_WIDTH) |
             SET_FIELD(((buffer_size - 1) >> 7) & 0x3fff, GEN7_SURFACE_HEIGHT);
   if (surface_format == BRW_SURFACEFORMAT_RAW)
      surf[3] = SET_FIELD(((buffer_size - 1) >> 21) & 0x3ff, BRW_SURFACE_DEPTH);
   else
      surf[3] = SET_FIELD(((buffer_size - 1) >> 21) & 0x3f, BRW_SURFACE_DEPTH);
   surf[3] |= (pitch - 1);
   surf[7] = SET_FIELD(HSW_SCS_RED,   GEN7_SURFACE_SCS_R) |
             SET_FIELD(HSW_SCS_GREEN, GEN7_SURFACE_SCS_G) |
             SET_FIELD(HSW_SCS_BLUE,  GEN7_SURFACE_SCS_B) |
             SET_FIELD(HSW_SCS_ALPHA, GEN7_SURFACE_SCS_A);
   /* reloc */
   *((uint64_t *) &surf[8]) = (bo ? bo->offset64 : 0) + buffer_offset;

   /* Emit relocation to surface contents. */
   if (bo) {
      drm_intel_bo_emit_reloc(brw->batch.bo, *out_offset + 8 * 4,
                              bo, buffer_offset, I915_GEM_DOMAIN_SAMPLER,
                              rw ? I915_GEM_DOMAIN_SAMPLER : 0);
   }
}

static void
gen8_emit_texture_surface_state(struct brw_context *brw,
                                struct intel_mipmap_tree *mt,
                                GLenum target,
                                unsigned min_layer, unsigned max_layer,
                                unsigned min_level, unsigned max_level,
                                unsigned format,
                                unsigned swizzle,
                                uint32_t *surf_offset,
                                bool rw, bool for_gather)
{
   const unsigned depth = max_layer - min_layer;
   struct intel_mipmap_tree *aux_mt = NULL;
   uint32_t aux_mode = 0;
   uint32_t mocs_wb = brw->gen >= 9 ? SKL_MOCS_WB : BDW_MOCS_WB;
   int surf_index = surf_offset - &brw->wm.base.surf_offset[0];
   unsigned tiling_mode, pitch;

   if (mt->format == MESA_FORMAT_S_UINT8) {
      tiling_mode = GEN8_SURFACE_TILING_W;
      pitch = 2 * mt->pitch;
   } else {
      tiling_mode = surface_tiling_mode(mt->tiling);
      pitch = mt->pitch;
   }

   if (mt->mcs_mt) {
      aux_mt = mt->mcs_mt;
      aux_mode = GEN8_SURFACE_AUX_MODE_MCS;
   }

   uint32_t *surf = allocate_surface_state(brw, surf_offset, surf_index);

   surf[0] = translate_tex_target(target) << BRW_SURFACE_TYPE_SHIFT |
             format << BRW_SURFACE_FORMAT_SHIFT |
             vertical_alignment(mt) |
             horizontal_alignment(mt) |
             tiling_mode;

   if (target == GL_TEXTURE_CUBE_MAP ||
       target == GL_TEXTURE_CUBE_MAP_ARRAY) {
      surf[0] |= BRW_SURFACE_CUBEFACE_ENABLES;
   }

   if (_mesa_is_array_texture(target) || target == GL_TEXTURE_CUBE_MAP)
      surf[0] |= GEN8_SURFACE_IS_ARRAY;

   surf[1] = SET_FIELD(mocs_wb, GEN8_SURFACE_MOCS) | mt->qpitch >> 2;

   surf[2] = SET_FIELD(mt->logical_width0 - 1, GEN7_SURFACE_WIDTH) |
             SET_FIELD(mt->logical_height0 - 1, GEN7_SURFACE_HEIGHT);

   surf[3] = SET_FIELD(depth - 1, BRW_SURFACE_DEPTH) | (pitch - 1);

   surf[4] = gen7_surface_msaa_bits(mt->num_samples, mt->msaa_layout) |
             SET_FIELD(min_layer, GEN7_SURFACE_MIN_ARRAY_ELEMENT) |
             SET_FIELD(depth - 1, GEN7_SURFACE_RENDER_TARGET_VIEW_EXTENT);

   surf[5] = SET_FIELD(min_level - mt->first_level, GEN7_SURFACE_MIN_LOD) |
             (max_level - min_level - 1); /* mip count */

   if (aux_mt) {
      surf[6] = SET_FIELD(mt->qpitch / 4, GEN8_SURFACE_AUX_QPITCH) |
                SET_FIELD((aux_mt->pitch / 128) - 1, GEN8_SURFACE_AUX_PITCH) |
                aux_mode;
   } else {
      surf[6] = 0;
   }

   surf[7] = mt->fast_clear_color_value |
      SET_FIELD(swizzle_to_scs(GET_SWZ(swizzle, 0)), GEN7_SURFACE_SCS_R) |
      SET_FIELD(swizzle_to_scs(GET_SWZ(swizzle, 1)), GEN7_SURFACE_SCS_G) |
      SET_FIELD(swizzle_to_scs(GET_SWZ(swizzle, 2)), GEN7_SURFACE_SCS_B) |
      SET_FIELD(swizzle_to_scs(GET_SWZ(swizzle, 3)), GEN7_SURFACE_SCS_A);

   *((uint64_t *) &surf[8]) = mt->bo->offset64 + mt->offset; /* reloc */

   if (aux_mt) {
      *((uint64_t *) &surf[10]) = aux_mt->bo->offset64;
      drm_intel_bo_emit_reloc(brw->batch.bo, *surf_offset + 10 * 4,
                              aux_mt->bo, 0,
                              I915_GEM_DOMAIN_SAMPLER,
                              (rw ? I915_GEM_DOMAIN_SAMPLER : 0));
   } else {
      surf[10] = 0;
      surf[11] = 0;
   }
   surf[12] = 0;

   /* Emit relocation to surface contents */
   drm_intel_bo_emit_reloc(brw->batch.bo,
                           *surf_offset + 8 * 4,
                           mt->bo,
                           mt->offset,
                           I915_GEM_DOMAIN_SAMPLER,
                           (rw ? I915_GEM_DOMAIN_SAMPLER : 0));
}

static void
gen8_update_texture_surface(struct gl_context *ctx,
                            unsigned unit,
                            uint32_t *surf_offset,
                            bool for_gather)
{
   struct brw_context *brw = brw_context(ctx);
   struct gl_texture_object *obj = ctx->Texture.Unit[unit]._Current;

   if (obj->Target == GL_TEXTURE_BUFFER) {
      brw_update_buffer_texture_surface(ctx, unit, surf_offset);

   } else {
      struct gl_texture_image *firstImage = obj->Image[0][obj->BaseLevel];
      struct intel_texture_object *intel_obj = intel_texture_object(obj);
      struct intel_mipmap_tree *mt = intel_obj->mt;
      struct gl_sampler_object *sampler = _mesa_get_samplerobj(ctx, unit);
      /* If this is a view with restricted NumLayers, then our effective depth
       * is not just the miptree depth.
       */
      const unsigned depth = (obj->Immutable && obj->Target != GL_TEXTURE_3D ?
                              obj->NumLayers : mt->logical_depth0);

      /* Handling GL_ALPHA as a surface format override breaks 1.30+ style
       * texturing functions that return a float, as our code generation always
       * selects the .x channel (which would always be 0).
       */
      const bool alpha_depth = obj->DepthMode == GL_ALPHA &&
         (firstImage->_BaseFormat == GL_DEPTH_COMPONENT ||
          firstImage->_BaseFormat == GL_DEPTH_STENCIL);
      const unsigned swizzle = (unlikely(alpha_depth) ? SWIZZLE_XYZW :
                                brw_get_texture_swizzle(&brw->ctx, obj));

      unsigned format = translate_tex_format(brw, intel_obj->_Format,
                                             sampler->sRGBDecode);
      if (obj->StencilSampling && firstImage->_BaseFormat == GL_DEPTH_STENCIL) {
         mt = mt->stencil_mt;
         format = BRW_SURFACEFORMAT_R8_UINT;
      }

      gen8_emit_texture_surface_state(brw, mt, obj->Target,
                                      obj->MinLayer, obj->MinLayer + depth,
                                      obj->MinLevel + obj->BaseLevel,
                                      obj->MinLevel + intel_obj->_MaxLevel + 1,
                                      format, swizzle, surf_offset,
                                      false, for_gather);
   }
}

/**
 * Creates a null surface.
 *
 * This is used when the shader doesn't write to any color output.  An FB
 * write to target 0 will still be emitted, because that's how the thread is
 * terminated (and computed depth is returned), so we need to have the
 * hardware discard the target 0 color output..
 */
static void
gen8_emit_null_surface_state(struct brw_context *brw,
                             unsigned width,
                             unsigned height,
                             unsigned samples,
                             uint32_t *out_offset)
{
   uint32_t *surf = allocate_surface_state(brw, out_offset, -1);

   surf[0] = BRW_SURFACE_NULL << BRW_SURFACE_TYPE_SHIFT |
             BRW_SURFACEFORMAT_B8G8R8A8_UNORM << BRW_SURFACE_FORMAT_SHIFT |
             GEN8_SURFACE_TILING_Y;
   surf[2] = SET_FIELD(width - 1, GEN7_SURFACE_WIDTH) |
             SET_FIELD(height - 1, GEN7_SURFACE_HEIGHT);
}

/**
 * Sets up a surface state structure to point at the given region.
 * While it is only used for the front/back buffer currently, it should be
 * usable for further buffers when doing ARB_draw_buffer support.
 */
static uint32_t
gen8_update_renderbuffer_surface(struct brw_context *brw,
                                 struct gl_renderbuffer *rb,
                                 bool layered, unsigned unit /* unused */,
                                 uint32_t surf_index)
{
   struct gl_context *ctx = &brw->ctx;
   struct intel_renderbuffer *irb = intel_renderbuffer(rb);
   struct intel_mipmap_tree *mt = irb->mt;
   struct intel_mipmap_tree *aux_mt = NULL;
   uint32_t aux_mode = 0;
   unsigned width = mt->logical_width0;
   unsigned height = mt->logical_height0;
   unsigned pitch = mt->pitch;
   uint32_t tiling = mt->tiling;
   uint32_t format = 0;
   uint32_t surf_type;
   uint32_t offset;
   bool is_array = false;
   int depth = MAX2(irb->layer_count, 1);
   const int min_array_element = (mt->format == MESA_FORMAT_S_UINT8) ?
      irb->mt_layer : (irb->mt_layer / MAX2(mt->num_samples, 1));
   GLenum gl_target =
      rb->TexImage ? rb->TexImage->TexObject->Target : GL_TEXTURE_2D;
   /* FINISHME: Use PTE MOCS on Skylake. */
   uint32_t mocs = brw->gen >= 9 ? SKL_MOCS_WT : BDW_MOCS_PTE;

   intel_miptree_used_for_rendering(mt);

   switch (gl_target) {
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_CUBE_MAP:
      surf_type = BRW_SURFACE_2D;
      is_array = true;
      depth *= 6;
      break;
   case GL_TEXTURE_3D:
      depth = MAX2(irb->mt->logical_depth0, 1);
      /* fallthrough */
   default:
      surf_type = translate_tex_target(gl_target);
      is_array = _mesa_tex_target_is_array(gl_target);
      break;
   }

   /* _NEW_BUFFERS */
   /* Render targets can't use IMS layout. Stencil in turn gets configured as
    * single sampled and indexed manually by the program.
    */
   if (mt->format == MESA_FORMAT_S_UINT8) {
      brw_configure_w_tiled(mt, true, &width, &height, &pitch,
                            &tiling, &format);
   } else {
      assert(mt->msaa_layout != INTEL_MSAA_LAYOUT_IMS);
      assert(brw_render_target_supported(brw, rb));
      mesa_format rb_format = _mesa_get_render_format(ctx,
                                                      intel_rb_format(irb));
      format = brw->render_target_format[rb_format];
      if (unlikely(!brw->format_supported_as_render_target[rb_format]))
         _mesa_problem(ctx, "%s: renderbuffer format %s unsupported\n",
                       __func__, _mesa_get_format_name(rb_format));
   }

   if (mt->mcs_mt) {
      aux_mt = mt->mcs_mt;
      aux_mode = GEN8_SURFACE_AUX_MODE_MCS;
   }

   uint32_t *surf = allocate_surface_state(brw, &offset, surf_index);

   surf[0] = (surf_type << BRW_SURFACE_TYPE_SHIFT) |
             (is_array ? GEN7_SURFACE_IS_ARRAY : 0) |
             (format << BRW_SURFACE_FORMAT_SHIFT) |
             vertical_alignment(mt) |
             horizontal_alignment(mt) |
             surface_tiling_mode(tiling);

   surf[1] = SET_FIELD(mocs, GEN8_SURFACE_MOCS) | mt->qpitch >> 2;

   surf[2] = SET_FIELD(width - 1, GEN7_SURFACE_WIDTH) |
             SET_FIELD(height - 1, GEN7_SURFACE_HEIGHT);

   surf[3] = (depth - 1) << BRW_SURFACE_DEPTH_SHIFT |
             (pitch - 1); /* Surface Pitch */

   surf[4] = min_array_element << GEN7_SURFACE_MIN_ARRAY_ELEMENT_SHIFT |
             (depth - 1) << GEN7_SURFACE_RENDER_TARGET_VIEW_EXTENT_SHIFT;

   if (mt->format != MESA_FORMAT_S_UINT8)
      surf[4] |= gen7_surface_msaa_bits(mt->num_samples, mt->msaa_layout);

   surf[5] = irb->mt_level - irb->mt->first_level;

   if (aux_mt) {
      surf[6] = SET_FIELD(mt->qpitch / 4, GEN8_SURFACE_AUX_QPITCH) |
                SET_FIELD((aux_mt->pitch / 128) - 1, GEN8_SURFACE_AUX_PITCH) |
                aux_mode;
   } else {
      surf[6] = 0;
   }

   surf[7] = mt->fast_clear_color_value |
             SET_FIELD(HSW_SCS_RED,   GEN7_SURFACE_SCS_R) |
             SET_FIELD(HSW_SCS_GREEN, GEN7_SURFACE_SCS_G) |
             SET_FIELD(HSW_SCS_BLUE,  GEN7_SURFACE_SCS_B) |
             SET_FIELD(HSW_SCS_ALPHA, GEN7_SURFACE_SCS_A);

   assert(mt->offset % mt->cpp == 0);
   *((uint64_t *) &surf[8]) = mt->bo->offset64 + mt->offset; /* reloc */

   if (aux_mt) {
      *((uint64_t *) &surf[10]) = aux_mt->bo->offset64;
      drm_intel_bo_emit_reloc(brw->batch.bo,
                              offset + 10 * 4,
                              aux_mt->bo, 0,
                              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER);
   } else {
      surf[10] = 0;
      surf[11] = 0;
   }
   surf[12] = 0;

   drm_intel_bo_emit_reloc(brw->batch.bo,
                           offset + 8 * 4,
                           mt->bo,
                           mt->offset,
                           I915_GEM_DOMAIN_RENDER,
                           I915_GEM_DOMAIN_RENDER);

   return offset;
}

void
gen8_init_vtable_surface_functions(struct brw_context *brw)
{
   brw->vtbl.update_texture_surface = gen8_update_texture_surface;
   brw->vtbl.update_renderbuffer_surface = gen8_update_renderbuffer_surface;
   brw->vtbl.emit_null_surface_state = gen8_emit_null_surface_state;
   brw->vtbl.emit_texture_surface_state = gen8_emit_texture_surface_state;
   brw->vtbl.emit_buffer_surface_state = gen8_emit_buffer_surface_state;
}
