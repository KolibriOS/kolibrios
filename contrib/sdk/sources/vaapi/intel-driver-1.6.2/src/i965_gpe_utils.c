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
 *
 * Authors:
 *    Xiang Haihao <haihao.xiang@intel.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"

#include "i965_gpe_utils.h"

static void
i965_gpe_select(VADriverContextP ctx,
                struct i965_gpe_context *gpe_context,
                struct intel_batchbuffer *batch)
{
    BEGIN_BATCH(batch, 1);
    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_MEDIA);
    ADVANCE_BATCH(batch);
}

static void
gen6_gpe_state_base_address(VADriverContextP ctx,
                            struct i965_gpe_context *gpe_context,
                            struct intel_batchbuffer *batch)
{
    BEGIN_BATCH(batch, 10);

    OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | (10 - 2));
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY);              /* General State Base Address */
    OUT_RELOC(batch,
              gpe_context->surface_state_binding_table.bo,
              I915_GEM_DOMAIN_INSTRUCTION,
              0,
              BASE_ADDRESS_MODIFY);                     /* Surface state base address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY);              /* Dynamic State Base Address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY);              /* Indirect Object Base Address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY);              /* Instruction Base Address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY);              /* General State Access Upper Bound */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY);              /* Dynamic State Access Upper Bound */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY);              /* Indirect Object Access Upper Bound */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY);              /* Instruction Access Upper Bound */

    ADVANCE_BATCH(batch);
}

static void
gen6_gpe_vfe_state(VADriverContextP ctx,
                   struct i965_gpe_context *gpe_context,
                   struct intel_batchbuffer *batch)
{

    BEGIN_BATCH(batch, 8);

    OUT_BATCH(batch, CMD_MEDIA_VFE_STATE | (8 - 2));
    OUT_BATCH(batch, 0);                                        /* Scratch Space Base Pointer and Space */
    OUT_BATCH(batch,
              gpe_context->vfe_state.max_num_threads << 16 |    /* Maximum Number of Threads */
              gpe_context->vfe_state.num_urb_entries << 8 |     /* Number of URB Entries */
              gpe_context->vfe_state.gpgpu_mode << 2);          /* MEDIA Mode */
    OUT_BATCH(batch, 0);                                        /* Debug: Object ID */
    OUT_BATCH(batch,
              gpe_context->vfe_state.urb_entry_size << 16 |     /* URB Entry Allocation Size */
              gpe_context->vfe_state.curbe_allocation_size);    /* CURBE Allocation Size */
    /* the vfe_desc5/6/7 will decide whether the scoreboard is used. */
    OUT_BATCH(batch, gpe_context->vfe_desc5.dword);                                        
    OUT_BATCH(batch, gpe_context->vfe_desc6.dword);                                       
    OUT_BATCH(batch, gpe_context->vfe_desc7.dword);                                       
	
    ADVANCE_BATCH(batch);

}

static void
gen6_gpe_curbe_load(VADriverContextP ctx,
                    struct i965_gpe_context *gpe_context,
                    struct intel_batchbuffer *batch)
{
    BEGIN_BATCH(batch, 4);

    OUT_BATCH(batch, CMD_MEDIA_CURBE_LOAD | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, gpe_context->curbe.length);
    OUT_RELOC(batch, gpe_context->curbe.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    ADVANCE_BATCH(batch);
}

static void
gen6_gpe_idrt(VADriverContextP ctx,
              struct i965_gpe_context *gpe_context,
              struct intel_batchbuffer *batch)
{
    BEGIN_BATCH(batch, 4);

    OUT_BATCH(batch, CMD_MEDIA_INTERFACE_LOAD | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, gpe_context->idrt.max_entries * gpe_context->idrt.entry_size);
    OUT_RELOC(batch, gpe_context->idrt.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    ADVANCE_BATCH(batch);
}

void
i965_gpe_load_kernels(VADriverContextP ctx,
                      struct i965_gpe_context *gpe_context,
                      struct i965_kernel *kernel_list,
                      unsigned int num_kernels)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i;

    assert(num_kernels <= MAX_GPE_KERNELS);
    memcpy(gpe_context->kernels, kernel_list, sizeof(*kernel_list) * num_kernels);
    gpe_context->num_kernels = num_kernels;

