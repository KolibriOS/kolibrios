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

#include "ilo_format.h"
#include "ilo_image.h"
#include "ilo_state_3d.h"
#include "../ilo_shader.h"

static void
rasterizer_init_clip(const struct ilo_dev *dev,
                     const struct pipe_rasterizer_state *state,
                     struct ilo_rasterizer_clip *clip)
{
   uint32_t dw1, dw2, dw3;

   ILO_DEV_ASSERT(dev, 6, 8);

   dw1 = GEN6_CLIP_DW1_STATISTICS;

   if (ilo_dev_gen(dev) >= ILO_GEN(7)) {
      /*
       * From the Ivy Bridge PRM, volume 2 part 1, page 219:
       *
       *     "Workaround : Due to Hardware issue "EarlyCull" needs to be
       *      enabled only for the cases where the incoming primitive topology
       *      into the clipper guaranteed to be Trilist."
       *
       * What does this mean?
       */
      dw1 |= 0 << 19 |
             GEN7_CLIP_DW1_EARLY_CULL_ENABLE;

      if (ilo_dev_gen(dev) < ILO_GEN(8)) {
         if (state->front_ccw)
            dw1 |= GEN7_CLIP_DW1_FRONTWINDING_CCW;

         switch (state->cull_face) {
         case PIPE_FACE_NONE:
            dw1 |= GEN7_CLIP_DW1_CULLMODE_NONE;
            break;
         case PIPE_FACE_FRONT:
            dw1 |= GEN7_CLIP_DW1_CULLMODE_FRONT;
            break;
         case PIPE_FACE_BACK:
            dw1 |= GEN7_CLIP_DW1_CULLMODE_BACK;
            break;
         case PIPE_FACE_FRONT_AND_BACK:
            dw1 |= GEN7_CLIP_DW1_CULLMODE_BOTH;
            break;
         }
      }
   }

   dw2 = GEN6_CLIP_DW2_CLIP_ENABLE |
         GEN6_CLIP_DW2_XY_TEST_ENABLE |
         state->clip_plane_enable << GEN6_CLIP_DW2_UCP_CLIP_ENABLES__SHIFT |
         GEN6_CLIP_DW2_CLIPMODE_NORMAL;

   if (state->clip_halfz)
      dw2 |= GEN6_CLIP_DW2_APIMODE_D3D;
   else
      dw2 |= GEN6_CLIP_DW2_APIMODE_OGL;

   if (ilo_dev_gen(dev) < ILO_GEN(8) && state->depth_clip)
      dw2 |= GEN6_CLIP_DW2_Z_TEST_ENABLE;

   if (state->flatshade_first) {
      dw2 |= 0 << GEN6_CLIP_DW2_TRI_PROVOKE__SHIFT |
             0 << GEN6_CLIP_DW2_LINE_PROVOKE__SHIFT |
             1 << GEN6_CLIP_DW2_TRIFAN_PROVOKE__SHIFT;
   }
   else {
      dw2 |= 2 << GEN6_CLIP_DW2_TRI_PROVOKE__SHIFT |
             1 << GEN6_CLIP_DW2_LINE_PROVOKE__SHIFT |
             2 << GEN6_CLIP_DW2_TRIFAN_PROVOKE__SHIFT;
   }

   dw3 = 0x1 << GEN6_CLIP_DW3_MIN_POINT_WIDTH__SHIFT |
         0x7ff << GEN6_CLIP_DW3_MAX_POINT_WIDTH__SHIFT;

   clip->payload[0] = dw1;
   clip->payload[1] = dw2;
   clip->payload[2] = dw3;

   clip->can_enable_guardband = true;

   /*
    * There are several reasons that guard band test should be disabled
    *
    *  - GL wide points (to avoid partially visibie object)
    *  - GL wide or AA lines (to avoid partially visibie object)
    */
   if (state->point_size_per_vertex || state->point_size > 1.0f)
      clip->can_enable_guardband = false;
   if (state->line_smooth || state->line_width > 1.0f)
      clip->can_enable_guardband = false;
}

static void
rasterizer_init_sf_depth_offset_gen6(const struct ilo_dev *dev,
                                     const struct pipe_rasterizer_state *state,
                                     struct ilo_rasterizer_sf *sf)
{
   ILO_DEV_ASSERT(dev, 6, 8);

   /*
    * Scale the constant term.  The minimum representable value used by the HW
    * is not large enouch to be the minimum resolvable difference.
    */
   sf->dw_depth_offset_const = fui(state->offset_units * 2.0f);
   sf->dw_depth_offset_scale = fui(state->offset_scale);
   sf->dw_depth_offset_clamp = fui(state->offset_clamp);
}

static void
rasterizer_init_sf_gen6(const struct ilo_dev *dev,
                        const struct pipe_rasterizer_state *state,
                        struct ilo_rasterizer_sf *sf)
{
   int line_width, point_width;
   uint32_t dw1, dw2, dw3;

   ILO_DEV_ASSERT(dev, 6, 7.5);

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 248:
    *
    *     "This bit (Statistics Enable) should be set whenever clipping is
    *      enabled and the Statistics Enable bit is set in CLIP_STATE. It
    *      should be cleared if clipping is disabled or Statistics Enable in
    *      CLIP_STATE is clear."
    */
   dw1 = GEN7_SF_DW1_STATISTICS |
         GEN7_SF_DW1_VIEWPORT_ENABLE;

   /* XXX GEN6 path seems to work fine for GEN7 */
   if (false && ilo_dev_gen(dev) >= ILO_GEN(7)) {
      /*
       * From the Ivy Bridge PRM, volume 2 part 1, page 258:
       *
       *     "This bit (Legacy Global Depth Bias Enable, Global Depth Offset
       *      Enable Solid , Global Depth Offset Enable Wireframe, and Global
       *      Depth Offset Enable Point) should be set whenever non zero depth
       *      bias (Slope, Bias) values are used. Setting this bit may have
       *      some degradation of performance for some workloads."
       */
      if (state->offset_tri || state->offset_line || state->offset_point) {
         /* XXX need to scale offset_const according to the depth format */
         dw1 |= GEN7_SF_DW1_LEGACY_DEPTH_OFFSET;

         dw1 |= GEN7_SF_DW1_DEPTH_OFFSET_SOLID |
                GEN7_SF_DW1_DEPTH_OFFSET_WIREFRAME |
                GEN7_SF_DW1_DEPTH_OFFSET_POINT;
      }
   } else {
      if (state->offset_tri)
         dw1 |= GEN7_SF_DW1_DEPTH_OFFSET_SOLID;
      if (state->offset_line)
         dw1 |= GEN7_SF_DW1_DEPTH_OFFSET_WIREFRAME;
      if (state->offset_point)
         dw1 |= GEN7_SF_DW1_DEPTH_OFFSET_POINT;
   }

   switch (state->fill_front) {
   case PIPE_POLYGON_MODE_FILL:
      dw1 |= GEN7_SF_DW1_FRONTFACE_SOLID;
      break;
   case PIPE_POLYGON_MODE_LINE:
      dw1 |= GEN7_SF_DW1_FRONTFACE_WIREFRAME;
      break;
   case PIPE_POLYGON_MODE_POINT:
      dw1 |= GEN7_SF_DW1_FRONTFACE_POINT;
      break;
   }

   switch (state->fill_back) {
   case PIPE_POLYGON_MODE_FILL:
      dw1 |= GEN7_SF_DW1_BACKFACE_SOLID;
      break;
   case PIPE_POLYGON_MODE_LINE:
      dw1 |= GEN7_SF_DW1_BACKFACE_WIREFRAME;
      break;
   case PIPE_POLYGON_MODE_POINT:
      dw1 |= GEN7_SF_DW1_BACKFACE_POINT;
      break;
   }

   if (state->front_ccw)
      dw1 |= GEN7_SF_DW1_FRONTWINDING_CCW;

   dw2 = 0;

   if (state->line_smooth) {
      /*
       * From the Sandy Bridge PRM, volume 2 part 1, page 251:
       *
       *     "This field (Anti-aliasing Enable) must be disabled if any of the
       *      render targets have integer (UINT or SINT) surface format."
       *
       * From the Sandy Bridge PRM, volume 2 part 1, page 317:
       *
       *     "This field (Hierarchical Depth Buffer Enable) must be disabled
       *      if Anti-aliasing Enable in 3DSTATE_SF is enabled.
       *
       * TODO We do not check those yet.
       */
      dw2 |= GEN7_SF_DW2_AA_LINE_ENABLE |
             GEN7_SF_DW2_AA_LINE_CAP_1_0;
   }

   switch (state->cull_face) {
   case PIPE_FACE_NONE:
      dw2 |= GEN7_SF_DW2_CULLMODE_NONE;
      break;
   case PIPE_FACE_FRONT:
      dw2 |= GEN7_SF_DW2_CULLMODE_FRONT;
      break;
   case PIPE_FACE_BACK:
      dw2 |= GEN7_SF_DW2_CULLMODE_BACK;
      break;
   case PIPE_FACE_FRONT_AND_BACK:
      dw2 |= GEN7_SF_DW2_CULLMODE_BOTH;
      break;
   }

   /*
    * Smooth lines should intersect ceil(line_width) or (ceil(line_width) + 1)
    * pixels in the minor direction.  We have to make the lines slightly
    * thicker, 0.5 pixel on both sides, so that they intersect that many
    * pixels are considered into the lines.
    *
    * Line width is in U3.7.
    */
   line_width = (int)
      ((state->line_width + (float) state->line_smooth) * 128.0f + 0.5f);
   line_width = CLAMP(line_width, 0, 1023);

   /* use GIQ rules */
   if (line_width == 128 && !state->line_smooth)
      line_width = 0;

   dw2 |= line_width << GEN7_SF_DW2_LINE_WIDTH__SHIFT;

   if (ilo_dev_gen(dev) == ILO_GEN(7.5) && state->line_stipple_enable)
      dw2 |= GEN75_SF_DW2_LINE_STIPPLE_ENABLE;

   if (state->scissor)
      dw2 |= GEN7_SF_DW2_SCISSOR_ENABLE;

   dw3 = GEN7_SF_DW3_TRUE_AA_LINE_DISTANCE |
         GEN7_SF_DW3_SUBPIXEL_8BITS;

   if (state->line_last_pixel)
      dw3 |= GEN7_SF_DW3_LINE_LAST_PIXEL_ENABLE;

   if (state->flatshade_first) {
      dw3 |= 0 << GEN7_SF_DW3_TRI_PROVOKE__SHIFT |
             0 << GEN7_SF_DW3_LINE_PROVOKE__SHIFT |
             1 << GEN7_SF_DW3_TRIFAN_PROVOKE__SHIFT;
   } else {
      dw3 |= 2 << GEN7_SF_DW3_TRI_PROVOKE__SHIFT |
             1 << GEN7_SF_DW3_LINE_PROVOKE__SHIFT |
             2 << GEN7_SF_DW3_TRIFAN_PROVOKE__SHIFT;
   }

