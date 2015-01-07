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
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"

#include "i965_defines.h"
#include "i965_drv_video.h"
#include "i965_encoder.h"
#include "gen6_vme.h"
#include "gen6_mfc.h"
#ifdef SURFACE_STATE_PADDED_SIZE
#undef SURFACE_STATE_PADDED_SIZE
#endif

#define VME_MSG_LENGTH		32

#define SURFACE_STATE_PADDED_SIZE               SURFACE_STATE_PADDED_SIZE_GEN7
#define SURFACE_STATE_OFFSET(index)             (SURFACE_STATE_PADDED_SIZE * index)
#define BINDING_TABLE_OFFSET(index)             (SURFACE_STATE_OFFSET(MAX_MEDIA_SURFACES_GEN6) + sizeof(unsigned int) * index)

#define CURBE_ALLOCATION_SIZE   37              /* in 256-bit */
#define CURBE_TOTAL_DATA_LENGTH (4 * 32)        /* in byte, it should be less than or equal to CURBE_ALLOCATION_SIZE * 32 */
#define CURBE_URB_ENTRY_LENGTH  4               /* in 256-bit, it should be less than or equal to CURBE_TOTAL_DATA_LENGTH / 32 */

enum VIDEO_CODING_TYPE{
    VIDEO_CODING_AVC = 0,
    VIDEO_CODING_MPEG2,
    VIDEO_CODING_SUM
};

enum AVC_VME_KERNEL_TYPE{ 
    AVC_VME_INTRA_SHADER = 0,
    AVC_VME_INTER_SHADER,
    AVC_VME_BATCHBUFFER,
    AVC_VME_BINTER_SHADER,
    AVC_VME_KERNEL_SUM
};

enum MPEG2_VME_KERNEL_TYPE{
    MPEG2_VME_INTER_SHADER = 0,
    MPEG2_VME_BATCHBUFFER,
    MPEG2_VME_KERNEL_SUM
};
 

static const uint32_t gen7_vme_intra_frame[][4] = {
#include "shaders/vme/intra_frame_ivb.g7b"
};

static const uint32_t gen7_vme_inter_frame[][4] = {
#include "shaders/vme/inter_frame_ivb.g7b"
};

static const uint32_t gen7_vme_batchbuffer[][4] = {
#include "shaders/vme/batchbuffer.g7b"
};

static const uint32_t gen7_vme_binter_frame[][4] = {
#include "shaders/vme/inter_bframe_ivb.g7b"
};

static struct i965_kernel gen7_vme_kernels[] = {
    {
        "AVC VME Intra Frame",
        AVC_VME_INTRA_SHADER,			/*index*/
        gen7_vme_intra_frame, 			
        sizeof(gen7_vme_intra_frame),		
        NULL
    },
    {
        "AVC VME inter Frame",
        AVC_VME_INTER_SHADER,
        gen7_vme_inter_frame,
        sizeof(gen7_vme_inter_frame),
        NULL
    },
    {
        "AVC VME BATCHBUFFER",
        AVC_VME_BATCHBUFFER,
        gen7_vme_batchbuffer,
        sizeof(gen7_vme_batchbuffer),
        NULL
    },
    {
        "AVC VME binter Frame",
        AVC_VME_BINTER_SHADER,
        gen7_vme_binter_frame,
        sizeof(gen7_vme_binter_frame),
        NULL
    }
};

static const uint32_t gen7_vme_mpeg2_inter_frame[][4] = {
#include "shaders/vme/mpeg2_inter_ivb.g7b"
};

static const uint32_t gen7_vme_mpeg2_batchbuffer[][4] = {
#include "shaders/vme/batchbuffer.g7b"
};

static struct i965_kernel gen7_vme_mpeg2_kernels[] = {
    {
        "MPEG2 VME inter Frame",
        MPEG2_VME_INTER_SHADER,
        gen7_vme_mpeg2_inter_frame,
        sizeof(gen7_vme_mpeg2_inter_frame),
        NULL
    },
    {
        "MPEG2 VME BATCHBUFFER",
        MPEG2_VME_BATCHBUFFER,
        gen7_vme_mpeg2_batchbuffer,
        sizeof(gen7_vme_mpeg2_batchbuffer),
        NULL
    },
};

/* only used for VME source surface state */
static void 
gen7_vme_source_surface_state(VADriverContextP ctx,
                              int index,
                              struct object_surface *obj_surface,
                              struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;

    vme_context->vme_surface2_setup(ctx,
                                    &vme_context->gpe_context,
                                    obj_surface,
                                    BINDING_TABLE_OFFSET(index),
                                    SURFACE_STATE_OFFSET(index));
}

static void
gen7_vme_media_source_surface_state(VADriverContextP ctx,
                                    int index,
                                    struct object_surface *obj_surface,
                                    struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;

    vme_context->vme_media_rw_surface_setup(ctx,
                                            &vme_context->gpe_context,
                                            obj_surface,
                                            BINDING_TABLE_OFFSET(index),
                                            SURFACE_STATE_OFFSET(index));
}