    for (i = 0; i < num_kernels; i++) {
        struct i965_kernel *kernel = &gpe_context->kernels[i];

        kernel->bo = dri_bo_alloc(i965->intel.bufmgr, 
                                  kernel->name, 
                                  kernel->size,
                                  0x1000);
        assert(kernel->bo);
        dri_bo_subdata(kernel->bo, 0, kernel->size, kernel->bin);
    }
}

void
i965_gpe_context_destroy(struct i965_gpe_context *gpe_context)
{
    int i;

    dri_bo_unreference(gpe_context->surface_state_binding_table.bo);
    gpe_context->surface_state_binding_table.bo = NULL;

    dri_bo_unreference(gpe_context->idrt.bo);
    gpe_context->idrt.bo = NULL;

    dri_bo_unreference(gpe_context->curbe.bo);
    gpe_context->curbe.bo = NULL;

    for (i = 0; i < gpe_context->num_kernels; i++) {
        struct i965_kernel *kernel = &gpe_context->kernels[i];

        dri_bo_unreference(kernel->bo);
        kernel->bo = NULL;
    }
}

void
i965_gpe_context_init(VADriverContextP ctx,
                      struct i965_gpe_context *gpe_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    dri_bo *bo;

    dri_bo_unreference(gpe_context->surface_state_binding_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      gpe_context->surface_state_binding_table.length,
                      4096);
    assert(bo);
    gpe_context->surface_state_binding_table.bo = bo;

    dri_bo_unreference(gpe_context->idrt.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "interface descriptor table",
                      gpe_context->idrt.entry_size * gpe_context->idrt.max_entries,
                      4096);
    assert(bo);
    gpe_context->idrt.bo = bo;

    dri_bo_unreference(gpe_context->curbe.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "curbe buffer",
                      gpe_context->curbe.length,
                      4096);
    assert(bo);
    gpe_context->curbe.bo = bo;
}

void
gen6_gpe_pipeline_setup(VADriverContextP ctx,
                        struct i965_gpe_context *gpe_context,
                        struct intel_batchbuffer *batch)
{
    intel_batchbuffer_emit_mi_flush(batch);

    i965_gpe_select(ctx, gpe_context, batch);
    gen6_gpe_state_base_address(ctx, gpe_context, batch);
    gen6_gpe_vfe_state(ctx, gpe_context, batch);
    gen6_gpe_curbe_load(ctx, gpe_context, batch);
    gen6_gpe_idrt(ctx, gpe_context, batch);
}

static void
i965_gpe_set_surface_tiling(struct i965_surface_state *ss, unsigned int tiling)
{
    switch (tiling) {
    case I915_TILING_NONE:
        ss->ss3.tiled_surface = 0;
        ss->ss3.tile_walk = 0;
        break;
    case I915_TILING_X:
        ss->ss3.tiled_surface = 1;
        ss->ss3.tile_walk = I965_TILEWALK_XMAJOR;
        break;
    case I915_TILING_Y:
        ss->ss3.tiled_surface = 1;
        ss->ss3.tile_walk = I965_TILEWALK_YMAJOR;
        break;
    }
}

static void
i965_gpe_set_surface2_tiling(struct i965_surface_state2 *ss, unsigned int tiling)
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
gen7_gpe_set_surface_tiling(struct gen7_surface_state *ss, unsigned int tiling)
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
gen7_gpe_set_surface2_tiling(struct gen7_surface_state2 *ss, unsigned int tiling)
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
gen8_gpe_set_surface_tiling(struct gen8_surface_state *ss, unsigned int tiling)
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
gen8_gpe_set_surface2_tiling(struct gen8_surface_state2 *ss, unsigned int tiling)
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
i965_gpe_set_surface2_state(VADriverContextP ctx,
                            struct object_surface *obj_surface,
                            struct i965_surface_state2 *ss)
{
    int w, h, w_pitch;
    unsigned int tiling, swizzle;

    assert(obj_surface->bo);
    assert(obj_surface->fourcc == VA_FOURCC_NV12);

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    w = obj_surface->orig_width;
    h = obj_surface->orig_height;
    w_pitch = obj_surface->width;

    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss0.surface_base_address = obj_surface->bo->offset;
    /* ss1 */
    ss->ss1.cbcr_pixel_offset_v_direction = 2;
    ss->ss1.width = w - 1;
    ss->ss1.height = h - 1;
    /* ss2 */
    ss->ss2.surface_format = MFX_SURFACE_PLANAR_420_8;
    ss->ss2.interleave_chroma = 1;
    ss->ss2.pitch = w_pitch - 1;
    ss->ss2.half_pitch_for_chroma = 0;
    i965_gpe_set_surface2_tiling(ss, tiling);
    /* ss3: UV offset for interleave mode */
    ss->ss3.x_offset_for_cb = obj_surface->x_cb_offset;
    ss->ss3.y_offset_for_cb = obj_surface->y_cb_offset;
}

void
i965_gpe_surface2_setup(VADriverContextP ctx,
                        struct i965_gpe_context *gpe_context,
                        struct object_surface *obj_surface,
                        unsigned long binding_table_offset,
                        unsigned long surface_state_offset)
{
    struct i965_surface_state2 *ss;
    dri_bo *bo;

    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);

