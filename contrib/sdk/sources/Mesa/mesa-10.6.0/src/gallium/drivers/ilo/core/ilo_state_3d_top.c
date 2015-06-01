/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2012-2014 LunarG, Inc.
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

#include "genhw/genhw.h"
#include "util/u_dual_blend.h"
#include "util/u_framebuffer.h"
#include "util/u_half.h"
#include "util/u_resource.h"

#include "ilo_buffer.h"
#include "ilo_format.h"
#include "ilo_image.h"
#include "ilo_state_3d.h"
#include "../ilo_shader.h"

static void
ve_init_cso(const struct ilo_dev *dev,
            const struct pipe_vertex_element *state,
            unsigned vb_index,
            struct ilo_ve_cso *cso)
{
   int comp[4] = {
      GEN6_VFCOMP_STORE_SRC,
      GEN6_VFCOMP_STORE_SRC,
      GEN6_VFCOMP_STORE_SRC,
      GEN6_VFCOMP_STORE_SRC,
   };
   int format;

   ILO_DEV_ASSERT(dev, 6, 8);

   switch (util_format_get_nr_components(state->src_format)) {
   case 1: comp[1] = GEN6_VFCOMP_STORE_0;
   case 2: comp[2] = GEN6_VFCOMP_STORE_0;
   case 3: comp[3] = (util_format_is_pure_integer(state->src_format)) ?
                     GEN6_VFCOMP_STORE_1_INT :
                     GEN6_VFCOMP_STORE_1_FP;
   }

   format = ilo_format_translate_vertex(dev, state->src_format);

   STATIC_ASSERT(Elements(cso->payload) >= 2);
   cso->payload[0] =
      vb_index << GEN6_VE_DW0_VB_INDEX__SHIFT |
      GEN6_VE_DW0_VALID |
      format << GEN6_VE_DW0_FORMAT__SHIFT |
      state->src_offset << GEN6_VE_DW0_VB_OFFSET__SHIFT;

   cso->payload[1] =
         comp[0] << GEN6_VE_DW1_COMP0__SHIFT |
         comp[1] << GEN6_VE_DW1_COMP1__SHIFT |
         comp[2] << GEN6_VE_DW1_COMP2__SHIFT |
         comp[3] << GEN6_VE_DW1_COMP3__SHIFT;
}

void
ilo_gpe_init_ve(const struct ilo_dev *dev,
                unsigned num_states,
                const struct pipe_vertex_element *states,
                struct ilo_ve_state *ve)
{
   unsigned i;

   ILO_DEV_ASSERT(dev, 6, 8);

   ve->count = num_states;
   ve->vb_count = 0;

   for (i = 0; i < num_states; i++) {
      const unsigned pipe_idx = states[i].vertex_buffer_index;
      const unsigned instance_divisor = states[i].instance_divisor;
      unsigned hw_idx;

      /*
       * map the pipe vb to the hardware vb, which has a fixed instance
       * divisor
       */
      for (hw_idx = 0; hw_idx < ve->vb_count; hw_idx++) {
         if (ve->vb_mapping[hw_idx] == pipe_idx &&
             ve->instance_divisors[hw_idx] == instance_divisor)
            break;
      }

      /* create one if there is no matching hardware vb */
      if (hw_idx >= ve->vb_count) {
         hw_idx = ve->vb_count++;

         ve->vb_mapping[hw_idx] = pipe_idx;
         ve->instance_divisors[hw_idx] = instance_divisor;
      }

      ve_init_cso(dev, &states[i], hw_idx, &ve->cso[i]);
   }
}

void
ilo_gpe_set_ve_edgeflag(const struct ilo_dev *dev,
                        struct ilo_ve_cso *cso)
{
   int format;

   ILO_DEV_ASSERT(dev, 6, 8);

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 94:
    *
    *     "- This bit (Edge Flag Enable) must only be ENABLED on the last
    *        valid VERTEX_ELEMENT structure.
    *
    *      - When set, Component 0 Control must be set to VFCOMP_STORE_SRC,
    *        and Component 1-3 Control must be set to VFCOMP_NOSTORE.
    *
    *      - The Source Element Format must be set to the UINT format.
    *
    *      - [DevSNB]: Edge Flags are not supported for QUADLIST
    *        primitives.  Software may elect to convert QUADLIST primitives
    *        to some set of corresponding edge-flag-supported primitive
    *        types (e.g., POLYGONs) prior to submission to the 3D pipeline."
    */
   cso->payload[0] |= GEN6_VE_DW0_EDGE_FLAG_ENABLE;

   /*
    * Edge flags have format GEN6_FORMAT_R8_USCALED when defined via
    * glEdgeFlagPointer(), and format GEN6_FORMAT_R32_FLOAT when defined
    * via glEdgeFlag(), as can be seen in vbo_attrib_tmp.h.
    *
    * Since all the hardware cares about is whether the flags are zero or not,
    * we can treat them as the corresponding _UINT formats.
    */
   format = GEN_EXTRACT(cso->payload[0], GEN6_VE_DW0_FORMAT);
   cso->payload[0] &= ~GEN6_VE_DW0_FORMAT__MASK;

   switch (format) {
   case GEN6_FORMAT_R32_FLOAT:
      format = GEN6_FORMAT_R32_UINT;
      break;
   case GEN6_FORMAT_R8_USCALED:
      format = GEN6_FORMAT_R8_UINT;
      break;
   default:
      break;
   }

   cso->payload[0] |= GEN_SHIFT32(format, GEN6_VE_DW0_FORMAT);

   cso->payload[1] =
         GEN6_VFCOMP_STORE_SRC << GEN6_VE_DW1_COMP0__SHIFT |
         GEN6_VFCOMP_NOSTORE << GEN6_VE_DW1_COMP1__SHIFT |
         GEN6_VFCOMP_NOSTORE << GEN6_VE_DW1_COMP2__SHIFT |
         GEN6_VFCOMP_NOSTORE << GEN6_VE_DW1_COMP3__SHIFT;
}

void
ilo_gpe_init_ve_nosrc(const struct ilo_dev *dev,
                          int comp0, int comp1, int comp2, int comp3,
                          struct ilo_ve_cso *cso)
{
   ILO_DEV_ASSERT(dev, 6, 8);

   STATIC_ASSERT(Elements(cso->payload) >= 2);

   assert(comp0 != GEN6_VFCOMP_STORE_SRC &&
          comp1 != GEN6_VFCOMP_STORE_SRC &&
          comp2 != GEN6_VFCOMP_STORE_SRC &&
          comp3 != GEN6_VFCOMP_STORE_SRC);

   cso->payload[0] = GEN6_VE_DW0_VALID;
   cso->payload[1] =
         comp0 << GEN6_VE_DW1_COMP0__SHIFT |
         comp1 << GEN6_VE_DW1_COMP1__SHIFT |
         comp2 << GEN6_VE_DW1_COMP2__SHIFT |
         comp3 << GEN6_VE_DW1_COMP3__SHIFT;
}

void
ilo_gpe_init_vs_cso(const struct ilo_dev *dev,
                    const struct ilo_shader_state *vs,
                    struct ilo_shader_cso *cso)
{
   int start_grf, vue_read_len, sampler_count, max_threads;
   uint32_t dw2, dw4, dw5;

   ILO_DEV_ASSERT(dev, 6, 8);

   start_grf = ilo_shader_get_kernel_param(vs, ILO_KERNEL_URB_DATA_START_REG);
   vue_read_len = ilo_shader_get_kernel_param(vs, ILO_KERNEL_INPUT_COUNT);
   sampler_count = ilo_shader_get_kernel_param(vs, ILO_KERNEL_SAMPLER_COUNT);

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 135:
    *
    *     "(Vertex URB Entry Read Length) Specifies the number of pairs of
    *      128-bit vertex elements to be passed into the payload for each
    *      vertex."
    *
    *     "It is UNDEFINED to set this field to 0 indicating no Vertex URB
    *      data to be read and passed to the thread."
    */
   vue_read_len = (vue_read_len + 1) / 2;
   if (!vue_read_len)
      vue_read_len = 1;

   max_threads = dev->thread_count;
   if (ilo_dev_gen(dev) == ILO_GEN(7.5) && dev->gt == 2)
      max_threads *= 2;

   dw2 = (true) ? 0 : GEN6_THREADDISP_FP_MODE_ALT;
   dw2 |= ((sampler_count + 3) / 4) << GEN6_THREADDISP_SAMPLER_COUNT__SHIFT;