static void
gen7_vme_output_buffer_setup(VADriverContextP ctx,
                             struct encode_state *encode_state,
                             int index,
                             struct intel_encoder_context *encoder_context)

{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    VAEncSliceParameterBufferH264 *pSliceParameter = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[0]->buffer;
    int is_intra = pSliceParameter->slice_type == SLICE_TYPE_I;
    int width_in_mbs = pSequenceParameter->picture_width_in_mbs;
    int height_in_mbs = pSequenceParameter->picture_height_in_mbs;

    vme_context->vme_output.num_blocks = width_in_mbs * height_in_mbs;
    vme_context->vme_output.pitch = 16; /* in bytes, always 16 */

    if (is_intra)
        vme_context->vme_output.size_block = INTRA_VME_OUTPUT_IN_BYTES;
    else
        vme_context->vme_output.size_block = INTER_VME_OUTPUT_IN_BYTES;

    vme_context->vme_output.bo = dri_bo_alloc(i965->intel.bufmgr, 
                                              "VME output buffer",
                                              vme_context->vme_output.num_blocks * vme_context->vme_output.size_block,
                                              0x1000);
    assert(vme_context->vme_output.bo);
    vme_context->vme_buffer_suface_setup(ctx,
                                         &vme_context->gpe_context,
                                         &vme_context->vme_output,
                                         BINDING_TABLE_OFFSET(index),
                                         SURFACE_STATE_OFFSET(index));
}

static void
gen7_vme_output_vme_batchbuffer_setup(VADriverContextP ctx,
                                      struct encode_state *encode_state,
                                      int index,
                                      struct intel_encoder_context *encoder_context)

{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    int width_in_mbs = pSequenceParameter->picture_width_in_mbs;
    int height_in_mbs = pSequenceParameter->picture_height_in_mbs;

    vme_context->vme_batchbuffer.num_blocks = width_in_mbs * height_in_mbs + 1;
    vme_context->vme_batchbuffer.size_block = 32; /* 2 OWORDs */
    vme_context->vme_batchbuffer.pitch = 16;
    vme_context->vme_batchbuffer.bo = dri_bo_alloc(i965->intel.bufmgr, 
                                                   "VME batchbuffer",
                                                   vme_context->vme_batchbuffer.num_blocks * vme_context->vme_batchbuffer.size_block,
                                                   0x1000);
    vme_context->vme_buffer_suface_setup(ctx,
                                         &vme_context->gpe_context,
                                         &vme_context->vme_batchbuffer,
                                         BINDING_TABLE_OFFSET(index),
                                         SURFACE_STATE_OFFSET(index));
}

static VAStatus
gen7_vme_surface_setup(VADriverContextP ctx, 
                       struct encode_state *encode_state,
                       int is_intra,
                       struct intel_encoder_context *encoder_context)
{
    struct object_surface *obj_surface;

    /*Setup surfaces state*/
    /* current picture for encoding */
    obj_surface = encode_state->input_yuv_object;
    gen7_vme_source_surface_state(ctx, 0, obj_surface, encoder_context);
    gen7_vme_media_source_surface_state(ctx, 4, obj_surface, encoder_context);

    if (!is_intra) {
	VAEncSliceParameterBufferH264 *slice_param = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[0]->buffer;
	int slice_type;

	slice_type = intel_avc_enc_slice_type_fixup(slice_param->slice_type);
        assert(slice_type != SLICE_TYPE_I && slice_type != SLICE_TYPE_SI);

        intel_avc_vme_reference_state(ctx, encode_state, encoder_context, 0, 1, gen7_vme_source_surface_state);

	if (slice_type == SLICE_TYPE_B)
            intel_avc_vme_reference_state(ctx, encode_state, encoder_context, 1, 2, gen7_vme_source_surface_state);
    }

    /* VME output */
    gen7_vme_output_buffer_setup(ctx, encode_state, 3, encoder_context);
    gen7_vme_output_vme_batchbuffer_setup(ctx, encode_state, 5, encoder_context);

    return VA_STATUS_SUCCESS;
}

