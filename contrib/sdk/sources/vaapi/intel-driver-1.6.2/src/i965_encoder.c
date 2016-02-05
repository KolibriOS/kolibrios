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
 *    Zhou Chang <chang.zhou@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"

#include "i965_defines.h"
#include "i965_drv_video.h"
#include "i965_encoder.h"
#include "gen6_vme.h"
#include "gen6_mfc.h"
#include "gen9_mfc.h"

extern Bool gen6_mfc_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);
extern Bool gen6_vme_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);
extern Bool gen7_mfc_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);
extern Bool gen9_hcpe_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);

static VAStatus
intel_encoder_check_yuv_surface(VADriverContextP ctx,
                                VAProfile profile,
                                struct encode_state *encode_state,
                                struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_surface src_surface, dst_surface;
    struct object_surface *obj_surface;
    VAStatus status;
    VARectangle rect;

    /* releae the temporary surface */
    if (encoder_context->is_tmp_id) {
        i965_DestroySurfaces(ctx, &encoder_context->input_yuv_surface, 1);
        encode_state->input_yuv_object = NULL;
    }

    encoder_context->is_tmp_id = 0;
    obj_surface = SURFACE(encode_state->current_render_target);
    assert(obj_surface && obj_surface->bo);

    if (!obj_surface || !obj_surface->bo)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    if (obj_surface->fourcc == VA_FOURCC_NV12) {
        unsigned int tiling = 0, swizzle = 0;

        dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);

        if (tiling == I915_TILING_Y) {
            encoder_context->input_yuv_surface = encode_state->current_render_target;
            encode_state->input_yuv_object = obj_surface;
            return VA_STATUS_SUCCESS;
        }
    }

    rect.x = 0;
    rect.y = 0;
    rect.width = obj_surface->orig_width;
    rect.height = obj_surface->orig_height;
    
    src_surface.base = (struct object_base *)obj_surface;
    src_surface.type = I965_SURFACE_TYPE_SURFACE;
    src_surface.flags = I965_SURFACE_FLAG_FRAME;
    
    status = i965_CreateSurfaces(ctx,
                                 obj_surface->orig_width,
                                 obj_surface->orig_height,
                                 VA_RT_FORMAT_YUV420,
                                 1,
                                 &encoder_context->input_yuv_surface);
    assert(status == VA_STATUS_SUCCESS);

    if (status != VA_STATUS_SUCCESS)
        return status;

    obj_surface = SURFACE(encoder_context->input_yuv_surface);
    encode_state->input_yuv_object = obj_surface;
    assert(obj_surface);
    i965_check_alloc_surface_bo(ctx, obj_surface, 1, VA_FOURCC_NV12, SUBSAMPLE_YUV420);
    
    dst_surface.base = (struct object_base *)obj_surface;
    dst_surface.type = I965_SURFACE_TYPE_SURFACE;
    dst_surface.flags = I965_SURFACE_FLAG_FRAME;

    status = i965_image_processing(ctx,
                                   &src_surface,
                                   &rect,
                                   &dst_surface,
                                   &rect);
    assert(status == VA_STATUS_SUCCESS);

    encoder_context->is_tmp_id = 1;

    return VA_STATUS_SUCCESS;
}