    ss = (struct i965_surface_state2 *)((char *)bo->virtual + surface_state_offset);
    i965_gpe_set_surface2_state(ctx, obj_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      0,
                      surface_state_offset + offsetof(struct i965_surface_state2, ss0),
                      obj_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}

static void
i965_gpe_set_media_rw_surface_state(VADriverContextP ctx,
                                    struct object_surface *obj_surface,
                                    struct i965_surface_state *ss)
{
    int w, h, w_pitch;
    unsigned int tiling, swizzle;

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    w = obj_surface->orig_width;
    h = obj_surface->orig_height;
    w_pitch = obj_surface->width;

    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = I965_SURFACEFORMAT_R8_UNORM;
    /* ss1 */
    ss->ss1.base_addr = obj_surface->bo->offset;
    /* ss2 */
    ss->ss2.width = w / 4 - 1;  /* in DWORDs for media read & write message */
    ss->ss2.height = h - 1;
    /* ss3 */
    ss->ss3.pitch = w_pitch - 1;
    i965_gpe_set_surface_tiling(ss, tiling);
}

void
i965_gpe_media_rw_surface_setup(VADriverContextP ctx,
                                struct i965_gpe_context *gpe_context,
                                struct object_surface *obj_surface,
                                unsigned long binding_table_offset,
                                unsigned long surface_state_offset)
{
    struct i965_surface_state *ss;
    dri_bo *bo;

    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, True);
    assert(bo->virtual);

    ss = (struct i965_surface_state *)((char *)bo->virtual + surface_state_offset);
    i965_gpe_set_media_rw_surface_state(ctx, obj_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      0,
                      surface_state_offset + offsetof(struct i965_surface_state, ss1),
                      obj_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}

static void
i965_gpe_set_buffer_surface_state(VADriverContextP ctx,
                                  struct i965_buffer_surface *buffer_surface,
                                  struct i965_surface_state *ss)
{
    int num_entries;

    assert(buffer_surface->bo);
    num_entries = buffer_surface->num_blocks * buffer_surface->size_block / buffer_surface->pitch;

    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss0.render_cache_read_mode = 1;
    ss->ss0.surface_type = I965_SURFACE_BUFFER;
    /* ss1 */
    ss->ss1.base_addr = buffer_surface->bo->offset;
    /* ss2 */
    ss->ss2.width = ((num_entries - 1) & 0x7f);
    ss->ss2.height = (((num_entries - 1) >> 7) & 0x1fff);
    /* ss3 */
    ss->ss3.depth = (((num_entries - 1) >> 20) & 0x7f);
    ss->ss3.pitch = buffer_surface->pitch - 1;
}

void
i965_gpe_buffer_suface_setup(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct i965_buffer_surface *buffer_surface,
                             unsigned long binding_table_offset,
                             unsigned long surface_state_offset)
{
    struct i965_surface_state *ss;
    dri_bo *bo;

    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);

    ss = (struct i965_surface_state *)((char *)bo->virtual + surface_state_offset);
    i965_gpe_set_buffer_surface_state(ctx, buffer_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
                      0,
                      surface_state_offset + offsetof(struct i965_surface_state, ss1),
                      buffer_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}

static void
gen7_gpe_set_surface2_state(VADriverContextP ctx,
                            struct object_surface *obj_surface,
                            struct gen7_surface_state2 *ss)
{
    int w, h, w_pitch;
    unsigned int tiling, swizzle;

    assert(obj_surface->bo);
    assert(obj_surface->fourcc == VA_FOURCC_NV12);

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    w = obj_surface->orig_width;
    h = obj_surface->orig_height;
    w_pitch = obj_surface->width;

    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss0.surface_base_address = obj_surface->bo->offset;
    /* ss1 */
    ss->ss1.cbcr_pixel_offset_v_direction = 2;
    ss->ss1.width = w - 1;
    ss->ss1.height = h - 1;
    /* ss2 */
    ss->ss2.surface_format = MFX_SURFACE_PLANAR_420_8;
    ss->ss2.interleave_chroma = 1;
    ss->ss2.pitch = w_pitch - 1;
    ss->ss2.half_pitch_for_chroma = 0;
    gen7_gpe_set_surface2_tiling(ss, tiling);
    /* ss3: UV offset for interleave mode */
    ss->ss3.x_offset_for_cb = obj_surface->x_cb_offset;
    ss->ss3.y_offset_for_cb = obj_surface->y_cb_offset;
}

void
gen7_gpe_surface2_setup(VADriverContextP ctx,
                        struct i965_gpe_context *gpe_context,
                        struct object_surface *obj_surface,
                        unsigned long binding_table_offset,
                        unsigned long surface_state_offset)
{
    struct gen7_surface_state2 *ss;
    dri_bo *bo;

    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);