static VAStatus gen7_vme_interface_setup(VADriverContextP ctx, 
                                         struct encode_state *encode_state,
                                         struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct gen6_interface_descriptor_data *desc;   
    int i;
    dri_bo *bo;

    bo = vme_context->gpe_context.idrt.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);
    desc = bo->virtual;

    for (i = 0; i < vme_context->vme_kernel_sum; i++) {
        struct i965_kernel *kernel;
        kernel = &vme_context->gpe_context.kernels[i];
        assert(sizeof(*desc) == 32);
        /*Setup the descritor table*/
        memset(desc, 0, sizeof(*desc));
        desc->desc0.kernel_start_pointer = (kernel->bo->offset >> 6);
        desc->desc2.sampler_count = 1; /* FIXME: */
        desc->desc2.sampler_state_pointer = (vme_context->vme_state.bo->offset >> 5);
        desc->desc3.binding_table_entry_count = 1; /* FIXME: */
        desc->desc3.binding_table_pointer = (BINDING_TABLE_OFFSET(0) >> 5);
        desc->desc4.constant_urb_entry_read_offset = 0;
        desc->desc4.constant_urb_entry_read_length = CURBE_URB_ENTRY_LENGTH;
 		
        /*kernel start*/
        dri_bo_emit_reloc(bo,	
                          I915_GEM_DOMAIN_INSTRUCTION, 0,
                          0,
                          i * sizeof(*desc) + offsetof(struct gen6_interface_descriptor_data, desc0),
                          kernel->bo);
        /*Sampler State(VME state pointer)*/
        dri_bo_emit_reloc(bo,
                          I915_GEM_DOMAIN_INSTRUCTION, 0,
                          (1 << 2),									//
                          i * sizeof(*desc) + offsetof(struct gen6_interface_descriptor_data, desc2),
                          vme_context->vme_state.bo);
        desc++;
    }
    dri_bo_unmap(bo);

    return VA_STATUS_SUCCESS;
}

static VAStatus gen7_vme_constant_setup(VADriverContextP ctx, 
                                        struct encode_state *encode_state,
                                        struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    unsigned char *constant_buffer;
    unsigned int *vme_state_message;
    int mv_num;

    vme_state_message = (unsigned int *)vme_context->vme_state_message;
    mv_num = 32;

    if (encoder_context->codec == CODEC_H264) {
        if (vme_context->h264_level >= 30) {
            mv_num = 16;
        
            if (vme_context->h264_level >= 31)
                mv_num = 8;
        }
    } else if (encoder_context->codec == CODEC_MPEG2) { 
        mv_num = 2;
    }


    vme_state_message[31] = mv_num;

    dri_bo_map(vme_context->gpe_context.curbe.bo, 1);
    assert(vme_context->gpe_context.curbe.bo->virtual);
    constant_buffer = vme_context->gpe_context.curbe.bo->virtual;

    /* Pass the required constant info into the constant buffer */
    memcpy(constant_buffer, (char *)vme_context->vme_state_message, 128);
	
    dri_bo_unmap( vme_context->gpe_context.curbe.bo);

    return VA_STATUS_SUCCESS;
}


static VAStatus gen7_vme_avc_state_setup(VADriverContextP ctx,
                                         struct encode_state *encode_state,
                                         int is_intra,
                                         struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    unsigned int *vme_state_message;
    unsigned int *mb_cost_table;
    int i;
    VAEncSliceParameterBufferH264 *slice_param = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[0]->buffer;
    unsigned int is_low_quality = (encoder_context->quality_level == ENCODER_LOW_QUALITY);

    mb_cost_table = (unsigned int *)vme_context->vme_state_message;
    //building VME state message
    dri_bo_map(vme_context->vme_state.bo, 1);
    assert(vme_context->vme_state.bo->virtual);
    vme_state_message = (unsigned int *)vme_context->vme_state.bo->virtual;

    if (((slice_param->slice_type == SLICE_TYPE_P) ||
        (slice_param->slice_type == SLICE_TYPE_SP) &&
        !is_low_quality)) {
        vme_state_message[0] = 0x01010101;
        vme_state_message[1] = 0x10010101;
        vme_state_message[2] = 0x0F0F0F0F;
        vme_state_message[3] = 0x100F0F0F;
        vme_state_message[4] = 0x01010101;
        vme_state_message[5] = 0x10010101;
        vme_state_message[6] = 0x0F0F0F0F;
        vme_state_message[7] = 0x100F0F0F;
        vme_state_message[8] = 0x01010101;
        vme_state_message[9] = 0x10010101;
        vme_state_message[10] = 0x0F0F0F0F;
        vme_state_message[11] = 0x000F0F0F;
        vme_state_message[12] = 0x00;
        vme_state_message[13] = 0x00;
    } else {
        vme_state_message[0] = 0x10010101;
        vme_state_message[1] = 0x100F0F0F;
        vme_state_message[2] = 0x10010101;
        vme_state_message[3] = 0x000F0F0F;
        vme_state_message[4] = 0;
        vme_state_message[5] = 0;
        vme_state_message[6] = 0;
        vme_state_message[7] = 0;
        vme_state_message[8] = 0;
        vme_state_message[9] = 0;
        vme_state_message[10] = 0;
        vme_state_message[11] = 0;
        vme_state_message[12] = 0;
        vme_state_message[13] = 0;
    }

    vme_state_message[14] = (mb_cost_table[2] & 0xFFFF);
    vme_state_message[15] = 0;
    vme_state_message[16] = mb_cost_table[0];
    vme_state_message[17] = mb_cost_table[1];
    vme_state_message[18] = mb_cost_table[3];
    vme_state_message[19] = mb_cost_table[4];

    for(i = 20; i < 32; i++) {
        vme_state_message[i] = 0;
    }

    dri_bo_unmap( vme_context->vme_state.bo);
    return VA_STATUS_SUCCESS;
}

