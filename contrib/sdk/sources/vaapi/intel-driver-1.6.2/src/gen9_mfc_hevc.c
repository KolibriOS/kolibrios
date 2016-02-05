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
 *    Qu Pengfei <Pengfei.Qu@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "i965_defines.h"
#include "i965_structs.h"
#include "i965_drv_video.h"
#include "i965_encoder.h"
#include "i965_encoder_utils.h"
#include "gen9_mfc.h"
#include "gen6_vme.h"
#include "intel_media.h"

typedef enum _gen6_brc_status {
    BRC_NO_HRD_VIOLATION = 0,
    BRC_UNDERFLOW = 1,
    BRC_OVERFLOW = 2,
    BRC_UNDERFLOW_WITH_MAX_QP = 3,
    BRC_OVERFLOW_WITH_MIN_QP = 4,
} gen6_brc_status;

/* BRC define */
#define BRC_CLIP(x, min, max)                                   \
    {                                                           \
        x = ((x > (max)) ? (max) : ((x < (min)) ? (min) : x));  \
    }

#define BRC_P_B_QP_DIFF 4
#define BRC_I_P_QP_DIFF 2
#define BRC_I_B_QP_DIFF (BRC_I_P_QP_DIFF + BRC_P_B_QP_DIFF)

#define BRC_PWEIGHT 0.6  /* weight if P slice with comparison to I slice */
#define BRC_BWEIGHT 0.25 /* weight if B slice with comparison to I slice */

#define BRC_QP_MAX_CHANGE 5 /* maximum qp modification */
#define BRC_CY 0.1 /* weight for */
#define BRC_CX_UNDERFLOW 5.
#define BRC_CX_OVERFLOW -4.

#define BRC_PI_0_5 1.5707963267948966192313216916398

/* intel buffer write */
#define ALLOC_ENCODER_BUFFER(gen_buffer, string, size) do {     \
        dri_bo_unreference(gen_buffer->bo);                     \
        gen_buffer->bo = dri_bo_alloc(i965->intel.bufmgr,       \
                                      string,                   \
                                      size,                     \
                                      0x1000);                  \
        assert(gen_buffer->bo);                                 \
    } while (0);


#define OUT_BUFFER_X(buf_bo, is_target, ma)  do {                         \
        if (buf_bo) {                                                   \
            OUT_BCS_RELOC(batch,                                        \
                          buf_bo,                                       \
                          I915_GEM_DOMAIN_INSTRUCTION,                       \
                          is_target ? I915_GEM_DOMAIN_INSTRUCTION : 0,       \
                          0);                                           \
        } else {                                                        \
            OUT_BCS_BATCH(batch, 0);                                    \
        }                                                               \
        OUT_BCS_BATCH(batch, 0);                                        \
        if (ma)                                                         \
            OUT_BCS_BATCH(batch, 0);                                    \
    } while (0)

#define OUT_BUFFER_MA_TARGET(buf_bo)       OUT_BUFFER_X(buf_bo, 1, 1)
#define OUT_BUFFER_MA_REFERENCE(buf_bo)    OUT_BUFFER_X(buf_bo, 0, 1)
#define OUT_BUFFER_NMA_TARGET(buf_bo)      OUT_BUFFER_X(buf_bo, 1, 0)
#define OUT_BUFFER_NMA_REFERENCE(buf_bo)   OUT_BUFFER_X(buf_bo, 0, 0)


#define SURFACE_STATE_PADDED_SIZE               SURFACE_STATE_PADDED_SIZE_GEN8
#define SURFACE_STATE_OFFSET(index)             (SURFACE_STATE_PADDED_SIZE * index)
#define BINDING_TABLE_OFFSET(index)             (SURFACE_STATE_OFFSET(MAX_MEDIA_SURFACES_GEN6) + sizeof(unsigned int) * index)

#define HCP_SOFTWARE_SKYLAKE    1

#define NUM_HCPE_KERNEL 2

#define     INTER_MODE_MASK     0x03
#define     INTER_8X8       0x03
#define     INTER_16X8      0x01
#define     INTER_8X16      0x02
#define     SUBMB_SHAPE_MASK    0x00FF00

#define     INTER_MV8       (4 << 20)
#define     INTER_MV32      (6 << 20)


/* HEVC */

/* utils */
static void
hevc_gen_default_iq_matrix_encoder(VAQMatrixBufferHEVC *iq_matrix)
{
    /* Flat_4x4_16 */
    memset(&iq_matrix->scaling_lists_4x4, 16, sizeof(iq_matrix->scaling_lists_4x4));

    /* Flat_8x8_16 */
    memset(&iq_matrix->scaling_lists_8x8, 16, sizeof(iq_matrix->scaling_lists_8x8));

    /* Flat_16x16_16 */
    memset(&iq_matrix->scaling_lists_16x16, 16, sizeof(iq_matrix->scaling_lists_16x16));

    /* Flat_32x32_16 */
    memset(&iq_matrix->scaling_lists_32x32, 16, sizeof(iq_matrix->scaling_lists_32x32));

    /* Flat_16x16_dc_16 */
    memset(&iq_matrix->scaling_list_dc_16x16, 16, sizeof(iq_matrix->scaling_list_dc_16x16));

    /* Flat_32x32_dc_16 */
    memset(&iq_matrix->scaling_list_dc_32x32, 16, sizeof(iq_matrix->scaling_list_dc_32x32));
}

/* HEVC picture and slice state related */