    ss = (struct gen7_surface_state2 *)((char *)bo->virtual + surface_state_offset);
    gen7_gpe_set_surface2_state(ctx, obj_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      0,
                      surface_state_offset + offsetof(struct gen7_surface_state2, ss0),
                      obj_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}

static void
gen7_gpe_set_media_rw_surface_state(VADriverContextP ctx,
                                    struct object_surface *obj_surface,
                                    struct gen7_surface_state *ss)
{
    int w, h, w_pitch;
    unsigned int tiling, swizzle;

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    w = obj_surface->orig_width;
    h = obj_surface->orig_height;
    w_pitch = obj_surface->width;

    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = I965_SURFACEFORMAT_R8_UNORM;
    /* ss1 */
    ss->ss1.base_addr = obj_surface->bo->offset;
    /* ss2 */
    ss->ss2.width = w / 4 - 1;  /* in DWORDs for media read & write message */
    ss->ss2.height = h - 1;
    /* ss3 */
    ss->ss3.pitch = w_pitch - 1;
    gen7_gpe_set_surface_tiling(ss, tiling);
}

static void
gen75_gpe_set_media_chroma_surface_state(VADriverContextP ctx,
                                    struct object_surface *obj_surface,
                                    struct gen7_surface_state *ss)
{
    int w, w_pitch;
    unsigned int tiling, swizzle;
    int cbcr_offset;

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    w = obj_surface->orig_width;
    w_pitch = obj_surface->width;

    cbcr_offset = obj_surface->height * obj_surface->width;
    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = I965_SURFACEFORMAT_R8_UNORM;
    /* ss1 */
    ss->ss1.base_addr = obj_surface->bo->offset + cbcr_offset;
    /* ss2 */
    ss->ss2.width = w / 4 - 1;  /* in DWORDs for media read & write message */
    ss->ss2.height = (obj_surface->height / 2) -1;
    /* ss3 */
    ss->ss3.pitch = w_pitch - 1;
    gen7_gpe_set_surface_tiling(ss, tiling);
}

void
gen7_gpe_media_rw_surface_setup(VADriverContextP ctx,
                                struct i965_gpe_context *gpe_context,
                                struct object_surface *obj_surface,
                                unsigned long binding_table_offset,
                                unsigned long surface_state_offset)
{
    struct gen7_surface_state *ss;
    dri_bo *bo;

    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, True);
    assert(bo->virtual);

    ss = (struct gen7_surface_state *)((char *)bo->virtual + surface_state_offset);
    gen7_gpe_set_media_rw_surface_state(ctx, obj_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      0,
                      surface_state_offset + offsetof(struct gen7_surface_state, ss1),
                      obj_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}

void
gen75_gpe_media_chroma_surface_setup(VADriverContextP ctx,
                                struct i965_gpe_context *gpe_context,
                                struct object_surface *obj_surface,
                                unsigned long binding_table_offset,
                                unsigned long surface_state_offset)
{
    struct gen7_surface_state *ss;
    dri_bo *bo;
    int cbcr_offset;

	assert(obj_surface->fourcc == VA_FOURCC_NV12);
    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, True);
    assert(bo->virtual);

    cbcr_offset = obj_surface->height * obj_surface->width;
    ss = (struct gen7_surface_state *)((char *)bo->virtual + surface_state_offset);
    gen75_gpe_set_media_chroma_surface_state(ctx, obj_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      cbcr_offset,
                      surface_state_offset + offsetof(struct gen7_surface_state, ss1),
                      obj_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}


static void
gen7_gpe_set_buffer_surface_state(VADriverContextP ctx,
                                  struct i965_buffer_surface *buffer_surface,
                                  struct gen7_surface_state *ss)
{
    int num_entries;

    assert(buffer_surface->bo);
    num_entries = buffer_surface->num_blocks * buffer_surface->size_block / buffer_surface->pitch;

    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss0.surface_type = I965_SURFACE_BUFFER;
    /* ss1 */
    ss->ss1.base_addr = buffer_surface->bo->offset;
    /* ss2 */
    ss->ss2.width = ((num_entries - 1) & 0x7f);
    ss->ss2.height = (((num_entries - 1) >> 7) & 0x3fff);
    /* ss3 */
    ss->ss3.depth = (((num_entries - 1) >> 21) & 0x3f);
    ss->ss3.pitch = buffer_surface->pitch - 1;
}

void
gen7_gpe_buffer_suface_setup(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct i965_buffer_surface *buffer_surface,
                             unsigned long binding_table_offset,
                             unsigned long surface_state_offset)
{
    struct gen7_surface_state *ss;
    dri_bo *bo;

    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);

    ss = (struct gen7_surface_state *)((char *)bo->virtual + surface_state_offset);
    gen7_gpe_set_buffer_surface_state(ctx, buffer_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
                      0,
                      surface_state_offset + offsetof(struct gen7_surface_state, ss1),
                      buffer_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}

static void
gen8_gpe_set_surface2_state(VADriverContextP ctx,
                            struct object_surface *obj_surface,
                            struct gen8_surface_state2 *ss)
{
    int w, h, w_pitch;
    unsigned int tiling, swizzle;

