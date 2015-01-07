/*
 * Copyright © 2014 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Keith Packard <keithp@keithp.com>
 *    Xiang Haihao <haihao.xiang@intel.com>
 *    Zhao Yakui <yakui.zhao@intel.com>
 *
 */

/*
 * Most of rendering codes are ported from xf86-video-intel/src/i965_video.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <va/va_drmcommon.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"
#include "i965_defines.h"
#include "i965_drv_video.h"
#include "i965_structs.h"

#include "i965_render.h"

#define SF_KERNEL_NUM_GRF       16
#define SF_MAX_THREADS          1

#define PS_KERNEL_NUM_GRF       48
#define PS_MAX_THREADS          32

/* Programs for Gen8 */
static const uint32_t sf_kernel_static_gen8[][4] ={

};
static const uint32_t ps_kernel_static_gen8[][4] = {
#include "shaders/render/exa_wm_src_affine.g8b"
#include "shaders/render/exa_wm_src_sample_planar.g8b"
#include "shaders/render/exa_wm_yuv_color_balance.g8b"
#include "shaders/render/exa_wm_yuv_rgb.g8b"
#include "shaders/render/exa_wm_write.g8b"
};

static const uint32_t ps_subpic_kernel_static_gen8[][4] = {
#include "shaders/render/exa_wm_src_affine.g8b"
#include "shaders/render/exa_wm_src_sample_argb.g8b"
#include "shaders/render/exa_wm_write.g8b"
};


#define SURFACE_STATE_PADDED_SIZE       SURFACE_STATE_PADDED_SIZE_GEN8

#define SURFACE_STATE_OFFSET(index)     (SURFACE_STATE_PADDED_SIZE * index)
#define BINDING_TABLE_OFFSET            SURFACE_STATE_OFFSET(MAX_RENDER_SURFACES)

enum {
    SF_KERNEL = 0,
    PS_KERNEL,
    PS_SUBPIC_KERNEL
};

static struct i965_kernel render_kernels_gen8[] = {
    {
        "SF",
        SF_KERNEL,
        sf_kernel_static_gen8,
        sizeof(sf_kernel_static_gen8),
        NULL
    },
    {
        "PS",
        PS_KERNEL,
        ps_kernel_static_gen8,
        sizeof(ps_kernel_static_gen8),
        NULL
    },

    {
        "PS_SUBPIC",
        PS_SUBPIC_KERNEL,
        ps_subpic_kernel_static_gen8,
        sizeof(ps_subpic_kernel_static_gen8),
        NULL
    }
};

#define URB_VS_ENTRIES	      8
#define URB_VS_ENTRY_SIZE     1

#define URB_GS_ENTRIES	      0
#define URB_GS_ENTRY_SIZE     0

#define URB_CLIP_ENTRIES      0
#define URB_CLIP_ENTRY_SIZE   0

#define URB_SF_ENTRIES	      1
#define URB_SF_ENTRY_SIZE     2

#define URB_CS_ENTRIES	      4
#define URB_CS_ENTRY_SIZE     4

static float yuv_to_rgb_bt601[3][4] = {
{1.164,		0,	1.596,		-0.06275,},
{1.164,		-0.392,	-0.813,		-0.50196,},
{1.164,		2.017,	0,		-0.50196,},
};

static float yuv_to_rgb_bt709[3][4] = {
{1.164,		0,	1.793,		-0.06275,},
{1.164,		-0.213,	-0.533,		-0.50196,},
{1.164,		2.112,	0,		-0.50196,},
};

static float yuv_to_rgb_smpte_240[3][4] = {
{1.164,		0,	1.794,		-0.06275,},
{1.164,		-0.258,	-0.5425,	-0.50196,},
{1.164,		2.078,	0,		-0.50196,},
};


static void
gen8_render_set_surface_tiling(struct gen8_surface_state *ss, uint32_t tiling)
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

/* Set "Shader Channel Select" for GEN8+ */
void
gen8_render_set_surface_scs(struct gen8_surface_state *ss)
{
    ss->ss7.shader_chanel_select_r = HSW_SCS_RED;
    ss->ss7.shader_chanel_select_g = HSW_SCS_GREEN;
    ss->ss7.shader_chanel_select_b = HSW_SCS_BLUE;
    ss->ss7.shader_chanel_select_a = HSW_SCS_ALPHA;
}

static void
gen8_render_set_surface_state(
    struct gen8_surface_state *ss,
    dri_bo                    *bo,
    unsigned long              offset,
    int                        width,
    int                        height,
    int                        pitch,
    int                        format,
    unsigned int               flags
)
{
    unsigned int tiling;
    unsigned int swizzle;

    memset(ss, 0, sizeof(*ss));

    switch (flags & (I965_PP_FLAG_TOP_FIELD|I965_PP_FLAG_BOTTOM_FIELD)) {
    case I965_PP_FLAG_BOTTOM_FIELD:
        ss->ss0.vert_line_stride_ofs = 1;
        /* fall-through */
    case I965_PP_FLAG_TOP_FIELD:
        ss->ss0.vert_line_stride = 1;
        height /= 2;
        break;
    }

    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = format;

    ss->ss8.base_addr = bo->offset + offset;

    ss->ss2.width = width - 1;
    ss->ss2.height = height - 1;

    ss->ss3.pitch = pitch - 1;

    /* Always set 1(align 4 mode) per B-spec */
    ss->ss0.vertical_alignment = 1;
    ss->ss0.horizontal_alignment = 1;

    dri_bo_get_tiling(bo, &tiling, &swizzle);
    gen8_render_set_surface_tiling(ss, tiling);
}