   if (!state->point_size_per_vertex)
      dw3 |= GEN7_SF_DW3_USE_POINT_WIDTH;

   /* in U8.3 */
   point_width = (int) (state->point_size * 8.0f + 0.5f);
   point_width = CLAMP(point_width, 1, 2047);

   dw3 |= point_width;

   STATIC_ASSERT(Elements(sf->payload) >= 3);
   sf->payload[0] = dw1;
   sf->payload[1] = dw2;
   sf->payload[2] = dw3;

   if (state->multisample) {
      sf->dw_msaa = GEN7_SF_DW2_MSRASTMODE_ON_PATTERN;

      /*
       * From the Sandy Bridge PRM, volume 2 part 1, page 251:
       *
       *     "Software must not program a value of 0.0 when running in
       *      MSRASTMODE_ON_xxx modes - zero-width lines are not available
       *      when multisampling rasterization is enabled."
       */
      if (!line_width) {
         line_width = 128; /* 1.0f */

         sf->dw_msaa |= line_width << GEN7_SF_DW2_LINE_WIDTH__SHIFT;
      }
   } else {
      sf->dw_msaa = 0;
   }

   rasterizer_init_sf_depth_offset_gen6(dev, state, sf);
   /* 3DSTATE_RASTER is Gen8+ only */
   sf->dw_raster = 0;
}

static uint32_t
rasterizer_get_sf_raster_gen8(const struct ilo_dev *dev,
                              const struct pipe_rasterizer_state *state)
{
   uint32_t dw = 0;

   ILO_DEV_ASSERT(dev, 8, 8);

   if (state->front_ccw)
      dw |= GEN8_RASTER_DW1_FRONTWINDING_CCW;

   switch (state->cull_face) {
   case PIPE_FACE_NONE:
      dw |= GEN8_RASTER_DW1_CULLMODE_NONE;
      break;
   case PIPE_FACE_FRONT:
      dw |= GEN8_RASTER_DW1_CULLMODE_FRONT;
      break;
   case PIPE_FACE_BACK:
      dw |= GEN8_RASTER_DW1_CULLMODE_BACK;
      break;
   case PIPE_FACE_FRONT_AND_BACK:
      dw |= GEN8_RASTER_DW1_CULLMODE_BOTH;
      break;
   }

   if (state->point_smooth)
      dw |= GEN8_RASTER_DW1_SMOOTH_POINT_ENABLE;

   if (state->multisample)
      dw |= GEN8_RASTER_DW1_API_MULTISAMPLE_ENABLE;

   if (state->offset_tri)
      dw|= GEN8_RASTER_DW1_DEPTH_OFFSET_SOLID;
   if (state->offset_line)
      dw|= GEN8_RASTER_DW1_DEPTH_OFFSET_WIREFRAME;
   if (state->offset_point)
      dw|= GEN8_RASTER_DW1_DEPTH_OFFSET_POINT;

   switch (state->fill_front) {
   case PIPE_POLYGON_MODE_FILL:
      dw |= GEN8_RASTER_DW1_FRONTFACE_SOLID;
      break;
   case PIPE_POLYGON_MODE_LINE:
      dw |= GEN8_RASTER_DW1_FRONTFACE_WIREFRAME;
      break;
   case PIPE_POLYGON_MODE_POINT:
      dw |= GEN8_RASTER_DW1_FRONTFACE_POINT;
      break;
   }

   switch (state->fill_back) {
   case PIPE_POLYGON_MODE_FILL:
      dw |= GEN8_RASTER_DW1_BACKFACE_SOLID;
      break;
   case PIPE_POLYGON_MODE_LINE:
      dw |= GEN8_RASTER_DW1_BACKFACE_WIREFRAME;
      break;
   case PIPE_POLYGON_MODE_POINT:
      dw |= GEN8_RASTER_DW1_BACKFACE_POINT;
      break;
   }

   if (state->line_smooth)
      dw |= GEN8_RASTER_DW1_AA_LINE_ENABLE;

   if (state->scissor)
      dw |= GEN8_RASTER_DW1_SCISSOR_ENABLE;

   if (state->depth_clip)
      dw |= GEN8_RASTER_DW1_Z_TEST_ENABLE;

   return dw;
}

static void
rasterizer_init_sf_gen8(const struct ilo_dev *dev,
                        const struct pipe_rasterizer_state *state,
                        struct ilo_rasterizer_sf *sf)
{
   int line_width, point_width;
   uint32_t dw1, dw2, dw3;

   ILO_DEV_ASSERT(dev, 8, 8);

   /* in U3.7 */
   line_width = (int)
      ((state->line_width + (float) state->line_smooth) * 128.0f + 0.5f);
   line_width = CLAMP(line_width, 0, 1023);

   /* use GIQ rules */
   if (line_width == 128 && !state->line_smooth)
      line_width = 0;

   /* in U8.3 */
   point_width = (int) (state->point_size * 8.0f + 0.5f);
   point_width = CLAMP(point_width, 1, 2047);

   dw1 = GEN7_SF_DW1_STATISTICS |
         GEN7_SF_DW1_VIEWPORT_ENABLE;

   dw2 = line_width << GEN7_SF_DW2_LINE_WIDTH__SHIFT;
   if (state->line_smooth)
      dw2 |= GEN7_SF_DW2_AA_LINE_CAP_1_0;

   dw3 = GEN7_SF_DW3_TRUE_AA_LINE_DISTANCE |
         GEN7_SF_DW3_SUBPIXEL_8BITS |
         point_width;

   if (state->line_last_pixel)
      dw3 |= GEN7_SF_DW3_LINE_LAST_PIXEL_ENABLE;

   if (state->flatshade_first) {
      dw3 |= 0 << GEN7_SF_DW3_TRI_PROVOKE__SHIFT |
             0 << GEN7_SF_DW3_LINE_PROVOKE__SHIFT |
             1 << GEN7_SF_DW3_TRIFAN_PROVOKE__SHIFT;
   } else {
      dw3 |= 2 << GEN7_SF_DW3_TRI_PROVOKE__SHIFT |
             1 << GEN7_SF_DW3_LINE_PROVOKE__SHIFT |
             2 << GEN7_SF_DW3_TRIFAN_PROVOKE__SHIFT;
   }

   if (!state->point_size_per_vertex)
      dw3 |= GEN7_SF_DW3_USE_POINT_WIDTH;

   dw3 |= point_width;

   STATIC_ASSERT(Elements(sf->payload) >= 3);
   sf->payload[0] = dw1;
   sf->payload[1] = dw2;
   sf->payload[2] = dw3;

   rasterizer_init_sf_depth_offset_gen6(dev, state, sf);

   sf->dw_msaa = 0;
   sf->dw_raster = rasterizer_get_sf_raster_gen8(dev, state);
}

static void
rasterizer_init_wm_gen6(const struct ilo_dev *dev,
                        const struct pipe_rasterizer_state *state,
                        struct ilo_rasterizer_wm *wm)
{
   uint32_t dw5, dw6;

   ILO_DEV_ASSERT(dev, 6, 6);

   /* only the FF unit states are set, as in GEN7 */

   dw5 = GEN6_WM_DW5_AA_LINE_WIDTH_2_0;

   /* same value as in 3DSTATE_SF */
   if (state->line_smooth)
      dw5 |= GEN6_WM_DW5_AA_LINE_CAP_1_0;

   if (state->poly_stipple_enable)
      dw5 |= GEN6_WM_DW5_POLY_STIPPLE_ENABLE;
   if (state->line_stipple_enable)
      dw5 |= GEN6_WM_DW5_LINE_STIPPLE_ENABLE;

   /*
    * assertion that makes sure
    *
    *   dw6 |= wm->dw_msaa_rast | wm->dw_msaa_disp;
    *
    * is valid
    */
   STATIC_ASSERT(GEN6_WM_DW6_MSRASTMODE_OFF_PIXEL == 0 &&
                 GEN6_WM_DW6_MSDISPMODE_PERSAMPLE == 0);
   dw6 = GEN6_WM_DW6_ZW_INTERP_PIXEL;

   if (state->bottom_edge_rule)
      dw6 |= GEN6_WM_DW6_POINT_RASTRULE_UPPER_RIGHT;

   wm->dw_msaa_rast =
      (state->multisample) ? GEN6_WM_DW6_MSRASTMODE_ON_PATTERN : 0;
   wm->dw_msaa_disp = GEN6_WM_DW6_MSDISPMODE_PERPIXEL;

   STATIC_ASSERT(Elements(wm->payload) >= 2);
   wm->payload[0] = dw5;
   wm->payload[1] = dw6;
}

static void
rasterizer_init_wm_gen7(const struct ilo_dev *dev,
                        const struct pipe_rasterizer_state *state,
                        struct ilo_rasterizer_wm *wm)
{
   uint32_t dw1, dw2;

   ILO_DEV_ASSERT(dev, 7, 7.5);

   /*
    * assertion that makes sure
    *
    *   dw1 |= wm->dw_msaa_rast;
    *   dw2 |= wm->dw_msaa_disp;
    *
    * is valid
    */
   STATIC_ASSERT(GEN7_WM_DW1_MSRASTMODE_OFF_PIXEL == 0 &&
                 GEN7_WM_DW2_MSDISPMODE_PERSAMPLE == 0);
   dw1 = GEN7_WM_DW1_ZW_INTERP_PIXEL |
         GEN7_WM_DW1_AA_LINE_WIDTH_2_0;
   dw2 = 0;

   /* same value as in 3DSTATE_SF */
   if (state->line_smooth)
      dw1 |= GEN7_WM_DW1_AA_LINE_CAP_1_0;

   if (state->poly_stipple_enable)
      dw1 |= GEN7_WM_DW1_POLY_STIPPLE_ENABLE;
   if (state->line_stipple_enable)
      dw1 |= GEN7_WM_DW1_LINE_STIPPLE_ENABLE;

   if (state->bottom_edge_rule)
      dw1 |= GEN7_WM_DW1_POINT_RASTRULE_UPPER_RIGHT;

   wm->dw_msaa_rast =
      (state->multisample) ? GEN7_WM_DW1_MSRASTMODE_ON_PATTERN : 0;
   wm->dw_msaa_disp = GEN7_WM_DW2_MSDISPMODE_PERPIXEL;

   STATIC_ASSERT(Elements(wm->payload) >= 2);
   wm->payload[0] = dw1;
   wm->payload[1] = dw2;
}

static uint32_t
rasterizer_get_wm_gen8(const struct ilo_dev *dev,
                       const struct pipe_rasterizer_state *state)
{
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 8, 8);

   dw = GEN7_WM_DW1_ZW_INTERP_PIXEL |
        GEN7_WM_DW1_AA_LINE_WIDTH_2_0;

   /* same value as in 3DSTATE_SF */
   if (state->line_smooth)
      dw |= GEN7_WM_DW1_AA_LINE_CAP_1_0;

   if (state->poly_stipple_enable)
      dw |= GEN7_WM_DW1_POLY_STIPPLE_ENABLE;
   if (state->line_stipple_enable)
      dw |= GEN7_WM_DW1_LINE_STIPPLE_ENABLE;

   if (state->bottom_edge_rule)
      dw |= GEN7_WM_DW1_POINT_RASTRULE_UPPER_RIGHT;

   return dw;
}