static void
gen9_hcpe_pipe_mode_select(VADriverContextP ctx,
                           int standard_select,
                           struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;

    assert(standard_select == HCP_CODEC_HEVC);

    BEGIN_BCS_BATCH(batch, 4);

    OUT_BCS_BATCH(batch, HCP_PIPE_MODE_SELECT | (4 - 2));
    OUT_BCS_BATCH(batch,
                  (standard_select << 5) |
                  (0 << 3) | /* disable Pic Status / Error Report */
                  HCP_CODEC_SELECT_ENCODE);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpe_surface_state(VADriverContextP ctx, struct encode_state *encode_state,
                        struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct object_surface *obj_surface = encode_state->reconstructed_object;
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;

    /* to do */
    unsigned int y_cb_offset;

    assert(obj_surface);

    y_cb_offset = obj_surface->y_cb_offset;

    BEGIN_BCS_BATCH(batch, 3);
    OUT_BCS_BATCH(batch, HCP_SURFACE_STATE | (3 - 2));
    OUT_BCS_BATCH(batch,
                  (1 << 28) |                   /* surface id */
                  (mfc_context->surface_state.w_pitch - 1));    /* pitch - 1 */
    OUT_BCS_BATCH(batch,
                  (SURFACE_FORMAT_PLANAR_420_8 << 28) |
                  y_cb_offset);
    ADVANCE_BCS_BATCH(batch);

    BEGIN_BCS_BATCH(batch, 3);
    OUT_BCS_BATCH(batch, HCP_SURFACE_STATE | (3 - 2));
    OUT_BCS_BATCH(batch,
                  (0 << 28) |                   /* surface id */
                  (mfc_context->surface_state.w_pitch - 1));    /* pitch - 1 */
    OUT_BCS_BATCH(batch,
                  (SURFACE_FORMAT_PLANAR_420_8 << 28) |
                  y_cb_offset);
    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpe_pipe_buf_addr_state(VADriverContextP ctx, struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    struct object_surface *obj_surface;
    GenHevcSurface *hcpe_hevc_surface;
    dri_bo *bo;
    unsigned int i;

    BEGIN_BCS_BATCH(batch, 95);

    OUT_BCS_BATCH(batch, HCP_PIPE_BUF_ADDR_STATE | (95 - 2));

    obj_surface = encode_state->reconstructed_object;
    assert(obj_surface && obj_surface->bo);
    hcpe_hevc_surface = obj_surface->private_data;
    assert(hcpe_hevc_surface && hcpe_hevc_surface->motion_vector_temporal_bo);

    OUT_BUFFER_MA_TARGET(obj_surface->bo); /* DW 1..3 */
    OUT_BUFFER_MA_TARGET(mfc_context->deblocking_filter_line_buffer.bo);/* DW 4..6 */
    OUT_BUFFER_MA_TARGET(mfc_context->deblocking_filter_tile_line_buffer.bo); /* DW 7..9 */
    OUT_BUFFER_MA_TARGET(mfc_context->deblocking_filter_tile_column_buffer.bo); /* DW 10..12 */
    OUT_BUFFER_MA_TARGET(mfc_context->metadata_line_buffer.bo);         /* DW 13..15 */
    OUT_BUFFER_MA_TARGET(mfc_context->metadata_tile_line_buffer.bo);    /* DW 16..18 */
    OUT_BUFFER_MA_TARGET(mfc_context->metadata_tile_column_buffer.bo);  /* DW 19..21 */
    OUT_BUFFER_MA_TARGET(mfc_context->sao_line_buffer.bo);              /* DW 22..24 */
    OUT_BUFFER_MA_TARGET(mfc_context->sao_tile_line_buffer.bo);         /* DW 25..27 */
    OUT_BUFFER_MA_TARGET(mfc_context->sao_tile_column_buffer.bo);       /* DW 28..30 */
    OUT_BUFFER_MA_TARGET(hcpe_hevc_surface->motion_vector_temporal_bo); /* DW 31..33 */
    OUT_BUFFER_MA_TARGET(NULL); /* DW 34..36, reserved */

    /* here only max 8 reference allowed */
    for (i = 0; i < ARRAY_ELEMS(mfc_context->reference_surfaces); i++) {
        bo = mfc_context->reference_surfaces[i].bo;

        if (bo) {
            OUT_BUFFER_NMA_REFERENCE(bo);
        } else
            OUT_BUFFER_NMA_REFERENCE(NULL);
    }
    OUT_BCS_BATCH(batch, 0);    /* DW 53, memory address attributes */

    OUT_BUFFER_MA_TARGET(mfc_context->uncompressed_picture_source.bo); /* DW 54..56, uncompressed picture source */
    OUT_BUFFER_MA_TARGET(NULL); /* DW 57..59, ignore  */
    OUT_BUFFER_MA_TARGET(NULL); /* DW 60..62, ignore  */
    OUT_BUFFER_MA_TARGET(NULL); /* DW 63..65, ignore  */

    for (i = 0; i < ARRAY_ELEMS(mfc_context->current_collocated_mv_temporal_buffer) - 1; i++) {
        bo = mfc_context->current_collocated_mv_temporal_buffer[i].bo;

        if (bo) {
            OUT_BUFFER_NMA_REFERENCE(bo);
        } else
            OUT_BUFFER_NMA_REFERENCE(NULL);
    }
    OUT_BCS_BATCH(batch, 0);    /* DW 82, memory address attributes */

    OUT_BUFFER_MA_TARGET(NULL);    /* DW 83..85, ignore for HEVC */
    OUT_BUFFER_MA_TARGET(NULL);    /* DW 86..88, ignore for HEVC */
    OUT_BUFFER_MA_TARGET(NULL);    /* DW 89..91, ignore for HEVC */
    OUT_BUFFER_MA_TARGET(NULL);    /* DW 92..94, ignore for HEVC */

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpe_ind_obj_base_addr_state(VADriverContextP ctx,
                                  struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;

    /* to do */
    BEGIN_BCS_BATCH(batch, 14);

    OUT_BCS_BATCH(batch, HCP_IND_OBJ_BASE_ADDR_STATE | (14 - 2));
    OUT_BUFFER_MA_REFERENCE(NULL);                 /* DW 1..3 igonre for encoder*/
    OUT_BUFFER_NMA_REFERENCE(NULL);                /* DW 4..5, Upper Bound */
    OUT_BUFFER_MA_TARGET(mfc_context->hcp_indirect_cu_object.bo);                 /* DW 6..8, CU */
    /* DW 9..11, PAK-BSE */
    OUT_BCS_RELOC(batch,
                  mfc_context->hcp_indirect_pak_bse_object.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  mfc_context->hcp_indirect_pak_bse_object.offset);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_RELOC(batch,
                  mfc_context->hcp_indirect_pak_bse_object.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  mfc_context->hcp_indirect_pak_bse_object.end_offset);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpe_fqm_state(VADriverContextP ctx,
                    int size_id,
                    int color_component,
                    int pred_type,
                    int dc,
                    unsigned int *fqm,
                    int fqm_length,
                    struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    unsigned int fqm_buffer[32];

    assert(fqm_length <= 32);
    assert(sizeof(*fqm) == 4);
    memset(fqm_buffer, 0, sizeof(fqm_buffer));
    memcpy(fqm_buffer, fqm, fqm_length * 4);

    BEGIN_BCS_BATCH(batch, 34);

    OUT_BCS_BATCH(batch, HCP_FQM_STATE | (34 - 2));
    OUT_BCS_BATCH(batch,
                  dc << 16 |
                  color_component << 3 |
                  size_id << 1 |
                  pred_type);
    intel_batchbuffer_data(batch, fqm_buffer, 32 * 4);

    ADVANCE_BCS_BATCH(batch);
}


static void
gen9_hcpe_hevc_fqm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    unsigned int qm[32] = {
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000
    };

    gen9_hcpe_fqm_state(ctx,
                        0, 0, 0, 0,
                        qm, 8,
                        encoder_context);
    gen9_hcpe_fqm_state(ctx,
                        0, 0, 1, 0,
                        qm, 8,
                        encoder_context);
    gen9_hcpe_fqm_state(ctx,
                        1, 0, 0, 0,
                        qm, 32,
                        encoder_context);
    gen9_hcpe_fqm_state(ctx,
                        1, 0, 1, 0,
                        qm, 32,
                        encoder_context);
    gen9_hcpe_fqm_state(ctx,
                        2, 0, 0, 0x1000,
                        qm, 0,
                        encoder_context);
    gen9_hcpe_fqm_state(ctx,
                        2, 0, 1, 0x1000,
                        qm, 0,
                        encoder_context);
    gen9_hcpe_fqm_state(ctx,
                        3, 0, 0, 0x1000,
                        qm, 0,
                        encoder_context);
    gen9_hcpe_fqm_state(ctx,
                        3, 0, 1, 0x1000,
                        qm, 0,
                        encoder_context);
}

static void
gen9_hcpe_qm_state(VADriverContextP ctx,
                   int size_id,
                   int color_component,
                   int pred_type,
                   int dc,
                   unsigned int *qm,
                   int qm_length,
                   struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    unsigned int qm_buffer[16];

    assert(qm_length <= 16);
    assert(sizeof(*qm) == 4);
    memset(qm_buffer, 0, sizeof(qm_buffer));
    memcpy(qm_buffer, qm, qm_length * 4);

    BEGIN_BCS_BATCH(batch, 18);

    OUT_BCS_BATCH(batch, HCP_QM_STATE | (18 - 2));
    OUT_BCS_BATCH(batch,
                  dc << 5 |
                  color_component << 3 |
                  size_id << 1 |
                  pred_type);
    intel_batchbuffer_data(batch, qm_buffer, 16 * 4);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpe_hevc_qm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{

    int i;

    unsigned int qm[16] = {
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010
    };

    for (i = 0; i < 6; i++) {
        gen9_hcpe_qm_state(ctx,
                           0, i % 3, i / 3, 0,
                           qm, 4,
                           encoder_context);
    }

    for (i = 0; i < 6; i++) {
        gen9_hcpe_qm_state(ctx,
                           1, i % 3, i / 3, 0,
                           qm, 16,
                           encoder_context);
    }

    for (i = 0; i < 6; i++) {
        gen9_hcpe_qm_state(ctx,
                           2, i % 3, i / 3, 16,
                           qm, 16,
                           encoder_context);
    }

    for (i = 0; i < 2; i++) {
        gen9_hcpe_qm_state(ctx,
                           3, 0, i % 2, 16,
                           qm, 16,
                           encoder_context);
    }
}

static void
gen9_hcpe_hevc_pic_state(VADriverContextP ctx, struct encode_state *encode_state,
                         struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    VAEncPictureParameterBufferHEVC *pic_param ;
    VAEncSequenceParameterBufferHEVC *seq_param ;

    int max_pcm_size_minus3 = 0, min_pcm_size_minus3 = 0;
    int pcm_sample_bit_depth_luma_minus1 = 7, pcm_sample_bit_depth_chroma_minus1 = 7;
    /*
     * 7.4.3.1
     *
     * When not present, the value of loop_filter_across_tiles_enabled_flag
     * is inferred to be equal to 1.
     */
    int loop_filter_across_tiles_enabled_flag = 0;
    pic_param = (VAEncPictureParameterBufferHEVC *)encode_state->pic_param_ext->buffer;
    seq_param = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;

    int log2_cu_size = seq_param->log2_min_luma_coding_block_size_minus3 + 3;
    int log2_ctb_size =  seq_param->log2_diff_max_min_luma_coding_block_size + log2_cu_size;
    int ctb_size = 1 << log2_ctb_size;
    double rawctubits = 8 * 3 * ctb_size * ctb_size / 2.0;
    int maxctubits = (int)(5 * rawctubits / 3) ;
    double bitrate = seq_param->bits_per_second * 1.0;
    double framebitrate = bitrate / 32 / 8; //32 byte unit
    int minframebitrate = 0;//(int) (framebitrate * 3 / 10);
    int maxframebitrate = (int)(framebitrate * 10 / 10);
    int maxdeltaframebitrate = 0x1c5c; //(int) (framebitrate * 1/ 10);
    int mindeltaframebitrate = 0; //(int) (framebitrate * 1/ 10);
    int minframesize = 0;//(int)(rawframebits * 1/50);

    if (seq_param->seq_fields.bits.pcm_enabled_flag) {
        max_pcm_size_minus3 = seq_param->log2_max_pcm_luma_coding_block_size_minus3;
        min_pcm_size_minus3 = seq_param->log2_min_pcm_luma_coding_block_size_minus3;
        pcm_sample_bit_depth_luma_minus1 = (seq_param->pcm_sample_bit_depth_luma_minus1 & 0x0f);
        pcm_sample_bit_depth_chroma_minus1 = (seq_param->pcm_sample_bit_depth_chroma_minus1 & 0x0f);
    } else {
        max_pcm_size_minus3 = MIN(seq_param->log2_min_luma_coding_block_size_minus3 + seq_param->log2_diff_max_min_luma_coding_block_size, 2);
    }

    if (pic_param->pic_fields.bits.tiles_enabled_flag)
        loop_filter_across_tiles_enabled_flag = pic_param->pic_fields.bits.loop_filter_across_tiles_enabled_flag;

    /* set zero for encoder */
    loop_filter_across_tiles_enabled_flag = 0;

    BEGIN_BCS_BATCH(batch, 19);

    OUT_BCS_BATCH(batch, HCP_PIC_STATE | (19 - 2));

    OUT_BCS_BATCH(batch,
                  mfc_context->pic_size.picture_height_in_min_cb_minus1 << 16 |
                  0 << 14 |
                  mfc_context->pic_size.picture_width_in_min_cb_minus1);
    OUT_BCS_BATCH(batch,
                  max_pcm_size_minus3 << 10 |
                  min_pcm_size_minus3 << 8 |
                  (seq_param->log2_min_transform_block_size_minus2 +
                   seq_param->log2_diff_max_min_transform_block_size) << 6 |
                  seq_param->log2_min_transform_block_size_minus2 << 4 |
                  (seq_param->log2_min_luma_coding_block_size_minus3 +
                   seq_param->log2_diff_max_min_luma_coding_block_size) << 2 |
                  seq_param->log2_min_luma_coding_block_size_minus3);
    OUT_BCS_BATCH(batch, 0); /* DW 3, ignored */
    OUT_BCS_BATCH(batch,
                  0 << 27 | /* CU packet structure is 0 for SKL */
                  seq_param->seq_fields.bits.strong_intra_smoothing_enabled_flag << 26 |
                  pic_param->pic_fields.bits.transquant_bypass_enabled_flag << 25 |
                  seq_param->seq_fields.bits.amp_enabled_flag << 23 |
                  pic_param->pic_fields.bits.transform_skip_enabled_flag << 22 |
                  0 << 21 | /* 0 for encoder !(pic_param->decoded_curr_pic.flags & VA_PICTURE_HEVC_BOTTOM_FIELD)*/
                  0 << 20 |     /* 0 for encoder !!(pic_param->decoded_curr_pic.flags & VA_PICTURE_HEVC_FIELD_PIC)*/
                  pic_param->pic_fields.bits.weighted_pred_flag << 19 |
                  pic_param->pic_fields.bits.weighted_bipred_flag << 18 |
                  pic_param->pic_fields.bits.tiles_enabled_flag << 17 |                 /* 0 for encoder */
                  pic_param->pic_fields.bits.entropy_coding_sync_enabled_flag << 16 |
                  loop_filter_across_tiles_enabled_flag << 15 |
                  pic_param->pic_fields.bits.sign_data_hiding_enabled_flag << 13 |  /* 0 for encoder */
                  pic_param->log2_parallel_merge_level_minus2 << 10 |               /* 0 for encoder */
                  pic_param->pic_fields.bits.constrained_intra_pred_flag << 9 |     /* 0 for encoder */
                  seq_param->seq_fields.bits.pcm_loop_filter_disabled_flag << 8 |
                  (pic_param->diff_cu_qp_delta_depth & 0x03) << 6 |                 /* 0 for encoder */
                  pic_param->pic_fields.bits.cu_qp_delta_enabled_flag << 5 |        /* 0 for encoder */
                  seq_param->seq_fields.bits.pcm_enabled_flag << 4 |
                  seq_param->seq_fields.bits.sample_adaptive_offset_enabled_flag << 3 | /* 0 for encoder */
                  0);
    OUT_BCS_BATCH(batch,
                  0 << 27 |                 /* 8 bit only for encoder */
                  0 << 24 |                 /* 8 bit only for encoder */
                  pcm_sample_bit_depth_luma_minus1 << 20 |
                  pcm_sample_bit_depth_chroma_minus1 << 16 |
                  seq_param->max_transform_hierarchy_depth_inter << 13 |    /*  for encoder */
                  seq_param->max_transform_hierarchy_depth_intra << 10 |    /*  for encoder */
                  (pic_param->pps_cr_qp_offset & 0x1f) << 5 |
                  (pic_param->pps_cb_qp_offset & 0x1f));
    OUT_BCS_BATCH(batch,
                  0 << 29 | /* must be 0 for encoder */
                  maxctubits); /* DW 6, max LCU bit size allowed for encoder  */
    OUT_BCS_BATCH(batch,
                  0 << 31 | /* frame bitrate max unit */
                  maxframebitrate); /* DW 7, frame bitrate max 0:13   */
    OUT_BCS_BATCH(batch,
                  0 << 31 | /* frame bitrate min unit */
                  minframebitrate); /* DW 8, frame bitrate min 0:13   */
    OUT_BCS_BATCH(batch,
                  maxdeltaframebitrate << 16 | /* frame bitrate max delta ,help to select deltaQP of slice*/
                  mindeltaframebitrate); /* DW 9,(0,14) frame bitrate min delta ,help to select deltaQP of slice*/
    OUT_BCS_BATCH(batch, 0x07050402);   /* DW 10, frame delta qp max */
    OUT_BCS_BATCH(batch, 0x0d0b0908);
    OUT_BCS_BATCH(batch, 0);    /* DW 12, frame delta qp min */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0x04030200);   /* DW 14, frame delta qp max range  */
    OUT_BCS_BATCH(batch, 0x100c0806);   /* DW 15 */
    OUT_BCS_BATCH(batch, 0x04030200);   /* DW 16, frame delta qp min range  */
    OUT_BCS_BATCH(batch, 0x100c0806);
    OUT_BCS_BATCH(batch,
                  0 << 30 |
                  minframesize);    /* DW 18, min frame size units */

    ADVANCE_BCS_BATCH(batch);
}


static void
gen9_hcpe_hevc_insert_object(VADriverContextP ctx, struct intel_encoder_context *encoder_context,
                             unsigned int *insert_data, int lenght_in_dws, int data_bits_in_last_dw,
                             int skip_emul_byte_count, int is_last_header, int is_end_of_slice, int emulation_flag,
                             struct intel_batchbuffer *batch)
{
    if (batch == NULL)
        batch = encoder_context->base.batch;

    if (data_bits_in_last_dw == 0)
        data_bits_in_last_dw = 32;

    BEGIN_BCS_BATCH(batch, lenght_in_dws + 2);

    OUT_BCS_BATCH(batch, HCP_INSERT_PAK_OBJECT | (lenght_in_dws + 2 - 2));
    OUT_BCS_BATCH(batch,
                  (0 << 31) |   /* inline payload */
                  (0 << 16) |   /* always start at offset 0 */
                  (0 << 15) |   /* HeaderLengthExcludeFrmSize */
                  (data_bits_in_last_dw << 8) |
                  (skip_emul_byte_count << 4) |
                  (!!emulation_flag << 3) |
                  ((!!is_last_header) << 2) |
                  ((!!is_end_of_slice) << 1) |
                  (0 << 0));    /* Reserved */
    intel_batchbuffer_data(batch, insert_data, lenght_in_dws * 4);

    ADVANCE_BCS_BATCH(batch);
}
/*
// To be do: future
static uint8_t
intel_get_ref_idx_state_1(VAPictureHEVC *va_pic, unsigned int frame_store_id)
{
    unsigned int is_long_term =
        !!(va_pic->flags & VA_PICTURE_HEVC_LONG_TERM_REFERENCE);
    unsigned int is_top_field =
        !!!(va_pic->flags & VA_PICTURE_HEVC_BOTTOM_FIELD);
    unsigned int is_bottom_field =
        !!(va_pic->flags & VA_PICTURE_HEVC_BOTTOM_FIELD);

    return ((is_long_term                         << 6) |
            ((is_top_field ^ is_bottom_field ^ 1) << 5) |
            (frame_store_id                       << 1) |
            ((is_top_field ^ 1) & is_bottom_field));
}
*/
static void
gen9_hcpe_ref_idx_state_1(struct intel_batchbuffer *batch,
                          int list,
                          struct intel_encoder_context *encoder_context,
                          struct encode_state *encode_state)
{
    int i;
    VAEncPictureParameterBufferHEVC *pic_param = (VAEncPictureParameterBufferHEVC *)encode_state->pic_param_ext->buffer;
    VAEncSliceParameterBufferHEVC *slice_param = (VAEncSliceParameterBufferHEVC *)encode_state->slice_params_ext[0]->buffer;
    uint8_t num_ref_minus1 = (list ? slice_param->num_ref_idx_l1_active_minus1 : slice_param->num_ref_idx_l0_active_minus1);
    VAPictureHEVC *ref_list = (list ? slice_param->ref_pic_list1 : slice_param->ref_pic_list0);
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct object_surface *obj_surface;
    int frame_index;

    int ref_idx_l0 = (vme_context->ref_index_in_mb[list] & 0xff);

    if (ref_idx_l0 > 3) {
        WARN_ONCE("ref_idx_l0 is out of range\n");
        ref_idx_l0 = 0;
    }

    obj_surface = vme_context->used_reference_objects[list];
    frame_index = -1;
    for (i = 0; i < 16; i++) {
        if (obj_surface &&
            obj_surface == encode_state->reference_objects[i]) {
            frame_index = i;
            break;
        }
    }
    if (frame_index == -1) {
        WARN_ONCE("RefPicList 0 or 1 is not found in DPB!\n");
    }

    if(num_ref_minus1 == 0 && frame_index == 1 && list == 0){
        WARN_ONCE("Input ref list is Wrong !\n");
    }

    BEGIN_BCS_BATCH(batch, 18);

    OUT_BCS_BATCH(batch, HCP_REF_IDX_STATE | (18 - 2));
    OUT_BCS_BATCH(batch,
                  num_ref_minus1 << 1 |
                  list);

    for (i = 0; i < 16; i++) {
        if (i < MIN((num_ref_minus1 + 1), 15)) {
            VAPictureHEVC *ref_pic = &ref_list[i];
            VAPictureHEVC *curr_pic = &pic_param->decoded_curr_pic;

            OUT_BCS_BATCH(batch,
                          1 << 15 |         /* bottom_field_flag 0 */
                          0 << 14 |         /* field_pic_flag 0 */
                          !!(ref_pic->flags & VA_PICTURE_HEVC_LONG_TERM_REFERENCE) << 13 |  /* short term is 1 */
                          0 << 12 | /* disable WP */
                          0 << 11 | /* disable WP */
                          frame_index << 8 |
                          (CLAMP(-128, 127, curr_pic->pic_order_cnt - ref_pic->pic_order_cnt) & 0xff));
        } else {
            OUT_BCS_BATCH(batch, 0);
        }
    }

    ADVANCE_BCS_BATCH(batch);
}

void
intel_hcpe_hevc_ref_idx_state(VADriverContextP ctx,
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context
                             )
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    VAEncSliceParameterBufferHEVC *slice_param = (VAEncSliceParameterBufferHEVC *)encode_state->slice_params_ext[0]->buffer;

    if (slice_param->slice_type == HEVC_SLICE_I)
        return;

    gen9_hcpe_ref_idx_state_1(batch, 0, encoder_context, encode_state);

    if (slice_param->slice_type == HEVC_SLICE_P)
        return;

    gen9_hcpe_ref_idx_state_1(batch, 1, encoder_context, encode_state);
}

static void
gen9_hcpe_hevc_slice_state(VADriverContextP ctx,
                           VAEncPictureParameterBufferHEVC *pic_param,
                           VAEncSliceParameterBufferHEVC *slice_param,
                           struct encode_state *encode_state,
                           struct intel_encoder_context *encoder_context,
                           struct intel_batchbuffer *batch)
{
    VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;
    int slice_type = slice_param->slice_type;

    int log2_cu_size = pSequenceParameter->log2_min_luma_coding_block_size_minus3 + 3;
    int log2_ctb_size = pSequenceParameter->log2_diff_max_min_luma_coding_block_size + log2_cu_size;
    int ctb_size = 1 << log2_ctb_size;
    int width_in_ctb = (pSequenceParameter->pic_width_in_luma_samples + ctb_size - 1) / ctb_size;
    int height_in_ctb = (pSequenceParameter->pic_height_in_luma_samples + ctb_size - 1) / ctb_size;
    int last_slice = (((slice_param->slice_segment_address + slice_param->num_ctu_in_slice) == (width_in_ctb * height_in_ctb)) ? 1 : 0);

    int slice_hor_pos, slice_ver_pos, next_slice_hor_pos, next_slice_ver_pos;

    slice_hor_pos = slice_param->slice_segment_address % width_in_ctb;
    slice_ver_pos = slice_param->slice_segment_address / width_in_ctb;

    next_slice_hor_pos = (slice_param->slice_segment_address + slice_param->num_ctu_in_slice) % width_in_ctb;
    next_slice_ver_pos = (slice_param->slice_segment_address + slice_param->num_ctu_in_slice) / width_in_ctb;

    /* only support multi slice begin from row start address */
    assert((slice_param->slice_segment_address % width_in_ctb) == 0);

    if (last_slice == 1) {
        if (slice_param->slice_segment_address == 0) {
            next_slice_hor_pos = 0;
            next_slice_ver_pos = height_in_ctb;
        } else {
            next_slice_hor_pos = 0;
            next_slice_ver_pos = 0;
        }
    }

    BEGIN_BCS_BATCH(batch, 9);

    OUT_BCS_BATCH(batch, HCP_SLICE_STATE | (9 - 2));

    OUT_BCS_BATCH(batch,
                  slice_ver_pos << 16 |
                  slice_hor_pos);
    OUT_BCS_BATCH(batch,
                  next_slice_ver_pos << 16 |
                  next_slice_hor_pos);
    OUT_BCS_BATCH(batch,
                  (slice_param->slice_cr_qp_offset & 0x1f) << 17 |
                  (slice_param->slice_cb_qp_offset & 0x1f) << 12 |
                  (pic_param->pic_init_qp + slice_param->slice_qp_delta) << 6 |
                  slice_param->slice_fields.bits.slice_temporal_mvp_enabled_flag << 5 |
                  slice_param->slice_fields.bits.dependent_slice_segment_flag << 4 |
                  last_slice << 2 |
                  slice_type);
    OUT_BCS_BATCH(batch,
                  0 << 26 |
                  (slice_param->max_num_merge_cand - 1)  << 23 |
                  slice_param->slice_fields.bits.cabac_init_flag << 22 |
                  slice_param->luma_log2_weight_denom << 19 |
                  (slice_param->luma_log2_weight_denom + slice_param->delta_chroma_log2_weight_denom) << 16 |
                  slice_param->slice_fields.bits.collocated_from_l0_flag << 15 |
                  (slice_type != HEVC_SLICE_B) << 14 |
                  slice_param->slice_fields.bits.mvd_l1_zero_flag << 13 |
                  slice_param->slice_fields.bits.slice_sao_luma_flag << 12 |
                  slice_param->slice_fields.bits.slice_sao_chroma_flag << 11 |
                  slice_param->slice_fields.bits.slice_loop_filter_across_slices_enabled_flag << 10 |
                  (slice_param->slice_beta_offset_div2 & 0xf) << 5 |
                  (slice_param->slice_tc_offset_div2 & 0xf) << 1 |
                  slice_param->slice_fields.bits.slice_deblocking_filter_disabled_flag);
    OUT_BCS_BATCH(batch, 0); /* DW 5 ,ignore for encoder.*/
    OUT_BCS_BATCH(batch,
                  4 << 26 |
                  4 << 20 |
                  0);
    OUT_BCS_BATCH(batch,
                  1 << 10 |  /* header insertion enable */
                  1 << 9  |  /* slice data enable */
                  1 << 8  |  /* tail insertion enable, must at end of frame, not slice */
                  1 << 2  |  /* RBSP or EBSP, EmulationByteSliceInsertEnable */
                  1 << 1  |  /* cabacZeroWordInsertionEnable */
                  0);        /* Ignored for decoding */
    OUT_BCS_BATCH(batch, 0); /* PAK-BSE data start offset */

    ADVANCE_BCS_BATCH(batch);
}

/* HEVC pipe line related */
static void gen9_hcpe_hevc_pipeline_picture_programing(VADriverContextP ctx,
        struct encode_state *encode_state,
        struct intel_encoder_context *encoder_context)
{
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;

    mfc_context->pipe_mode_select(ctx, HCP_CODEC_HEVC, encoder_context);
    mfc_context->set_surface_state(ctx, encode_state, encoder_context);
    gen9_hcpe_pipe_buf_addr_state(ctx, encode_state, encoder_context);
    mfc_context->ind_obj_base_addr_state(ctx, encoder_context);

    mfc_context->qm_state(ctx, encoder_context);
    mfc_context->fqm_state(ctx, encoder_context);
    mfc_context->pic_state(ctx, encode_state, encoder_context);
    intel_hcpe_hevc_ref_idx_state(ctx, encode_state, encoder_context);
}

static void gen9_hcpe_init(VADriverContextP ctx,
                           struct encode_state *encode_state,
                           struct intel_encoder_context *encoder_context)
{
    /* to do */
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;
    VAEncSliceParameterBufferHEVC *slice_param = (VAEncSliceParameterBufferHEVC *)encode_state->slice_params_ext[0]->buffer;
    dri_bo *bo;
    int i, size = 0;
    int slice_batchbuffer_size;
    int slice_type = slice_param->slice_type;
    int is_inter = (slice_type != HEVC_SLICE_I);

    int log2_cu_size = pSequenceParameter->log2_min_luma_coding_block_size_minus3 + 3;
    int log2_ctb_size = pSequenceParameter->log2_diff_max_min_luma_coding_block_size + log2_cu_size;
    int ctb_size = 1 << log2_ctb_size;
    int cu_size  = 1 << log2_cu_size;

    int width_in_ctb  = ALIGN(pSequenceParameter->pic_width_in_luma_samples , ctb_size) / ctb_size;
    int height_in_ctb = ALIGN(pSequenceParameter->pic_height_in_luma_samples, ctb_size) / ctb_size;
    int width_in_cu  = ALIGN(pSequenceParameter->pic_width_in_luma_samples , cu_size) / cu_size;
    int height_in_cu = ALIGN(pSequenceParameter->pic_height_in_luma_samples, cu_size) / cu_size;
    int width_in_mb  = ALIGN(pSequenceParameter->pic_width_in_luma_samples , 16) / 16;
    int height_in_mb = ALIGN(pSequenceParameter->pic_height_in_luma_samples, 16) / 16;

    int num_cu_record = 64;

    if (log2_ctb_size == 5) num_cu_record = 16;
    else if (log2_ctb_size == 4) num_cu_record = 4;
    else if (log2_ctb_size == 6) num_cu_record = 64;

    /* frame size in samples, cu,ctu, mb */
    mfc_context->pic_size.picture_width_in_samples = pSequenceParameter->pic_width_in_luma_samples;
    mfc_context->pic_size.picture_height_in_samples = pSequenceParameter->pic_height_in_luma_samples;
    mfc_context->pic_size.ctb_size = ctb_size;
    mfc_context->pic_size.picture_width_in_ctbs = width_in_ctb;
    mfc_context->pic_size.picture_height_in_ctbs = height_in_ctb;
    mfc_context->pic_size.min_cb_size = cu_size;
    mfc_context->pic_size.picture_width_in_min_cb_minus1 = width_in_cu - 1;
    mfc_context->pic_size.picture_height_in_min_cb_minus1 = height_in_cu - 1;
    mfc_context->pic_size.picture_width_in_mbs = width_in_mb;
    mfc_context->pic_size.picture_height_in_mbs = height_in_mb;

    slice_batchbuffer_size = 64 * width_in_ctb * width_in_ctb + 4096 +
                             (SLICE_HEADER + SLICE_TAIL) * encode_state->num_slice_params_ext;

    /*Encode common setup for HCP*/
    /*deblocking */
    dri_bo_unreference(mfc_context->deblocking_filter_line_buffer.bo);
    mfc_context->deblocking_filter_line_buffer.bo = NULL;

    dri_bo_unreference(mfc_context->deblocking_filter_tile_line_buffer.bo);
    mfc_context->deblocking_filter_tile_line_buffer.bo = NULL;

    dri_bo_unreference(mfc_context->deblocking_filter_tile_column_buffer.bo);
    mfc_context->deblocking_filter_tile_column_buffer.bo = NULL;

    /* input source */
    dri_bo_unreference(mfc_context->uncompressed_picture_source.bo);
    mfc_context->uncompressed_picture_source.bo = NULL;

    /* metadata */
    dri_bo_unreference(mfc_context->metadata_line_buffer.bo);
    mfc_context->metadata_line_buffer.bo = NULL;

    dri_bo_unreference(mfc_context->metadata_tile_line_buffer.bo);
    mfc_context->metadata_tile_line_buffer.bo = NULL;

    dri_bo_unreference(mfc_context->metadata_tile_column_buffer.bo);
    mfc_context->metadata_tile_column_buffer.bo = NULL;

    /* sao */
    dri_bo_unreference(mfc_context->sao_line_buffer.bo);
    mfc_context->sao_line_buffer.bo = NULL;

    dri_bo_unreference(mfc_context->sao_tile_line_buffer.bo);
    mfc_context->sao_tile_line_buffer.bo = NULL;

    dri_bo_unreference(mfc_context->sao_tile_column_buffer.bo);
    mfc_context->sao_tile_column_buffer.bo = NULL;

    /* mv temporal buffer */
    for (i = 0; i < NUM_HCP_CURRENT_COLLOCATED_MV_TEMPORAL_BUFFERS; i++) {
        if (mfc_context->current_collocated_mv_temporal_buffer[i].bo != NULL)
            dri_bo_unreference(mfc_context->current_collocated_mv_temporal_buffer[i].bo);
        mfc_context->current_collocated_mv_temporal_buffer[i].bo = NULL;
    }

    /* reference */
    for (i = 0; i < MAX_HCP_REFERENCE_SURFACES; i++) {
        if (mfc_context->reference_surfaces[i].bo != NULL)
            dri_bo_unreference(mfc_context->reference_surfaces[i].bo);
        mfc_context->reference_surfaces[i].bo = NULL;
    }

    /* indirect data CU recording */
    dri_bo_unreference(mfc_context->hcp_indirect_cu_object.bo);
    mfc_context->hcp_indirect_cu_object.bo = NULL;

    dri_bo_unreference(mfc_context->hcp_indirect_pak_bse_object.bo);
    mfc_context->hcp_indirect_pak_bse_object.bo = NULL;

    /* Current internal buffer for HCP */

    size = ALIGN(pSequenceParameter->pic_width_in_luma_samples, 32) >> 3;
    size <<= 6;
    ALLOC_ENCODER_BUFFER((&mfc_context->deblocking_filter_line_buffer), "line buffer", size);
    ALLOC_ENCODER_BUFFER((&mfc_context->deblocking_filter_tile_line_buffer), "tile line buffer", size);

    size = ALIGN(pSequenceParameter->pic_height_in_luma_samples + 6 * width_in_ctb, 32) >> 3;
    size <<= 6;
    ALLOC_ENCODER_BUFFER((&mfc_context->deblocking_filter_tile_column_buffer), "tile column buffer", size);

    if (is_inter) {
        size = (((pSequenceParameter->pic_width_in_luma_samples + 15) >> 4) * 188 + 9 * width_in_ctb + 1023) >> 9;
        size <<= 6;
        ALLOC_ENCODER_BUFFER((&mfc_context->metadata_line_buffer), "metadata line buffer", size);

        size = (((pSequenceParameter->pic_width_in_luma_samples + 15) >> 4) * 172 + 9 * width_in_ctb + 1023) >> 9;
        size <<= 6;
        ALLOC_ENCODER_BUFFER((&mfc_context->metadata_tile_line_buffer), "metadata tile line buffer", size);

        size = (((pSequenceParameter->pic_height_in_luma_samples + 15) >> 4) * 176 + 89 * width_in_ctb + 1023) >> 9;
        size <<= 6;
        ALLOC_ENCODER_BUFFER((&mfc_context->metadata_tile_column_buffer), "metadata tile column buffer", size);
    } else {
        size = (pSequenceParameter->pic_width_in_luma_samples + 8 * width_in_ctb + 1023) >> 9;
        size <<= 6;
        ALLOC_ENCODER_BUFFER((&mfc_context->metadata_line_buffer), "metadata line buffer", size);

        size = (pSequenceParameter->pic_width_in_luma_samples + 16 * width_in_ctb + 1023) >> 9;
        size <<= 6;
        ALLOC_ENCODER_BUFFER((&mfc_context->metadata_tile_line_buffer), "metadata tile line buffer", size);

        size = (pSequenceParameter->pic_height_in_luma_samples + 8 * height_in_ctb + 1023) >> 9;
        size <<= 6;
        ALLOC_ENCODER_BUFFER((&mfc_context->metadata_tile_column_buffer), "metadata tile column buffer", size);
    }

    size = ALIGN(((pSequenceParameter->pic_width_in_luma_samples >> 1) + 3 * width_in_ctb), 16) >> 3;
    size <<= 6;
    ALLOC_ENCODER_BUFFER((&mfc_context->sao_line_buffer), "sao line buffer", size);

    size = ALIGN(((pSequenceParameter->pic_width_in_luma_samples >> 1) + 6 * width_in_ctb), 16) >> 3;
    size <<= 6;
    ALLOC_ENCODER_BUFFER((&mfc_context->sao_tile_line_buffer), "sao tile line buffer", size);

    size = ALIGN(((pSequenceParameter->pic_height_in_luma_samples >> 1) + 6 * height_in_ctb), 16) >> 3;
    size <<= 6;
    ALLOC_ENCODER_BUFFER((&mfc_context->sao_tile_column_buffer), "sao tile column buffer", size);

    /////////////////////
    dri_bo_unreference(mfc_context->hcp_indirect_cu_object.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Indirect data CU Buffer",
                      width_in_ctb * height_in_ctb * num_cu_record * 16 * 4,
                      0x1000);
    assert(bo);
    mfc_context->hcp_indirect_cu_object.bo = bo;

    /* to do pak bse object buffer */
    /* to do current collocated mv temporal buffer */

    dri_bo_unreference(mfc_context->hcp_batchbuffer_surface.bo);
    mfc_context->hcp_batchbuffer_surface.bo = NULL;

    dri_bo_unreference(mfc_context->aux_batchbuffer_surface.bo);
    mfc_context->aux_batchbuffer_surface.bo = NULL;

    if (mfc_context->aux_batchbuffer)
        intel_batchbuffer_free(mfc_context->aux_batchbuffer);

    mfc_context->aux_batchbuffer = intel_batchbuffer_new(&i965->intel, I915_EXEC_BSD, slice_batchbuffer_size);
    mfc_context->aux_batchbuffer_surface.bo = mfc_context->aux_batchbuffer->buffer;
    dri_bo_reference(mfc_context->aux_batchbuffer_surface.bo);
    mfc_context->aux_batchbuffer_surface.pitch = 16;
    mfc_context->aux_batchbuffer_surface.num_blocks = mfc_context->aux_batchbuffer->size / 16;
    mfc_context->aux_batchbuffer_surface.size_block = 16;
}

static VAStatus gen9_hcpe_run(VADriverContextP ctx,
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;

    intel_batchbuffer_flush(batch);     //run the pipeline

    return VA_STATUS_SUCCESS;
}


static VAStatus
gen9_hcpe_stop(VADriverContextP ctx,
               struct encode_state *encode_state,
               struct intel_encoder_context *encoder_context,
               int *encoded_bits_size)
{
    VAStatus vaStatus = VA_STATUS_ERROR_UNKNOWN;
    VAEncPictureParameterBufferHEVC *pPicParameter = (VAEncPictureParameterBufferHEVC *)encode_state->pic_param_ext->buffer;
    VACodedBufferSegment *coded_buffer_segment;

    vaStatus = i965_MapBuffer(ctx, pPicParameter->coded_buf, (void **)&coded_buffer_segment);
    assert(vaStatus == VA_STATUS_SUCCESS);
    *encoded_bits_size = coded_buffer_segment->size * 8;
    i965_UnmapBuffer(ctx, pPicParameter->coded_buf);

    return VA_STATUS_SUCCESS;
}


int intel_hevc_find_skipemulcnt(unsigned char *buf, int bits_length)
{
    /* to do */
    int i, found;
    int leading_zero_cnt, byte_length, zero_byte;
    int nal_unit_type;
    int skip_cnt = 0;

#define NAL_UNIT_TYPE_MASK 0x7e
#define HW_MAX_SKIP_LENGTH 15

    byte_length = ALIGN(bits_length, 32) >> 3;


    leading_zero_cnt = 0;
    found = 0;
    for (i = 0; i < byte_length - 4; i++) {
        if (((buf[i] == 0) && (buf[i + 1] == 0) && (buf[i + 2] == 1)) ||
            ((buf[i] == 0) && (buf[i + 1] == 0) && (buf[i + 2] == 0) && (buf[i + 3] == 1))) {
            found = 1;
            break;
        }
        leading_zero_cnt++;
    }
    if (!found) {
        /* warning message is complained. But anyway it will be inserted. */
        WARN_ONCE("Invalid packed header data. "
                  "Can't find the 000001 start_prefix code\n");
        return 0;
    }
    i = leading_zero_cnt;

    zero_byte = 0;
    if (!((buf[i] == 0) && (buf[i + 1] == 0) && (buf[i + 2] == 1)))
        zero_byte = 1;

    skip_cnt = leading_zero_cnt + zero_byte + 3;

    /* the unit header byte is accounted */
    nal_unit_type = (buf[skip_cnt]) & NAL_UNIT_TYPE_MASK;
    skip_cnt += 1;
    skip_cnt += 1;  /* two bytes length of nal headers in hevc */

    if (nal_unit_type == 14 || nal_unit_type == 20 || nal_unit_type == 21) {
        /* more unit header bytes are accounted for MVC/SVC */
        //skip_cnt += 3;
    }
    if (skip_cnt > HW_MAX_SKIP_LENGTH) {
        WARN_ONCE("Too many leading zeros are padded for packed data. "
                  "It is beyond the HW range.!!!\n");
    }
    return skip_cnt;
}

#ifdef HCP_SOFTWARE_SKYLAKE

static int
gen9_hcpe_hevc_pak_object(VADriverContextP ctx, int lcu_x, int lcu_y, int isLast_ctb,
                          struct intel_encoder_context *encoder_context,
                          int cu_count_in_lcu, unsigned int split_coding_unit_flag,
                          struct intel_batchbuffer *batch)
{
    int len_in_dwords = 3;

    if (batch == NULL)
        batch = encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, len_in_dwords);

    OUT_BCS_BATCH(batch, HCP_PAK_OBJECT | (len_in_dwords - 2));
    OUT_BCS_BATCH(batch,
                  (((isLast_ctb > 0) ? 1 : 0) << 31) |  /* last ctb?*/
                  ((cu_count_in_lcu - 1) << 24) |           /* No motion vector */
                  split_coding_unit_flag);

    OUT_BCS_BATCH(batch, (lcu_y << 16) | lcu_x);        /* LCU  for Y*/

    ADVANCE_BCS_BATCH(batch);

    return len_in_dwords;
}

#define     AVC_INTRA_RDO_OFFSET    4
#define     AVC_INTER_RDO_OFFSET    10
#define     AVC_INTER_MSG_OFFSET    8
#define     AVC_INTER_MV_OFFSET     48
#define     AVC_RDO_MASK            0xFFFF

#define     AVC_INTRA_MODE_MASK     0x30
#define     AVC_INTRA_16X16         0x00
#define     AVC_INTRA_8X8           0x01
#define     AVC_INTRA_4X4           0x02

#define     AVC_INTER_MODE_MASK     0x03
#define     AVC_INTER_8X8           0x03
#define     AVC_INTER_8X16          0x02
#define     AVC_INTER_16X8          0x01
#define     AVC_INTER_16X16         0x00
#define     AVC_SUBMB_SHAPE_MASK    0x00FF00

/* VME output message, write back message */
#define     AVC_INTER_SUBMB_PRE_MODE_MASK       0x00ff0000
#define     AVC_SUBMB_SHAPE_MASK    0x00FF00

/* here 1 MB = 1CU = 16x16 */
static void
gen9_hcpe_hevc_fill_indirect_cu_intra(VADriverContextP ctx,
                                      struct encode_state *encode_state,
                                      struct intel_encoder_context *encoder_context,
                                      int qp, unsigned int *msg,
                                      int ctb_x, int ctb_y,
                                      int mb_x, int mb_y,
                                      int ctb_width_in_mb, int width_in_ctb, int num_cu_record, int slice_type,int cu_index,int index)
{
    /* here cu == mb, so we use mb address as the cu address */
    /* to fill the indirect cu by the vme out */
    static int intra_mode_8x8_avc2hevc[9] = {26, 10, 1, 34, 18, 24, 13, 28, 8};
    static int intra_mode_16x16_avc2hevc[4] = {26, 10, 1, 34};
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    unsigned char * cu_record_ptr = NULL;
    unsigned int * cu_msg = NULL;
    int ctb_address = (ctb_y * width_in_ctb + ctb_x) * num_cu_record;
    int mb_address_in_ctb = 0;
    int cu_address = (ctb_address + mb_address_in_ctb + cu_index) * 16 * 4;
    int zero = 0;
    int is_inter = 0;
    int intraMbMode = 0;
    int cu_part_mode = 0;
    int intraMode[4];
    int inerpred_idc = 0;
    int intra_chroma_mode = 5;
    int cu_size = 1;
    int tu_size = 0x55;
    int tu_count = 4;

    if (!is_inter) inerpred_idc = 0xff;

    intraMbMode = (msg[0] & AVC_INTRA_MODE_MASK) >> 4;


    if (intraMbMode == AVC_INTRA_16X16) {
        cu_part_mode = 0; //2Nx2N
        intra_chroma_mode = 5;
        cu_size = 1;
        tu_size = 0x55;
        tu_count = 4;
        intraMode[0] = intra_mode_16x16_avc2hevc[msg[1] & 0xf];
        intraMode[1] = intra_mode_16x16_avc2hevc[msg[1] & 0xf];
        intraMode[2] = intra_mode_16x16_avc2hevc[msg[1] & 0xf];
        intraMode[3] = intra_mode_16x16_avc2hevc[msg[1] & 0xf];
    } else if (intraMbMode == AVC_INTRA_8X8) {
        cu_part_mode = 0; //2Nx2N
        intra_chroma_mode = 5;
        cu_size = 0;
        tu_size = 0;
        tu_count = 4;
        intraMode[0] = intra_mode_8x8_avc2hevc[msg[1] >> (index << 2) & 0xf];
        intraMode[1] = intra_mode_8x8_avc2hevc[msg[1] >> (index << 2) & 0xf];
        intraMode[2] = intra_mode_8x8_avc2hevc[msg[1] >> (index << 2) & 0xf];
        intraMode[3] = intra_mode_8x8_avc2hevc[msg[1] >> (index << 2) & 0xf];

    } else { // for 4x4 to use 8x8 replace
        cu_part_mode = 3; //NxN
        intra_chroma_mode = 0;
        cu_size = 0;
        tu_size = 0;
        tu_count = 4;
        intraMode[0] = intra_mode_8x8_avc2hevc[msg[1] >> ((index << 4) + 0) & 0xf];
        intraMode[1] = intra_mode_8x8_avc2hevc[msg[1] >> ((index << 4) + 4) & 0xf];
        intraMode[2] = intra_mode_8x8_avc2hevc[msg[1] >> ((index << 4) + 8) & 0xf];
        intraMode[3] = intra_mode_8x8_avc2hevc[msg[1] >> ((index << 4) + 12) & 0xf];

    }

    cu_record_ptr = (unsigned char *)mfc_context->hcp_indirect_cu_object.bo->virtual;
    /* get the mb info from the vme out */
    cu_msg = (unsigned int *)(cu_record_ptr + cu_address);

    cu_msg[0] = (inerpred_idc << 24 |   /* interpred_idc[3:0][1:0] */
                 zero << 23 |   /* reserved */
                 qp << 16 | /* CU_qp */
                 zero << 11 |   /* reserved */
                 intra_chroma_mode << 8 |   /* intra_chroma_mode */
                 zero << 7 |    /* IPCM_enable , reserved for SKL*/
                 cu_part_mode << 4 |    /* cu_part_mode */
                 zero << 3 |    /* cu_transquant_bypass_flag */
                 is_inter << 2 |    /* cu_pred_mode :intra 1,inter 1*/
                 cu_size          /* cu_size */
                );
    cu_msg[1] = (zero << 30 |   /* reserved  */
                 intraMode[3] << 24 |   /* intra_mode */
                 zero << 22 |   /* reserved  */
                 intraMode[2] << 16 |   /* intra_mode */
                 zero << 14 |   /* reserved  */
                 intraMode[1] << 8 |    /* intra_mode */
                 zero << 6 |    /* reserved  */
                 intraMode[0]           /* intra_mode */
                );
    /* l0: 4 MV (x,y); l1； 4 MV (x,y) */
    cu_msg[2] = (zero << 16 |   /* mvx_l0[1]  */
                 zero           /* mvx_l0[0] */
                );
    cu_msg[3] = (zero << 16 |   /* mvx_l0[3]  */
                 zero           /* mvx_l0[2] */
                );
    cu_msg[4] = (zero << 16 |   /* mvy_l0[1]  */
                 zero           /* mvy_l0[0] */
                );
    cu_msg[5] = (zero << 16 |   /* mvy_l0[3]  */
                 zero           /* mvy_l0[2] */
                );

    cu_msg[6] = (zero << 16 |   /* mvx_l1[1]  */
                 zero           /* mvx_l1[0] */
                );
    cu_msg[7] = (zero << 16 |   /* mvx_l1[3]  */
                 zero           /* mvx_l1[2] */
                );
    cu_msg[8] = (zero << 16 |   /* mvy_l1[1]  */
                 zero           /* mvy_l1[0] */
                );
    cu_msg[9] = (zero << 16 |   /* mvy_l1[3]  */
                 zero           /* mvy_l1[2] */
                );

    cu_msg[10] = (zero << 28 |  /* ref_idx_l1[3]  */
                  zero << 24 |  /* ref_idx_l1[2] */
                  zero << 20 |  /* ref_idx_l1[1]  */
                  zero << 16 |  /* ref_idx_l1[0] */
                  zero << 12 |  /* ref_idx_l0[3]  */
                  zero << 8 |   /* ref_idx_l0[2] */
                  zero << 4 |   /* ref_idx_l0[1]  */
                  zero          /* ref_idx_l0[0] */
                 );

    cu_msg[11] = tu_size; /* tu_size 00000000 00000000 00000000 10101010  or 0x0*/
    cu_msg[12] = ((tu_count - 1) << 28 | /* tu count - 1 */
                  zero << 16 |  /* reserved  */
                  zero          /* tu_xform_Yskip[15:0] */
                 );
    cu_msg[13] = (zero << 16 |  /* tu_xform_Vskip[15:0]  */
                  zero          /* tu_xform_Uskip[15:0] */
                 );
    cu_msg[14] = zero ;
    cu_msg[15] = zero ;
}

/* here 1 MB = 1CU = 16x16 */
static void
gen9_hcpe_hevc_fill_indirect_cu_inter(VADriverContextP ctx,
                                      struct encode_state *encode_state,
                                      struct intel_encoder_context *encoder_context,
                                      int qp, unsigned int *msg,
                                      int ctb_x, int ctb_y,
                                      int mb_x, int mb_y,
                                      int ctb_width_in_mb, int width_in_ctb, int num_cu_record, int slice_type, int cu_index,int index)
{
    /* here cu == mb, so we use mb address as the cu address */
    /* to fill the indirect cu by the vme out */
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    unsigned char * cu_record_ptr = NULL;
    unsigned int * cu_msg = NULL;
    int ctb_address = (ctb_y * width_in_ctb + ctb_x) * num_cu_record;
    int mb_address_in_ctb = 0;
    int cu_address = (ctb_address + mb_address_in_ctb + cu_index) * 16 * 4;
    int zero = 0;
    int cu_part_mode = 0;
    int submb_pre_mode = 0;
    int is_inter = 1;
    int cu_size = 1;
    int tu_size = 0x55;
    int tu_count = 4;
    int inter_mode = 0;

    unsigned int *mv_ptr;
    {
        inter_mode = (msg[0] & AVC_INTER_MODE_MASK);
        submb_pre_mode = (msg[1] & AVC_INTER_SUBMB_PRE_MODE_MASK) >> 16;
#define MSG_MV_OFFSET   4
        mv_ptr = msg + MSG_MV_OFFSET;
        /* MV of VME output is based on 16 sub-blocks. So it is necessary
        * to convert them to be compatible with the format of AVC_PAK
        * command.
        */
        /* 0/2/4/6/8... ： l0, 1/3/5/7...: l1 ; now it only support 16x16,16x8,8x16,8x8*/

        if (inter_mode == AVC_INTER_16X16) {
            mv_ptr[4] = mv_ptr[0];
            mv_ptr[5] = mv_ptr[1];
            mv_ptr[2] = mv_ptr[0];
            mv_ptr[3] = mv_ptr[1];
            mv_ptr[6] = mv_ptr[0];
            mv_ptr[7] = mv_ptr[1];
            cu_part_mode = 0;
            cu_size = 1;
            tu_size = 0x55;
            tu_count = 4;
        } else if (inter_mode == AVC_INTER_8X16) {
            mv_ptr[4] = mv_ptr[0];
            mv_ptr[5] = mv_ptr[1];
            mv_ptr[2] = mv_ptr[8];
            mv_ptr[3] = mv_ptr[9];
            mv_ptr[6] = mv_ptr[8];
            mv_ptr[7] = mv_ptr[9];
            cu_part_mode = 1;
            cu_size = 1;
            tu_size = 0x55;
            tu_count = 4;
        } else if (inter_mode == AVC_INTER_16X8) {
            mv_ptr[2] = mv_ptr[0];
            mv_ptr[3] = mv_ptr[1];
            mv_ptr[4] = mv_ptr[16];
            mv_ptr[5] = mv_ptr[17];
            mv_ptr[6] = mv_ptr[24];
            mv_ptr[7] = mv_ptr[25];
            cu_part_mode = 2;
            cu_size = 1;
            tu_size = 0x55;
            tu_count = 4;
        }else if(inter_mode == AVC_INTER_8X8) {
            mv_ptr[0] = mv_ptr[index * 8 + 0 ];
            mv_ptr[1] = mv_ptr[index * 8 + 1 ];
            mv_ptr[2] = mv_ptr[index * 8 + 0 ];
            mv_ptr[3] = mv_ptr[index * 8 + 1 ];
            mv_ptr[4] = mv_ptr[index * 8 + 0 ];
            mv_ptr[5] = mv_ptr[index * 8 + 1 ];
            mv_ptr[6] = mv_ptr[index * 8 + 0 ];
            mv_ptr[7] = mv_ptr[index * 8 + 1 ];
            cu_part_mode = 0;
            cu_size = 0;
            tu_size = 0x0;
            tu_count = 4;

        }else
        {
            mv_ptr[4] = mv_ptr[0];
            mv_ptr[5] = mv_ptr[1];
            mv_ptr[2] = mv_ptr[0];
            mv_ptr[3] = mv_ptr[1];
            mv_ptr[6] = mv_ptr[0];
            mv_ptr[7] = mv_ptr[1];
            cu_part_mode = 0;
            cu_size = 1;
            tu_size = 0x55;
            tu_count = 4;

        }
    }

    cu_record_ptr = (unsigned char *)mfc_context->hcp_indirect_cu_object.bo->virtual;
    /* get the mb info from the vme out */
    cu_msg = (unsigned int *)(cu_record_ptr + cu_address);

    cu_msg[0] = (submb_pre_mode << 24 | /* interpred_idc[3:0][1:0] */
                 zero << 23 |   /* reserved */
                 qp << 16 | /* CU_qp */
                 zero << 11 |   /* reserved */
                 5 << 8 |   /* intra_chroma_mode */
                 zero << 7 |    /* IPCM_enable , reserved for SKL*/
                 cu_part_mode << 4 |    /* cu_part_mode */
                 zero << 3 |    /* cu_transquant_bypass_flag */
                 is_inter << 2 |    /* cu_pred_mode :intra 1,inter 1*/
                 cu_size          /* cu_size */
                );
    cu_msg[1] = (zero << 30 |   /* reserved  */
                 zero << 24 |   /* intra_mode */
                 zero << 22 |   /* reserved  */
                 zero << 16 |   /* intra_mode */
                 zero << 14 |   /* reserved  */
                 zero << 8 |    /* intra_mode */
                 zero << 6 |    /* reserved  */
                 zero           /* intra_mode */
                );
    /* l0: 4 MV (x,y); l1； 4 MV (x,y) */
    cu_msg[2] = ((mv_ptr[2] & 0xffff) << 16 |   /* mvx_l0[1]  */
                 (mv_ptr[0] & 0xffff)           /* mvx_l0[0] */
                );
    cu_msg[3] = ((mv_ptr[6] & 0xffff) << 16 |   /* mvx_l0[3]  */
                 (mv_ptr[4] & 0xffff)           /* mvx_l0[2] */
                );
    cu_msg[4] = ((mv_ptr[2] & 0xffff0000) |         /* mvy_l0[1]  */
                 (mv_ptr[0] & 0xffff0000) >> 16     /* mvy_l0[0] */
                );
    cu_msg[5] = ((mv_ptr[6] & 0xffff0000) |         /* mvy_l0[3]  */
                 (mv_ptr[4] & 0xffff0000) >> 16     /* mvy_l0[2] */
                );

    cu_msg[6] = ((mv_ptr[3] & 0xffff) << 16 |   /* mvx_l1[1]  */
                 (mv_ptr[1] & 0xffff)           /* mvx_l1[0] */
                );
    cu_msg[7] = ((mv_ptr[7] & 0xffff) << 16 |   /* mvx_l1[3]  */
                 (mv_ptr[5] & 0xffff)           /* mvx_l1[2] */
                );
    cu_msg[8] = ((mv_ptr[3] & 0xffff0000) |         /* mvy_l1[1]  */
                 (mv_ptr[1] & 0xffff0000) >> 16     /* mvy_l1[0] */
                );
    cu_msg[9] = ((mv_ptr[7] & 0xffff0000) |         /* mvy_l1[3]  */
                 (mv_ptr[5] & 0xffff0000) >> 16     /* mvy_l1[2] */
                );

    cu_msg[10] = (((vme_context->ref_index_in_mb[1] >> 24) & 0xf) << 28 |   /* ref_idx_l1[3]  */
                  ((vme_context->ref_index_in_mb[1] >> 16) & 0xf) << 24 |   /* ref_idx_l1[2] */
                  ((vme_context->ref_index_in_mb[1] >> 8) & 0xf) << 20 |    /* ref_idx_l1[1]  */
                  ((vme_context->ref_index_in_mb[1] >> 0) & 0xf) << 16 |    /* ref_idx_l1[0] */
                  ((vme_context->ref_index_in_mb[0] >> 24) & 0xf) << 12 |   /* ref_idx_l0[3]  */
                  ((vme_context->ref_index_in_mb[0] >> 16) & 0xf) << 8  |   /* ref_idx_l0[2] */
                  ((vme_context->ref_index_in_mb[0] >> 8) & 0xf) << 4 |     /* ref_idx_l0[1]  */
                  ((vme_context->ref_index_in_mb[0] >> 0) & 0xf)            /* ref_idx_l0[0] */
                 );

    cu_msg[11] = tu_size; /* tu_size 00000000 00000000 00000000 10101010  or 0x0*/
    cu_msg[12] = ((tu_count - 1) << 28 | /* tu count - 1 */
                  zero << 16 |  /* reserved  */
                  zero          /* tu_xform_Yskip[15:0] */
                 );
    cu_msg[13] = (zero << 16 |  /* tu_xform_Vskip[15:0]  */
                  zero          /* tu_xform_Uskip[15:0] */
                 );
    cu_msg[14] = zero ;
    cu_msg[15] = zero ;
}

#define HEVC_SPLIT_CU_FLAG_64_64 ((0x1<<20)|(0xf<<16)|(0x0<<12)|(0x0<<8)|(0x0<<4)|(0x0))
#define HEVC_SPLIT_CU_FLAG_32_32 ((0x1<<20)|(0x0<<16)|(0x0<<12)|(0x0<<8)|(0x0<<4)|(0x0))
#define HEVC_SPLIT_CU_FLAG_16_16 ((0x0<<20)|(0x0<<16)|(0x0<<12)|(0x0<<8)|(0x0<<4)|(0x0))
#define HEVC_SPLIT_CU_FLAG_8_8   ((0x1<<20)|(0x0<<16)|(0x0<<12)|(0x0<<8)|(0x0<<4)|(0x0))


void
intel_hevc_slice_insert_packed_data(VADriverContextP ctx,
                                    struct encode_state *encode_state,
                                    struct intel_encoder_context *encoder_context,
                                    int slice_index,
                                    struct intel_batchbuffer *slice_batch)
{
    int count, i, start_index;
    unsigned int length_in_bits;
    VAEncPackedHeaderParameterBuffer *param = NULL;
    unsigned int *header_data = NULL;
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    int slice_header_index;

    if (encode_state->slice_header_index[slice_index] == 0)
        slice_header_index = -1;
    else
        slice_header_index = (encode_state->slice_header_index[slice_index] & SLICE_PACKED_DATA_INDEX_MASK);

    count = encode_state->slice_rawdata_count[slice_index];
    start_index = (encode_state->slice_rawdata_index[slice_index] & SLICE_PACKED_DATA_INDEX_MASK);

    for (i = 0; i < count; i++) {
        unsigned int skip_emul_byte_cnt;

        header_data = (unsigned int *)encode_state->packed_header_data_ext[start_index + i]->buffer;

        param = (VAEncPackedHeaderParameterBuffer *)
                (encode_state->packed_header_params_ext[start_index + i]->buffer);

        /* skip the slice header packed data type as it is lastly inserted */
        if (param->type == VAEncPackedHeaderSlice)
            continue;

        length_in_bits = param->bit_length;

        skip_emul_byte_cnt = intel_hevc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);

        /* as the slice header is still required, the last header flag is set to
         * zero.
         */
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   0,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    }

    if (slice_header_index == -1) {
        unsigned char *slice_header = NULL;
        int slice_header_length_in_bits = 0;
        VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;
        VAEncPictureParameterBufferHEVC *pPicParameter = (VAEncPictureParameterBufferHEVC *)encode_state->pic_param_ext->buffer;
        VAEncSliceParameterBufferHEVC *pSliceParameter = (VAEncSliceParameterBufferHEVC *)encode_state->slice_params_ext[slice_index]->buffer;

        /* For the Normal HEVC */
        slice_header_length_in_bits = build_hevc_slice_header(pSequenceParameter,
                                      pPicParameter,
                                      pSliceParameter,
                                      &slice_header,
                                      0);
        mfc_context->insert_object(ctx, encoder_context,
                                   (unsigned int *)slice_header,
                                   ALIGN(slice_header_length_in_bits, 32) >> 5,
                                   slice_header_length_in_bits & 0x1f,
                                   5,  /* first 6 bytes are start code + nal unit type */
                                   1, 0, 1, slice_batch);
        free(slice_header);
    } else {
        unsigned int skip_emul_byte_cnt;

        header_data = (unsigned int *)encode_state->packed_header_data_ext[slice_header_index]->buffer;

        param = (VAEncPackedHeaderParameterBuffer *)
                (encode_state->packed_header_params_ext[slice_header_index]->buffer);
        length_in_bits = param->bit_length;

        /* as the slice header is the last header data for one slice,
         * the last header flag is set to one.
         */
        skip_emul_byte_cnt = intel_hevc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);

        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   1,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    }

    return;
}

