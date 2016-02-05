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
 *
 */

#include "sysdeps.h"

#include <va/va.h>
#include <va/va_dec_hevc.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"
#include "i965_defines.h"
#include "i965_drv_video.h"
#include "i965_decoder_utils.h"

#include "gen9_mfd.h"
#include "intel_media.h"
#include "vp9_probs.h"

#define OUT_BUFFER(buf_bo, is_target, ma)  do {                         \
        if (buf_bo) {                                                   \
            OUT_BCS_RELOC(batch,                                        \
                          buf_bo,                                       \
                          I915_GEM_DOMAIN_RENDER,                       \
                          is_target ? I915_GEM_DOMAIN_RENDER : 0,       \
                          0);                                           \
        } else {                                                        \
            OUT_BCS_BATCH(batch, 0);                                    \
        }                                                               \
        OUT_BCS_BATCH(batch, 0);                                        \
        if (ma)                                                         \
            OUT_BCS_BATCH(batch, 0);                                    \
    } while (0)

#define OUT_BUFFER_MA_TARGET(buf_bo)       OUT_BUFFER(buf_bo, 1, 1)
#define OUT_BUFFER_MA_REFERENCE(buf_bo)    OUT_BUFFER(buf_bo, 0, 1)
#define OUT_BUFFER_NMA_TARGET(buf_bo)      OUT_BUFFER(buf_bo, 1, 0)
#define OUT_BUFFER_NMA_REFERENCE(buf_bo)   OUT_BUFFER(buf_bo, 0, 0)

static void
gen9_hcpd_init_hevc_surface(VADriverContextP ctx,
                            VAPictureParameterBufferHEVC *pic_param,
                            struct object_surface *obj_surface,
                            struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    GenHevcSurface *gen9_hevc_surface;

    if (!obj_surface)
        return;

    obj_surface->free_private_data = gen_free_hevc_surface;
    gen9_hevc_surface = obj_surface->private_data;

    if (!gen9_hevc_surface) {
        gen9_hevc_surface = calloc(sizeof(GenHevcSurface), 1);
        assert(gen9_hevc_surface);
        gen9_hevc_surface->base.frame_store_id = -1;
        obj_surface->private_data = gen9_hevc_surface;
    }

    if (gen9_hevc_surface->motion_vector_temporal_bo == NULL) {
        uint32_t size;

        if (gen9_hcpd_context->ctb_size == 16)
            size = ((gen9_hcpd_context->picture_width_in_pixels + 63) >> 6) *
                ((gen9_hcpd_context->picture_height_in_pixels + 15) >> 4);
        else
            size = ((gen9_hcpd_context->picture_width_in_pixels + 31) >> 5) *
                ((gen9_hcpd_context->picture_height_in_pixels + 31) >> 5);

        size <<= 6; /* in unit of 64bytes */
        gen9_hevc_surface->motion_vector_temporal_bo = dri_bo_alloc(i965->intel.bufmgr,
                                                                    "motion vector temporal buffer",
                                                                    size,
                                                                    0x1000);
    }
}

static VAStatus
gen9_hcpd_hevc_decode_init(VADriverContextP ctx,
                           struct decode_state *decode_state,
                           struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAPictureParameterBufferHEVC *pic_param;
    struct object_surface *obj_surface;
    uint32_t size;
    int size_shift = 3;

    assert(decode_state->pic_param && decode_state->pic_param->buffer);
    pic_param = (VAPictureParameterBufferHEVC *)decode_state->pic_param->buffer;
    intel_update_hevc_frame_store_index(ctx,
                                        decode_state,
                                        pic_param,
                                        gen9_hcpd_context->reference_surfaces,
                                        &gen9_hcpd_context->fs_ctx);

    gen9_hcpd_context->picture_width_in_pixels = pic_param->pic_width_in_luma_samples;
    gen9_hcpd_context->picture_height_in_pixels = pic_param->pic_height_in_luma_samples;
    gen9_hcpd_context->ctb_size = (1 << (pic_param->log2_min_luma_coding_block_size_minus3 +
                                         3 +
                                         pic_param->log2_diff_max_min_luma_coding_block_size));
    gen9_hcpd_context->picture_width_in_ctbs = ALIGN(gen9_hcpd_context->picture_width_in_pixels, gen9_hcpd_context->ctb_size) / gen9_hcpd_context->ctb_size;
    gen9_hcpd_context->picture_height_in_ctbs = ALIGN(gen9_hcpd_context->picture_height_in_pixels, gen9_hcpd_context->ctb_size) / gen9_hcpd_context->ctb_size;
    gen9_hcpd_context->min_cb_size = (1 << (pic_param->log2_min_luma_coding_block_size_minus3 + 3));
    gen9_hcpd_context->picture_width_in_min_cb_minus1 = gen9_hcpd_context->picture_width_in_pixels / gen9_hcpd_context->min_cb_size - 1;
    gen9_hcpd_context->picture_height_in_min_cb_minus1 = gen9_hcpd_context->picture_height_in_pixels / gen9_hcpd_context->min_cb_size - 1;

    /* Current decoded picture */
    obj_surface = decode_state->render_object;
    hevc_ensure_surface_bo(ctx, decode_state, obj_surface, pic_param);
    gen9_hcpd_init_hevc_surface(ctx, pic_param, obj_surface, gen9_hcpd_context);

    if((pic_param->bit_depth_luma_minus8 > 0)
        || (pic_param->bit_depth_chroma_minus8 > 0))
        size_shift = 2;

    size = ALIGN(gen9_hcpd_context->picture_width_in_pixels, 32) >> size_shift;
    size <<= 6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->deblocking_filter_line_buffer), "line buffer", size);
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->deblocking_filter_tile_line_buffer), "tile line buffer", size);

    size = ALIGN(gen9_hcpd_context->picture_height_in_pixels + 6 * gen9_hcpd_context->picture_height_in_ctbs, 32) >> size_shift;
    size <<= 6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->deblocking_filter_tile_column_buffer), "tile column buffer", size);

    size = (((gen9_hcpd_context->picture_width_in_pixels + 15) >> 4) * 188 + 9 * gen9_hcpd_context->picture_width_in_ctbs + 1023) >> 9;
    size <<= 6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->metadata_line_buffer), "metadata line buffer", size);

    size = (((gen9_hcpd_context->picture_width_in_pixels + 15) >> 4) * 172 + 9 * gen9_hcpd_context->picture_width_in_ctbs + 1023) >> 9;
    size <<= 6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->metadata_tile_line_buffer), "metadata tile line buffer", size);

    if (IS_CHERRYVIEW(i965->intel.device_info))
        size = (((gen9_hcpd_context->picture_height_in_pixels + 15) >> 4) * 256 + 9 * gen9_hcpd_context->picture_height_in_ctbs + 1023) >> 9;
    else
        size = (((gen9_hcpd_context->picture_height_in_pixels + 15) >> 4) * 176 + 89 * gen9_hcpd_context->picture_height_in_ctbs + 1023) >> 9;
    size <<= 6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->metadata_tile_column_buffer), "metadata tile column buffer", size);

    size = ALIGN(((gen9_hcpd_context->picture_width_in_pixels >> 1) + 3 * gen9_hcpd_context->picture_width_in_ctbs), 16) >> size_shift;
    size <<= 6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->sao_line_buffer), "sao line buffer", size);

    size = ALIGN(((gen9_hcpd_context->picture_width_in_pixels >> 1) + 6 * gen9_hcpd_context->picture_width_in_ctbs), 16) >> size_shift;
    size <<= 6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->sao_tile_line_buffer), "sao tile line buffer", size);

    size = ALIGN(((gen9_hcpd_context->picture_height_in_pixels >> 1) + 6 * gen9_hcpd_context->picture_height_in_ctbs), 16) >> size_shift;
    size <<= 6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->sao_tile_column_buffer), "sao tile column buffer", size);

    gen9_hcpd_context->first_inter_slice_collocated_ref_idx = 0;
    gen9_hcpd_context->first_inter_slice_collocated_from_l0_flag = 0;
    gen9_hcpd_context->first_inter_slice_valid = 0;

    return VA_STATUS_SUCCESS;
}