void
ilo_gpe_init_rasterizer(const struct ilo_dev *dev,
                        const struct pipe_rasterizer_state *state,
                        struct ilo_rasterizer_state *rasterizer)
{
   rasterizer_init_clip(dev, state, &rasterizer->clip);

   if (ilo_dev_gen(dev) >= ILO_GEN(8)) {
      memset(&rasterizer->wm, 0, sizeof(rasterizer->wm));
      rasterizer->wm.payload[0] = rasterizer_get_wm_gen8(dev, state);

      rasterizer_init_sf_gen8(dev, state, &rasterizer->sf);
   } else if (ilo_dev_gen(dev) >= ILO_GEN(7)) {
      rasterizer_init_wm_gen7(dev, state, &rasterizer->wm);
      rasterizer_init_sf_gen6(dev, state, &rasterizer->sf);
   } else {
      rasterizer_init_wm_gen6(dev, state, &rasterizer->wm);
      rasterizer_init_sf_gen6(dev, state, &rasterizer->sf);
   }
}

static void
fs_init_cso_gen6(const struct ilo_dev *dev,
                 const struct ilo_shader_state *fs,
                 struct ilo_shader_cso *cso)
{
   int start_grf, input_count, sampler_count, interps, max_threads;
   uint32_t dw2, dw4, dw5, dw6;

   ILO_DEV_ASSERT(dev, 6, 6);

   start_grf = ilo_shader_get_kernel_param(fs, ILO_KERNEL_URB_DATA_START_REG);
   input_count = ilo_shader_get_kernel_param(fs, ILO_KERNEL_INPUT_COUNT);
   sampler_count = ilo_shader_get_kernel_param(fs, ILO_KERNEL_SAMPLER_COUNT);
   interps = ilo_shader_get_kernel_param(fs,
         ILO_KERNEL_FS_BARYCENTRIC_INTERPOLATIONS);

   /* see brwCreateContext() */
   max_threads = (dev->gt == 2) ? 80 : 40;

   dw2 = (true) ? 0 : GEN6_THREADDISP_FP_MODE_ALT;
   dw2 |= ((sampler_count + 3) / 4) << GEN6_THREADDISP_SAMPLER_COUNT__SHIFT;

   dw4 = start_grf << GEN6_WM_DW4_URB_GRF_START0__SHIFT |
         0 << GEN6_WM_DW4_URB_GRF_START1__SHIFT |
         0 << GEN6_WM_DW4_URB_GRF_START2__SHIFT;

   dw5 = (max_threads - 1) << GEN6_WM_DW5_MAX_THREADS__SHIFT;

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 275:
    *
    *     "This bit (Pixel Shader Kill Pixel), if ENABLED, indicates that the
    *      PS kernel or color calculator has the ability to kill (discard)
    *      pixels or samples, other than due to depth or stencil testing.
    *      This bit is required to be ENABLED in the following situations:
    *
    *      The API pixel shader program contains "killpix" or "discard"
    *      instructions, or other code in the pixel shader kernel that can
    *      cause the final pixel mask to differ from the pixel mask received
    *      on dispatch.
    *
    *      A sampler with chroma key enabled with kill pixel mode is used by
    *      the pixel shader.
    *
    *      Any render target has Alpha Test Enable or AlphaToCoverage Enable
    *      enabled.
    *
    *      The pixel shader kernel generates and outputs oMask.
    *
    *      Note: As ClipDistance clipping is fully supported in hardware and
    *      therefore not via PS instructions, there should be no need to
    *      ENABLE this bit due to ClipDistance clipping."
    */
   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_USE_KILL))
      dw5 |= GEN6_WM_DW5_PS_KILL_PIXEL;

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 275:
    *
    *     "If a NULL Depth Buffer is selected, the Pixel Shader Computed Depth
    *      field must be set to disabled."
    *
    * TODO This is not checked yet.
    */
   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_OUTPUT_Z))
      dw5 |= GEN6_WM_DW5_PS_COMPUTE_DEPTH;

   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_INPUT_Z))
      dw5 |= GEN6_WM_DW5_PS_USE_DEPTH;

   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_INPUT_W))
      dw5 |= GEN6_WM_DW5_PS_USE_W;

   /*
    * TODO set this bit only when
    *
    *  a) fs writes colors and color is not masked, or
    *  b) fs writes depth, or
    *  c) fs or cc kills
    */
   if (true)
      dw5 |= GEN6_WM_DW5_PS_DISPATCH_ENABLE;

   assert(!ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_DISPATCH_16_OFFSET));
   dw5 |= GEN6_PS_DISPATCH_8 << GEN6_WM_DW5_PS_DISPATCH_MODE__SHIFT;

   dw6 = input_count << GEN6_WM_DW6_SF_ATTR_COUNT__SHIFT |
         GEN6_WM_DW6_PS_POSOFFSET_NONE |
         interps << GEN6_WM_DW6_BARYCENTRIC_INTERP__SHIFT;

   STATIC_ASSERT(Elements(cso->payload) >= 4);
   cso->payload[0] = dw2;
   cso->payload[1] = dw4;
   cso->payload[2] = dw5;
   cso->payload[3] = dw6;
}

static uint32_t
fs_get_wm_gen7(const struct ilo_dev *dev,
               const struct ilo_shader_state *fs)
{
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 7, 7.5);

   dw = ilo_shader_get_kernel_param(fs,
         ILO_KERNEL_FS_BARYCENTRIC_INTERPOLATIONS) <<
      GEN7_WM_DW1_BARYCENTRIC_INTERP__SHIFT;

   /*
    * TODO set this bit only when
    *
    *  a) fs writes colors and color is not masked, or
    *  b) fs writes depth, or
    *  c) fs or cc kills
    */
   dw |= GEN7_WM_DW1_PS_DISPATCH_ENABLE;

   /*
    * From the Ivy Bridge PRM, volume 2 part 1, page 278:
    *
    *     "This bit (Pixel Shader Kill Pixel), if ENABLED, indicates that
    *      the PS kernel or color calculator has the ability to kill
    *      (discard) pixels or samples, other than due to depth or stencil
    *      testing. This bit is required to be ENABLED in the following
    *      situations:
    *
    *      - The API pixel shader program contains "killpix" or "discard"
    *        instructions, or other code in the pixel shader kernel that
    *        can cause the final pixel mask to differ from the pixel mask
    *        received on dispatch.
    *
    *      - A sampler with chroma key enabled with kill pixel mode is used
    *        by the pixel shader.
    *
    *      - Any render target has Alpha Test Enable or AlphaToCoverage
    *        Enable enabled.
    *
    *      - The pixel shader kernel generates and outputs oMask.
    *
    *      Note: As ClipDistance clipping is fully supported in hardware
    *      and therefore not via PS instructions, there should be no need
    *      to ENABLE this bit due to ClipDistance clipping."
    */
   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_USE_KILL))
      dw |= GEN7_WM_DW1_PS_KILL_PIXEL;

   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_OUTPUT_Z))
      dw |= GEN7_WM_DW1_PSCDEPTH_ON;

   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_INPUT_Z))
      dw |= GEN7_WM_DW1_PS_USE_DEPTH;

   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_INPUT_W))
      dw |= GEN7_WM_DW1_PS_USE_W;

   return dw;
}

static void
fs_init_cso_gen7(const struct ilo_dev *dev,
                 const struct ilo_shader_state *fs,
                 struct ilo_shader_cso *cso)
{
   int start_grf, sampler_count, max_threads;
   uint32_t dw2, dw4, dw5;

   ILO_DEV_ASSERT(dev, 7, 7.5);

   start_grf = ilo_shader_get_kernel_param(fs, ILO_KERNEL_URB_DATA_START_REG);
   sampler_count = ilo_shader_get_kernel_param(fs, ILO_KERNEL_SAMPLER_COUNT);

   dw2 = (true) ? 0 : GEN6_THREADDISP_FP_MODE_ALT;
   dw2 |= ((sampler_count + 3) / 4) << GEN6_THREADDISP_SAMPLER_COUNT__SHIFT;

   dw4 = GEN7_PS_DW4_POSOFFSET_NONE;

   /* see brwCreateContext() */
   switch (ilo_dev_gen(dev)) {
   case ILO_GEN(7.5):
      max_threads = (dev->gt == 3) ? 408 : (dev->gt == 2) ? 204 : 102;
      dw4 |= (max_threads - 1) << GEN75_PS_DW4_MAX_THREADS__SHIFT;
      dw4 |= 1 << GEN75_PS_DW4_SAMPLE_MASK__SHIFT;
      break;
   case ILO_GEN(7):
   default:
      max_threads = (dev->gt == 2) ? 172 : 48;
      dw4 |= (max_threads - 1) << GEN7_PS_DW4_MAX_THREADS__SHIFT;
      break;
   }

   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_PCB_CBUF0_SIZE))
      dw4 |= GEN7_PS_DW4_PUSH_CONSTANT_ENABLE;

   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_INPUT_COUNT))
      dw4 |= GEN7_PS_DW4_ATTR_ENABLE;

   assert(!ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_DISPATCH_16_OFFSET));
   dw4 |= GEN6_PS_DISPATCH_8 << GEN7_PS_DW4_DISPATCH_MODE__SHIFT;

   dw5 = start_grf << GEN7_PS_DW5_URB_GRF_START0__SHIFT |
         0 << GEN7_PS_DW5_URB_GRF_START1__SHIFT |
         0 << GEN7_PS_DW5_URB_GRF_START2__SHIFT;

   STATIC_ASSERT(Elements(cso->payload) >= 4);
   cso->payload[0] = dw2;
   cso->payload[1] = dw4;
   cso->payload[2] = dw5;
   cso->payload[3] = fs_get_wm_gen7(dev, fs);
}

static uint32_t
fs_get_psx_gen8(const struct ilo_dev *dev,
                const struct ilo_shader_state *fs)
{
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 8, 8);

   dw = GEN8_PSX_DW1_DISPATCH_ENABLE;

   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_USE_KILL))
      dw |= GEN8_PSX_DW1_KILL_PIXEL;
   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_OUTPUT_Z))
      dw |= GEN8_PSX_DW1_PSCDEPTH_ON;
   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_INPUT_Z))
      dw |= GEN8_PSX_DW1_USE_DEPTH;
   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_INPUT_W))
      dw |= GEN8_PSX_DW1_USE_W;
   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_INPUT_COUNT))
      dw |= GEN8_PSX_DW1_ATTR_ENABLE;

   return dw;
}