static void
gen9_hcpe_hevc_pipeline_slice_programing(VADriverContextP ctx,
        struct encode_state *encode_state,
        struct intel_encoder_context *encoder_context,
        int slice_index,
        struct intel_batchbuffer *slice_batch)
{
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;
    VAEncPictureParameterBufferHEVC *pPicParameter = (VAEncPictureParameterBufferHEVC *)encode_state->pic_param_ext->buffer;
    VAEncSliceParameterBufferHEVC *pSliceParameter = (VAEncSliceParameterBufferHEVC *)encode_state->slice_params_ext[slice_index]->buffer;
    int qp_slice = pPicParameter->pic_init_qp + pSliceParameter->slice_qp_delta;
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    //unsigned char *slice_header = NULL;	  // for future use
    //int slice_header_length_in_bits = 0;
    unsigned int tail_data[] = { 0x0, 0x0 };
    int slice_type = pSliceParameter->slice_type;

    int log2_cu_size = pSequenceParameter->log2_min_luma_coding_block_size_minus3 + 3;
    int log2_ctb_size = pSequenceParameter->log2_diff_max_min_luma_coding_block_size + log2_cu_size;
    int ctb_size = 1 << log2_ctb_size;
    int width_in_ctb = (pSequenceParameter->pic_width_in_luma_samples + ctb_size - 1) / ctb_size;
    int height_in_ctb = (pSequenceParameter->pic_height_in_luma_samples + ctb_size - 1) / ctb_size;
    int last_slice = (pSliceParameter->slice_segment_address + pSliceParameter->num_ctu_in_slice) == (width_in_ctb * height_in_ctb);
    int ctb_width_in_mb = (ctb_size + 15) / 16;
    int i_ctb, ctb_x, ctb_y;
    unsigned int split_coding_unit_flag = 0;
    int width_in_mbs = (pSequenceParameter->pic_width_in_luma_samples + 15) / 16;
    int row_pad_flag = (pSequenceParameter->pic_height_in_luma_samples % ctb_size)> 0 ? 1:0;
    int col_pad_flag = (pSequenceParameter->pic_width_in_luma_samples % ctb_size)> 0 ? 1:0;

