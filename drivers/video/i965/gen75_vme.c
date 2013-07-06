/*
 * Copyright © 2010-2011 Intel Corporation
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
 *    Zhao Yakui <yakui.zhao@intel.com>
 *    Xiang HaiHao <haihao.xiang@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"

#include "i965_defines.h"
#include "i965_drv_video.h"
#include "gen6_vme.h"
#include "i965_encoder.h"

#define SURFACE_STATE_PADDED_SIZE_0_GEN7        ALIGN(sizeof(struct gen7_surface_state), 32)
#define SURFACE_STATE_PADDED_SIZE_1_GEN7        ALIGN(sizeof(struct gen7_surface_state2), 32)
#define SURFACE_STATE_PADDED_SIZE_GEN7          MAX(SURFACE_STATE_PADDED_SIZE_0_GEN7, SURFACE_STATE_PADDED_SIZE_1_GEN7)

#define SURFACE_STATE_PADDED_SIZE_0_GEN6        ALIGN(sizeof(struct i965_surface_state), 32)
#define SURFACE_STATE_PADDED_SIZE_1_GEN6        ALIGN(sizeof(struct i965_surface_state2), 32)
#define SURFACE_STATE_PADDED_SIZE_GEN6          MAX(SURFACE_STATE_PADDED_SIZE_0_GEN6, SURFACE_STATE_PADDED_SIZE_1_GEN7)

#define SURFACE_STATE_PADDED_SIZE               MAX(SURFACE_STATE_PADDED_SIZE_GEN6, SURFACE_STATE_PADDED_SIZE_GEN7)
#define SURFACE_STATE_OFFSET(index)             (SURFACE_STATE_PADDED_SIZE * index)
#define BINDING_TABLE_OFFSET                    SURFACE_STATE_OFFSET(MAX_MEDIA_SURFACES_GEN6)

#define VME_INTRA_SHADER	0	
#define VME_INTER_SHADER	1

#define CURBE_ALLOCATION_SIZE   37              /* in 256-bit */
#define CURBE_TOTAL_DATA_LENGTH (4 * 32)        /* in byte, it should be less than or equal to CURBE_ALLOCATION_SIZE * 32 */
#define CURBE_URB_ENTRY_LENGTH  4               /* in 256-bit, it should be less than or equal to CURBE_TOTAL_DATA_LENGTH / 32 */

#define VME_MSG_LENGTH		32
  
static const uint32_t gen75_vme_intra_frame[][4] = {
#include "shaders/vme/intra_frame_haswell.g75b"
};

static const uint32_t gen75_vme_inter_frame[][4] = {
#include "shaders/vme/inter_frame_haswell.g75b"
};

static struct i965_kernel gen75_vme_kernels[] = {
    {
        "VME Intra Frame",
        VME_INTRA_SHADER,										/*index*/
        gen75_vme_intra_frame, 			
        sizeof(gen75_vme_intra_frame),		
        NULL
    },
    {
        "VME inter Frame",
        VME_INTER_SHADER,
        gen75_vme_inter_frame,
        sizeof(gen75_vme_inter_frame),
        NULL
    }
};

/*
 * Surface state for IvyBridge
 */
static
void gen75_vme_set_common_surface_tiling(struct gen7_surface_state *ss, unsigned int tiling)
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
gen75_vme_set_source_surface_tiling(struct gen7_surface_state2 *ss, unsigned int tiling)
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