static VAStatus gen7_vme_mpeg2_state_setup(VADriverContextP ctx,
                                           struct encode_state *encode_state,
                                           int is_intra,
                                           struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    unsigned int *vme_state_message;
    int i;
    unsigned int *mb_cost_table;

    mb_cost_table = (unsigned int *)vme_context->vme_state_message;
	
    //building VME state message
    dri_bo_map(vme_context->vme_state.bo, 1);
    assert(vme_context->vme_state.bo->virtual);
    vme_state_message = (unsigned int *)vme_context->vme_state.bo->virtual;

    vme_state_message[0] = 0x01010101;
    vme_state_message[1] = 0x10010101;
    vme_state_message[2] = 0x0F0F0F0F;
    vme_state_message[3] = 0x100F0F0F;
    vme_state_message[4] = 0x01010101;
    vme_state_message[5] = 0x10010101;
    vme_state_message[6] = 0x0F0F0F0F;
    vme_state_message[7] = 0x100F0F0F;
    vme_state_message[8] = 0x01010101;
    vme_state_message[9] = 0x10010101;
    vme_state_message[10] = 0x0F0F0F0F;
    vme_state_message[11] = 0x000F0F0F;
    vme_state_message[12] = 0x00;
    vme_state_message[13] = 0x00;

    vme_state_message[14] = (mb_cost_table[2] & 0xFFFF);
    vme_state_message[15] = 0;
    vme_state_message[16] = mb_cost_table[0];
    vme_state_message[17] = 0;
    vme_state_message[18] = mb_cost_table[3];
    vme_state_message[19] = mb_cost_table[4];

    for(i = 20; i < 32; i++) {
        vme_state_message[i] = 0;
    }
    //vme_state_message[16] = 0x42424242;			//cost function LUT set 0 for Intra

    dri_bo_unmap( vme_context->vme_state.bo);
    return VA_STATUS_SUCCESS;
}

static void
gen7_vme_fill_vme_batchbuffer(VADriverContextP ctx, 
                              struct encode_state *encode_state,
                              int mb_width, int mb_height,
                              int kernel,
                              int transform_8x8_mode_flag,
                              struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    int mb_x = 0, mb_y = 0;
    int i, s, j;
    unsigned int *command_ptr;


    dri_bo_map(vme_context->vme_batchbuffer.bo, 1);
    command_ptr = vme_context->vme_batchbuffer.bo->virtual;

    for (s = 0; s < encode_state->num_slice_params_ext; s++) {
        VAEncSliceParameterBufferMPEG2 *slice_param = (VAEncSliceParameterBufferMPEG2 *)encode_state->slice_params_ext[s]->buffer;

        for (j = 0; j < encode_state->slice_params_ext[s]->num_elements; j++) {
            int slice_mb_begin = slice_param->macroblock_address;
            int slice_mb_number = slice_param->num_macroblocks;
            unsigned int mb_intra_ub;
            int slice_mb_x = slice_param->macroblock_address % mb_width;

            for (i = 0; i < slice_mb_number;) {
                int mb_count = i + slice_mb_begin;    

                mb_x = mb_count % mb_width;
                mb_y = mb_count / mb_width;
                mb_intra_ub = 0;

                if (mb_x != 0) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_AE;
                }

                if (mb_y != 0) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_B;

                    if (mb_x != 0)
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_D;

                    if (mb_x != (mb_width -1))
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_C;
                }

                if (i < mb_width) {
                    if (i == 0)
                        mb_intra_ub &= ~(INTRA_PRED_AVAIL_FLAG_AE);

                    mb_intra_ub &= ~(INTRA_PRED_AVAIL_FLAG_BCD_MASK);

                    if ((i == (mb_width - 1)) && slice_mb_x) {
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_C;
                    }
                }
		
                if ((i == mb_width) && slice_mb_x) {
                    mb_intra_ub &= ~(INTRA_PRED_AVAIL_FLAG_D);
                }

                *command_ptr++ = (CMD_MEDIA_OBJECT | (8 - 2));
                *command_ptr++ = kernel;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
   
                /*inline data */
                *command_ptr++ = (mb_width << 16 | mb_y << 8 | mb_x);
                *command_ptr++ = ((encoder_context->quality_level << 24) | (1 << 16) | transform_8x8_mode_flag | (mb_intra_ub << 8));

                i += 1;
            }

            slice_param++;
        }
    }

    *command_ptr++ = 0;
    *command_ptr++ = MI_BATCH_BUFFER_END;

    dri_bo_unmap(vme_context->vme_batchbuffer.bo);
}