static void
gen8_render_src_surface_state(
    VADriverContextP ctx,
    int              index,
    dri_bo          *region,
    unsigned long    offset,
    int              w,
    int              h,
    int              pitch,
    int              format,
    unsigned int     flags
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    void *ss;
    dri_bo *ss_bo = render_state->wm.surface_state_binding_table_bo;

    assert(index < MAX_RENDER_SURFACES);

    dri_bo_map(ss_bo, 1);
    assert(ss_bo->virtual);
    ss = (char *)ss_bo->virtual + SURFACE_STATE_OFFSET(index);

    gen8_render_set_surface_state(ss,
                                  region, offset,
                                  w, h,
                                  pitch, format, flags);
    gen8_render_set_surface_scs(ss);
    dri_bo_emit_reloc(ss_bo,
                      I915_GEM_DOMAIN_SAMPLER, 0,
                      offset,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct gen8_surface_state, ss8),
                      region);

    ((unsigned int *)((char *)ss_bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(ss_bo);
    render_state->wm.sampler_count++;
}

static void
gen8_render_src_surfaces_state(
    VADriverContextP ctx,
    struct object_surface *obj_surface,
    unsigned int     flags
)
{
    int region_pitch;
    int rw, rh;
    dri_bo *region;

    region_pitch = obj_surface->width;
    rw = obj_surface->orig_width;
    rh = obj_surface->orig_height;
    region = obj_surface->bo;

    gen8_render_src_surface_state(ctx, 1, region, 0, rw, rh, region_pitch, I965_SURFACEFORMAT_R8_UNORM, flags);     /* Y */
    gen8_render_src_surface_state(ctx, 2, region, 0, rw, rh, region_pitch, I965_SURFACEFORMAT_R8_UNORM, flags);

    if (obj_surface->fourcc == VA_FOURCC_Y800) /* single plane for grayscale */
        return;

    if (obj_surface->fourcc == VA_FOURCC_NV12) {
        gen8_render_src_surface_state(ctx, 3, region,
                                      region_pitch * obj_surface->y_cb_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8G8_UNORM, flags); /* UV */
        gen8_render_src_surface_state(ctx, 4, region,
                                      region_pitch * obj_surface->y_cb_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8G8_UNORM, flags);
    } else {
        gen8_render_src_surface_state(ctx, 3, region,
                                      region_pitch * obj_surface->y_cb_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8_UNORM, flags); /* U */
        gen8_render_src_surface_state(ctx, 4, region,
                                      region_pitch * obj_surface->y_cb_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8_UNORM, flags);
        gen8_render_src_surface_state(ctx, 5, region,
                                      region_pitch * obj_surface->y_cr_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8_UNORM, flags); /* V */
        gen8_render_src_surface_state(ctx, 6, region,
                                      region_pitch * obj_surface->y_cr_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8_UNORM, flags);
    }
}

static void
gen8_subpic_render_src_surfaces_state(VADriverContextP ctx,
                                      struct object_surface *obj_surface)
{
    dri_bo *subpic_region;
    unsigned int index = obj_surface->subpic_render_idx;
    struct object_subpic *obj_subpic = obj_surface->obj_subpic[index];
    struct object_image *obj_image = obj_subpic->obj_image;

    assert(obj_surface);
    assert(obj_surface->bo);
    subpic_region = obj_image->bo;
    /*subpicture surface*/
    gen8_render_src_surface_state(ctx, 1, subpic_region, 0, obj_subpic->width, obj_subpic->height, obj_subpic->pitch, obj_subpic->format, 0);
    gen8_render_src_surface_state(ctx, 2, subpic_region, 0, obj_subpic->width, obj_subpic->height, obj_subpic->pitch, obj_subpic->format, 0);
}

static void
gen8_render_dest_surface_state(VADriverContextP ctx, int index)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct intel_region *dest_region = render_state->draw_region;
    void *ss;
    dri_bo *ss_bo = render_state->wm.surface_state_binding_table_bo;
    int format;
    assert(index < MAX_RENDER_SURFACES);

    if (dest_region->cpp == 2) {
	format = I965_SURFACEFORMAT_B5G6R5_UNORM;
    } else {
	format = I965_SURFACEFORMAT_B8G8R8A8_UNORM;
    }

    dri_bo_map(ss_bo, 1);
    assert(ss_bo->virtual);
    ss = (char *)ss_bo->virtual + SURFACE_STATE_OFFSET(index);

    gen8_render_set_surface_state(ss,
                                  dest_region->bo, 0,
                                  dest_region->width, dest_region->height,
                                  dest_region->pitch, format, 0);
    gen8_render_set_surface_scs(ss);
    dri_bo_emit_reloc(ss_bo,
                      I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
                      0,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct gen8_surface_state, ss8),
                      dest_region->bo);

    ((unsigned int *)((char *)ss_bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(ss_bo);
}

static void
i965_fill_vertex_buffer(
    VADriverContextP ctx,
    float tex_coords[4], /* [(u1,v1);(u2,v2)] */
    float vid_coords[4]  /* [(x1,y1);(x2,y2)] */
)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    float vb[12];

    enum { X1, Y1, X2, Y2 };

    static const unsigned int g_rotation_indices[][6] = {
        [VA_ROTATION_NONE] = { X2, Y2, X1, Y2, X1, Y1 },
        [VA_ROTATION_90]   = { X2, Y1, X2, Y2, X1, Y2 },
        [VA_ROTATION_180]  = { X1, Y1, X2, Y1, X2, Y2 },
        [VA_ROTATION_270]  = { X1, Y2, X1, Y1, X2, Y1 },
    };

    const unsigned int * const rotation_indices =
        g_rotation_indices[i965->rotation_attrib->value];

    vb[0]  = tex_coords[rotation_indices[0]]; /* bottom-right corner */
    vb[1]  = tex_coords[rotation_indices[1]];
    vb[2]  = vid_coords[X2];
    vb[3]  = vid_coords[Y2];

    vb[4]  = tex_coords[rotation_indices[2]]; /* bottom-left corner */
    vb[5]  = tex_coords[rotation_indices[3]];
    vb[6]  = vid_coords[X1];
    vb[7]  = vid_coords[Y2];

    vb[8]  = tex_coords[rotation_indices[4]]; /* top-left corner */
    vb[9]  = tex_coords[rotation_indices[5]];
    vb[10] = vid_coords[X1];
    vb[11] = vid_coords[Y1];

    dri_bo_subdata(i965->render_state.vb.vertex_buffer, 0, sizeof(vb), vb);
}

static void
i965_subpic_render_upload_vertex(VADriverContextP ctx,
                                 struct object_surface *obj_surface,
                                 const VARectangle *output_rect)
{
    unsigned int index = obj_surface->subpic_render_idx;
    struct object_subpic     *obj_subpic   = obj_surface->obj_subpic[index];
    float tex_coords[4], vid_coords[4];
    VARectangle dst_rect;

    if (obj_subpic->flags & VA_SUBPICTURE_DESTINATION_IS_SCREEN_COORD)
        dst_rect = obj_subpic->dst_rect;
    else {
        const float sx  = (float)output_rect->width  / obj_surface->orig_width;
        const float sy  = (float)output_rect->height / obj_surface->orig_height;
        dst_rect.x      = output_rect->x + sx * obj_subpic->dst_rect.x;
        dst_rect.y      = output_rect->y + sy * obj_subpic->dst_rect.y;
        dst_rect.width  = sx * obj_subpic->dst_rect.width;
        dst_rect.height = sy * obj_subpic->dst_rect.height;
    }

    tex_coords[0] = (float)obj_subpic->src_rect.x / obj_subpic->width;
    tex_coords[1] = (float)obj_subpic->src_rect.y / obj_subpic->height;
    tex_coords[2] = (float)(obj_subpic->src_rect.x + obj_subpic->src_rect.width) / obj_subpic->width;
    tex_coords[3] = (float)(obj_subpic->src_rect.y + obj_subpic->src_rect.height) / obj_subpic->height;

    vid_coords[0] = dst_rect.x;
    vid_coords[1] = dst_rect.y;
    vid_coords[2] = (float)(dst_rect.x + dst_rect.width);
    vid_coords[3] = (float)(dst_rect.y + dst_rect.height);

    i965_fill_vertex_buffer(ctx, tex_coords, vid_coords);
}