   dw4 = start_grf << GEN6_VS_DW4_URB_GRF_START__SHIFT |
         vue_read_len << GEN6_VS_DW4_URB_READ_LEN__SHIFT |
         0 << GEN6_VS_DW4_URB_READ_OFFSET__SHIFT;

   dw5 = GEN6_VS_DW5_STATISTICS |
         GEN6_VS_DW5_VS_ENABLE;

   if (ilo_dev_gen(dev) >= ILO_GEN(7.5))
      dw5 |= (max_threads - 1) << GEN75_VS_DW5_MAX_THREADS__SHIFT;
   else
      dw5 |= (max_threads - 1) << GEN6_VS_DW5_MAX_THREADS__SHIFT;

   STATIC_ASSERT(Elements(cso->payload) >= 3);
   cso->payload[0] = dw2;
   cso->payload[1] = dw4;
   cso->payload[2] = dw5;
}

static void
gs_init_cso_gen6(const struct ilo_dev *dev,
                 const struct ilo_shader_state *gs,
                 struct ilo_shader_cso *cso)
{
   int start_grf, vue_read_len, max_threads;
   uint32_t dw2, dw4, dw5, dw6;

   ILO_DEV_ASSERT(dev, 6, 6);

   if (ilo_shader_get_type(gs) == PIPE_SHADER_GEOMETRY) {
      start_grf = ilo_shader_get_kernel_param(gs,
            ILO_KERNEL_URB_DATA_START_REG);

      vue_read_len = ilo_shader_get_kernel_param(gs, ILO_KERNEL_INPUT_COUNT);
   }
   else {
      start_grf = ilo_shader_get_kernel_param(gs,
            ILO_KERNEL_VS_GEN6_SO_START_REG);

      vue_read_len = ilo_shader_get_kernel_param(gs, ILO_KERNEL_OUTPUT_COUNT);
   }

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 153:
    *
    *     "Specifies the amount of URB data read and passed in the thread
    *      payload for each Vertex URB entry, in 256-bit register increments.
    *
    *      It is UNDEFINED to set this field (Vertex URB Entry Read Length) to
    *      0 indicating no Vertex URB data to be read and passed to the
    *      thread."
    */
   vue_read_len = (vue_read_len + 1) / 2;
   if (!vue_read_len)
      vue_read_len = 1;

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 154:
    *
    *     "Maximum Number of Threads valid range is [0,27] when Rendering
    *      Enabled bit is set."
    *
    * From the Sandy Bridge PRM, volume 2 part 1, page 173:
    *
    *     "Programming Note: If the GS stage is enabled, software must always
    *      allocate at least one GS URB Entry. This is true even if the GS
    *      thread never needs to output vertices to the pipeline, e.g., when
    *      only performing stream output. This is an artifact of the need to
    *      pass the GS thread an initial destination URB handle."
    *
    * As such, we always enable rendering, and limit the number of threads.
    */
   if (dev->gt == 2) {
      /* maximum is 60, but limited to 28 */
      max_threads = 28;
   }
   else {
      /* maximum is 24, but limited to 21 (see brwCreateContext()) */
      max_threads = 21;
   }

   dw2 = GEN6_THREADDISP_SPF;

   dw4 = vue_read_len << GEN6_GS_DW4_URB_READ_LEN__SHIFT |
         0 << GEN6_GS_DW4_URB_READ_OFFSET__SHIFT |
         start_grf << GEN6_GS_DW4_URB_GRF_START__SHIFT;

   dw5 = (max_threads - 1) << GEN6_GS_DW5_MAX_THREADS__SHIFT |
         GEN6_GS_DW5_STATISTICS |
         GEN6_GS_DW5_SO_STATISTICS |
         GEN6_GS_DW5_RENDER_ENABLE;

   /*
    * we cannot make use of GEN6_GS_REORDER because it will reorder
    * triangle strips according to D3D rules (triangle 2N+1 uses vertices
    * (2N+1, 2N+3, 2N+2)), instead of GL rules (triangle 2N+1 uses vertices
    * (2N+2, 2N+1, 2N+3)).
    */
   dw6 = GEN6_GS_DW6_GS_ENABLE;

   if (ilo_shader_get_kernel_param(gs, ILO_KERNEL_GS_DISCARD_ADJACENCY))
      dw6 |= GEN6_GS_DW6_DISCARD_ADJACENCY;

   if (ilo_shader_get_kernel_param(gs, ILO_KERNEL_VS_GEN6_SO)) {
      const uint32_t svbi_post_inc =
         ilo_shader_get_kernel_param(gs, ILO_KERNEL_GS_GEN6_SVBI_POST_INC);

      dw6 |= GEN6_GS_DW6_SVBI_PAYLOAD_ENABLE;
      if (svbi_post_inc) {
         dw6 |= GEN6_GS_DW6_SVBI_POST_INC_ENABLE |
                svbi_post_inc << GEN6_GS_DW6_SVBI_POST_INC_VAL__SHIFT;
      }
   }

   STATIC_ASSERT(Elements(cso->payload) >= 4);
   cso->payload[0] = dw2;
   cso->payload[1] = dw4;
   cso->payload[2] = dw5;
   cso->payload[3] = dw6;
}

static void
gs_init_cso_gen7(const struct ilo_dev *dev,
                 const struct ilo_shader_state *gs,
                 struct ilo_shader_cso *cso)
{
   int start_grf, vue_read_len, sampler_count, max_threads;
   uint32_t dw2, dw4, dw5;

   ILO_DEV_ASSERT(dev, 7, 7.5);

   start_grf = ilo_shader_get_kernel_param(gs, ILO_KERNEL_URB_DATA_START_REG);
   vue_read_len = ilo_shader_get_kernel_param(gs, ILO_KERNEL_INPUT_COUNT);
   sampler_count = ilo_shader_get_kernel_param(gs, ILO_KERNEL_SAMPLER_COUNT);

   /* in pairs */
   vue_read_len = (vue_read_len + 1) / 2;

   switch (ilo_dev_gen(dev)) {
   case ILO_GEN(7.5):
      max_threads = (dev->gt >= 2) ? 256 : 70;
      break;
   case ILO_GEN(7):
      max_threads = (dev->gt == 2) ? 128 : 36;
      break;
   default:
      max_threads = 1;
      break;
   }

   dw2 = (true) ? 0 : GEN6_THREADDISP_FP_MODE_ALT;
   dw2 |= ((sampler_count + 3) / 4) << GEN6_THREADDISP_SAMPLER_COUNT__SHIFT;

   dw4 = vue_read_len << GEN7_GS_DW4_URB_READ_LEN__SHIFT |
         GEN7_GS_DW4_INCLUDE_VERTEX_HANDLES |
         0 << GEN7_GS_DW4_URB_READ_OFFSET__SHIFT |
         start_grf << GEN7_GS_DW4_URB_GRF_START__SHIFT;

   dw5 = (max_threads - 1) << GEN7_GS_DW5_MAX_THREADS__SHIFT |
         GEN7_GS_DW5_STATISTICS |
         GEN7_GS_DW5_GS_ENABLE;

   STATIC_ASSERT(Elements(cso->payload) >= 3);
   cso->payload[0] = dw2;
   cso->payload[1] = dw4;
   cso->payload[2] = dw5;
}

void
ilo_gpe_init_gs_cso(const struct ilo_dev *dev,
                    const struct ilo_shader_state *gs,
                    struct ilo_shader_cso *cso)
{
   if (ilo_dev_gen(dev) >= ILO_GEN(7))
      gs_init_cso_gen7(dev, gs, cso);
   else
      gs_init_cso_gen6(dev, gs, cso);
}

static void
view_init_null_gen6(const struct ilo_dev *dev,
                    unsigned width, unsigned height,
                    unsigned depth, unsigned level,
                    struct ilo_view_surface *surf)
{
   uint32_t *dw;

   ILO_DEV_ASSERT(dev, 6, 6);

   assert(width >= 1 && height >= 1 && depth >= 1);

   /*
    * From the Sandy Bridge PRM, volume 4 part 1, page 71:
    *
    *     "A null surface will be used in instances where an actual surface is
    *      not bound. When a write message is generated to a null surface, no
    *      actual surface is written to. When a read message (including any
    *      sampling engine message) is generated to a null surface, the result
    *      is all zeros. Note that a null surface type is allowed to be used
    *      with all messages, even if it is not specificially indicated as
    *      supported. All of the remaining fields in surface state are ignored
    *      for null surfaces, with the following exceptions:
    *
    *        * [DevSNB+]: Width, Height, Depth, and LOD fields must match the
    *          depth buffer's corresponding state for all render target
    *          surfaces, including null.
    *        * Surface Format must be R8G8B8A8_UNORM."
    *
    * From the Sandy Bridge PRM, volume 4 part 1, page 82:
    *
    *     "If Surface Type is SURFTYPE_NULL, this field (Tiled Surface) must be
    *      true"
    */