static void gen7_vme_media_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    dri_bo *bo;

    i965_gpe_context_init(ctx, &vme_context->gpe_context);

    /* VME output buffer */
    dri_bo_unreference(vme_context->vme_output.bo);
    vme_context->vme_output.bo = NULL;

    dri_bo_unreference(vme_context->vme_batchbuffer.bo);
    vme_context->vme_batchbuffer.bo = NULL;

    /* VME state */
    dri_bo_unreference(vme_context->vme_state.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      1024*16, 64);
    assert(bo);
    vme_context->vme_state.bo = bo;
}

static void gen7_vme_pipeline_programing(VADriverContextP ctx, 
                                         struct encode_state *encode_state,
                                         struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    VAEncPictureParameterBufferH264 *pPicParameter = (VAEncPictureParameterBufferH264 *)encode_state->pic_param_ext->buffer;
    VAEncSliceParameterBufferH264 *pSliceParameter = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[0]->buffer;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    int width_in_mbs = pSequenceParameter->picture_width_in_mbs;
    int height_in_mbs = pSequenceParameter->picture_height_in_mbs;
    int s;
    bool allow_hwscore = true;
    int kernel_shader;
    unsigned int is_low_quality = (encoder_context->quality_level == ENCODER_LOW_QUALITY);

    if (is_low_quality)
        allow_hwscore = false;
    else {
        for (s = 0; s < encode_state->num_slice_params_ext; s++) {
            pSliceParameter = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[s]->buffer; 
            if ((pSliceParameter->macroblock_address % width_in_mbs)) {
                allow_hwscore = false;
                break;
            }
        }
    }

    if ((pSliceParameter->slice_type == SLICE_TYPE_I) ||
	(pSliceParameter->slice_type == SLICE_TYPE_I)) {
	kernel_shader = AVC_VME_INTRA_SHADER;
    } else if ((pSliceParameter->slice_type == SLICE_TYPE_P) ||
               (pSliceParameter->slice_type == SLICE_TYPE_SP)) {
	kernel_shader = AVC_VME_INTER_SHADER;
    } else {
	kernel_shader = AVC_VME_BINTER_SHADER;
	if (!allow_hwscore)
            kernel_shader = AVC_VME_INTER_SHADER;
    }

    if (allow_hwscore)
	gen7_vme_walker_fill_vme_batchbuffer(ctx, 
                                             encode_state,
                                             width_in_mbs, height_in_mbs,
                                             kernel_shader,
                                             pPicParameter->pic_fields.bits.transform_8x8_mode_flag,
                                             encoder_context);
	
    else
	gen7_vme_fill_vme_batchbuffer(ctx, 
                                      encode_state,
                                      width_in_mbs, height_in_mbs,
                                      kernel_shader,
                                      pPicParameter->pic_fields.bits.transform_8x8_mode_flag,
                                      encoder_context);

    intel_batchbuffer_start_atomic(batch, 0x1000);
    gen6_gpe_pipeline_setup(ctx, &vme_context->gpe_context, batch);
    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, MI_BATCH_BUFFER_START | (1 << 8));
    OUT_RELOC(batch,
              vme_context->vme_batchbuffer.bo,
              I915_GEM_DOMAIN_COMMAND, 0, 
              0);
    ADVANCE_BATCH(batch);

    intel_batchbuffer_end_atomic(batch);	
}

static VAStatus gen7_vme_prepare(VADriverContextP ctx, 
                                 struct encode_state *encode_state,
                                 struct intel_encoder_context *encoder_context)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    VAEncSliceParameterBufferH264 *pSliceParameter = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[0]->buffer;
    int is_intra = pSliceParameter->slice_type == SLICE_TYPE_I;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;

    if (!vme_context->h264_level ||
        (vme_context->h264_level != pSequenceParameter->level_idc)) {
	vme_context->h264_level = pSequenceParameter->level_idc;	
    }
	
    intel_vme_update_mbmv_cost(ctx, encode_state, encoder_context);
    /*Setup all the memory object*/
    gen7_vme_surface_setup(ctx, encode_state, is_intra, encoder_context);
    gen7_vme_interface_setup(ctx, encode_state, encoder_context);
    gen7_vme_constant_setup(ctx, encode_state, encoder_context);
    gen7_vme_avc_state_setup(ctx, encode_state, is_intra, encoder_context);

    /*Programing media pipeline*/
    gen7_vme_pipeline_programing(ctx, encode_state, encoder_context);

    return vaStatus;
}

static VAStatus gen7_vme_run(VADriverContextP ctx, 
                             struct encode_state *encode_state,
                             struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;

    intel_batchbuffer_flush(batch);

    return VA_STATUS_SUCCESS;
}

static VAStatus gen7_vme_stop(VADriverContextP ctx, 
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context)
{
    return VA_STATUS_SUCCESS;
}

static VAStatus
gen7_vme_pipeline(VADriverContextP ctx,
                  VAProfile profile,
                  struct encode_state *encode_state,
                  struct intel_encoder_context *encoder_context)
{
    gen7_vme_media_init(ctx, encoder_context);
    gen7_vme_prepare(ctx, encode_state, encoder_context);
    gen7_vme_run(ctx, encode_state, encoder_context);
    gen7_vme_stop(ctx, encode_state, encoder_context);