    int is_intra = (slice_type == HEVC_SLICE_I);
    unsigned int *msg = NULL;
    unsigned char *msg_ptr = NULL;
    int macroblock_address = 0;
    int num_cu_record = 64;
    int cu_count = 1;
    int tmp_mb_mode = 0;
    int mb_x = 0, mb_y = 0;
    int mb_addr = 0;
    int cu_index = 0;
    int inter_rdo, intra_rdo;
    int qp;

    if (log2_ctb_size == 5) num_cu_record = 16;
    else if (log2_ctb_size == 4) num_cu_record = 4;
    else if (log2_ctb_size == 6) num_cu_record = 64;

    qp = qp_slice;
    if (rate_control_mode == VA_RC_CBR) {
        qp = mfc_context->bit_rate_control_context[slice_type].QpPrimeY;
        if(slice_type == HEVC_SLICE_B) {
            if(pSequenceParameter->ip_period == 1)
            {
                qp = mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY;

            }else if(mfc_context->vui_hrd.i_frame_number % pSequenceParameter->ip_period == 1){
                qp = mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY;
            }
        }
        if (encode_state->slice_header_index[slice_index] == 0) {
            pSliceParameter->slice_qp_delta = qp - pPicParameter->pic_init_qp;
        }
    }

    /* only support for 8-bit pixel bit-depth */
    assert(pSequenceParameter->seq_fields.bits.bit_depth_luma_minus8 == 0);
    assert(pSequenceParameter->seq_fields.bits.bit_depth_chroma_minus8 == 0);
    assert(pPicParameter->pic_init_qp >= 0 && pPicParameter->pic_init_qp < 52);
    assert(qp >= 0 && qp < 52);