static void
i965_render_upload_vertex(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct intel_region *dest_region = render_state->draw_region;
    float tex_coords[4], vid_coords[4];
    int width, height;

    width  = obj_surface->orig_width;
    height = obj_surface->orig_height;

    tex_coords[0] = (float)src_rect->x / width;
    tex_coords[1] = (float)src_rect->y / height;
    tex_coords[2] = (float)(src_rect->x + src_rect->width) / width;
    tex_coords[3] = (float)(src_rect->y + src_rect->height) / height;

    vid_coords[0] = dest_region->x + dst_rect->x;
    vid_coords[1] = dest_region->y + dst_rect->y;
    vid_coords[2] = vid_coords[0] + dst_rect->width;
    vid_coords[3] = vid_coords[1] + dst_rect->height;

    i965_fill_vertex_buffer(ctx, tex_coords, vid_coords);
}

static void
i965_render_drawing_rectangle(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;
    struct intel_region *dest_region = render_state->draw_region;

    BEGIN_BATCH(batch, 4);
    OUT_BATCH(batch, CMD_DRAWING_RECTANGLE | 2);
    OUT_BATCH(batch, 0x00000000);
    OUT_BATCH(batch, (dest_region->width - 1) | (dest_region->height - 1) << 16);
    OUT_BATCH(batch, 0x00000000);
    ADVANCE_BATCH(batch);
}

static void
i965_render_upload_image_palette(
    VADriverContextP ctx,
    struct object_image *obj_image,
    unsigned int     alpha
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    unsigned int i;

    assert(obj_image);

    if (!obj_image)
        return;

    if (obj_image->image.num_palette_entries == 0)
        return;

    BEGIN_BATCH(batch, 1 + obj_image->image.num_palette_entries);
    OUT_BATCH(batch, CMD_SAMPLER_PALETTE_LOAD | (obj_image->image.num_palette_entries - 1));
    /*fill palette*/
    //int32_t out[16]; //0-23:color 23-31:alpha
    for (i = 0; i < obj_image->image.num_palette_entries; i++)
        OUT_BATCH(batch, (alpha << 24) | obj_image->palette[i]);
    ADVANCE_BATCH(batch);
}

static void
gen8_clear_dest_region(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;
    struct intel_region *dest_region = render_state->draw_region;
    unsigned int blt_cmd, br13;
    int pitch;

    blt_cmd = GEN8_XY_COLOR_BLT_CMD;
    br13 = 0xf0 << 16;
    pitch = dest_region->pitch;

    if (dest_region->cpp == 4) {
        br13 |= BR13_8888;
        blt_cmd |= (XY_COLOR_BLT_WRITE_RGB | XY_COLOR_BLT_WRITE_ALPHA);
    } else {
        assert(dest_region->cpp == 2);
        br13 |= BR13_565;
    }

    if (dest_region->tiling != I915_TILING_NONE) {
        blt_cmd |= XY_COLOR_BLT_DST_TILED;
        pitch /= 4;
    }

    br13 |= pitch;

    intel_batchbuffer_start_atomic_blt(batch, 24);
    BEGIN_BLT_BATCH(batch, 7);

    OUT_BATCH(batch, blt_cmd);
    OUT_BATCH(batch, br13);
    OUT_BATCH(batch, (dest_region->y << 16) | (dest_region->x));
    OUT_BATCH(batch, ((dest_region->y + dest_region->height) << 16) |
              (dest_region->x + dest_region->width));
    OUT_RELOC(batch, dest_region->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
              0);
    OUT_BATCH(batch, 0x0);
    OUT_BATCH(batch, 0x0);
    ADVANCE_BATCH(batch);
    intel_batchbuffer_end_atomic(batch);
}


/*
 * for GEN8
 */
#define ALIGNMENT       64

static void
gen8_render_initialize(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    dri_bo *bo;
    int size;
    unsigned int end_offset;

    /* VERTEX BUFFER */
    dri_bo_unreference(render_state->vb.vertex_buffer);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vertex buffer",
                      4096,
                      4096);
    assert(bo);
    render_state->vb.vertex_buffer = bo;

    /* WM */
    dri_bo_unreference(render_state->wm.surface_state_binding_table_bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      (SURFACE_STATE_PADDED_SIZE + sizeof(unsigned int)) * MAX_RENDER_SURFACES,
                      4096);
    assert(bo);
    render_state->wm.surface_state_binding_table_bo = bo;

    render_state->curbe_size = 256;

    render_state->wm.sampler_count = 0;

    render_state->sampler_size = MAX_SAMPLERS * sizeof(struct gen8_sampler_state);

    render_state->cc_state_size = sizeof(struct gen6_color_calc_state);

    render_state->cc_viewport_size = sizeof(struct i965_cc_viewport);

    render_state->blend_state_size = sizeof(struct gen8_global_blend_state) +
			16 * sizeof(struct gen8_blend_state_rt);

    render_state->sf_clip_size = 1024;

    render_state->scissor_size = 1024;

    size = ALIGN(render_state->curbe_size, ALIGNMENT) +
        ALIGN(render_state->sampler_size, ALIGNMENT) +
        ALIGN(render_state->cc_viewport_size, ALIGNMENT) +
        ALIGN(render_state->cc_state_size, ALIGNMENT) +
        ALIGN(render_state->blend_state_size, ALIGNMENT) +
        ALIGN(render_state->sf_clip_size, ALIGNMENT) +
        ALIGN(render_state->scissor_size, ALIGNMENT);

    dri_bo_unreference(render_state->dynamic_state.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "dynamic_state",
                      size,
                      4096);

    render_state->dynamic_state.bo = bo;

    end_offset = 0;
    render_state->dynamic_state.end_offset = 0;

    /* Constant buffer offset */
    render_state->curbe_offset = end_offset;
    end_offset += ALIGN(render_state->curbe_size, ALIGNMENT);

    /* Sampler_state  */
    render_state->sampler_offset = end_offset;
    end_offset += ALIGN(render_state->sampler_size, ALIGNMENT);

    /* CC_VIEWPORT_state  */
    render_state->cc_viewport_offset = end_offset;
    end_offset += ALIGN(render_state->cc_viewport_size, ALIGNMENT);

    /* CC_STATE_state  */
    render_state->cc_state_offset = end_offset;
    end_offset += ALIGN(render_state->cc_state_size, ALIGNMENT);

    /* Blend_state  */
    render_state->blend_state_offset = end_offset;
    end_offset += ALIGN(render_state->blend_state_size, ALIGNMENT);

    /* SF_CLIP_state  */
    render_state->sf_clip_offset = end_offset;
    end_offset += ALIGN(render_state->sf_clip_size, ALIGNMENT);

    /* SCISSOR_state  */
    render_state->scissor_offset = end_offset;
    end_offset += ALIGN(render_state->scissor_size, ALIGNMENT);

    /* update the end offset of dynamic_state */
    render_state->dynamic_state.end_offset = end_offset;

}

