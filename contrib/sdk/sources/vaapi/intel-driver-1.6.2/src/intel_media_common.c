/*
 * Copyright (C) 2006-2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "intel_driver.h"
#include "intel_media.h"

//static pthread_mutex_t free_avc_surface_lock = PTHREAD_MUTEX_INITIALIZER;

void
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

/* This is to convert one float to the given format interger.
 * For example: 1.25 to S1.6 or U2.6 and so on
 */
int intel_format_convert(float src, int out_int_bits, int out_frac_bits,int out_sign_flag)
{
     unsigned char negative_flag = (src < 0.0) ? 1 : 0;
     float src_1 = (!negative_flag)? src: -src ;
     unsigned int factor = 1 << out_frac_bits;
     int output_value = 0;         
 
     unsigned int integer_part  = floorf(src_1);
     unsigned int fraction_part = ((int)((src_1 - integer_part) * factor)) & (factor - 1) ;

     output_value = (integer_part << out_frac_bits) | fraction_part;

     if(negative_flag)
         output_value = (~output_value + 1) & ((1 <<(out_int_bits + out_frac_bits)) -1);

     if(out_sign_flag == 1 && negative_flag)
     {
          output_value |= negative_flag <<(out_int_bits + out_frac_bits);
     }
     return output_value;
}

//static pthread_mutex_t free_hevc_surface_lock = PTHREAD_MUTEX_INITIALIZER;

void
gen_free_hevc_surface(void **data)
{
    GenHevcSurface *hevc_surface;

//    pthread_mutex_lock(&free_hevc_surface_lock);

    hevc_surface = *data;

    if (!hevc_surface) {
//        pthread_mutex_unlock(&free_hevc_surface_lock);
        return;
    }

    dri_bo_unreference(hevc_surface->motion_vector_temporal_bo);
    hevc_surface->motion_vector_temporal_bo = NULL;

    free(hevc_surface);
    *data = NULL;

//    pthread_mutex_unlock(&free_hevc_surface_lock);
}

//static pthread_mutex_t free_vp9_surface_lock = PTHREAD_MUTEX_INITIALIZER;

void gen_free_vp9_surface(void **data)
{
    GenVP9Surface *vp9_surface;

//    pthread_mutex_lock(&free_vp9_surface_lock);

    vp9_surface = *data;

    if (!vp9_surface) {
//        pthread_mutex_unlock(&free_vp9_surface_lock);
        return;
    }

    dri_bo_unreference(vp9_surface->motion_vector_temporal_bo);
    vp9_surface->motion_vector_temporal_bo = NULL;

    free(vp9_surface);
    *data = NULL;

//    pthread_mutex_unlock(&free_vp9_surface_lock);
}