/* only used for VME source surface state */
static void gen75_vme_source_surface_state(VADriverContextP ctx,
                                          int index,
                                          struct object_surface *obj_surface,
                                          struct gen6_encoder_context *gen6_encoder_context)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;
    struct gen7_surface_state2 *ss;
    dri_bo *bo;
    int w, h, w_pitch, h_pitch;
    unsigned int tiling, swizzle;

    assert(obj_surface->bo);

    w = obj_surface->orig_width;
    h = obj_surface->orig_height;
    w_pitch = obj_surface->width;
    h_pitch = obj_surface->height;

    bo = vme_context->surface_state_binding_table.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);

    ss = (struct gen7_surface_state2 *)((char *)bo->virtual + SURFACE_STATE_OFFSET(index));
    memset(ss, 0, sizeof(*ss));

    ss->ss0.surface_base_address = obj_surface->bo->offset;

    ss->ss1.cbcr_pixel_offset_v_direction = 2;
    ss->ss1.width = w - 1;
    ss->ss1.height = h - 1;

    ss->ss2.surface_format = MFX_SURFACE_PLANAR_420_8;
    ss->ss2.interleave_chroma = 1;
    ss->ss2.pitch = w_pitch - 1;
    ss->ss2.half_pitch_for_chroma = 0;

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    gen75_vme_set_source_surface_tiling(ss, tiling);

    /* UV offset for interleave mode */
    ss->ss3.x_offset_for_cb = 0;
    ss->ss3.y_offset_for_cb = h_pitch;

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      0,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct gen7_surface_state2, ss0),
                      obj_surface->bo);

    ((unsigned int *)((char *)bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(bo);
}

static void
gen75_vme_media_source_surface_state(VADriverContextP ctx,
                                    int index,
                                    struct object_surface *obj_surface,
                                    struct gen6_encoder_context *gen6_encoder_context)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;
    struct gen7_surface_state *ss;
    dri_bo *bo;
    int w, h, w_pitch;
    unsigned int tiling, swizzle;

    /* Y plane */
    w = obj_surface->orig_width;
    h = obj_surface->orig_height;
    w_pitch = obj_surface->width;

    bo = vme_context->surface_state_binding_table.bo;
    dri_bo_map(bo, True);
    assert(bo->virtual);

    ss = (struct gen7_surface_state *)((char *)bo->virtual + SURFACE_STATE_OFFSET(index));
    memset(ss, 0, sizeof(*ss));

    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = I965_SURFACEFORMAT_R8_UNORM;

    ss->ss1.base_addr = obj_surface->bo->offset;

    ss->ss2.width = w / 4 - 1;
    ss->ss2.height = h - 1;

    ss->ss3.pitch = w_pitch - 1;

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    gen75_vme_set_common_surface_tiling(ss, tiling);

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      0,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct gen7_surface_state, ss1),
                      obj_surface->bo);

    ((unsigned int *)((char *)bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(bo);
}

static VAStatus
gen75_vme_output_buffer_setup(VADriverContextP ctx,
                             struct encode_state *encode_state,
                             int index,
                             struct gen6_encoder_context *gen6_encoder_context)

{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;
    struct gen7_surface_state *ss;
    dri_bo *bo;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param->buffer;
    VAEncSliceParameterBuffer *pSliceParameter = (VAEncSliceParameterBuffer *)encode_state->slice_params[0]->buffer;
    int is_intra = pSliceParameter->slice_flags.bits.is_intra;
    int width_in_mbs = pSequenceParameter->picture_width_in_mbs;
    int height_in_mbs = pSequenceParameter->picture_height_in_mbs;
    int num_entries;

    if ( is_intra ) {
        vme_context->vme_output.size_block = INTRA_VME_OUTPUT_IN_BYTES * 2;
    } else {
        vme_context->vme_output.size_block = INTRA_VME_OUTPUT_IN_BYTES * 24;
	/*
	 * Inter MV . 32-byte Intra search + 16 IME info + 128 IME MV + 32 IME Ref
	 * + 16 FBR Info + 128 FBR MV + 32 FBR Ref.
	 * 16 * (2 + 2 * (1 + 8 + 2))= 16 * 24.
	 */
    }
    vme_context->vme_output.num_blocks = width_in_mbs * height_in_mbs;
    vme_context->vme_output.pitch = 16;
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "VME output buffer",
                      vme_context->vme_output.num_blocks * vme_context->vme_output.size_block,
                      0x1000);
    assert(bo);
    vme_context->vme_output.bo = bo;

    bo = vme_context->surface_state_binding_table.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);

    ss = (struct gen7_surface_state *)((char *)bo->virtual + SURFACE_STATE_OFFSET(index));
    memset(ss, 0, sizeof(*ss));

    /* always use 16 bytes as pitch on Sandy Bridge */
    num_entries = vme_context->vme_output.num_blocks * vme_context->vme_output.size_block / 16;

    ss->ss0.surface_type = I965_SURFACE_BUFFER;

    ss->ss1.base_addr = vme_context->vme_output.bo->offset;

    ss->ss2.width = ((num_entries - 1) & 0x7f);
    ss->ss2.height = (((num_entries - 1) >> 7) & 0x3fff);
    ss->ss3.depth = (((num_entries - 1) >> 21) & 0x3f);

    ss->ss3.pitch = vme_context->vme_output.pitch - 1;

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
                      0,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct gen7_surface_state, ss1),
                      vme_context->vme_output.bo);

    ((unsigned int *)((char *)bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(bo);

    return VA_STATUS_SUCCESS;
}

static VAStatus gen75_vme_surface_setup(VADriverContextP ctx, 
                                       struct encode_state *encode_state,
                                       int is_intra,
                                       struct gen6_encoder_context *gen6_encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_surface *obj_surface;
    VAEncPictureParameterBufferH264 *pPicParameter = (VAEncPictureParameterBufferH264 *)encode_state->pic_param->buffer;

    /*Setup surfaces state*/
    /* current picture for encoding */
    obj_surface = SURFACE(encode_state->current_render_target);
    assert(obj_surface);
    gen75_vme_source_surface_state(ctx, 0, obj_surface, gen6_encoder_context);
    gen75_vme_media_source_surface_state(ctx, 4, obj_surface, gen6_encoder_context);

    if ( ! is_intra ) {
        /* reference 0 */
        obj_surface = SURFACE(pPicParameter->reference_picture);
        assert(obj_surface);
        gen75_vme_source_surface_state(ctx, 1, obj_surface, gen6_encoder_context);
        /* reference 1, FIXME: */
        // obj_surface = SURFACE(pPicParameter->reference_picture);
        // assert(obj_surface);
        //gen7_vme_source_surface_state(ctx, 2, obj_surface);
    }

    /* VME output */
    gen75_vme_output_buffer_setup(ctx, encode_state, 3, gen6_encoder_context);

    return VA_STATUS_SUCCESS;
}

static VAStatus gen75_vme_interface_setup(VADriverContextP ctx, 
                                         struct encode_state *encode_state,
                                         struct gen6_encoder_context *gen6_encoder_context)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;
    struct gen6_interface_descriptor_data *desc;   
    int i;
    dri_bo *bo;

    bo = vme_context->idrt.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);
    desc = bo->virtual;

    for (i = 0; i < GEN6_VME_KERNEL_NUMBER; i++) {
        struct i965_kernel *kernel;
        kernel = &vme_context->vme_kernels[i];
        assert(sizeof(*desc) == 32);
        /*Setup the descritor table*/
        memset(desc, 0, sizeof(*desc));
        desc->desc0.kernel_start_pointer = (kernel->bo->offset >> 6);
        desc->desc2.sampler_count = 0; /* FIXME: */
        desc->desc2.sampler_state_pointer = 0;
        desc->desc3.binding_table_entry_count = 1; /* FIXME: */
        desc->desc3.binding_table_pointer = (BINDING_TABLE_OFFSET >> 5);
        desc->desc4.constant_urb_entry_read_offset = 0;
        desc->desc4.constant_urb_entry_read_length = CURBE_URB_ENTRY_LENGTH;
 		
        /*kernel start*/
        dri_bo_emit_reloc(bo,	
                          I915_GEM_DOMAIN_INSTRUCTION, 0,
                          0,
                          i * sizeof(*desc) + offsetof(struct gen6_interface_descriptor_data, desc0),
                          kernel->bo);
        desc++;
    }
    dri_bo_unmap(bo);

    return VA_STATUS_SUCCESS;
}