static uint32_t
fs_get_wm_gen8(const struct ilo_dev *dev,
               const struct ilo_shader_state *fs)
{
   ILO_DEV_ASSERT(dev, 8, 8);

   return ilo_shader_get_kernel_param(fs,
         ILO_KERNEL_FS_BARYCENTRIC_INTERPOLATIONS) <<
      GEN7_WM_DW1_BARYCENTRIC_INTERP__SHIFT;
}

static void
fs_init_cso_gen8(const struct ilo_dev *dev,
                 const struct ilo_shader_state *fs,
                 struct ilo_shader_cso *cso)
{
   int start_grf, sampler_count;
   uint32_t dw3, dw6, dw7;

   ILO_DEV_ASSERT(dev, 8, 8);

   start_grf = ilo_shader_get_kernel_param(fs, ILO_KERNEL_URB_DATA_START_REG);
   sampler_count = ilo_shader_get_kernel_param(fs, ILO_KERNEL_SAMPLER_COUNT);

   dw3 = (true) ? 0 : GEN6_THREADDISP_FP_MODE_ALT;
   dw3 |= ((sampler_count + 3) / 4) << GEN6_THREADDISP_SAMPLER_COUNT__SHIFT;

   /* always 64? */
   dw6 = (64 - 2) << GEN8_PS_DW6_MAX_THREADS__SHIFT |
         GEN8_PS_DW6_POSOFFSET_NONE;
   if (ilo_shader_get_kernel_param(fs, ILO_KERNEL_PCB_CBUF0_SIZE))
      dw6 |= GEN8_PS_DW6_PUSH_CONSTANT_ENABLE;

   assert(!ilo_shader_get_kernel_param(fs, ILO_KERNEL_FS_DISPATCH_16_OFFSET));
   dw6 |= GEN6_PS_DISPATCH_8 << GEN8_PS_DW6_DISPATCH_MODE__SHIFT;

   dw7 = start_grf << GEN8_PS_DW7_URB_GRF_START0__SHIFT |
         0 << GEN8_PS_DW7_URB_GRF_START1__SHIFT |
         0 << GEN8_PS_DW7_URB_GRF_START2__SHIFT;

   STATIC_ASSERT(Elements(cso->payload) >= 5);
   cso->payload[0] = dw3;
   cso->payload[1] = dw6;
   cso->payload[2] = dw7;
   cso->payload[3] = fs_get_psx_gen8(dev, fs);
   cso->payload[4] = fs_get_wm_gen8(dev, fs);
}

void
ilo_gpe_init_fs_cso(const struct ilo_dev *dev,
                    const struct ilo_shader_state *fs,
                    struct ilo_shader_cso *cso)
{
   if (ilo_dev_gen(dev) >= ILO_GEN(8))
      fs_init_cso_gen8(dev, fs, cso);
   else if (ilo_dev_gen(dev) >= ILO_GEN(7))
      fs_init_cso_gen7(dev, fs, cso);
   else
      fs_init_cso_gen6(dev, fs, cso);
}

struct ilo_zs_surface_info {
   int surface_type;
   int format;

   struct {
      struct intel_bo *bo;
      unsigned stride;
      unsigned qpitch;
      enum gen_surface_tiling tiling;
      uint32_t offset;
   } zs, stencil, hiz;

   unsigned width, height, depth;
   unsigned lod, first_layer, num_layers;
};

static void
zs_init_info_null(const struct ilo_dev *dev,
                  struct ilo_zs_surface_info *info)
{
   ILO_DEV_ASSERT(dev, 6, 8);

   memset(info, 0, sizeof(*info));

   info->surface_type = GEN6_SURFTYPE_NULL;
   info->format = GEN6_ZFORMAT_D32_FLOAT;
   info->width = 1;
   info->height = 1;
   info->depth = 1;
   info->num_layers = 1;
}

static void
zs_init_info(const struct ilo_dev *dev,
             const struct ilo_image *img,
             const struct ilo_image *s8_img,
             enum pipe_texture_target target,
             enum pipe_format format, unsigned level,
             unsigned first_layer, unsigned num_layers,
             struct ilo_zs_surface_info *info)
{
   bool separate_stencil;

   ILO_DEV_ASSERT(dev, 6, 8);

   memset(info, 0, sizeof(*info));

   info->surface_type = ilo_gpe_gen6_translate_texture(target);

   if (info->surface_type == GEN6_SURFTYPE_CUBE) {
      /*
       * From the Sandy Bridge PRM, volume 2 part 1, page 325-326:
       *
       *     "For Other Surfaces (Cube Surfaces):
       *      This field (Minimum Array Element) is ignored."
       *
       *     "For Other Surfaces (Cube Surfaces):
       *      This field (Render Target View Extent) is ignored."
       *
       * As such, we cannot set first_layer and num_layers on cube surfaces.
       * To work around that, treat it as a 2D surface.
       */
      info->surface_type = GEN6_SURFTYPE_2D;
   }

   if (ilo_dev_gen(dev) >= ILO_GEN(7)) {
      separate_stencil = true;
   } else {
      /*
       * From the Sandy Bridge PRM, volume 2 part 1, page 317:
       *
       *     "This field (Separate Stencil Buffer Enable) must be set to the
       *      same value (enabled or disabled) as Hierarchical Depth Buffer
       *      Enable."
       */
      separate_stencil = ilo_image_can_enable_aux(img, level);
   }

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 317:
    *
    *     "If this field (Hierarchical Depth Buffer Enable) is enabled, the
    *      Surface Format of the depth buffer cannot be
    *      D32_FLOAT_S8X24_UINT or D24_UNORM_S8_UINT. Use of stencil
    *      requires the separate stencil buffer."
    *
    * From the Ironlake PRM, volume 2 part 1, page 330:
    *
    *     "If this field (Separate Stencil Buffer Enable) is disabled, the
    *      Surface Format of the depth buffer cannot be D24_UNORM_X8_UINT."
    *
    * There is no similar restriction for GEN6.  But when D24_UNORM_X8_UINT
    * is indeed used, the depth values output by the fragment shaders will
    * be different when read back.
    *
    * As for GEN7+, separate_stencil is always true.
    */
   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      info->format = GEN6_ZFORMAT_D16_UNORM;
      break;
   case PIPE_FORMAT_Z32_FLOAT:
      info->format = GEN6_ZFORMAT_D32_FLOAT;
      break;
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      info->format = (separate_stencil) ?
         GEN6_ZFORMAT_D24_UNORM_X8_UINT :
         GEN6_ZFORMAT_D24_UNORM_S8_UINT;
      break;
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      info->format = (separate_stencil) ?
         GEN6_ZFORMAT_D32_FLOAT :
         GEN6_ZFORMAT_D32_FLOAT_S8X24_UINT;
      break;
   case PIPE_FORMAT_S8_UINT:
      if (separate_stencil) {
         info->format = GEN6_ZFORMAT_D32_FLOAT;
         break;
      }
      /* fall through */
   default:
      assert(!"unsupported depth/stencil format");
      zs_init_info_null(dev, info);
      return;
      break;
   }

   if (format != PIPE_FORMAT_S8_UINT) {
      info->zs.bo = img->bo;
      info->zs.stride = img->bo_stride;

      assert(img->walk_layer_height % 4 == 0);
      info->zs.qpitch = img->walk_layer_height / 4;

      info->zs.tiling = img->tiling;
      info->zs.offset = 0;
   }

   if (s8_img || format == PIPE_FORMAT_S8_UINT) {
      info->stencil.bo = s8_img->bo;

      /*
       * From the Sandy Bridge PRM, volume 2 part 1, page 329:
       *
       *     "The pitch must be set to 2x the value computed based on width,
       *       as the stencil buffer is stored with two rows interleaved."
       *
       * For GEN7, we still dobule the stride because we did not double the
       * slice widths when initializing the layout.
       */
      info->stencil.stride = s8_img->bo_stride * 2;

      assert(s8_img->walk_layer_height % 4 == 0);
      info->stencil.qpitch = s8_img->walk_layer_height / 4;

      info->stencil.tiling = s8_img->tiling;

      if (ilo_dev_gen(dev) == ILO_GEN(6)) {
         unsigned x, y;

         assert(s8_img->walk == ILO_IMAGE_WALK_LOD);

         /* offset to the level */
         ilo_image_get_slice_pos(s8_img, level, 0, &x, &y);
         ilo_image_pos_to_mem(s8_img, x, y, &x, &y);
         info->stencil.offset = ilo_image_mem_to_raw(s8_img, x, y);
      }
   }

   if (ilo_image_can_enable_aux(img, level)) {
      info->hiz.bo = img->aux.bo;
      info->hiz.stride = img->aux.bo_stride;

      assert(img->aux.walk_layer_height % 4 == 0);
      info->hiz.qpitch = img->aux.walk_layer_height / 4;

      info->hiz.tiling = GEN6_TILING_Y;

      /* offset to the level */
      if (ilo_dev_gen(dev) == ILO_GEN(6))
         info->hiz.offset = img->aux.walk_lod_offsets[level];
   }

   info->width = img->width0;
   info->height = img->height0;
   info->depth = (target == PIPE_TEXTURE_3D) ? img->depth0 : num_layers;

   info->lod = level;
   info->first_layer = first_layer;
   info->num_layers = num_layers;
}

void
ilo_gpe_init_zs_surface(const struct ilo_dev *dev,
                        const struct ilo_image *img,
                        const struct ilo_image *s8_img,
                        enum pipe_texture_target target,
                        enum pipe_format format, unsigned level,
                        unsigned first_layer, unsigned num_layers,
                        struct ilo_zs_surface *zs)
{
   const int max_2d_size = (ilo_dev_gen(dev) >= ILO_GEN(7)) ? 16384 : 8192;
   const int max_array_size = (ilo_dev_gen(dev) >= ILO_GEN(7)) ? 2048 : 512;
   struct ilo_zs_surface_info info;
   uint32_t dw1, dw2, dw3, dw4, dw5, dw6;
   int align_w = 8, align_h = 4;

   ILO_DEV_ASSERT(dev, 6, 8);

   if (img) {
      zs_init_info(dev, img, s8_img, target, format,
            level, first_layer, num_layers, &info);

      switch (img->sample_count) {
      case 2:
         align_w /= 2;
         break;
      case 4:
         align_w /= 2;
         align_h /= 2;
         break;
      case 8:
         align_w /= 4;
         align_h /= 2;
         break;
      case 16:
         align_w /= 4;
         align_h /= 4;
         break;
      default:
         break;
      }
   } else {
      zs_init_info_null(dev, &info);
   }