    return VA_STATUS_SUCCESS;
}

static void
gen7_vme_mpeg2_output_buffer_setup(VADriverContextP ctx,
                                   struct encode_state *encode_state,
                                   int index,
                                   int is_intra,
                                   struct intel_encoder_context *encoder_context)

{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncSequenceParameterBufferMPEG2 *seq_param = (VAEncSequenceParameterBufferMPEG2 *)encode_state->seq_param_ext->buffer;
    int width_in_mbs = ALIGN(seq_param->picture_width, 16) / 16;
    int height_in_mbs = ALIGN(seq_param->picture_height, 16) / 16;

    vme_context->vme_output.num_blocks = width_in_mbs * height_in_mbs;
    vme_context->vme_output.pitch = 16; /* in bytes, always 16 */

    if (is_intra)
        vme_context->vme_output.size_block = INTRA_VME_OUTPUT_IN_BYTES;
    else
        vme_context->vme_output.size_block = INTER_VME_OUTPUT_IN_BYTES;

    vme_context->vme_output.bo = dri_bo_alloc(i965->intel.bufmgr,
                                              "VME output buffer",
                                              vme_context->vme_output.num_blocks * vme_context->vme_output.size_block,
                                              0x1000);
    assert(vme_context->vme_output.bo);
    vme_context->vme_buffer_suface_setup(ctx,
                                         &vme_context->gpe_context,
                                         &vme_context->vme_output,
                                         BINDING_TABLE_OFFSET(index),
                                         SURFACE_STATE_OFFSET(index));
}

static void
gen7_vme_mpeg2_output_vme_batchbuffer_setup(VADriverContextP ctx,
                                            struct encode_state *encode_state,
                                            int index,
                                            struct intel_encoder_context *encoder_context)

{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncSequenceParameterBufferMPEG2 *seq_param = (VAEncSequenceParameterBufferMPEG2 *)encode_state->seq_param_ext->buffer;
    int width_in_mbs = ALIGN(seq_param->picture_width, 16) / 16;
    int height_in_mbs = ALIGN(seq_param->picture_height, 16) / 16;

    vme_context->vme_batchbuffer.num_blocks = width_in_mbs * height_in_mbs + 1;
    vme_context->vme_batchbuffer.size_block = 32; /* 4 OWORDs */
    vme_context->vme_batchbuffer.pitch = 16;
    vme_context->vme_batchbuffer.bo = dri_bo_alloc(i965->intel.bufmgr, 
                                                   "VME batchbuffer",
                                                   vme_context->vme_batchbuffer.num_blocks * vme_context->vme_batchbuffer.size_block,
                                                   0x1000);
    vme_context->vme_buffer_suface_setup(ctx,
                                         &vme_context->gpe_context,
                                         &vme_context->vme_batchbuffer,
                                         BINDING_TABLE_OFFSET(index),
                                         SURFACE_STATE_OFFSET(index));
}

static VAStatus
gen7_vme_mpeg2_surface_setup(VADriverContextP ctx, 
                             struct encode_state *encode_state,
                             int is_intra,
                             struct intel_encoder_context *encoder_context)
{
    struct object_surface *obj_surface;

    /*Setup surfaces state*/
    /* current picture for encoding */
    obj_surface = encode_state->input_yuv_object;
    gen7_vme_source_surface_state(ctx, 0, obj_surface, encoder_context);
    gen7_vme_media_source_surface_state(ctx, 4, obj_surface, encoder_context);

    if (!is_intra) {
        /* reference 0 */
        obj_surface = encode_state->reference_objects[0];
        if (obj_surface->bo != NULL)
            gen7_vme_source_surface_state(ctx, 1, obj_surface, encoder_context);

        /* reference 1 */
        obj_surface = encode_state->reference_objects[1];
        if (obj_surface && obj_surface->bo != NULL) 
            gen7_vme_source_surface_state(ctx, 2, obj_surface, encoder_context);
    }

    /* VME output */
    gen7_vme_mpeg2_output_buffer_setup(ctx, encode_state, 3, is_intra, encoder_context);
    gen7_vme_mpeg2_output_vme_batchbuffer_setup(ctx, encode_state, 5, encoder_context);

    return VA_STATUS_SUCCESS;
}