static void
gen8_render_sampler(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen8_sampler_state *sampler_state;
    int i;
    unsigned char *cc_ptr;

    assert(render_state->wm.sampler_count > 0);
    assert(render_state->wm.sampler_count <= MAX_SAMPLERS);

    dri_bo_map(render_state->dynamic_state.bo, 1);
    assert(render_state->dynamic_state.bo->virtual);

    cc_ptr = (unsigned char *) render_state->dynamic_state.bo->virtual +
			render_state->sampler_offset;

    sampler_state = (struct gen8_sampler_state *) cc_ptr;

    for (i = 0; i < render_state->wm.sampler_count; i++) {
        memset(sampler_state, 0, sizeof(*sampler_state));
        sampler_state->ss0.min_filter = I965_MAPFILTER_LINEAR;
        sampler_state->ss0.mag_filter = I965_MAPFILTER_LINEAR;
        sampler_state->ss3.r_wrap_mode = I965_TEXCOORDMODE_CLAMP;
        sampler_state->ss3.s_wrap_mode = I965_TEXCOORDMODE_CLAMP;
        sampler_state->ss3.t_wrap_mode = I965_TEXCOORDMODE_CLAMP;
        sampler_state++;
    }

    dri_bo_unmap(render_state->dynamic_state.bo);
}

static void
gen8_render_blend_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen8_global_blend_state *global_blend_state;
    struct gen8_blend_state_rt *blend_state;
    unsigned char *cc_ptr;

    dri_bo_map(render_state->dynamic_state.bo, 1);
    assert(render_state->dynamic_state.bo->virtual);

    cc_ptr = (unsigned char *) render_state->dynamic_state.bo->virtual +
			render_state->blend_state_offset;

    global_blend_state = (struct gen8_global_blend_state*) cc_ptr;

    memset(global_blend_state, 0, render_state->blend_state_size);
    /* Global blend state + blend_state for Render Target */
    blend_state = (struct gen8_blend_state_rt *)(global_blend_state + 1);
    blend_state->blend1.logic_op_enable = 1;
    blend_state->blend1.logic_op_func = 0xc;
    blend_state->blend1.pre_blend_clamp_enable = 1;

    dri_bo_unmap(render_state->dynamic_state.bo);
}


static void
gen8_render_cc_viewport(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct i965_cc_viewport *cc_viewport;
    unsigned char *cc_ptr;

    dri_bo_map(render_state->dynamic_state.bo, 1);
    assert(render_state->dynamic_state.bo->virtual);

    cc_ptr = (unsigned char *) render_state->dynamic_state.bo->virtual +
			render_state->cc_viewport_offset;

    cc_viewport = (struct i965_cc_viewport *) cc_ptr;

    memset(cc_viewport, 0, sizeof(*cc_viewport));

    cc_viewport->min_depth = -1.e35;
    cc_viewport->max_depth = 1.e35;

    dri_bo_unmap(render_state->dynamic_state.bo);
}

static void
gen8_render_color_calc_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen6_color_calc_state *color_calc_state;
    unsigned char *cc_ptr;

    dri_bo_map(render_state->dynamic_state.bo, 1);
    assert(render_state->dynamic_state.bo->virtual);

    cc_ptr = (unsigned char *) render_state->dynamic_state.bo->virtual +
			render_state->cc_state_offset;

    color_calc_state = (struct gen6_color_calc_state *) cc_ptr;

    memset(color_calc_state, 0, sizeof(*color_calc_state));
    color_calc_state->constant_r = 1.0;
    color_calc_state->constant_g = 0.0;
    color_calc_state->constant_b = 1.0;
    color_calc_state->constant_a = 1.0;
    dri_bo_unmap(render_state->dynamic_state.bo);
}

#define PI  3.1415926

static void
gen8_render_upload_constants(VADriverContextP ctx,
                             struct object_surface *obj_surface,
                             unsigned int flags)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    unsigned short *constant_buffer;
    unsigned char *cc_ptr;
    float *color_balance_base;
    float contrast = (float)i965->contrast_attrib->value / DEFAULT_CONTRAST;
    float brightness = (float)i965->brightness_attrib->value / 255; /* YUV is float in the shader */
    float hue = (float)i965->hue_attrib->value / 180 * PI;
    float saturation = (float)i965->saturation_attrib->value / DEFAULT_SATURATION;
    float *yuv_to_rgb;
    unsigned int color_flag;

    dri_bo_map(render_state->dynamic_state.bo, 1);
    assert(render_state->dynamic_state.bo->virtual);

    cc_ptr = (unsigned char *) render_state->dynamic_state.bo->virtual +
			render_state->curbe_offset;

    constant_buffer = (unsigned short *) cc_ptr;

    if (obj_surface->subsampling == SUBSAMPLE_YUV400) {
        assert(obj_surface->fourcc == VA_FOURCC_Y800);

        *constant_buffer = 2;
    } else {
        if (obj_surface->fourcc == VA_FOURCC_NV12)
            *constant_buffer = 1;
        else
            *constant_buffer = 0;
    }

    if (i965->contrast_attrib->value == DEFAULT_CONTRAST &&
        i965->brightness_attrib->value == DEFAULT_BRIGHTNESS &&
        i965->hue_attrib->value == DEFAULT_HUE &&
        i965->saturation_attrib->value == DEFAULT_SATURATION)
        constant_buffer[1] = 1; /* skip color balance transformation */
    else
        constant_buffer[1] = 0;

    color_balance_base = (float *)constant_buffer + 4;
    *color_balance_base++ = contrast;
    *color_balance_base++ = brightness;
    *color_balance_base++ = cos(hue) * contrast * saturation;
    *color_balance_base++ = sin(hue) * contrast * saturation;

    color_flag = flags & VA_SRC_COLOR_MASK;
    yuv_to_rgb = (float *)constant_buffer + 8;
    if (color_flag == VA_SRC_BT709)
        memcpy(yuv_to_rgb, yuv_to_rgb_bt709, sizeof(yuv_to_rgb_bt709));
    else if (color_flag == VA_SRC_SMPTE_240)
        memcpy(yuv_to_rgb, yuv_to_rgb_smpte_240, sizeof(yuv_to_rgb_smpte_240));
    else
        memcpy(yuv_to_rgb, yuv_to_rgb_bt601, sizeof(yuv_to_rgb_bt601));

    dri_bo_unmap(render_state->dynamic_state.bo);
}

