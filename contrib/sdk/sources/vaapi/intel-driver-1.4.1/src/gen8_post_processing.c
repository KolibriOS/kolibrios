/*
 * Copyright © 2014 Intel Corporation
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
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Xiang Haihao <haihao.xiang@intel.com>
 *    Zhao Yakui <yakui.zhao@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"
#include "i965_defines.h"
#include "i965_structs.h"
#include "i965_drv_video.h"
#include "i965_post_processing.h"
#include "i965_render.h"
#include "intel_media.h"

#define SURFACE_STATE_PADDED_SIZE               SURFACE_STATE_PADDED_SIZE_GEN8

#define SURFACE_STATE_OFFSET(index)             (SURFACE_STATE_PADDED_SIZE * index)
#define BINDING_TABLE_OFFSET                    SURFACE_STATE_OFFSET(MAX_PP_SURFACES)

#define GPU_ASM_BLOCK_WIDTH         16
#define GPU_ASM_BLOCK_HEIGHT        8
#define GPU_ASM_X_OFFSET_ALIGNMENT  4

#define VA_STATUS_SUCCESS_1                     0xFFFFFFFE

static VAStatus pp_null_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                   const struct i965_surface *src_surface,
                                   const VARectangle *src_rect,
                                   struct i965_surface *dst_surface,
                                   const VARectangle *dst_rect,
                                   void *filter_param);

static VAStatus gen8_pp_plx_avs_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                           const struct i965_surface *src_surface,
                                           const VARectangle *src_rect,
                                           struct i965_surface *dst_surface,
                                           const VARectangle *dst_rect,
                                           void *filter_param);

/* TODO: Modify the shader and then compile it again.
 * Currently it is derived from Haswell*/
static const uint32_t pp_null_gen8[][4] = {
};

static const uint32_t pp_nv12_load_save_nv12_gen8[][4] = {
#include "shaders/post_processing/gen8/pl2_to_pl2.g8b"
};

static const uint32_t pp_nv12_load_save_pl3_gen8[][4] = {
#include "shaders/post_processing/gen8/pl2_to_pl3.g8b"
};

static const uint32_t pp_pl3_load_save_nv12_gen8[][4] = {
#include "shaders/post_processing/gen8/pl3_to_pl2.g8b"
};

static const uint32_t pp_pl3_load_save_pl3_gen8[][4] = {
#include "shaders/post_processing/gen8/pl3_to_pl3.g8b"
};

static const uint32_t pp_nv12_scaling_gen8[][4] = {
#include "shaders/post_processing/gen8/pl2_to_pl2.g8b"
};

static const uint32_t pp_nv12_avs_gen8[][4] = {
#include "shaders/post_processing/gen8/pl2_to_pl2.g8b"
};

static const uint32_t pp_nv12_dndi_gen8[][4] = {
// #include "shaders/post_processing/gen7/dndi.g75b"
};

static const uint32_t pp_nv12_dn_gen8[][4] = {
// #include "shaders/post_processing/gen7/nv12_dn_nv12.g75b"
};
static const uint32_t pp_nv12_load_save_pa_gen8[][4] = {
#include "shaders/post_processing/gen8/pl2_to_pa.g8b"
};
static const uint32_t pp_pl3_load_save_pa_gen8[][4] = {
#include "shaders/post_processing/gen8/pl3_to_pa.g8b"
};
static const uint32_t pp_pa_load_save_nv12_gen8[][4] = {
#include "shaders/post_processing/gen8/pa_to_pl2.g8b"
};
static const uint32_t pp_pa_load_save_pl3_gen8[][4] = {
#include "shaders/post_processing/gen8/pa_to_pl3.g8b"
};
static const uint32_t pp_pa_load_save_pa_gen8[][4] = {
#include "shaders/post_processing/gen8/pa_to_pa.g8b"
};
static const uint32_t pp_rgbx_load_save_nv12_gen8[][4] = {
#include "shaders/post_processing/gen8/rgbx_to_nv12.g8b"
};
static const uint32_t pp_nv12_load_save_rgbx_gen8[][4] = {
#include "shaders/post_processing/gen8/pl2_to_rgbx.g8b"
};

static struct pp_module pp_modules_gen8[] = {
    {
        {
            "NULL module (for testing)",
            PP_NULL,
            pp_null_gen8,
            sizeof(pp_null_gen8),
            NULL,
        },

        pp_null_initialize,
    },

