/*
 * Copyright © 2011 Intel Corporation
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

#ifndef _GEN7_MFD_H_
#define _GEN7_MFD_H_

#include <xf86drm.h>
#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>
#include "i965_decoder.h"

#define GEN7_VC1_I_PICTURE              0
#define GEN7_VC1_P_PICTURE              1
#define GEN7_VC1_B_PICTURE              2
#define GEN7_VC1_BI_PICTURE             3
#define GEN7_VC1_SKIPPED_PICTURE        4

#define GEN7_VC1_SIMPLE_PROFILE         0
#define GEN7_VC1_MAIN_PROFILE           1
#define GEN7_VC1_ADVANCED_PROFILE       2
#define GEN7_VC1_RESERVED_PROFILE       3

#define GEN7_JPEG_ROTATION_0            0
#define GEN7_JPEG_ROTATION_90           1
#define GEN7_JPEG_ROTATION_270          2
#define GEN7_JPEG_ROTATION_180          3

#define GEN7_YUV400                     0
#define GEN7_YUV420                     1
#define GEN7_YUV422H_2Y                 2
#define GEN7_YUV444                     3
#define GEN7_YUV411                     4
#define GEN7_YUV422V_2Y                 5
#define GEN7_YUV422H_4Y                 6
#define GEN7_YUV422V_4Y                 7

struct gen7_vc1_surface
{
    dri_bo *dmv;
    int picture_type;
};

struct hw_context;

struct gen7_mfd_context
{
    struct hw_context base;

    union {
        VAIQMatrixBufferMPEG2 mpeg2;
        VAIQMatrixBufferH264  h264;     /* flat scaling lists (default) */
    } iq_matrix;

    GenFrameStoreContext fs_ctx;
    GenFrameStore       reference_surface[MAX_GEN_REFERENCE_FRAMES];
    GenBuffer           post_deblocking_output;
    GenBuffer           pre_deblocking_output;
    GenBuffer           intra_row_store_scratch_buffer;
    GenBuffer           deblocking_filter_row_store_scratch_buffer;
    GenBuffer           bsd_mpc_row_store_scratch_buffer;
    GenBuffer           mpr_row_store_scratch_buffer;
    GenBuffer           bitplane_read_buffer;
    GenBuffer           segmentation_buffer;
    
    VASurfaceID jpeg_wa_surface_id;
    struct object_surface *jpeg_wa_surface_object;
    dri_bo *jpeg_wa_slice_data_bo;

    int                 wa_mpeg2_slice_vertical_position;
};

#endif /* _GEN7_MFD_H_ */