   STATIC_ASSERT(Elements(surf->payload) >= 6);
   dw = surf->payload;

   dw[0] = GEN6_SURFTYPE_NULL << GEN6_SURFACE_DW0_TYPE__SHIFT |
           GEN6_FORMAT_B8G8R8A8_UNORM << GEN6_SURFACE_DW0_FORMAT__SHIFT;

   dw[1] = 0;

   dw[2] = (height - 1) << GEN6_SURFACE_DW2_HEIGHT__SHIFT |
           (width  - 1) << GEN6_SURFACE_DW2_WIDTH__SHIFT |
           level << GEN6_SURFACE_DW2_MIP_COUNT_LOD__SHIFT;

   dw[3] = (depth - 1) << GEN6_SURFACE_DW3_DEPTH__SHIFT |
           GEN6_TILING_X;

   dw[4] = 0;
   dw[5] = 0;
}

static void
view_init_for_buffer_gen6(const struct ilo_dev *dev,
                          const struct ilo_buffer *buf,
                          unsigned offset, unsigned size,
                          unsigned struct_size,
                          enum pipe_format elem_format,
                          bool is_rt, bool render_cache_rw,
                          struct ilo_view_surface *surf)
{
   const int elem_size = util_format_get_blocksize(elem_format);
   int width, height, depth, pitch;
   int surface_format, num_entries;
   uint32_t *dw;

   ILO_DEV_ASSERT(dev, 6, 6);

   /*
    * For SURFTYPE_BUFFER, a SURFACE_STATE specifies an element of a
    * structure in a buffer.
    */

   surface_format = ilo_format_translate_color(dev, elem_format);

   num_entries = size / struct_size;
   /* see if there is enough space to fit another element */
   if (size % struct_size >= elem_size)
      num_entries++;

   /*
    * From the Sandy Bridge PRM, volume 4 part 1, page 76:
    *
    *     "For SURFTYPE_BUFFER render targets, this field (Surface Base
    *      Address) specifies the base address of first element of the
    *      surface. The surface is interpreted as a simple array of that
    *      single element type. The address must be naturally-aligned to the
    *      element size (e.g., a buffer containing R32G32B32A32_FLOAT elements
    *      must be 16-byte aligned).
    *
    *      For SURFTYPE_BUFFER non-rendertarget surfaces, this field specifies
    *      the base address of the first element of the surface, computed in
    *      software by adding the surface base address to the byte offset of
    *      the element in the buffer."
    */
   if (is_rt)
      assert(offset % elem_size == 0);

   /*
    * From the Sandy Bridge PRM, volume 4 part 1, page 77:
    *
    *     "For buffer surfaces, the number of entries in the buffer ranges
    *      from 1 to 2^27."
    */
   assert(num_entries >= 1 && num_entries <= 1 << 27);

   /*
    * From the Sandy Bridge PRM, volume 4 part 1, page 81:
    *
    *     "For surfaces of type SURFTYPE_BUFFER, this field (Surface Pitch)
    *      indicates the size of the structure."
    */
   pitch = struct_size;

   pitch--;
   num_entries--;
   /* bits [6:0] */
   width  = (num_entries & 0x0000007f);
   /* bits [19:7] */
   height = (num_entries & 0x000fff80) >> 7;
   /* bits [26:20] */
   depth  = (num_entries & 0x07f00000) >> 20;

   STATIC_ASSERT(Elements(surf->payload) >= 6);
   dw = surf->payload;

   dw[0] = GEN6_SURFTYPE_BUFFER << GEN6_SURFACE_DW0_TYPE__SHIFT |
           surface_format << GEN6_SURFACE_DW0_FORMAT__SHIFT;
   if (render_cache_rw)
      dw[0] |= GEN6_SURFACE_DW0_RENDER_CACHE_RW;

   dw[1] = offset;

   dw[2] = height << GEN6_SURFACE_DW2_HEIGHT__SHIFT |
           width << GEN6_SURFACE_DW2_WIDTH__SHIFT;

   dw[3] = depth << GEN6_SURFACE_DW3_DEPTH__SHIFT |
           pitch << GEN6_SURFACE_DW3_PITCH__SHIFT;

   dw[4] = 0;
   dw[5] = 0;
}

static void
view_init_for_image_gen6(const struct ilo_dev *dev,
                         const struct ilo_image *img,
                         enum pipe_texture_target target,
                         enum pipe_format format,
                         unsigned first_level,
                         unsigned num_levels,
                         unsigned first_layer,
                         unsigned num_layers,
                         bool is_rt,
                         struct ilo_view_surface *surf)
{
   int surface_type, surface_format;
   int width, height, depth, pitch, lod;
   uint32_t *dw;

   ILO_DEV_ASSERT(dev, 6, 6);

   surface_type = ilo_gpe_gen6_translate_texture(target);
   assert(surface_type != GEN6_SURFTYPE_BUFFER);

   if (format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT && img->separate_stencil)
      format = PIPE_FORMAT_Z32_FLOAT;

   if (is_rt)
      surface_format = ilo_format_translate_render(dev, format);
   else
      surface_format = ilo_format_translate_texture(dev, format);
   assert(surface_format >= 0);

   width = img->width0;
   height = img->height0;
   depth = (target == PIPE_TEXTURE_3D) ? img->depth0 : num_layers;
   pitch = img->bo_stride;

   if (surface_type == GEN6_SURFTYPE_CUBE) {
      /*
       * From the Sandy Bridge PRM, volume 4 part 1, page 81:
       *
       *     "For SURFTYPE_CUBE: [DevSNB+]: for Sampling Engine Surfaces, the
       *      range of this field (Depth) is [0,84], indicating the number of
       *      cube array elements (equal to the number of underlying 2D array
       *      elements divided by 6). For other surfaces, this field must be
       *      zero."
       *
       * When is_rt is true, we treat the texture as a 2D one to avoid the
       * restriction.
       */
      if (is_rt) {
         surface_type = GEN6_SURFTYPE_2D;
      }
      else {
         assert(num_layers % 6 == 0);
         depth = num_layers / 6;
      }
   }

   /* sanity check the size */
   assert(width >= 1 && height >= 1 && depth >= 1 && pitch >= 1);
   switch (surface_type) {
   case GEN6_SURFTYPE_1D:
      assert(width <= 8192 && height == 1 && depth <= 512);
      assert(first_layer < 512 && num_layers <= 512);
      break;
   case GEN6_SURFTYPE_2D:
      assert(width <= 8192 && height <= 8192 && depth <= 512);
      assert(first_layer < 512 && num_layers <= 512);
      break;
   case GEN6_SURFTYPE_3D:
      assert(width <= 2048 && height <= 2048 && depth <= 2048);
      assert(first_layer < 2048 && num_layers <= 512);
      if (!is_rt)
         assert(first_layer == 0);
      break;
   case GEN6_SURFTYPE_CUBE:
      assert(width <= 8192 && height <= 8192 && depth <= 85);
      assert(width == height);
      assert(first_layer < 512 && num_layers <= 512);
      if (is_rt)
         assert(first_layer == 0);
      break;
   default:
      assert(!"unexpected surface type");
      break;
   }

   /* non-full array spacing is supported only on GEN7+ */
   assert(img->walk != ILO_IMAGE_WALK_LOD);
   /* non-interleaved samples are supported only on GEN7+ */
   if (img->sample_count > 1)
      assert(img->interleaved_samples);

   if (is_rt) {
      assert(num_levels == 1);
      lod = first_level;
   }
   else {
      lod = num_levels - 1;
   }

   /*
    * From the Sandy Bridge PRM, volume 4 part 1, page 76:
    *
    *     "Linear render target surface base addresses must be element-size
    *      aligned, for non-YUV surface formats, or a multiple of 2
    *      element-sizes for YUV surface formats. Other linear surfaces have
    *      no alignment requirements (byte alignment is sufficient.)"
    *
    * From the Sandy Bridge PRM, volume 4 part 1, page 81:
    *
    *     "For linear render target surfaces, the pitch must be a multiple
    *      of the element size for non-YUV surface formats. Pitch must be a
    *      multiple of 2 * element size for YUV surface formats."
    *
    * From the Sandy Bridge PRM, volume 4 part 1, page 86:
    *
    *     "For linear surfaces, this field (X Offset) must be zero"
    */
   if (img->tiling == GEN6_TILING_NONE) {
      if (is_rt) {
         const int elem_size = util_format_get_blocksize(format);
         assert(pitch % elem_size == 0);
      }
   }