static VAStatus
intel_encoder_check_jpeg_yuv_surface(VADriverContextP ctx,
                                VAProfile profile,
                                struct encode_state *encode_state,
                                struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_surface src_surface, dst_surface;
    struct object_surface *obj_surface;
    VAStatus status;
    VARectangle rect;
    int format=0, fourcc=0, subsample=0;

    /* releae the temporary surface */
    if (encoder_context->is_tmp_id) {
        i965_DestroySurfaces(ctx, &encoder_context->input_yuv_surface, 1);
        encode_state->input_yuv_object = NULL;
    }

    encoder_context->is_tmp_id = 0;
    obj_surface = SURFACE(encode_state->current_render_target);
    assert(obj_surface && obj_surface->bo);

    if (!obj_surface || !obj_surface->bo)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    unsigned int tiling = 0, swizzle = 0;

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);

    if (tiling == I915_TILING_Y) {
        if( (obj_surface->fourcc==VA_FOURCC_NV12)  || (obj_surface->fourcc==VA_FOURCC_UYVY) ||
            (obj_surface->fourcc==VA_FOURCC_YUY2)  || (obj_surface->fourcc==VA_FOURCC_Y800) ||
            (obj_surface->fourcc==VA_FOURCC_RGBA)  || (obj_surface->fourcc==VA_FOURCC_444P) ) {
            encoder_context->input_yuv_surface = encode_state->current_render_target;
            encode_state->input_yuv_object = obj_surface;
            return VA_STATUS_SUCCESS;
        }
    }

    rect.x = 0;
    rect.y = 0;
    rect.width = obj_surface->orig_width;
    rect.height = obj_surface->orig_height;

    src_surface.base = (struct object_base *)obj_surface;
    src_surface.type = I965_SURFACE_TYPE_SURFACE;
    src_surface.flags = I965_SURFACE_FLAG_FRAME;

    switch( obj_surface->fourcc) {

        case VA_FOURCC_YUY2:
            fourcc = VA_FOURCC_YUY2;
            format = VA_RT_FORMAT_YUV422;
            subsample = SUBSAMPLE_YUV422H;
            break;

        case VA_FOURCC_UYVY:
            fourcc = VA_FOURCC_UYVY;
            format = VA_RT_FORMAT_YUV422;
            subsample = SUBSAMPLE_YUV422H;
            break;

        case VA_FOURCC_Y800:
            fourcc = VA_FOURCC_Y800;
            format = VA_RT_FORMAT_YUV400;
            subsample = SUBSAMPLE_YUV400;
            break;

        case VA_FOURCC_444P:
            fourcc = VA_FOURCC_444P;
            format = VA_RT_FORMAT_YUV444;
            subsample = SUBSAMPLE_YUV444;
            break;

        case VA_FOURCC_RGBA:
            fourcc = VA_FOURCC_RGBA;
            format = VA_RT_FORMAT_RGB32;
            subsample = SUBSAMPLE_RGBX;
            break;

        default: //All other scenarios will have NV12 format
            fourcc = VA_FOURCC_NV12;
            format = VA_RT_FORMAT_YUV420;
            subsample = SUBSAMPLE_YUV420;
            break;
    }

    status = i965_CreateSurfaces(ctx,
                                 obj_surface->orig_width,
                                 obj_surface->orig_height,
                                 format,
                                 1,
                                 &encoder_context->input_yuv_surface);
    assert(status == VA_STATUS_SUCCESS);

    if (status != VA_STATUS_SUCCESS)
        return status;

    obj_surface = SURFACE(encoder_context->input_yuv_surface);
    encode_state->input_yuv_object = obj_surface;
    assert(obj_surface);
    i965_check_alloc_surface_bo(ctx, obj_surface, 1, fourcc, subsample);

    dst_surface.base = (struct object_base *)obj_surface;
    dst_surface.type = I965_SURFACE_TYPE_SURFACE;
    dst_surface.flags = I965_SURFACE_FLAG_FRAME;

    //The Y800 format is expected to be tiled.
    //Linear Y800 is a corner case and needs code in the i965_image_processing.
    if(obj_surface->fourcc != VA_FOURCC_Y800){
        status = i965_image_processing(ctx,
                                   &src_surface,
                                   &rect,
                                   &dst_surface,
                                   &rect);
        assert(status == VA_STATUS_SUCCESS);
    }

    encoder_context->is_tmp_id = 1;

    return VA_STATUS_SUCCESS;
}

static VAStatus
intel_encoder_check_misc_parameter(VADriverContextP ctx,
                                  struct encode_state *encode_state,
                                  struct intel_encoder_context *encoder_context)
{