static void
gen9_hcpd_pipe_mode_select(VADriverContextP ctx,
                           struct decode_state *decode_state,
                           int codec,
                           struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;

    assert((codec == HCP_CODEC_HEVC) || (codec == HCP_CODEC_VP9));

    BEGIN_BCS_BATCH(batch, 4);

    OUT_BCS_BATCH(batch, HCP_PIPE_MODE_SELECT | (4 - 2));
    OUT_BCS_BATCH(batch,
                  (codec << 5) |
                  (0 << 3) | /* disable Pic Status / Error Report */
                  HCP_CODEC_SELECT_DECODE);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpd_surface_state(VADriverContextP ctx,
                        struct decode_state *decode_state,
                        struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    struct object_surface *obj_surface = decode_state->render_object;
    unsigned int y_cb_offset;
    VAPictureParameterBufferHEVC *pic_param;

    assert(obj_surface);

    pic_param = (VAPictureParameterBufferHEVC *)decode_state->pic_param->buffer;
    y_cb_offset = obj_surface->y_cb_offset;

    BEGIN_BCS_BATCH(batch, 3);

    OUT_BCS_BATCH(batch, HCP_SURFACE_STATE | (3 - 2));
    OUT_BCS_BATCH(batch,
                  (0 << 28) |                   /* surface id */
                  (obj_surface->width - 1));    /* pitch - 1 */
    if((pic_param->bit_depth_luma_minus8 > 0)
        || (pic_param->bit_depth_chroma_minus8 > 0))
    {
        OUT_BCS_BATCH(batch,
                  (SURFACE_FORMAT_P010 << 28) |
                  y_cb_offset);
    }
    else
    {
        OUT_BCS_BATCH(batch,
                  (SURFACE_FORMAT_PLANAR_420_8 << 28) |
                  y_cb_offset);
    }

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpd_pipe_buf_addr_state(VADriverContextP ctx,
                              struct decode_state *decode_state,
                              struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    struct object_surface *obj_surface;
    GenHevcSurface *gen9_hevc_surface;
    int i;

    BEGIN_BCS_BATCH(batch, 95);

    OUT_BCS_BATCH(batch, HCP_PIPE_BUF_ADDR_STATE | (95 - 2));

    obj_surface = decode_state->render_object;
    assert(obj_surface && obj_surface->bo);
    gen9_hevc_surface = obj_surface->private_data;
    assert(gen9_hevc_surface && gen9_hevc_surface->motion_vector_temporal_bo);

    OUT_BUFFER_MA_TARGET(obj_surface->bo); /* DW 1..3 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->deblocking_filter_line_buffer.bo);/* DW 4..6 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->deblocking_filter_tile_line_buffer.bo); /* DW 7..9 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->deblocking_filter_tile_column_buffer.bo); /* DW 10..12 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->metadata_line_buffer.bo);         /* DW 13..15 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->metadata_tile_line_buffer.bo);    /* DW 16..18 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->metadata_tile_column_buffer.bo);  /* DW 19..21 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->sao_line_buffer.bo);              /* DW 22..24 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->sao_tile_line_buffer.bo);         /* DW 25..27 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->sao_tile_column_buffer.bo);       /* DW 28..30 */
    OUT_BUFFER_MA_TARGET(gen9_hevc_surface->motion_vector_temporal_bo); /* DW 31..33 */
    OUT_BUFFER_MA_TARGET(NULL); /* DW 34..36, reserved */

    for (i = 0; i < ARRAY_ELEMS(gen9_hcpd_context->reference_surfaces); i++) {
        obj_surface = gen9_hcpd_context->reference_surfaces[i].obj_surface;

        if (obj_surface)
            OUT_BUFFER_NMA_REFERENCE(obj_surface->bo);
        else
            OUT_BUFFER_NMA_REFERENCE(NULL);
    }
    OUT_BCS_BATCH(batch, 0);    /* DW 53, memory address attributes */

    OUT_BUFFER_MA_REFERENCE(NULL); /* DW 54..56, ignore for decoding mode */
    OUT_BUFFER_MA_TARGET(NULL);
    OUT_BUFFER_MA_TARGET(NULL);
    OUT_BUFFER_MA_TARGET(NULL);

    for (i = 0; i < ARRAY_ELEMS(gen9_hcpd_context->reference_surfaces); i++) {
        obj_surface = gen9_hcpd_context->reference_surfaces[i].obj_surface;
        gen9_hevc_surface = NULL;

        if (obj_surface && obj_surface->private_data)
            gen9_hevc_surface = obj_surface->private_data;

        if (gen9_hevc_surface)
            OUT_BUFFER_NMA_REFERENCE(gen9_hevc_surface->motion_vector_temporal_bo);
        else
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
gen9_hcpd_ind_obj_base_addr_state(VADriverContextP ctx,
                                  dri_bo *slice_data_bo,
                                  struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;

    BEGIN_BCS_BATCH(batch, 14);

    OUT_BCS_BATCH(batch, HCP_IND_OBJ_BASE_ADDR_STATE | (14 - 2));
    OUT_BUFFER_MA_REFERENCE(slice_data_bo);        /* DW 1..3 */
    OUT_BUFFER_NMA_REFERENCE(NULL);                /* DW 4..5, Upper Bound */
    OUT_BUFFER_MA_REFERENCE(NULL);                 /* DW 6..8, CU, ignored */
    OUT_BUFFER_MA_TARGET(NULL);                    /* DW 9..11, PAK-BSE, ignored */
    OUT_BUFFER_NMA_TARGET(NULL);                   /* DW 12..13, Upper Bound  */

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpd_qm_state(VADriverContextP ctx,
                   int size_id,
                   int color_component,
                   int pred_type,
                   int dc,
                   unsigned char *qm,
                   int qm_length,
                   struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    unsigned char qm_buffer[64];

    assert(qm_length <= 64);
    memset(qm_buffer, 0, sizeof(qm_buffer));
    memcpy(qm_buffer, qm, qm_length);

    BEGIN_BCS_BATCH(batch, 18);

    OUT_BCS_BATCH(batch, HCP_QM_STATE | (18 - 2));
    OUT_BCS_BATCH(batch,
                  dc << 5 |
                  color_component << 3 |
                  size_id << 1 |
                  pred_type);
    intel_batchbuffer_data(batch, qm_buffer, 64);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpd_hevc_qm_state(VADriverContextP ctx,
                        struct decode_state *decode_state,
                        struct gen9_hcpd_context *gen9_hcpd_context)
{
    VAIQMatrixBufferHEVC *iq_matrix;
    VAPictureParameterBufferHEVC *pic_param;
    int i;

    if (decode_state->iq_matrix && decode_state->iq_matrix->buffer)
        iq_matrix = (VAIQMatrixBufferHEVC *)decode_state->iq_matrix->buffer;
    else
        iq_matrix = &gen9_hcpd_context->iq_matrix_hevc;

    assert(decode_state->pic_param && decode_state->pic_param->buffer);
    pic_param = (VAPictureParameterBufferHEVC *)decode_state->pic_param->buffer;

    if (!pic_param->pic_fields.bits.scaling_list_enabled_flag)
        iq_matrix = &gen9_hcpd_context->iq_matrix_hevc;

    for (i = 0; i < 6; i++) {
        gen9_hcpd_qm_state(ctx,
                           0, i % 3, i / 3, 0,
                           iq_matrix->ScalingList4x4[i], 16,
                           gen9_hcpd_context);
    }

    for (i = 0; i < 6; i++) {
        gen9_hcpd_qm_state(ctx,
                           1, i % 3, i / 3, 0,
                           iq_matrix->ScalingList8x8[i], 64,
                           gen9_hcpd_context);
    }

    for (i = 0; i < 6; i++) {
        gen9_hcpd_qm_state(ctx,
                           2, i % 3, i / 3, iq_matrix->ScalingListDC16x16[i],
                           iq_matrix->ScalingList16x16[i], 64,
                           gen9_hcpd_context);
    }

    for (i = 0; i < 2; i++) {
        gen9_hcpd_qm_state(ctx,
                           3, 0, i % 2, iq_matrix->ScalingListDC32x32[i],
                           iq_matrix->ScalingList32x32[i], 64,
                           gen9_hcpd_context);
    }
}

static void
gen9_hcpd_pic_state(VADriverContextP ctx,
                    struct decode_state *decode_state,
                    struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    VAPictureParameterBufferHEVC *pic_param;
    int max_pcm_size_minus3 = 0, min_pcm_size_minus3 = 0;
    int pcm_sample_bit_depth_luma_minus1 = 7, pcm_sample_bit_depth_chroma_minus1 = 7;
    /*
     * 7.4.3.1
     *
     * When not present, the value of loop_filter_across_tiles_enabled_flag
     * is inferred to be equal to 1.
     */
    int loop_filter_across_tiles_enabled_flag = 1;

    assert(decode_state->pic_param && decode_state->pic_param->buffer);
    pic_param = (VAPictureParameterBufferHEVC *)decode_state->pic_param->buffer;

    if (pic_param->pic_fields.bits.pcm_enabled_flag) {
        max_pcm_size_minus3 = pic_param->log2_min_pcm_luma_coding_block_size_minus3 +
            pic_param->log2_diff_max_min_pcm_luma_coding_block_size;
        min_pcm_size_minus3 = pic_param->log2_min_pcm_luma_coding_block_size_minus3;
        pcm_sample_bit_depth_luma_minus1 = (pic_param->pcm_sample_bit_depth_luma_minus1 & 0x0f);
        pcm_sample_bit_depth_chroma_minus1 = (pic_param->pcm_sample_bit_depth_chroma_minus1 & 0x0f);
    } else {
        max_pcm_size_minus3 = MIN(pic_param->log2_min_luma_coding_block_size_minus3 + pic_param->log2_diff_max_min_luma_coding_block_size, 2);
    }

    if (pic_param->pic_fields.bits.tiles_enabled_flag)
        loop_filter_across_tiles_enabled_flag = pic_param->pic_fields.bits.loop_filter_across_tiles_enabled_flag;

    BEGIN_BCS_BATCH(batch, 19);

    OUT_BCS_BATCH(batch, HCP_PIC_STATE | (19 - 2));

    OUT_BCS_BATCH(batch,
                  gen9_hcpd_context->picture_height_in_min_cb_minus1 << 16 |
                  gen9_hcpd_context->picture_width_in_min_cb_minus1);
    OUT_BCS_BATCH(batch,
                  max_pcm_size_minus3 << 10 |
                  min_pcm_size_minus3 << 8 |
                  (pic_param->log2_min_transform_block_size_minus2 +
                   pic_param->log2_diff_max_min_transform_block_size) << 6 |
                  pic_param->log2_min_transform_block_size_minus2 << 4 |
                  (pic_param->log2_min_luma_coding_block_size_minus3 +
                   pic_param->log2_diff_max_min_luma_coding_block_size) << 2 |
                  pic_param->log2_min_luma_coding_block_size_minus3);
    OUT_BCS_BATCH(batch, 0); /* DW 3, ignored */
    OUT_BCS_BATCH(batch,
                  0 << 27 |
                  pic_param->pic_fields.bits.strong_intra_smoothing_enabled_flag << 26 |
                  pic_param->pic_fields.bits.transquant_bypass_enabled_flag << 25 |
                  pic_param->pic_fields.bits.amp_enabled_flag << 23 |
                  pic_param->pic_fields.bits.transform_skip_enabled_flag << 22 |
                  !(pic_param->CurrPic.flags & VA_PICTURE_HEVC_BOTTOM_FIELD) << 21 |
                  !!(pic_param->CurrPic.flags & VA_PICTURE_HEVC_FIELD_PIC) << 20 |
                  pic_param->pic_fields.bits.weighted_pred_flag << 19 |
                  pic_param->pic_fields.bits.weighted_bipred_flag << 18 |
                  pic_param->pic_fields.bits.tiles_enabled_flag << 17 |
                  pic_param->pic_fields.bits.entropy_coding_sync_enabled_flag << 16 |
                  loop_filter_across_tiles_enabled_flag << 15 |
                  pic_param->pic_fields.bits.sign_data_hiding_enabled_flag << 13 |
                  pic_param->log2_parallel_merge_level_minus2 << 10 |
                  pic_param->pic_fields.bits.constrained_intra_pred_flag << 9 |
                  pic_param->pic_fields.bits.pcm_loop_filter_disabled_flag << 8 |
                  (pic_param->diff_cu_qp_delta_depth & 0x03) << 6 |
                  pic_param->pic_fields.bits.cu_qp_delta_enabled_flag << 5 |
                  pic_param->pic_fields.bits.pcm_enabled_flag << 4 |
                  pic_param->slice_parsing_fields.bits.sample_adaptive_offset_enabled_flag << 3 |
                  0);
    OUT_BCS_BATCH(batch,
                  pic_param->bit_depth_luma_minus8 << 27 |
                  pic_param->bit_depth_chroma_minus8 << 24 |
                  pcm_sample_bit_depth_luma_minus1 << 20 |
                  pcm_sample_bit_depth_chroma_minus1 << 16 |
                  pic_param->max_transform_hierarchy_depth_inter << 13 |
                  pic_param->max_transform_hierarchy_depth_intra << 10 |
                  (pic_param->pps_cr_qp_offset & 0x1f) << 5 |
                  (pic_param->pps_cb_qp_offset & 0x1f));
    OUT_BCS_BATCH(batch,
                  0 << 29 |
                  0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0); /* DW 10 */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0); /* DW 15 */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpd_tile_state(VADriverContextP ctx,
                     struct decode_state *decode_state,
                     struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    VAPictureParameterBufferHEVC *pic_param;
    uint8_t pos_col[20], pos_row[24];
    int i;

    assert(decode_state->pic_param && decode_state->pic_param->buffer);
    pic_param = (VAPictureParameterBufferHEVC *)decode_state->pic_param->buffer;

    memset(pos_col, 0, sizeof(pos_col));
    memset(pos_row, 0, sizeof(pos_row));

    for (i = 0; i <= MIN(pic_param->num_tile_columns_minus1, 18); i++)
        pos_col[i + 1] = pos_col[i] + pic_param->column_width_minus1[i] + 1;

    for (i = 0; i <= MIN(pic_param->num_tile_rows_minus1, 20); i++)
        pos_row[i + 1] = pos_row[i] + pic_param->row_height_minus1[i] + 1;

    BEGIN_BCS_BATCH(batch, 13);

    OUT_BCS_BATCH(batch, HCP_TILE_STATE | (13 - 2));

    OUT_BCS_BATCH(batch,
                  pic_param->num_tile_columns_minus1 << 5 |
                  pic_param->num_tile_rows_minus1);
    intel_batchbuffer_data(batch, pos_col, 20);
    intel_batchbuffer_data(batch, pos_row, 24);

    ADVANCE_BCS_BATCH(batch);
}

static int
gen9_hcpd_get_reference_picture_frame_id(VAPictureHEVC *ref_pic,
                                         GenFrameStore frame_store[MAX_GEN_HCP_REFERENCE_FRAMES])
{
    int i;

    if (ref_pic->picture_id == VA_INVALID_ID ||
        (ref_pic->flags & VA_PICTURE_HEVC_INVALID))
        return 0;

    for (i = 0; i < MAX_GEN_HCP_REFERENCE_FRAMES; i++) {
        if (ref_pic->picture_id == frame_store[i].surface_id) {
            assert(frame_store[i].frame_store_id < MAX_GEN_HCP_REFERENCE_FRAMES);
            return frame_store[i].frame_store_id;
        }
    }

    /* Should never get here !!! */
    assert(0);
    return 0;
}

static void
gen9_hcpd_ref_idx_state_1(struct intel_batchbuffer *batch,
                          int list,
                          VAPictureParameterBufferHEVC *pic_param,
                          VASliceParameterBufferHEVC *slice_param,
                          GenFrameStore frame_store[MAX_GEN_HCP_REFERENCE_FRAMES])
{
    int i;
    uint8_t num_ref_minus1 = (list ? slice_param->num_ref_idx_l1_active_minus1 : slice_param->num_ref_idx_l0_active_minus1);
    uint8_t *ref_list = slice_param->RefPicList[list];

    BEGIN_BCS_BATCH(batch, 18);

    OUT_BCS_BATCH(batch, HCP_REF_IDX_STATE | (18 - 2));
    OUT_BCS_BATCH(batch,
                  num_ref_minus1 << 1 |
                  list);

    for (i = 0; i < 16; i++) {
        if (i < MIN((num_ref_minus1 + 1), 15)) {
            VAPictureHEVC *ref_pic = &pic_param->ReferenceFrames[ref_list[i]];
            VAPictureHEVC *curr_pic = &pic_param->CurrPic;

            OUT_BCS_BATCH(batch,
                          !(ref_pic->flags & VA_PICTURE_HEVC_BOTTOM_FIELD) << 15 |
                          !!(ref_pic->flags & VA_PICTURE_HEVC_FIELD_PIC) << 14 |
                          !!(ref_pic->flags & VA_PICTURE_HEVC_LONG_TERM_REFERENCE) << 13 |
                          0 << 12 |
                          0 << 11 |
                          gen9_hcpd_get_reference_picture_frame_id(ref_pic, frame_store) << 8 |
                          (CLAMP(-128, 127, curr_pic->pic_order_cnt - ref_pic->pic_order_cnt) & 0xff));
        } else {
            OUT_BCS_BATCH(batch, 0);
        }
    }

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpd_ref_idx_state(VADriverContextP ctx,
                        VAPictureParameterBufferHEVC *pic_param,
                        VASliceParameterBufferHEVC *slice_param,
                        struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;

    if (slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_I)
        return;

    gen9_hcpd_ref_idx_state_1(batch, 0, pic_param, slice_param, gen9_hcpd_context->reference_surfaces);

    if (slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_P)
        return;

    gen9_hcpd_ref_idx_state_1(batch, 1, pic_param, slice_param, gen9_hcpd_context->reference_surfaces);
}

static void
gen9_hcpd_weightoffset_state_1(struct intel_batchbuffer *batch,
                               int list,
                               VASliceParameterBufferHEVC *slice_param)
{
    int i;
    uint8_t num_ref_minus1 = (list == 1) ? slice_param->num_ref_idx_l1_active_minus1 : slice_param->num_ref_idx_l0_active_minus1;
    int8_t *luma_offset = (list == 1) ? slice_param->luma_offset_l1 : slice_param->luma_offset_l0;
    int8_t *delta_luma_weight = (list == 1) ? slice_param->delta_luma_weight_l1 : slice_param->delta_luma_weight_l0;
    int8_t (* chroma_offset)[2] = (list == 1) ? slice_param->ChromaOffsetL1 : slice_param->ChromaOffsetL0;
    int8_t (* delta_chroma_weight)[2] = (list == 1) ? slice_param->delta_chroma_weight_l1 : slice_param->delta_chroma_weight_l0;

    BEGIN_BCS_BATCH(batch, 34);

    OUT_BCS_BATCH(batch, HCP_WEIGHTOFFSET | (34 - 2));
    OUT_BCS_BATCH(batch, list);

    for (i = 0; i < 16; i++) {
        if (i < MIN((num_ref_minus1 + 1), 15)) {
            OUT_BCS_BATCH(batch,
                          (luma_offset[i] & 0xff) << 8 |
                          (delta_luma_weight[i] & 0xff));
        } else {
            OUT_BCS_BATCH(batch, 0);
        }
    }
    for (i = 0; i < 16; i++) {
        if (i < MIN((num_ref_minus1 + 1), 15)) {
            OUT_BCS_BATCH(batch,
                          (chroma_offset[i][1] & 0xff) << 24 |
                          (delta_chroma_weight[i][1] & 0xff) << 16 |
                          (chroma_offset[i][0] & 0xff) << 8 |
                          (delta_chroma_weight[i][0] & 0xff));
        } else {
            OUT_BCS_BATCH(batch, 0);
        }
    }

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpd_weightoffset_state(VADriverContextP ctx,
                             VAPictureParameterBufferHEVC *pic_param,
                             VASliceParameterBufferHEVC *slice_param,
                             struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;

    if (slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_I)
        return;

    if ((slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_P &&
         !pic_param->pic_fields.bits.weighted_pred_flag) ||
        (slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_B &&
         !pic_param->pic_fields.bits.weighted_bipred_flag))
        return;

    gen9_hcpd_weightoffset_state_1(batch, 0, slice_param);

    if (slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_P)
        return;

    gen9_hcpd_weightoffset_state_1(batch, 1, slice_param);
}

static int
gen9_hcpd_get_collocated_ref_idx(VADriverContextP ctx,
                                 VAPictureParameterBufferHEVC *pic_param,
                                 VASliceParameterBufferHEVC *slice_param,
                                 struct gen9_hcpd_context *gen9_hcpd_context)
{
    uint8_t *ref_list;
    VAPictureHEVC *ref_pic;

    if (slice_param->collocated_ref_idx > 14)
        return 0;

    if (!slice_param->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag)
        return 0;

    if (slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_I)
        return 0;

    if (slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_P ||
        (slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_B &&
         slice_param->LongSliceFlags.fields.collocated_from_l0_flag))
        ref_list = slice_param->RefPicList[0];
    else {
        assert(slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_B);
        ref_list = slice_param->RefPicList[1];
    }

    ref_pic = &pic_param->ReferenceFrames[ref_list[slice_param->collocated_ref_idx]];

    return gen9_hcpd_get_reference_picture_frame_id(ref_pic, gen9_hcpd_context->reference_surfaces);
}

static int
gen9_hcpd_is_list_low_delay(uint8_t ref_list_count,
                            uint8_t ref_list[15],
                            VAPictureHEVC *curr_pic,
                            VAPictureHEVC ref_surfaces[15])
{
    int i;

    for (i = 0; i < MIN(ref_list_count, 15); i++) {
        VAPictureHEVC *ref_pic;

        if (ref_list[i] > 14)
            continue;

        ref_pic = &ref_surfaces[ref_list[i]];

        if (ref_pic->pic_order_cnt > curr_pic->pic_order_cnt)
            return 0;
    }

    return 1;
}

static int
gen9_hcpd_is_low_delay(VADriverContextP ctx,
                       VAPictureParameterBufferHEVC *pic_param,
                       VASliceParameterBufferHEVC *slice_param)
{
    if (slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_I)
        return 0;
    else if (slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_P)
        return gen9_hcpd_is_list_low_delay(slice_param->num_ref_idx_l0_active_minus1 + 1,
                                           slice_param->RefPicList[0],
                                           &pic_param->CurrPic,
                                           pic_param->ReferenceFrames);
    else
        return gen9_hcpd_is_list_low_delay(slice_param->num_ref_idx_l0_active_minus1 + 1,
                                           slice_param->RefPicList[0],
                                           &pic_param->CurrPic,
                                           pic_param->ReferenceFrames) &&
            gen9_hcpd_is_list_low_delay(slice_param->num_ref_idx_l1_active_minus1 + 1,
                                        slice_param->RefPicList[1],
                                        &pic_param->CurrPic,
                                        pic_param->ReferenceFrames);
}

static void
gen9_hcpd_slice_state(VADriverContextP ctx,
                      VAPictureParameterBufferHEVC *pic_param,
                      VASliceParameterBufferHEVC *slice_param,
                      VASliceParameterBufferHEVC *next_slice_param,
                      struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    int slice_hor_pos, slice_ver_pos, next_slice_hor_pos, next_slice_ver_pos;
    unsigned short collocated_ref_idx, collocated_from_l0_flag;
    int sliceqp_sign_flag = 0, sliceqp = 0;

    slice_hor_pos = slice_param->slice_segment_address % gen9_hcpd_context->picture_width_in_ctbs;
    slice_ver_pos = slice_param->slice_segment_address / gen9_hcpd_context->picture_width_in_ctbs;

    if (next_slice_param) {
        next_slice_hor_pos = next_slice_param->slice_segment_address % gen9_hcpd_context->picture_width_in_ctbs;
        next_slice_ver_pos = next_slice_param->slice_segment_address / gen9_hcpd_context->picture_width_in_ctbs;
    } else {
        next_slice_hor_pos = 0;
        next_slice_ver_pos = 0;
    }

    collocated_ref_idx = gen9_hcpd_get_collocated_ref_idx(ctx, pic_param, slice_param, gen9_hcpd_context);
    collocated_from_l0_flag = slice_param->LongSliceFlags.fields.collocated_from_l0_flag;

    if ((!gen9_hcpd_context->first_inter_slice_valid) &&
        (slice_param->LongSliceFlags.fields.slice_type != HEVC_SLICE_I) &&
        slice_param->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag) {
        gen9_hcpd_context->first_inter_slice_collocated_ref_idx = collocated_ref_idx;
        gen9_hcpd_context->first_inter_slice_collocated_from_l0_flag = collocated_from_l0_flag;
        gen9_hcpd_context->first_inter_slice_valid = 1;
    }

    /* HW requirement */
    if (gen9_hcpd_context->first_inter_slice_valid &&
        ((slice_param->LongSliceFlags.fields.slice_type == HEVC_SLICE_I) ||
         (!slice_param->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag))) {
        collocated_ref_idx = gen9_hcpd_context->first_inter_slice_collocated_ref_idx;
        collocated_from_l0_flag = gen9_hcpd_context->first_inter_slice_collocated_from_l0_flag;
    }

    sliceqp = pic_param->init_qp_minus26 + 26 + slice_param->slice_qp_delta;
    if((pic_param->bit_depth_luma_minus8 > 0)
        || (pic_param->bit_depth_chroma_minus8 > 0))
    {
        if(sliceqp < 0)
        {
            sliceqp_sign_flag = 1;
            sliceqp = -sliceqp;
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
                  sliceqp << 6 |
                  slice_param->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag << 5 |
                  slice_param->LongSliceFlags.fields.dependent_slice_segment_flag << 4 |
                  sliceqp_sign_flag << 3 |
                  !next_slice_param << 2 |
                  slice_param->LongSliceFlags.fields.slice_type);
    OUT_BCS_BATCH(batch,
                  collocated_ref_idx << 26 |
                  (5 - slice_param->five_minus_max_num_merge_cand - 1) << 23 |
                  slice_param->LongSliceFlags.fields.cabac_init_flag << 22 |
                  slice_param->luma_log2_weight_denom << 19 |
                  ((slice_param->luma_log2_weight_denom + slice_param->delta_chroma_log2_weight_denom) & 0x7) << 16 |
                  collocated_from_l0_flag << 15 |
                  gen9_hcpd_is_low_delay(ctx, pic_param, slice_param) << 14 |
                  slice_param->LongSliceFlags.fields.mvd_l1_zero_flag << 13 |
                  slice_param->LongSliceFlags.fields.slice_sao_luma_flag << 12 |
                  slice_param->LongSliceFlags.fields.slice_sao_chroma_flag << 11 |
                  slice_param->LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag << 10 |
                  (slice_param->slice_beta_offset_div2 & 0xf) << 5 |
                  (slice_param->slice_tc_offset_div2 & 0xf) << 1 |
                  slice_param->LongSliceFlags.fields.slice_deblocking_filter_disabled_flag);
    OUT_BCS_BATCH(batch,
                  slice_param->slice_data_byte_offset); /* DW 5 */
    OUT_BCS_BATCH(batch,
                  0 << 26 |
                  0 << 20 |
                  0);
    OUT_BCS_BATCH(batch, 0);    /* Ignored for decoding */
    OUT_BCS_BATCH(batch, 0);    /* Ignored for decoding */

    ADVANCE_BCS_BATCH(batch);
}

static void
gen9_hcpd_bsd_object(VADriverContextP ctx,
                     VASliceParameterBufferHEVC *slice_param,
                     struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;

    BEGIN_BCS_BATCH(batch, 3);

    OUT_BCS_BATCH(batch, HCP_BSD_OBJECT | (3 - 2));

    OUT_BCS_BATCH(batch, slice_param->slice_data_size);
    OUT_BCS_BATCH(batch, slice_param->slice_data_offset);

    ADVANCE_BCS_BATCH(batch);
}

static VAStatus
gen9_hcpd_hevc_decode_picture(VADriverContextP ctx,
                              struct decode_state *decode_state,
                              struct gen9_hcpd_context *gen9_hcpd_context)
{
    VAStatus vaStatus;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    VAPictureParameterBufferHEVC *pic_param;
    VASliceParameterBufferHEVC *slice_param, *next_slice_param, *next_slice_group_param;
    dri_bo *slice_data_bo;
    int i, j;

    vaStatus = gen9_hcpd_hevc_decode_init(ctx, decode_state, gen9_hcpd_context);

    if (vaStatus != VA_STATUS_SUCCESS)
        goto out;

    assert(decode_state->pic_param && decode_state->pic_param->buffer);
    pic_param = (VAPictureParameterBufferHEVC *)decode_state->pic_param->buffer;

    if (i965->intel.has_bsd2)
        intel_batchbuffer_start_atomic_bcs_override(batch, 0x1000, BSD_RING0);
    else
        intel_batchbuffer_start_atomic_bcs(batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(batch);

    gen9_hcpd_pipe_mode_select(ctx, decode_state, HCP_CODEC_HEVC, gen9_hcpd_context);
    gen9_hcpd_surface_state(ctx, decode_state, gen9_hcpd_context);
    gen9_hcpd_pipe_buf_addr_state(ctx, decode_state, gen9_hcpd_context);
    gen9_hcpd_hevc_qm_state(ctx, decode_state, gen9_hcpd_context);
    gen9_hcpd_pic_state(ctx, decode_state, gen9_hcpd_context);

    if (pic_param->pic_fields.bits.tiles_enabled_flag)
        gen9_hcpd_tile_state(ctx, decode_state, gen9_hcpd_context);

    /* Need to double it works or not if the two slice groups have differenct slice data buffers */
    for (j = 0; j < decode_state->num_slice_params; j++) {
        assert(decode_state->slice_params && decode_state->slice_params[j]->buffer);
        slice_param = (VASliceParameterBufferHEVC *)decode_state->slice_params[j]->buffer;
        slice_data_bo = decode_state->slice_datas[j]->bo;

        gen9_hcpd_ind_obj_base_addr_state(ctx, slice_data_bo, gen9_hcpd_context);

        if (j == decode_state->num_slice_params - 1)
            next_slice_group_param = NULL;
        else
            next_slice_group_param = (VASliceParameterBufferHEVC *)decode_state->slice_params[j + 1]->buffer;

        for (i = 0; i < decode_state->slice_params[j]->num_elements; i++) {
            if (i < decode_state->slice_params[j]->num_elements - 1)
                next_slice_param = slice_param + 1;
            else
                next_slice_param = next_slice_group_param;

            gen9_hcpd_slice_state(ctx, pic_param, slice_param, next_slice_param, gen9_hcpd_context);
            gen9_hcpd_ref_idx_state(ctx, pic_param, slice_param, gen9_hcpd_context);
            gen9_hcpd_weightoffset_state(ctx, pic_param, slice_param, gen9_hcpd_context);
            gen9_hcpd_bsd_object(ctx, slice_param, gen9_hcpd_context);
            slice_param++;
        }
    }

    intel_batchbuffer_end_atomic(batch);
    intel_batchbuffer_flush(batch);

out:
    return vaStatus;
}

/*********************************************************/
/*                  VP9 Code                             */
/*********************************************************/


#define VP9_PROB_BUFFER_FIRST_PART_SIZE 2010
#define VP9_PROB_BUFFER_SECOND_PART_SIZE 10
#define VP9_PROB_BUFFER_KEY_INTER_OFFSET 1667
#define VP9_PROB_BUFFER_KEY_INTER_SIZE   343

#define VP9_PROB_BUFFER_UPDATE_NO   0
#define VP9_PROB_BUFFER_UPDATE_SECNE_1    1
#define VP9_PROB_BUFFER_UPDATE_SECNE_2    2
#define VP9_PROB_BUFFER_UPDATE_SECNE_3    3
#define VP9_PROB_BUFFER_UPDATE_SECNE_4    4
#define VP9_PROB_BUFFER_UPDATE_SECNE_5    5

#define VP9_PROB_BUFFER_SAVED_NO   0
#define VP9_PROB_BUFFER_SAVED_SECNE_1    1
#define VP9_PROB_BUFFER_SAVED_SECNE_2    2

#define VP9_PROB_BUFFER_RESTORED_NO   0
#define VP9_PROB_BUFFER_RESTORED_SECNE_1    1
#define VP9_PROB_BUFFER_RESTORED_SECNE_2    2
#define VP9_PROB_BUFFER_RESTORED_SECNE_MAX    (VP9_PROB_BUFFER_RESTORED_SECNE_2 + 1)

static void vp9_update_segmentId_buffer(VADriverContextP ctx,
                          struct decode_state *decode_state,
                          struct gen9_hcpd_context *gen9_hcpd_context, uint8_t isScaling)
{
    VADecPictureParameterBufferVP9 *pic_param;

    assert(decode_state->pic_param && decode_state->pic_param->buffer);
    pic_param = (VADecPictureParameterBufferVP9 *)decode_state->pic_param->buffer;

    int size = 0;

    if((pic_param->pic_fields.bits.frame_type == HCP_VP9_KEY_FRAME) ||
        pic_param->pic_fields.bits.error_resilient_mode ||
        pic_param->pic_fields.bits.intra_only || isScaling) {

        size = (gen9_hcpd_context->picture_width_in_min_cb_minus1+2)*(gen9_hcpd_context->picture_height_in_min_cb_minus1 + 2) * 1;
        size<<=6;
        //VP9 Segment ID buffer needs to be zero
        dri_bo_map(gen9_hcpd_context->vp9_segment_id_buffer.bo,1);
        memset((unsigned char *)gen9_hcpd_context->vp9_segment_id_buffer.bo->virtual,0, size);
        dri_bo_unmap(gen9_hcpd_context->vp9_segment_id_buffer.bo);
    }
}

static void
vp9_gen_default_probabilities(VADriverContextP ctx, struct gen9_hcpd_context *gen9_hcpd_context)
{
    int i = 0;
    uint32_t size = 0;

    size = sizeof(FRAME_CONTEXT);
    memset(&gen9_hcpd_context->vp9_fc_key_default,0,size);
    memset(&gen9_hcpd_context->vp9_fc_inter_default,0,size);
    memset(&gen9_hcpd_context->vp9_frame_ctx,0,size*FRAME_CONTEXTS);
    //more code to come here below

    //1. key default
    gen9_hcpd_context->vp9_fc_key_default.tx_probs = default_tx_probs;
    //dummy 52
    vp9_copy(gen9_hcpd_context->vp9_fc_key_default.coeff_probs4x4, default_coef_probs_4x4);
    vp9_copy(gen9_hcpd_context->vp9_fc_key_default.coeff_probs8x8, default_coef_probs_8x8);
    vp9_copy(gen9_hcpd_context->vp9_fc_key_default.coeff_probs16x16, default_coef_probs_16x16);
    vp9_copy(gen9_hcpd_context->vp9_fc_key_default.coeff_probs32x32, default_coef_probs_32x32);
    //dummy 16
    vp9_copy(gen9_hcpd_context->vp9_fc_key_default.skip_probs, default_skip_probs);
    vp9_copy(gen9_hcpd_context->vp9_fc_key_default.partition_prob, vp9_kf_partition_probs);
    //dummy 47
    vp9_copy(gen9_hcpd_context->vp9_fc_key_default.uv_mode_prob, vp9_kf_uv_mode_prob);
    vp9_copy(gen9_hcpd_context->vp9_fc_key_default.seg_tree_probs, default_seg_tree_probs);
    vp9_copy(gen9_hcpd_context->vp9_fc_key_default.seg_pred_probs, default_seg_pred_probs);

    //2. inter default
    gen9_hcpd_context->vp9_fc_inter_default.tx_probs = default_tx_probs;
    //dummy 52
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.coeff_probs4x4, default_coef_probs_4x4);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.coeff_probs8x8, default_coef_probs_8x8);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.coeff_probs16x16, default_coef_probs_16x16);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.coeff_probs32x32, default_coef_probs_32x32);
    //dummy 16
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.skip_probs, default_skip_probs);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.inter_mode_probs, default_inter_mode_probs);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.switchable_interp_prob, default_switchable_interp_prob);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.intra_inter_prob, default_intra_inter_p);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.comp_inter_prob, default_comp_inter_p);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.single_ref_prob, default_single_ref_p);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.comp_ref_prob, default_comp_ref_p);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.y_mode_prob, default_if_y_probs);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.partition_prob, default_partition_probs);
    gen9_hcpd_context->vp9_fc_inter_default.nmvc = default_nmv_context;
    //dummy 47
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.uv_mode_prob, default_if_uv_probs);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.seg_tree_probs, default_seg_tree_probs);
    vp9_copy(gen9_hcpd_context->vp9_fc_inter_default.seg_pred_probs, default_seg_pred_probs);

    for(i = 0; i < FRAME_CONTEXTS; i++)
    {
        gen9_hcpd_context->vp9_frame_ctx[i] = gen9_hcpd_context->vp9_fc_inter_default;
    }
    gen9_hcpd_context->vp9_fc = gen9_hcpd_context->vp9_fc_inter_default;

}