    {
        gen9_hcpe_hevc_slice_state(ctx,
                                   pPicParameter,
                                   pSliceParameter,
                                   encode_state, encoder_context,
                                   slice_batch);

        if (slice_index == 0)
            intel_hcpe_hevc_pipeline_header_programing(ctx, encode_state, encoder_context, slice_batch);

        intel_hevc_slice_insert_packed_data(ctx, encode_state, encoder_context, slice_index, slice_batch);

        /*
        slice_header_length_in_bits = build_hevc_slice_header(pSequenceParameter, pPicParameter, pSliceParameter, &slice_header, slice_index);
        int skip_emul_byte_cnt = intel_hevc_find_skipemulcnt((unsigned char *)slice_header, slice_header_length_in_bits);

        mfc_context->insert_object(ctx, encoder_context,
                                   (unsigned int *)slice_header, ALIGN(slice_header_length_in_bits, 32) >> 5, slice_header_length_in_bits & 0x1f,
                                    skip_emul_byte_cnt,
                                    1, 0, 1, slice_batch);
        free(slice_header);
        */
    }



    split_coding_unit_flag = (ctb_width_in_mb == 4) ? HEVC_SPLIT_CU_FLAG_64_64 : ((ctb_width_in_mb == 2) ? HEVC_SPLIT_CU_FLAG_32_32 : HEVC_SPLIT_CU_FLAG_16_16);

    dri_bo_map(vme_context->vme_output.bo , 1);
    msg_ptr = (unsigned char *)vme_context->vme_output.bo->virtual;
    dri_bo_map(mfc_context->hcp_indirect_cu_object.bo , 1);