static void
gen8_render_setup_states(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags
)
{
    gen8_render_dest_surface_state(ctx, 0);
    gen8_render_src_surfaces_state(ctx, obj_surface, flags);
    gen8_render_sampler(ctx);
    gen8_render_cc_viewport(ctx);
    gen8_render_color_calc_state(ctx);
    gen8_render_blend_state(ctx);
    gen8_render_upload_constants(ctx, obj_surface, flags);
    i965_render_upload_vertex(ctx, obj_surface, src_rect, dst_rect);
}

static void
gen8_emit_state_base_address(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 16);
    OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | (16 - 2));
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* General state base address */
	OUT_BATCH(batch, 0);
	OUT_BATCH(batch, 0);
	/*DW4 */
    OUT_RELOC(batch, render_state->wm.surface_state_binding_table_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY); /* Surface state base address */
	OUT_BATCH(batch, 0);

	/*DW6*/
    /* Dynamic state base address */
    OUT_RELOC(batch, render_state->dynamic_state.bo, I915_GEM_DOMAIN_RENDER | I915_GEM_DOMAIN_SAMPLER,
		0, BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0);

	/*DW8*/
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Indirect object base address */
    OUT_BATCH(batch, 0);

	/*DW10 */
    /* Instruction base address */
    OUT_RELOC(batch, render_state->instruction_state.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0);

	/*DW12 */
    OUT_BATCH(batch, 0xFFFF0000 | BASE_ADDRESS_MODIFY); /* General state upper bound */
    OUT_BATCH(batch, 0xFFFF0000 | BASE_ADDRESS_MODIFY); /* Dynamic state upper bound */
    OUT_BATCH(batch, 0xFFFF0000 | BASE_ADDRESS_MODIFY); /* Indirect object upper bound */
    OUT_BATCH(batch, 0xFFFF0000 | BASE_ADDRESS_MODIFY); /* Instruction access upper bound */
    ADVANCE_BATCH(batch);
}

static void
gen8_emit_cc_state_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN6_3DSTATE_CC_STATE_POINTERS | (2 - 2));
    OUT_BATCH(batch, (render_state->cc_state_offset + 1));
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BLEND_STATE_POINTERS | (2 - 2));
    OUT_BATCH(batch, (render_state->blend_state_offset + 1));
    ADVANCE_BATCH(batch);

}

static void
gen8_emit_vertices(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 5);
    OUT_BATCH(batch, CMD_VERTEX_BUFFERS | (5 - 2));
    OUT_BATCH(batch,
              (0 << GEN8_VB0_BUFFER_INDEX_SHIFT) |
	      (0 << GEN8_VB0_MOCS_SHIFT) |
              GEN7_VB0_ADDRESS_MODIFYENABLE |
              ((4 * 4) << VB0_BUFFER_PITCH_SHIFT));
    OUT_RELOC(batch, render_state->vb.vertex_buffer, I915_GEM_DOMAIN_VERTEX, 0, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 12 * 4);
    ADVANCE_BATCH(batch);

    /* Topology in 3D primitive is overrided by VF_TOPOLOGY command */
    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN8_3DSTATE_VF_TOPOLOGY | (2 - 2));
    OUT_BATCH(batch,
              _3DPRIM_RECTLIST);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 7);
    OUT_BATCH(batch, CMD_3DPRIMITIVE | (7 - 2));
    OUT_BATCH(batch,
              GEN7_3DPRIM_VERTEXBUFFER_ACCESS_SEQUENTIAL);
    OUT_BATCH(batch, 3); /* vertex count per instance */
    OUT_BATCH(batch, 0); /* start vertex offset */
    OUT_BATCH(batch, 1); /* single instance */
    OUT_BATCH(batch, 0); /* start instance location */
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen8_emit_vertex_element_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    /*
     * The VUE layout
     * dword 0-3: pad (0, 0, 0. 0)
     * dword 4-7: position (x, y, 1.0, 1.0),
     * dword 8-11: texture coordinate 0 (u0, v0, 1.0, 1.0)
     */

    /* Set up our vertex elements, sourced from the single vertex buffer. */
    OUT_BATCH(batch, CMD_VERTEX_ELEMENTS | (7 - 2));

    /* Element state 0. These are 4 dwords of 0 required for the VUE format.
     * We don't really know or care what they do.
     */

    OUT_BATCH(batch, (0 << GEN8_VE0_VERTEX_BUFFER_INDEX_SHIFT) |
              GEN8_VE0_VALID |
              (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
              (0 << VE0_OFFSET_SHIFT));
    OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_0_SHIFT) |
              (I965_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_1_SHIFT) |
              (I965_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT) |
              (I965_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_3_SHIFT));

    /* offset 8: X, Y -> {x, y, 1.0, 1.0} */
    OUT_BATCH(batch, (0 << GEN8_VE0_VERTEX_BUFFER_INDEX_SHIFT) |
              GEN8_VE0_VALID |
              (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
              (8 << VE0_OFFSET_SHIFT));
    OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT) |
	      (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT));

    /* offset 0: u,v -> {U, V, 1.0, 1.0} */
    OUT_BATCH(batch, (0 << GEN8_VE0_VERTEX_BUFFER_INDEX_SHIFT) |
              GEN8_VE0_VALID |
              (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
              (0 << VE0_OFFSET_SHIFT));
    OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT) |
              (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT));
}

static void
gen8_emit_vs_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    /* disable VS constant buffer */
    BEGIN_BATCH(batch, 11);
    OUT_BATCH(batch, GEN6_3DSTATE_CONSTANT_VS | (11 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /* CS Buffer 0 */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /* CS Buffer 1 */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /* CS Buffer 2 */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /* CS Buffer 3 */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 9);
    OUT_BATCH(batch, GEN6_3DSTATE_VS | (9 - 2));
    OUT_BATCH(batch, 0); /* without VS kernel */
    OUT_BATCH(batch, 0);
    /* VS shader dispatch flag */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /* DW6. VS shader GRF and URB buffer definition */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* pass-through */
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BINDING_TABLE_POINTERS_VS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_SAMPLER_STATE_POINTERS_VS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

}

/*
 * URB layout on GEN8
 * ----------------------------------------
 * | PS Push Constants (8KB) | VS entries |
 * ----------------------------------------
 */