static void
vp9_update_probabilities(VADriverContextP ctx,
                          struct decode_state *decode_state,
                          struct gen9_hcpd_context *gen9_hcpd_context)
{
    VADecPictureParameterBufferVP9 *pic_param;
    int i = 0;
    uint8_t is_saved = VP9_PROB_BUFFER_SAVED_NO;
    uint8_t is_restored = VP9_PROB_BUFFER_RESTORED_NO;

    uint8_t last_frame_type = gen9_hcpd_context->last_frame.frame_type;
    uint8_t temp_frame_ctx_id;

    assert(decode_state->pic_param && decode_state->pic_param->buffer);
    pic_param = (VADecPictureParameterBufferVP9 *)decode_state->pic_param->buffer;
    temp_frame_ctx_id = pic_param->pic_fields.bits.frame_context_idx;

    if(pic_param->pic_fields.bits.frame_type == HCP_VP9_KEY_FRAME)
    {
        gen9_hcpd_context->vp9_fc = gen9_hcpd_context->vp9_fc_key_default;
        gen9_hcpd_context->last_frame.prob_buffer_saved_flag = VP9_PROB_BUFFER_SAVED_NO;
        gen9_hcpd_context->last_frame.prob_buffer_restored_flag = VP9_PROB_BUFFER_RESTORED_NO;

    }else
    {
        gen9_hcpd_context->vp9_fc = gen9_hcpd_context->vp9_fc_inter_default;
    }

