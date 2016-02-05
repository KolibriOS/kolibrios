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
#include <string.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"

#include "i965_structs.h"
#include "i965_defines.h"
#include "i965_drv_video.h"
#include "gen75_vpp_gpe.h"

#define MAX_INTERFACE_DESC_GEN6      MAX_GPE_KERNELS
#define MAX_MEDIA_SURFACES_GEN6      34

#define SURFACE_STATE_OFFSET_GEN7(index)   (SURFACE_STATE_PADDED_SIZE_GEN7 * (index))
#define BINDING_TABLE_OFFSET_GEN7(index)   (SURFACE_STATE_OFFSET_GEN7(MAX_MEDIA_SURFACES_GEN6) + sizeof(unsigned int) * (index))

#define SURFACE_STATE_OFFSET_GEN8(index)   (SURFACE_STATE_PADDED_SIZE_GEN8 * (index))
#define BINDING_TABLE_OFFSET_GEN8(index)   (SURFACE_STATE_OFFSET_GEN8(MAX_MEDIA_SURFACES_GEN6) + sizeof(unsigned int) * (index))

#define CURBE_ALLOCATION_SIZE   37              
#define CURBE_TOTAL_DATA_LENGTH (4 * 32)        
#define CURBE_URB_ENTRY_LENGTH  4               

/* Shaders information for sharpening */
static const unsigned int gen75_gpe_sharpening_h_blur[][4] = {
   #include "shaders/post_processing/gen75/sharpening_h_blur.g75b"
};
static const unsigned int gen75_gpe_sharpening_v_blur[][4] = {
   #include "shaders/post_processing/gen75/sharpening_v_blur.g75b"
};
static const unsigned int gen75_gpe_sharpening_unmask[][4] = {
   #include "shaders/post_processing/gen75/sharpening_unmask.g75b"
};
static struct i965_kernel gen75_vpp_sharpening_kernels[] = {
    {
        "vpp: sharpening(horizontal blur)",
        VPP_GPE_SHARPENING,
        gen75_gpe_sharpening_h_blur, 			
        sizeof(gen75_gpe_sharpening_h_blur),		
        NULL
    },
    {
        "vpp: sharpening(vertical blur)",
        VPP_GPE_SHARPENING,
        gen75_gpe_sharpening_v_blur, 			
        sizeof(gen75_gpe_sharpening_v_blur),		
        NULL
    },
    {
        "vpp: sharpening(unmask)",
        VPP_GPE_SHARPENING,
        gen75_gpe_sharpening_unmask, 			
        sizeof(gen75_gpe_sharpening_unmask),		
        NULL
    },
}; 

/* sharpening kernels for Broadwell */
static const unsigned int gen8_gpe_sharpening_h_blur[][4] = {
   #include "shaders/post_processing/gen8/sharpening_h_blur.g8b"
};
static const unsigned int gen8_gpe_sharpening_v_blur[][4] = {
   #include "shaders/post_processing/gen8/sharpening_v_blur.g8b"
};
static const unsigned int gen8_gpe_sharpening_unmask[][4] = {
   #include "shaders/post_processing/gen8/sharpening_unmask.g8b"
};

static struct i965_kernel gen8_vpp_sharpening_kernels[] = {
    {
        "vpp: sharpening(horizontal blur)",
        VPP_GPE_SHARPENING,
        gen8_gpe_sharpening_h_blur,
        sizeof(gen8_gpe_sharpening_h_blur),
        NULL
    },
    {
        "vpp: sharpening(vertical blur)",
        VPP_GPE_SHARPENING,
        gen8_gpe_sharpening_v_blur,
        sizeof(gen8_gpe_sharpening_v_blur),
        NULL
    },
    {
        "vpp: sharpening(unmask)",
        VPP_GPE_SHARPENING,
        gen8_gpe_sharpening_unmask,
        sizeof(gen8_gpe_sharpening_unmask),
        NULL
    },
};

static VAStatus
gen75_gpe_process_surfaces_setup(VADriverContextP ctx,
                   struct vpp_gpe_context *vpp_gpe_ctx)
{
    struct object_surface *obj_surface;
    unsigned int i = 0;
    unsigned char input_surface_sum = (1 + vpp_gpe_ctx->forward_surf_sum +
                                         vpp_gpe_ctx->backward_surf_sum) * 2;

