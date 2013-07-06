/*
 * Copyright © 2010 Intel Corporation
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
 *    Li Xiaowei <xiaowei.a.li@intel.com>
 *
 */

#ifndef _GEN75_VPP_VEBOX_H
#define _GEN75_VPP_VEBOX_H

#include <xf86drm.h>
#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>
#include "i965_drv_video.h"

#define INPUT_SURFACE  0
#define OUTPUT_SURFACE 1

#define VPP_DNDI_DN        0x00000001
#define VPP_DNDI_DI        0x00000002
#define VPP_IECP_STD_STE   0x00000100
#define VPP_IECP_ACE       0x00000200
#define VPP_IECP_TCC       0x00000400
#define VPP_IECP_PRO_AMP   0x00000800
#define VPP_IECP_CSC       0x00001000
#define VPP_IECP_AOI       0x00002000
#define MAX_FILTER_SUM     8

enum {
    FRAME_IN_CURRENT = 0,
    FRAME_IN_PREVIOUS,
    FRAME_IN_STMM,
    FRAME_OUT_STMM,
    FRAME_OUT_CURRENT_DN,
    FRAME_OUT_CURRENT,
    FRAME_OUT_PREVIOUS,
    FRAME_OUT_STATISTIC,
    FRAME_STORE_SUM,
};

enum SURFACE_FORMAT{
    YCRCB_NORMAL = 0,
    YCRCB_SWAPUVY,
    YCRCB_SWAPUV,
    YCRCB_SWAPY,
    PLANAR_420_8,  //NV12
    PACKED_444A_8,
    PACKED_422_16,
    R10G10B10A2_UNORM_SRGB,
    R8G8B8A8_UNORM_SRGB,
    PACKED_444_16,
    PLANAR_422_16,
    Y8_UNORM,
    PLANAR_420_16,
    R16G16B16A16,
    SURFACE_FORMAT_SUM
};

typedef struct veb_frame_store {
    VASurfaceID surface_id;
    dri_bo  *bo;
    unsigned char  is_internal_surface;
} VEBFrameStore;

typedef struct veb_buffer {
    dri_bo  *bo;
    void *  ptr;
    unsigned char  valid;
} VEBBuffer;

struct intel_vebox_context
{
    struct intel_batchbuffer *batch;

    VASurfaceID surface_input;
    VASurfaceID surface_output;
    unsigned int fourcc_input;
    unsigned int fourcc_output;
    unsigned int pic_width;
    unsigned int pic_height;
 
    VEBFrameStore frame_store[FRAME_STORE_SUM];

    VEBBuffer dndi_state_table;
    VEBBuffer iecp_state_table;
    VEBBuffer gamut_state_table;
    VEBBuffer vertex_state_table;

    unsigned int  filters_mask;
    unsigned char is_first_frame;

    /*
    VAProcPipelineParameterBuffer * pipeline_param;
    void * filter_dn;
    void * filter_di;
    void * filter_iecp_std;
    void * filter_iecp_ace;
    void * filter_iecp_tcc;
    void * filter_iecp_amp;
    void * filter_iecp_csc;
    */
};

VAStatus gen75_vebox_process_picture(VADriverContextP ctx,
                         struct intel_vebox_context *proc_ctx);

void gen75_vebox_context_destroy(VADriverContextP ctx, 
                          struct intel_vebox_context *proc_ctx);

struct intel_vebox_context * gen75_vebox_context_init(VADriverContextP ctx);

#endif