static void
gen8_emit_urb(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    unsigned int num_urb_entries = 64;

    /* The minimum urb entries is 64 */

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_PUSH_CONSTANT_ALLOC_VS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_PUSH_CONSTANT_ALLOC_DS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_PUSH_CONSTANT_ALLOC_HS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_PUSH_CONSTANT_ALLOC_GS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* Size is 8Kbs and base address is 0Kb */
    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_PUSH_CONSTANT_ALLOC_PS | (2 - 2));
    /* Size is 8Kbs and base address is 0Kb */
    OUT_BATCH(batch,
		(0 << GEN8_PUSH_CONSTANT_BUFFER_OFFSET_SHIFT) |
		(8 << GEN8_PUSH_CONSTANT_BUFFER_SIZE_SHIFT));
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_URB_VS | (2 - 2));
    OUT_BATCH(batch,
              (num_urb_entries << GEN7_URB_ENTRY_NUMBER_SHIFT) |
              (4 - 1) << GEN7_URB_ENTRY_SIZE_SHIFT |
              (4 << GEN7_URB_STARTING_ADDRESS_SHIFT));
   ADVANCE_BATCH(batch);

   BEGIN_BATCH(batch, 2);
   OUT_BATCH(batch, GEN7_3DSTATE_URB_GS | (2 - 2));
   OUT_BATCH(batch,
             (0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
             (5 << GEN7_URB_STARTING_ADDRESS_SHIFT));
   ADVANCE_BATCH(batch);

   BEGIN_BATCH(batch, 2);
   OUT_BATCH(batch, GEN7_3DSTATE_URB_HS | (2 - 2));
   OUT_BATCH(batch,
             (0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
             (6 << GEN7_URB_STARTING_ADDRESS_SHIFT));
   ADVANCE_BATCH(batch);

   BEGIN_BATCH(batch, 2);
   OUT_BATCH(batch, GEN7_3DSTATE_URB_DS | (2 - 2));
   OUT_BATCH(batch,
             (0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
             (7 << GEN7_URB_STARTING_ADDRESS_SHIFT));
   ADVANCE_BATCH(batch);
}

static void
gen8_emit_bypass_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    /* bypass GS */
    BEGIN_BATCH(batch, 11);
    OUT_BATCH(batch, GEN6_3DSTATE_CONSTANT_GS | (11 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 10);
    OUT_BATCH(batch, GEN6_3DSTATE_GS | (10 - 2));
    /* GS shader address */
    OUT_BATCH(batch, 0); /* without GS kernel */
    OUT_BATCH(batch, 0);
    /* DW3. GS shader dispatch flag */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /* DW6. GS shader GRF and URB offset/length */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* pass-through */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BINDING_TABLE_POINTERS_GS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_SAMPLER_STATE_POINTERS_GS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* disable HS */
    BEGIN_BATCH(batch, 11);
    OUT_BATCH(batch, GEN7_3DSTATE_CONSTANT_HS | (11 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 9);
    OUT_BATCH(batch, GEN7_3DSTATE_HS | (9 - 2));
    OUT_BATCH(batch, 0);
    /*DW2. HS pass-through */
    OUT_BATCH(batch, 0);
    /*DW3. HS shader address */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /*DW5. HS shader flag. URB offset/length and so on */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BINDING_TABLE_POINTERS_HS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_SAMPLER_STATE_POINTERS_HS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* Disable TE */
    BEGIN_BATCH(batch, 4);
    OUT_BATCH(batch, GEN7_3DSTATE_TE | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* Disable DS */
    BEGIN_BATCH(batch, 11);
    OUT_BATCH(batch, GEN7_3DSTATE_CONSTANT_DS | (11 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 9);
    OUT_BATCH(batch, GEN7_3DSTATE_DS | (9 - 2));
    /* DW1. DS shader pointer */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /* DW3-5. DS shader dispatch flag.*/
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /* DW6-7. DS shader pass-through, GRF,URB offset/Length,Thread Number*/
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /* DW8. DS shader output URB */
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BINDING_TABLE_POINTERS_DS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_SAMPLER_STATE_POINTERS_DS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* Disable STREAMOUT */
    BEGIN_BATCH(batch, 5);
    OUT_BATCH(batch, GEN7_3DSTATE_STREAMOUT | (5 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen8_emit_invarient_states(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 1);
    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_3D);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN8_3DSTATE_MULTISAMPLE | (2 - 2));
    OUT_BATCH(batch, GEN6_3DSTATE_MULTISAMPLE_PIXEL_LOCATION_CENTER |
              GEN6_3DSTATE_MULTISAMPLE_NUMSAMPLES_1); /* 1 sample/pixel */
    ADVANCE_BATCH(batch);

    /* Update 3D Multisample pattern */
    BEGIN_BATCH(batch, 9);
    OUT_BATCH(batch, GEN8_3DSTATE_SAMPLE_PATTERN | (9 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);


    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN6_3DSTATE_SAMPLE_MASK | (2 - 2));
    OUT_BATCH(batch, 1);
    ADVANCE_BATCH(batch);

    /* Set system instruction pointer */
    BEGIN_BATCH(batch, 3);
    OUT_BATCH(batch, CMD_STATE_SIP | 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen8_emit_clip_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    OUT_BATCH(batch, GEN6_3DSTATE_CLIP | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* pass-through */
    OUT_BATCH(batch, 0);
}

static void
gen8_emit_sf_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 5);
    OUT_BATCH(batch, GEN8_3DSTATE_RASTER | (5 - 2));
    OUT_BATCH(batch, GEN8_3DSTATE_RASTER_CULL_NONE);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);


    BEGIN_BATCH(batch, 4);
    OUT_BATCH(batch, GEN7_3DSTATE_SBE | (4 - 2));
    OUT_BATCH(batch,
	      (GEN8_SBE_FORCE_URB_ENTRY_READ_LENGTH) |
	      (GEN8_SBE_FORCE_URB_ENTRY_READ_OFFSET) |
              (1 << GEN7_SBE_NUM_OUTPUTS_SHIFT) |
              (1 << GEN7_SBE_URB_ENTRY_READ_LENGTH_SHIFT) |
              (1 << GEN8_SBE_URB_ENTRY_READ_OFFSET_SHIFT));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* SBE for backend setup */
    BEGIN_BATCH(batch, 11);
    OUT_BATCH(batch, GEN8_3DSTATE_SBE_SWIZ | (11 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 4);
    OUT_BATCH(batch, GEN6_3DSTATE_SF | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 2 << GEN6_3DSTATE_SF_TRIFAN_PROVOKE_SHIFT);
    ADVANCE_BATCH(batch);
}

static void
gen8_emit_wm_state(VADriverContextP ctx, int kernel)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;
    unsigned int num_samples = 0;
    unsigned int max_threads;

    max_threads = i965->intel.device_info->max_wm_threads - 2;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN8_3DSTATE_PSEXTRA | (2 - 2));
    OUT_BATCH(batch,
              (GEN8_PSX_PIXEL_SHADER_VALID | GEN8_PSX_ATTRIBUTE_ENABLE));
    ADVANCE_BATCH(batch);

    if (kernel == PS_KERNEL) {
	BEGIN_BATCH(batch, 2);
	OUT_BATCH(batch, GEN8_3DSTATE_PSBLEND | (2 - 2));
	OUT_BATCH(batch,
		GEN8_PS_BLEND_HAS_WRITEABLE_RT);
    	ADVANCE_BATCH(batch);
    } else if (kernel == PS_SUBPIC_KERNEL) {
	BEGIN_BATCH(batch, 2);
	OUT_BATCH(batch, GEN8_3DSTATE_PSBLEND | (2 - 2));
	OUT_BATCH(batch,
              	(GEN8_PS_BLEND_HAS_WRITEABLE_RT |
		 GEN8_PS_BLEND_COLOR_BUFFER_BLEND_ENABLE |
		 (I965_BLENDFACTOR_SRC_ALPHA << GEN8_PS_BLEND_SRC_ALPHA_BLEND_FACTOR_SHIFT) |
		 (I965_BLENDFACTOR_INV_SRC_ALPHA << GEN8_PS_BLEND_DST_ALPHA_BLEND_FACTOR_SHIFT) |
		 (I965_BLENDFACTOR_SRC_ALPHA << GEN8_PS_BLEND_SRC_BLEND_FACTOR_SHIFT) |
		 (I965_BLENDFACTOR_INV_SRC_ALPHA << GEN8_PS_BLEND_DST_BLEND_FACTOR_SHIFT)));
	ADVANCE_BATCH(batch);
    }

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN6_3DSTATE_WM | (2 - 2));
    OUT_BATCH(batch,
              GEN7_WM_PERSPECTIVE_PIXEL_BARYCENTRIC);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 11);
    OUT_BATCH(batch, GEN6_3DSTATE_CONSTANT_PS | (11 - 2));
    OUT_BATCH(batch, URB_CS_ENTRY_SIZE);
    OUT_BATCH(batch, 0);
    /*DW3-4. Constant buffer 0 */
    OUT_BATCH(batch, render_state->curbe_offset);
    OUT_BATCH(batch, 0);

    /*DW5-10. Constant buffer 1-3 */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 12);
    OUT_BATCH(batch, GEN7_3DSTATE_PS | (12 - 2));
    /* PS shader address */
    OUT_BATCH(batch, render_state->render_kernels[kernel].kernel_offset);

    OUT_BATCH(batch, 0);
    /* DW3. PS shader flag .Binding table cnt/sample cnt */
    OUT_BATCH(batch,
              (1 << GEN7_PS_SAMPLER_COUNT_SHIFT) |
              (5 << GEN7_PS_BINDING_TABLE_ENTRY_COUNT_SHIFT));
    /* DW4-5. Scatch space */
    OUT_BATCH(batch, 0); /* scratch space base offset */
    OUT_BATCH(batch, 0);
    /* DW6. PS shader threads. */
    OUT_BATCH(batch,
              ((max_threads - 1) << GEN8_PS_MAX_THREADS_SHIFT) | num_samples |
              GEN7_PS_PUSH_CONSTANT_ENABLE |
              GEN7_PS_16_DISPATCH_ENABLE);
    /* DW7. PS shader GRF */
    OUT_BATCH(batch,
              (6 << GEN7_PS_DISPATCH_START_GRF_SHIFT_0));
    OUT_BATCH(batch, 0); /* kernel 1 pointer */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* kernel 2 pointer */
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BINDING_TABLE_POINTERS_PS | (2 - 2));
    OUT_BATCH(batch, BINDING_TABLE_OFFSET);
    ADVANCE_BATCH(batch);
}