    /* Binding input NV12 surfaces (Luma + Chroma)*/
    for( i = 0; i < input_surface_sum; i += 2){ 
         obj_surface = vpp_gpe_ctx->surface_input_object[i/2];
         assert(obj_surface);
         gen7_gpe_media_rw_surface_setup(ctx,
                                         &vpp_gpe_ctx->gpe_ctx,
                                          obj_surface,
                                          BINDING_TABLE_OFFSET_GEN7(i),
                                          SURFACE_STATE_OFFSET_GEN7(i));

         gen75_gpe_media_chroma_surface_setup(ctx,
                                          &vpp_gpe_ctx->gpe_ctx,
                                          obj_surface,
                                          BINDING_TABLE_OFFSET_GEN7(i + 1),
                                          SURFACE_STATE_OFFSET_GEN7(i + 1));
    }

    /* Binding output NV12 surface(Luma + Chroma) */
    obj_surface = vpp_gpe_ctx->surface_output_object;
    assert(obj_surface);
    gen7_gpe_media_rw_surface_setup(ctx,
                                    &vpp_gpe_ctx->gpe_ctx,
                                    obj_surface,
                                    BINDING_TABLE_OFFSET_GEN7(input_surface_sum),
                                    SURFACE_STATE_OFFSET_GEN7(input_surface_sum));
    gen75_gpe_media_chroma_surface_setup(ctx,
                                    &vpp_gpe_ctx->gpe_ctx,
                                    obj_surface,
                                    BINDING_TABLE_OFFSET_GEN7(input_surface_sum + 1),
                                    SURFACE_STATE_OFFSET_GEN7(input_surface_sum + 1));
    /* Bind kernel return buffer surface */
    gen7_gpe_buffer_suface_setup(ctx,
                                  &vpp_gpe_ctx->gpe_ctx,
                                  &vpp_gpe_ctx->vpp_kernel_return,
                                  BINDING_TABLE_OFFSET_GEN7((input_surface_sum + 2)),
                                  SURFACE_STATE_OFFSET_GEN7(input_surface_sum + 2));

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen75_gpe_process_interface_setup(VADriverContextP ctx,
                    struct vpp_gpe_context *vpp_gpe_ctx)
{
    struct gen6_interface_descriptor_data *desc;   
    dri_bo *bo = vpp_gpe_ctx->gpe_ctx.idrt.bo;
    int i; 

    dri_bo_map(bo, 1);
    assert(bo->virtual);
    desc = bo->virtual;
    
    /*Setup the descritor table*/
    for(i = 0; i < vpp_gpe_ctx->sub_shader_sum; i++){
        struct i965_kernel *kernel = &vpp_gpe_ctx->gpe_ctx.kernels[i];
        assert(sizeof(*desc) == 32);
        memset(desc, 0, sizeof(*desc));
        desc->desc0.kernel_start_pointer = (kernel->bo->offset >> 6);
        desc->desc2.sampler_count = 0; /* FIXME: */
        desc->desc2.sampler_state_pointer = 0;
        desc->desc3.binding_table_entry_count = 6; /* FIXME: */
        desc->desc3.binding_table_pointer = (BINDING_TABLE_OFFSET_GEN7(0) >> 5);
        desc->desc4.constant_urb_entry_read_offset = 0;
        desc->desc4.constant_urb_entry_read_length = 0;

        dri_bo_emit_reloc(bo,	
                          I915_GEM_DOMAIN_INSTRUCTION, 0,
                          0,
                          i* sizeof(*desc) + offsetof(struct gen6_interface_descriptor_data, desc0),
                          kernel->bo);
        desc++;
    }

    dri_bo_unmap(bo);

    return VA_STATUS_SUCCESS;
}

static VAStatus 
gen75_gpe_process_parameters_fill(VADriverContextP ctx,
                           struct vpp_gpe_context *vpp_gpe_ctx)
{
    unsigned int *command_ptr;
    unsigned int i, size = vpp_gpe_ctx->thread_param_size;
    unsigned char* position = NULL;

    /* Thread inline data setting*/
    dri_bo_map(vpp_gpe_ctx->vpp_batchbuffer.bo, 1);
    command_ptr = vpp_gpe_ctx->vpp_batchbuffer.bo->virtual;

    for(i = 0; i < vpp_gpe_ctx->thread_num; i ++)
    {
         *command_ptr++ = (CMD_MEDIA_OBJECT | (size/sizeof(int) + 6 - 2));
         *command_ptr++ = vpp_gpe_ctx->sub_shader_index;
         *command_ptr++ = 0;
         *command_ptr++ = 0;
         *command_ptr++ = 0;
         *command_ptr++ = 0;
   
         /* copy thread inline data */
         position =(unsigned char*)(vpp_gpe_ctx->thread_param + size * i);
         memcpy(command_ptr, position, size);
         command_ptr += size/sizeof(int);
    }   

    *command_ptr++ = 0;
    *command_ptr++ = MI_BATCH_BUFFER_END;