    // restore?
    if(gen9_hcpd_context->last_frame.prob_buffer_saved_flag == VP9_PROB_BUFFER_SAVED_SECNE_1)
    {
        if((pic_param->pic_fields.bits.frame_type == HCP_VP9_INTER_FRAME))
        {
            //save the inter frame values for the 343 bytes
            //Update the 343 bytes of the buffer with Intra values
            is_restored = VP9_PROB_BUFFER_RESTORED_SECNE_1;
        }
    }else if(gen9_hcpd_context->last_frame.prob_buffer_saved_flag == VP9_PROB_BUFFER_SAVED_SECNE_2)
    {
        if((pic_param->pic_fields.bits.frame_type == HCP_VP9_KEY_FRAME) ||pic_param->pic_fields.bits.intra_only|pic_param->pic_fields.bits.error_resilient_mode)
        {
            temp_frame_ctx_id = 0;
        }

        if((pic_param->pic_fields.bits.frame_type == HCP_VP9_INTER_FRAME) &&
            (temp_frame_ctx_id == 0))
        {

            is_restored = VP9_PROB_BUFFER_RESTORED_SECNE_2;
        }
    }

    if(is_restored > VP9_PROB_BUFFER_RESTORED_NO && is_restored < VP9_PROB_BUFFER_RESTORED_SECNE_MAX)
    {
        memcpy(gen9_hcpd_context->vp9_frame_ctx[gen9_hcpd_context->last_frame.frame_context_idx].inter_mode_probs,gen9_hcpd_context->vp9_saved_fc.inter_mode_probs,VP9_PROB_BUFFER_KEY_INTER_SIZE);
    }