    assert(obj_surface->bo);
    assert(obj_surface->fourcc == VA_FOURCC_NV12);

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    w = obj_surface->orig_width;
    h = obj_surface->orig_height;
    w_pitch = obj_surface->width;

    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss6.base_addr = obj_surface->bo->offset;
    /* ss1 */
    ss->ss1.cbcr_pixel_offset_v_direction = 2;
    ss->ss1.width = w - 1;
    ss->ss1.height = h - 1;
    /* ss2 */
    ss->ss2.surface_format = MFX_SURFACE_PLANAR_420_8;
    ss->ss2.interleave_chroma = 1;
    ss->ss2.pitch = w_pitch - 1;
    ss->ss2.half_pitch_for_chroma = 0;
    gen8_gpe_set_surface2_tiling(ss, tiling);
    /* ss3: UV offset for interleave mode */
    ss->ss3.x_offset_for_cb = obj_surface->x_cb_offset;
    ss->ss3.y_offset_for_cb = obj_surface->y_cb_offset;
}

void
gen8_gpe_surface2_setup(VADriverContextP ctx,
                        struct i965_gpe_context *gpe_context,
                        struct object_surface *obj_surface,
                        unsigned long binding_table_offset,
                        unsigned long surface_state_offset)
{
    struct gen8_surface_state2 *ss;
    dri_bo *bo;

    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);

    ss = (struct gen8_surface_state2 *)((char *)bo->virtual + surface_state_offset);
    gen8_gpe_set_surface2_state(ctx, obj_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      0,
                      surface_state_offset + offsetof(struct gen8_surface_state2, ss6),
                      obj_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}

static void
gen8_gpe_set_media_rw_surface_state(VADriverContextP ctx,
                                    struct object_surface *obj_surface,
                                    struct gen8_surface_state *ss)
{
    int w, h, w_pitch;
    unsigned int tiling, swizzle;

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    w = obj_surface->orig_width;
    h = obj_surface->orig_height;
    w_pitch = obj_surface->width;

    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = I965_SURFACEFORMAT_R8_UNORM;
    /* ss1 */
    ss->ss8.base_addr = obj_surface->bo->offset;
    /* ss2 */
    ss->ss2.width = w / 4 - 1;  /* in DWORDs for media read & write message */
    ss->ss2.height = h - 1;
    /* ss3 */
    ss->ss3.pitch = w_pitch - 1;
    gen8_gpe_set_surface_tiling(ss, tiling);
}

static void
gen8_gpe_set_media_chroma_surface_state(VADriverContextP ctx,
                                    struct object_surface *obj_surface,
                                    struct gen8_surface_state *ss)
{
    int w, w_pitch;
    unsigned int tiling, swizzle;
    int cbcr_offset;

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    w = obj_surface->orig_width;
    w_pitch = obj_surface->width;

    cbcr_offset = obj_surface->height * obj_surface->width;
    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = I965_SURFACEFORMAT_R8_UNORM;
    /* ss1 */
    ss->ss8.base_addr = obj_surface->bo->offset + cbcr_offset;
    /* ss2 */
    ss->ss2.width = w / 4 - 1;  /* in DWORDs for media read & write message */
    ss->ss2.height = (obj_surface->height / 2) -1;
    /* ss3 */
    ss->ss3.pitch = w_pitch - 1;
    gen8_gpe_set_surface_tiling(ss, tiling);
}

void
gen8_gpe_media_rw_surface_setup(VADriverContextP ctx,
                                struct i965_gpe_context *gpe_context,
                                struct object_surface *obj_surface,
                                unsigned long binding_table_offset,
                                unsigned long surface_state_offset)
{
    struct gen8_surface_state *ss;
    dri_bo *bo;

    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, True);
    assert(bo->virtual);

    ss = (struct gen8_surface_state *)((char *)bo->virtual + surface_state_offset);
    gen8_gpe_set_media_rw_surface_state(ctx, obj_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      0,
                      surface_state_offset + offsetof(struct gen8_surface_state, ss8),
                      obj_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}

void
gen8_gpe_media_chroma_surface_setup(VADriverContextP ctx,
                                struct i965_gpe_context *gpe_context,
                                struct object_surface *obj_surface,
                                unsigned long binding_table_offset,
                                unsigned long surface_state_offset)
{
    struct gen8_surface_state *ss;
    dri_bo *bo;
    int cbcr_offset;

	assert(obj_surface->fourcc == VA_FOURCC_NV12);
    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, True);
    assert(bo->virtual);

    cbcr_offset = obj_surface->height * obj_surface->width;
    ss = (struct gen8_surface_state *)((char *)bo->virtual + surface_state_offset);
    gen8_gpe_set_media_chroma_surface_state(ctx, obj_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      cbcr_offset,
                      surface_state_offset + offsetof(struct gen8_surface_state, ss8),
                      obj_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}


static void
gen8_gpe_set_buffer_surface_state(VADriverContextP ctx,
                                  struct i965_buffer_surface *buffer_surface,
                                  struct gen8_surface_state *ss)
{
    int num_entries;

    assert(buffer_surface->bo);
    num_entries = buffer_surface->num_blocks * buffer_surface->size_block / buffer_surface->pitch;