static void
gen7_vme_mpeg2_fill_vme_batchbuffer(VADriverContextP ctx,
                                    struct encode_state *encode_state,
                                    int mb_width, int mb_height,
                                    int kernel,
                                    int transform_8x8_mode_flag,
                                    struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    int mb_x = 0, mb_y = 0;
    int i, s, j;
    unsigned int *command_ptr;

    dri_bo_map(vme_context->vme_batchbuffer.bo, 1);
    command_ptr = vme_context->vme_batchbuffer.bo->virtual;

    for (s = 0; s < encode_state->num_slice_params_ext; s++) {
        VAEncSliceParameterBufferMPEG2 *slice_param = (VAEncSliceParameterBufferMPEG2 *)encode_state->slice_params_ext[s]->buffer;

        for (j = 0; j < encode_state->slice_params_ext[s]->num_elements; j++) {
            int slice_mb_begin = slice_param->macroblock_address;
            int slice_mb_number = slice_param->num_macroblocks;
            unsigned int mb_intra_ub;

            for (i = 0; i < slice_mb_number;) {
                int mb_count = i + slice_mb_begin;    

                mb_x = mb_count % mb_width;
                mb_y = mb_count / mb_width;
                mb_intra_ub = 0;

                if (mb_x != 0) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_AE;
                }

                if (mb_y != 0) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_B;

                    if (mb_x != 0)
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_D;

                    if (mb_x != (mb_width -1))
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_C;
                }

		

                *command_ptr++ = (CMD_MEDIA_OBJECT | (8 - 2));
                *command_ptr++ = kernel;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
   
                /*inline data */
                *command_ptr++ = (mb_width << 16 | mb_y << 8 | mb_x);
                *command_ptr++ = ( (1 << 16) | transform_8x8_mode_flag | (mb_intra_ub << 8));

                i += 1;
            }

            slice_param++;
        }
    }

    *command_ptr++ = 0;
    *command_ptr++ = MI_BATCH_BUFFER_END;

    dri_bo_unmap(vme_context->vme_batchbuffer.bo);
}

static void
gen7_vme_mpeg2_pipeline_programing(VADriverContextP ctx, 
                                   struct encode_state *encode_state,
                                   int is_intra,
                                   struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    VAEncSequenceParameterBufferMPEG2 *seq_param = (VAEncSequenceParameterBufferMPEG2 *)encode_state->seq_param_ext->buffer;
    int width_in_mbs = ALIGN(seq_param->picture_width, 16) / 16;
    int height_in_mbs = ALIGN(seq_param->picture_height, 16) / 16;

    bool allow_hwscore = true;
    int s;

    for (s = 0; s < encode_state->num_slice_params_ext; s++) {
	int j;
        VAEncSliceParameterBufferMPEG2 *slice_param = (VAEncSliceParameterBufferMPEG2 *)encode_state->slice_params_ext[s]->buffer;

        for (j = 0; j < encode_state->slice_params_ext[s]->num_elements; j++) {
	    if (slice_param->macroblock_address % width_in_mbs) {
		allow_hwscore = false;
		break;
	    }
	}
    }

    if (allow_hwscore) 
	gen7_vme_mpeg2_walker_fill_vme_batchbuffer(ctx,
                                                   encode_state,
                                                   width_in_mbs, height_in_mbs,
                                                   MPEG2_VME_INTER_SHADER,
                                                   encoder_context);
    else
    	gen7_vme_mpeg2_fill_vme_batchbuffer(ctx, 
                                            encode_state,
                                            width_in_mbs, height_in_mbs,
                                            MPEG2_VME_INTER_SHADER,
                                            0,
                                            encoder_context);

    intel_batchbuffer_start_atomic(batch, 0x1000);
    gen6_gpe_pipeline_setup(ctx, &vme_context->gpe_context, batch);
    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, MI_BATCH_BUFFER_START | (1 << 8));
    OUT_RELOC(batch,
              vme_context->vme_batchbuffer.bo,
              I915_GEM_DOMAIN_COMMAND, 0, 
              0);
    ADVANCE_BATCH(batch);

    intel_batchbuffer_end_atomic(batch);
}

static VAStatus
gen7_vme_mpeg2_prepare(VADriverContextP ctx, 
                       struct encode_state *encode_state,
                       struct intel_encoder_context *encoder_context)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    VAEncSequenceParameterBufferMPEG2 *seq_param = (VAEncSequenceParameterBufferMPEG2 *)encode_state->seq_param_ext->buffer;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;

    if ((!vme_context->mpeg2_level) ||
        (vme_context->mpeg2_level != (seq_param->sequence_extension.bits.profile_and_level_indication & MPEG2_LEVEL_MASK))) {
	vme_context->mpeg2_level = seq_param->sequence_extension.bits.profile_and_level_indication & MPEG2_LEVEL_MASK;
    }

    /*Setup all the memory object*/

    intel_vme_mpeg2_state_setup(ctx, encode_state, encoder_context);
    gen7_vme_mpeg2_surface_setup(ctx, encode_state, 0, encoder_context);
    gen7_vme_interface_setup(ctx, encode_state, encoder_context);
    gen7_vme_constant_setup(ctx, encode_state, encoder_context);
    gen7_vme_mpeg2_state_setup(ctx, encode_state, 0, encoder_context);

    /*Programing media pipeline*/
    gen7_vme_mpeg2_pipeline_programing(ctx, encode_state, 0, encoder_context);

    return vaStatus;
}