    if((gen9_hcpd_context->last_frame.prob_buffer_restored_flag == VP9_PROB_BUFFER_RESTORED_SECNE_MAX) ||
        (gen9_hcpd_context->last_frame.refresh_frame_context && last_frame_type == HCP_VP9_KEY_FRAME && (pic_param->pic_fields.bits.frame_type == HCP_VP9_INTER_FRAME)))
    {
        if(pic_param->pic_fields.bits.frame_type == HCP_VP9_INTER_FRAME)
        {
            memcpy(gen9_hcpd_context->vp9_frame_ctx[gen9_hcpd_context->last_frame.frame_context_idx].inter_mode_probs,gen9_hcpd_context->vp9_fc_inter_default.inter_mode_probs,VP9_PROB_BUFFER_KEY_INTER_SIZE);
        }
    }
    //first part buffer update: Case 1)Reset all 4 probablity buffers
   if((pic_param->pic_fields.bits.frame_type == HCP_VP9_KEY_FRAME) ||pic_param->pic_fields.bits.intra_only|pic_param->pic_fields.bits.error_resilient_mode)
    {
        if((pic_param->pic_fields.bits.frame_type == HCP_VP9_KEY_FRAME) ||
            (pic_param->pic_fields.bits.reset_frame_context == 3)||
            pic_param->pic_fields.bits.error_resilient_mode)
        {
            //perform full buffer update
            for(i = 0; i < FRAME_CONTEXTS; i++)
            {
                memcpy(&gen9_hcpd_context->vp9_frame_ctx[i],&gen9_hcpd_context->vp9_fc_inter_default,VP9_PROB_BUFFER_FIRST_PART_SIZE);
            }
        }else if(pic_param->pic_fields.bits.reset_frame_context == 2&&pic_param->pic_fields.bits.intra_only)
        {
            memcpy(&gen9_hcpd_context->vp9_frame_ctx[pic_param->pic_fields.bits.frame_context_idx],&gen9_hcpd_context->vp9_fc_inter_default,VP9_PROB_BUFFER_FIRST_PART_SIZE);
        }
        pic_param->pic_fields.bits.frame_context_idx = 0;
    }

    //Full buffer update: Case 2.2)Reset only segmentation prob buffer
    if(pic_param->pic_fields.bits.segmentation_enabled &&
        pic_param->pic_fields.bits.segmentation_update_map)
    {
        for(i = 0; i < FRAME_CONTEXTS; i++)
        {
            //Reset only the segementation probability buffers
            vp9_copy(gen9_hcpd_context->vp9_frame_ctx[i].seg_tree_probs, default_seg_tree_probs);
            vp9_copy(gen9_hcpd_context->vp9_frame_ctx[i].seg_pred_probs, default_seg_pred_probs);
        }
    }

    //update vp9_fc according to frame_context_id
    {
        gen9_hcpd_context->vp9_fc = gen9_hcpd_context->vp9_frame_ctx[pic_param->pic_fields.bits.frame_context_idx];
    }
    //Partial Buffer Update
    //Case 1) Update top 3 probabilities only
    if(pic_param->pic_fields.bits.frame_type == HCP_VP9_KEY_FRAME)
    {
        memcpy(gen9_hcpd_context->vp9_fc.inter_mode_probs,gen9_hcpd_context->vp9_fc_key_default.inter_mode_probs,VP9_PROB_BUFFER_KEY_INTER_SIZE);
        if((!pic_param->pic_fields.bits.segmentation_enabled ||
            !pic_param->pic_fields.bits.segmentation_update_map)) {
            //Update with key frame default probability values for only
            //tx_probs, coef_probs, and the next 343 bytes
            memcpy(&gen9_hcpd_context->vp9_fc,&gen9_hcpd_context->vp9_fc_key_default,VP9_PROB_BUFFER_FIRST_PART_SIZE);
        }
    }
    //Case 2) Update 343 bytes for first inter following key frame
    if( last_frame_type == HCP_VP9_KEY_FRAME &&
        (pic_param->pic_fields.bits.frame_type == HCP_VP9_INTER_FRAME)) {
        //Update with inter frame default values for the 343 bytes
        memcpy(gen9_hcpd_context->vp9_fc.inter_mode_probs,gen9_hcpd_context->vp9_fc_inter_default.inter_mode_probs,VP9_PROB_BUFFER_KEY_INTER_SIZE);
    }
    //Case 2.1) Update 343 bytes for first intra-inly frame following key frame
    if( last_frame_type == HCP_VP9_KEY_FRAME &&
        pic_param->pic_fields.bits.intra_only) {
        //Update with inter frame default values for the 343 bytes
        memcpy(gen9_hcpd_context->vp9_fc.inter_mode_probs,gen9_hcpd_context->vp9_fc_key_default.inter_mode_probs,VP9_PROB_BUFFER_KEY_INTER_SIZE);
    }
    //Case 3) Update only segment probabilities
    if((pic_param->pic_fields.bits.segmentation_enabled &&
        pic_param->pic_fields.bits.segmentation_update_map))
    {
        //Update seg_tree_probs and seg_pred_probs accordingly
        for (i=0; i<SEG_TREE_PROBS; i++)
        {
            gen9_hcpd_context->vp9_fc.seg_tree_probs[i] = pic_param->mb_segment_tree_probs[i];
        }
        for (i=0; i<PREDICTION_PROBS; i++)
        {
            gen9_hcpd_context->vp9_fc.seg_pred_probs[i] = pic_param->segment_pred_probs[i];
        }
    }

    //Case 4) Considering Intra only frame in the middle of Inter frames
    if((pic_param->pic_fields.bits.intra_only &&
        (pic_param->pic_fields.bits.frame_type == HCP_VP9_INTER_FRAME)&&
        (pic_param->pic_fields.bits.reset_frame_context==0  ||
        pic_param->pic_fields.bits.reset_frame_context==1||
        pic_param->pic_fields.bits.reset_frame_context==2||
        pic_param->pic_fields.bits.reset_frame_context==3)) &&
        (last_frame_type == HCP_VP9_INTER_FRAME))
    {
        //save the inter frame values for the 343 bytes
        //Update the 343 bytes of the buffer with Intra values
        is_saved = VP9_PROB_BUFFER_SAVED_SECNE_1;
    }
    //Case 5) Considering Intra only frame among 3 Inter frames
    if((pic_param->pic_fields.bits.intra_only &&
        (pic_param->pic_fields.bits.frame_type == HCP_VP9_INTER_FRAME)&&
        (pic_param->pic_fields.bits.reset_frame_context==2 ||
        pic_param->pic_fields.bits.frame_context_idx==1)) &&
        ((last_frame_type == HCP_VP9_INTER_FRAME) &&
        (gen9_hcpd_context->last_frame.frame_context_idx==0)))
    {
        //save the inter frame values for the 343 bytes
        //Update the 343 bytes of the buffer with Intra values
        is_saved = VP9_PROB_BUFFER_SAVED_SECNE_2;
    }

    if(is_saved > VP9_PROB_BUFFER_SAVED_NO)
    {
        gen9_hcpd_context->vp9_saved_fc = gen9_hcpd_context->vp9_fc;
        memcpy(gen9_hcpd_context->vp9_fc.inter_mode_probs,gen9_hcpd_context->vp9_fc_key_default.inter_mode_probs,VP9_PROB_BUFFER_KEY_INTER_SIZE);
    }else if(pic_param->pic_fields.bits.intra_only)
    {
        is_restored = VP9_PROB_BUFFER_RESTORED_SECNE_MAX;

    }

    // update after the restored
    if(gen9_hcpd_context->last_frame.prob_buffer_restored_flag == VP9_PROB_BUFFER_RESTORED_SECNE_2)
    {
        if((pic_param->pic_fields.bits.frame_type == HCP_VP9_INTER_FRAME) &&
            pic_param->pic_fields.bits.frame_context_idx==1)
        {
            memcpy(&gen9_hcpd_context->vp9_fc,&gen9_hcpd_context->vp9_fc_inter_default,VP9_PROB_BUFFER_FIRST_PART_SIZE);
        }
    }

    {
        dri_bo_map(gen9_hcpd_context->vp9_probability_buffer.bo,1);
        memcpy((unsigned char *)gen9_hcpd_context->vp9_probability_buffer.bo->virtual,&gen9_hcpd_context->vp9_fc,2048);
        dri_bo_unmap(gen9_hcpd_context->vp9_probability_buffer.bo);
    }
    // save the flag in order to restore or update prob buffer
    gen9_hcpd_context->last_frame.prob_buffer_saved_flag = is_saved;
    gen9_hcpd_context->last_frame.prob_buffer_restored_flag = is_restored;
}