    {
        {
            "NV12_NV12",
            PP_NV12_LOAD_SAVE_N12,
            pp_nv12_load_save_nv12_gen8,
            sizeof(pp_nv12_load_save_nv12_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "NV12_PL3",
            PP_NV12_LOAD_SAVE_PL3,
            pp_nv12_load_save_pl3_gen8,
            sizeof(pp_nv12_load_save_pl3_gen8),
            NULL,
        },
        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "PL3_NV12",
            PP_PL3_LOAD_SAVE_N12,
            pp_pl3_load_save_nv12_gen8,
            sizeof(pp_pl3_load_save_nv12_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "PL3_PL3",
            PP_PL3_LOAD_SAVE_N12,
            pp_pl3_load_save_pl3_gen8,
            sizeof(pp_pl3_load_save_pl3_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "NV12 Scaling module",
            PP_NV12_SCALING,
            pp_nv12_scaling_gen8,
            sizeof(pp_nv12_scaling_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "NV12 AVS module",
            PP_NV12_AVS,
            pp_nv12_avs_gen8,
            sizeof(pp_nv12_avs_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "NV12 DNDI module",
            PP_NV12_DNDI,
            pp_nv12_dndi_gen8,
            sizeof(pp_nv12_dndi_gen8),
            NULL,
        },

        pp_null_initialize,
    },

    {
        {
            "NV12 DN module",
            PP_NV12_DN,
            pp_nv12_dn_gen8,
            sizeof(pp_nv12_dn_gen8),
            NULL,
        },

        pp_null_initialize,
    },
    {
        {
            "NV12_PA module",
            PP_NV12_LOAD_SAVE_PA,
            pp_nv12_load_save_pa_gen8,
            sizeof(pp_nv12_load_save_pa_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "PL3_PA module",
            PP_PL3_LOAD_SAVE_PA,
            pp_pl3_load_save_pa_gen8,
            sizeof(pp_pl3_load_save_pa_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "PA_NV12 module",
            PP_PA_LOAD_SAVE_NV12,
            pp_pa_load_save_nv12_gen8,
            sizeof(pp_pa_load_save_nv12_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "PA_PL3 module",
            PP_PA_LOAD_SAVE_PL3,
            pp_pa_load_save_pl3_gen8,
            sizeof(pp_pa_load_save_pl3_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "PA_PA module",
            PP_PA_LOAD_SAVE_PA,
            pp_pa_load_save_pa_gen8,
            sizeof(pp_pa_load_save_pa_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "RGBX_NV12 module",
            PP_RGBX_LOAD_SAVE_NV12,
            pp_rgbx_load_save_nv12_gen8,
            sizeof(pp_rgbx_load_save_nv12_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },

    {
        {
            "NV12_RGBX module",
            PP_NV12_LOAD_SAVE_RGBX,
            pp_nv12_load_save_rgbx_gen8,
            sizeof(pp_nv12_load_save_rgbx_gen8),
            NULL,
        },

        gen8_pp_plx_avs_initialize,
    },
};

static int
pp_get_surface_fourcc(VADriverContextP ctx, const struct i965_surface *surface)
{
    int fourcc;

    if (surface->type == I965_SURFACE_TYPE_IMAGE) {
        struct object_image *obj_image = (struct object_image *)surface->base;
        fourcc = obj_image->image.format.fourcc;
    } else {
        struct object_surface *obj_surface = (struct object_surface *)surface->base;
        fourcc = obj_surface->fourcc;
    }

    return fourcc;
}

static void
gen8_pp_set_surface_tiling(struct gen8_surface_state *ss, unsigned int tiling)
{
    switch (tiling) {
    case I915_TILING_NONE:
        ss->ss0.tiled_surface = 0;
        ss->ss0.tile_walk = 0;
        break;
    case I915_TILING_X:
        ss->ss0.tiled_surface = 1;
        ss->ss0.tile_walk = I965_TILEWALK_XMAJOR;
        break;
    case I915_TILING_Y:
        ss->ss0.tiled_surface = 1;
        ss->ss0.tile_walk = I965_TILEWALK_YMAJOR;
        break;
    }
}

static void
gen8_pp_set_surface2_tiling(struct gen8_surface_state2 *ss, unsigned int tiling)
{
    switch (tiling) {
    case I915_TILING_NONE:
        ss->ss2.tiled_surface = 0;
        ss->ss2.tile_walk = 0;
        break;
    case I915_TILING_X:
        ss->ss2.tiled_surface = 1;
        ss->ss2.tile_walk = I965_TILEWALK_XMAJOR;
        break;
    case I915_TILING_Y:
        ss->ss2.tiled_surface = 1;
        ss->ss2.tile_walk = I965_TILEWALK_YMAJOR;
        break;
    }
}


static void
gen8_pp_set_surface_state(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                          dri_bo *surf_bo, unsigned long surf_bo_offset,
                          int width, int height, int pitch, int format,
			  int index, int is_target)
{
    struct gen8_surface_state *ss;
    dri_bo *ss_bo;
    unsigned int tiling;
    unsigned int swizzle;

    dri_bo_get_tiling(surf_bo, &tiling, &swizzle);
    ss_bo = pp_context->surface_state_binding_table.bo;
    assert(ss_bo);

    dri_bo_map(ss_bo, True);
    assert(ss_bo->virtual);
    ss = (struct gen8_surface_state *)((char *)ss_bo->virtual + SURFACE_STATE_OFFSET(index));
    memset(ss, 0, sizeof(*ss));
    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = format;
    ss->ss8.base_addr = surf_bo->offset + surf_bo_offset;
    ss->ss2.width = width - 1;
    ss->ss2.height = height - 1;
    ss->ss3.pitch = pitch - 1;

    /* Always set 1(align 4 mode) per B-spec */
    ss->ss0.vertical_alignment = 1;
    ss->ss0.horizontal_alignment = 1;

    gen8_pp_set_surface_tiling(ss, tiling);
    gen8_render_set_surface_scs(ss);
    dri_bo_emit_reloc(ss_bo,
                      I915_GEM_DOMAIN_RENDER, is_target ? I915_GEM_DOMAIN_RENDER : 0,
                      surf_bo_offset,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct gen8_surface_state, ss8),
                      surf_bo);
    ((unsigned int *)((char *)ss_bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(ss_bo);
}


static void
gen8_pp_set_surface2_state(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                           dri_bo *surf_bo, unsigned long surf_bo_offset,
                           int width, int height, int wpitch,
                           int xoffset, int yoffset,
                           int format, int interleave_chroma,
                           int index)
{
    struct gen8_surface_state2 *ss2;
    dri_bo *ss2_bo;
    unsigned int tiling;
    unsigned int swizzle;

    dri_bo_get_tiling(surf_bo, &tiling, &swizzle);
    ss2_bo = pp_context->surface_state_binding_table.bo;
    assert(ss2_bo);

    dri_bo_map(ss2_bo, True);
    assert(ss2_bo->virtual);
    ss2 = (struct gen8_surface_state2 *)((char *)ss2_bo->virtual + SURFACE_STATE_OFFSET(index));
    memset(ss2, 0, sizeof(*ss2));
    ss2->ss6.base_addr = surf_bo->offset + surf_bo_offset;
    ss2->ss1.cbcr_pixel_offset_v_direction = 0;
    ss2->ss1.width = width - 1;
    ss2->ss1.height = height - 1;
    ss2->ss2.pitch = wpitch - 1;
    ss2->ss2.interleave_chroma = interleave_chroma;
    ss2->ss2.surface_format = format;
    ss2->ss3.x_offset_for_cb = xoffset;
    ss2->ss3.y_offset_for_cb = yoffset;
    gen8_pp_set_surface2_tiling(ss2, tiling);
    dri_bo_emit_reloc(ss2_bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      surf_bo_offset,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct gen8_surface_state2, ss6),
                      surf_bo);
    ((unsigned int *)((char *)ss2_bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(ss2_bo);
}

static void
gen8_pp_set_media_rw_message_surface(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                     const struct i965_surface *surface,
                                     int base_index, int is_target,
                                     const VARectangle *rect,
                                     int *width, int *height, int *pitch, int *offset)
{
    struct object_surface *obj_surface;
    struct object_image *obj_image;
    dri_bo *bo;
    int fourcc = pp_get_surface_fourcc(ctx, surface);
    const i965_fourcc_info *fourcc_info = get_fourcc_info(fourcc);

    if (fourcc_info == NULL)
        return;

    if (surface->type == I965_SURFACE_TYPE_SURFACE) {
        obj_surface = (struct object_surface *)surface->base;
        bo = obj_surface->bo;
        width[0] = MIN(rect->x + rect->width, obj_surface->orig_width);
        height[0] = MIN(rect->y + rect->height, obj_surface->orig_height);
        pitch[0] = obj_surface->width;
        offset[0] = 0;

        if (fourcc_info->num_planes == 1 && is_target)
            width[0] = width[0] * (fourcc_info->bpp[0] / 8); /* surface format is R8 */

        width[1] = MIN(rect->x / fourcc_info->hfactor + rect->width / fourcc_info->hfactor, obj_surface->cb_cr_width);
        height[1] = MIN(rect->y / fourcc_info->vfactor + rect->height / fourcc_info->vfactor, obj_surface->cb_cr_height);
        pitch[1] = obj_surface->cb_cr_pitch;
        offset[1] = obj_surface->y_cb_offset * obj_surface->width;

        width[2] = MIN(rect->x / fourcc_info->hfactor + rect->width / fourcc_info->hfactor, obj_surface->cb_cr_width);
        height[2] = MIN(rect->y / fourcc_info->vfactor + rect->height / fourcc_info->vfactor, obj_surface->cb_cr_height);
        pitch[2] = obj_surface->cb_cr_pitch;
        offset[2] = obj_surface->y_cr_offset * obj_surface->width;
    } else {
        int U = 0, V = 0;

        /* FIXME: add support for ARGB/ABGR image */
        obj_image = (struct object_image *)surface->base;
        bo = obj_image->bo;
        width[0] = MIN(rect->x + rect->width, obj_image->image.width);
        height[0] = MIN(rect->y + rect->height, obj_image->image.height);
        pitch[0] = obj_image->image.pitches[0];
        offset[0] = obj_image->image.offsets[0];

        if (fourcc_info->num_planes == 1) {
            if (is_target)
                width[0] = width[0] * (fourcc_info->bpp[0] / 8); /* surface format is R8 */
        } else if (fourcc_info->num_planes == 2) {
            U = 1, V = 1;
        } else {
            assert(fourcc_info->num_components == 3);

            U = fourcc_info->components[1].plane;
            V = fourcc_info->components[2].plane;
            assert((U == 1 && V == 2) ||
                   (U == 2 && V == 1));
        }

        /* Always set width/height although they aren't used for fourcc_info->num_planes == 1 */
        width[1] = MIN(rect->x / fourcc_info->hfactor + rect->width / fourcc_info->hfactor, obj_image->image.width / fourcc_info->hfactor);
        height[1] = MIN(rect->y / fourcc_info->vfactor + rect->height / fourcc_info->vfactor, obj_image->image.height / fourcc_info->vfactor);
        pitch[1] = obj_image->image.pitches[U];
        offset[1] = obj_image->image.offsets[U];

        width[2] = MIN(rect->x / fourcc_info->hfactor + rect->width / fourcc_info->hfactor, obj_image->image.width / fourcc_info->hfactor);
        height[2] = MIN(rect->y / fourcc_info->vfactor + rect->height / fourcc_info->vfactor, obj_image->image.height / fourcc_info->vfactor);
        pitch[2] = obj_image->image.pitches[V];
        offset[2] = obj_image->image.offsets[V];
    }

    if (is_target) {
        gen8_pp_set_surface_state(ctx, pp_context,
                                  bo, 0,
                                  width[0] / 4, height[0], pitch[0],
                                  I965_SURFACEFORMAT_R8_UINT,
                                  base_index, 1);

        if (fourcc_info->num_planes == 2) {
            gen8_pp_set_surface_state(ctx, pp_context,
                                      bo, offset[1],
                                      width[1] / 2, height[1], pitch[1],
                                      I965_SURFACEFORMAT_R8G8_SINT,
                                      base_index + 1, 1);
        } else if (fourcc_info->num_planes == 3) {
            gen8_pp_set_surface_state(ctx, pp_context,
                                      bo, offset[1],
                                      width[1] / 4, height[1], pitch[1],
                                      I965_SURFACEFORMAT_R8_SINT,
                                      base_index + 1, 1);
            gen8_pp_set_surface_state(ctx, pp_context,
                                      bo, offset[2],
                                      width[2] / 4, height[2], pitch[2],
                                      I965_SURFACEFORMAT_R8_SINT,
                                      base_index + 2, 1);
        }

        if (fourcc_info->format == I965_COLOR_RGB) {
            struct gen7_pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
            /* the format is MSB: X-B-G-R */
            pp_static_parameter->grf2.save_avs_rgb_swap = 0;
            if ((fourcc == VA_FOURCC_BGRA) ||
                (fourcc == VA_FOURCC_BGRX)) {
                /* It is stored as MSB: X-R-G-B */
                pp_static_parameter->grf2.save_avs_rgb_swap = 1;
            }
        }
    } else {
        int format0 = SURFACE_FORMAT_Y8_UNORM;

        switch (fourcc) {
        case VA_FOURCC_YUY2:
            format0 = SURFACE_FORMAT_YCRCB_NORMAL;
            break;

        case VA_FOURCC_UYVY:
            format0 = SURFACE_FORMAT_YCRCB_SWAPY;
            break;

        default:
            break;
        }

	if (fourcc_info->format == I965_COLOR_RGB) {
    	    struct gen7_pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
	    /* Only R8G8B8A8_UNORM is supported for BGRX or RGBX */
	    format0 = SURFACE_FORMAT_R8G8B8A8_UNORM;
	    pp_static_parameter->grf2.src_avs_rgb_swap = 0;
	    if ((fourcc == VA_FOURCC_BGRA) ||
                (fourcc == VA_FOURCC_BGRX)) {
		pp_static_parameter->grf2.src_avs_rgb_swap = 1;
	    }
	}

        gen8_pp_set_surface2_state(ctx, pp_context,
                                   bo, offset[0],
                                   width[0], height[0], pitch[0],
                                   0, 0,
                                   format0, 0,
                                   base_index);

        if (fourcc_info->num_planes == 2) {
            gen8_pp_set_surface2_state(ctx, pp_context,
                                       bo, offset[1],
                                       width[1], height[1], pitch[1],
                                       0, 0,
                                       SURFACE_FORMAT_R8B8_UNORM, 0,
                                       base_index + 1);
        } else if (fourcc_info->num_planes == 3) {
            gen8_pp_set_surface2_state(ctx, pp_context,
                                       bo, offset[1],
                                       width[1], height[1], pitch[1],
                                       0, 0,
                                       SURFACE_FORMAT_R8_UNORM, 0,
                                       base_index + 1);
            gen8_pp_set_surface2_state(ctx, pp_context,
                                       bo, offset[2],
                                       width[2], height[2], pitch[2],
                                       0, 0,
                                       SURFACE_FORMAT_R8_UNORM, 0,
                                       base_index + 2);
        }
    }
}

static int
pp_null_x_steps(void *private_context)
{
    return 1;
}

static int
pp_null_y_steps(void *private_context)
{
    return 1;
}

static int
pp_null_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    return 0;
}

static VAStatus
pp_null_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                   const struct i965_surface *src_surface,
                   const VARectangle *src_rect,
                   struct i965_surface *dst_surface,
                   const VARectangle *dst_rect,
                   void *filter_param)
{
    /* private function & data */
    pp_context->pp_x_steps = pp_null_x_steps;
    pp_context->pp_y_steps = pp_null_y_steps;
    pp_context->private_context = NULL;
    pp_context->pp_set_block_parameter = pp_null_set_block_parameter;

    dst_surface->flags = src_surface->flags;

    return VA_STATUS_SUCCESS;
}

static void calculate_boundary_block_mask(struct i965_post_processing_context *pp_context, const VARectangle *dst_rect)
{
    int i, dst_width_adjust;
    /* x offset of dest surface must be dword aligned.
     * so we have to extend dst surface on left edge, and mask out pixels not interested
     */
    if (dst_rect->x%GPU_ASM_X_OFFSET_ALIGNMENT) {
        pp_context->block_horizontal_mask_left = 0;
        for (i=dst_rect->x%GPU_ASM_X_OFFSET_ALIGNMENT; i<GPU_ASM_BLOCK_WIDTH; i++)
        {
            pp_context->block_horizontal_mask_left |= 1<<i;
        }
    }
    else {
        pp_context->block_horizontal_mask_left = 0xffff;
    }

    dst_width_adjust = dst_rect->width + dst_rect->x%GPU_ASM_X_OFFSET_ALIGNMENT;
    if (dst_width_adjust%GPU_ASM_BLOCK_WIDTH){
        pp_context->block_horizontal_mask_right = (1 << (dst_width_adjust%GPU_ASM_BLOCK_WIDTH)) - 1;
    }
    else {
        pp_context->block_horizontal_mask_right = 0xffff;
    }

    if (dst_rect->height%GPU_ASM_BLOCK_HEIGHT){
        pp_context->block_vertical_mask_bottom = (1 << (dst_rect->height%GPU_ASM_BLOCK_HEIGHT)) - 1;
    }
    else {
        pp_context->block_vertical_mask_bottom = 0xff;
    }

}

static int
gen7_pp_avs_x_steps(void *private_context)
{
    struct pp_avs_context *pp_avs_context = private_context;

    return pp_avs_context->dest_w / 16;
}

static int
gen7_pp_avs_y_steps(void *private_context)
{
    struct pp_avs_context *pp_avs_context = private_context;

    return pp_avs_context->dest_h / 16;
}

static int
gen7_pp_avs_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    struct pp_avs_context *pp_avs_context = (struct pp_avs_context *)pp_context->private_context;
    struct gen7_pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;

    pp_inline_parameter->grf7.destination_block_horizontal_origin = x * 16 + pp_avs_context->dest_x;
    pp_inline_parameter->grf7.destination_block_vertical_origin = y * 16 + pp_avs_context->dest_y;
    pp_inline_parameter->grf7.constant_0 = 0xffffffff;
    pp_inline_parameter->grf7.sampler_load_main_video_x_scaling_step = pp_avs_context->horiz_range / pp_avs_context->src_w;

    return 0;
}

static void gen7_update_src_surface_uv_offset(VADriverContextP    ctx,
                                              struct i965_post_processing_context *pp_context,
                                              const struct i965_surface *surface)
{
    struct gen7_pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    int fourcc = pp_get_surface_fourcc(ctx, surface);

    if (fourcc == VA_FOURCC_YUY2) {
        pp_static_parameter->grf2.di_destination_packed_y_component_offset = 0;
        pp_static_parameter->grf2.di_destination_packed_u_component_offset = 1;
        pp_static_parameter->grf2.di_destination_packed_v_component_offset = 3;
    } else if (fourcc == VA_FOURCC_UYVY) {
        pp_static_parameter->grf2.di_destination_packed_y_component_offset = 1;
        pp_static_parameter->grf2.di_destination_packed_u_component_offset = 0;
        pp_static_parameter->grf2.di_destination_packed_v_component_offset = 2;
    }
}

static VAStatus
gen8_pp_plx_avs_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                           const struct i965_surface *src_surface,
                           const VARectangle *src_rect,
                           struct i965_surface *dst_surface,
                           const VARectangle *dst_rect,
                           void *filter_param)
{
/* TODO: Add the sampler_8x8 state */
    struct pp_avs_context *pp_avs_context = (struct pp_avs_context *)&pp_context->pp_avs_context;
    struct gen7_pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    struct gen8_sampler_8x8_avs *sampler_8x8;
    struct i965_sampler_8x8_coefficient *sampler_8x8_state;
    int i;
    int width[3], height[3], pitch[3], offset[3];
    int src_width, src_height;
    unsigned char *cc_ptr;

    memset(pp_static_parameter, 0, sizeof(struct gen7_pp_static_parameter));

    /* source surface */
    gen8_pp_set_media_rw_message_surface(ctx, pp_context, src_surface, 0, 0,
                                         src_rect,
                                         width, height, pitch, offset);
    src_height = height[0];
    src_width  = width[0];

    /* destination surface */
    gen8_pp_set_media_rw_message_surface(ctx, pp_context, dst_surface, 24, 1,
                                         dst_rect,
                                         width, height, pitch, offset);

    /* sampler 8x8 state */
    dri_bo_map(pp_context->dynamic_state.bo, True);
    assert(pp_context->dynamic_state.bo->virtual);

    cc_ptr = (unsigned char *) pp_context->dynamic_state.bo->virtual +
			pp_context->sampler_offset;
    /* Currently only one gen8 sampler_8x8 is initialized */
    sampler_8x8 = (struct gen8_sampler_8x8_avs *) cc_ptr;
    memset(sampler_8x8, 0, sizeof(*sampler_8x8));

    sampler_8x8->dw0.gain_factor = 44;
    sampler_8x8->dw0.weak_edge_threshold = 1;
    sampler_8x8->dw0.strong_edge_threshold = 8;
    /* Use the value like that on Ivy instead of default
     * sampler_8x8->dw0.r3x_coefficient = 5;
     */
    sampler_8x8->dw0.r3x_coefficient = 27;
    sampler_8x8->dw0.r3c_coefficient = 5;

    sampler_8x8->dw2.global_noise_estimation = 255;
    sampler_8x8->dw2.non_edge_weight = 1;
    sampler_8x8->dw2.regular_weight = 2;
    sampler_8x8->dw2.strong_edge_weight = 7;
    /* Use the value like that on Ivy instead of default
     * sampler_8x8->dw2.r5x_coefficient = 7;
     * sampler_8x8->dw2.r5cx_coefficient = 7;
     * sampler_8x8->dw2.r5c_coefficient = 7;
     */
    sampler_8x8->dw2.r5x_coefficient = 9;
    sampler_8x8->dw2.r5cx_coefficient = 8;
    sampler_8x8->dw2.r5c_coefficient = 3;

    sampler_8x8->dw3.sin_alpha = 101; /* sin_alpha = 0 */
    sampler_8x8->dw3.cos_alpha = 79; /* cos_alpha = 0 */
    sampler_8x8->dw3.sat_max = 0x1f;
    sampler_8x8->dw3.hue_max = 14;
    /* The 8tap filter will determine whether the adaptive Filter is
     * applied for all channels(dw153).
     * If the 8tap filter is disabled, the adaptive filter should be disabled.
     * Only when 8tap filter is enabled, it can be enabled or not.
     */
    sampler_8x8->dw3.enable_8tap_filter = 3;
    sampler_8x8->dw3.ief4_smooth_enable = 0;

    sampler_8x8->dw4.s3u = 0;
    sampler_8x8->dw4.diamond_margin = 4;
    sampler_8x8->dw4.vy_std_enable = 0;
    sampler_8x8->dw4.umid = 110;
    sampler_8x8->dw4.vmid = 154;

    sampler_8x8->dw5.diamond_dv = 0;
    sampler_8x8->dw5.diamond_th = 35;
    sampler_8x8->dw5.diamond_alpha = 100; /* diamond_alpha = 0 */
    sampler_8x8->dw5.hs_margin = 3;
    sampler_8x8->dw5.diamond_du = 2;

    sampler_8x8->dw6.y_point1 = 46;
    sampler_8x8->dw6.y_point2 = 47;
    sampler_8x8->dw6.y_point3 = 254;
    sampler_8x8->dw6.y_point4 = 255;

    sampler_8x8->dw7.inv_margin_vyl = 3300; /* inv_margin_vyl = 0 */

    sampler_8x8->dw8.inv_margin_vyu = 1600; /* inv_margin_vyu = 0 */
    sampler_8x8->dw8.p0l = 46;
    sampler_8x8->dw8.p1l = 216;

    sampler_8x8->dw9.p2l = 236;
    sampler_8x8->dw9.p3l = 236;
    sampler_8x8->dw9.b0l = 133;
    sampler_8x8->dw9.b1l = 130;

    sampler_8x8->dw10.b2l = 130;
    sampler_8x8->dw10.b3l = 130;
    /* s0l = -5 / 256. s2.8 */
    sampler_8x8->dw10.s0l = 1029;    /* s0l = 0 */
    sampler_8x8->dw10.y_slope2 = 31; /* y_slop2 = 0 */

    sampler_8x8->dw11.s1l = 0;
    sampler_8x8->dw11.s2l = 0;

    sampler_8x8->dw12.s3l = 0;
    sampler_8x8->dw12.p0u = 46;
    sampler_8x8->dw12.p1u = 66;
    sampler_8x8->dw12.y_slope1 = 31; /* y_slope1 = 0 */

    sampler_8x8->dw13.p2u = 130;
    sampler_8x8->dw13.p3u = 236;
    sampler_8x8->dw13.b0u = 143;
    sampler_8x8->dw13.b1u = 163;

    sampler_8x8->dw14.b2u = 200;
    sampler_8x8->dw14.b3u = 140;
    sampler_8x8->dw14.s0u = 256;  /* s0u = 0 */

    sampler_8x8->dw15.s1u = 113; /* s1u = 0 */
    sampler_8x8->dw15.s2u = 1203; /* s2u = 0 */

    sampler_8x8_state = sampler_8x8->coefficients;

    for (i = 0; i < 17; i++) {
	float coff;
	coff = i;
	coff = coff / 16;

        memset(sampler_8x8_state, 0, sizeof(*sampler_8x8_state));
        /* for Y channel, currently ignore */
        sampler_8x8_state->dw0.table_0x_filter_c0 = 0x0;
        sampler_8x8_state->dw0.table_0x_filter_c1 = 0x0;
        sampler_8x8_state->dw0.table_0x_filter_c2 = 0x0;
        sampler_8x8_state->dw0.table_0x_filter_c3 =
				intel_format_convert(1 - coff, 1, 6, 0);
        sampler_8x8_state->dw1.table_0x_filter_c4 =
				intel_format_convert(coff, 1, 6, 0);
        sampler_8x8_state->dw1.table_0x_filter_c5 = 0x0;
        sampler_8x8_state->dw1.table_0x_filter_c6 = 0x0;
        sampler_8x8_state->dw1.table_0x_filter_c7 = 0x0;
        sampler_8x8_state->dw2.table_0y_filter_c0 = 0x0;
        sampler_8x8_state->dw2.table_0y_filter_c1 = 0x0;
        sampler_8x8_state->dw2.table_0y_filter_c2 = 0x0;
        sampler_8x8_state->dw2.table_0y_filter_c3 =
                                intel_format_convert(1 - coff, 1, 6, 0);
        sampler_8x8_state->dw3.table_0y_filter_c4 =
                                intel_format_convert(coff, 1, 6, 0);
        sampler_8x8_state->dw3.table_0y_filter_c5 = 0x0;
        sampler_8x8_state->dw3.table_0y_filter_c6 = 0x0;
        sampler_8x8_state->dw3.table_0y_filter_c7 = 0x0;
        /* for U/V channel, 0.25 */
        sampler_8x8_state->dw4.table_1x_filter_c0 = 0x0;
        sampler_8x8_state->dw4.table_1x_filter_c1 = 0x0;
        sampler_8x8_state->dw4.table_1x_filter_c2 = 0x0;
        sampler_8x8_state->dw4.table_1x_filter_c3 =
				intel_format_convert(1 - coff, 1, 6, 0);
        sampler_8x8_state->dw5.table_1x_filter_c4 =
                                intel_format_convert(coff, 1, 6, 0);
        sampler_8x8_state->dw5.table_1x_filter_c5 = 0x00;
        sampler_8x8_state->dw5.table_1x_filter_c6 = 0x0;
        sampler_8x8_state->dw5.table_1x_filter_c7 = 0x0;
        sampler_8x8_state->dw6.table_1y_filter_c0 = 0x0;
        sampler_8x8_state->dw6.table_1y_filter_c1 = 0x0;
        sampler_8x8_state->dw6.table_1y_filter_c2 = 0x0;
        sampler_8x8_state->dw6.table_1y_filter_c3 =
				intel_format_convert(1 - coff, 1, 6, 0);
        sampler_8x8_state->dw7.table_1y_filter_c4 =
				intel_format_convert(coff, 1, 6,0);
        sampler_8x8_state->dw7.table_1y_filter_c5 = 0x0;
        sampler_8x8_state->dw7.table_1y_filter_c6 = 0x0;
        sampler_8x8_state->dw7.table_1y_filter_c7 = 0x0;
        sampler_8x8_state++;
    }

    sampler_8x8->dw152.default_sharpness_level = 0;
    sampler_8x8->dw153.adaptive_filter_for_all_channel = 1;
    sampler_8x8->dw153.bypass_y_adaptive_filtering = 1;
    sampler_8x8->dw153.bypass_x_adaptive_filtering = 1;

    dri_bo_unmap(pp_context->dynamic_state.bo);


    /* private function & data */
    pp_context->pp_x_steps = gen7_pp_avs_x_steps;
    pp_context->pp_y_steps = gen7_pp_avs_y_steps;
    pp_context->private_context = &pp_context->pp_avs_context;
    pp_context->pp_set_block_parameter = gen7_pp_avs_set_block_parameter;

    pp_avs_context->dest_x = dst_rect->x;
    pp_avs_context->dest_y = dst_rect->y;
    pp_avs_context->dest_w = ALIGN(dst_rect->width, 16);
    pp_avs_context->dest_h = ALIGN(dst_rect->height, 16);
    pp_avs_context->src_w = src_rect->width;
    pp_avs_context->src_h = src_rect->height;
    pp_avs_context->horiz_range = (float)src_rect->width / src_width;

    int dw = (pp_avs_context->src_w - 1) / 16 + 1;
    dw = MAX(dw, dst_rect->width);

    pp_static_parameter->grf1.pointer_to_inline_parameter = 7;
    pp_static_parameter->grf2.avs_wa_enable = 0; /* It is not required on GEN8+ */
    pp_static_parameter->grf2.alpha = 255;

    pp_static_parameter->grf3.sampler_load_horizontal_scaling_step_ratio = (float) pp_avs_context->src_w / dw;
    pp_static_parameter->grf4.sampler_load_vertical_scaling_step = (float) src_rect->height / src_height / dst_rect->height;
    pp_static_parameter->grf5.sampler_load_vertical_frame_origin = (float) src_rect->y / src_height -
        (float) pp_avs_context->dest_y * pp_static_parameter->grf4.sampler_load_vertical_scaling_step;
    pp_static_parameter->grf6.sampler_load_horizontal_frame_origin = (float) src_rect->x / src_width -
        (float) pp_avs_context->dest_x * pp_avs_context->horiz_range / dw;

    gen7_update_src_surface_uv_offset(ctx, pp_context, dst_surface);

    dst_surface->flags = src_surface->flags;

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen8_pp_initialize(
    VADriverContextP   ctx,
    struct i965_post_processing_context *pp_context,
    const struct i965_surface *src_surface,
    const VARectangle *src_rect,
    struct i965_surface *dst_surface,
    const VARectangle *dst_rect,
    int                pp_index,
    void * filter_param
)
{
    VAStatus va_status;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    dri_bo *bo;
    int bo_size;
    unsigned int end_offset;
    struct pp_module *pp_module;
    int static_param_size, inline_param_size;

    dri_bo_unreference(pp_context->surface_state_binding_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      (SURFACE_STATE_PADDED_SIZE + sizeof(unsigned int)) * MAX_PP_SURFACES,
                      4096);
    assert(bo);
    pp_context->surface_state_binding_table.bo = bo;

    pp_context->idrt.num_interface_descriptors = 0;

    pp_context->sampler_size = 2 * 4096;

    bo_size = 4096 + pp_context->curbe_size + pp_context->sampler_size
		+ pp_context->idrt_size;

    dri_bo_unreference(pp_context->dynamic_state.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "dynamic_state",
                      bo_size,
		      4096);

    assert(bo);
    pp_context->dynamic_state.bo = bo;
    pp_context->dynamic_state.bo_size = bo_size;

    end_offset = 0;
    pp_context->dynamic_state.end_offset = 0;

    /* Constant buffer offset */
    pp_context->curbe_offset = ALIGN(end_offset, 64);
    end_offset = pp_context->curbe_offset + pp_context->curbe_size;

    /* Interface descriptor offset */
    pp_context->idrt_offset = ALIGN(end_offset, 64);
    end_offset = pp_context->idrt_offset + pp_context->idrt_size;

    /* Sampler state offset */
    pp_context->sampler_offset = ALIGN(end_offset, 64);
    end_offset = pp_context->sampler_offset + pp_context->sampler_size;

    /* update the end offset of dynamic_state */
    pp_context->dynamic_state.end_offset = ALIGN(end_offset, 64);

    static_param_size = sizeof(struct gen7_pp_static_parameter);
    inline_param_size = sizeof(struct gen7_pp_inline_parameter);

    memset(pp_context->pp_static_parameter, 0, static_param_size);
    memset(pp_context->pp_inline_parameter, 0, inline_param_size);

    assert(pp_index >= PP_NULL && pp_index < NUM_PP_MODULES);
    pp_context->current_pp = pp_index;
    pp_module = &pp_context->pp_modules[pp_index];

    if (pp_module->initialize)
        va_status = pp_module->initialize(ctx, pp_context,
                                          src_surface,
                                          src_rect,
                                          dst_surface,
                                          dst_rect,
                                          filter_param);
    else
        va_status = VA_STATUS_ERROR_UNIMPLEMENTED;

    calculate_boundary_block_mask(pp_context, dst_rect);

    return va_status;
}

static void
gen8_pp_interface_descriptor_table(VADriverContextP   ctx,
                                   struct i965_post_processing_context *pp_context)
{
    struct gen8_interface_descriptor_data *desc;
    dri_bo *bo;
    int pp_index = pp_context->current_pp;
    unsigned char *cc_ptr;

    bo = pp_context->dynamic_state.bo;

    dri_bo_map(bo, 1);
    assert(bo->virtual);
    cc_ptr = (unsigned char *)bo->virtual + pp_context->idrt_offset;

    desc = (struct gen8_interface_descriptor_data *) cc_ptr +
		pp_context->idrt.num_interface_descriptors;

    memset(desc, 0, sizeof(*desc));
    desc->desc0.kernel_start_pointer =
                pp_context->pp_modules[pp_index].kernel.kernel_offset >> 6; /* reloc */
    desc->desc2.single_program_flow = 1;
    desc->desc2.floating_point_mode = FLOATING_POINT_IEEE_754;
    desc->desc3.sampler_count = 0;      /* 1 - 4 samplers used */
    desc->desc3.sampler_state_pointer = pp_context->sampler_offset >> 5;
    desc->desc4.binding_table_entry_count = 0;
    desc->desc4.binding_table_pointer = (BINDING_TABLE_OFFSET >> 5);
    desc->desc5.constant_urb_entry_read_offset = 0;

    desc->desc5.constant_urb_entry_read_length = 6; /* grf 1-6 */

    dri_bo_unmap(bo);
    pp_context->idrt.num_interface_descriptors++;
}


static void
gen8_pp_upload_constants(VADriverContextP ctx,
                         struct i965_post_processing_context *pp_context)
{
    unsigned char *constant_buffer;
    int param_size;

    assert(sizeof(struct gen7_pp_static_parameter) == 192);

    param_size = sizeof(struct gen7_pp_static_parameter);

    dri_bo_map(pp_context->dynamic_state.bo, 1);
    assert(pp_context->dynamic_state.bo->virtual);
    constant_buffer = (unsigned char *) pp_context->dynamic_state.bo->virtual +
			pp_context->curbe_offset;

    memcpy(constant_buffer, pp_context->pp_static_parameter, param_size);
    dri_bo_unmap(pp_context->dynamic_state.bo);
    return;
}

static void
gen8_pp_states_setup(VADriverContextP ctx,
                     struct i965_post_processing_context *pp_context)
{
    gen8_pp_interface_descriptor_table(ctx, pp_context);
    gen8_pp_upload_constants(ctx, pp_context);
}

static void
gen6_pp_pipeline_select(VADriverContextP ctx,
                        struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 1);
    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_MEDIA);
    ADVANCE_BATCH(batch);
}

static void
gen8_pp_state_base_address(VADriverContextP ctx,
                           struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 16);
    OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | (16 - 2));
	/* DW1 Generate state address */
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
	OUT_BATCH(batch, 0);
	OUT_BATCH(batch, 0);
	/* DW4. Surface state address */
    OUT_RELOC(batch, pp_context->surface_state_binding_table.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY); /* Surface state base address */
	OUT_BATCH(batch, 0);
	/* DW6. Dynamic state address */
    OUT_RELOC(batch, pp_context->dynamic_state.bo, I915_GEM_DOMAIN_RENDER | I915_GEM_DOMAIN_SAMPLER,
		0, 0 | BASE_ADDRESS_MODIFY);
	OUT_BATCH(batch, 0);

	/* DW8. Indirect object address */
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
	OUT_BATCH(batch, 0);

	/* DW10. Instruction base address */
    OUT_RELOC(batch, pp_context->instruction_state.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY);
	OUT_BATCH(batch, 0);

    OUT_BATCH(batch, 0xFFFF0000 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0xFFFF0000 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0xFFFF0000 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0xFFFF0000 | BASE_ADDRESS_MODIFY);
    ADVANCE_BATCH(batch);
}

static void
gen8_pp_vfe_state(VADriverContextP ctx,
                  struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 9);
    OUT_BATCH(batch, CMD_MEDIA_VFE_STATE | (9 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch,
              (pp_context->vfe_gpu_state.max_num_threads - 1) << 16 |
              pp_context->vfe_gpu_state.num_urb_entries << 8);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch,
              (pp_context->vfe_gpu_state.urb_entry_size) << 16 |
                /* URB Entry Allocation Size, in 256 bits unit */
              (pp_context->vfe_gpu_state.curbe_allocation_size));
		/* CURBE Allocation Size, in 256 bits unit */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen8_interface_descriptor_load(VADriverContextP ctx,
                               struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 6);

    OUT_BATCH(batch, CMD_MEDIA_STATE_FLUSH);
    OUT_BATCH(batch, 0);

    OUT_BATCH(batch, CMD_MEDIA_INTERFACE_DESCRIPTOR_LOAD | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch,
              pp_context->idrt.num_interface_descriptors * sizeof(struct gen8_interface_descriptor_data));
    OUT_BATCH(batch, pp_context->idrt_offset);
    ADVANCE_BATCH(batch);
}

static void
gen8_pp_curbe_load(VADriverContextP ctx,
                   struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int param_size = 64;

    param_size = sizeof(struct gen7_pp_static_parameter);

    BEGIN_BATCH(batch, 4);
    OUT_BATCH(batch, CMD_MEDIA_CURBE_LOAD | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch,
              param_size);
    OUT_BATCH(batch, pp_context->curbe_offset);
    ADVANCE_BATCH(batch);
}

static void
gen8_pp_object_walker(VADriverContextP ctx,
                      struct i965_post_processing_context *pp_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = pp_context->batch;
    int x, x_steps, y, y_steps;
    int param_size, command_length_in_dws, extra_cmd_in_dws;
    dri_bo *command_buffer;
    unsigned int *command_ptr;

    param_size = sizeof(struct gen7_pp_inline_parameter);

    x_steps = pp_context->pp_x_steps(pp_context->private_context);
    y_steps = pp_context->pp_y_steps(pp_context->private_context);
    command_length_in_dws = 6 + (param_size >> 2);
    extra_cmd_in_dws = 2;
    command_buffer = dri_bo_alloc(i965->intel.bufmgr,
                                  "command objects buffer",
                                  (command_length_in_dws + extra_cmd_in_dws) * 4 * x_steps * y_steps + 64,
                                  4096);

    dri_bo_map(command_buffer, 1);
    command_ptr = command_buffer->virtual;

    for (y = 0; y < y_steps; y++) {
        for (x = 0; x < x_steps; x++) {
            if (!pp_context->pp_set_block_parameter(pp_context, x, y)) {

                *command_ptr++ = (CMD_MEDIA_OBJECT | (command_length_in_dws - 2));
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                memcpy(command_ptr, pp_context->pp_inline_parameter, param_size);
                command_ptr += (param_size >> 2);

                *command_ptr++ = CMD_MEDIA_STATE_FLUSH;
                *command_ptr++ = 0;
            }
        }
    }

    if ((command_length_in_dws + extra_cmd_in_dws) * x_steps * y_steps % 2 == 0)
        *command_ptr++ = 0;

    *command_ptr++ = MI_BATCH_BUFFER_END;
    *command_ptr++ = 0;

    dri_bo_unmap(command_buffer);

    BEGIN_BATCH(batch, 3);
    OUT_BATCH(batch, MI_BATCH_BUFFER_START | (1 << 8) | (1 << 0));
    OUT_RELOC(batch, command_buffer,
              I915_GEM_DOMAIN_COMMAND, 0, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    dri_bo_unreference(command_buffer);

    /* Have to execute the batch buffer here becuase MI_BATCH_BUFFER_END
     * will cause control to pass back to ring buffer
     */
    intel_batchbuffer_end_atomic(batch);
    intel_batchbuffer_flush(batch);
    intel_batchbuffer_start_atomic(batch, 0x1000);
}

static void
gen8_pp_pipeline_setup(VADriverContextP ctx,
                       struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    intel_batchbuffer_start_atomic(batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(batch);
    gen6_pp_pipeline_select(ctx, pp_context);
    gen8_pp_state_base_address(ctx, pp_context);
    gen8_pp_vfe_state(ctx, pp_context);
    gen8_pp_curbe_load(ctx, pp_context);
    gen8_interface_descriptor_load(ctx, pp_context);
    gen8_pp_vfe_state(ctx, pp_context);
    gen8_pp_object_walker(ctx, pp_context);
    intel_batchbuffer_end_atomic(batch);
}

static VAStatus
gen8_post_processing(
    VADriverContextP   ctx,
    struct i965_post_processing_context *pp_context,
    const struct i965_surface *src_surface,
    const VARectangle *src_rect,
    struct i965_surface *dst_surface,
    const VARectangle *dst_rect,
    int                pp_index,
    void * filter_param
)
{
    VAStatus va_status;

    va_status = gen8_pp_initialize(ctx, pp_context,
                                   src_surface,
                                   src_rect,
                                   dst_surface,
                                   dst_rect,
                                   pp_index,
                                   filter_param);

    if (va_status == VA_STATUS_SUCCESS) {
        gen8_pp_states_setup(ctx, pp_context);
        gen8_pp_pipeline_setup(ctx, pp_context);
    }

    return va_status;
}

static void
gen8_post_processing_context_finalize(struct i965_post_processing_context *pp_context)
{
    dri_bo_unreference(pp_context->surface_state_binding_table.bo);
    pp_context->surface_state_binding_table.bo = NULL;

    dri_bo_unreference(pp_context->pp_dndi_context.stmm_bo);
    pp_context->pp_dndi_context.stmm_bo = NULL;

    dri_bo_unreference(pp_context->pp_dn_context.stmm_bo);
    pp_context->pp_dn_context.stmm_bo = NULL;

    if (pp_context->instruction_state.bo) {
	dri_bo_unreference(pp_context->instruction_state.bo);
	pp_context->instruction_state.bo = NULL;
    }

    if (pp_context->indirect_state.bo) {
	dri_bo_unreference(pp_context->indirect_state.bo);
	pp_context->indirect_state.bo = NULL;
    }

    if (pp_context->dynamic_state.bo) {
	dri_bo_unreference(pp_context->dynamic_state.bo);
	pp_context->dynamic_state.bo = NULL;
    }

    free(pp_context->pp_static_parameter);
    free(pp_context->pp_inline_parameter);
    pp_context->pp_static_parameter = NULL;
    pp_context->pp_inline_parameter = NULL;
}

#define VPP_CURBE_ALLOCATION_SIZE	32

void
gen8_post_processing_context_init(VADriverContextP ctx,
                                  void *data,
                                  struct intel_batchbuffer *batch)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i, kernel_size;
    unsigned int kernel_offset, end_offset;
    unsigned char *kernel_ptr;
    struct pp_module *pp_module;
    struct i965_post_processing_context *pp_context = data;

    {
	pp_context->vfe_gpu_state.max_num_threads = 60;
	pp_context->vfe_gpu_state.num_urb_entries = 59;
	pp_context->vfe_gpu_state.gpgpu_mode = 0;
	pp_context->vfe_gpu_state.urb_entry_size = 16 - 1;
	pp_context->vfe_gpu_state.curbe_allocation_size = VPP_CURBE_ALLOCATION_SIZE;
    }

    pp_context->intel_post_processing = gen8_post_processing;
    pp_context->finalize = gen8_post_processing_context_finalize;

    assert(NUM_PP_MODULES == ARRAY_ELEMS(pp_modules_gen8));

    memcpy(pp_context->pp_modules, pp_modules_gen8, sizeof(pp_context->pp_modules));

    kernel_size = 4096 ;

    for (i = 0; i < NUM_PP_MODULES; i++) {
        pp_module = &pp_context->pp_modules[i];

        if (pp_module->kernel.bin && pp_module->kernel.size) {
	    kernel_size += pp_module->kernel.size;
        }
    }

    pp_context->instruction_state.bo = dri_bo_alloc(i965->intel.bufmgr,
                                  "kernel shader",
                                  kernel_size,
                                  0x1000);
    if (pp_context->instruction_state.bo == NULL) {
        WARN_ONCE("failure to allocate the buffer space for kernel shader in VPP\n");
        return;
    }

    assert(pp_context->instruction_state.bo);


    pp_context->instruction_state.bo_size = kernel_size;
    pp_context->instruction_state.end_offset = 0;
    end_offset = 0;

    dri_bo_map(pp_context->instruction_state.bo, 1);
    kernel_ptr = (unsigned char *)(pp_context->instruction_state.bo->virtual);

    for (i = 0; i < NUM_PP_MODULES; i++) {
        pp_module = &pp_context->pp_modules[i];

        kernel_offset = ALIGN(end_offset, 64);
        pp_module->kernel.kernel_offset = kernel_offset;

        if (pp_module->kernel.bin && pp_module->kernel.size) {

	    memcpy(kernel_ptr + kernel_offset, pp_module->kernel.bin, pp_module->kernel.size);
	    end_offset = kernel_offset + pp_module->kernel.size;
        }
    }

    pp_context->instruction_state.end_offset = ALIGN(end_offset, 64);

    dri_bo_unmap(pp_context->instruction_state.bo);

    /* static & inline parameters */
    pp_context->pp_static_parameter = calloc(sizeof(struct gen7_pp_static_parameter), 1);
    pp_context->pp_inline_parameter = calloc(sizeof(struct gen7_pp_inline_parameter), 1);

    pp_context->pp_dndi_context.current_out_surface = VA_INVALID_SURFACE;
    pp_context->pp_dndi_context.current_out_obj_surface = NULL;
    pp_context->pp_dndi_context.frame_order = -1;
    pp_context->batch = batch;

    pp_context->idrt_size = 5 * sizeof(struct gen8_interface_descriptor_data);
    pp_context->curbe_size = 256;
}