    memset(ss, 0, sizeof(*ss));
    /* ss0 */
    ss->ss0.surface_type = I965_SURFACE_BUFFER;
    /* ss1 */
    ss->ss8.base_addr = buffer_surface->bo->offset;
    /* ss2 */
    ss->ss2.width = ((num_entries - 1) & 0x7f);
    ss->ss2.height = (((num_entries - 1) >> 7) & 0x3fff);
    /* ss3 */
    ss->ss3.depth = (((num_entries - 1) >> 21) & 0x3f);
    ss->ss3.pitch = buffer_surface->pitch - 1;
}

void
gen8_gpe_buffer_suface_setup(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct i965_buffer_surface *buffer_surface,
                             unsigned long binding_table_offset,
                             unsigned long surface_state_offset)
{
    struct gen8_surface_state *ss;
    dri_bo *bo;

    bo = gpe_context->surface_state_binding_table.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);

    ss = (struct gen8_surface_state *)((char *)bo->virtual + surface_state_offset);
    gen8_gpe_set_buffer_surface_state(ctx, buffer_surface, ss);
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
                      0,
                      surface_state_offset + offsetof(struct gen8_surface_state, ss8),
                      buffer_surface->bo);

    *((unsigned int *)((char *)bo->virtual + binding_table_offset)) = surface_state_offset;
    dri_bo_unmap(bo);
}

static void
gen8_gpe_state_base_address(VADriverContextP ctx,
                            struct i965_gpe_context *gpe_context,
                            struct intel_batchbuffer *batch)
{
    BEGIN_BATCH(batch, 16);

    OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | 14);

    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);				//General State Base Address
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);

	/*DW4 Surface state base address */
    OUT_RELOC(batch, gpe_context->surface_state_binding_table.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY); /* Surface state base address */
    OUT_BATCH(batch, 0);

	/*DW6. Dynamic state base address */
    if (gpe_context->dynamic_state.bo)
        OUT_RELOC(batch, gpe_context->dynamic_state.bo,
                  I915_GEM_DOMAIN_RENDER | I915_GEM_DOMAIN_SAMPLER,
                  0, BASE_ADDRESS_MODIFY);
    else
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);

    OUT_BATCH(batch, 0);

	/*DW8. Indirect Object base address */
    if (gpe_context->indirect_state.bo)
        OUT_RELOC(batch, gpe_context->indirect_state.bo,
                  I915_GEM_DOMAIN_SAMPLER,
                  0, BASE_ADDRESS_MODIFY);
    else
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);

    OUT_BATCH(batch, 0);

	/*DW10. Instruct base address */
    if (gpe_context->instruction_state.bo)
        OUT_RELOC(batch, gpe_context->instruction_state.bo,
                  I915_GEM_DOMAIN_INSTRUCTION,
                  0, BASE_ADDRESS_MODIFY);
    else
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);

    OUT_BATCH(batch, 0);

	/* DW12. Size limitation */
    OUT_BATCH(batch, 0xFFFFF000 | BASE_ADDRESS_MODIFY);		//General State Access Upper Bound	
    OUT_BATCH(batch, 0xFFFFF000 | BASE_ADDRESS_MODIFY);		//Dynamic State Access Upper Bound
    OUT_BATCH(batch, 0xFFFFF000 | BASE_ADDRESS_MODIFY);		//Indirect Object Access Upper Bound
    OUT_BATCH(batch, 0xFFFFF000 | BASE_ADDRESS_MODIFY);		//Instruction Access Upper Bound

    /*
      OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);				//LLC Coherent Base Address
      OUT_BATCH(batch, 0xFFFFF000 | BASE_ADDRESS_MODIFY );		//LLC Coherent Upper Bound
    */

    ADVANCE_BATCH(batch);
}

static void
gen8_gpe_vfe_state(VADriverContextP ctx,
                   struct i965_gpe_context *gpe_context,
                   struct intel_batchbuffer *batch)
{

    BEGIN_BATCH(batch, 9);

    OUT_BATCH(batch, CMD_MEDIA_VFE_STATE | (9 - 2));
    /* Scratch Space Base Pointer and Space */
    OUT_BATCH(batch, 0);    
    OUT_BATCH(batch, 0);

    OUT_BATCH(batch,
              gpe_context->vfe_state.max_num_threads << 16 |    /* Maximum Number of Threads */
              gpe_context->vfe_state.num_urb_entries << 8 |     /* Number of URB Entries */
              gpe_context->vfe_state.gpgpu_mode << 2);          /* MEDIA Mode */
    OUT_BATCH(batch, 0);                                        /* Debug: Object ID */
    OUT_BATCH(batch,
              gpe_context->vfe_state.urb_entry_size << 16 |     /* URB Entry Allocation Size */
              gpe_context->vfe_state.curbe_allocation_size);    /* CURBE Allocation Size */

    /* the vfe_desc5/6/7 will decide whether the scoreboard is used. */
    OUT_BATCH(batch, gpe_context->vfe_desc5.dword);                                        
    OUT_BATCH(batch, gpe_context->vfe_desc6.dword);                                       
    OUT_BATCH(batch, gpe_context->vfe_desc7.dword);                                       
	