static VAStatus gen75_vme_constant_setup(VADriverContextP ctx, 
                                        struct encode_state *encode_state,
                                        struct gen6_encoder_context *gen6_encoder_context)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;
    unsigned char *constant_buffer;

    dri_bo_map(vme_context->curbe.bo, 1);
    assert(vme_context->curbe.bo->virtual);
    constant_buffer = vme_context->curbe.bo->virtual;
	
	/* VME MV/Mb cost table is passed by using const buffer */
	/* Now it uses the fixed search path. So it is constructed directly
	 * in the GPU shader.
	 */
    memcpy(constant_buffer, (char *)vme_context->vme_state_message, 32);

    dri_bo_unmap( vme_context->curbe.bo);

    return VA_STATUS_SUCCESS;
}

static VAStatus gen75_vme_vme_state_setup(VADriverContextP ctx,
                                         struct encode_state *encode_state,
                                         int is_intra,
                                         struct gen6_encoder_context *gen6_encoder_context)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;
    unsigned int *vme_state_message;
    int i;
	
    //building VME state message
    //pass the MV/Mb cost into VME message on HASWell
    assert(vme_context->vme_state_message);
    vme_state_message = (unsigned int *)vme_context->vme_state_message;

    vme_state_message[0] = 0x4a4a4a4a;
    vme_state_message[1] = 0x4a4a4a4a;
    vme_state_message[2] = 0x4a4a4a4a;
    vme_state_message[3] = 0x22120200;
    vme_state_message[4] = 0x62524232;

    for (i=5; i < 8; i++) {
	vme_state_message[i] = 0;
     }

    return VA_STATUS_SUCCESS;
}