   switch (info.surface_type) {
   case GEN6_SURFTYPE_NULL:
      break;
   case GEN6_SURFTYPE_1D:
      assert(info.width <= max_2d_size && info.height == 1 &&
             info.depth <= max_array_size);
      assert(info.first_layer < max_array_size - 1 &&
             info.num_layers <= max_array_size);
      break;
   case GEN6_SURFTYPE_2D:
      assert(info.width <= max_2d_size && info.height <= max_2d_size &&
             info.depth <= max_array_size);
      assert(info.first_layer < max_array_size - 1 &&
             info.num_layers <= max_array_size);
      break;
   case GEN6_SURFTYPE_3D:
      assert(info.width <= 2048 && info.height <= 2048 && info.depth <= 2048);
      assert(info.first_layer < 2048 && info.num_layers <= max_array_size);
      break;
   case GEN6_SURFTYPE_CUBE:
      assert(info.width <= max_2d_size && info.height <= max_2d_size &&
             info.depth == 1);
      assert(info.first_layer == 0 && info.num_layers == 1);
      assert(info.width == info.height);
      break;
   default:
      assert(!"unexpected depth surface type");
      break;
   }

   dw1 = info.surface_type << GEN6_DEPTH_DW1_TYPE__SHIFT |
         info.format << GEN6_DEPTH_DW1_FORMAT__SHIFT;

   if (info.zs.bo) {
      /* required for GEN6+ */
      assert(info.zs.tiling == GEN6_TILING_Y);
      assert(info.zs.stride > 0 && info.zs.stride < 128 * 1024 &&
            info.zs.stride % 128 == 0);
      assert(info.width <= info.zs.stride);

      dw1 |= (info.zs.stride - 1);
      dw2 = info.zs.offset;
   } else {
      dw2 = 0;
   }

   if (ilo_dev_gen(dev) >= ILO_GEN(7)) {
      if (info.zs.bo)
         dw1 |= GEN7_DEPTH_DW1_DEPTH_WRITE_ENABLE;

      if (info.stencil.bo)
         dw1 |= GEN7_DEPTH_DW1_STENCIL_WRITE_ENABLE;

      if (info.hiz.bo)
         dw1 |= GEN7_DEPTH_DW1_HIZ_ENABLE;

      dw3 = (info.height - 1) << GEN7_DEPTH_DW3_HEIGHT__SHIFT |
            (info.width - 1) << GEN7_DEPTH_DW3_WIDTH__SHIFT |
            info.lod << GEN7_DEPTH_DW3_LOD__SHIFT;

      zs->dw_aligned_8x4 =
         (align(info.height, align_h) - 1) << GEN7_DEPTH_DW3_HEIGHT__SHIFT |
         (align(info.width, align_w) - 1) << GEN7_DEPTH_DW3_WIDTH__SHIFT |
         info.lod << GEN7_DEPTH_DW3_LOD__SHIFT;

      dw4 = (info.depth - 1) << GEN7_DEPTH_DW4_DEPTH__SHIFT |
            info.first_layer << GEN7_DEPTH_DW4_MIN_ARRAY_ELEMENT__SHIFT;

      dw5 = 0;

      dw6 = (info.num_layers - 1) << GEN7_DEPTH_DW6_RT_VIEW_EXTENT__SHIFT;

      if (ilo_dev_gen(dev) >= ILO_GEN(8))
         dw6 |= info.zs.qpitch;
   } else {
      /* always Y-tiled */
      dw1 |= GEN6_TILING_Y << GEN6_DEPTH_DW1_TILING__SHIFT;

      if (info.hiz.bo) {
         dw1 |= GEN6_DEPTH_DW1_HIZ_ENABLE |
                GEN6_DEPTH_DW1_SEPARATE_STENCIL;
      }

      dw3 = (info.height - 1) << GEN6_DEPTH_DW3_HEIGHT__SHIFT |
            (info.width - 1) << GEN6_DEPTH_DW3_WIDTH__SHIFT |
            info.lod << GEN6_DEPTH_DW3_LOD__SHIFT |
            GEN6_DEPTH_DW3_MIPLAYOUT_BELOW;

      zs->dw_aligned_8x4 =
         (align(info.height, align_h) - 1) << GEN6_DEPTH_DW3_HEIGHT__SHIFT |
         (align(info.width, align_w) - 1) << GEN6_DEPTH_DW3_WIDTH__SHIFT |
         info.lod << GEN6_DEPTH_DW3_LOD__SHIFT |
         GEN6_DEPTH_DW3_MIPLAYOUT_BELOW;

      dw4 = (info.depth - 1) << GEN6_DEPTH_DW4_DEPTH__SHIFT |
            info.first_layer << GEN6_DEPTH_DW4_MIN_ARRAY_ELEMENT__SHIFT |
            (info.num_layers - 1) << GEN6_DEPTH_DW4_RT_VIEW_EXTENT__SHIFT;

      dw5 = 0;

      dw6 = 0;
   }

   STATIC_ASSERT(Elements(zs->payload) >= 12);

   zs->payload[0] = dw1;
   zs->payload[1] = dw2;
   zs->payload[2] = dw3;
   zs->payload[3] = dw4;
   zs->payload[4] = dw5;
   zs->payload[5] = dw6;

   /* do not increment reference count */
   zs->bo = info.zs.bo;

   /* separate stencil */
   if (info.stencil.bo) {
      assert(info.stencil.stride > 0 && info.stencil.stride < 128 * 1024 &&
             info.stencil.stride % 128 == 0);

      dw1 = (info.stencil.stride - 1) << GEN6_STENCIL_DW1_PITCH__SHIFT;
      if (ilo_dev_gen(dev) >= ILO_GEN(7.5))
         dw1 |= GEN75_STENCIL_DW1_STENCIL_BUFFER_ENABLE;

      dw2 = info.stencil.offset;
      dw4 = info.stencil.qpitch;
   } else {
      dw1 = 0;
      dw2 = 0;
      dw4 = 0;
   }

   zs->payload[6] = dw1;
   zs->payload[7] = dw2;
   zs->payload[8] = dw4;
   /* do not increment reference count */
   zs->separate_s8_bo = info.stencil.bo;

   /* hiz */
   if (info.hiz.bo) {
      dw1 = (info.hiz.stride - 1) << GEN6_HIZ_DW1_PITCH__SHIFT;
      dw2 = info.hiz.offset;
      dw4 = info.hiz.qpitch;
   } else {
      dw1 = 0;
      dw2 = 0;
      dw4 = 0;
   }

   zs->payload[9] = dw1;
   zs->payload[10] = dw2;
   zs->payload[11] = dw4;
   /* do not increment reference count */
   zs->hiz_bo = info.hiz.bo;
}

static void
viewport_get_guardband(const struct ilo_dev *dev,
                       int center_x, int center_y,
                       int *min_gbx, int *max_gbx,
                       int *min_gby, int *max_gby)
{
   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 234:
    *
    *     "Per-Device Guardband Extents
    *
    *       - Supported X,Y ScreenSpace "Guardband" Extent: [-16K,16K-1]
    *       - Maximum Post-Clamp Delta (X or Y): 16K"
    *
    *     "In addition, in order to be correctly rendered, objects must have a
    *      screenspace bounding box not exceeding 8K in the X or Y direction.
    *      This additional restriction must also be comprehended by software,
    *      i.e., enforced by use of clipping."
    *
    * From the Ivy Bridge PRM, volume 2 part 1, page 248:
    *
    *     "Per-Device Guardband Extents
    *
    *       - Supported X,Y ScreenSpace "Guardband" Extent: [-32K,32K-1]
    *       - Maximum Post-Clamp Delta (X or Y): N/A"
    *
    *     "In addition, in order to be correctly rendered, objects must have a
    *      screenspace bounding box not exceeding 8K in the X or Y direction.
    *      This additional restriction must also be comprehended by software,
    *      i.e., enforced by use of clipping."
    *
    * Combined, the bounding box of any object can not exceed 8K in both
    * width and height.
    *
    * Below we set the guardband as a squre of length 8K, centered at where
    * the viewport is.  This makes sure all objects passing the GB test are
    * valid to the renderer, and those failing the XY clipping have a
    * better chance of passing the GB test.
    */
   const int max_extent = (ilo_dev_gen(dev) >= ILO_GEN(7)) ? 32768 : 16384;
   const int half_len = 8192 / 2;

   /* make sure the guardband is within the valid range */
   if (center_x - half_len < -max_extent)
      center_x = -max_extent + half_len;
   else if (center_x + half_len > max_extent - 1)
      center_x = max_extent - half_len;

   if (center_y - half_len < -max_extent)
      center_y = -max_extent + half_len;
   else if (center_y + half_len > max_extent - 1)
      center_y = max_extent - half_len;

   *min_gbx = (float) (center_x - half_len);
   *max_gbx = (float) (center_x + half_len);
   *min_gby = (float) (center_y - half_len);
   *max_gby = (float) (center_y + half_len);
}

void
ilo_gpe_set_viewport_cso(const struct ilo_dev *dev,
                         const struct pipe_viewport_state *state,
                         struct ilo_viewport_cso *vp)
{
   const float scale_x = fabs(state->scale[0]);
   const float scale_y = fabs(state->scale[1]);
   const float scale_z = fabs(state->scale[2]);
   int min_gbx, max_gbx, min_gby, max_gby;

   ILO_DEV_ASSERT(dev, 6, 8);

   viewport_get_guardband(dev,
         (int) state->translate[0],
         (int) state->translate[1],
         &min_gbx, &max_gbx, &min_gby, &max_gby);

   /* matrix form */
   vp->m00 = state->scale[0];
   vp->m11 = state->scale[1];
   vp->m22 = state->scale[2];
   vp->m30 = state->translate[0];
   vp->m31 = state->translate[1];
   vp->m32 = state->translate[2];

   /* guardband in NDC space */
   vp->min_gbx = ((float) min_gbx - state->translate[0]) / scale_x;
   vp->max_gbx = ((float) max_gbx - state->translate[0]) / scale_x;
   vp->min_gby = ((float) min_gby - state->translate[1]) / scale_y;
   vp->max_gby = ((float) max_gby - state->translate[1]) / scale_y;

   /* viewport in screen space */
   vp->min_x = scale_x * -1.0f + state->translate[0];
   vp->max_x = scale_x *  1.0f + state->translate[0];
   vp->min_y = scale_y * -1.0f + state->translate[1];
   vp->max_y = scale_y *  1.0f + state->translate[1];
   vp->min_z = scale_z * -1.0f + state->translate[2];
   vp->max_z = scale_z *  1.0f + state->translate[2];
}

/**
 * Translate a pipe logicop to the matching hardware logicop.
 */