    ADVANCE_BATCH(batch);

}


static void
gen8_gpe_curbe_load(VADriverContextP ctx,
                    struct i965_gpe_context *gpe_context,
                    struct intel_batchbuffer *batch)
{
    BEGIN_BATCH(batch, 4);

    OUT_BATCH(batch, CMD_MEDIA_CURBE_LOAD | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, gpe_context->curbe_size);
    OUT_BATCH(batch, gpe_context->curbe_offset);

    ADVANCE_BATCH(batch);
}

static void
gen8_gpe_idrt(VADriverContextP ctx,
              struct i965_gpe_context *gpe_context,
              struct intel_batchbuffer *batch)
{
    BEGIN_BATCH(batch, 6);

    OUT_BATCH(batch, CMD_MEDIA_STATE_FLUSH);
    OUT_BATCH(batch, 0);

    OUT_BATCH(batch, CMD_MEDIA_INTERFACE_LOAD | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, gpe_context->idrt_size);
    OUT_BATCH(batch, gpe_context->idrt_offset);

    ADVANCE_BATCH(batch);
}


void
gen8_gpe_pipeline_setup(VADriverContextP ctx,
                        struct i965_gpe_context *gpe_context,
                        struct intel_batchbuffer *batch)
{
    intel_batchbuffer_emit_mi_flush(batch);

    i965_gpe_select(ctx, gpe_context, batch);
    gen8_gpe_state_base_address(ctx, gpe_context, batch);
    gen8_gpe_vfe_state(ctx, gpe_context, batch);
    gen8_gpe_curbe_load(ctx, gpe_context, batch);
    gen8_gpe_idrt(ctx, gpe_context, batch);
}

void
gen8_gpe_context_init(VADriverContextP ctx,
                      struct i965_gpe_context *gpe_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    dri_bo *bo;
    int bo_size;
    unsigned int start_offset, end_offset;

    dri_bo_unreference(gpe_context->surface_state_binding_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      gpe_context->surface_state_binding_table.length,
                      4096);
    assert(bo);
    gpe_context->surface_state_binding_table.bo = bo;

    bo_size = gpe_context->idrt_size + gpe_context->curbe_size + gpe_context->sampler_size + 192;
    dri_bo_unreference(gpe_context->dynamic_state.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      bo_size,
                      4096);
    assert(bo);
    gpe_context->dynamic_state.bo = bo;
    gpe_context->dynamic_state.bo_size = bo_size;

    end_offset = 0;
    gpe_context->dynamic_state.end_offset = 0;

    /* Constant buffer offset */
    start_offset = ALIGN(end_offset, 64);
    gpe_context->curbe_offset = start_offset;
    end_offset = start_offset + gpe_context->curbe_size;

    /* Interface descriptor offset */
    start_offset = ALIGN(end_offset, 64);
    gpe_context->idrt_offset = start_offset;
    end_offset = start_offset + gpe_context->idrt_size;

    /* Sampler state offset */
    start_offset = ALIGN(end_offset, 64);
    gpe_context->sampler_offset = start_offset;
    end_offset = start_offset + gpe_context->sampler_size;

    /* update the end offset of dynamic_state */
    gpe_context->dynamic_state.end_offset = end_offset;
}


void
gen8_gpe_context_destroy(struct i965_gpe_context *gpe_context)
{
    dri_bo_unreference(gpe_context->surface_state_binding_table.bo);
    gpe_context->surface_state_binding_table.bo = NULL;

    dri_bo_unreference(gpe_context->instruction_state.bo);
    gpe_context->instruction_state.bo = NULL;

    dri_bo_unreference(gpe_context->dynamic_state.bo);
    gpe_context->dynamic_state.bo = NULL;

    dri_bo_unreference(gpe_context->indirect_state.bo);
    gpe_context->indirect_state.bo = NULL;

}


void
gen8_gpe_load_kernels(VADriverContextP ctx,
                      struct i965_gpe_context *gpe_context,
                      struct i965_kernel *kernel_list,
                      unsigned int num_kernels)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i, kernel_size;
    unsigned int kernel_offset, end_offset;
    unsigned char *kernel_ptr;
    struct i965_kernel *kernel;

    assert(num_kernels <= MAX_GPE_KERNELS);
    memcpy(gpe_context->kernels, kernel_list, sizeof(*kernel_list) * num_kernels);
    gpe_context->num_kernels = num_kernels;

    kernel_size = num_kernels * 64;
    for (i = 0; i < num_kernels; i++) {
        kernel = &gpe_context->kernels[i];

        kernel_size += kernel->size;
    }

    gpe_context->instruction_state.bo = dri_bo_alloc(i965->intel.bufmgr,
                                  "kernel shader",
                                  kernel_size,
                                  0x1000);
    if (gpe_context->instruction_state.bo == NULL) {
        WARN_ONCE("failure to allocate the buffer space for kernel shader\n");
        return;
    }

    assert(gpe_context->instruction_state.bo);

    gpe_context->instruction_state.bo_size = kernel_size;
    gpe_context->instruction_state.end_offset = 0;
    end_offset = 0;

    dri_bo_map(gpe_context->instruction_state.bo, 1);
    kernel_ptr = (unsigned char *)(gpe_context->instruction_state.bo->virtual);
    for (i = 0; i < num_kernels; i++) {
        kernel_offset = ALIGN(end_offset, 64);
        kernel = &gpe_context->kernels[i];
        kernel->kernel_offset = kernel_offset;

        if (kernel->size) {
            memcpy(kernel_ptr + kernel_offset, kernel->bin, kernel->size);

            end_offset = kernel_offset + kernel->size;
        }
    }

    gpe_context->instruction_state.end_offset = end_offset;

    dri_bo_unmap(gpe_context->instruction_state.bo);

    return;
}