   STATIC_ASSERT(Elements(surf->payload) >= 6);
   dw = surf->payload;

   dw[0] = surface_type << GEN6_SURFACE_DW0_TYPE__SHIFT |
           surface_format << GEN6_SURFACE_DW0_FORMAT__SHIFT |
           GEN6_SURFACE_DW0_MIPLAYOUT_BELOW;

   if (surface_type == GEN6_SURFTYPE_CUBE && !is_rt) {
      dw[0] |= 1 << 9 |
               GEN6_SURFACE_DW0_CUBE_FACE_ENABLES__MASK;
   }

   if (is_rt)
      dw[0] |= GEN6_SURFACE_DW0_RENDER_CACHE_RW;

   dw[1] = 0;

   dw[2] = (height - 1) << GEN6_SURFACE_DW2_HEIGHT__SHIFT |
           (width - 1) << GEN6_SURFACE_DW2_WIDTH__SHIFT |
           lod << GEN6_SURFACE_DW2_MIP_COUNT_LOD__SHIFT;

   assert(img->tiling != GEN8_TILING_W);
   dw[3] = (depth - 1) << GEN6_SURFACE_DW3_DEPTH__SHIFT |
           (pitch - 1) << GEN6_SURFACE_DW3_PITCH__SHIFT |
           img->tiling;

   dw[4] = first_level << GEN6_SURFACE_DW4_MIN_LOD__SHIFT |
           first_layer << 17 |
           (num_layers - 1) << 8 |
           ((img->sample_count > 1) ? GEN6_SURFACE_DW4_MULTISAMPLECOUNT_4 :
                                      GEN6_SURFACE_DW4_MULTISAMPLECOUNT_1);

   dw[5] = 0;

   assert(img->align_j == 2 || img->align_j == 4);
   if (img->align_j == 4)
      dw[5] |= GEN6_SURFACE_DW5_VALIGN_4;
}

static void
view_init_null_gen7(const struct ilo_dev *dev,
                    unsigned width, unsigned height,
                    unsigned depth, unsigned level,
                    struct ilo_view_surface *surf)
{
   uint32_t *dw;

   ILO_DEV_ASSERT(dev, 7, 8);

   assert(width >= 1 && height >= 1 && depth >= 1);

   /*
    * From the Ivy Bridge PRM, volume 4 part 1, page 62:
    *
    *     "A null surface is used in instances where an actual surface is not
    *      bound. When a write message is generated to a null surface, no
    *      actual surface is written to. When a read message (including any
    *      sampling engine message) is generated to a null surface, the result
    *      is all zeros.  Note that a null surface type is allowed to be used
    *      with all messages, even if it is not specificially indicated as
    *      supported. All of the remaining fields in surface state are ignored
    *      for null surfaces, with the following exceptions:
    *
    *      * Width, Height, Depth, LOD, and Render Target View Extent fields
    *        must match the depth buffer's corresponding state for all render
    *        target surfaces, including null.
    *      * All sampling engine and data port messages support null surfaces
    *        with the above behavior, even if not mentioned as specifically
    *        supported, except for the following:
    *        * Data Port Media Block Read/Write messages.
    *      * The Surface Type of a surface used as a render target (accessed
    *        via the Data Port's Render Target Write message) must be the same
    *        as the Surface Type of all other render targets and of the depth
    *        buffer (defined in 3DSTATE_DEPTH_BUFFER), unless either the depth
    *        buffer or render targets are SURFTYPE_NULL."
    *
    * From the Ivy Bridge PRM, volume 4 part 1, page 65:
    *
    *     "If Surface Type is SURFTYPE_NULL, this field (Tiled Surface) must be
    *      true"
    */

   STATIC_ASSERT(Elements(surf->payload) >= 13);
   dw = surf->payload;

   dw[0] = GEN6_SURFTYPE_NULL << GEN7_SURFACE_DW0_TYPE__SHIFT |
           GEN6_FORMAT_B8G8R8A8_UNORM << GEN7_SURFACE_DW0_FORMAT__SHIFT;

   if (ilo_dev_gen(dev) >= ILO_GEN(8))
      dw[0] |= GEN6_TILING_X << GEN8_SURFACE_DW0_TILING__SHIFT;
   else
      dw[0] |= GEN6_TILING_X << GEN7_SURFACE_DW0_TILING__SHIFT;

   dw[1] = 0;

   dw[2] = GEN_SHIFT32(height - 1, GEN7_SURFACE_DW2_HEIGHT) |
           GEN_SHIFT32(width  - 1, GEN7_SURFACE_DW2_WIDTH);

   dw[3] = GEN_SHIFT32(depth - 1, GEN7_SURFACE_DW3_DEPTH);

   dw[4] = 0;
   dw[5] = level;

   dw[6] = 0;
   dw[7] = 0;

   if (ilo_dev_gen(dev) >= ILO_GEN(8))
      memset(&dw[8], 0, sizeof(*dw) * (13 - 8));
}

static void
view_init_for_buffer_gen7(const struct ilo_dev *dev,
                          const struct ilo_buffer *buf,
                          unsigned offset, unsigned size,
                          unsigned struct_size,
                          enum pipe_format elem_format,
                          bool is_rt, bool render_cache_rw,
                          struct ilo_view_surface *surf)
{
   const bool typed = (elem_format != PIPE_FORMAT_NONE);
   const bool structured = (!typed && struct_size > 1);
   const int elem_size = (typed) ?
      util_format_get_blocksize(elem_format) : 1;
   int width, height, depth, pitch;
   int surface_type, surface_format, num_entries;
   uint32_t *dw;

   ILO_DEV_ASSERT(dev, 7, 8);

   surface_type = (structured) ? GEN7_SURFTYPE_STRBUF : GEN6_SURFTYPE_BUFFER;

   surface_format = (typed) ?
      ilo_format_translate_color(dev, elem_format) : GEN6_FORMAT_RAW;

   num_entries = size / struct_size;
   /* see if there is enough space to fit another element */
   if (size % struct_size >= elem_size && !structured)
      num_entries++;

   /*
    * From the Ivy Bridge PRM, volume 4 part 1, page 67:
    *
    *     "For SURFTYPE_BUFFER render targets, this field (Surface Base
    *      Address) specifies the base address of first element of the
    *      surface. The surface is interpreted as a simple array of that
    *      single element type. The address must be naturally-aligned to the
    *      element size (e.g., a buffer containing R32G32B32A32_FLOAT elements
    *      must be 16-byte aligned)
    *
    *      For SURFTYPE_BUFFER non-rendertarget surfaces, this field specifies
    *      the base address of the first element of the surface, computed in
    *      software by adding the surface base address to the byte offset of
    *      the element in the buffer."
    */
   if (is_rt)
      assert(offset % elem_size == 0);

   /*
    * From the Ivy Bridge PRM, volume 4 part 1, page 68:
    *
    *     "For typed buffer and structured buffer surfaces, the number of
    *      entries in the buffer ranges from 1 to 2^27.  For raw buffer
    *      surfaces, the number of entries in the buffer is the number of
    *      bytes which can range from 1 to 2^30."
    */
   assert(num_entries >= 1 &&
          num_entries <= 1 << ((typed || structured) ? 27 : 30));

   /*
    * From the Ivy Bridge PRM, volume 4 part 1, page 69:
    *
    *     "For SURFTYPE_BUFFER: The low two bits of this field (Width) must be
    *      11 if the Surface Format is RAW (the size of the buffer must be a
    *      multiple of 4 bytes)."
    *
    * From the Ivy Bridge PRM, volume 4 part 1, page 70:
    *
    *     "For surfaces of type SURFTYPE_BUFFER and SURFTYPE_STRBUF, this
    *      field (Surface Pitch) indicates the size of the structure."
    *
    *     "For linear surfaces with Surface Type of SURFTYPE_STRBUF, the pitch
    *      must be a multiple of 4 bytes."
    */
   if (structured)
      assert(struct_size % 4 == 0);
   else if (!typed)
      assert(num_entries % 4 == 0);

   pitch = struct_size;

   pitch--;
   num_entries--;
   /* bits [6:0] */
   width  = (num_entries & 0x0000007f);
   /* bits [20:7] */
   height = (num_entries & 0x001fff80) >> 7;
   /* bits [30:21] */
   depth  = (num_entries & 0x7fe00000) >> 21;
   /* limit to [26:21] */
   if (typed || structured)
      depth &= 0x3f;

