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
 *   Li Xiaowei <xiaowei.a.li@intel.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"
#include "i965_defines.h"
#include "i965_structs.h"

#include "i965_drv_video.h"
#include "i965_post_processing.h"
#include "gen75_picture_process.h"

extern struct hw_context *
i965_proc_context_init(VADriverContextP ctx,
                       struct object_config *obj_config);

static VAStatus 
gen75_vpp_fmt_cvt(VADriverContextP ctx, 
                  VAProfile profile, 
                  union codec_state *codec_state,
                  struct hw_context *hw_context)
{
    VAStatus va_status = VA_STATUS_SUCCESS;
    struct intel_video_process_context *proc_ctx = 
             (struct intel_video_process_context *)hw_context;
  
    /* implicity surface format coversion and scaling */
    if(proc_ctx->vpp_fmt_cvt_ctx == NULL){
         proc_ctx->vpp_fmt_cvt_ctx = i965_proc_context_init(ctx, NULL);
    }

    va_status = i965_proc_picture(ctx, profile, codec_state,
                                  proc_ctx->vpp_fmt_cvt_ctx);

    return va_status;
}

static VAStatus 
gen75_vpp_vebox(VADriverContextP ctx, 
                struct intel_video_process_context* proc_ctx)
{
     VAStatus va_status = VA_STATUS_SUCCESS;
     VAProcPipelineParameterBuffer* pipeline_param = proc_ctx->pipeline_param; 
     struct i965_driver_data *i965 = i965_driver_data(ctx); 
 
     /* vpp features based on VEBox fixed function */
     if(proc_ctx->vpp_vebox_ctx == NULL) {
         proc_ctx->vpp_vebox_ctx = gen75_vebox_context_init(ctx);
     }

     proc_ctx->vpp_vebox_ctx->pipeline_param  = pipeline_param;
     proc_ctx->vpp_vebox_ctx->surface_input_object = proc_ctx->surface_pipeline_input_object;
     proc_ctx->vpp_vebox_ctx->surface_output_object  = proc_ctx->surface_render_output_object;

     if (IS_HASWELL(i965->intel.device_info))
         va_status = gen75_vebox_process_picture(ctx, proc_ctx->vpp_vebox_ctx);
     else if (IS_GEN8(i965->intel.device_info))
         va_status = gen8_vebox_process_picture(ctx, proc_ctx->vpp_vebox_ctx);
     else if (IS_GEN9(i965->intel.device_info))
         va_status = gen9_vebox_process_picture(ctx, proc_ctx->vpp_vebox_ctx);

     return va_status;
} 

static VAStatus 
gen75_vpp_gpe(VADriverContextP ctx, 
              struct intel_video_process_context* proc_ctx)
{
     VAStatus va_status = VA_STATUS_SUCCESS;

     if(proc_ctx->vpp_gpe_ctx == NULL){
         proc_ctx->vpp_gpe_ctx = vpp_gpe_context_init(ctx);
     }
   
     proc_ctx->vpp_gpe_ctx->pipeline_param = proc_ctx->pipeline_param;
     proc_ctx->vpp_gpe_ctx->surface_pipeline_input_object = proc_ctx->surface_pipeline_input_object;
     proc_ctx->vpp_gpe_ctx->surface_output_object = proc_ctx->surface_render_output_object;

     va_status = vpp_gpe_process_picture(ctx, proc_ctx->vpp_gpe_ctx);
 
     return va_status;     
}