    for (i_ctb = pSliceParameter->slice_segment_address;i_ctb < pSliceParameter->slice_segment_address + pSliceParameter->num_ctu_in_slice; i_ctb++) {
        int last_ctb = (i_ctb == (pSliceParameter->slice_segment_address + pSliceParameter->num_ctu_in_slice - 1));
        int ctb_height_in_mb_internal = ctb_width_in_mb;
        int ctb_width_in_mb_internal = ctb_width_in_mb;
        ctb_x = i_ctb % width_in_ctb;
        ctb_y = i_ctb / width_in_ctb;
        if(ctb_y == (height_in_ctb - 1) && row_pad_flag)  ctb_height_in_mb_internal = 1;
        if(ctb_x == (width_in_ctb - 1) && col_pad_flag)  ctb_width_in_mb_internal = 1;

        mb_x = 0;
        mb_y = 0;
        macroblock_address = ctb_y * width_in_mbs * ctb_width_in_mb + ctb_x * ctb_width_in_mb;
        split_coding_unit_flag = ((ctb_width_in_mb == 2) ? HEVC_SPLIT_CU_FLAG_32_32 : HEVC_SPLIT_CU_FLAG_16_16);
        cu_count = 1;
        cu_index = 0;
        mb_addr = 0;
        msg = NULL;
        for (mb_y = 0; mb_y < ctb_height_in_mb_internal; mb_y++)
        {
            mb_addr = macroblock_address + mb_y * width_in_mbs ;
            for (mb_x = 0; mb_x < ctb_width_in_mb_internal; mb_x++)
            {
                /* get the mb info from the vme out */
                msg = (unsigned int *)(msg_ptr + mb_addr * vme_context->vme_output.size_block);

                inter_rdo = msg[AVC_INTER_RDO_OFFSET] & AVC_RDO_MASK;
                intra_rdo = msg[AVC_INTRA_RDO_OFFSET] & AVC_RDO_MASK;
                /*fill to indirect cu */
                /*to do */
                if (is_intra || intra_rdo < inter_rdo) {
                    /* fill intra cu */
                    tmp_mb_mode = (msg[0] & AVC_INTRA_MODE_MASK) >> 4;
                    if (tmp_mb_mode == AVC_INTRA_16X16) {
                        gen9_hcpe_hevc_fill_indirect_cu_intra(ctx, encode_state, encoder_context, qp, msg, ctb_x, ctb_y, mb_x, mb_y, ctb_width_in_mb, width_in_ctb, num_cu_record, slice_type,cu_index++,0);
                    } else { // for 4x4 to use 8x8 replace
                        gen9_hcpe_hevc_fill_indirect_cu_intra(ctx, encode_state, encoder_context, qp, msg, ctb_x, ctb_y, mb_x, mb_y, ctb_width_in_mb, width_in_ctb, num_cu_record, slice_type,cu_index++,0);
                        gen9_hcpe_hevc_fill_indirect_cu_intra(ctx, encode_state, encoder_context, qp, msg, ctb_x, ctb_y, mb_x, mb_y, ctb_width_in_mb, width_in_ctb, num_cu_record, slice_type,cu_index++,1);
                        gen9_hcpe_hevc_fill_indirect_cu_intra(ctx, encode_state, encoder_context, qp, msg, ctb_x, ctb_y, mb_x, mb_y, ctb_width_in_mb, width_in_ctb, num_cu_record, slice_type,cu_index++,2);
                        gen9_hcpe_hevc_fill_indirect_cu_intra(ctx, encode_state, encoder_context, qp, msg, ctb_x, ctb_y, mb_x, mb_y, ctb_width_in_mb, width_in_ctb, num_cu_record, slice_type,cu_index++,3);
                        if(ctb_width_in_mb == 2)
                            split_coding_unit_flag |= 0x1 << (mb_x + mb_y * ctb_width_in_mb + 16);
                        else if(ctb_width_in_mb == 1)
                            split_coding_unit_flag |= 0x1 << 20;
                    }
                } else {
                    msg += AVC_INTER_MSG_OFFSET;
                    /* fill inter cu */
                    tmp_mb_mode = msg[0] & AVC_INTER_MODE_MASK;
                    if (tmp_mb_mode == AVC_INTER_8X8){
                        gen9_hcpe_hevc_fill_indirect_cu_inter(ctx, encode_state, encoder_context, qp, msg, ctb_x, ctb_y, mb_x, mb_y, ctb_width_in_mb, width_in_ctb, num_cu_record, slice_type,cu_index++,0);
                        gen9_hcpe_hevc_fill_indirect_cu_inter(ctx, encode_state, encoder_context, qp, msg, ctb_x, ctb_y, mb_x, mb_y, ctb_width_in_mb, width_in_ctb, num_cu_record, slice_type,cu_index++,1);
                        gen9_hcpe_hevc_fill_indirect_cu_inter(ctx, encode_state, encoder_context, qp, msg, ctb_x, ctb_y, mb_x, mb_y, ctb_width_in_mb, width_in_ctb, num_cu_record, slice_type,cu_index++,2);
                        gen9_hcpe_hevc_fill_indirect_cu_inter(ctx, encode_state, encoder_context, qp, msg, ctb_x, ctb_y, mb_x, mb_y, ctb_width_in_mb, width_in_ctb, num_cu_record, slice_type,cu_index++,3);
                        if(ctb_width_in_mb == 2)
                            split_coding_unit_flag |= 0x1 << (mb_x + mb_y * ctb_width_in_mb + 16);
                        else if(ctb_width_in_mb == 1)
                            split_coding_unit_flag |= 0x1 << 20;

                    }else if(tmp_mb_mode == AVC_INTER_16X16 ||
                        tmp_mb_mode == AVC_INTER_8X16 ||
                        tmp_mb_mode == AVC_INTER_16X8) {
                        gen9_hcpe_hevc_fill_indirect_cu_inter(ctx, encode_state, encoder_context, qp, msg, ctb_x, ctb_y, mb_x, mb_y, ctb_width_in_mb, width_in_ctb, num_cu_record, slice_type,cu_index++,0);
                    }
                }
                mb_addr++;
            }
        }

        cu_count = cu_index;
        // PAK object fill accordingly.
        gen9_hcpe_hevc_pak_object(ctx, ctb_x, ctb_y, last_ctb, encoder_context, cu_count, split_coding_unit_flag, slice_batch);
    }

    dri_bo_unmap(mfc_context->hcp_indirect_cu_object.bo);
    dri_bo_unmap(vme_context->vme_output.bo);

    if (last_slice) {
        mfc_context->insert_object(ctx, encoder_context,
                                   tail_data, 2, 8,
                                   2, 1, 1, 0, slice_batch);
    } else {
        mfc_context->insert_object(ctx, encoder_context,
                                   tail_data, 1, 8,
                                   1, 1, 1, 0, slice_batch);
    }
}

static dri_bo *
gen9_hcpe_hevc_software_batchbuffer(VADriverContextP ctx,
                                    struct encode_state *encode_state,
                                    struct intel_encoder_context *encoder_context)
{
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    struct intel_batchbuffer *batch;
    dri_bo *batch_bo;
    int i;

    batch = mfc_context->aux_batchbuffer;
    batch_bo = batch->buffer;

    for (i = 0; i < encode_state->num_slice_params_ext; i++) {
        gen9_hcpe_hevc_pipeline_slice_programing(ctx, encode_state, encoder_context, i, batch);
    }

    intel_batchbuffer_align(batch, 8);

    BEGIN_BCS_BATCH(batch, 2);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, MI_BATCH_BUFFER_END);
    ADVANCE_BCS_BATCH(batch);

    dri_bo_reference(batch_bo);
    intel_batchbuffer_free(batch);
    mfc_context->aux_batchbuffer = NULL;

    return batch_bo;
}

#else

#endif

static void
gen9_hcpe_hevc_pipeline_programing(VADriverContextP ctx,
                                   struct encode_state *encode_state,
                                   struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    dri_bo *slice_batch_bo;

#ifdef HCP_SOFTWARE_SKYLAKE
    slice_batch_bo = gen9_hcpe_hevc_software_batchbuffer(ctx, encode_state, encoder_context);
#else
    slice_batch_bo = gen9_hcpe_hevc_hardware_batchbuffer(ctx, encode_state, encoder_context);
#endif

    // begin programing
    if (i965->intel.has_bsd2)
        intel_batchbuffer_start_atomic_bcs_override(batch, 0x4000, BSD_RING0);
    else
        intel_batchbuffer_start_atomic_bcs(batch, 0x4000);
    intel_batchbuffer_emit_mi_flush(batch);

    // picture level programing
    gen9_hcpe_hevc_pipeline_picture_programing(ctx, encode_state, encoder_context);

    BEGIN_BCS_BATCH(batch, 3);
    OUT_BCS_BATCH(batch, MI_BATCH_BUFFER_START | (1 << 8) | (1 << 0));
    OUT_BCS_RELOC(batch,
                  slice_batch_bo,
                  I915_GEM_DOMAIN_COMMAND, 0,
                  0);
    OUT_BCS_BATCH(batch, 0);
    ADVANCE_BCS_BATCH(batch);

    // end programing
    intel_batchbuffer_end_atomic(batch);

    dri_bo_unreference(slice_batch_bo);
}

void intel_hcpe_hevc_pipeline_header_programing(VADriverContextP ctx,
        struct encode_state *encode_state,
        struct intel_encoder_context *encoder_context,
        struct intel_batchbuffer *slice_batch)
{
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    int idx = va_enc_packed_type_to_idx(VAEncPackedHeaderHEVC_VPS);
    unsigned int skip_emul_byte_cnt;

    if (encode_state->packed_header_data[idx]) {
        VAEncPackedHeaderParameterBuffer *param = NULL;
        unsigned int *header_data = (unsigned int *)encode_state->packed_header_data[idx]->buffer;
        unsigned int length_in_bits;

        assert(encode_state->packed_header_param[idx]);
        param = (VAEncPackedHeaderParameterBuffer *)encode_state->packed_header_param[idx]->buffer;
        length_in_bits = param->bit_length;

        skip_emul_byte_cnt = intel_hevc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   0,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    }

    idx = va_enc_packed_type_to_idx(VAEncPackedHeaderHEVC_VPS) + 1; // index to SPS

    if (encode_state->packed_header_data[idx]) {
        VAEncPackedHeaderParameterBuffer *param = NULL;
        unsigned int *header_data = (unsigned int *)encode_state->packed_header_data[idx]->buffer;
        unsigned int length_in_bits;

        assert(encode_state->packed_header_param[idx]);
        param = (VAEncPackedHeaderParameterBuffer *)encode_state->packed_header_param[idx]->buffer;
        length_in_bits = param->bit_length;

        skip_emul_byte_cnt = intel_hevc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   0,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    }

    idx = va_enc_packed_type_to_idx(VAEncPackedHeaderHEVC_PPS);

    if (encode_state->packed_header_data[idx]) {
        VAEncPackedHeaderParameterBuffer *param = NULL;
        unsigned int *header_data = (unsigned int *)encode_state->packed_header_data[idx]->buffer;
        unsigned int length_in_bits;

        assert(encode_state->packed_header_param[idx]);
        param = (VAEncPackedHeaderParameterBuffer *)encode_state->packed_header_param[idx]->buffer;
        length_in_bits = param->bit_length;

        skip_emul_byte_cnt = intel_hevc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);

        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   0,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    }

    idx = va_enc_packed_type_to_idx(VAEncPackedHeaderHEVC_SEI);

    if (encode_state->packed_header_data[idx]) {
        VAEncPackedHeaderParameterBuffer *param = NULL;
        unsigned int *header_data = (unsigned int *)encode_state->packed_header_data[idx]->buffer;
        unsigned int length_in_bits;

        assert(encode_state->packed_header_param[idx]);
        param = (VAEncPackedHeaderParameterBuffer *)encode_state->packed_header_param[idx]->buffer;
        length_in_bits = param->bit_length;

        skip_emul_byte_cnt = intel_hevc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   0,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    }
}

VAStatus intel_hcpe_hevc_prepare(VADriverContextP ctx,
                                 struct encode_state *encode_state,
                                 struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    struct object_surface *obj_surface;
    struct object_buffer *obj_buffer;
    GenHevcSurface *hevc_encoder_surface;
    dri_bo *bo;
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int i;
	struct i965_coded_buffer_segment *coded_buffer_segment;
    VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;

    /*Setup all the input&output object*/

    /* Setup current frame and current direct mv buffer*/
    obj_surface = encode_state->reconstructed_object;
    i965_check_alloc_surface_bo(ctx, obj_surface, 1, VA_FOURCC_NV12, SUBSAMPLE_YUV420);

    if (obj_surface->private_data == NULL) {
        uint32_t size;

        if (mfc_context->pic_size.ctb_size == 16)
            size = ((pSequenceParameter->pic_width_in_luma_samples + 63) >> 6) *
                   ((pSequenceParameter->pic_height_in_luma_samples + 15) >> 4);
        else
            size = ((pSequenceParameter->pic_width_in_luma_samples + 31) >> 5) *
                   ((pSequenceParameter->pic_height_in_luma_samples + 31) >> 5);
        size <<= 6; /* in unit of 64bytes */

        hevc_encoder_surface = calloc(sizeof(GenHevcSurface), 1);

        assert(hevc_encoder_surface);
        hevc_encoder_surface->motion_vector_temporal_bo =
            dri_bo_alloc(i965->intel.bufmgr,
                         "motion vector temporal buffer",
                         size,
                         0x1000);
        assert(hevc_encoder_surface->motion_vector_temporal_bo);

        obj_surface->private_data = (void *)hevc_encoder_surface;
        obj_surface->free_private_data = (void *)gen_free_hevc_surface;
    }
    hevc_encoder_surface = (GenHevcSurface *) obj_surface->private_data;

    if (hevc_encoder_surface) {
        hevc_encoder_surface->base.frame_store_id = -1;
        mfc_context->current_collocated_mv_temporal_buffer[NUM_HCP_CURRENT_COLLOCATED_MV_TEMPORAL_BUFFERS - 1].bo = hevc_encoder_surface->motion_vector_temporal_bo;

        dri_bo_reference(hevc_encoder_surface->motion_vector_temporal_bo);
    }

    mfc_context->surface_state.width = obj_surface->orig_width;
    mfc_context->surface_state.height = obj_surface->orig_height;
    mfc_context->surface_state.w_pitch = obj_surface->width;
    mfc_context->surface_state.h_pitch = obj_surface->height;

    /* Setup reference frames and direct mv buffers*/
    for (i = 0; i < MAX_HCP_REFERENCE_SURFACES; i++) {
        obj_surface = encode_state->reference_objects[i];

        if (obj_surface && obj_surface->bo) {
            mfc_context->reference_surfaces[i].bo = obj_surface->bo;
            dri_bo_reference(obj_surface->bo);

            /* Check MV temporal buffer */
            if (obj_surface->private_data == NULL) {
                uint32_t size;

                if (mfc_context->pic_size.ctb_size == 16)
                    size = ((pSequenceParameter->pic_width_in_luma_samples + 63) >> 6) *
                           ((pSequenceParameter->pic_height_in_luma_samples + 15) >> 4);
                else
                    size = ((pSequenceParameter->pic_width_in_luma_samples + 31) >> 5) *
                           ((pSequenceParameter->pic_height_in_luma_samples + 31) >> 5);
                size <<= 6; /* in unit of 64bytes */

                hevc_encoder_surface = calloc(sizeof(GenHevcSurface), 1);

                if (hevc_encoder_surface) {
                    hevc_encoder_surface->motion_vector_temporal_bo =
                        dri_bo_alloc(i965->intel.bufmgr,
                                     "motion vector temporal buffer",
                                     size,
                                     0x1000);
                    assert(hevc_encoder_surface->motion_vector_temporal_bo);
                }

                obj_surface->private_data = (void *)hevc_encoder_surface;
                obj_surface->free_private_data = (void *)gen_free_hevc_surface;
            }

            hevc_encoder_surface = (GenHevcSurface *) obj_surface->private_data;

            if (hevc_encoder_surface) {
                hevc_encoder_surface->base.frame_store_id = -1;
                /* Setup MV temporal buffer */
                mfc_context->current_collocated_mv_temporal_buffer[i].bo = hevc_encoder_surface->motion_vector_temporal_bo;
                dri_bo_reference(hevc_encoder_surface->motion_vector_temporal_bo);
            }
        } else {
            break;
        }
    }

    mfc_context->uncompressed_picture_source.bo = encode_state->input_yuv_object->bo;
    dri_bo_reference(mfc_context->uncompressed_picture_source.bo);

    obj_buffer = encode_state->coded_buf_object;
    bo = obj_buffer->buffer_store->bo;
    mfc_context->hcp_indirect_pak_bse_object.bo = bo;
    mfc_context->hcp_indirect_pak_bse_object.offset = I965_CODEDBUFFER_HEADER_SIZE;
    mfc_context->hcp_indirect_pak_bse_object.end_offset = ALIGN(obj_buffer->size_element - 0x1000, 0x1000);
    dri_bo_reference(mfc_context->hcp_indirect_pak_bse_object.bo);

    dri_bo_map(bo, 1);
    coded_buffer_segment = (struct i965_coded_buffer_segment *)(bo->virtual);
    coded_buffer_segment->mapped = 0;
    coded_buffer_segment->codec = encoder_context->codec;
    dri_bo_unmap(bo);

    return vaStatus;
}