static VAStatus
gen7_vme_mpeg2_pipeline(VADriverContextP ctx,
                        VAProfile profile,
                        struct encode_state *encode_state,
                        struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncSliceParameterBufferMPEG2 *slice_param = 
        (VAEncSliceParameterBufferMPEG2 *)encode_state->slice_params_ext[0]->buffer;
    VAEncSequenceParameterBufferMPEG2 *seq_param = 
        (VAEncSequenceParameterBufferMPEG2 *)encode_state->seq_param_ext->buffer;
 
    /*No need of to exec VME for Intra slice */
    if (slice_param->is_intra_slice) {
        if(!vme_context->vme_output.bo) {
            int w_in_mbs = ALIGN(seq_param->picture_width, 16) / 16;
            int h_in_mbs = ALIGN(seq_param->picture_height, 16) / 16;

            vme_context->vme_output.num_blocks = w_in_mbs * h_in_mbs;
            vme_context->vme_output.pitch = 16; /* in bytes, always 16 */
            vme_context->vme_output.size_block = INTRA_VME_OUTPUT_IN_BYTES;
            vme_context->vme_output.bo = dri_bo_alloc(i965->intel.bufmgr,
                                                      "MPEG2 VME output buffer",
                                                      vme_context->vme_output.num_blocks
                                                      * vme_context->vme_output.size_block,
                                                      0x1000);
        }

        return VA_STATUS_SUCCESS;
    }

    gen7_vme_media_init(ctx, encoder_context);
    gen7_vme_mpeg2_prepare(ctx, encode_state, encoder_context);
    gen7_vme_run(ctx, encode_state, encoder_context);
    gen7_vme_stop(ctx, encode_state, encoder_context);

    return VA_STATUS_SUCCESS;
}

static void
gen7_vme_context_destroy(void *context)
{
    struct gen6_vme_context *vme_context = context;

    i965_gpe_context_destroy(&vme_context->gpe_context);

    dri_bo_unreference(vme_context->vme_output.bo);
    vme_context->vme_output.bo = NULL;

    dri_bo_unreference(vme_context->vme_state.bo);
    vme_context->vme_state.bo = NULL;

    dri_bo_unreference(vme_context->vme_batchbuffer.bo);
    vme_context->vme_batchbuffer.bo = NULL;

    if (vme_context->vme_state_message) {
	free(vme_context->vme_state_message);
	vme_context->vme_state_message = NULL;
    }

    free(vme_context);
}

Bool gen7_vme_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = calloc(1, sizeof(struct gen6_vme_context));
    struct i965_kernel *vme_kernel_list = NULL;

    vme_context->gpe_context.surface_state_binding_table.length =
        (SURFACE_STATE_PADDED_SIZE + sizeof(unsigned int)) * MAX_MEDIA_SURFACES_GEN6;

    vme_context->gpe_context.idrt.max_entries = MAX_INTERFACE_DESC_GEN6;
    vme_context->gpe_context.idrt.entry_size = sizeof(struct gen6_interface_descriptor_data);
    vme_context->gpe_context.curbe.length = CURBE_TOTAL_DATA_LENGTH;

    vme_context->gpe_context.vfe_state.max_num_threads = 60 - 1;
    vme_context->gpe_context.vfe_state.num_urb_entries = 16;
    vme_context->gpe_context.vfe_state.gpgpu_mode = 0;
    vme_context->gpe_context.vfe_state.urb_entry_size = 59 - 1;
    vme_context->gpe_context.vfe_state.curbe_allocation_size = CURBE_ALLOCATION_SIZE - 1;

    gen7_vme_scoreboard_init(ctx, vme_context);

    if (encoder_context->codec == CODEC_H264) {
        vme_kernel_list = gen7_vme_kernels;
        vme_context->video_coding_type = VIDEO_CODING_AVC;
        vme_context->vme_kernel_sum = AVC_VME_KERNEL_SUM; 
        encoder_context->vme_pipeline = gen7_vme_pipeline; 
    } else if (encoder_context->codec == CODEC_MPEG2) {
        vme_kernel_list = gen7_vme_mpeg2_kernels;
        vme_context->video_coding_type = VIDEO_CODING_MPEG2;
        vme_context->vme_kernel_sum = MPEG2_VME_KERNEL_SUM;
        encoder_context->vme_pipeline = gen7_vme_mpeg2_pipeline;
    } else {
        /* Unsupported codec */
        assert(0);
    }

    i965_gpe_load_kernels(ctx,
                          &vme_context->gpe_context,
                          vme_kernel_list,
                          vme_context->vme_kernel_sum);

    vme_context->vme_surface2_setup = gen7_gpe_surface2_setup;
    vme_context->vme_media_rw_surface_setup = gen7_gpe_media_rw_surface_setup;
    vme_context->vme_buffer_suface_setup = gen7_gpe_buffer_suface_setup;

    encoder_context->vme_context = vme_context;
    encoder_context->vme_context_destroy = gen7_vme_context_destroy;
    vme_context->vme_state_message = malloc(VME_MSG_LENGTH * sizeof(int));

    return True;
}