    dri_bo_unmap(vpp_gpe_ctx->vpp_batchbuffer.bo);

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen75_gpe_process_pipeline_setup(VADriverContextP ctx,
                   struct vpp_gpe_context *vpp_gpe_ctx)
{
    intel_batchbuffer_start_atomic(vpp_gpe_ctx->batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(vpp_gpe_ctx->batch);

    gen6_gpe_pipeline_setup(ctx, &vpp_gpe_ctx->gpe_ctx, vpp_gpe_ctx->batch);
 
    gen75_gpe_process_parameters_fill(ctx, vpp_gpe_ctx);
   
    BEGIN_BATCH(vpp_gpe_ctx->batch, 2);
    OUT_BATCH(vpp_gpe_ctx->batch, MI_BATCH_BUFFER_START | (1 << 8));
    OUT_RELOC(vpp_gpe_ctx->batch,
              vpp_gpe_ctx->vpp_batchbuffer.bo,
              I915_GEM_DOMAIN_COMMAND, 0, 
              0);
    ADVANCE_BATCH(vpp_gpe_ctx->batch);

    intel_batchbuffer_end_atomic(vpp_gpe_ctx->batch);
	
    return VA_STATUS_SUCCESS;
}

static VAStatus
gen75_gpe_process_init(VADriverContextP ctx,
                 struct vpp_gpe_context *vpp_gpe_ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    dri_bo *bo;

    unsigned int batch_buf_size = vpp_gpe_ctx->thread_num * 
                 (vpp_gpe_ctx->thread_param_size + 6 * sizeof(int)) + 16;

    vpp_gpe_ctx->vpp_kernel_return.num_blocks = vpp_gpe_ctx->thread_num;
    vpp_gpe_ctx->vpp_kernel_return.size_block = 16;
    vpp_gpe_ctx->vpp_kernel_return.pitch = 1;
    unsigned int kernel_return_size =  vpp_gpe_ctx->vpp_kernel_return.num_blocks   
           * vpp_gpe_ctx->vpp_kernel_return.size_block;
 
    dri_bo_unreference(vpp_gpe_ctx->vpp_batchbuffer.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vpp batch buffer",
                       batch_buf_size, 0x1000);
    vpp_gpe_ctx->vpp_batchbuffer.bo = bo;

    dri_bo_unreference(vpp_gpe_ctx->vpp_kernel_return.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vpp kernel return buffer",
                       kernel_return_size, 0x1000);
    vpp_gpe_ctx->vpp_kernel_return.bo = bo;

    vpp_gpe_ctx->gpe_context_init(ctx, &vpp_gpe_ctx->gpe_ctx);

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen75_gpe_process_prepare(VADriverContextP ctx,
                    struct vpp_gpe_context *vpp_gpe_ctx)
{
    /*Setup all the memory object*/
    gen75_gpe_process_surfaces_setup(ctx, vpp_gpe_ctx);
    gen75_gpe_process_interface_setup(ctx, vpp_gpe_ctx);
    //gen75_gpe_process_constant_setup(ctx, vpp_gpe_ctx);

    /*Programing media pipeline*/
    gen75_gpe_process_pipeline_setup(ctx, vpp_gpe_ctx);
	
    return VA_STATUS_SUCCESS;
}

static VAStatus
gen75_gpe_process_run(VADriverContextP ctx,
                struct vpp_gpe_context *vpp_gpe_ctx)
{
    intel_batchbuffer_flush(vpp_gpe_ctx->batch);
    
    return VA_STATUS_SUCCESS;
}

static VAStatus
gen75_gpe_process(VADriverContextP ctx,
                  struct vpp_gpe_context * vpp_gpe_ctx)
{
    VAStatus va_status = VA_STATUS_SUCCESS;

    va_status = gen75_gpe_process_init(ctx, vpp_gpe_ctx);
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    va_status = gen75_gpe_process_prepare(ctx, vpp_gpe_ctx);
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    va_status = gen75_gpe_process_run(ctx, vpp_gpe_ctx);
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen8_gpe_process_surfaces_setup(VADriverContextP ctx,
                   struct vpp_gpe_context *vpp_gpe_ctx)
{
    struct object_surface *obj_surface;
    unsigned int i = 0;
    unsigned char input_surface_sum = (1 + vpp_gpe_ctx->forward_surf_sum +
                                         vpp_gpe_ctx->backward_surf_sum) * 2;

    /* Binding input NV12 surfaces (Luma + Chroma)*/
    for( i = 0; i < input_surface_sum; i += 2){
         obj_surface = vpp_gpe_ctx->surface_input_object[i/2];
         assert(obj_surface);
         gen8_gpe_media_rw_surface_setup(ctx,
                                         &vpp_gpe_ctx->gpe_ctx,
                                          obj_surface,
                                          BINDING_TABLE_OFFSET_GEN8(i),
                                          SURFACE_STATE_OFFSET_GEN8(i));

         gen8_gpe_media_chroma_surface_setup(ctx,
                                          &vpp_gpe_ctx->gpe_ctx,
                                          obj_surface,
                                          BINDING_TABLE_OFFSET_GEN8(i + 1),
                                          SURFACE_STATE_OFFSET_GEN8(i + 1));
    }

    /* Binding output NV12 surface(Luma + Chroma) */
    obj_surface = vpp_gpe_ctx->surface_output_object;
    assert(obj_surface);
    gen8_gpe_media_rw_surface_setup(ctx,
                                    &vpp_gpe_ctx->gpe_ctx,
                                    obj_surface,
                                    BINDING_TABLE_OFFSET_GEN8(input_surface_sum),
                                    SURFACE_STATE_OFFSET_GEN8(input_surface_sum));
    gen8_gpe_media_chroma_surface_setup(ctx,
                                    &vpp_gpe_ctx->gpe_ctx,
                                    obj_surface,
                                    BINDING_TABLE_OFFSET_GEN8(input_surface_sum + 1),
                                    SURFACE_STATE_OFFSET_GEN8(input_surface_sum + 1));
    /* Bind kernel return buffer surface */
    gen7_gpe_buffer_suface_setup(ctx,
                                  &vpp_gpe_ctx->gpe_ctx,
                                  &vpp_gpe_ctx->vpp_kernel_return,
                                  BINDING_TABLE_OFFSET_GEN8((input_surface_sum + 2)),
                                  SURFACE_STATE_OFFSET_GEN8(input_surface_sum + 2));

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen8_gpe_process_interface_setup(VADriverContextP ctx,
                    struct vpp_gpe_context *vpp_gpe_ctx)
{
    struct gen8_interface_descriptor_data *desc;
    dri_bo *bo = vpp_gpe_ctx->gpe_ctx.dynamic_state.bo;
    int i;

    dri_bo_map(bo, 1);
    assert(bo->virtual);
    desc = (struct gen8_interface_descriptor_data *)(bo->virtual
                               + vpp_gpe_ctx->gpe_ctx.idrt_offset);

    /*Setup the descritor table*/
    for (i = 0; i < vpp_gpe_ctx->sub_shader_sum; i++){
        struct i965_kernel *kernel;
        kernel = &vpp_gpe_ctx->gpe_ctx.kernels[i];
        assert(sizeof(*desc) == 32);
        /*Setup the descritor table*/
         memset(desc, 0, sizeof(*desc));
         desc->desc0.kernel_start_pointer = kernel->kernel_offset >> 6;
         desc->desc3.sampler_count = 0; /* FIXME: */
         desc->desc3.sampler_state_pointer = 0;
         desc->desc4.binding_table_entry_count = 6; /* FIXME: */
         desc->desc4.binding_table_pointer = (BINDING_TABLE_OFFSET_GEN8(0) >> 5);
         desc->desc5.constant_urb_entry_read_offset = 0;
         desc->desc5.constant_urb_entry_read_length = 0;

         desc++;
    }

    dri_bo_unmap(bo);

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen8_gpe_process_parameters_fill(VADriverContextP ctx,
                           struct vpp_gpe_context *vpp_gpe_ctx)
{
    unsigned int *command_ptr;
    unsigned int i, size = vpp_gpe_ctx->thread_param_size;
    unsigned char* position = NULL;

    /* Thread inline data setting*/
    dri_bo_map(vpp_gpe_ctx->vpp_batchbuffer.bo, 1);
    command_ptr = vpp_gpe_ctx->vpp_batchbuffer.bo->virtual;

    for(i = 0; i < vpp_gpe_ctx->thread_num; i ++)
    {
         *command_ptr++ = (CMD_MEDIA_OBJECT | (size/sizeof(int) + 6 - 2));
         *command_ptr++ = vpp_gpe_ctx->sub_shader_index;
         *command_ptr++ = 0;
         *command_ptr++ = 0;
         *command_ptr++ = 0;
         *command_ptr++ = 0;

         /* copy thread inline data */
         position =(unsigned char*)(vpp_gpe_ctx->thread_param + size * i);
         memcpy(command_ptr, position, size);
         command_ptr += size/sizeof(int);

         *command_ptr++ = CMD_MEDIA_STATE_FLUSH;
         *command_ptr++ = 0;
    }

    *command_ptr++ = 0;
    *command_ptr++ = MI_BATCH_BUFFER_END;

    dri_bo_unmap(vpp_gpe_ctx->vpp_batchbuffer.bo);

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen8_gpe_process_pipeline_setup(VADriverContextP ctx,
                   struct vpp_gpe_context *vpp_gpe_ctx)
{
    intel_batchbuffer_start_atomic(vpp_gpe_ctx->batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(vpp_gpe_ctx->batch);

    gen8_gpe_pipeline_setup(ctx, &vpp_gpe_ctx->gpe_ctx, vpp_gpe_ctx->batch);

    gen8_gpe_process_parameters_fill(ctx, vpp_gpe_ctx);

    BEGIN_BATCH(vpp_gpe_ctx->batch, 3);
    OUT_BATCH(vpp_gpe_ctx->batch, MI_BATCH_BUFFER_START | (1 << 8) | (1 << 0));
    OUT_RELOC(vpp_gpe_ctx->batch,
              vpp_gpe_ctx->vpp_batchbuffer.bo,
              I915_GEM_DOMAIN_COMMAND, 0,
              0);
    OUT_BATCH(vpp_gpe_ctx->batch, 0);

    ADVANCE_BATCH(vpp_gpe_ctx->batch);

    intel_batchbuffer_end_atomic(vpp_gpe_ctx->batch);

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen8_gpe_process_init(VADriverContextP ctx,
                 struct vpp_gpe_context *vpp_gpe_ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    dri_bo *bo;

    unsigned int batch_buf_size = vpp_gpe_ctx->thread_num *
                 (vpp_gpe_ctx->thread_param_size + 6 * sizeof(int)) + 16;

    vpp_gpe_ctx->vpp_kernel_return.num_blocks = vpp_gpe_ctx->thread_num;
    vpp_gpe_ctx->vpp_kernel_return.size_block = 16;
    vpp_gpe_ctx->vpp_kernel_return.pitch = 1;

    unsigned int kernel_return_size =  vpp_gpe_ctx->vpp_kernel_return.num_blocks
           * vpp_gpe_ctx->vpp_kernel_return.size_block;

    dri_bo_unreference(vpp_gpe_ctx->vpp_batchbuffer.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vpp batch buffer",
                       batch_buf_size, 0x1000);
    vpp_gpe_ctx->vpp_batchbuffer.bo = bo;

    dri_bo_unreference(vpp_gpe_ctx->vpp_kernel_return.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vpp kernel return buffer",
                       kernel_return_size, 0x1000);
    vpp_gpe_ctx->vpp_kernel_return.bo = bo;

    vpp_gpe_ctx->gpe_context_init(ctx, &vpp_gpe_ctx->gpe_ctx);

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen8_gpe_process_prepare(VADriverContextP ctx,
                    struct vpp_gpe_context *vpp_gpe_ctx)
{
    /*Setup all the memory object*/
    gen8_gpe_process_surfaces_setup(ctx, vpp_gpe_ctx);
    gen8_gpe_process_interface_setup(ctx, vpp_gpe_ctx);
    //gen8_gpe_process_constant_setup(ctx, vpp_gpe_ctx);

    /*Programing media pipeline*/
    gen8_gpe_process_pipeline_setup(ctx, vpp_gpe_ctx);

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen8_gpe_process_run(VADriverContextP ctx,
                struct vpp_gpe_context *vpp_gpe_ctx)
{
    intel_batchbuffer_flush(vpp_gpe_ctx->batch);

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen8_gpe_process(VADriverContextP ctx,
                  struct vpp_gpe_context * vpp_gpe_ctx)
{
    VAStatus va_status = VA_STATUS_SUCCESS;

    va_status = gen8_gpe_process_init(ctx, vpp_gpe_ctx);
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    va_status = gen8_gpe_process_prepare(ctx, vpp_gpe_ctx);
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    va_status = gen8_gpe_process_run(ctx, vpp_gpe_ctx);
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    return VA_STATUS_SUCCESS;
}

static VAStatus
vpp_gpe_process(VADriverContextP ctx,
                  struct vpp_gpe_context * vpp_gpe_ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    if (IS_HASWELL(i965->intel.device_info))
       return gen75_gpe_process(ctx, vpp_gpe_ctx);
    else if (IS_GEN8(i965->intel.device_info) ||
             IS_GEN9(i965->intel.device_info))
       return gen8_gpe_process(ctx, vpp_gpe_ctx);

     return VA_STATUS_ERROR_UNIMPLEMENTED;
}

static VAStatus
vpp_gpe_process_sharpening(VADriverContextP ctx,
                             struct vpp_gpe_context * vpp_gpe_ctx)
{
     VAStatus va_status = VA_STATUS_SUCCESS;
     struct i965_driver_data *i965 = i965_driver_data(ctx);
     struct object_surface *origin_in_obj_surface = vpp_gpe_ctx->surface_input_object[0];
     struct object_surface *origin_out_obj_surface = vpp_gpe_ctx->surface_output_object;

     VAProcPipelineParameterBuffer* pipe = vpp_gpe_ctx->pipeline_param;
     VABufferID *filter_ids = (VABufferID*)pipe->filters ;
     struct object_buffer *obj_buf = BUFFER((*(filter_ids + 0)));

     assert(obj_buf && obj_buf->buffer_store && obj_buf->buffer_store->buffer);
       
     if (!obj_buf ||
         !obj_buf->buffer_store ||
         !obj_buf->buffer_store->buffer)
         goto error;

     VAProcFilterParameterBuffer* filter =
                  (VAProcFilterParameterBuffer*)obj_buf-> buffer_store->buffer;
     float sharpening_intensity = filter->value;

     ThreadParameterSharpening thr_param;
     unsigned int thr_param_size = sizeof(ThreadParameterSharpening);
     unsigned int i;
     unsigned char * pos;

     if(vpp_gpe_ctx->is_first_frame){
         vpp_gpe_ctx->sub_shader_sum = 3;
         struct i965_kernel * vpp_kernels;
         if (IS_HASWELL(i965->intel.device_info))
             vpp_kernels = gen75_vpp_sharpening_kernels;
         else if (IS_GEN8(i965->intel.device_info) ||
                  IS_GEN9(i965->intel.device_info)) // TODO: build the sharpening kernel for GEN9
             vpp_kernels = gen8_vpp_sharpening_kernels;
         else
             return VA_STATUS_ERROR_UNIMPLEMENTED;

         vpp_gpe_ctx->gpe_load_kernels(ctx,
                               &vpp_gpe_ctx->gpe_ctx,
                               vpp_kernels,
                               vpp_gpe_ctx->sub_shader_sum);
     }

     if(vpp_gpe_ctx->surface_tmp == VA_INVALID_ID){
        va_status = i965_CreateSurfaces(ctx,
                                       vpp_gpe_ctx->in_frame_w,
                                       vpp_gpe_ctx->in_frame_h,
                                       VA_RT_FORMAT_YUV420,
                                       1,
                                       &vpp_gpe_ctx->surface_tmp);
       assert(va_status == VA_STATUS_SUCCESS);
    
       struct object_surface * obj_surf = SURFACE(vpp_gpe_ctx->surface_tmp);
       assert(obj_surf);

       if (obj_surf) {
           i965_check_alloc_surface_bo(ctx, obj_surf, 1, VA_FOURCC_NV12,
                                       SUBSAMPLE_YUV420);
           vpp_gpe_ctx->surface_tmp_object = obj_surf;
       }
    }                

    assert(sharpening_intensity >= 0.0 && sharpening_intensity <= 1.0);
    thr_param.l_amount = (unsigned int)(sharpening_intensity * 128);
    thr_param.d_amount = (unsigned int)(sharpening_intensity * 128);

    thr_param.base.pic_width = vpp_gpe_ctx->in_frame_w;
    thr_param.base.pic_height = vpp_gpe_ctx->in_frame_h;

    /* Step 1: horizontal blur process */      
    vpp_gpe_ctx->forward_surf_sum = 0;
    vpp_gpe_ctx->backward_surf_sum = 0;
 
    vpp_gpe_ctx->thread_num = vpp_gpe_ctx->in_frame_h/16;
    vpp_gpe_ctx->thread_param_size = thr_param_size;
    vpp_gpe_ctx->thread_param = (unsigned char*) malloc(vpp_gpe_ctx->thread_param_size
                                                       *vpp_gpe_ctx->thread_num);
    pos = vpp_gpe_ctx->thread_param;

    if (!pos) {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    for( i = 0 ; i < vpp_gpe_ctx->thread_num; i++){
        thr_param.base.v_pos = 16 * i;
        thr_param.base.h_pos = 0;
        memcpy(pos, &thr_param, thr_param_size);
        pos += thr_param_size;
    }

    vpp_gpe_ctx->sub_shader_index = 0;
    va_status = vpp_gpe_process(ctx, vpp_gpe_ctx);
    free(vpp_gpe_ctx->thread_param);

    /* Step 2: vertical blur process */ 
    vpp_gpe_ctx->surface_input_object[0] = vpp_gpe_ctx->surface_output_object;
    vpp_gpe_ctx->surface_output_object = vpp_gpe_ctx->surface_tmp_object;
    vpp_gpe_ctx->forward_surf_sum = 0;
    vpp_gpe_ctx->backward_surf_sum = 0;
 
    vpp_gpe_ctx->thread_num = vpp_gpe_ctx->in_frame_w/16;
    vpp_gpe_ctx->thread_param_size = thr_param_size;
    vpp_gpe_ctx->thread_param = (unsigned char*) malloc(vpp_gpe_ctx->thread_param_size
                                                       *vpp_gpe_ctx->thread_num);
    pos = vpp_gpe_ctx->thread_param;

    if (!pos) {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    for( i = 0 ; i < vpp_gpe_ctx->thread_num; i++){
        thr_param.base.v_pos = 0;
        thr_param.base.h_pos = 16 * i;
        memcpy(pos, &thr_param, thr_param_size);
        pos += thr_param_size;
    }

    vpp_gpe_ctx->sub_shader_index = 1;
    vpp_gpe_process(ctx, vpp_gpe_ctx);
    free(vpp_gpe_ctx->thread_param);

    /* Step 3: apply the blur to original surface */      
    vpp_gpe_ctx->surface_input_object[0]  = origin_in_obj_surface;
    vpp_gpe_ctx->surface_input_object[1]  = vpp_gpe_ctx->surface_tmp_object;
    vpp_gpe_ctx->surface_output_object    = origin_out_obj_surface;
    vpp_gpe_ctx->forward_surf_sum  = 1;
    vpp_gpe_ctx->backward_surf_sum = 0;
 
    vpp_gpe_ctx->thread_num = vpp_gpe_ctx->in_frame_h/4;
    vpp_gpe_ctx->thread_param_size = thr_param_size;
    vpp_gpe_ctx->thread_param = (unsigned char*) malloc(vpp_gpe_ctx->thread_param_size
                                                       *vpp_gpe_ctx->thread_num);
    pos = vpp_gpe_ctx->thread_param;

    if (!pos) {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    for( i = 0 ; i < vpp_gpe_ctx->thread_num; i++){
        thr_param.base.v_pos = 4 * i;
        thr_param.base.h_pos = 0;
        memcpy(pos, &thr_param, thr_param_size);
        pos += thr_param_size;
    }

    vpp_gpe_ctx->sub_shader_index = 2;
    va_status = vpp_gpe_process(ctx, vpp_gpe_ctx);
    free(vpp_gpe_ctx->thread_param);

    return va_status;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

VAStatus vpp_gpe_process_picture(VADriverContextP ctx,
                    struct vpp_gpe_context * vpp_gpe_ctx)
{
    VAStatus va_status = VA_STATUS_SUCCESS;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAProcPipelineParameterBuffer* pipe = vpp_gpe_ctx->pipeline_param;
    VAProcFilterParameterBuffer* filter = NULL;
    unsigned int i;
    struct object_surface *obj_surface = NULL;

    if (pipe->num_filters && !pipe->filters)
        goto error;

    for(i = 0; i < pipe->num_filters; i++){
        struct object_buffer *obj_buf = BUFFER(pipe->filters[i]);

        assert(obj_buf && obj_buf->buffer_store && obj_buf->buffer_store->buffer);

        if (!obj_buf ||
            !obj_buf->buffer_store ||
            !obj_buf->buffer_store->buffer)
            goto error;

        filter = (VAProcFilterParameterBuffer*)obj_buf-> buffer_store->buffer;
        if(filter->type == VAProcFilterSharpening){
           break;
        }
    }
       
    assert(pipe->num_forward_references + pipe->num_backward_references <= 4);
    vpp_gpe_ctx->surface_input_object[0] = vpp_gpe_ctx->surface_pipeline_input_object;

    vpp_gpe_ctx->forward_surf_sum = 0;
    vpp_gpe_ctx->backward_surf_sum = 0;
 
    for(i = 0; i < pipe->num_forward_references; i ++)
    {
        obj_surface = SURFACE(pipe->forward_references[i]);

        assert(obj_surface);
        vpp_gpe_ctx->surface_input_object[i + 1] = obj_surface;
        vpp_gpe_ctx->forward_surf_sum++;
    } 

    for(i = 0; i < pipe->num_backward_references; i ++)
    {
        obj_surface = SURFACE(pipe->backward_references[i]);
        
        assert(obj_surface);
        vpp_gpe_ctx->surface_input_object[vpp_gpe_ctx->forward_surf_sum + 1 + i ] = obj_surface;
        vpp_gpe_ctx->backward_surf_sum++;
    } 

    obj_surface = vpp_gpe_ctx->surface_input_object[0];
    vpp_gpe_ctx->in_frame_w = obj_surface->orig_width;
    vpp_gpe_ctx->in_frame_h = obj_surface->orig_height;

    if(filter && filter->type == VAProcFilterSharpening) {
       va_status = vpp_gpe_process_sharpening(ctx, vpp_gpe_ctx); 
    } else {
       va_status = VA_STATUS_ERROR_ATTR_NOT_SUPPORTED;
    }

    vpp_gpe_ctx->is_first_frame = 0;

    return va_status;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

void 
vpp_gpe_context_destroy(VADriverContextP ctx,
                               struct vpp_gpe_context *vpp_gpe_ctx)
{
    dri_bo_unreference(vpp_gpe_ctx->vpp_batchbuffer.bo);
    vpp_gpe_ctx->vpp_batchbuffer.bo = NULL;

    dri_bo_unreference(vpp_gpe_ctx->vpp_kernel_return.bo);
    vpp_gpe_ctx->vpp_kernel_return.bo = NULL;

    vpp_gpe_ctx->gpe_context_destroy(&vpp_gpe_ctx->gpe_ctx);

    if(vpp_gpe_ctx->surface_tmp != VA_INVALID_ID){
        assert(vpp_gpe_ctx->surface_tmp_object != NULL);
        i965_DestroySurfaces(ctx, &vpp_gpe_ctx->surface_tmp, 1);
        vpp_gpe_ctx->surface_tmp = VA_INVALID_ID;
        vpp_gpe_ctx->surface_tmp_object = NULL;
    }   

    if (vpp_gpe_ctx->batch)
        intel_batchbuffer_free(vpp_gpe_ctx->batch);

    free(vpp_gpe_ctx);
}

struct vpp_gpe_context *
vpp_gpe_context_init(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct vpp_gpe_context  *vpp_gpe_ctx = calloc(1, sizeof(struct vpp_gpe_context));
    assert(vpp_gpe_ctx);
    struct i965_gpe_context *gpe_ctx = &(vpp_gpe_ctx->gpe_ctx);

    assert(IS_HASWELL(i965->intel.device_info) ||
           IS_GEN8(i965->intel.device_info) ||
           IS_GEN9(i965->intel.device_info));

    vpp_gpe_ctx->surface_tmp = VA_INVALID_ID;
    vpp_gpe_ctx->surface_tmp_object = NULL;
    vpp_gpe_ctx->batch = intel_batchbuffer_new(&i965->intel, I915_EXEC_RENDER, 0);
    vpp_gpe_ctx->is_first_frame = 1;

    gpe_ctx->vfe_state.max_num_threads = 60 - 1;
    gpe_ctx->vfe_state.num_urb_entries = 16;
    gpe_ctx->vfe_state.gpgpu_mode = 0;
    gpe_ctx->vfe_state.urb_entry_size = 59 - 1;
    gpe_ctx->vfe_state.curbe_allocation_size = CURBE_ALLOCATION_SIZE - 1;
 
    if (IS_HASWELL(i965->intel.device_info)) {
        vpp_gpe_ctx->gpe_context_init     = i965_gpe_context_init;
        vpp_gpe_ctx->gpe_context_destroy  = i965_gpe_context_destroy;
        vpp_gpe_ctx->gpe_load_kernels     = i965_gpe_load_kernels;
        gpe_ctx->surface_state_binding_table.length =
               (SURFACE_STATE_PADDED_SIZE_GEN7 + sizeof(unsigned int)) * MAX_MEDIA_SURFACES_GEN6;

        gpe_ctx->curbe.length = CURBE_TOTAL_DATA_LENGTH;
        gpe_ctx->idrt.max_entries = MAX_INTERFACE_DESC_GEN6;
        gpe_ctx->idrt.entry_size = sizeof(struct gen6_interface_descriptor_data);

    } else if (IS_GEN8(i965->intel.device_info) ||
               IS_GEN9(i965->intel.device_info)) {
        vpp_gpe_ctx->gpe_context_init     = gen8_gpe_context_init;
        vpp_gpe_ctx->gpe_context_destroy  = gen8_gpe_context_destroy;
        vpp_gpe_ctx->gpe_load_kernels     = gen8_gpe_load_kernels;
        gpe_ctx->surface_state_binding_table.length =
               (SURFACE_STATE_PADDED_SIZE_GEN8 + sizeof(unsigned int)) * MAX_MEDIA_SURFACES_GEN6;

        gpe_ctx->curbe_size = CURBE_TOTAL_DATA_LENGTH;
        gpe_ctx->idrt_size  = sizeof(struct gen8_interface_descriptor_data) * MAX_INTERFACE_DESC_GEN6;

    }

    return vpp_gpe_ctx;
}