static void gen75_vme_pipeline_select(VADriverContextP ctx,
                                      struct gen6_encoder_context *gen6_encoder_context,
                                      struct intel_batchbuffer *batch)
{
    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BATCH(batch, 1);
    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_MEDIA);
    ADVANCE_BATCH(batch);
}

static void gen75_vme_state_base_address(VADriverContextP ctx,
                                         struct gen6_encoder_context *gen6_encoder_context,
                                         struct intel_batchbuffer *batch)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BATCH(batch, 10);

    OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | 8);

    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);				//General State Base Address
    OUT_RELOC(batch, vme_context->surface_state_binding_table.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY); /* Surface state base address */
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);				//Dynamic State Base Address
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);				//Indirect Object Base Address
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);				//Instruction Base Address

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

static void gen75_vme_vfe_state(VADriverContextP ctx,
                                struct gen6_encoder_context *gen6_encoder_context,
                                struct intel_batchbuffer *batch)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BATCH(batch, 8);

    OUT_BATCH(batch, CMD_MEDIA_VFE_STATE | 6);					/*Gen6 CMD_MEDIA_STATE_POINTERS = CMD_MEDIA_STATE */
    OUT_BATCH(batch, 0);												/*Scratch Space Base Pointer and Space*/
    OUT_BATCH(batch, (vme_context->vfe_state.max_num_threads << 16) 
              | (vme_context->vfe_state.num_urb_entries << 8) 
              | (vme_context->vfe_state.gpgpu_mode << 2) );	/*Maximum Number of Threads , Number of URB Entries, MEDIA Mode*/
    OUT_BATCH(batch, 0);												/*Debug: Object ID*/
    OUT_BATCH(batch, (vme_context->vfe_state.urb_entry_size << 16) 
              | vme_context->vfe_state.curbe_allocation_size);				/*URB Entry Allocation Size , CURBE Allocation Size*/
    OUT_BATCH(batch, 0);											/*Disable Scoreboard*/
    OUT_BATCH(batch, 0);											/*Disable Scoreboard*/
    OUT_BATCH(batch, 0);											/*Disable Scoreboard*/
	
    ADVANCE_BATCH(batch);

}