   STATIC_ASSERT(Elements(surf->payload) >= 13);
   dw = surf->payload;

   dw[0] = surface_type << GEN7_SURFACE_DW0_TYPE__SHIFT |
           surface_format << GEN7_SURFACE_DW0_FORMAT__SHIFT;
   if (render_cache_rw)
      dw[0] |= GEN7_SURFACE_DW0_RENDER_CACHE_RW;

   if (ilo_dev_gen(dev) >= ILO_GEN(8)) {
      dw[8] = offset;
      memset(&dw[9], 0, sizeof(*dw) * (13 - 9));
   } else {
      dw[1] = offset;
   }

   dw[2] = GEN_SHIFT32(height, GEN7_SURFACE_DW2_HEIGHT) |
           GEN_SHIFT32(width, GEN7_SURFACE_DW2_WIDTH);

   dw[3] = GEN_SHIFT32(depth, GEN7_SURFACE_DW3_DEPTH) |
           pitch;

   dw[4] = 0;
   dw[5] = 0;

   dw[6] = 0;
   dw[7] = 0;

   if (ilo_dev_gen(dev) >= ILO_GEN(7.5)) {
      dw[7] |= GEN_SHIFT32(GEN75_SCS_RED,   GEN75_SURFACE_DW7_SCS_R) |
               GEN_SHIFT32(GEN75_SCS_GREEN, GEN75_SURFACE_DW7_SCS_G) |
               GEN_SHIFT32(GEN75_SCS_BLUE,  GEN75_SURFACE_DW7_SCS_B) |
               GEN_SHIFT32(GEN75_SCS_ALPHA, GEN75_SURFACE_DW7_SCS_A);
   }
}

static void
view_init_for_image_gen7(const struct ilo_dev *dev,
                         const struct ilo_image *img,
                         enum pipe_texture_target target,
                         enum pipe_format format,
                         unsigned first_level,
                         unsigned num_levels,
                         unsigned first_layer,
                         unsigned num_layers,
                         bool is_rt,
                         struct ilo_view_surface *surf)
{
   int surface_type, surface_format;
   int width, height, depth, pitch, lod;
   uint32_t *dw;

   ILO_DEV_ASSERT(dev, 7, 8);

   surface_type = ilo_gpe_gen6_translate_texture(target);
   assert(surface_type != GEN6_SURFTYPE_BUFFER);

   if (format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT && img->separate_stencil)
      format = PIPE_FORMAT_Z32_FLOAT;

   if (is_rt)
      surface_format = ilo_format_translate_render(dev, format);
   else
      surface_format = ilo_format_translate_texture(dev, format);
   assert(surface_format >= 0);

   width = img->width0;
   height = img->height0;
   depth = (target == PIPE_TEXTURE_3D) ? img->depth0 : num_layers;
   pitch = img->bo_stride;

   if (surface_type == GEN6_SURFTYPE_CUBE) {
      /*
       * From the Ivy Bridge PRM, volume 4 part 1, page 70:
       *
       *     "For SURFTYPE_CUBE:For Sampling Engine Surfaces, the range of
       *      this field is [0,340], indicating the number of cube array
       *      elements (equal to the number of underlying 2D array elements
       *      divided by 6). For other surfaces, this field must be zero."
       *
       * When is_rt is true, we treat the texture as a 2D one to avoid the
       * restriction.
       */
      if (is_rt) {
         surface_type = GEN6_SURFTYPE_2D;
      }
      else {
         assert(num_layers % 6 == 0);
         depth = num_layers / 6;
      }
   }

   /* sanity check the size */
   assert(width >= 1 && height >= 1 && depth >= 1 && pitch >= 1);
   assert(first_layer < 2048 && num_layers <= 2048);
   switch (surface_type) {
   case GEN6_SURFTYPE_1D:
      assert(width <= 16384 && height == 1 && depth <= 2048);
      break;
   case GEN6_SURFTYPE_2D:
      assert(width <= 16384 && height <= 16384 && depth <= 2048);
      break;
   case GEN6_SURFTYPE_3D:
      assert(width <= 2048 && height <= 2048 && depth <= 2048);
      if (!is_rt)
         assert(first_layer == 0);
      break;
   case GEN6_SURFTYPE_CUBE:
      assert(width <= 16384 && height <= 16384 && depth <= 86);
      assert(width == height);
      if (is_rt)
         assert(first_layer == 0);
      break;
   default:
      assert(!"unexpected surface type");
      break;
   }

   if (is_rt) {
      assert(num_levels == 1);
      lod = first_level;
   }
   else {
      lod = num_levels - 1;
   }

   /*
    * From the Ivy Bridge PRM, volume 4 part 1, page 68:
    *
    *     "The Base Address for linear render target surfaces and surfaces
    *      accessed with the typed surface read/write data port messages must
    *      be element-size aligned, for non-YUV surface formats, or a multiple
    *      of 2 element-sizes for YUV surface formats.  Other linear surfaces
    *      have no alignment requirements (byte alignment is sufficient)."
    *
    * From the Ivy Bridge PRM, volume 4 part 1, page 70:
    *
    *     "For linear render target surfaces and surfaces accessed with the
    *      typed data port messages, the pitch must be a multiple of the
    *      element size for non-YUV surface formats. Pitch must be a multiple
    *      of 2 * element size for YUV surface formats. For linear surfaces
    *      with Surface Type of SURFTYPE_STRBUF, the pitch must be a multiple
    *      of 4 bytes.For other linear surfaces, the pitch can be any multiple
    *      of bytes."
    *
    * From the Ivy Bridge PRM, volume 4 part 1, page 74:
    *
    *     "For linear surfaces, this field (X Offset) must be zero."
    */
   if (img->tiling == GEN6_TILING_NONE) {
      if (is_rt) {
         const int elem_size = util_format_get_blocksize(format);
         assert(pitch % elem_size == 0);
      }
   }

   STATIC_ASSERT(Elements(surf->payload) >= 13);
   dw = surf->payload;

   dw[0] = surface_type << GEN7_SURFACE_DW0_TYPE__SHIFT |
           surface_format << GEN7_SURFACE_DW0_FORMAT__SHIFT;

   /*
    * From the Ivy Bridge PRM, volume 4 part 1, page 63:
    *
    *     "If this field (Surface Array) is enabled, the Surface Type must be
    *      SURFTYPE_1D, SURFTYPE_2D, or SURFTYPE_CUBE. If this field is
    *      disabled and Surface Type is SURFTYPE_1D, SURFTYPE_2D, or
    *      SURFTYPE_CUBE, the Depth field must be set to zero."
    *
    * For non-3D sampler surfaces, resinfo (the sampler message) always
    * returns zero for the number of layers when this field is not set.
    */
   if (surface_type != GEN6_SURFTYPE_3D) {
      switch (target) {
      case PIPE_TEXTURE_1D_ARRAY:
      case PIPE_TEXTURE_2D_ARRAY:
      case PIPE_TEXTURE_CUBE_ARRAY:
         dw[0] |= GEN7_SURFACE_DW0_IS_ARRAY;
         break;
      default:
         assert(depth == 1);
         break;
      }
   }

   if (ilo_dev_gen(dev) >= ILO_GEN(8)) {
      switch (img->align_j) {
      case 4:
         dw[0] |= GEN7_SURFACE_DW0_VALIGN_4;
         break;
      case 8:
         dw[0] |= GEN8_SURFACE_DW0_VALIGN_8;
         break;
      case 16:
         dw[0] |= GEN8_SURFACE_DW0_VALIGN_16;
         break;
      default:
         assert(!"unsupported valign");
         break;
      }

      switch (img->align_i) {
      case 4:
         dw[0] |= GEN8_SURFACE_DW0_HALIGN_4;
         break;
      case 8:
         dw[0] |= GEN8_SURFACE_DW0_HALIGN_8;
         break;
      case 16:
         dw[0] |= GEN8_SURFACE_DW0_HALIGN_16;
         break;
      default:
         assert(!"unsupported halign");
         break;
      }

      dw[0] |= img->tiling << GEN8_SURFACE_DW0_TILING__SHIFT;
   } else {
      assert(img->align_i == 4 || img->align_i == 8);
      assert(img->align_j == 2 || img->align_j == 4);

      if (img->align_j == 4)
         dw[0] |= GEN7_SURFACE_DW0_VALIGN_4;

      if (img->align_i == 8)
         dw[0] |= GEN7_SURFACE_DW0_HALIGN_8;

      assert(img->tiling != GEN8_TILING_W);
      dw[0] |= img->tiling << GEN7_SURFACE_DW0_TILING__SHIFT;

      if (img->walk == ILO_IMAGE_WALK_LOD)
         dw[0] |= GEN7_SURFACE_DW0_ARYSPC_LOD0;
      else
         dw[0] |= GEN7_SURFACE_DW0_ARYSPC_FULL;
   }

