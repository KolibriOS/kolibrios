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
#ifndef _GEN75_PICTURE_PROCESS_H
#define _GEN75_PICTURE_PROCESS_H

#include <va/va_vpp.h>
#include "i965_drv_video.h"
#include "gen75_vpp_vebox.h"
#include "gen75_vpp_gpe.h"

struct intel_video_process_context
{
    struct hw_context base;
    void* driver_context;
  
    struct intel_vebox_context *vpp_vebox_ctx;
    struct hw_context          *vpp_fmt_cvt_ctx;
    struct vpp_gpe_context     *vpp_gpe_ctx;

    VAProcPipelineParameterBuffer* pipeline_param;

    struct object_surface *surface_render_output_object;
    struct object_surface *surface_pipeline_input_object;
};

struct hw_context *
gen75_proc_context_init(VADriverContextP ctx, struct object_config *obj_config);

#endif

