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

#ifndef GEN9_MFD_H
#define GEN9_MFD_H

#include <xf86drm.h>
#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>
#include "i965_decoder.h"
#include "vp9_probs.h"

struct hw_context;

typedef struct vp9_frame_status
{
    uint16_t frame_width;
    uint16_t frame_height;
    uint8_t frame_type;
    uint8_t show_frame;
    uint8_t refresh_frame_context;
    uint8_t frame_context_idx;
    uint8_t intra_only;
    uint8_t prob_buffer_saved_flag;
    uint8_t prob_buffer_restored_flag;
    dri_bo * mv_temporal_buffer_bo;
}vp9_last_frame_status;

struct gen9_hcpd_context
{
    struct hw_context base;

    GenFrameStoreContext fs_ctx;

    GenFrameStore reference_surfaces[MAX_GEN_HCP_REFERENCE_FRAMES];

    VAIQMatrixBufferHEVC  iq_matrix_hevc;

    uint16_t picture_width_in_pixels;
    uint16_t picture_height_in_pixels;
    uint16_t picture_width_in_ctbs;
    uint16_t picture_height_in_ctbs;
    uint16_t picture_width_in_min_cb_minus1;
    uint16_t picture_height_in_min_cb_minus1;
    uint8_t ctb_size;
    uint8_t min_cb_size;

    GenBuffer deblocking_filter_line_buffer;
    GenBuffer deblocking_filter_tile_line_buffer;
    GenBuffer deblocking_filter_tile_column_buffer;
    GenBuffer metadata_line_buffer;
    GenBuffer metadata_tile_line_buffer;
    GenBuffer metadata_tile_column_buffer;
    GenBuffer sao_line_buffer;
    GenBuffer sao_tile_line_buffer;
    GenBuffer sao_tile_column_buffer;
    GenBuffer hvd_line_rowstore_buffer;
    GenBuffer hvd_tile_rowstore_buffer;
    GenBuffer vp9_probability_buffer;
    GenBuffer vp9_segment_id_buffer;

    unsigned short first_inter_slice_collocated_ref_idx;
    unsigned short first_inter_slice_collocated_from_l0_flag;
    int first_inter_slice_valid;

    vp9_last_frame_status last_frame;
    FRAME_CONTEXT vp9_frame_ctx[FRAME_CONTEXTS];
    FRAME_CONTEXT vp9_fc_inter_default;
    FRAME_CONTEXT vp9_fc_key_default;
    FRAME_CONTEXT vp9_fc;
    FRAME_CONTEXT vp9_saved_fc;
};

#endif /* GEN9_MFD_H */