   if (is_rt)
      dw[0] |= GEN7_SURFACE_DW0_RENDER_CACHE_RW;

   if (surface_type == GEN6_SURFTYPE_CUBE && !is_rt)
      dw[0] |= GEN7_SURFACE_DW0_CUBE_FACE_ENABLES__MASK;

   if (ilo_dev_gen(dev) >= ILO_GEN(8)) {
      assert(img->walk_layer_height % 4 == 0);
      dw[1] = img->walk_layer_height / 4;
   } else {
      dw[1] = 0;
   }

   dw[2] = GEN_SHIFT32(height - 1, GEN7_SURFACE_DW2_HEIGHT) |
           GEN_SHIFT32(width - 1, GEN7_SURFACE_DW2_WIDTH);

   dw[3] = GEN_SHIFT32(depth - 1, GEN7_SURFACE_DW3_DEPTH) |
           (pitch - 1);

   dw[4] = first_layer << 18 |
           (num_layers - 1) << 7;

   /*
    * MSFMT_MSS means the samples are not interleaved and MSFMT_DEPTH_STENCIL
    * means the samples are interleaved.  The layouts are the same when the
    * number of samples is 1.
    */
   if (img->interleaved_samples && img->sample_count > 1) {
      assert(!is_rt);
      dw[4] |= GEN7_SURFACE_DW4_MSFMT_DEPTH_STENCIL;
   }
   else {
      dw[4] |= GEN7_SURFACE_DW4_MSFMT_MSS;
   }

   switch (img->sample_count) {
   case 0:
   case 1:
   default:
      dw[4] |= GEN7_SURFACE_DW4_MULTISAMPLECOUNT_1;
      break;
   case 2:
      dw[4] |= GEN8_SURFACE_DW4_MULTISAMPLECOUNT_2;
      break;
   case 4:
      dw[4] |= GEN7_SURFACE_DW4_MULTISAMPLECOUNT_4;
      break;
   case 8:
      dw[4] |= GEN7_SURFACE_DW4_MULTISAMPLECOUNT_8;
      break;
   case 16:
      dw[4] |= GEN8_SURFACE_DW4_MULTISAMPLECOUNT_16;
      break;
   }

   dw[5] = GEN_SHIFT32(first_level, GEN7_SURFACE_DW5_MIN_LOD) |
           lod;

   dw[6] = 0;
   dw[7] = 0;

   if (ilo_dev_gen(dev) >= ILO_GEN(7.5)) {
      dw[7] |= GEN_SHIFT32(GEN75_SCS_RED,   GEN75_SURFACE_DW7_SCS_R) |
               GEN_SHIFT32(GEN75_SCS_GREEN, GEN75_SURFACE_DW7_SCS_G) |
               GEN_SHIFT32(GEN75_SCS_BLUE,  GEN75_SURFACE_DW7_SCS_B) |
               GEN_SHIFT32(GEN75_SCS_ALPHA, GEN75_SURFACE_DW7_SCS_A);
   }

   if (ilo_dev_gen(dev) >= ILO_GEN(8))
      memset(&dw[8], 0, sizeof(*dw) * (13 - 8));
}

void
ilo_gpe_init_view_surface_null(const struct ilo_dev *dev,
                               unsigned width, unsigned height,
                               unsigned depth, unsigned level,
                               struct ilo_view_surface *surf)
{
   if (ilo_dev_gen(dev) >= ILO_GEN(7)) {
      view_init_null_gen7(dev,
            width, height, depth, level, surf);
   } else {
      view_init_null_gen6(dev,
            width, height, depth, level, surf);
   }

   surf->bo = NULL;
   surf->scanout = false;
}

void
ilo_gpe_init_view_surface_for_buffer(const struct ilo_dev *dev,
                                     const struct ilo_buffer *buf,
                                     unsigned offset, unsigned size,
                                     unsigned struct_size,
                                     enum pipe_format elem_format,
                                     bool is_rt, bool render_cache_rw,
                                     struct ilo_view_surface *surf)
{
   if (ilo_dev_gen(dev) >= ILO_GEN(7)) {
      view_init_for_buffer_gen7(dev, buf, offset, size,
            struct_size, elem_format, is_rt, render_cache_rw, surf);
   } else {
      view_init_for_buffer_gen6(dev, buf, offset, size,
            struct_size, elem_format, is_rt, render_cache_rw, surf);
   }

   /* do not increment reference count */
   surf->bo = buf->bo;
   surf->scanout = false;
}

void
ilo_gpe_init_view_surface_for_image(const struct ilo_dev *dev,
                                    const struct ilo_image *img,
                                    enum pipe_texture_target target,
                                    enum pipe_format format,
                                    unsigned first_level,
                                    unsigned num_levels,
                                    unsigned first_layer,
                                    unsigned num_layers,
                                    bool is_rt,
                                    struct ilo_view_surface *surf)
{
   if (ilo_dev_gen(dev) >= ILO_GEN(7)) {
      view_init_for_image_gen7(dev, img, target, format,
            first_level, num_levels, first_layer, num_layers,
            is_rt, surf);
   } else {
      view_init_for_image_gen6(dev, img, target, format,
            first_level, num_levels, first_layer, num_layers,
            is_rt, surf);
   }

   surf->scanout = img->scanout;
   /* do not increment reference count */
   surf->bo = img->bo;
}

static void
sampler_init_border_color_gen6(const struct ilo_dev *dev,
                               const union pipe_color_union *color,
                               uint32_t *dw, int num_dwords)
{
   float rgba[4] = {
      color->f[0], color->f[1], color->f[2], color->f[3],
   };

   ILO_DEV_ASSERT(dev, 6, 6);

   assert(num_dwords >= 12);

   /*
    * This state is not documented in the Sandy Bridge PRM, but in the
    * Ironlake PRM.  SNORM8 seems to be in DW11 instead of DW1.
    */

   /* IEEE_FP */
   dw[1] = fui(rgba[0]);
   dw[2] = fui(rgba[1]);
   dw[3] = fui(rgba[2]);
   dw[4] = fui(rgba[3]);

   /* FLOAT_16 */
   dw[5] = util_float_to_half(rgba[0]) |
           util_float_to_half(rgba[1]) << 16;
   dw[6] = util_float_to_half(rgba[2]) |
           util_float_to_half(rgba[3]) << 16;

   /* clamp to [-1.0f, 1.0f] */
   rgba[0] = CLAMP(rgba[0], -1.0f, 1.0f);
   rgba[1] = CLAMP(rgba[1], -1.0f, 1.0f);
   rgba[2] = CLAMP(rgba[2], -1.0f, 1.0f);
   rgba[3] = CLAMP(rgba[3], -1.0f, 1.0f);

   /* SNORM16 */
   dw[9] =  (int16_t) util_iround(rgba[0] * 32767.0f) |
            (int16_t) util_iround(rgba[1] * 32767.0f) << 16;
   dw[10] = (int16_t) util_iround(rgba[2] * 32767.0f) |
            (int16_t) util_iround(rgba[3] * 32767.0f) << 16;

   /* SNORM8 */
   dw[11] = (int8_t) util_iround(rgba[0] * 127.0f) |
            (int8_t) util_iround(rgba[1] * 127.0f) << 8 |
            (int8_t) util_iround(rgba[2] * 127.0f) << 16 |
            (int8_t) util_iround(rgba[3] * 127.0f) << 24;

   /* clamp to [0.0f, 1.0f] */
   rgba[0] = CLAMP(rgba[0], 0.0f, 1.0f);
   rgba[1] = CLAMP(rgba[1], 0.0f, 1.0f);
   rgba[2] = CLAMP(rgba[2], 0.0f, 1.0f);
   rgba[3] = CLAMP(rgba[3], 0.0f, 1.0f);

   /* UNORM8 */
   dw[0] = (uint8_t) util_iround(rgba[0] * 255.0f) |
           (uint8_t) util_iround(rgba[1] * 255.0f) << 8 |
           (uint8_t) util_iround(rgba[2] * 255.0f) << 16 |
           (uint8_t) util_iround(rgba[3] * 255.0f) << 24;

   /* UNORM16 */
   dw[7] = (uint16_t) util_iround(rgba[0] * 65535.0f) |
           (uint16_t) util_iround(rgba[1] * 65535.0f) << 16;
   dw[8] = (uint16_t) util_iround(rgba[2] * 65535.0f) |
           (uint16_t) util_iround(rgba[3] * 65535.0f) << 16;
}