static int
gen6_translate_pipe_logicop(unsigned logicop)
{
   switch (logicop) {
   case PIPE_LOGICOP_CLEAR:         return GEN6_LOGICOP_CLEAR;
   case PIPE_LOGICOP_NOR:           return GEN6_LOGICOP_NOR;
   case PIPE_LOGICOP_AND_INVERTED:  return GEN6_LOGICOP_AND_INVERTED;
   case PIPE_LOGICOP_COPY_INVERTED: return GEN6_LOGICOP_COPY_INVERTED;
   case PIPE_LOGICOP_AND_REVERSE:   return GEN6_LOGICOP_AND_REVERSE;
   case PIPE_LOGICOP_INVERT:        return GEN6_LOGICOP_INVERT;
   case PIPE_LOGICOP_XOR:           return GEN6_LOGICOP_XOR;
   case PIPE_LOGICOP_NAND:          return GEN6_LOGICOP_NAND;
   case PIPE_LOGICOP_AND:           return GEN6_LOGICOP_AND;
   case PIPE_LOGICOP_EQUIV:         return GEN6_LOGICOP_EQUIV;
   case PIPE_LOGICOP_NOOP:          return GEN6_LOGICOP_NOOP;
   case PIPE_LOGICOP_OR_INVERTED:   return GEN6_LOGICOP_OR_INVERTED;
   case PIPE_LOGICOP_COPY:          return GEN6_LOGICOP_COPY;
   case PIPE_LOGICOP_OR_REVERSE:    return GEN6_LOGICOP_OR_REVERSE;
   case PIPE_LOGICOP_OR:            return GEN6_LOGICOP_OR;
   case PIPE_LOGICOP_SET:           return GEN6_LOGICOP_SET;
   default:
      assert(!"unknown logicop function");
      return GEN6_LOGICOP_CLEAR;
   }
}

/**
 * Translate a pipe blend function to the matching hardware blend function.
 */
static int
gen6_translate_pipe_blend(unsigned blend)
{
   switch (blend) {
   case PIPE_BLEND_ADD:                return GEN6_BLENDFUNCTION_ADD;
   case PIPE_BLEND_SUBTRACT:           return GEN6_BLENDFUNCTION_SUBTRACT;
   case PIPE_BLEND_REVERSE_SUBTRACT:   return GEN6_BLENDFUNCTION_REVERSE_SUBTRACT;
   case PIPE_BLEND_MIN:                return GEN6_BLENDFUNCTION_MIN;
   case PIPE_BLEND_MAX:                return GEN6_BLENDFUNCTION_MAX;
   default:
      assert(!"unknown blend function");
      return GEN6_BLENDFUNCTION_ADD;
   };
}

/**
 * Translate a pipe blend factor to the matching hardware blend factor.
 */
static int
gen6_translate_pipe_blendfactor(unsigned blendfactor)
{
   switch (blendfactor) {
   case PIPE_BLENDFACTOR_ONE:                return GEN6_BLENDFACTOR_ONE;
   case PIPE_BLENDFACTOR_SRC_COLOR:          return GEN6_BLENDFACTOR_SRC_COLOR;
   case PIPE_BLENDFACTOR_SRC_ALPHA:          return GEN6_BLENDFACTOR_SRC_ALPHA;
   case PIPE_BLENDFACTOR_DST_ALPHA:          return GEN6_BLENDFACTOR_DST_ALPHA;
   case PIPE_BLENDFACTOR_DST_COLOR:          return GEN6_BLENDFACTOR_DST_COLOR;
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE: return GEN6_BLENDFACTOR_SRC_ALPHA_SATURATE;
   case PIPE_BLENDFACTOR_CONST_COLOR:        return GEN6_BLENDFACTOR_CONST_COLOR;
   case PIPE_BLENDFACTOR_CONST_ALPHA:        return GEN6_BLENDFACTOR_CONST_ALPHA;
   case PIPE_BLENDFACTOR_SRC1_COLOR:         return GEN6_BLENDFACTOR_SRC1_COLOR;
   case PIPE_BLENDFACTOR_SRC1_ALPHA:         return GEN6_BLENDFACTOR_SRC1_ALPHA;
   case PIPE_BLENDFACTOR_ZERO:               return GEN6_BLENDFACTOR_ZERO;
   case PIPE_BLENDFACTOR_INV_SRC_COLOR:      return GEN6_BLENDFACTOR_INV_SRC_COLOR;
   case PIPE_BLENDFACTOR_INV_SRC_ALPHA:      return GEN6_BLENDFACTOR_INV_SRC_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_ALPHA:      return GEN6_BLENDFACTOR_INV_DST_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_COLOR:      return GEN6_BLENDFACTOR_INV_DST_COLOR;
   case PIPE_BLENDFACTOR_INV_CONST_COLOR:    return GEN6_BLENDFACTOR_INV_CONST_COLOR;
   case PIPE_BLENDFACTOR_INV_CONST_ALPHA:    return GEN6_BLENDFACTOR_INV_CONST_ALPHA;
   case PIPE_BLENDFACTOR_INV_SRC1_COLOR:     return GEN6_BLENDFACTOR_INV_SRC1_COLOR;
   case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:     return GEN6_BLENDFACTOR_INV_SRC1_ALPHA;
   default:
      assert(!"unknown blend factor");
      return GEN6_BLENDFACTOR_ONE;
   };
}

/**
 * Translate a pipe stencil op to the matching hardware stencil op.
 */
static int
gen6_translate_pipe_stencil_op(unsigned stencil_op)
{
   switch (stencil_op) {
   case PIPE_STENCIL_OP_KEEP:       return GEN6_STENCILOP_KEEP;
   case PIPE_STENCIL_OP_ZERO:       return GEN6_STENCILOP_ZERO;
   case PIPE_STENCIL_OP_REPLACE:    return GEN6_STENCILOP_REPLACE;
   case PIPE_STENCIL_OP_INCR:       return GEN6_STENCILOP_INCRSAT;
   case PIPE_STENCIL_OP_DECR:       return GEN6_STENCILOP_DECRSAT;
   case PIPE_STENCIL_OP_INCR_WRAP:  return GEN6_STENCILOP_INCR;
   case PIPE_STENCIL_OP_DECR_WRAP:  return GEN6_STENCILOP_DECR;
   case PIPE_STENCIL_OP_INVERT:     return GEN6_STENCILOP_INVERT;
   default:
      assert(!"unknown stencil op");
      return GEN6_STENCILOP_KEEP;
   }
}

static int
gen6_blend_factor_dst_alpha_forced_one(int factor)
{
   switch (factor) {
   case GEN6_BLENDFACTOR_DST_ALPHA:
      return GEN6_BLENDFACTOR_ONE;
   case GEN6_BLENDFACTOR_INV_DST_ALPHA:
   case GEN6_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return GEN6_BLENDFACTOR_ZERO;
   default:
      return factor;
   }
}

static uint32_t
blend_get_rt_blend_enable_gen6(const struct ilo_dev *dev,
                               const struct pipe_rt_blend_state *rt,
                               bool dst_alpha_forced_one)
{
   int rgb_src, rgb_dst, a_src, a_dst;
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 6, 7.5);

   if (!rt->blend_enable)
      return 0;

   rgb_src = gen6_translate_pipe_blendfactor(rt->rgb_src_factor);
   rgb_dst = gen6_translate_pipe_blendfactor(rt->rgb_dst_factor);
   a_src = gen6_translate_pipe_blendfactor(rt->alpha_src_factor);
   a_dst = gen6_translate_pipe_blendfactor(rt->alpha_dst_factor);

   if (dst_alpha_forced_one) {
      rgb_src = gen6_blend_factor_dst_alpha_forced_one(rgb_src);
      rgb_dst = gen6_blend_factor_dst_alpha_forced_one(rgb_dst);
      a_src = gen6_blend_factor_dst_alpha_forced_one(a_src);
      a_dst = gen6_blend_factor_dst_alpha_forced_one(a_dst);
   }

   dw = GEN6_RT_DW0_BLEND_ENABLE |
        gen6_translate_pipe_blend(rt->alpha_func) << 26 |
        a_src << 20 |
        a_dst << 15 |
        gen6_translate_pipe_blend(rt->rgb_func) << 11 |
        rgb_src << 5 |
        rgb_dst;

   if (rt->rgb_func != rt->alpha_func ||
       rgb_src != a_src || rgb_dst != a_dst)
      dw |= GEN6_RT_DW0_INDEPENDENT_ALPHA_ENABLE;

   return dw;
}

static uint32_t
blend_get_rt_blend_enable_gen8(const struct ilo_dev *dev,
                               const struct pipe_rt_blend_state *rt,
                               bool dst_alpha_forced_one,
                               bool *independent_alpha)
{
   int rgb_src, rgb_dst, a_src, a_dst;
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 8, 8);

   if (!rt->blend_enable) {
      *independent_alpha = false;
      return 0;
   }

   rgb_src = gen6_translate_pipe_blendfactor(rt->rgb_src_factor);
   rgb_dst = gen6_translate_pipe_blendfactor(rt->rgb_dst_factor);
   a_src = gen6_translate_pipe_blendfactor(rt->alpha_src_factor);
   a_dst = gen6_translate_pipe_blendfactor(rt->alpha_dst_factor);

   if (dst_alpha_forced_one) {
      rgb_src = gen6_blend_factor_dst_alpha_forced_one(rgb_src);
      rgb_dst = gen6_blend_factor_dst_alpha_forced_one(rgb_dst);
      a_src = gen6_blend_factor_dst_alpha_forced_one(a_src);
      a_dst = gen6_blend_factor_dst_alpha_forced_one(a_dst);
   }

   dw = GEN8_RT_DW0_BLEND_ENABLE |
        rgb_src << 26 |
        rgb_dst << 21 |
        gen6_translate_pipe_blend(rt->rgb_func) << 18 |
        a_src << 13 |
        a_dst << 8 |
        gen6_translate_pipe_blend(rt->alpha_func) << 5;

   *independent_alpha = (rt->rgb_func != rt->alpha_func ||
                         rgb_src != a_src ||
                         rgb_dst != a_dst);

   return dw;
}

static void
blend_init_cso_gen6(const struct ilo_dev *dev,
                    const struct pipe_blend_state *state,
                    struct ilo_blend_state *blend,
                    unsigned index)
{
   const struct pipe_rt_blend_state *rt = &state->rt[index];
   struct ilo_blend_cso *cso = &blend->cso[index];

   ILO_DEV_ASSERT(dev, 6, 7.5);

   cso->payload[0] = 0;
   cso->payload[1] = GEN6_RT_DW1_COLORCLAMP_RTFORMAT |
                     GEN6_RT_DW1_PRE_BLEND_CLAMP |
                     GEN6_RT_DW1_POST_BLEND_CLAMP;

   if (!(rt->colormask & PIPE_MASK_A))
      cso->payload[1] |= GEN6_RT_DW1_WRITE_DISABLE_A;
   if (!(rt->colormask & PIPE_MASK_R))
      cso->payload[1] |= GEN6_RT_DW1_WRITE_DISABLE_R;
   if (!(rt->colormask & PIPE_MASK_G))
      cso->payload[1] |= GEN6_RT_DW1_WRITE_DISABLE_G;
   if (!(rt->colormask & PIPE_MASK_B))
      cso->payload[1] |= GEN6_RT_DW1_WRITE_DISABLE_B;

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 365:
    *
    *     "Color Buffer Blending and Logic Ops must not be enabled
    *      simultaneously, or behavior is UNDEFINED."
    *
    * Since state->logicop_enable takes precedence over rt->blend_enable,
    * no special care is needed.
    */
   if (state->logicop_enable) {
      cso->dw_blend = 0;
      cso->dw_blend_dst_alpha_forced_one = 0;
   } else {
      cso->dw_blend = blend_get_rt_blend_enable_gen6(dev, rt, false);
      cso->dw_blend_dst_alpha_forced_one =
         blend_get_rt_blend_enable_gen6(dev, rt, true);
   }
}