static void
gen8_emit_depth_buffer_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 8);
    OUT_BATCH(batch, GEN7_3DSTATE_DEPTH_BUFFER | (8 - 2));
    OUT_BATCH(batch,
              (I965_DEPTHFORMAT_D32_FLOAT << 18) |
              (I965_SURFACE_NULL << 29));
    /* DW2-3. Depth Buffer Address */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    /* DW4-7. Surface structure */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* Update the Hier Depth buffer */
    BEGIN_BATCH(batch, 5);
    OUT_BATCH(batch, GEN7_3DSTATE_HIER_DEPTH_BUFFER | (5 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* Update the stencil buffer */
    BEGIN_BATCH(batch, 5);
    OUT_BATCH(batch, GEN7_3DSTATE_STENCIL_BUFFER | (5 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 3);
    OUT_BATCH(batch, GEN7_3DSTATE_CLEAR_PARAMS | (3 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen8_emit_depth_stencil_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 3);
    OUT_BATCH(batch, GEN8_3DSTATE_WM_DEPTH_STENCIL | (3 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen8_emit_wm_hz_op(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 5);
    OUT_BATCH(batch, GEN8_3DSTATE_WM_HZ_OP | (5 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen8_emit_viewport_state_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_VIEWPORT_STATE_POINTERS_CC | (2 - 2));
    OUT_BATCH(batch, render_state->cc_viewport_offset);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CL | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen8_emit_sampler_state_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_SAMPLER_STATE_POINTERS_PS | (2 - 2));
    OUT_BATCH(batch, render_state->sampler_offset);
    ADVANCE_BATCH(batch);
}


static void
gen7_emit_drawing_rectangle(VADriverContextP ctx)
{
    i965_render_drawing_rectangle(ctx);
}

static void
gen8_render_emit_states(VADriverContextP ctx, int kernel)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    intel_batchbuffer_start_atomic(batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(batch);
    gen8_emit_invarient_states(ctx);
    gen8_emit_state_base_address(ctx);
    gen8_emit_viewport_state_pointers(ctx);
    gen8_emit_urb(ctx);
    gen8_emit_cc_state_pointers(ctx);
    gen8_emit_sampler_state_pointers(ctx);
    gen8_emit_wm_hz_op(ctx);
    gen8_emit_bypass_state(ctx);
    gen8_emit_vs_state(ctx);
    gen8_emit_clip_state(ctx);
    gen8_emit_sf_state(ctx);
    gen8_emit_depth_stencil_state(ctx);
    gen8_emit_wm_state(ctx, kernel);
    gen8_emit_depth_buffer_state(ctx);
    gen7_emit_drawing_rectangle(ctx);
    gen8_emit_vertex_element_state(ctx);
    gen8_emit_vertices(ctx);
    intel_batchbuffer_end_atomic(batch);
}

static void
gen8_render_put_surface(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    gen8_render_initialize(ctx);
    gen8_render_setup_states(ctx, obj_surface, src_rect, dst_rect, flags);
    gen8_clear_dest_region(ctx);
    gen8_render_emit_states(ctx, PS_KERNEL);
    intel_batchbuffer_flush(batch);
}

static void
gen8_subpicture_render_blend_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen8_global_blend_state *global_blend_state;
    struct gen8_blend_state_rt *blend_state;
    unsigned char *cc_ptr;

    dri_bo_map(render_state->dynamic_state.bo, 1);
    assert(render_state->dynamic_state.bo->virtual);

    cc_ptr = (unsigned char *) render_state->dynamic_state.bo->virtual +
			render_state->blend_state_offset;

    global_blend_state = (struct gen8_global_blend_state*) cc_ptr;

    memset(global_blend_state, 0, render_state->blend_state_size);
    /* Global blend state + blend_state for Render Target */
    blend_state = (struct gen8_blend_state_rt *)(global_blend_state + 1);
    blend_state->blend0.color_blend_func = I965_BLENDFUNCTION_ADD;
    blend_state->blend0.dest_blend_factor = I965_BLENDFACTOR_INV_SRC_ALPHA;
    blend_state->blend0.src_blend_factor = I965_BLENDFACTOR_SRC_ALPHA;
    blend_state->blend0.alpha_blend_func = I965_BLENDFUNCTION_ADD;
    blend_state->blend0.ia_dest_blend_factor = I965_BLENDFACTOR_INV_SRC_ALPHA;
    blend_state->blend0.ia_src_blend_factor = I965_BLENDFACTOR_SRC_ALPHA;
    blend_state->blend0.colorbuf_blend = 1;
    blend_state->blend1.post_blend_clamp_enable = 1;
    blend_state->blend1.pre_blend_clamp_enable = 1;
    blend_state->blend1.clamp_range = 0; /* clamp range [0, 1] */

    dri_bo_unmap(render_state->dynamic_state.bo);
}

static void
gen8_subpic_render_upload_constants(VADriverContextP ctx,
                                    struct object_surface *obj_surface)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    float *constant_buffer;
    float global_alpha = 1.0;
    unsigned int index = obj_surface->subpic_render_idx;
    struct object_subpic *obj_subpic = obj_surface->obj_subpic[index];
    unsigned char *cc_ptr;

    if (obj_subpic->flags & VA_SUBPICTURE_GLOBAL_ALPHA) {
        global_alpha = obj_subpic->global_alpha;
    }


    dri_bo_map(render_state->dynamic_state.bo, 1);
    assert(render_state->dynamic_state.bo->virtual);

    cc_ptr = (unsigned char *) render_state->dynamic_state.bo->virtual +
				render_state->curbe_offset;

    constant_buffer = (float *) cc_ptr;
    *constant_buffer = global_alpha;

    dri_bo_unmap(render_state->dynamic_state.bo);
}

static void
gen8_subpicture_render_setup_states(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    gen8_render_dest_surface_state(ctx, 0);
    gen8_subpic_render_src_surfaces_state(ctx, obj_surface);
    gen8_render_sampler(ctx);
    gen8_render_cc_viewport(ctx);
    gen8_render_color_calc_state(ctx);
    gen8_subpicture_render_blend_state(ctx);
    gen8_subpic_render_upload_constants(ctx, obj_surface);
    i965_subpic_render_upload_vertex(ctx, obj_surface, dst_rect);
}

static void
gen8_render_put_subpicture(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    unsigned int index = obj_surface->subpic_render_idx;
    struct object_subpic *obj_subpic = obj_surface->obj_subpic[index];

    assert(obj_subpic);
    gen8_render_initialize(ctx);
    gen8_subpicture_render_setup_states(ctx, obj_surface, src_rect, dst_rect);
    gen8_render_emit_states(ctx, PS_SUBPIC_KERNEL);
    i965_render_upload_image_palette(ctx, obj_subpic->obj_image, 0xff);
    intel_batchbuffer_flush(batch);
}

static void
gen8_render_terminate(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;

    dri_bo_unreference(render_state->vb.vertex_buffer);
    render_state->vb.vertex_buffer = NULL;

    dri_bo_unreference(render_state->wm.surface_state_binding_table_bo);
    render_state->wm.surface_state_binding_table_bo = NULL;

    if (render_state->instruction_state.bo) {
        dri_bo_unreference(render_state->instruction_state.bo);
        render_state->instruction_state.bo = NULL;
    }

    if (render_state->dynamic_state.bo) {
        dri_bo_unreference(render_state->dynamic_state.bo);
        render_state->dynamic_state.bo = NULL;
    }

    if (render_state->indirect_state.bo) {
        dri_bo_unreference(render_state->indirect_state.bo);
        render_state->indirect_state.bo = NULL;
    }

    if (render_state->draw_region) {
        dri_bo_unreference(render_state->draw_region->bo);
        free(render_state->draw_region);
        render_state->draw_region = NULL;
    }
}

bool
gen8_render_init(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    int i, kernel_size;
    unsigned int kernel_offset, end_offset;
    unsigned char *kernel_ptr;
    struct i965_kernel *kernel;

    render_state->render_put_surface = gen8_render_put_surface;
    render_state->render_put_subpicture = gen8_render_put_subpicture;
    render_state->render_terminate = gen8_render_terminate;

    memcpy(render_state->render_kernels, render_kernels_gen8,
           sizeof(render_state->render_kernels));

    kernel_size = 4096;

    for (i = 0; i < NUM_RENDER_KERNEL; i++) {
        kernel = &render_state->render_kernels[i];

        if (!kernel->size)
            continue;

        kernel_size += kernel->size;
    }

    render_state->instruction_state.bo = dri_bo_alloc(i965->intel.bufmgr,
                                  "kernel shader",
                                  kernel_size,
                                  0x1000);
    if (render_state->instruction_state.bo == NULL) {
        WARN_ONCE("failure to allocate the buffer space for kernel shader\n");
        return false;
    }

    assert(render_state->instruction_state.bo);

    render_state->instruction_state.bo_size = kernel_size;
    render_state->instruction_state.end_offset = 0;
    end_offset = 0;

    dri_bo_map(render_state->instruction_state.bo, 1);
    kernel_ptr = (unsigned char *)(render_state->instruction_state.bo->virtual);
    for (i = 0; i < NUM_RENDER_KERNEL; i++) {
        kernel = &render_state->render_kernels[i];
        kernel_offset = end_offset;
        kernel->kernel_offset = kernel_offset;

        if (!kernel->size)
            continue;

        memcpy(kernel_ptr + kernel_offset, kernel->bin, kernel->size);

        end_offset += ALIGN(kernel->size, ALIGNMENT);
    }

    render_state->instruction_state.end_offset = end_offset;

    dri_bo_unmap(render_state->instruction_state.bo);

    return true;
}
