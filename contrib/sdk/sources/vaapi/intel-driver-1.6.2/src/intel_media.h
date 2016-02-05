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

#ifndef INTEL_MEDIA_H
#define INTEL_MEDIA_H

#include <stdint.h>
#include <stdlib.h>

#include <va/va.h>
#include <intel_bufmgr.h>

typedef struct gen_codec_surface GenCodecSurface;

struct gen_codec_surface
{
    int frame_store_id;
};

typedef struct gen_avc_surface GenAvcSurface;
struct gen_avc_surface
{
    GenCodecSurface base;
    dri_bo *dmv_top;
    dri_bo *dmv_bottom;
    int dmv_bottom_flag;
};

extern void gen_free_avc_surface(void **data);


extern int intel_format_convert(float src, int out_int_bits, int out_frac_bits,int out_sign_flag);

typedef struct gen_hevc_surface GenHevcSurface;
struct gen_hevc_surface
{
    GenCodecSurface base;
    dri_bo *motion_vector_temporal_bo;
};

typedef struct gen_vp9_surface GenVP9Surface;
struct gen_vp9_surface
{
    GenCodecSurface base;
    uint16_t frame_width;
    uint16_t frame_height;
    dri_bo *motion_vector_temporal_bo;
};

extern void gen_free_hevc_surface(void **data);

extern void gen_free_vp9_surface(void **data);

#endif /* INTEL_MEDIA_H */