static bool
blend_init_cso_gen8(const struct ilo_dev *dev,
                    const struct pipe_blend_state *state,
                    struct ilo_blend_state *blend,
                    unsigned index)
{
   const struct pipe_rt_blend_state *rt = &state->rt[index];
   struct ilo_blend_cso *cso = &blend->cso[index];
   bool independent_alpha = false;

   ILO_DEV_ASSERT(dev, 8, 8);

   cso->payload[0] = 0;
   cso->payload[1] = GEN8_RT_DW1_COLORCLAMP_RTFORMAT |
                     GEN8_RT_DW1_PRE_BLEND_CLAMP |
                     GEN8_RT_DW1_POST_BLEND_CLAMP;

   if (!(rt->colormask & PIPE_MASK_A))
      cso->payload[0] |= GEN8_RT_DW0_WRITE_DISABLE_A;
   if (!(rt->colormask & PIPE_MASK_R))
      cso->payload[0] |= GEN8_RT_DW0_WRITE_DISABLE_R;
   if (!(rt->colormask & PIPE_MASK_G))
      cso->payload[0] |= GEN8_RT_DW0_WRITE_DISABLE_G;
   if (!(rt->colormask & PIPE_MASK_B))
      cso->payload[0] |= GEN8_RT_DW0_WRITE_DISABLE_B;

   if (state->logicop_enable) {
      cso->dw_blend = 0;
      cso->dw_blend_dst_alpha_forced_one = 0;
   } else {
      bool tmp[2];

      cso->dw_blend = blend_get_rt_blend_enable_gen8(dev, rt, false, &tmp[0]);
      cso->dw_blend_dst_alpha_forced_one =
         blend_get_rt_blend_enable_gen8(dev, rt, true, &tmp[1]);

      if (tmp[0] || tmp[1])
         independent_alpha = true;
   }

   return independent_alpha;
}

static uint32_t
blend_get_logicop_enable_gen6(const struct ilo_dev *dev,
                              const struct pipe_blend_state *state)
{
   ILO_DEV_ASSERT(dev, 6, 7.5);

   if (!state->logicop_enable)
      return 0;

   return GEN6_RT_DW1_LOGICOP_ENABLE |
          gen6_translate_pipe_logicop(state->logicop_func) << 18;
}

static uint32_t
blend_get_logicop_enable_gen8(const struct ilo_dev *dev,
                              const struct pipe_blend_state *state)
{
   ILO_DEV_ASSERT(dev, 8, 8);

   if (!state->logicop_enable)
      return 0;

   return GEN8_RT_DW1_LOGICOP_ENABLE |
          gen6_translate_pipe_logicop(state->logicop_func) << 27;
}

static uint32_t
blend_get_alpha_mod_gen6(const struct ilo_dev *dev,
                         const struct pipe_blend_state *state,
                         bool dual_blend)
{
   uint32_t dw = 0;

   ILO_DEV_ASSERT(dev, 6, 7.5);

   if (state->alpha_to_coverage) {
      dw |= GEN6_RT_DW1_ALPHA_TO_COVERAGE;
      if (ilo_dev_gen(dev) >= ILO_GEN(7))
         dw |= GEN6_RT_DW1_ALPHA_TO_COVERAGE_DITHER;
   }
   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 378:
    *
    *     "If Dual Source Blending is enabled, this bit (AlphaToOne Enable)
    *      must be disabled."
    */
   if (state->alpha_to_one && !dual_blend)
      dw |= GEN6_RT_DW1_ALPHA_TO_ONE;

   return dw;
}

static uint32_t
blend_get_alpha_mod_gen8(const struct ilo_dev *dev,
                         const struct pipe_blend_state *state,
                         bool dual_blend)
{
   uint32_t dw = 0;

   ILO_DEV_ASSERT(dev, 8, 8);

   if (state->alpha_to_coverage) {
      dw |= GEN8_BLEND_DW0_ALPHA_TO_COVERAGE |
            GEN8_BLEND_DW0_ALPHA_TO_COVERAGE_DITHER;
   }

   if (state->alpha_to_one && !dual_blend)
      dw |= GEN8_BLEND_DW0_ALPHA_TO_ONE;

   return dw;
}

static uint32_t
blend_get_ps_blend_gen8(const struct ilo_dev *dev, uint32_t rt_dw0)
{
   int rgb_src, rgb_dst, a_src, a_dst;
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 8, 8);

   if (!(rt_dw0 & GEN8_RT_DW0_BLEND_ENABLE))
      return 0;

   a_src = GEN_EXTRACT(rt_dw0, GEN8_RT_DW0_SRC_ALPHA_FACTOR);
   a_dst = GEN_EXTRACT(rt_dw0, GEN8_RT_DW0_DST_ALPHA_FACTOR);
   rgb_src = GEN_EXTRACT(rt_dw0, GEN8_RT_DW0_SRC_COLOR_FACTOR);
   rgb_dst = GEN_EXTRACT(rt_dw0, GEN8_RT_DW0_DST_COLOR_FACTOR);

   dw = GEN8_PS_BLEND_DW1_BLEND_ENABLE;
   dw |= GEN_SHIFT32(a_src, GEN8_PS_BLEND_DW1_SRC_ALPHA_FACTOR);
   dw |= GEN_SHIFT32(a_dst, GEN8_PS_BLEND_DW1_DST_ALPHA_FACTOR);
   dw |= GEN_SHIFT32(rgb_src, GEN8_PS_BLEND_DW1_SRC_COLOR_FACTOR);
   dw |= GEN_SHIFT32(rgb_dst, GEN8_PS_BLEND_DW1_DST_COLOR_FACTOR);

   if (a_src != rgb_src || a_dst != rgb_dst)
      dw |= GEN8_PS_BLEND_DW1_INDEPENDENT_ALPHA_ENABLE;

   return dw;
}

void
ilo_gpe_init_blend(const struct ilo_dev *dev,
                   const struct pipe_blend_state *state,
                   struct ilo_blend_state *blend)
{
   unsigned i;

   ILO_DEV_ASSERT(dev, 6, 8);

   blend->dual_blend = (util_blend_state_is_dual(state, 0) &&
                        state->rt[0].blend_enable &&
                        !state->logicop_enable);
   blend->alpha_to_coverage = state->alpha_to_coverage;

   if (ilo_dev_gen(dev) >= ILO_GEN(8)) {
      bool independent_alpha;

      blend->dw_alpha_mod =
         blend_get_alpha_mod_gen8(dev, state, blend->dual_blend);
      blend->dw_logicop = blend_get_logicop_enable_gen8(dev, state);
      blend->dw_shared = (state->dither) ? GEN8_BLEND_DW0_DITHER_ENABLE : 0;

      independent_alpha = blend_init_cso_gen8(dev, state, blend, 0);
      if (independent_alpha)
         blend->dw_shared |= GEN8_BLEND_DW0_INDEPENDENT_ALPHA_ENABLE;

      blend->dw_ps_blend = blend_get_ps_blend_gen8(dev,
            blend->cso[0].dw_blend);
      blend->dw_ps_blend_dst_alpha_forced_one = blend_get_ps_blend_gen8(dev,
            blend->cso[0].dw_blend_dst_alpha_forced_one);

      if (state->independent_blend_enable) {
         for (i = 1; i < Elements(blend->cso); i++) {
            independent_alpha = blend_init_cso_gen8(dev, state, blend, i);
            if (independent_alpha)
               blend->dw_shared |= GEN8_BLEND_DW0_INDEPENDENT_ALPHA_ENABLE;
         }
      } else {
         for (i = 1; i < Elements(blend->cso); i++)
            blend->cso[i] = blend->cso[0];
      }
   } else {
      blend->dw_alpha_mod =
         blend_get_alpha_mod_gen6(dev, state, blend->dual_blend);
      blend->dw_logicop = blend_get_logicop_enable_gen6(dev, state);
      blend->dw_shared = (state->dither) ? GEN6_RT_DW1_DITHER_ENABLE : 0;

      blend->dw_ps_blend = 0;
      blend->dw_ps_blend_dst_alpha_forced_one = 0;

      blend_init_cso_gen6(dev, state, blend, 0);
      if (state->independent_blend_enable) {
         for (i = 1; i < Elements(blend->cso); i++)
            blend_init_cso_gen6(dev, state, blend, i);
      } else {
         for (i = 1; i < Elements(blend->cso); i++)
            blend->cso[i] = blend->cso[0];
      }
   }
}

/**
 * Translate a pipe DSA test function to the matching hardware compare
 * function.
 */
static int
gen6_translate_dsa_func(unsigned func)
{
   switch (func) {
   case PIPE_FUNC_NEVER:      return GEN6_COMPAREFUNCTION_NEVER;
   case PIPE_FUNC_LESS:       return GEN6_COMPAREFUNCTION_LESS;
   case PIPE_FUNC_EQUAL:      return GEN6_COMPAREFUNCTION_EQUAL;
   case PIPE_FUNC_LEQUAL:     return GEN6_COMPAREFUNCTION_LEQUAL;
   case PIPE_FUNC_GREATER:    return GEN6_COMPAREFUNCTION_GREATER;
   case PIPE_FUNC_NOTEQUAL:   return GEN6_COMPAREFUNCTION_NOTEQUAL;
   case PIPE_FUNC_GEQUAL:     return GEN6_COMPAREFUNCTION_GEQUAL;
   case PIPE_FUNC_ALWAYS:     return GEN6_COMPAREFUNCTION_ALWAYS;
   default:
      assert(!"unknown depth/stencil/alpha test function");
      return GEN6_COMPAREFUNCTION_NEVER;
   }
}

static uint32_t
dsa_get_stencil_enable_gen6(const struct ilo_dev *dev,
                            const struct pipe_stencil_state *stencil0,
                            const struct pipe_stencil_state *stencil1)
{
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 6, 7.5);

   if (!stencil0->enabled)
      return 0;

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 359:
    *
    *     "If the Depth Buffer is either undefined or does not have a surface
    *      format of D32_FLOAT_S8X24_UINT or D24_UNORM_S8_UINT and separate
    *      stencil buffer is disabled, Stencil Test Enable must be DISABLED"
    *
    * From the Sandy Bridge PRM, volume 2 part 1, page 370:
    *
    *     "This field (Stencil Test Enable) cannot be enabled if
    *      Surface Format in 3DSTATE_DEPTH_BUFFER is set to D16_UNORM."
    *
    * TODO We do not check these yet.
    */
   dw = GEN6_ZS_DW0_STENCIL_TEST_ENABLE |
        gen6_translate_dsa_func(stencil0->func) << 28 |
        gen6_translate_pipe_stencil_op(stencil0->fail_op) << 25 |
        gen6_translate_pipe_stencil_op(stencil0->zfail_op) << 22 |
        gen6_translate_pipe_stencil_op(stencil0->zpass_op) << 19;
   if (stencil0->writemask)
      dw |= GEN6_ZS_DW0_STENCIL_WRITE_ENABLE;

   if (stencil1->enabled) {
      dw |= GEN6_ZS_DW0_STENCIL1_ENABLE |
            gen6_translate_dsa_func(stencil1->func) << 12 |
            gen6_translate_pipe_stencil_op(stencil1->fail_op) << 9 |
            gen6_translate_pipe_stencil_op(stencil1->zfail_op) << 6 |
            gen6_translate_pipe_stencil_op(stencil1->zpass_op) << 3;
      if (stencil1->writemask)
         dw |= GEN6_ZS_DW0_STENCIL_WRITE_ENABLE;
   }

   return dw;
}