    if (encode_state->misc_param[VAEncMiscParameterTypeQualityLevel] &&
        encode_state->misc_param[VAEncMiscParameterTypeQualityLevel]->buffer) {
        VAEncMiscParameterBuffer* pMiscParam = (VAEncMiscParameterBuffer*)encode_state->misc_param[VAEncMiscParameterTypeQualityLevel]->buffer;
        VAEncMiscParameterBufferQualityLevel* param_quality_level = (VAEncMiscParameterBufferQualityLevel*)pMiscParam->data;
        encoder_context->quality_level = param_quality_level->quality_level;

        if (encoder_context->quality_level == 0)
            encoder_context->quality_level = ENCODER_DEFAULT_QUALITY;
        else if (encoder_context->quality_level > encoder_context->quality_range)
            goto error;
   }

    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

static VAStatus
intel_encoder_check_avc_parameter(VADriverContextP ctx,
                                  struct encode_state *encode_state,
                                  struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_surface *obj_surface;	
    struct object_buffer *obj_buffer;
    VAEncPictureParameterBufferH264 *pic_param = (VAEncPictureParameterBufferH264 *)encode_state->pic_param_ext->buffer;
    int i;

    assert(!(pic_param->CurrPic.flags & VA_PICTURE_H264_INVALID));

    if (pic_param->CurrPic.flags & VA_PICTURE_H264_INVALID)
        goto error;

    obj_surface = SURFACE(pic_param->CurrPic.picture_id);
    assert(obj_surface); /* It is possible the store buffer isn't allocated yet */
    
    if (!obj_surface)
        goto error;

    encode_state->reconstructed_object = obj_surface;
    obj_buffer = BUFFER(pic_param->coded_buf);
    assert(obj_buffer && obj_buffer->buffer_store && obj_buffer->buffer_store->bo);

    if (!obj_buffer || !obj_buffer->buffer_store || !obj_buffer->buffer_store->bo)
        goto error;

    encode_state->coded_buf_object = obj_buffer;

    for (i = 0; i < 16; i++) {
        if (pic_param->ReferenceFrames[i].flags & VA_PICTURE_H264_INVALID ||
            pic_param->ReferenceFrames[i].picture_id == VA_INVALID_SURFACE)
            break;
        else {
            obj_surface = SURFACE(pic_param->ReferenceFrames[i].picture_id);
            assert(obj_surface);

            if (!obj_surface)
                goto error;

            if (obj_surface->bo)
                encode_state->reference_objects[i] = obj_surface;
            else
                encode_state->reference_objects[i] = NULL; /* FIXME: Warning or Error ??? */
        }
    }

    for ( ; i < 16; i++)
        encode_state->reference_objects[i] = NULL;
    
    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

static VAStatus
intel_encoder_check_mpeg2_parameter(VADriverContextP ctx,
                                    struct encode_state *encode_state,
                                    struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAEncPictureParameterBufferMPEG2 *pic_param = (VAEncPictureParameterBufferMPEG2 *)encode_state->pic_param_ext->buffer;
    struct object_surface *obj_surface;	
    struct object_buffer *obj_buffer;
    int i = 0;
    
    obj_surface = SURFACE(pic_param->reconstructed_picture);
    assert(obj_surface); /* It is possible the store buffer isn't allocated yet */
    
    if (!obj_surface)
        goto error;
    
    encode_state->reconstructed_object = obj_surface;    
    obj_buffer = BUFFER(pic_param->coded_buf);
    assert(obj_buffer && obj_buffer->buffer_store && obj_buffer->buffer_store->bo);

    if (!obj_buffer || !obj_buffer->buffer_store || !obj_buffer->buffer_store->bo)
        goto error;

    encode_state->coded_buf_object = obj_buffer;

    if (pic_param->picture_type == VAEncPictureTypeIntra) {
    } else if (pic_param->picture_type == VAEncPictureTypePredictive) {
        assert(pic_param->forward_reference_picture != VA_INVALID_SURFACE);
        obj_surface = SURFACE(pic_param->forward_reference_picture);
        assert(obj_surface && obj_surface->bo);

        if (!obj_surface || !obj_surface->bo)
            goto error;

        encode_state->reference_objects[i++] = obj_surface;
    } else if (pic_param->picture_type == VAEncPictureTypeBidirectional) {
        assert(pic_param->forward_reference_picture != VA_INVALID_SURFACE);
        obj_surface = SURFACE(pic_param->forward_reference_picture);
        assert(obj_surface && obj_surface->bo);

        if (!obj_surface || !obj_surface->bo)
            goto error;

        encode_state->reference_objects[i++] = obj_surface;

        assert(pic_param->backward_reference_picture != VA_INVALID_SURFACE);
        obj_surface = SURFACE(pic_param->backward_reference_picture);
        assert(obj_surface && obj_surface->bo);

        if (!obj_surface || !obj_surface->bo)
            goto error;

        encode_state->reference_objects[i++] = obj_surface;
    } else 
        goto error;

    for ( ; i < 16; i++)
        encode_state->reference_objects[i] = NULL;

    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

static VAStatus
intel_encoder_check_jpeg_parameter(VADriverContextP ctx,
                                  struct encode_state *encode_state,
                                  struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_buffer *obj_buffer;
    VAEncPictureParameterBufferJPEG *pic_param = (VAEncPictureParameterBufferJPEG *)encode_state->pic_param_ext->buffer;


    assert(!(pic_param->pic_flags.bits.profile)); //Baseline profile is 0.

    obj_buffer = BUFFER(pic_param->coded_buf);
    assert(obj_buffer && obj_buffer->buffer_store && obj_buffer->buffer_store->bo);

    if (!obj_buffer || !obj_buffer->buffer_store || !obj_buffer->buffer_store->bo)
        goto error;

    encode_state->coded_buf_object = obj_buffer;

    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

static VAStatus
intel_encoder_check_vp8_parameter(VADriverContextP ctx,
                                    struct encode_state *encode_state,
                                    struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;
    struct object_surface *obj_surface;
    struct object_buffer *obj_buffer;
    int i = 0;
    int is_key_frame = !pic_param->pic_flags.bits.frame_type;
 
    obj_surface = SURFACE(pic_param->reconstructed_frame);
    assert(obj_surface); /* It is possible the store buffer isn't allocated yet */
    
    if (!obj_surface)
        goto error;
    
    encode_state->reconstructed_object = obj_surface;    
    obj_buffer = BUFFER(pic_param->coded_buf);
    assert(obj_buffer && obj_buffer->buffer_store && obj_buffer->buffer_store->bo);

    if (!obj_buffer || !obj_buffer->buffer_store || !obj_buffer->buffer_store->bo)
        goto error;

    encode_state->coded_buf_object = obj_buffer;

    if (!is_key_frame) {
        assert(pic_param->ref_last_frame != VA_INVALID_SURFACE);
        obj_surface = SURFACE(pic_param->ref_last_frame);
        assert(obj_surface && obj_surface->bo);

        if (!obj_surface || !obj_surface->bo)
            goto error;

        encode_state->reference_objects[i++] = obj_surface;

        assert(pic_param->ref_gf_frame != VA_INVALID_SURFACE);
        obj_surface = SURFACE(pic_param->ref_gf_frame);
        assert(obj_surface && obj_surface->bo);

        if (!obj_surface || !obj_surface->bo)
            goto error;

        encode_state->reference_objects[i++] = obj_surface;

        assert(pic_param->ref_arf_frame != VA_INVALID_SURFACE);
        obj_surface = SURFACE(pic_param->ref_arf_frame);
        assert(obj_surface && obj_surface->bo);

        if (!obj_surface || !obj_surface->bo)
            goto error;

        encode_state->reference_objects[i++] = obj_surface;
    }

    for ( ; i < 16; i++)
        encode_state->reference_objects[i] = NULL;

    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

static VAStatus
intel_encoder_check_hevc_parameter(VADriverContextP ctx,
                                  struct encode_state *encode_state,
                                  struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_surface *obj_surface;	
    struct object_buffer *obj_buffer;
    VAEncPictureParameterBufferHEVC *pic_param = (VAEncPictureParameterBufferHEVC *)encode_state->pic_param_ext->buffer;
    VAEncSliceParameterBufferHEVC *slice_param;
    int i;

    assert(!(pic_param->decoded_curr_pic.flags & VA_PICTURE_HEVC_INVALID));

    if (pic_param->decoded_curr_pic.flags & VA_PICTURE_HEVC_INVALID)
        goto error;

    obj_surface = SURFACE(pic_param->decoded_curr_pic.picture_id);
    assert(obj_surface); /* It is possible the store buffer isn't allocated yet */
    
    if (!obj_surface)
        goto error;

    encode_state->reconstructed_object = obj_surface;
    obj_buffer = BUFFER(pic_param->coded_buf);
    assert(obj_buffer && obj_buffer->buffer_store && obj_buffer->buffer_store->bo);

    if (!obj_buffer || !obj_buffer->buffer_store || !obj_buffer->buffer_store->bo)
        goto error;

    encode_state->coded_buf_object = obj_buffer;

    for (i = 0; i < 15; i++) {
        if (pic_param->reference_frames[i].flags & VA_PICTURE_HEVC_INVALID ||
            pic_param->reference_frames[i].picture_id == VA_INVALID_SURFACE)
            break;
        else {
            obj_surface = SURFACE(pic_param->reference_frames[i].picture_id);
            assert(obj_surface);

            if (!obj_surface)
                goto error;

            if (obj_surface->bo)
                encode_state->reference_objects[i] = obj_surface;
            else
                encode_state->reference_objects[i] = NULL; /* FIXME: Warning or Error ??? */
        }
    }

    for ( ; i < 15; i++)
        encode_state->reference_objects[i] = NULL;

    for (i = 0; i < encode_state->num_slice_params_ext; i++) {
        slice_param = (VAEncSliceParameterBufferHEVC *)encode_state->slice_params_ext[i]->buffer;

        if (slice_param->slice_type != HEVC_SLICE_I &&
            slice_param->slice_type != HEVC_SLICE_P &&
            slice_param->slice_type != HEVC_SLICE_B)
            goto error;

        /* TODO: add more check here */
    }

    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}
static VAStatus
intel_encoder_sanity_check_input(VADriverContextP ctx,
                                 VAProfile profile,
                                 struct encode_state *encode_state,
                                 struct intel_encoder_context *encoder_context)
{
    VAStatus vaStatus;

    switch (profile) {
    case VAProfileH264ConstrainedBaseline:
    case VAProfileH264Main:
    case VAProfileH264High:
    case VAProfileH264MultiviewHigh:
    case VAProfileH264StereoHigh: {
        vaStatus = intel_encoder_check_avc_parameter(ctx, encode_state, encoder_context);
        if (vaStatus != VA_STATUS_SUCCESS)
            goto out;
        vaStatus = intel_encoder_check_yuv_surface(ctx, profile, encode_state, encoder_context);
        break;
    }

    case VAProfileMPEG2Simple:
    case VAProfileMPEG2Main: {
        vaStatus = intel_encoder_check_mpeg2_parameter(ctx, encode_state, encoder_context);
        if (vaStatus != VA_STATUS_SUCCESS)
            goto out;
        vaStatus = intel_encoder_check_yuv_surface(ctx, profile, encode_state, encoder_context);
        break;
    }

    case VAProfileJPEGBaseline:  {
        vaStatus = intel_encoder_check_jpeg_parameter(ctx, encode_state, encoder_context);
        if (vaStatus != VA_STATUS_SUCCESS)
            goto out;
        vaStatus = intel_encoder_check_jpeg_yuv_surface(ctx, profile, encode_state, encoder_context);
        break;
    }
 
    case VAProfileVP8Version0_3: {
        vaStatus = intel_encoder_check_vp8_parameter(ctx, encode_state, encoder_context);
         if (vaStatus != VA_STATUS_SUCCESS)
            goto out;
        vaStatus = intel_encoder_check_yuv_surface(ctx, profile, encode_state, encoder_context);
        break;
    }

    case VAProfileHEVCMain:  {
        vaStatus = intel_encoder_check_hevc_parameter(ctx, encode_state, encoder_context);
        if (vaStatus != VA_STATUS_SUCCESS)
            goto out;
        vaStatus = intel_encoder_check_yuv_surface(ctx, profile, encode_state, encoder_context);
        break;
    }
    default:
        vaStatus = VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        break;
    }

    if (vaStatus == VA_STATUS_SUCCESS)
        vaStatus = intel_encoder_check_misc_parameter(ctx, encode_state, encoder_context);

out:    
    return vaStatus;
}
 
static VAStatus
intel_encoder_end_picture(VADriverContextP ctx, 
                          VAProfile profile, 
                          union codec_state *codec_state,
                          struct hw_context *hw_context)
{
    struct intel_encoder_context *encoder_context = (struct intel_encoder_context *)hw_context;
    struct encode_state *encode_state = &codec_state->encode;
    VAStatus vaStatus;

    vaStatus = intel_encoder_sanity_check_input(ctx, profile, encode_state, encoder_context);

    if (vaStatus != VA_STATUS_SUCCESS)
        return vaStatus;

    encoder_context->mfc_brc_prepare(encode_state, encoder_context);

    if((encoder_context->vme_context && encoder_context->vme_pipeline)) {
        vaStatus = encoder_context->vme_pipeline(ctx, profile, encode_state, encoder_context);
    }

    if (vaStatus == VA_STATUS_SUCCESS)
        encoder_context->mfc_pipeline(ctx, profile, encode_state, encoder_context);
    return VA_STATUS_SUCCESS;
}

static void
intel_encoder_context_destroy(void *hw_context)
{
    struct intel_encoder_context *encoder_context = (struct intel_encoder_context *)hw_context;

    encoder_context->mfc_context_destroy(encoder_context->mfc_context);

    if (encoder_context->vme_context_destroy && encoder_context->vme_context)
       encoder_context->vme_context_destroy(encoder_context->vme_context);

    intel_batchbuffer_free(encoder_context->base.batch);
    free(encoder_context);
}

typedef Bool (* hw_init_func)(VADriverContextP, struct intel_encoder_context *);

static struct hw_context *
intel_enc_hw_context_init(VADriverContextP ctx,
                          struct object_config *obj_config,
                          hw_init_func vme_context_init,
                          hw_init_func mfc_context_init)
{
    struct intel_driver_data *intel = intel_driver_data(ctx);
    struct intel_encoder_context *encoder_context = calloc(1, sizeof(struct intel_encoder_context));
    int i;

    assert(encoder_context);
    encoder_context->base.destroy = intel_encoder_context_destroy;
    encoder_context->base.run = intel_encoder_end_picture;
    encoder_context->base.batch = intel_batchbuffer_new(intel, I915_EXEC_RENDER, 0);
    encoder_context->input_yuv_surface = VA_INVALID_SURFACE;
    encoder_context->is_tmp_id = 0;
    encoder_context->rate_control_mode = VA_RC_NONE;
    encoder_context->quality_level = ENCODER_DEFAULT_QUALITY;
    encoder_context->quality_range = 1;

    switch (obj_config->profile) {
    case VAProfileMPEG2Simple:
    case VAProfileMPEG2Main:
        encoder_context->codec = CODEC_MPEG2;
        break;
        
    case VAProfileH264ConstrainedBaseline:
    case VAProfileH264Main:
    case VAProfileH264High:
        encoder_context->codec = CODEC_H264;
        encoder_context->quality_range = ENCODER_QUALITY_RANGE;
        break;

    case VAProfileH264StereoHigh:
    case VAProfileH264MultiviewHigh:
        encoder_context->codec = CODEC_H264_MVC;
        break;
        
    case VAProfileJPEGBaseline:
        encoder_context->codec = CODEC_JPEG;
        break;

    case VAProfileVP8Version0_3:
        encoder_context->codec = CODEC_VP8;
        break;

    case VAProfileHEVCMain:
        encoder_context->codec = CODEC_HEVC;
        break;

    default:
        /* Never get here */
        assert(0);
        break;
    }

    for (i = 0; i < obj_config->num_attribs; i++) {
        if (obj_config->attrib_list[i].type == VAConfigAttribRateControl) {
            encoder_context->rate_control_mode = obj_config->attrib_list[i].value;

            if (encoder_context->codec == CODEC_MPEG2 &&
                encoder_context->rate_control_mode & VA_RC_CBR) {
                WARN_ONCE("Don't support CBR for MPEG-2 encoding\n");
                encoder_context->rate_control_mode &= ~VA_RC_CBR;
            }

            break;
        }
    }

    vme_context_init(ctx, encoder_context);
    if(obj_config->profile != VAProfileJPEGBaseline) {
        assert(encoder_context->vme_context);
        assert(encoder_context->vme_context_destroy);
        assert(encoder_context->vme_pipeline);
    }

    mfc_context_init(ctx, encoder_context);
    assert(encoder_context->mfc_context);
    assert(encoder_context->mfc_context_destroy);
    assert(encoder_context->mfc_pipeline);

    return (struct hw_context *)encoder_context;
}

struct hw_context *
gen6_enc_hw_context_init(VADriverContextP ctx, struct object_config *obj_config)
{
    return intel_enc_hw_context_init(ctx, obj_config, gen6_vme_context_init, gen6_mfc_context_init);
}

struct hw_context *
gen7_enc_hw_context_init(VADriverContextP ctx, struct object_config *obj_config)
{

    return intel_enc_hw_context_init(ctx, obj_config, gen7_vme_context_init, gen7_mfc_context_init);
}

struct hw_context *
gen75_enc_hw_context_init(VADriverContextP ctx, struct object_config *obj_config)
{
    return intel_enc_hw_context_init(ctx, obj_config, gen75_vme_context_init, gen75_mfc_context_init);
}

struct hw_context *
gen8_enc_hw_context_init(VADriverContextP ctx, struct object_config *obj_config)
{
    return intel_enc_hw_context_init(ctx, obj_config, gen8_vme_context_init, gen8_mfc_context_init);
}

struct hw_context *
gen9_enc_hw_context_init(VADriverContextP ctx, struct object_config *obj_config)
{
    if (obj_config->profile == VAProfileHEVCMain) {
        return intel_enc_hw_context_init(ctx, obj_config, gen9_vme_context_init, gen9_hcpe_context_init);
    } else if (obj_config->profile == VAProfileJPEGBaseline)
        return intel_enc_hw_context_init(ctx, obj_config, gen8_vme_context_init, gen8_mfc_context_init);
    else
        return intel_enc_hw_context_init(ctx, obj_config, gen9_vme_context_init, gen9_mfc_context_init);
}