/**
 * Translate a pipe texture mipfilter to the matching hardware mipfilter.
 */
static int
gen6_translate_tex_mipfilter(unsigned filter)
{
   switch (filter) {
   case PIPE_TEX_MIPFILTER_NEAREST: return GEN6_MIPFILTER_NEAREST;
   case PIPE_TEX_MIPFILTER_LINEAR:  return GEN6_MIPFILTER_LINEAR;
   case PIPE_TEX_MIPFILTER_NONE:    return GEN6_MIPFILTER_NONE;
   default:
      assert(!"unknown mipfilter");
      return GEN6_MIPFILTER_NONE;
   }
}

/**
 * Translate a pipe texture filter to the matching hardware mapfilter.
 */
static int
gen6_translate_tex_filter(unsigned filter)
{
   switch (filter) {
   case PIPE_TEX_FILTER_NEAREST: return GEN6_MAPFILTER_NEAREST;
   case PIPE_TEX_FILTER_LINEAR:  return GEN6_MAPFILTER_LINEAR;
   default:
      assert(!"unknown sampler filter");
      return GEN6_MAPFILTER_NEAREST;
   }
}

/**
 * Translate a pipe texture coordinate wrapping mode to the matching hardware
 * wrapping mode.
 */
static int
gen6_translate_tex_wrap(unsigned wrap)
{
   switch (wrap) {
   case PIPE_TEX_WRAP_CLAMP:              return GEN8_TEXCOORDMODE_HALF_BORDER;
   case PIPE_TEX_WRAP_REPEAT:             return GEN6_TEXCOORDMODE_WRAP;
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:      return GEN6_TEXCOORDMODE_CLAMP;
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:    return GEN6_TEXCOORDMODE_CLAMP_BORDER;
   case PIPE_TEX_WRAP_MIRROR_REPEAT:      return GEN6_TEXCOORDMODE_MIRROR;
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
   default:
      assert(!"unknown sampler wrap mode");
      return GEN6_TEXCOORDMODE_WRAP;
   }
}

/**
 * Translate a pipe shadow compare function to the matching hardware shadow
 * function.
 */
static int
gen6_translate_shadow_func(unsigned func)
{
   /*
    * For PIPE_FUNC_x, the reference value is on the left-hand side of the
    * comparison, and 1.0 is returned when the comparison is true.
    *
    * For GEN6_COMPAREFUNCTION_x, the reference value is on the right-hand side of
    * the comparison, and 0.0 is returned when the comparison is true.
    */
   switch (func) {
   case PIPE_FUNC_NEVER:      return GEN6_COMPAREFUNCTION_ALWAYS;
   case PIPE_FUNC_LESS:       return GEN6_COMPAREFUNCTION_LEQUAL;
   case PIPE_FUNC_EQUAL:      return GEN6_COMPAREFUNCTION_NOTEQUAL;
   case PIPE_FUNC_LEQUAL:     return GEN6_COMPAREFUNCTION_LESS;
   case PIPE_FUNC_GREATER:    return GEN6_COMPAREFUNCTION_GEQUAL;
   case PIPE_FUNC_NOTEQUAL:   return GEN6_COMPAREFUNCTION_EQUAL;
   case PIPE_FUNC_GEQUAL:     return GEN6_COMPAREFUNCTION_GREATER;
   case PIPE_FUNC_ALWAYS:     return GEN6_COMPAREFUNCTION_NEVER;
   default:
      assert(!"unknown shadow compare function");
      return GEN6_COMPAREFUNCTION_NEVER;
   }
}

void
ilo_gpe_init_sampler_cso(const struct ilo_dev *dev,
                         const struct pipe_sampler_state *state,
                         struct ilo_sampler_cso *sampler)
{
   int mip_filter, min_filter, mag_filter, max_aniso;
   int lod_bias, max_lod, min_lod;
   int wrap_s, wrap_t, wrap_r, wrap_cube;
   uint32_t dw0, dw1, dw3;

   ILO_DEV_ASSERT(dev, 6, 8);

   memset(sampler, 0, sizeof(*sampler));

   mip_filter = gen6_translate_tex_mipfilter(state->min_mip_filter);
   min_filter = gen6_translate_tex_filter(state->min_img_filter);
   mag_filter = gen6_translate_tex_filter(state->mag_img_filter);

   sampler->anisotropic = state->max_anisotropy;

   if (state->max_anisotropy >= 2 && state->max_anisotropy <= 16)
      max_aniso = state->max_anisotropy / 2 - 1;
   else if (state->max_anisotropy > 16)
      max_aniso = GEN6_ANISORATIO_16;
   else
      max_aniso = GEN6_ANISORATIO_2;

   /*
    *
    * Here is how the hardware calculate per-pixel LOD, from my reading of the
    * PRMs:
    *
    *  1) LOD is set to log2(ratio of texels to pixels) if not specified in
    *     other ways.  The number of texels is measured using level
    *     SurfMinLod.
    *  2) Bias is added to LOD.
    *  3) LOD is clamped to [MinLod, MaxLod], and the clamped value is
    *     compared with Base to determine whether magnification or
    *     minification is needed.  (if preclamp is disabled, LOD is compared
    *     with Base before clamping)
    *  4) If magnification is needed, or no mipmapping is requested, LOD is
    *     set to floor(MinLod).
    *  5) LOD is clamped to [0, MIPCnt], and SurfMinLod is added to LOD.
    *
    * With Gallium interface, Base is always zero and
    * pipe_sampler_view::u.tex.first_level specifies SurfMinLod.
    */
   if (ilo_dev_gen(dev) >= ILO_GEN(7)) {
      const float scale = 256.0f;

      /* [-16.0, 16.0) in S4.8 */
      lod_bias = (int)
         (CLAMP(state->lod_bias, -16.0f, 15.9f) * scale);
      lod_bias &= 0x1fff;

      /* [0.0, 14.0] in U4.8 */
      max_lod = (int) (CLAMP(state->max_lod, 0.0f, 14.0f) * scale);
      min_lod = (int) (CLAMP(state->min_lod, 0.0f, 14.0f) * scale);
   }
   else {
      const float scale = 64.0f;

      /* [-16.0, 16.0) in S4.6 */
      lod_bias = (int)
         (CLAMP(state->lod_bias, -16.0f, 15.9f) * scale);
      lod_bias &= 0x7ff;

      /* [0.0, 13.0] in U4.6 */
      max_lod = (int) (CLAMP(state->max_lod, 0.0f, 13.0f) * scale);
      min_lod = (int) (CLAMP(state->min_lod, 0.0f, 13.0f) * scale);
   }

   /*
    * We want LOD to be clamped to determine magnification/minification, and
    * get set to zero when it is magnification or when mipmapping is disabled.
    * The hardware would set LOD to floor(MinLod) and that is a problem when
    * MinLod is greater than or equal to 1.0f.
    *
    * With Base being zero, it is always minification when MinLod is non-zero.
    * To achieve our goal, we just need to set MinLod to zero and set
    * MagFilter to MinFilter when mipmapping is disabled.
    */
   if (state->min_mip_filter == PIPE_TEX_MIPFILTER_NONE && min_lod) {
      min_lod = 0;
      mag_filter = min_filter;
   }

   /* determine wrap s/t/r */
   wrap_s = gen6_translate_tex_wrap(state->wrap_s);
   wrap_t = gen6_translate_tex_wrap(state->wrap_t);
   wrap_r = gen6_translate_tex_wrap(state->wrap_r);
   if (ilo_dev_gen(dev) < ILO_GEN(8)) {
      /*
       * For nearest filtering, PIPE_TEX_WRAP_CLAMP means
       * PIPE_TEX_WRAP_CLAMP_TO_EDGE;  for linear filtering,
       * PIPE_TEX_WRAP_CLAMP means PIPE_TEX_WRAP_CLAMP_TO_BORDER while
       * additionally clamping the texture coordinates to [0.0, 1.0].
       *
       * PIPE_TEX_WRAP_CLAMP is not supported natively until Gen8.  The
       * clamping has to be taken care of in the shaders.  There are two
       * filters here, but let the minification one has a say.
       */
      const bool clamp_is_to_edge =
         (state->min_img_filter == PIPE_TEX_FILTER_NEAREST);

      if (clamp_is_to_edge) {
         if (wrap_s == GEN8_TEXCOORDMODE_HALF_BORDER)
            wrap_s = GEN6_TEXCOORDMODE_CLAMP;
         if (wrap_t == GEN8_TEXCOORDMODE_HALF_BORDER)
            wrap_t = GEN6_TEXCOORDMODE_CLAMP;
         if (wrap_r == GEN8_TEXCOORDMODE_HALF_BORDER)
            wrap_r = GEN6_TEXCOORDMODE_CLAMP;
      } else {
         if (wrap_s == GEN8_TEXCOORDMODE_HALF_BORDER) {
            wrap_s = GEN6_TEXCOORDMODE_CLAMP_BORDER;
            sampler->saturate_s = true;
         }
         if (wrap_t == GEN8_TEXCOORDMODE_HALF_BORDER) {
            wrap_t = GEN6_TEXCOORDMODE_CLAMP_BORDER;
            sampler->saturate_t = true;
         }
         if (wrap_r == GEN8_TEXCOORDMODE_HALF_BORDER) {
            wrap_r = GEN6_TEXCOORDMODE_CLAMP_BORDER;
            sampler->saturate_r = true;
         }
      }
   }