static void
gen9_gpe_state_base_address(VADriverContextP ctx,
                            struct i965_gpe_context *gpe_context,
                            struct intel_batchbuffer *batch)
{
    BEGIN_BATCH(batch, 19);

    OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | (19 - 2));

    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);				//General State Base Address
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);

	/*DW4 Surface state base address */
    OUT_RELOC(batch, gpe_context->surface_state_binding_table.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY); /* Surface state base address */
    OUT_BATCH(batch, 0);

	/*DW6. Dynamic state base address */
    if (gpe_context->dynamic_state.bo)
        OUT_RELOC(batch, gpe_context->dynamic_state.bo,
                  I915_GEM_DOMAIN_RENDER | I915_GEM_DOMAIN_SAMPLER,
                  0, BASE_ADDRESS_MODIFY);
    else
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);

    OUT_BATCH(batch, 0);

	/*DW8. Indirect Object base address */
    if (gpe_context->indirect_state.bo)
        OUT_RELOC(batch, gpe_context->indirect_state.bo,
                  I915_GEM_DOMAIN_SAMPLER,
                  0, BASE_ADDRESS_MODIFY);
    else
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);

    OUT_BATCH(batch, 0);

	/*DW10. Instruct base address */
    if (gpe_context->instruction_state.bo)
        OUT_RELOC(batch, gpe_context->instruction_state.bo,
                  I915_GEM_DOMAIN_INSTRUCTION,
                  0, BASE_ADDRESS_MODIFY);
    else
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);

    OUT_BATCH(batch, 0);

	/* DW12. Size limitation */
    OUT_BATCH(batch, 0xFFFFF000 | BASE_ADDRESS_MODIFY);		//General State Access Upper Bound
    OUT_BATCH(batch, 0xFFFFF000 | BASE_ADDRESS_MODIFY);		//Dynamic State Access Upper Bound
    OUT_BATCH(batch, 0xFFFFF000 | BASE_ADDRESS_MODIFY);		//Indirect Object Access Upper Bound
    OUT_BATCH(batch, 0xFFFFF000 | BASE_ADDRESS_MODIFY);		//Instruction Access Upper Bound

    /* the bindless surface state address */
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0xFFFFF000);

    ADVANCE_BATCH(batch);
}

static void
gen9_gpe_select(VADriverContextP ctx,
                struct i965_gpe_context *gpe_context,
                struct intel_batchbuffer *batch)
{
    BEGIN_BATCH(batch, 1);
    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_MEDIA |
                     GEN9_PIPELINE_SELECTION_MASK |
                     GEN9_MEDIA_DOP_GATE_OFF |
                     GEN9_MEDIA_DOP_GATE_MASK |
                     GEN9_FORCE_MEDIA_AWAKE_ON |
                     GEN9_FORCE_MEDIA_AWAKE_MASK);
    ADVANCE_BATCH(batch);
}

void
gen9_gpe_pipeline_setup(VADriverContextP ctx,
                        struct i965_gpe_context *gpe_context,
                        struct intel_batchbuffer *batch)
{
    intel_batchbuffer_emit_mi_flush(batch);

    gen9_gpe_select(ctx, gpe_context, batch);
    gen9_gpe_state_base_address(ctx, gpe_context, batch);
    gen8_gpe_vfe_state(ctx, gpe_context, batch);
    gen8_gpe_curbe_load(ctx, gpe_context, batch);
    gen8_gpe_idrt(ctx, gpe_context, batch);
}

void
gen9_gpe_pipeline_end(VADriverContextP ctx,
                      struct i965_gpe_context *gpe_context,
                      struct intel_batchbuffer *batch)
{
    BEGIN_BATCH(batch, 1);
    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_MEDIA |
              GEN9_PIPELINE_SELECTION_MASK |
              GEN9_MEDIA_DOP_GATE_ON |
              GEN9_MEDIA_DOP_GATE_MASK |
              GEN9_FORCE_MEDIA_AWAKE_OFF |
              GEN9_FORCE_MEDIA_AWAKE_MASK);
    ADVANCE_BATCH(batch);
}