static void gen75_vme_curbe_load(VADriverContextP ctx,
                                 struct gen6_encoder_context *gen6_encoder_context,
                                 struct intel_batchbuffer *batch)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BATCH(batch, 4);

    OUT_BATCH(batch, CMD_MEDIA_CURBE_LOAD | 2);
    OUT_BATCH(batch, 0);

    OUT_BATCH(batch, CURBE_TOTAL_DATA_LENGTH);
    OUT_RELOC(batch, vme_context->curbe.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    ADVANCE_BATCH(batch);
}

static void gen75_vme_idrt(VADriverContextP ctx,
                           struct gen6_encoder_context *gen6_encoder_context,
                           struct intel_batchbuffer *batch)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BATCH(batch, 4);

    OUT_BATCH(batch, CMD_MEDIA_INTERFACE_LOAD | 2);	
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, GEN6_VME_KERNEL_NUMBER * sizeof(struct gen6_interface_descriptor_data));
    OUT_RELOC(batch, vme_context->idrt.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    ADVANCE_BATCH(batch);
}

static int gen75_vme_media_object(VADriverContextP ctx, 
                                  struct encode_state *encode_state,
                                  int mb_x, int mb_y,
                                  int kernel, unsigned int mb_intra_ub,
                                  struct gen6_encoder_context *gen6_encoder_context,
                                  struct intel_batchbuffer *batch)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_surface *obj_surface = SURFACE(encode_state->current_render_target);
    int mb_width = ALIGN(obj_surface->orig_width, 16) / 16;
    int len_in_dowrds = 8;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BATCH(batch, len_in_dowrds);
    
    OUT_BATCH(batch, CMD_MEDIA_OBJECT | (len_in_dowrds - 2));
    OUT_BATCH(batch, kernel);		/*Interface Descriptor Offset*/	
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
   
    /*inline data */
    OUT_BATCH(batch, mb_width << 16 | mb_y << 8 | mb_x);			/*M0.0 Refrence0 X,Y, not used in Intra*/

 	OUT_BATCH(batch, ((mb_intra_ub << 8) | 0));
    ADVANCE_BATCH(batch);

    return len_in_dowrds * 4;
}

static void gen75_vme_media_init(VADriverContextP ctx, struct gen6_encoder_context *gen6_encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;
    dri_bo *bo;

    /* constant buffer */
    dri_bo_unreference(vme_context->curbe.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      CURBE_TOTAL_DATA_LENGTH, 64);
    assert(bo);
    vme_context->curbe.bo = bo;

    dri_bo_unreference(vme_context->surface_state_binding_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      (SURFACE_STATE_PADDED_SIZE + sizeof(unsigned int)) * MAX_MEDIA_SURFACES_GEN6,
                      4096);
    assert(bo);
    vme_context->surface_state_binding_table.bo = bo;

    /* interface descriptor remapping table */
    dri_bo_unreference(vme_context->idrt.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "Buffer", 
                      MAX_INTERFACE_DESC_GEN6 * sizeof(struct gen6_interface_descriptor_data), 16);
    assert(bo);
    vme_context->idrt.bo = bo;

    /* VME output buffer */
    dri_bo_unreference(vme_context->vme_output.bo);
    vme_context->vme_output.bo = NULL;

    /* VME state */
    dri_bo_unreference(vme_context->vme_state.bo);
    vme_context->vme_state.bo = NULL;

    vme_context->vfe_state.max_num_threads = 60 - 1;
    vme_context->vfe_state.num_urb_entries = 16;
    vme_context->vfe_state.gpgpu_mode = 0;
    vme_context->vfe_state.urb_entry_size = 59 - 1;
    vme_context->vfe_state.curbe_allocation_size = CURBE_ALLOCATION_SIZE - 1;
}

