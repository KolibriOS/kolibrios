/*
 * Copyright (C) 2012 Intel Corporation
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
 */

#ifndef I965_DECODER_H
#define I965_DECODER_H

#include <stdint.h>
#include <stdlib.h>

#include <va/va.h>
#include <intel_bufmgr.h>

#define MAX_GEN_REFERENCE_FRAMES 16

typedef struct gen_frame_store GenFrameStore;
struct gen_frame_store {
    VASurfaceID surface_id;
    int         frame_store_id;
};

typedef struct gen_buffer GenBuffer;
struct gen_buffer {
    dri_bo     *bo;
    int         valid;
};

#if HAVE_GEN_AVC_SURFACE

//static pthread_mutex_t free_avc_surface_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct gen_avc_surface GenAvcSurface;
struct gen_avc_surface
{
    dri_bo *dmv_top;
    dri_bo *dmv_bottom;
    int dmv_bottom_flag;
};

static void 
gen_free_avc_surface(void **data)
{
    GenAvcSurface *avc_surface;

//    pthread_mutex_lock(&free_avc_surface_lock);

    avc_surface = *data;

    if (!avc_surface) {
//        pthread_mutex_unlock(&free_avc_surface_lock);
        return;
    }


    dri_bo_unreference(avc_surface->dmv_top);
    avc_surface->dmv_top = NULL;
    dri_bo_unreference(avc_surface->dmv_bottom);
    avc_surface->dmv_bottom = NULL;

    free(avc_surface);
    *data = NULL;

//    pthread_mutex_unlock(&free_avc_surface_lock);
}

#endif

extern struct hw_context *
gen75_dec_hw_context_init(VADriverContextP ctx, VAProfile profile);

#endif /* I965_DECODER_H */