   /*
    * From the Sandy Bridge PRM, volume 4 part 1, page 107:
    *
    *     "When using cube map texture coordinates, only TEXCOORDMODE_CLAMP
    *      and TEXCOORDMODE_CUBE settings are valid, and each TC component
    *      must have the same Address Control mode."
    *
    * From the Ivy Bridge PRM, volume 4 part 1, page 96:
    *
    *     "This field (Cube Surface Control Mode) must be set to
    *      CUBECTRLMODE_PROGRAMMED"
    *
    * Therefore, we cannot use "Cube Surface Control Mode" for semless cube
    * map filtering.
    */
   if (state->seamless_cube_map &&
       (state->min_img_filter != PIPE_TEX_FILTER_NEAREST ||
        state->mag_img_filter != PIPE_TEX_FILTER_NEAREST)) {
      wrap_cube = GEN6_TEXCOORDMODE_CUBE;
   }
   else {
      wrap_cube = GEN6_TEXCOORDMODE_CLAMP;
   }

   if (!state->normalized_coords) {
      /*
       * From the Ivy Bridge PRM, volume 4 part 1, page 98:
       *
       *     "The following state must be set as indicated if this field
       *      (Non-normalized Coordinate Enable) is enabled:
       *
       *      - TCX/Y/Z Address Control Mode must be TEXCOORDMODE_CLAMP,
       *        TEXCOORDMODE_HALF_BORDER, or TEXCOORDMODE_CLAMP_BORDER.
       *      - Surface Type must be SURFTYPE_2D or SURFTYPE_3D.
       *      - Mag Mode Filter must be MAPFILTER_NEAREST or
       *        MAPFILTER_LINEAR.
       *      - Min Mode Filter must be MAPFILTER_NEAREST or
       *        MAPFILTER_LINEAR.
       *      - Mip Mode Filter must be MIPFILTER_NONE.
       *      - Min LOD must be 0.
       *      - Max LOD must be 0.
       *      - MIP Count must be 0.
       *      - Surface Min LOD must be 0.
       *      - Texture LOD Bias must be 0."
       */
      assert(wrap_s == GEN6_TEXCOORDMODE_CLAMP ||
             wrap_s == GEN6_TEXCOORDMODE_CLAMP_BORDER);
      assert(wrap_t == GEN6_TEXCOORDMODE_CLAMP ||
             wrap_t == GEN6_TEXCOORDMODE_CLAMP_BORDER);
      assert(wrap_r == GEN6_TEXCOORDMODE_CLAMP ||
             wrap_r == GEN6_TEXCOORDMODE_CLAMP_BORDER);

      assert(mag_filter == GEN6_MAPFILTER_NEAREST ||
             mag_filter == GEN6_MAPFILTER_LINEAR);
      assert(min_filter == GEN6_MAPFILTER_NEAREST ||
             min_filter == GEN6_MAPFILTER_LINEAR);

      /* work around a bug in util_blitter */
      mip_filter = GEN6_MIPFILTER_NONE;

      assert(mip_filter == GEN6_MIPFILTER_NONE);
   }

   if (ilo_dev_gen(dev) >= ILO_GEN(7)) {
      dw0 = 1 << 28 |
            mip_filter << 20 |
            lod_bias << 1;

      sampler->dw_filter = mag_filter << 17 |
                           min_filter << 14;

      sampler->dw_filter_aniso = GEN6_MAPFILTER_ANISOTROPIC << 17 |
                                 GEN6_MAPFILTER_ANISOTROPIC << 14 |
                                 1;

      dw1 = min_lod << 20 |
            max_lod << 8;

      if (state->compare_mode != PIPE_TEX_COMPARE_NONE)
         dw1 |= gen6_translate_shadow_func(state->compare_func) << 1;

      dw3 = max_aniso << 19;

      /* round the coordinates for linear filtering */
      if (min_filter != GEN6_MAPFILTER_NEAREST) {
         dw3 |= (GEN6_SAMPLER_DW3_U_MIN_ROUND |
                 GEN6_SAMPLER_DW3_V_MIN_ROUND |
                 GEN6_SAMPLER_DW3_R_MIN_ROUND);
      }
      if (mag_filter != GEN6_MAPFILTER_NEAREST) {
         dw3 |= (GEN6_SAMPLER_DW3_U_MAG_ROUND |
                 GEN6_SAMPLER_DW3_V_MAG_ROUND |
                 GEN6_SAMPLER_DW3_R_MAG_ROUND);
      }

      if (!state->normalized_coords)
         dw3 |= 1 << 10;

      sampler->dw_wrap = wrap_s << 6 |
                         wrap_t << 3 |
                         wrap_r;

      /*
       * As noted in the classic i965 driver, the HW may still reference
       * wrap_t and wrap_r for 1D textures.  We need to set them to a safe
       * mode
       */
      sampler->dw_wrap_1d = wrap_s << 6 |
                            GEN6_TEXCOORDMODE_WRAP << 3 |
                            GEN6_TEXCOORDMODE_WRAP;

      sampler->dw_wrap_cube = wrap_cube << 6 |
                              wrap_cube << 3 |
                              wrap_cube;

      STATIC_ASSERT(Elements(sampler->payload) >= 7);

      sampler->payload[0] = dw0;
      sampler->payload[1] = dw1;
      sampler->payload[2] = dw3;

      memcpy(&sampler->payload[3],
            state->border_color.ui, sizeof(state->border_color.ui));
   }
   else {
      dw0 = 1 << 28 |
            mip_filter << 20 |
            lod_bias << 3;

      if (state->compare_mode != PIPE_TEX_COMPARE_NONE)
         dw0 |= gen6_translate_shadow_func(state->compare_func);

      sampler->dw_filter = (min_filter != mag_filter) << 27 |
                           mag_filter << 17 |
                           min_filter << 14;

      sampler->dw_filter_aniso = GEN6_MAPFILTER_ANISOTROPIC << 17 |
                                 GEN6_MAPFILTER_ANISOTROPIC << 14;

      dw1 = min_lod << 22 |
            max_lod << 12;

      sampler->dw_wrap = wrap_s << 6 |
                         wrap_t << 3 |
                         wrap_r;

      sampler->dw_wrap_1d = wrap_s << 6 |
                            GEN6_TEXCOORDMODE_WRAP << 3 |
                            GEN6_TEXCOORDMODE_WRAP;

      sampler->dw_wrap_cube = wrap_cube << 6 |
                              wrap_cube << 3 |
                              wrap_cube;

      dw3 = max_aniso << 19;

      /* round the coordinates for linear filtering */
      if (min_filter != GEN6_MAPFILTER_NEAREST) {
         dw3 |= (GEN6_SAMPLER_DW3_U_MIN_ROUND |
                 GEN6_SAMPLER_DW3_V_MIN_ROUND |
                 GEN6_SAMPLER_DW3_R_MIN_ROUND);
      }
      if (mag_filter != GEN6_MAPFILTER_NEAREST) {
         dw3 |= (GEN6_SAMPLER_DW3_U_MAG_ROUND |
                 GEN6_SAMPLER_DW3_V_MAG_ROUND |
                 GEN6_SAMPLER_DW3_R_MAG_ROUND);
      }

      if (!state->normalized_coords)
         dw3 |= 1;

      STATIC_ASSERT(Elements(sampler->payload) >= 15);

      sampler->payload[0] = dw0;
      sampler->payload[1] = dw1;
      sampler->payload[2] = dw3;

      sampler_init_border_color_gen6(dev,
            &state->border_color, &sampler->payload[3], 12);
   }
}