#define		INTRA_PRED_AVAIL_FLAG_AE	0x60
#define		INTRA_PRED_AVAIL_FLAG_B		0x10
#define		INTRA_PRED_AVAIL_FLAG_C       	0x8
#define		INTRA_PRED_AVAIL_FLAG_D		0x4
#define		INTRA_PRED_AVAIL_FLAG_BCD_MASK	0x1C

static void gen75_vme_pipeline_programing(VADriverContextP ctx, 
                                         struct encode_state *encode_state,
                                         struct gen6_encoder_context *gen6_encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *main_batch = gen6_encoder_context->base.batch;
    VAEncSliceParameterBuffer *pSliceParameter = (VAEncSliceParameterBuffer *)encode_state->slice_params[0]->buffer;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param->buffer;
    int is_intra = pSliceParameter->slice_flags.bits.is_intra;
    int width_in_mbs = pSequenceParameter->picture_width_in_mbs;
    int height_in_mbs = pSequenceParameter->picture_height_in_mbs;
    int emit_new_state = 1, object_len_in_bytes;
    int x, y;
    unsigned int mb_intra_ub; 
    struct intel_batchbuffer *batch = intel_batchbuffer_new(&i965->intel, I915_EXEC_RENDER, width_in_mbs * height_in_mbs * 8 * 4 + 0x200);

    intel_batchbuffer_start_atomic(batch, width_in_mbs * height_in_mbs * 8 * 4 + 0x100);

    for(y = 0; y < height_in_mbs; y++){
        for(x = 0; x < width_in_mbs; x++){	
	    mb_intra_ub = 0;
	    if (x != 0) {
		mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_AE;
	    }
	    if (y != 0) {
		mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_B;
		if (x != 0)
			mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_D;
		if (x != (width_in_mbs -1))
			mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_C;
	    }

            if (emit_new_state) {
                /*Step1: MI_FLUSH/PIPE_CONTROL*/
                intel_batchbuffer_emit_mi_flush(batch);

                /*Step2: State command PIPELINE_SELECT*/
                gen75_vme_pipeline_select(ctx, gen6_encoder_context, batch);

                /*Step3: State commands configuring pipeline states*/
                gen75_vme_state_base_address(ctx, gen6_encoder_context, batch);
                gen75_vme_vfe_state(ctx, gen6_encoder_context, batch);
                gen75_vme_curbe_load(ctx, gen6_encoder_context, batch);
                gen75_vme_idrt(ctx, gen6_encoder_context, batch);

                emit_new_state = 0;
            }

            /*Step4: Primitive commands*/
            object_len_in_bytes = gen75_vme_media_object(ctx, encode_state, x, y, is_intra ? VME_INTRA_SHADER : VME_INTER_SHADER, mb_intra_ub, gen6_encoder_context, batch);

            if (intel_batchbuffer_check_free_space(batch, object_len_in_bytes) == 0) {
                assert(0);
                intel_batchbuffer_end_atomic(batch);	
                intel_batchbuffer_flush(batch);
                emit_new_state = 1;
                intel_batchbuffer_start_atomic(batch, 0x1000);
            }
        }
    }

    intel_batchbuffer_align(batch, 8);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, MI_BATCH_BUFFER_END);
    ADVANCE_BATCH(batch);

    intel_batchbuffer_end_atomic(batch);

    /* chain to the main batch buffer */
    intel_batchbuffer_start_atomic(main_batch, 0x100);
    intel_batchbuffer_emit_mi_flush(main_batch);
    BEGIN_BATCH(main_batch, 2);
    OUT_BATCH(main_batch, MI_BATCH_BUFFER_START | (2 << 6));
    OUT_RELOC(main_batch,
              batch->buffer,
              I915_GEM_DOMAIN_COMMAND, 0,
              0);
    ADVANCE_BATCH(main_batch);
    intel_batchbuffer_end_atomic(main_batch);

    // end programing             
    intel_batchbuffer_free(batch);
}