static void
gen9_hcpd_init_vp9_surface(VADriverContextP ctx,
                            VADecPictureParameterBufferVP9 *pic_param,
                            struct object_surface *obj_surface,
                            struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    GenVP9Surface *gen9_vp9_surface;
    uint32_t size=0;

    if (!obj_surface)
        return;

    obj_surface->free_private_data = gen_free_vp9_surface;
    gen9_vp9_surface = obj_surface->private_data;

    if (!gen9_vp9_surface) {
        gen9_vp9_surface = calloc(sizeof(GenVP9Surface), 1);
        gen9_vp9_surface->base.frame_store_id = -1;
        obj_surface->private_data = gen9_vp9_surface;
    }

    //Super block size in VP9 is 64x64, size in SBs
    size = gen9_hcpd_context->picture_width_in_ctbs * gen9_hcpd_context->picture_height_in_ctbs * 9 ;
    size<<=6; //CL aligned

    if (gen9_vp9_surface->motion_vector_temporal_bo == NULL) {
        gen9_vp9_surface->motion_vector_temporal_bo = dri_bo_alloc(i965->intel.bufmgr,
                                                                   "current motion vector temporal buffer",
                                                                    size,
                                                                    0x1000);
    }
    gen9_vp9_surface->frame_width  = pic_param->frame_width;
    gen9_vp9_surface->frame_height = pic_param->frame_height;

}

static VAStatus
gen9_hcpd_vp9_decode_init(VADriverContextP ctx,
                           struct decode_state *decode_state,
                           struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VADecPictureParameterBufferVP9 *pic_param;
    struct object_surface *obj_surface;
    uint32_t size;
    int width_in_mbs=0, height_in_mbs=0;

    assert(decode_state->pic_param && decode_state->pic_param->buffer);
    pic_param = (VADecPictureParameterBufferVP9 *)decode_state->pic_param->buffer;

    width_in_mbs = (pic_param->frame_width + 15) / 16;
    height_in_mbs = (pic_param->frame_height + 15) / 16;

    //For BXT, we support only till 4K
    assert(width_in_mbs > 0 && width_in_mbs <= 256); /* 4K */
    assert(height_in_mbs > 0 && height_in_mbs <= 256);

    //Update the frame store buffers with the reference frames information
    intel_update_vp9_frame_store_index(ctx,
                                        decode_state,
                                        pic_param,
                                        gen9_hcpd_context->reference_surfaces);

    /* Current decoded picture */
    obj_surface = decode_state->render_object;
    //Ensure there is a tiled render surface in NV12 format. If not, create one.
    vp9_ensure_surface_bo(ctx, decode_state, obj_surface, pic_param);


    //Super block in VP9 is 64x64
    gen9_hcpd_context->ctb_size = 64;
    gen9_hcpd_context->min_cb_size = 8; //Min block size is 4 or 8?

    //If picture width/height is not multiple of 64, needs to upsize it to the next 64 pixels
    //before calculation below.
    gen9_hcpd_context->picture_width_in_ctbs  = ALIGN(pic_param->frame_width, gen9_hcpd_context->ctb_size) / gen9_hcpd_context->ctb_size;
    gen9_hcpd_context->picture_height_in_ctbs = ALIGN(pic_param->frame_height, gen9_hcpd_context->ctb_size) / gen9_hcpd_context->ctb_size;

    gen9_hcpd_context->picture_width_in_min_cb_minus1  = ALIGN(pic_param->frame_width, gen9_hcpd_context->min_cb_size) / gen9_hcpd_context->min_cb_size - 1;
    gen9_hcpd_context->picture_height_in_min_cb_minus1 = ALIGN(pic_param->frame_height, gen9_hcpd_context->min_cb_size) / gen9_hcpd_context->min_cb_size - 1;

    gen9_hcpd_context->picture_width_in_pixels  = (gen9_hcpd_context->picture_width_in_min_cb_minus1  + 1) * gen9_hcpd_context->min_cb_size ;
    gen9_hcpd_context->picture_height_in_pixels = (gen9_hcpd_context->picture_height_in_min_cb_minus1 + 1) * gen9_hcpd_context->min_cb_size ;

    gen9_hcpd_init_vp9_surface(ctx, pic_param, obj_surface, gen9_hcpd_context);

    size = gen9_hcpd_context->picture_width_in_ctbs*18; //num_width_in_SB * 18
    size<<=6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->deblocking_filter_line_buffer), "line buffer", size);
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->deblocking_filter_tile_line_buffer), "tile line buffer", size);

    size = gen9_hcpd_context->picture_height_in_ctbs*17; //num_height_in_SB * 17
    size<<=6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->deblocking_filter_tile_column_buffer), "tile column buffer", size);

    size = gen9_hcpd_context->picture_width_in_ctbs*5; //num_width_in_SB * 5
    size<<=6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->metadata_line_buffer), "metadata line buffer", size);
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->metadata_tile_line_buffer), "metadata tile line buffer", size);

    size = gen9_hcpd_context->picture_height_in_ctbs*5; //num_height_in_SB * 5
    size<<=6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->metadata_tile_column_buffer), "metadata tile column buffer", size);

    size =gen9_hcpd_context->picture_width_in_ctbs*1; //num_width_in_SB * 1
    size<<=6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->hvd_line_rowstore_buffer), "hvd line rowstore buffer", size);
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->hvd_tile_rowstore_buffer), "hvd tile rowstore buffer", size);

    size = 32;
    size<<=6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->vp9_probability_buffer), "vp9 probability buffer", size);

    size = (gen9_hcpd_context->picture_width_in_min_cb_minus1+2)*(gen9_hcpd_context->picture_height_in_min_cb_minus1 + 2) * 1;
    size<<=6;
    ALLOC_GEN_BUFFER((&gen9_hcpd_context->vp9_segment_id_buffer), "vp9 segment id buffer", size);

    gen9_hcpd_context->first_inter_slice_collocated_ref_idx = 0;
    gen9_hcpd_context->first_inter_slice_collocated_from_l0_flag = 0;
    gen9_hcpd_context->first_inter_slice_valid = 0;

    return VA_STATUS_SUCCESS;
}

static void
gen9_hcpd_vp9_surface_state(VADriverContextP ctx,
                        struct decode_state *decode_state,
                        struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    struct object_surface *obj_surface = decode_state->render_object;
    struct object_surface *tmp_obj_surface = NULL;
    unsigned int y_cb_offset;
    int i = 0;

    assert(obj_surface);

    y_cb_offset = obj_surface->y_cb_offset;

    BEGIN_BCS_BATCH(batch, 3);

    OUT_BCS_BATCH(batch, HCP_SURFACE_STATE | (3 - 2));
    OUT_BCS_BATCH(batch,
                  (0 << 28) |                   /* surface id */
                  (obj_surface->width - 1));    /* pitch - 1 */
    OUT_BCS_BATCH(batch,
                  (SURFACE_FORMAT_PLANAR_420_8 << 28) |
                  y_cb_offset);
    ADVANCE_BCS_BATCH(batch);

    tmp_obj_surface = obj_surface;

    for(i = 0; i < 3; i++)
    {
        obj_surface = gen9_hcpd_context->reference_surfaces[i].obj_surface;
        if (obj_surface && obj_surface->private_data)
        {
            BEGIN_BCS_BATCH(batch, 3);

            OUT_BCS_BATCH(batch, HCP_SURFACE_STATE | (3 - 2));
            OUT_BCS_BATCH(batch,
                ((i + 2) << 28) |                   /* surface id */
                (obj_surface->width - 1));    /* pitch - 1 */
            OUT_BCS_BATCH(batch,
                (SURFACE_FORMAT_PLANAR_420_8 << 28) |
                obj_surface->y_cb_offset);
            ADVANCE_BCS_BATCH(batch);
        }else
        {
            BEGIN_BCS_BATCH(batch, 3);

            OUT_BCS_BATCH(batch, HCP_SURFACE_STATE | (3 - 2));
            OUT_BCS_BATCH(batch,
                ((i + 2) << 28) |                   /* surface id */
                (tmp_obj_surface->width - 1));    /* pitch - 1 */
            OUT_BCS_BATCH(batch,
                (SURFACE_FORMAT_PLANAR_420_8 << 28) |
                tmp_obj_surface->y_cb_offset);
            ADVANCE_BCS_BATCH(batch);
        }
    }
}

static void
gen9_hcpd_vp9_pipe_buf_addr_state(VADriverContextP ctx,
                              struct decode_state *decode_state,
                              struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    struct object_surface *obj_surface;
    GenVP9Surface *gen9_vp9_surface;
    int i=0;

    BEGIN_BCS_BATCH(batch, 95);

    OUT_BCS_BATCH(batch, HCP_PIPE_BUF_ADDR_STATE | (95 - 2));

    obj_surface = decode_state->render_object;
    assert(obj_surface && obj_surface->bo);
    gen9_vp9_surface = obj_surface->private_data;
    assert(gen9_vp9_surface && gen9_vp9_surface->motion_vector_temporal_bo);

    OUT_BUFFER_MA_TARGET(obj_surface->bo); /* DW 1..3 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->deblocking_filter_line_buffer.bo);/* DW 4..6 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->deblocking_filter_tile_line_buffer.bo); /* DW 7..9 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->deblocking_filter_tile_column_buffer.bo); /* DW 10..12 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->metadata_line_buffer.bo);         /* DW 13..15 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->metadata_tile_line_buffer.bo);    /* DW 16..18 */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->metadata_tile_column_buffer.bo);  /* DW 19..21 */
    OUT_BUFFER_MA_TARGET(NULL);    /* DW 22..24, ignore for VP9 */
    OUT_BUFFER_MA_TARGET(NULL);    /* DW 25..27, ignore for VP9 */
    OUT_BUFFER_MA_TARGET(NULL);    /* DW 28..30, ignore for VP9 */
    OUT_BUFFER_MA_TARGET(gen9_vp9_surface->motion_vector_temporal_bo); /* DW 31..33 */

    OUT_BUFFER_MA_TARGET(NULL); /* DW 34..36, reserved */

    /* DW 37..52 - Reference picture address */
    for (i = 0; i < 3; i++)
    {
        obj_surface = gen9_hcpd_context->reference_surfaces[i].obj_surface;

        if (obj_surface)
        {
            OUT_BUFFER_NMA_REFERENCE(obj_surface->bo);
        }
        else
            OUT_BUFFER_NMA_REFERENCE(NULL);
    }
    for (; i < ARRAY_ELEMS(gen9_hcpd_context->reference_surfaces); i++)
    {
       OUT_BUFFER_NMA_REFERENCE(NULL);
    }
    OUT_BCS_BATCH(batch, 0);    /* DW 53, memory address attributes */

    OUT_BUFFER_MA_REFERENCE(NULL); /* DW 54..56, ignore for decoding mode */
    OUT_BUFFER_MA_TARGET(NULL); /* DW 57..59, StreamOutEnable - used for transcoding */
    OUT_BUFFER_MA_TARGET(NULL); /* DW 60..62, DecodedPictureStatusError, ignored */
    OUT_BUFFER_MA_TARGET(NULL); /* DW 63..65, Ignored */

    /* DW 66..81 - for 8 Collocated motion vectors */
    for (i = 0; i < 1; i++)
    {
        OUT_BUFFER_NMA_REFERENCE(gen9_hcpd_context->last_frame.mv_temporal_buffer_bo);
    }
    for (; i < ARRAY_ELEMS(gen9_hcpd_context->reference_surfaces); i++)
    {
        OUT_BUFFER_NMA_REFERENCE(NULL);
    }

    OUT_BCS_BATCH(batch, 0);    /* DW 82, memory address attributes */

    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->vp9_probability_buffer.bo); /* DW 83..85, VP9 Probability bufffer */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->vp9_segment_id_buffer.bo);  /* DW 86..88, VP9 Segment ID buffer */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->hvd_line_rowstore_buffer.bo);/* DW 89..91, VP9 HVD Line Rowstore buffer */
    OUT_BUFFER_MA_TARGET(gen9_hcpd_context->hvd_tile_rowstore_buffer.bo);/* DW 92..94, VP9 HVD Tile Rowstore buffer */

    ADVANCE_BCS_BATCH(batch);
    gen9_hcpd_context->last_frame.mv_temporal_buffer_bo = gen9_vp9_surface->motion_vector_temporal_bo;
}

