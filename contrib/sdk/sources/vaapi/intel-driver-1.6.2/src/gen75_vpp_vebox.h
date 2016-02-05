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
#include <va/va_vpp.h>
#include "i965_drv_video.h"

#include "i965_post_processing.h"

#define INPUT_SURFACE  0
#define OUTPUT_SURFACE 1

#define VPP_DNDI_DN        0x00000001
#define VPP_DNDI_DI        0x00000002
#define VPP_DNDI_MASK      0x000000ff
#define VPP_IECP_STD_STE   0x00000100
#define VPP_IECP_ACE       0x00000200
#define VPP_IECP_TCC       0x00000400
#define VPP_IECP_PRO_AMP   0x00000800
#define VPP_IECP_CSC       0x00001000
#define VPP_IECP_AOI       0x00002000
#define VPP_IECP_MASK      0x0000ff00
#define MAX_FILTER_SUM     8

#define PRE_FORMAT_CONVERT      0x01
#define POST_FORMAT_CONVERT     0x02
#define POST_SCALING_CONVERT    0x04
#define POST_COPY_CONVERT       0x08

enum {
    FRAME_IN_CURRENT = 0,
    FRAME_IN_PREVIOUS,
    FRAME_IN_STMM,
    FRAME_OUT_STMM,
    FRAME_OUT_CURRENT_DN,
    FRAME_OUT_CURRENT,
    FRAME_OUT_PREVIOUS,
    FRAME_OUT_STATISTIC,
    FRAME_STORE_COUNT,
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
    struct object_surface *obj_surface;
    VASurfaceID surface_id; /* always relative to the input surface */
    unsigned int is_internal_surface : 1;
    unsigned int is_scratch_surface : 1;
} VEBFrameStore;

typedef struct veb_buffer {
    dri_bo  *bo;
    void *  ptr;
    unsigned char  valid;
} VEBBuffer;

struct intel_vebox_context
{
    struct intel_batchbuffer *batch;

    struct object_surface *surface_input_object;
    struct object_surface *surface_output_object;
    VASurfaceID surface_input_vebox;
    struct object_surface *surface_input_vebox_object;    
    VASurfaceID surface_output_vebox;
    struct object_surface *surface_output_vebox_object;
    VASurfaceID surface_output_scaled;
    struct object_surface *surface_output_scaled_object;

    unsigned int fourcc_input;
    unsigned int fourcc_output;

    int width_input;
    int height_input;
    int width_output;
    int height_output;

    VEBFrameStore frame_store[FRAME_STORE_COUNT];

    VEBBuffer dndi_state_table;
    VEBBuffer iecp_state_table;
    VEBBuffer gamut_state_table;
    VEBBuffer vertex_state_table;

    unsigned int  filters_mask;
    int current_output;
    int current_output_type; /* 0:Both, 1:Previous, 2:Current */

    VAProcPipelineParameterBuffer * pipeline_param;
    void * filter_dn;
    void * filter_di;
    void * filter_iecp_std;
    void * filter_iecp_ace;
    void * filter_iecp_tcc;
    void * filter_iecp_amp;

    unsigned int  filter_iecp_amp_num_elements;
    unsigned char format_convert_flags;

    /* Temporary flags live until the current picture is processed */
    unsigned int is_iecp_enabled        : 1;
    unsigned int is_dn_enabled          : 1;
    unsigned int is_di_enabled          : 1;
    unsigned int is_di_adv_enabled      : 1;
    unsigned int is_first_frame         : 1;
    unsigned int is_second_field        : 1;
};

VAStatus gen75_vebox_process_picture(VADriverContextP ctx,
                         struct intel_vebox_context *proc_ctx);

void gen75_vebox_context_destroy(VADriverContextP ctx, 
                          struct intel_vebox_context *proc_ctx);

struct intel_vebox_context * gen75_vebox_context_init(VADriverContextP ctx);

VAStatus gen8_vebox_process_picture(VADriverContextP ctx,
                         struct intel_vebox_context *proc_ctx);

VAStatus gen9_vebox_process_picture(VADriverContextP ctx,
                         struct intel_vebox_context *proc_ctx);

#endif