/* HEVC BRC related */

static void
intel_hcpe_bit_rate_control_context_init(struct encode_state *encode_state,
        struct gen9_hcpe_context *mfc_context)
{
    VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;
    int ctb_size = 16;
    int width_in_mbs = (pSequenceParameter->pic_width_in_luma_samples + ctb_size - 1) / ctb_size;
    int height_in_mbs = (pSequenceParameter->pic_height_in_luma_samples + ctb_size - 1) / ctb_size;

    float fps =  pSequenceParameter->vui_time_scale * 0.5 / pSequenceParameter->vui_num_units_in_tick ;
    double bitrate = pSequenceParameter->bits_per_second * 1.0;
    int inter_mb_size = bitrate * 1.0 / (fps + 4.0) / width_in_mbs / height_in_mbs;
    int intra_mb_size = inter_mb_size * 5.0;
    int i;

    mfc_context->bit_rate_control_context[HEVC_SLICE_I].target_mb_size = intra_mb_size;
    mfc_context->bit_rate_control_context[HEVC_SLICE_I].target_frame_size = intra_mb_size * width_in_mbs * height_in_mbs;
    mfc_context->bit_rate_control_context[HEVC_SLICE_P].target_mb_size = inter_mb_size;
    mfc_context->bit_rate_control_context[HEVC_SLICE_P].target_frame_size = inter_mb_size * width_in_mbs * height_in_mbs;
    mfc_context->bit_rate_control_context[HEVC_SLICE_B].target_mb_size = inter_mb_size;
    mfc_context->bit_rate_control_context[HEVC_SLICE_B].target_frame_size = inter_mb_size * width_in_mbs * height_in_mbs;

    for (i = 0 ; i < 3; i++) {
        mfc_context->bit_rate_control_context[i].QpPrimeY = 26;
        mfc_context->bit_rate_control_context[i].MaxQpNegModifier = 6;
        mfc_context->bit_rate_control_context[i].MaxQpPosModifier = 6;
        mfc_context->bit_rate_control_context[i].GrowInit = 6;
        mfc_context->bit_rate_control_context[i].GrowResistance = 4;
        mfc_context->bit_rate_control_context[i].ShrinkInit = 6;
        mfc_context->bit_rate_control_context[i].ShrinkResistance = 4;

        mfc_context->bit_rate_control_context[i].Correct[0] = 8;
        mfc_context->bit_rate_control_context[i].Correct[1] = 4;
        mfc_context->bit_rate_control_context[i].Correct[2] = 2;
        mfc_context->bit_rate_control_context[i].Correct[3] = 2;
        mfc_context->bit_rate_control_context[i].Correct[4] = 4;
        mfc_context->bit_rate_control_context[i].Correct[5] = 8;
    }

    mfc_context->bit_rate_control_context[HEVC_SLICE_I].TargetSizeInWord = (intra_mb_size + 16) / 16;
    mfc_context->bit_rate_control_context[HEVC_SLICE_P].TargetSizeInWord = (inter_mb_size + 16) / 16;
    mfc_context->bit_rate_control_context[HEVC_SLICE_B].TargetSizeInWord = (inter_mb_size + 16) / 16;

    mfc_context->bit_rate_control_context[HEVC_SLICE_I].MaxSizeInWord = mfc_context->bit_rate_control_context[HEVC_SLICE_I].TargetSizeInWord * 1.5;
    mfc_context->bit_rate_control_context[HEVC_SLICE_P].MaxSizeInWord = mfc_context->bit_rate_control_context[HEVC_SLICE_P].TargetSizeInWord * 1.5;
    mfc_context->bit_rate_control_context[HEVC_SLICE_B].MaxSizeInWord = mfc_context->bit_rate_control_context[HEVC_SLICE_B].TargetSizeInWord * 1.5;
}

static void intel_hcpe_brc_init(struct encode_state *encode_state,
                                struct intel_encoder_context* encoder_context)
{
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;
    VAEncMiscParameterHRD* pParameterHRD = NULL;
    VAEncMiscParameterBuffer* pMiscParamHRD = NULL;

    double bitrate = pSequenceParameter->bits_per_second * 1.0;
    double framerate = (double)pSequenceParameter->vui_time_scale / (2 * (double)pSequenceParameter->vui_num_units_in_tick);
    int inum = 1, pnum = 0, bnum = 0; /* Gop structure: number of I, P, B frames in the Gop. */
    int intra_period = pSequenceParameter->intra_period;
    int ip_period = pSequenceParameter->ip_period;
    double qp1_size = 0.1 * 8 * 3 * pSequenceParameter->pic_width_in_luma_samples * pSequenceParameter->pic_height_in_luma_samples / 2;
    double qp51_size = 0.001 * 8 * 3 * pSequenceParameter->pic_width_in_luma_samples * pSequenceParameter->pic_height_in_luma_samples / 2;
    double bpf;
    int ratio_min = 1;
    int ratio_max = 32;
    int ratio = 8;
    double buffer_size = 0;

    if (!encode_state->misc_param[VAEncMiscParameterTypeHRD] || !encode_state->misc_param[VAEncMiscParameterTypeHRD]->buffer)
        return;

    pMiscParamHRD = (VAEncMiscParameterBuffer*)encode_state->misc_param[VAEncMiscParameterTypeHRD]->buffer;
    pParameterHRD = (VAEncMiscParameterHRD*)pMiscParamHRD->data;

    if (pSequenceParameter->ip_period) {
        pnum = (intra_period + ip_period - 1) / ip_period - 1;
        bnum = intra_period - inum - pnum;
    }

    mfc_context->brc.mode = encoder_context->rate_control_mode;

    mfc_context->brc.target_frame_size[HEVC_SLICE_I] = (int)((double)((bitrate * intra_period) / framerate) /
            (double)(inum + BRC_PWEIGHT * pnum + BRC_BWEIGHT * bnum));
    mfc_context->brc.target_frame_size[HEVC_SLICE_P] = BRC_PWEIGHT * mfc_context->brc.target_frame_size[HEVC_SLICE_I];
    mfc_context->brc.target_frame_size[HEVC_SLICE_B] = BRC_BWEIGHT * mfc_context->brc.target_frame_size[HEVC_SLICE_I];

    mfc_context->brc.gop_nums[HEVC_SLICE_I] = inum;
    mfc_context->brc.gop_nums[HEVC_SLICE_P] = pnum;
    mfc_context->brc.gop_nums[HEVC_SLICE_B] = bnum;

    bpf = mfc_context->brc.bits_per_frame = bitrate / framerate;

    if (!pParameterHRD || pParameterHRD->buffer_size <= 0)
    {
        mfc_context->hrd.buffer_size = bitrate * ratio;
        mfc_context->hrd.current_buffer_fullness =
            (double)(bitrate * ratio/2 < mfc_context->hrd.buffer_size) ?
            bitrate * ratio/2 : mfc_context->hrd.buffer_size / 2.;
    }else
    {
        buffer_size = (double)pParameterHRD->buffer_size ;
        if(buffer_size < bitrate * ratio_min)
        {
            buffer_size = bitrate * ratio_min;
        }else if (buffer_size > bitrate * ratio_max)
        {
            buffer_size = bitrate * ratio_max ;
        }
        mfc_context->hrd.buffer_size =buffer_size;
        if(pParameterHRD->initial_buffer_fullness > 0)
        {
            mfc_context->hrd.current_buffer_fullness =
                (double)(pParameterHRD->initial_buffer_fullness < mfc_context->hrd.buffer_size) ?
                pParameterHRD->initial_buffer_fullness : mfc_context->hrd.buffer_size / 2.;
        }else
        {
            mfc_context->hrd.current_buffer_fullness = mfc_context->hrd.buffer_size / 2.;

        }
    }

    mfc_context->hrd.target_buffer_fullness = (double)mfc_context->hrd.buffer_size / 2.;
    mfc_context->hrd.buffer_capacity = (double)mfc_context->hrd.buffer_size / qp1_size;
    mfc_context->hrd.violation_noted = 0;

    if ((bpf > qp51_size) && (bpf < qp1_size)) {
        mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY = 51 - 50 * (bpf - qp51_size) / (qp1_size - qp51_size);
    } else if (bpf >= qp1_size)
        mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY = 1;
    else if (bpf <= qp51_size)
        mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY = 51;

    mfc_context->bit_rate_control_context[HEVC_SLICE_I].QpPrimeY = mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY;
    mfc_context->bit_rate_control_context[HEVC_SLICE_B].QpPrimeY = mfc_context->bit_rate_control_context[HEVC_SLICE_I].QpPrimeY;

    BRC_CLIP(mfc_context->bit_rate_control_context[HEVC_SLICE_I].QpPrimeY, 1, 51);
    BRC_CLIP(mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY, 1, 51);
    BRC_CLIP(mfc_context->bit_rate_control_context[HEVC_SLICE_B].QpPrimeY, 1, 51);
}

int intel_hcpe_update_hrd(struct encode_state *encode_state,
                          struct gen9_hcpe_context *mfc_context,
                          int frame_bits)
{
    double prev_bf = mfc_context->hrd.current_buffer_fullness;

    mfc_context->hrd.current_buffer_fullness -= frame_bits;

    if (mfc_context->hrd.buffer_size > 0 && mfc_context->hrd.current_buffer_fullness <= 0.) {
        mfc_context->hrd.current_buffer_fullness = prev_bf;
        return BRC_UNDERFLOW;
    }

    mfc_context->hrd.current_buffer_fullness += mfc_context->brc.bits_per_frame;
    if (mfc_context->hrd.buffer_size > 0 && mfc_context->hrd.current_buffer_fullness > mfc_context->hrd.buffer_size) {
        if (mfc_context->brc.mode == VA_RC_VBR)
            mfc_context->hrd.current_buffer_fullness = mfc_context->hrd.buffer_size;
        else {
            mfc_context->hrd.current_buffer_fullness = prev_bf;
            return BRC_OVERFLOW;
        }
    }
    return BRC_NO_HRD_VIOLATION;
}

