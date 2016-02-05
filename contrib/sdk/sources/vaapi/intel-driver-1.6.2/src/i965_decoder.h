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
#include <va/va_dec_vp8.h>
#include <va/va_dec_hevc.h>
#include <intel_bufmgr.h>

#define MAX_GEN_REFERENCE_FRAMES 16
#define MAX_GEN_HCP_REFERENCE_FRAMES    8

#define ALLOC_GEN_BUFFER(gen_buffer, string, size) do {         \
        dri_bo_unreference(gen_buffer->bo);                     \
        gen_buffer->bo = dri_bo_alloc(i965->intel.bufmgr,       \
                                      string,                   \
                                      size,                     \
                                      0x1000);                  \
        assert(gen_buffer->bo);                                 \
        gen_buffer->valid = 1;                                  \
    } while (0);

#define FREE_GEN_BUFFER(gen_buffer) do {        \
        dri_bo_unreference(gen_buffer->bo);     \
        gen_buffer->bo = NULL;                  \
        gen_buffer->valid = 0;                  \
    } while (0)

typedef struct gen_frame_store GenFrameStore;
struct gen_frame_store {
    VASurfaceID surface_id;
    int         frame_store_id;
    struct      object_surface *obj_surface;

    /* This represents the time when this frame store was last used to
       hold a reference frame. This is not connected to a presentation
       timestamp (PTS), and this is not a common decoding time stamp
       (DTS) either. It serves the purpose of tracking retired
       reference frame candidates.

       This is only used for H.264 decoding on platforms before Haswell */
    uint64_t    ref_age;
};

typedef struct gen_frame_store_context GenFrameStoreContext;
struct gen_frame_store_context {
    uint64_t    age;
    int         prev_poc;
};

typedef struct gen_buffer GenBuffer;
struct gen_buffer {
    dri_bo     *bo;
    int         valid;
};

struct hw_context *
gen75_dec_hw_context_init(VADriverContextP ctx, struct object_config *obj_config);

extern struct hw_context *
gen8_dec_hw_context_init(VADriverContextP ctx, struct object_config *obj_config);
#endif /* I965_DECODER_H */
