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

#ifndef GEN75_VPP_GPE
#define GEN75_VPP_GPE

#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>
#include <va/va_vpp.h>
#include "i965_gpe_utils.h"

#define MAX_SURF_IN_SUM 5

enum VPP_GPE_TYPE{
   VPP_GPE_SHARPENING,
   VPP_GPE_BLENDING,
   VPP_GPE_SCENE_CHANGE_DETECTION,
   VPP_GPE_FILTER_SUM,
};

typedef struct _KernelParameterBase{
   unsigned short pic_width;
   unsigned short pic_height;
}KernelParameterBase;

typedef struct _KernelParameterSharpening{
   KernelParameterBase base;
}KernelParameterSharpening;

typedef struct _ThreadParameterBase{
  unsigned int pic_width;
  unsigned int pic_height;
  unsigned int v_pos;
  unsigned int h_pos;
}ThreadParameterBase;

typedef struct _ThreadParameterSharpenig{
   ThreadParameterBase base;
   unsigned int l_amount;
   unsigned int d_amount;
}ThreadParameterSharpening;

struct vpp_gpe_context{
    struct intel_batchbuffer *batch;
    struct i965_gpe_context gpe_ctx;
    struct i965_buffer_surface vpp_batchbuffer;
    struct i965_buffer_surface vpp_kernel_return;

    VAProcPipelineParameterBuffer *pipeline_param;
    enum VPP_GPE_TYPE filter_type;
    unsigned int sub_shader_index;
    unsigned int sub_shader_sum;
  
    unsigned char * kernel_param;  
    unsigned int kernel_param_size;

    unsigned char * thread_param;  
    unsigned int thread_param_size;
    unsigned int thread_num;

    struct object_surface *surface_pipeline_input_object;
    struct object_surface *surface_output_object;
    VASurfaceID  surface_tmp;
    struct object_surface *surface_tmp_object;
    struct object_surface *surface_input_object[MAX_SURF_IN_SUM];
    unsigned  int forward_surf_sum;
    unsigned  int backward_surf_sum;

    unsigned int in_frame_w;
    unsigned int in_frame_h;
    unsigned int is_first_frame;

    void (*gpe_context_init)(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context);

    void (*gpe_context_destroy)(struct i965_gpe_context *gpe_context);

    void (*gpe_load_kernels)(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct i965_kernel *kernel_list,
                             unsigned int num_kernels);

};

struct vpp_gpe_context *
vpp_gpe_context_init(VADriverContextP ctx);

void 
vpp_gpe_context_destroy(VADriverContextP ctx,
                        struct vpp_gpe_context* vpp_context);

VAStatus
vpp_gpe_process_picture(VADriverContextP ctx,
                        struct vpp_gpe_context * vpp_context);
#endif