VAStatus 
gen75_proc_picture(VADriverContextP ctx,
                   VAProfile profile,
                   union codec_state *codec_state,
                   struct hw_context *hw_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct proc_state* proc_st = &(codec_state->proc);
    struct intel_video_process_context *proc_ctx = 
             (struct intel_video_process_context *)hw_context;
    VAProcPipelineParameterBuffer *pipeline_param = 
             (VAProcPipelineParameterBuffer *)proc_st->pipeline_param->buffer;
    struct object_surface *obj_dst_surf = NULL;
    struct object_surface *obj_src_surf = NULL;
    VAStatus status;

    proc_ctx->pipeline_param = pipeline_param;

    if (proc_st->current_render_target == VA_INVALID_SURFACE ||
        pipeline_param->surface == VA_INVALID_SURFACE) {
        status = VA_STATUS_ERROR_INVALID_SURFACE;
        goto error;
    }

    obj_dst_surf = SURFACE(proc_st->current_render_target);

    if (!obj_dst_surf) {
        status = VA_STATUS_ERROR_INVALID_SURFACE;
        goto error;
    }

    obj_src_surf = SURFACE(proc_ctx->pipeline_param->surface);

    if (!obj_src_surf) {
        status = VA_STATUS_ERROR_INVALID_SURFACE;
        goto error;
    }

    if (!obj_src_surf->bo) {
        status = VA_STATUS_ERROR_INVALID_VALUE; /* The input surface is created without valid content */
        goto error;
    }

    if (pipeline_param->num_filters && !pipeline_param->filters) {
        status = VA_STATUS_ERROR_INVALID_PARAMETER;
        goto error;
    }

    if (!obj_dst_surf->bo) {
        unsigned int is_tiled = 1;
        unsigned int fourcc = VA_FOURCC_NV12;
        int sampling = SUBSAMPLE_YUV420;
        i965_check_alloc_surface_bo(ctx, obj_dst_surf, is_tiled, fourcc, sampling);
    }  

    proc_ctx->surface_render_output_object = obj_dst_surf;
    proc_ctx->surface_pipeline_input_object = obj_src_surf;
    assert(pipeline_param->num_filters <= 4);

    VABufferID *filter_id = (VABufferID*) pipeline_param->filters;
 
    if(pipeline_param->num_filters == 0 || pipeline_param->filters == NULL ){
        /* implicity surface format coversion and scaling */
        gen75_vpp_fmt_cvt(ctx, profile, codec_state, hw_context);
    }else if(pipeline_param->num_filters == 1) {
       struct object_buffer * obj_buf = BUFFER((*filter_id) + 0);

       assert(obj_buf && obj_buf->buffer_store && obj_buf->buffer_store->buffer);
       
       if (!obj_buf ||
           !obj_buf->buffer_store ||
           !obj_buf->buffer_store->buffer) {
           status = VA_STATUS_ERROR_INVALID_FILTER_CHAIN;
           goto error;
       }

       VAProcFilterParameterBuffer* filter =
           (VAProcFilterParameterBuffer*)obj_buf-> buffer_store->buffer;

       if (filter->type == VAProcFilterNoiseReduction         ||
           filter->type == VAProcFilterDeinterlacing          ||
           filter->type == VAProcFilterSkinToneEnhancement    ||
           filter->type == VAProcFilterColorBalance){
           gen75_vpp_vebox(ctx, proc_ctx);
       }else if(filter->type == VAProcFilterSharpening){
           if (obj_src_surf->fourcc != VA_FOURCC_NV12 ||
               obj_dst_surf->fourcc != VA_FOURCC_NV12) {
               status = VA_STATUS_ERROR_UNIMPLEMENTED;
               goto error;
           }

           gen75_vpp_gpe(ctx, proc_ctx);
       } 
    }else if (pipeline_param->num_filters >= 2) {
         unsigned int i = 0;
         for (i = 0; i < pipeline_param->num_filters; i++){
             struct object_buffer * obj_buf = BUFFER(pipeline_param->filters[i]);

             if (!obj_buf ||
                 !obj_buf->buffer_store ||
                 !obj_buf->buffer_store->buffer) {
                 status = VA_STATUS_ERROR_INVALID_FILTER_CHAIN;
                 goto error;
             }

             VAProcFilterParameterBuffer* filter =
                 (VAProcFilterParameterBuffer*)obj_buf-> buffer_store->buffer;

             if (filter->type != VAProcFilterNoiseReduction       &&
                 filter->type != VAProcFilterDeinterlacing        &&
                 filter->type != VAProcFilterSkinToneEnhancement  &&
                 filter->type != VAProcFilterColorBalance) {
                 fprintf(stderr, "Do not support multiply filters outside vebox pipeline \n");
                 assert(0);
             }
         }
         gen75_vpp_vebox(ctx, proc_ctx);
    }     

    return VA_STATUS_SUCCESS;

error:
    return status;
}

static void 
gen75_proc_context_destroy(void *hw_context)
{
    struct intel_video_process_context *proc_ctx =
                      (struct intel_video_process_context *)hw_context;
    VADriverContextP ctx = (VADriverContextP)(proc_ctx->driver_context);

    if(proc_ctx->vpp_fmt_cvt_ctx){
        proc_ctx->vpp_fmt_cvt_ctx->destroy(proc_ctx->vpp_fmt_cvt_ctx);
        proc_ctx->vpp_fmt_cvt_ctx = NULL;
    }

    if(proc_ctx->vpp_vebox_ctx){
       gen75_vebox_context_destroy(ctx,proc_ctx->vpp_vebox_ctx);
       proc_ctx->vpp_vebox_ctx = NULL;
    }

    if(proc_ctx->vpp_gpe_ctx){
       vpp_gpe_context_destroy(ctx,proc_ctx->vpp_gpe_ctx);
       proc_ctx->vpp_gpe_ctx = NULL;
    }

    free(proc_ctx);
}

struct hw_context * 
gen75_proc_context_init(VADriverContextP ctx, 
                        struct object_config *obj_config)
{
   struct intel_video_process_context *proc_context 
           = calloc(1, sizeof(struct intel_video_process_context));

    assert(proc_context);
    proc_context->base.destroy = gen75_proc_context_destroy;
    proc_context->base.run     = gen75_proc_picture;

    proc_context->vpp_vebox_ctx    = NULL;
    proc_context->vpp_gpe_ctx      = NULL;
    proc_context->vpp_fmt_cvt_ctx  = NULL;
 
    proc_context->driver_context = ctx;

    return (struct hw_context *)proc_context;
}