static uint32_t
dsa_get_stencil_enable_gen8(const struct ilo_dev *dev,
                            const struct pipe_stencil_state *stencil0,
                            const struct pipe_stencil_state *stencil1)
{
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 8, 8);

   if (!stencil0->enabled)
      return 0;

   dw = gen6_translate_pipe_stencil_op(stencil0->fail_op) << 29 |
        gen6_translate_pipe_stencil_op(stencil0->zfail_op) << 26 |
        gen6_translate_pipe_stencil_op(stencil0->zpass_op) << 23 |
        gen6_translate_dsa_func(stencil0->func) << 8 |
        GEN8_ZS_DW1_STENCIL_TEST_ENABLE;
   if (stencil0->writemask)
      dw |= GEN8_ZS_DW1_STENCIL_WRITE_ENABLE;

   if (stencil1->enabled) {
      dw |= gen6_translate_dsa_func(stencil1->func) << 20 |
            gen6_translate_pipe_stencil_op(stencil1->fail_op) << 17 |
            gen6_translate_pipe_stencil_op(stencil1->zfail_op) << 14 |
            gen6_translate_pipe_stencil_op(stencil1->zpass_op) << 11 |
            GEN8_ZS_DW1_STENCIL1_ENABLE;
      if (stencil1->writemask)
         dw |= GEN8_ZS_DW1_STENCIL_WRITE_ENABLE;
   }

   return dw;
}

static uint32_t
dsa_get_depth_enable_gen6(const struct ilo_dev *dev,
                          const struct pipe_depth_state *state)
{
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 6, 7.5);

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 360:
    *
    *     "Enabling the Depth Test function without defining a Depth Buffer is
    *      UNDEFINED."
    *
    * From the Sandy Bridge PRM, volume 2 part 1, page 375:
    *
    *     "A Depth Buffer must be defined before enabling writes to it, or
    *      operation is UNDEFINED."
    *
    * TODO We do not check these yet.
    */
   if (state->enabled) {
      dw = GEN6_ZS_DW2_DEPTH_TEST_ENABLE |
           gen6_translate_dsa_func(state->func) << 27;
   } else {
      dw = GEN6_COMPAREFUNCTION_ALWAYS << 27;
   }

   if (state->writemask)
      dw |= GEN6_ZS_DW2_DEPTH_WRITE_ENABLE;

   return dw;
}

static uint32_t
dsa_get_depth_enable_gen8(const struct ilo_dev *dev,
                          const struct pipe_depth_state *state)
{
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 8, 8);

   if (state->enabled) {
      dw = GEN8_ZS_DW1_DEPTH_TEST_ENABLE |
           gen6_translate_dsa_func(state->func) << 5;
   } else {
      dw = GEN6_COMPAREFUNCTION_ALWAYS << 5;
   }

   if (state->writemask)
      dw |= GEN8_ZS_DW1_DEPTH_WRITE_ENABLE;

   return dw;
}

static uint32_t
dsa_get_alpha_enable_gen6(const struct ilo_dev *dev,
                          const struct pipe_alpha_state *state)
{
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 6, 7.5);

   if (!state->enabled)
      return 0;

   /* this will be ORed to BLEND_STATE */
   dw = GEN6_RT_DW1_ALPHA_TEST_ENABLE |
        gen6_translate_dsa_func(state->func) << 13;

   return dw;
}

static uint32_t
dsa_get_alpha_enable_gen8(const struct ilo_dev *dev,
                          const struct pipe_alpha_state *state)
{
   uint32_t dw;

   ILO_DEV_ASSERT(dev, 8, 8);

   if (!state->enabled)
      return 0;

   /* this will be ORed to BLEND_STATE */
   dw = GEN8_BLEND_DW0_ALPHA_TEST_ENABLE |
        gen6_translate_dsa_func(state->func) << 24;

   return dw;
}

void
ilo_gpe_init_dsa(const struct ilo_dev *dev,
                 const struct pipe_depth_stencil_alpha_state *state,
                 struct ilo_dsa_state *dsa)
{
   ILO_DEV_ASSERT(dev, 6, 8);

   STATIC_ASSERT(Elements(dsa->payload) >= 3);

   if (ilo_dev_gen(dev) >= ILO_GEN(8)) {
      const uint32_t dw_stencil = dsa_get_stencil_enable_gen8(dev,
            &state->stencil[0], &state->stencil[1]);
      const uint32_t dw_depth = dsa_get_depth_enable_gen8(dev, &state->depth);

      assert(!(dw_stencil & dw_depth));
      dsa->payload[0] = dw_stencil | dw_depth;

      dsa->dw_blend_alpha = dsa_get_alpha_enable_gen8(dev, &state->alpha);
      dsa->dw_ps_blend_alpha = (state->alpha.enabled) ?
         GEN8_PS_BLEND_DW1_ALPHA_TEST_ENABLE : 0;
   } else {
      dsa->payload[0] = dsa_get_stencil_enable_gen6(dev,
            &state->stencil[0], &state->stencil[1]);
      dsa->payload[2] = dsa_get_depth_enable_gen6(dev, &state->depth);

      dsa->dw_blend_alpha = dsa_get_alpha_enable_gen6(dev, &state->alpha);
      dsa->dw_ps_blend_alpha = 0;
   }

   dsa->payload[1] = state->stencil[0].valuemask << 24 |
                     state->stencil[0].writemask << 16 |
                     state->stencil[1].valuemask << 8 |
                     state->stencil[1].writemask;

   dsa->alpha_ref = float_to_ubyte(state->alpha.ref_value);
}

void
ilo_gpe_set_scissor(const struct ilo_dev *dev,
                    unsigned start_slot,
                    unsigned num_states,
                    const struct pipe_scissor_state *states,
                    struct ilo_scissor_state *scissor)
{
   unsigned i;

   ILO_DEV_ASSERT(dev, 6, 8);

   for (i = 0; i < num_states; i++) {
      uint16_t min_x, min_y, max_x, max_y;

      /* both max and min are inclusive in SCISSOR_RECT */
      if (states[i].minx < states[i].maxx &&
          states[i].miny < states[i].maxy) {
         min_x = states[i].minx;
         min_y = states[i].miny;
         max_x = states[i].maxx - 1;
         max_y = states[i].maxy - 1;
      }
      else {
         /* we have to make min greater than max */
         min_x = 1;
         min_y = 1;
         max_x = 0;
         max_y = 0;
      }

      scissor->payload[(start_slot + i) * 2 + 0] = min_y << 16 | min_x;
      scissor->payload[(start_slot + i) * 2 + 1] = max_y << 16 | max_x;
   }

   if (!start_slot && num_states)
      scissor->scissor0 = states[0];
}

void
ilo_gpe_set_scissor_null(const struct ilo_dev *dev,
                         struct ilo_scissor_state *scissor)
{
   unsigned i;

   for (i = 0; i < Elements(scissor->payload); i += 2) {
      scissor->payload[i + 0] = 1 << 16 | 1;
      scissor->payload[i + 1] = 0;
   }
}

static void
fb_set_blend_caps(const struct ilo_dev *dev,
                  enum pipe_format format,
                  struct ilo_fb_blend_caps *caps)
{
   const struct util_format_description *desc =
      util_format_description(format);
   const int ch = util_format_get_first_non_void_channel(format);

   memset(caps, 0, sizeof(*caps));

   if (format == PIPE_FORMAT_NONE || desc->is_mixed)
      return;

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 365:
    *
    *     "Logic Ops are only supported on *_UNORM surfaces (excluding _SRGB
    *      variants), otherwise Logic Ops must be DISABLED."
    *
    * According to the classic driver, this is lifted on Gen8+.
    */
   if (ilo_dev_gen(dev) >= ILO_GEN(8)) {
      caps->can_logicop = true;
   } else {
      caps->can_logicop = (ch >= 0 && desc->channel[ch].normalized &&
            desc->channel[ch].type == UTIL_FORMAT_TYPE_UNSIGNED &&
            desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB);
   }

   /* no blending for pure integer formats */
   caps->can_blend = !util_format_is_pure_integer(format);

   /*
    * From the Sandy Bridge PRM, volume 2 part 1, page 382:
    *
    *     "Alpha Test can only be enabled if Pixel Shader outputs a float
    *      alpha value."
    */
   caps->can_alpha_test = !util_format_is_pure_integer(format);

   caps->dst_alpha_forced_one =
      (ilo_format_translate_render(dev, format) !=
       ilo_format_translate_color(dev, format));

   /* sanity check */
   if (caps->dst_alpha_forced_one) {
      enum pipe_format render_format;

      switch (format) {
      case PIPE_FORMAT_B8G8R8X8_UNORM:
         render_format = PIPE_FORMAT_B8G8R8A8_UNORM;
         break;
      default:
         render_format = PIPE_FORMAT_NONE;
         break;
      }

      assert(ilo_format_translate_render(dev, format) ==
             ilo_format_translate_color(dev, render_format));
   }
}

void
ilo_gpe_set_fb(const struct ilo_dev *dev,
               const struct pipe_framebuffer_state *state,
               struct ilo_fb_state *fb)
{
   const struct pipe_surface *first_surf = NULL;
   int i;

   ILO_DEV_ASSERT(dev, 6, 8);

   util_copy_framebuffer_state(&fb->state, state);

   ilo_gpe_init_view_surface_null(dev,
         (state->width) ? state->width : 1,
         (state->height) ? state->height : 1,
         1, 0, &fb->null_rt);

   for (i = 0; i < state->nr_cbufs; i++) {
      if (state->cbufs[i]) {
         fb_set_blend_caps(dev, state->cbufs[i]->format, &fb->blend_caps[i]);

         if (!first_surf)
            first_surf = state->cbufs[i];
      } else {
         fb_set_blend_caps(dev, PIPE_FORMAT_NONE, &fb->blend_caps[i]);
      }
   }

   if (!first_surf && state->zsbuf)
      first_surf = state->zsbuf;

   fb->num_samples = (first_surf) ? first_surf->texture->nr_samples : 1;
   if (!fb->num_samples)
      fb->num_samples = 1;

   /*
    * The PRMs list several restrictions when the framebuffer has more than
    * one surface.  It seems they are actually lifted on GEN6+.
    */
}