static VAStatus gen75_vme_prepare(VADriverContextP ctx, 
                                 struct encode_state *encode_state,
                                 struct gen6_encoder_context *gen6_encoder_context)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    VAEncSliceParameterBuffer *pSliceParameter = (VAEncSliceParameterBuffer *)encode_state->slice_params[0]->buffer;
    int is_intra = pSliceParameter->slice_flags.bits.is_intra;
	
        gen75_vme_surface_setup(ctx, encode_state, is_intra, gen6_encoder_context);

    gen75_vme_interface_setup(ctx, encode_state, gen6_encoder_context);
    gen75_vme_vme_state_setup(ctx, encode_state, is_intra, gen6_encoder_context);
    gen75_vme_constant_setup(ctx, encode_state, gen6_encoder_context);

    /*Programing media pipeline*/
    gen75_vme_pipeline_programing(ctx, encode_state, gen6_encoder_context);

    return vaStatus;
}

static VAStatus gen75_vme_run(VADriverContextP ctx, 
                             struct encode_state *encode_state,
                             struct gen6_encoder_context *gen6_encoder_context)
{
    struct intel_batchbuffer *batch = gen6_encoder_context->base.batch;

    intel_batchbuffer_flush(batch);

    return VA_STATUS_SUCCESS;
}

static VAStatus gen75_vme_stop(VADriverContextP ctx, 
                              struct encode_state *encode_state,
                              struct gen6_encoder_context *gen6_encoder_context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus gen75_vme_pipeline(VADriverContextP ctx,
                           VAProfile profile,
                           struct encode_state *encode_state,
                           struct gen6_encoder_context *gen6_encoder_context)
{
    gen75_vme_media_init(ctx, gen6_encoder_context);
    gen75_vme_prepare(ctx, encode_state, gen6_encoder_context);
    gen75_vme_run(ctx, encode_state, gen6_encoder_context);
    gen75_vme_stop(ctx, encode_state, gen6_encoder_context);

    return VA_STATUS_SUCCESS;
}

Bool gen75_vme_context_init(VADriverContextP ctx, struct gen6_vme_context *vme_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i;

        memcpy(vme_context->vme_kernels, gen75_vme_kernels, sizeof(vme_context->vme_kernels));

    for (i = 0; i < GEN6_VME_KERNEL_NUMBER; i++) {
        /*Load kernel into GPU memory*/	
        struct i965_kernel *kernel = &vme_context->vme_kernels[i];

        kernel->bo = dri_bo_alloc(i965->intel.bufmgr, 
                                  kernel->name, 
                                  kernel->size,
                                  0x1000);
        assert(kernel->bo);
        dri_bo_subdata(kernel->bo, 0, kernel->size, kernel->bin);
    }
    
	vme_context->vme_state_message = malloc(VME_MSG_LENGTH * sizeof(int));
    return True;
}

Bool gen75_vme_context_destroy(struct gen6_vme_context *vme_context)
{
    int i;

    dri_bo_unreference(vme_context->idrt.bo);
    vme_context->idrt.bo = NULL;

    dri_bo_unreference(vme_context->surface_state_binding_table.bo);
    vme_context->surface_state_binding_table.bo = NULL;

    dri_bo_unreference(vme_context->curbe.bo);
    vme_context->curbe.bo = NULL;

    dri_bo_unreference(vme_context->vme_output.bo);
    vme_context->vme_output.bo = NULL;

    dri_bo_unreference(vme_context->vme_state.bo);
    vme_context->vme_state.bo = NULL;

    for (i = 0; i < GEN6_VME_KERNEL_NUMBER; i++) {
        /*Load kernel into GPU memory*/	
        struct i965_kernel *kernel = &vme_context->vme_kernels[i];

        dri_bo_unreference(kernel->bo);
        kernel->bo = NULL;
    }

    if (vme_context->vme_state_message) {
	free(vme_context->vme_state_message);
	vme_context->vme_state_message = NULL;
    }

    return True;
}