int intel_hcpe_brc_postpack(struct encode_state *encode_state,
                            struct gen9_hcpe_context *mfc_context,
                            int frame_bits)
{
    gen6_brc_status sts = BRC_NO_HRD_VIOLATION;
    VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;
    VAEncSliceParameterBufferHEVC *pSliceParameter = (VAEncSliceParameterBufferHEVC *)encode_state->slice_params_ext[0]->buffer;
    int slicetype = pSliceParameter->slice_type;
    int qpi = mfc_context->bit_rate_control_context[HEVC_SLICE_I].QpPrimeY;
    int qpp = mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY;
    int qpb = mfc_context->bit_rate_control_context[HEVC_SLICE_B].QpPrimeY;
    int qp; // quantizer of previously encoded slice of current type
    int qpn; // predicted quantizer for next frame of current type in integer format
    double qpf; // predicted quantizer for next frame of current type in float format
    double delta_qp; // QP correction
    int target_frame_size, frame_size_next;
    /* Notes:
     *  x - how far we are from HRD buffer borders
     *  y - how far we are from target HRD buffer fullness
     */
    double x, y;
    double frame_size_alpha;

    if(slicetype == HEVC_SLICE_B) {
        if(pSequenceParameter->ip_period == 1)
        {
            slicetype = HEVC_SLICE_P;
        }else if(mfc_context->vui_hrd.i_frame_number % pSequenceParameter->ip_period == 1){
            slicetype = HEVC_SLICE_P;
        }
    }

    qp = mfc_context->bit_rate_control_context[slicetype].QpPrimeY;

    target_frame_size = mfc_context->brc.target_frame_size[slicetype];
    if (mfc_context->hrd.buffer_capacity < 5)
        frame_size_alpha = 0;
    else
        frame_size_alpha = (double)mfc_context->brc.gop_nums[slicetype];
    if (frame_size_alpha > 30) frame_size_alpha = 30;
    frame_size_next = target_frame_size + (double)(target_frame_size - frame_bits) /
                      (double)(frame_size_alpha + 1.);

    /* frame_size_next: avoiding negative number and too small value */
    if ((double)frame_size_next < (double)(target_frame_size * 0.25))
        frame_size_next = (int)((double)target_frame_size * 0.25);

    qpf = (double)qp * target_frame_size / frame_size_next;
    qpn = (int)(qpf + 0.5);

    if (qpn == qp) {
        /* setting qpn we round qpf making mistakes: now we are trying to compensate this */
        mfc_context->brc.qpf_rounding_accumulator += qpf - qpn;
        if (mfc_context->brc.qpf_rounding_accumulator > 1.0) {
            qpn++;
            mfc_context->brc.qpf_rounding_accumulator = 0.;
        } else if (mfc_context->brc.qpf_rounding_accumulator < -1.0) {
            qpn--;
            mfc_context->brc.qpf_rounding_accumulator = 0.;
        }
    }
    /* making sure that QP is not changing too fast */
    if ((qpn - qp) > BRC_QP_MAX_CHANGE) qpn = qp + BRC_QP_MAX_CHANGE;
    else if ((qpn - qp) < -BRC_QP_MAX_CHANGE) qpn = qp - BRC_QP_MAX_CHANGE;
    /* making sure that with QP predictions we did do not leave QPs range */
    BRC_CLIP(qpn, 1, 51);

    /* checking wthether HRD compliance is still met */
    sts = intel_hcpe_update_hrd(encode_state, mfc_context, frame_bits);

    /* calculating QP delta as some function*/
    x = mfc_context->hrd.target_buffer_fullness - mfc_context->hrd.current_buffer_fullness;
    if (x > 0) {
        x /= mfc_context->hrd.target_buffer_fullness;
        y = mfc_context->hrd.current_buffer_fullness;
    } else {
        x /= (mfc_context->hrd.buffer_size - mfc_context->hrd.target_buffer_fullness);
        y = mfc_context->hrd.buffer_size - mfc_context->hrd.current_buffer_fullness;
    }
    if (y < 0.01) y = 0.01;
    if (x > 1) x = 1;
    else if (x < -1) x = -1;

    delta_qp = BRC_QP_MAX_CHANGE * exp(-1 / y) * sin(BRC_PI_0_5 * x);
    qpn = (int)(qpn + delta_qp + 0.5);

    /* making sure that with QP predictions we did do not leave QPs range */
    BRC_CLIP(qpn, 1, 51);

    if (sts == BRC_NO_HRD_VIOLATION) { // no HRD violation
        /* correcting QPs of slices of other types */
        if (slicetype == HEVC_SLICE_P) {
            if (abs(qpn + BRC_P_B_QP_DIFF - qpb) > 2)
                mfc_context->bit_rate_control_context[HEVC_SLICE_B].QpPrimeY += (qpn + BRC_P_B_QP_DIFF - qpb) >> 1;
            if (abs(qpn - BRC_I_P_QP_DIFF - qpi) > 2)
                mfc_context->bit_rate_control_context[HEVC_SLICE_I].QpPrimeY += (qpn - BRC_I_P_QP_DIFF - qpi) >> 1;
        } else if (slicetype == HEVC_SLICE_I) {
            if (abs(qpn + BRC_I_B_QP_DIFF - qpb) > 4)
                mfc_context->bit_rate_control_context[HEVC_SLICE_B].QpPrimeY += (qpn + BRC_I_B_QP_DIFF - qpb) >> 2;
            if (abs(qpn + BRC_I_P_QP_DIFF - qpp) > 2)
                mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY += (qpn + BRC_I_P_QP_DIFF - qpp) >> 2;
        } else { // HEVC_SLICE_B
            if (abs(qpn - BRC_P_B_QP_DIFF - qpp) > 2)
                mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY += (qpn - BRC_P_B_QP_DIFF - qpp) >> 1;
            if (abs(qpn - BRC_I_B_QP_DIFF - qpi) > 4)
                mfc_context->bit_rate_control_context[HEVC_SLICE_I].QpPrimeY += (qpn - BRC_I_B_QP_DIFF - qpi) >> 2;
        }
        BRC_CLIP(mfc_context->bit_rate_control_context[HEVC_SLICE_I].QpPrimeY, 1, 51);
        BRC_CLIP(mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY, 1, 51);
        BRC_CLIP(mfc_context->bit_rate_control_context[HEVC_SLICE_B].QpPrimeY, 1, 51);
    } else if (sts == BRC_UNDERFLOW) { // underflow
        if (qpn <= qp) qpn = qp + 1;
        if (qpn > 51) {
            qpn = 51;
            sts = BRC_UNDERFLOW_WITH_MAX_QP; //underflow with maxQP
        }
    } else if (sts == BRC_OVERFLOW) {
        if (qpn >= qp) qpn = qp - 1;
        if (qpn < 1) { // < 0 (?) overflow with minQP
            qpn = 1;
            sts = BRC_OVERFLOW_WITH_MIN_QP; // bit stuffing to be done
        }
    }

    mfc_context->bit_rate_control_context[slicetype].QpPrimeY = qpn;

    return sts;
}

static void intel_hcpe_hrd_context_init(struct encode_state *encode_state,
                                        struct intel_encoder_context *encoder_context)
{
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    int target_bit_rate = pSequenceParameter->bits_per_second;

    // current we only support CBR mode.
    if (rate_control_mode == VA_RC_CBR) {
        mfc_context->vui_hrd.i_bit_rate_value = target_bit_rate >> 10;
        mfc_context->vui_hrd.i_cpb_size_value = (target_bit_rate * 8) >> 10;
        mfc_context->vui_hrd.i_initial_cpb_removal_delay = mfc_context->vui_hrd.i_cpb_size_value * 0.5 * 1024 / target_bit_rate * 90000;
        mfc_context->vui_hrd.i_cpb_removal_delay = 2;
        mfc_context->vui_hrd.i_frame_number = 0;

        mfc_context->vui_hrd.i_initial_cpb_removal_delay_length = 24;
        mfc_context->vui_hrd.i_cpb_removal_delay_length = 24;
        mfc_context->vui_hrd.i_dpb_output_delay_length = 24;
    }

}

void
intel_hcpe_hrd_context_update(struct encode_state *encode_state,
                              struct gen9_hcpe_context *mfc_context)
{
    mfc_context->vui_hrd.i_frame_number++;
}

int intel_hcpe_interlace_check(VADriverContextP ctx,
                               struct encode_state *encode_state,
                               struct intel_encoder_context *encoder_context)
{
    VAEncSliceParameterBufferHEVC *pSliceParameter;
    VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;
    int log2_cu_size = pSequenceParameter->log2_min_luma_coding_block_size_minus3 + 3;
    int log2_ctb_size = pSequenceParameter->log2_diff_max_min_luma_coding_block_size + log2_cu_size;
    int ctb_size = 1 << log2_ctb_size;
    int width_in_ctb = (pSequenceParameter->pic_width_in_luma_samples + ctb_size - 1) / ctb_size;
    int height_in_ctb = (pSequenceParameter->pic_height_in_luma_samples + ctb_size - 1) / ctb_size;
    int i;
    int ctbCount = 0;

    for (i = 0; i < encode_state->num_slice_params_ext; i++) {
        pSliceParameter = (VAEncSliceParameterBufferHEVC *)encode_state->slice_params_ext[i]->buffer;
        ctbCount += pSliceParameter->num_ctu_in_slice;
    }

    if (ctbCount == (width_in_ctb * height_in_ctb))
        return 0;

    return 1;
}

/*
 * Check whether the parameters related with CBR are updated and decide whether
 * it needs to reinitialize the configuration related with CBR.
 * Currently it will check the following parameters:
 *      bits_per_second
 *      frame_rate
 *      gop_configuration(intra_period, ip_period, intra_idr_period)
 */
static bool intel_hcpe_brc_updated_check(struct encode_state *encode_state,
        struct intel_encoder_context *encoder_context)
{
    /* to do */
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    double cur_fps, cur_bitrate;
    VAEncSequenceParameterBufferHEVC *pSequenceParameter;


    if (rate_control_mode != VA_RC_CBR) {
        return false;
    }

    pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;

    cur_bitrate = pSequenceParameter->bits_per_second;
    cur_fps = (double)pSequenceParameter->vui_time_scale /
              (2 * (double)pSequenceParameter->vui_num_units_in_tick);

    if ((cur_bitrate == mfc_context->brc.saved_bps) &&
        (cur_fps == mfc_context->brc.saved_fps) &&
        (pSequenceParameter->intra_period == mfc_context->brc.saved_intra_period) &&
        (pSequenceParameter->intra_idr_period == mfc_context->brc.saved_idr_period) &&
        (pSequenceParameter->intra_period == mfc_context->brc.saved_intra_period)) {
        /* the parameters related with CBR are not updaetd */
        return false;
    }

    mfc_context->brc.saved_ip_period = pSequenceParameter->ip_period;
    mfc_context->brc.saved_intra_period = pSequenceParameter->intra_period;
    mfc_context->brc.saved_idr_period = pSequenceParameter->intra_idr_period;
    mfc_context->brc.saved_fps = cur_fps;
    mfc_context->brc.saved_bps = cur_bitrate;
    return true;
}

void intel_hcpe_brc_prepare(struct encode_state *encode_state,
                            struct intel_encoder_context *encoder_context)
{
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;

    if (rate_control_mode == VA_RC_CBR) {
        bool brc_updated;
        assert(encoder_context->codec != CODEC_MPEG2);

        brc_updated = intel_hcpe_brc_updated_check(encode_state, encoder_context);

        /*Programing bit rate control */
        if ((mfc_context->bit_rate_control_context[HEVC_SLICE_I].MaxSizeInWord == 0) ||
            brc_updated) {
            intel_hcpe_bit_rate_control_context_init(encode_state, mfc_context);
            intel_hcpe_brc_init(encode_state, encoder_context);
        }

        /*Programing HRD control */
        if ((mfc_context->vui_hrd.i_cpb_size_value == 0) || brc_updated)
            intel_hcpe_hrd_context_init(encode_state, encoder_context);
    }
}

/* HEVC interface API for encoder */

static VAStatus
gen9_hcpe_hevc_encode_picture(VADriverContextP ctx,
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context)
{
    struct gen9_hcpe_context *hcpe_context = encoder_context->mfc_context;
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    int current_frame_bits_size;
    int sts;

    for (;;) {
        gen9_hcpe_init(ctx, encode_state, encoder_context);
        intel_hcpe_hevc_prepare(ctx, encode_state, encoder_context);
        /*Programing bcs pipeline*/
        gen9_hcpe_hevc_pipeline_programing(ctx, encode_state, encoder_context); //filling the pipeline
        gen9_hcpe_run(ctx, encode_state, encoder_context);
        if (rate_control_mode == VA_RC_CBR /*|| rate_control_mode == VA_RC_VBR*/) {
            gen9_hcpe_stop(ctx, encode_state, encoder_context, &current_frame_bits_size);
            sts = intel_hcpe_brc_postpack(encode_state, hcpe_context, current_frame_bits_size);
            if (sts == BRC_NO_HRD_VIOLATION) {
                intel_hcpe_hrd_context_update(encode_state, hcpe_context);
                break;
            } else if (sts == BRC_OVERFLOW_WITH_MIN_QP || sts == BRC_UNDERFLOW_WITH_MAX_QP) {
                if (!hcpe_context->hrd.violation_noted) {
                    fprintf(stderr, "Unrepairable %s!\n", (sts == BRC_OVERFLOW_WITH_MIN_QP) ? "overflow" : "underflow");
                    hcpe_context->hrd.violation_noted = 1;
                }
                return VA_STATUS_SUCCESS;
            }
        } else {
            break;
        }
    }

    return VA_STATUS_SUCCESS;
}

void
gen9_hcpe_context_destroy(void *context)
{
    struct gen9_hcpe_context *hcpe_context = context;
    int i;

    dri_bo_unreference(hcpe_context->deblocking_filter_line_buffer.bo);
    hcpe_context->deblocking_filter_line_buffer.bo = NULL;

    dri_bo_unreference(hcpe_context->deblocking_filter_tile_line_buffer.bo);
    hcpe_context->deblocking_filter_tile_line_buffer.bo = NULL;

    dri_bo_unreference(hcpe_context->deblocking_filter_tile_column_buffer.bo);
    hcpe_context->deblocking_filter_tile_column_buffer.bo = NULL;

    dri_bo_unreference(hcpe_context->uncompressed_picture_source.bo);
    hcpe_context->uncompressed_picture_source.bo = NULL;

    dri_bo_unreference(hcpe_context->metadata_line_buffer.bo);
    hcpe_context->metadata_line_buffer.bo = NULL;

    dri_bo_unreference(hcpe_context->metadata_tile_line_buffer.bo);
    hcpe_context->metadata_tile_line_buffer.bo = NULL;

    dri_bo_unreference(hcpe_context->metadata_tile_column_buffer.bo);
    hcpe_context->metadata_tile_column_buffer.bo = NULL;

    dri_bo_unreference(hcpe_context->sao_line_buffer.bo);
    hcpe_context->sao_line_buffer.bo = NULL;

    dri_bo_unreference(hcpe_context->sao_tile_line_buffer.bo);
    hcpe_context->sao_tile_line_buffer.bo = NULL;

    dri_bo_unreference(hcpe_context->sao_tile_column_buffer.bo);
    hcpe_context->sao_tile_column_buffer.bo = NULL;

    /* mv temporal buffer */
    for (i = 0; i < NUM_HCP_CURRENT_COLLOCATED_MV_TEMPORAL_BUFFERS; i++) {
        if (hcpe_context->current_collocated_mv_temporal_buffer[i].bo != NULL)
            dri_bo_unreference(hcpe_context->current_collocated_mv_temporal_buffer[i].bo);
        hcpe_context->current_collocated_mv_temporal_buffer[i].bo = NULL;
    }

    for (i = 0; i < MAX_HCP_REFERENCE_SURFACES; i++) {
        dri_bo_unreference(hcpe_context->reference_surfaces[i].bo);
        hcpe_context->reference_surfaces[i].bo = NULL;
    }

    dri_bo_unreference(hcpe_context->hcp_indirect_cu_object.bo);
    hcpe_context->hcp_indirect_cu_object.bo = NULL;

    dri_bo_unreference(hcpe_context->hcp_indirect_pak_bse_object.bo);
    hcpe_context->hcp_indirect_pak_bse_object.bo = NULL;

    dri_bo_unreference(hcpe_context->hcp_batchbuffer_surface.bo);
    hcpe_context->hcp_batchbuffer_surface.bo = NULL;

    dri_bo_unreference(hcpe_context->aux_batchbuffer_surface.bo);
    hcpe_context->aux_batchbuffer_surface.bo = NULL;

    if (hcpe_context->aux_batchbuffer)
        intel_batchbuffer_free(hcpe_context->aux_batchbuffer);

    hcpe_context->aux_batchbuffer = NULL;

    free(hcpe_context);
}

VAStatus gen9_hcpe_pipeline(VADriverContextP ctx,
                            VAProfile profile,
                            struct encode_state *encode_state,
                            struct intel_encoder_context *encoder_context)
{
    VAStatus vaStatus;

    switch (profile) {
    case VAProfileHEVCMain:
        vaStatus = gen9_hcpe_hevc_encode_picture(ctx, encode_state, encoder_context);
        break;

    default:
        vaStatus = VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        break;
    }

    return vaStatus;
}

Bool gen9_hcpe_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct gen9_hcpe_context *hcpe_context = calloc(1, sizeof(struct gen9_hcpe_context));

    assert(hcpe_context);
    hcpe_context->pipe_mode_select = gen9_hcpe_pipe_mode_select;
    hcpe_context->set_surface_state = gen9_hcpe_surface_state;
    hcpe_context->ind_obj_base_addr_state = gen9_hcpe_ind_obj_base_addr_state;
    hcpe_context->pic_state = gen9_hcpe_hevc_pic_state;
    hcpe_context->qm_state = gen9_hcpe_hevc_qm_state;
    hcpe_context->fqm_state = gen9_hcpe_hevc_fqm_state;
    hcpe_context->insert_object = gen9_hcpe_hevc_insert_object;
    hcpe_context->buffer_suface_setup = gen8_gpe_buffer_suface_setup;

    encoder_context->mfc_context = hcpe_context;
    encoder_context->mfc_context_destroy = gen9_hcpe_context_destroy;
    encoder_context->mfc_pipeline = gen9_hcpe_pipeline;
    encoder_context->mfc_brc_prepare = intel_hcpe_brc_prepare;

    hevc_gen_default_iq_matrix_encoder(&hcpe_context->iq_matrix_hevc);

    return True;
}