static inline int
gen9_hcpd_vp9_valid_ref_frame_size(int ref_width, int ref_height,
                                   int cur_width, int cur_height) {
  return 2 * cur_width >= ref_width &&
         2 * cur_height >= ref_height &&
         cur_width <= 16 * ref_width &&
         cur_height <= 16 * ref_height;
}
static void
gen9_hcpd_vp9_pic_state(VADriverContextP ctx,
                       struct decode_state *decode_state,
                       struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    VADecPictureParameterBufferVP9 *pic_param;
    struct object_surface *obj_surface;
    GenVP9Surface *gen9_vp9_surface;
    uint16_t scale_h = 0;
    uint16_t scale_w = 0;
    uint16_t frame_width_in_pixel = 0;
    uint16_t frame_height_in_pixel = 0;
    uint16_t fwidth = 64;
    uint16_t fheight = 64;
    int i;
#define LEN_COMMAND_OWN 12
    assert(decode_state->pic_param && decode_state->pic_param->buffer);
    pic_param = (VADecPictureParameterBufferVP9 *)decode_state->pic_param->buffer;

    uint8_t segmentIDStreamInEnable = 0;
    uint8_t segmentIDStreamOutEnable = (pic_param->pic_fields.bits.segmentation_enabled && pic_param->pic_fields.bits.segmentation_update_map);

    // For KEY_FRAME or INTRA_ONLY frame, this bit should be set to "0".
    uint8_t segmentation_temporal_update =
    ((pic_param->pic_fields.bits.frame_type == HCP_VP9_KEY_FRAME) || (pic_param->pic_fields.bits.intra_only)) ? 0 : pic_param->pic_fields.bits.segmentation_temporal_update;


    if(pic_param->pic_fields.bits.intra_only || (pic_param->pic_fields.bits.frame_type == HCP_VP9_KEY_FRAME))
    {
        segmentIDStreamInEnable = 1;
    }else if(pic_param->pic_fields.bits.segmentation_enabled)
    {
        if(!pic_param->pic_fields.bits.segmentation_update_map)
        {
            segmentIDStreamInEnable = 1;

        }else if( pic_param->pic_fields.bits.segmentation_temporal_update)
        {
            segmentIDStreamInEnable = 1;
        }
    }

    if(pic_param->pic_fields.bits.error_resilient_mode)
    {
        segmentIDStreamInEnable = 1;
    }

    //frame type of previous frame (Key or Non-Key Frame).
    uint8_t last_frame_type = gen9_hcpd_context->last_frame.frame_type;

    uint8_t use_pre_frame_mvs = 0;
    use_pre_frame_mvs = !((pic_param->pic_fields.bits.error_resilient_mode) ||
                                (pic_param->frame_width != gen9_hcpd_context->last_frame.frame_width) ||
                                (pic_param->frame_height != gen9_hcpd_context->last_frame.frame_height) ||
                                (pic_param->pic_fields.bits.intra_only) ||
                                (pic_param->pic_fields.bits.frame_type == HCP_VP9_KEY_FRAME)||
                                (gen9_hcpd_context->last_frame.intra_only)||
                                (last_frame_type == HCP_VP9_KEY_FRAME) ||
                                (!gen9_hcpd_context->last_frame.show_frame));

    uint8_t adapt_probabilities_flag = 0;
    if((pic_param->pic_fields.bits.error_resilient_mode || pic_param->pic_fields.bits.frame_parallel_decoding_mode))
        adapt_probabilities_flag = 0; //0: Do not adapt (error resilient or frame_parallel_mode are set)
    else if(!(pic_param->pic_fields.bits.error_resilient_mode) && !(pic_param->pic_fields.bits.frame_parallel_decoding_mode))
        adapt_probabilities_flag = 1; //1: Adapt (not error resilient and not frame_ parallel_mode)

    frame_width_in_pixel  = (gen9_hcpd_context->picture_width_in_min_cb_minus1  + 1) * gen9_hcpd_context->min_cb_size ;
    frame_height_in_pixel = (gen9_hcpd_context->picture_height_in_min_cb_minus1 + 1) * gen9_hcpd_context->min_cb_size ;

    fwidth = (fwidth > frame_width_in_pixel)?frame_width_in_pixel:fwidth;
    fheight = (fheight > frame_height_in_pixel)?frame_height_in_pixel:fheight;

    BEGIN_BCS_BATCH(batch, LEN_COMMAND_OWN);

    OUT_BCS_BATCH(batch, HCP_VP9_PIC_STATE | (LEN_COMMAND_OWN - 2));

    OUT_BCS_BATCH(batch,
                  (frame_height_in_pixel - 1) << 16 |
                  (frame_width_in_pixel - 1));         /* DW 1 */
    OUT_BCS_BATCH(batch,
                  segmentIDStreamInEnable << 31 |
                  segmentIDStreamOutEnable << 30 |
                  pic_param->pic_fields.bits.lossless_flag << 29 |
                  segmentation_temporal_update << 28 |
                  pic_param->pic_fields.bits.segmentation_update_map << 27 |
                  pic_param->pic_fields.bits.segmentation_enabled << 26   |
                  pic_param->sharpness_level << 23 |
                  pic_param->filter_level << 17 |
                  pic_param->pic_fields.bits.frame_parallel_decoding_mode << 16 |
                  pic_param->pic_fields.bits.error_resilient_mode << 15 |
                  pic_param->pic_fields.bits.refresh_frame_context << 14 |
                  last_frame_type << 13 |
                  0 << 12 |   /* tx select mode */
                  0 << 11 |   /* Hybrid Prediction Mode */
                  use_pre_frame_mvs << 10 |
                  pic_param->pic_fields.bits.alt_ref_frame_sign_bias << 9 |
                  pic_param->pic_fields.bits.golden_ref_frame_sign_bias << 8 |
                  pic_param->pic_fields.bits.last_ref_frame_sign_bias << 7 |
                  pic_param->pic_fields.bits.mcomp_filter_type << 4 |
                  pic_param->pic_fields.bits.allow_high_precision_mv << 3 |
                  pic_param->pic_fields.bits.intra_only <<2 |
                  adapt_probabilities_flag << 1 |
                  pic_param->pic_fields.bits.frame_type <<0);               /* DW 2 */
    OUT_BCS_BATCH(batch,
        HCP_VP9_PROFILE0 << 28 |  /* Profile 0 only supports 8 bit 420 only */
        pic_param->log2_tile_rows << 8 |
        pic_param->log2_tile_columns <<0);                       /* DW 3 */
    // resolution change case

    // DW4-DW6
    for(i = 0; i < 3; i++)
    {
        obj_surface = gen9_hcpd_context->reference_surfaces[i].obj_surface;
        gen9_vp9_surface = NULL;
        scale_w = 0;
        scale_h = 0;
        if (obj_surface && obj_surface->private_data)
        {
            gen9_vp9_surface = obj_surface->private_data;
            if(!gen9_hcpd_vp9_valid_ref_frame_size(gen9_vp9_surface->frame_width,gen9_vp9_surface->frame_height,pic_param->frame_width,pic_param->frame_height))
            {
                scale_w = -1;
                scale_h = -1;
            }else
            {
                scale_w = (gen9_vp9_surface->frame_width  << 14) /pic_param->frame_width ;
                scale_h = (gen9_vp9_surface->frame_height << 14) /pic_param->frame_height ;
            }
            OUT_BCS_BATCH(batch,
                scale_w<<16 |
                scale_h);
        }else
        {
            OUT_BCS_BATCH(batch, 0);
        }
    }

    // DW7-DW9
    for(i = 0; i < 3; i++)
    {
        obj_surface = gen9_hcpd_context->reference_surfaces[i].obj_surface;
        gen9_vp9_surface = NULL;

        if (obj_surface && obj_surface->private_data)
        {
            gen9_vp9_surface = obj_surface->private_data;
            OUT_BCS_BATCH(batch,
                ((gen9_vp9_surface->frame_height- 1)&0x3fff)<<16 |
                ((gen9_vp9_surface->frame_width - 1)&0x3fff));
        }else
        {
            OUT_BCS_BATCH(batch, 0);
        }
    }

    OUT_BCS_BATCH(batch,
                  pic_param->first_partition_size << 16 |
                  pic_param->frame_header_length_in_bytes <<0); /* DW 10 */
    OUT_BCS_BATCH(batch,
                  (0 << 3) |
                  (0 << 2) |
                  (1 << 1) |
                  (0 << 0)); /* DW 11, ignored */
    //Rest of the DWs are not valid for BXT
    for(i = 12; i < LEN_COMMAND_OWN; i++)
    {
        OUT_BCS_BATCH(batch, 0);
    }

    ADVANCE_BCS_BATCH(batch);

}

static void
gen9_hcpd_vp9_segment_state(VADriverContextP ctx,
                            VADecPictureParameterBufferVP9 *pic_param,
                            VASegmentParameterVP9 *seg_param, uint8_t seg_id,
                            struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;

    BEGIN_BCS_BATCH(batch, 7);

    OUT_BCS_BATCH(batch, HCP_VP9_SEGMENT_STATE | (7 - 2));
    OUT_BCS_BATCH(batch, seg_id << 0); /* DW 1 - SegmentID */
    OUT_BCS_BATCH(batch,
                  seg_param->segment_flags.fields.segment_reference_enabled << 3 |
                  seg_param->segment_flags.fields.segment_reference << 1 |
                  seg_param->segment_flags.fields.segment_reference_skipped <<0 ); /* DW 2 */
    OUT_BCS_BATCH(batch,
                  seg_param->filter_level[1][1] << 24    | //FilterLevelRef1Mode1
                  seg_param->filter_level[1][0] << 16    | //FilterLevelRef1Mode0
                  seg_param->filter_level[0][1] << 8     | //FilterLevelRef0Mode1
                  seg_param->filter_level[0][0] << 0 );     //FilterLevelRef0Mode0 /* DW 3 */
    OUT_BCS_BATCH(batch,
                  seg_param->filter_level[3][1] << 24    | //FilterLevelRef3Mode1
                  seg_param->filter_level[3][0] << 16    | //FilterLevelRef3Mode0
                  seg_param->filter_level[2][1] << 8     | //FilterLevelRef2Mode1
                  seg_param->filter_level[2][0] << 0 );    //FilterLevelRef2Mode0 /* DW 4 */
    OUT_BCS_BATCH(batch,
                  seg_param->luma_ac_quant_scale << 16   |
                  seg_param->luma_dc_quant_scale << 0 );    /* DW 5 */
    OUT_BCS_BATCH(batch,
                  seg_param->chroma_ac_quant_scale << 16 |
                  seg_param->chroma_dc_quant_scale << 0 );  /* DW 6 */

    ADVANCE_BCS_BATCH(batch);

}

static void
gen9_hcpd_vp9_bsd_object(VADriverContextP ctx,
                     VADecPictureParameterBufferVP9 *pic_param,
                     VASliceParameterBufferVP9 *slice_param,
                     struct gen9_hcpd_context *gen9_hcpd_context)
{
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    int slice_data_size   = slice_param->slice_data_size - pic_param->frame_header_length_in_bytes;
    int slice_data_offset = slice_param->slice_data_offset + pic_param->frame_header_length_in_bytes;

    BEGIN_BCS_BATCH(batch, 3);

    OUT_BCS_BATCH(batch, HCP_BSD_OBJECT | (3 - 2));

    OUT_BCS_BATCH(batch, slice_data_size );
    OUT_BCS_BATCH(batch, slice_data_offset);

    ADVANCE_BCS_BATCH(batch);

}

static VAStatus
gen9_hcpd_vp9_decode_picture(VADriverContextP ctx,
                              struct decode_state *decode_state,
                              struct gen9_hcpd_context *gen9_hcpd_context)
{
    VAStatus vaStatus;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = gen9_hcpd_context->base.batch;
    VADecPictureParameterBufferVP9 *pic_param;
    VASliceParameterBufferVP9 *slice_param;
    dri_bo *slice_data_bo;
    int i = 0, num_segments=0, isScaling=0;
    struct object_surface *obj_surface;
    GenVP9Surface *gen9_vp9_surface;
    vaStatus = gen9_hcpd_vp9_decode_init(ctx, decode_state, gen9_hcpd_context);

    if (vaStatus != VA_STATUS_SUCCESS)
        goto out;

    assert(decode_state->pic_param && decode_state->pic_param->buffer);
    pic_param = (VADecPictureParameterBufferVP9 *)decode_state->pic_param->buffer;

    //****And set the isScaling value accordingly******
    isScaling = 0;
    for(i = 0; i < 3; i++)
    {
        obj_surface = gen9_hcpd_context->reference_surfaces[i].obj_surface;
        gen9_vp9_surface = NULL;
        if (obj_surface && obj_surface->private_data)
        {
            gen9_vp9_surface = obj_surface->private_data;
            isScaling |= (gen9_vp9_surface->frame_width == pic_param->frame_width &&
                gen9_vp9_surface->frame_height == pic_param->frame_height)? 0:1;
        }
    }

    //Update segment id buffer if needed
    vp9_update_segmentId_buffer(ctx, decode_state, gen9_hcpd_context, isScaling);
    //Update probability buffer if needed
    vp9_update_probabilities(ctx, decode_state, gen9_hcpd_context);

    if (i965->intel.has_bsd2)
        intel_batchbuffer_start_atomic_bcs_override(batch, 0x1000, BSD_RING0);
    else
        intel_batchbuffer_start_atomic_bcs(batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(batch);

    gen9_hcpd_pipe_mode_select(ctx, decode_state, HCP_CODEC_VP9, gen9_hcpd_context);
    //Not sure what the surface id value should be: Gold? ALtRef? PrevRef? or Just RefPic?
    gen9_hcpd_vp9_surface_state(ctx, decode_state, gen9_hcpd_context);

    //Only one VASliceParameterBufferVP9 should be sent per frame
    assert(decode_state->slice_params && decode_state->slice_params[0]->buffer);
    slice_param = (VASliceParameterBufferVP9 *)decode_state->slice_params[0]->buffer;
    slice_data_bo = decode_state->slice_datas[0]->bo;

    gen9_hcpd_ind_obj_base_addr_state(ctx, slice_data_bo, gen9_hcpd_context);

    gen9_hcpd_vp9_pipe_buf_addr_state(ctx, decode_state, gen9_hcpd_context);
    //If segmentation is disabled, only SegParam[0] is valid,
    //all others should be populated with 0
    if(!pic_param->pic_fields.bits.segmentation_enabled)
        num_segments = 1;
    else  //If segmentation is enabled, all 8 entries should be valid.
        num_segments = 8;

    for(i=0; i<num_segments; i++) {
        VASegmentParameterVP9 seg_param = slice_param->seg_param[i];
        gen9_hcpd_vp9_segment_state(ctx, pic_param, &seg_param, i, gen9_hcpd_context);
    }

    gen9_hcpd_vp9_pic_state(ctx, decode_state, gen9_hcpd_context);

    gen9_hcpd_vp9_bsd_object(ctx, pic_param, slice_param, gen9_hcpd_context);

    // keep track of the last frame status
    gen9_hcpd_context->last_frame.frame_width = pic_param->frame_width;
    gen9_hcpd_context->last_frame.frame_height = pic_param->frame_height;
    gen9_hcpd_context->last_frame.show_frame = pic_param->pic_fields.bits.show_frame;
    gen9_hcpd_context->last_frame.frame_type = pic_param->pic_fields.bits.frame_type;
    gen9_hcpd_context->last_frame.refresh_frame_context = pic_param->pic_fields.bits.refresh_frame_context;
    gen9_hcpd_context->last_frame.frame_context_idx = pic_param->pic_fields.bits.frame_context_idx;
    gen9_hcpd_context->last_frame.intra_only = pic_param->pic_fields.bits.intra_only;

    // update prob buffer to vp9_fc;

    intel_batchbuffer_end_atomic(batch);
    intel_batchbuffer_flush(batch);

    //update vp9_frame_ctx according to frame_context_id
    if (pic_param->pic_fields.bits.refresh_frame_context)
    {
        //update vp9_fc to frame_context
        dri_bo_map(gen9_hcpd_context->vp9_probability_buffer.bo,1);
        memcpy(&gen9_hcpd_context->vp9_fc,(unsigned char *)gen9_hcpd_context->vp9_probability_buffer.bo->virtual,2048);
        dri_bo_unmap(gen9_hcpd_context->vp9_probability_buffer.bo);
        gen9_hcpd_context->vp9_frame_ctx[pic_param->pic_fields.bits.frame_context_idx] = gen9_hcpd_context->vp9_fc;

    }

out:
    return vaStatus;
}


static VAStatus
gen9_hcpd_decode_picture(VADriverContextP ctx,
                         VAProfile profile,
                         union codec_state *codec_state,
                         struct hw_context *hw_context)
{
    struct gen9_hcpd_context *gen9_hcpd_context = (struct gen9_hcpd_context *)hw_context;
    struct decode_state *decode_state = &codec_state->decode;
    VAStatus vaStatus;

    assert(gen9_hcpd_context);

    vaStatus = intel_decoder_sanity_check_input(ctx, profile, decode_state);

    if (vaStatus != VA_STATUS_SUCCESS)
        goto out;

    switch (profile) {
    case VAProfileHEVCMain:
    case VAProfileHEVCMain10:
        vaStatus = gen9_hcpd_hevc_decode_picture(ctx, decode_state, gen9_hcpd_context);
        break;
    case VAProfileVP9Profile0:
        vaStatus = gen9_hcpd_vp9_decode_picture(ctx, decode_state, gen9_hcpd_context);
        break;

    default:
        /* should never get here 1!! */
        assert(0);
        break;
    }

out:
    return vaStatus;
}

static void
gen9_hcpd_context_destroy(void *hw_context)
{
    struct gen9_hcpd_context *gen9_hcpd_context = (struct gen9_hcpd_context *)hw_context;

    FREE_GEN_BUFFER((&gen9_hcpd_context->deblocking_filter_line_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->deblocking_filter_tile_line_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->deblocking_filter_tile_column_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->metadata_line_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->metadata_tile_line_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->metadata_tile_column_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->sao_line_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->sao_tile_line_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->sao_tile_column_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->hvd_line_rowstore_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->hvd_tile_rowstore_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->vp9_probability_buffer));
    FREE_GEN_BUFFER((&gen9_hcpd_context->vp9_segment_id_buffer));

    intel_batchbuffer_free(gen9_hcpd_context->base.batch);
    free(gen9_hcpd_context);
}

static void
gen9_hcpd_hevc_context_init(VADriverContextP ctx,
                            struct gen9_hcpd_context *gen9_hcpd_context)
{
    hevc_gen_default_iq_matrix(&gen9_hcpd_context->iq_matrix_hevc);
}

static void
gen9_hcpd_vp9_context_init(VADriverContextP ctx,
                            struct gen9_hcpd_context *gen9_hcpd_context)
{
    int default_value = 255;

    gen9_hcpd_context->last_frame.frame_height  = 0;
    gen9_hcpd_context->last_frame.show_frame    = 0;
    gen9_hcpd_context->last_frame.frame_type    = 0;
    gen9_hcpd_context->last_frame.refresh_frame_context = default_value;
    gen9_hcpd_context->last_frame.frame_context_idx = default_value;
    gen9_hcpd_context->last_frame.intra_only = 0;
    gen9_hcpd_context->last_frame.prob_buffer_saved_flag = 0;
    gen9_hcpd_context->last_frame.prob_buffer_restored_flag = 0;
    gen9_hcpd_context->last_frame.mv_temporal_buffer_bo = NULL;

    vp9_gen_default_probabilities(ctx, gen9_hcpd_context);
}

static struct hw_context *
gen9_hcpd_context_init(VADriverContextP ctx, struct object_config *object_config)
{
    struct intel_driver_data *intel = intel_driver_data(ctx);
    struct gen9_hcpd_context *gen9_hcpd_context = calloc(1, sizeof(struct gen9_hcpd_context));
    int i;

    if (!gen9_hcpd_context)
        return NULL;

    gen9_hcpd_context->base.destroy = gen9_hcpd_context_destroy;
    gen9_hcpd_context->base.run = gen9_hcpd_decode_picture;
    gen9_hcpd_context->base.batch = intel_batchbuffer_new(intel, I915_EXEC_VEBOX, 0);

    for (i = 0; i < ARRAY_ELEMS(gen9_hcpd_context->reference_surfaces); i++) {
        gen9_hcpd_context->reference_surfaces[i].surface_id = VA_INVALID_ID;
        gen9_hcpd_context->reference_surfaces[i].frame_store_id = -1;
        gen9_hcpd_context->reference_surfaces[i].obj_surface = NULL;
    }

    switch (object_config->profile) {
    case VAProfileHEVCMain:
    case VAProfileHEVCMain10:
        gen9_hcpd_hevc_context_init(ctx, gen9_hcpd_context);
        break;
    case VAProfileVP9Profile0:
        gen9_hcpd_vp9_context_init(ctx, gen9_hcpd_context);
        break;

    default:
        break;
    }

    return (struct hw_context *)gen9_hcpd_context;
}

struct hw_context *
gen9_dec_hw_context_init(VADriverContextP ctx, struct object_config *obj_config)
{
    if (obj_config->profile == VAProfileHEVCMain ||
        obj_config->profile == VAProfileHEVCMain10 ||
        obj_config->profile == VAProfileVP9Profile0) {
        return gen9_hcpd_context_init(ctx, obj_config);
    } else {
        return gen8_dec_hw_context_init(ctx, obj_config);
    }
}
