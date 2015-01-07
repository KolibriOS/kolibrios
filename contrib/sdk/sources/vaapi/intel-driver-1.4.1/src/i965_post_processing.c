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
 *    Xiang Haihao <haihao.xiang@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"
#include "i965_defines.h"
#include "i965_structs.h"
#include "i965_drv_video.h"
#include "i965_post_processing.h"
#include "i965_render.h"
#include "intel_media.h"

extern VAStatus
vpp_surface_convert(VADriverContextP ctx,
                    struct object_surface *src_obj_surf,
                    struct object_surface *dst_obj_surf);

#define HAS_VPP(ctx) ((ctx)->codec_info->has_vpp)

#define SURFACE_STATE_PADDED_SIZE               MAX(SURFACE_STATE_PADDED_SIZE_GEN8,\
			MAX(SURFACE_STATE_PADDED_SIZE_GEN6, SURFACE_STATE_PADDED_SIZE_GEN7))

#define SURFACE_STATE_OFFSET(index)             (SURFACE_STATE_PADDED_SIZE * index)
#define BINDING_TABLE_OFFSET                    SURFACE_STATE_OFFSET(MAX_PP_SURFACES)

#define GPU_ASM_BLOCK_WIDTH         16
#define GPU_ASM_BLOCK_HEIGHT        8
#define GPU_ASM_X_OFFSET_ALIGNMENT  4

#define VA_STATUS_SUCCESS_1                     0xFFFFFFFE

static const uint32_t pp_null_gen5[][4] = {
#include "shaders/post_processing/gen5_6/null.g4b.gen5"
};

static const uint32_t pp_nv12_load_save_nv12_gen5[][4] = {
#include "shaders/post_processing/gen5_6/nv12_load_save_nv12.g4b.gen5"
};

static const uint32_t pp_nv12_load_save_pl3_gen5[][4] = {
#include "shaders/post_processing/gen5_6/nv12_load_save_pl3.g4b.gen5"
};

static const uint32_t pp_pl3_load_save_nv12_gen5[][4] = {
#include "shaders/post_processing/gen5_6/pl3_load_save_nv12.g4b.gen5"
};

static const uint32_t pp_pl3_load_save_pl3_gen5[][4] = {
#include "shaders/post_processing/gen5_6/pl3_load_save_pl3.g4b.gen5"
};

static const uint32_t pp_nv12_scaling_gen5[][4] = {
#include "shaders/post_processing/gen5_6/nv12_scaling_nv12.g4b.gen5"
};

static const uint32_t pp_nv12_avs_gen5[][4] = {
#include "shaders/post_processing/gen5_6/nv12_avs_nv12.g4b.gen5"
};

static const uint32_t pp_nv12_dndi_gen5[][4] = {
#include "shaders/post_processing/gen5_6/nv12_dndi_nv12.g4b.gen5"
};

static const uint32_t pp_nv12_dn_gen5[][4] = {
#include "shaders/post_processing/gen5_6/nv12_dn_nv12.g4b.gen5"
};

static const uint32_t pp_nv12_load_save_pa_gen5[][4] = {
#include "shaders/post_processing/gen5_6/nv12_load_save_pa.g4b.gen5"
};

static const uint32_t pp_pl3_load_save_pa_gen5[][4] = {
#include "shaders/post_processing/gen5_6/pl3_load_save_pa.g4b.gen5"
};

static const uint32_t pp_pa_load_save_nv12_gen5[][4] = {
#include "shaders/post_processing/gen5_6/pa_load_save_nv12.g4b.gen5"
};

static const uint32_t pp_pa_load_save_pl3_gen5[][4] = {
#include "shaders/post_processing/gen5_6/pa_load_save_pl3.g4b.gen5"
};

static const uint32_t pp_pa_load_save_pa_gen5[][4] = {
#include "shaders/post_processing/gen5_6/pa_load_save_pa.g4b.gen5"
};

static const uint32_t pp_rgbx_load_save_nv12_gen5[][4] = {
#include "shaders/post_processing/gen5_6/rgbx_load_save_nv12.g4b.gen5"
};

static const uint32_t pp_nv12_load_save_rgbx_gen5[][4] = {
#include "shaders/post_processing/gen5_6/nv12_load_save_rgbx.g4b.gen5"
};

static VAStatus pp_null_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                   const struct i965_surface *src_surface,
                                   const VARectangle *src_rect,
                                   struct i965_surface *dst_surface,
                                   const VARectangle *dst_rect,
                                   void *filter_param);
static VAStatus pp_nv12_avs_initialize_nlas(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                            const struct i965_surface *src_surface,
                                            const VARectangle *src_rect,
                                            struct i965_surface *dst_surface,
                                            const VARectangle *dst_rect,
                                            void *filter_param);
static VAStatus pp_nv12_scaling_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                           const struct i965_surface *src_surface,
                                           const VARectangle *src_rect,
                                           struct i965_surface *dst_surface,
                                           const VARectangle *dst_rect,
                                           void *filter_param);
static VAStatus gen6_nv12_scaling_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                             const struct i965_surface *src_surface,
                                             const VARectangle *src_rect,
                                             struct i965_surface *dst_surface,
                                             const VARectangle *dst_rect,
                                             void *filter_param);
static VAStatus pp_plx_load_save_plx_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                                const struct i965_surface *src_surface,
                                                const VARectangle *src_rect,
                                                struct i965_surface *dst_surface,
                                                const VARectangle *dst_rect,
                                                void *filter_param);
static VAStatus pp_nv12_dndi_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                        const struct i965_surface *src_surface,
                                        const VARectangle *src_rect,
                                        struct i965_surface *dst_surface,
                                        const VARectangle *dst_rect,
                                        void *filter_param);
static VAStatus pp_nv12_dn_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                      const struct i965_surface *src_surface,
                                      const VARectangle *src_rect,
                                      struct i965_surface *dst_surface,
                                      const VARectangle *dst_rect,
                                      void *filter_param);

static struct pp_module pp_modules_gen5[] = {
    {
        {
            "NULL module (for testing)",
            PP_NULL,
            pp_null_gen5,
            sizeof(pp_null_gen5),
            NULL,
        },

        pp_null_initialize,
    },

    {
        {
            "NV12_NV12",
            PP_NV12_LOAD_SAVE_N12,
            pp_nv12_load_save_nv12_gen5,
            sizeof(pp_nv12_load_save_nv12_gen5),
            NULL,
        },

        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "NV12_PL3",
            PP_NV12_LOAD_SAVE_PL3,
            pp_nv12_load_save_pl3_gen5,
            sizeof(pp_nv12_load_save_pl3_gen5),
            NULL,
        },

        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PL3_NV12",
            PP_PL3_LOAD_SAVE_N12,
            pp_pl3_load_save_nv12_gen5,
            sizeof(pp_pl3_load_save_nv12_gen5),
            NULL,
        },

        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PL3_PL3",
            PP_PL3_LOAD_SAVE_PL3,
            pp_pl3_load_save_pl3_gen5,
            sizeof(pp_pl3_load_save_pl3_gen5),
            NULL,
        },

        pp_plx_load_save_plx_initialize
    },

    {
        {
            "NV12 Scaling module",
            PP_NV12_SCALING,
            pp_nv12_scaling_gen5,
            sizeof(pp_nv12_scaling_gen5),
            NULL,
        },

        pp_nv12_scaling_initialize,
    },

    {
        {
            "NV12 AVS module",
            PP_NV12_AVS,
            pp_nv12_avs_gen5,
            sizeof(pp_nv12_avs_gen5),
            NULL,
        },

        pp_nv12_avs_initialize_nlas,
    },

    {
        {
            "NV12 DNDI module",
            PP_NV12_DNDI,
            pp_nv12_dndi_gen5,
            sizeof(pp_nv12_dndi_gen5),
            NULL,
        },

        pp_nv12_dndi_initialize,
    },

    {
        {
            "NV12 DN module",
            PP_NV12_DN,
            pp_nv12_dn_gen5,
            sizeof(pp_nv12_dn_gen5),
            NULL,
        },

        pp_nv12_dn_initialize,
    },

    {
        {
            "NV12_PA module",
            PP_NV12_LOAD_SAVE_PA,
            pp_nv12_load_save_pa_gen5,
            sizeof(pp_nv12_load_save_pa_gen5),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PL3_PA module",
            PP_PL3_LOAD_SAVE_PA,
            pp_pl3_load_save_pa_gen5,
            sizeof(pp_pl3_load_save_pa_gen5),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PA_NV12 module",
            PP_PA_LOAD_SAVE_NV12,
            pp_pa_load_save_nv12_gen5,
            sizeof(pp_pa_load_save_nv12_gen5),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PA_PL3 module",
            PP_PA_LOAD_SAVE_PL3,
            pp_pa_load_save_pl3_gen5,
            sizeof(pp_pa_load_save_pl3_gen5),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PA_PA module",
            PP_PA_LOAD_SAVE_PA,
            pp_pa_load_save_pa_gen5,
            sizeof(pp_pa_load_save_pa_gen5),
            NULL,
        },

        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "RGBX_NV12 module",
            PP_RGBX_LOAD_SAVE_NV12,
            pp_rgbx_load_save_nv12_gen5,
            sizeof(pp_rgbx_load_save_nv12_gen5),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },
            
    {
        {
            "NV12_RGBX module",
            PP_NV12_LOAD_SAVE_RGBX,
            pp_nv12_load_save_rgbx_gen5,
            sizeof(pp_nv12_load_save_rgbx_gen5),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },
};

static const uint32_t pp_null_gen6[][4] = {
#include "shaders/post_processing/gen5_6/null.g6b"
};

static const uint32_t pp_nv12_load_save_nv12_gen6[][4] = {
#include "shaders/post_processing/gen5_6/nv12_load_save_nv12.g6b"
};

static const uint32_t pp_nv12_load_save_pl3_gen6[][4] = {
#include "shaders/post_processing/gen5_6/nv12_load_save_pl3.g6b"
};

static const uint32_t pp_pl3_load_save_nv12_gen6[][4] = {
#include "shaders/post_processing/gen5_6/pl3_load_save_nv12.g6b"
};

static const uint32_t pp_pl3_load_save_pl3_gen6[][4] = {
#include "shaders/post_processing/gen5_6/pl3_load_save_pl3.g6b"
};

static const uint32_t pp_nv12_scaling_gen6[][4] = {
#include "shaders/post_processing/gen5_6/nv12_avs_nv12.g6b"
};

static const uint32_t pp_nv12_avs_gen6[][4] = {
#include "shaders/post_processing/gen5_6/nv12_avs_nv12.g6b"
};

static const uint32_t pp_nv12_dndi_gen6[][4] = {
#include "shaders/post_processing/gen5_6/nv12_dndi_nv12.g6b"
};

static const uint32_t pp_nv12_dn_gen6[][4] = {
#include "shaders/post_processing/gen5_6/nv12_dn_nv12.g6b"
};

static const uint32_t pp_nv12_load_save_pa_gen6[][4] = {
#include "shaders/post_processing/gen5_6/nv12_load_save_pa.g6b"
};

static const uint32_t pp_pl3_load_save_pa_gen6[][4] = {
#include "shaders/post_processing/gen5_6/pl3_load_save_pa.g6b"
};

static const uint32_t pp_pa_load_save_nv12_gen6[][4] = {
#include "shaders/post_processing/gen5_6/pa_load_save_nv12.g6b"
};

static const uint32_t pp_pa_load_save_pl3_gen6[][4] = {
#include "shaders/post_processing/gen5_6/pa_load_save_pl3.g6b"
};

static const uint32_t pp_pa_load_save_pa_gen6[][4] = {
#include "shaders/post_processing/gen5_6/pa_load_save_pa.g6b"
};

static const uint32_t pp_rgbx_load_save_nv12_gen6[][4] = {
#include "shaders/post_processing/gen5_6/rgbx_load_save_nv12.g6b"
};

static const uint32_t pp_nv12_load_save_rgbx_gen6[][4] = {
#include "shaders/post_processing/gen5_6/nv12_load_save_rgbx.g6b"
};

static struct pp_module pp_modules_gen6[] = {
    {
        {
            "NULL module (for testing)",
            PP_NULL,
            pp_null_gen6,
            sizeof(pp_null_gen6),
            NULL,
        },

        pp_null_initialize,
    },

    {
        {
            "NV12_NV12",
            PP_NV12_LOAD_SAVE_N12,
            pp_nv12_load_save_nv12_gen6,
            sizeof(pp_nv12_load_save_nv12_gen6),
            NULL,
        },

        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "NV12_PL3",
            PP_NV12_LOAD_SAVE_PL3,
            pp_nv12_load_save_pl3_gen6,
            sizeof(pp_nv12_load_save_pl3_gen6),
            NULL,
        },
        
        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PL3_NV12",
            PP_PL3_LOAD_SAVE_N12,
            pp_pl3_load_save_nv12_gen6,
            sizeof(pp_pl3_load_save_nv12_gen6),
            NULL,
        },

        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PL3_PL3",
            PP_PL3_LOAD_SAVE_PL3,
            pp_pl3_load_save_pl3_gen6,
            sizeof(pp_pl3_load_save_pl3_gen6),
            NULL,
        },

        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "NV12 Scaling module",
            PP_NV12_SCALING,
            pp_nv12_scaling_gen6,
            sizeof(pp_nv12_scaling_gen6),
            NULL,
        },

        gen6_nv12_scaling_initialize,
    },

    {
        {
            "NV12 AVS module",
            PP_NV12_AVS,
            pp_nv12_avs_gen6,
            sizeof(pp_nv12_avs_gen6),
            NULL,
        },

        pp_nv12_avs_initialize_nlas,
    },

    {
        {
            "NV12 DNDI module",
            PP_NV12_DNDI,
            pp_nv12_dndi_gen6,
            sizeof(pp_nv12_dndi_gen6),
            NULL,
        },

        pp_nv12_dndi_initialize,
    },

    {
        {
            "NV12 DN module",
            PP_NV12_DN,
            pp_nv12_dn_gen6,
            sizeof(pp_nv12_dn_gen6),
            NULL,
        },

        pp_nv12_dn_initialize,
    },
    {
        {
            "NV12_PA module",
            PP_NV12_LOAD_SAVE_PA,
            pp_nv12_load_save_pa_gen6,
            sizeof(pp_nv12_load_save_pa_gen6),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PL3_PA module",
            PP_PL3_LOAD_SAVE_PA,
            pp_pl3_load_save_pa_gen6,
            sizeof(pp_pl3_load_save_pa_gen6),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PA_NV12 module",
            PP_PA_LOAD_SAVE_NV12,
            pp_pa_load_save_nv12_gen6,
            sizeof(pp_pa_load_save_nv12_gen6),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PA_PL3 module",
            PP_PA_LOAD_SAVE_PL3,
            pp_pa_load_save_pl3_gen6,
            sizeof(pp_pa_load_save_pl3_gen6),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "PA_PA module",
            PP_PA_LOAD_SAVE_PA,
            pp_pa_load_save_pa_gen6,
            sizeof(pp_pa_load_save_pa_gen6),
            NULL,
        },

        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "RGBX_NV12 module",
            PP_RGBX_LOAD_SAVE_NV12,
            pp_rgbx_load_save_nv12_gen6,
            sizeof(pp_rgbx_load_save_nv12_gen6),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },

    {
        {
            "NV12_RGBX module",
            PP_NV12_LOAD_SAVE_RGBX,
            pp_nv12_load_save_rgbx_gen6,
            sizeof(pp_nv12_load_save_rgbx_gen6),
            NULL,
        },
    
        pp_plx_load_save_plx_initialize,
    },
};

static const uint32_t pp_null_gen7[][4] = {
};

static const uint32_t pp_nv12_load_save_nv12_gen7[][4] = {
#include "shaders/post_processing/gen7/pl2_to_pl2.g7b"
};

static const uint32_t pp_nv12_load_save_pl3_gen7[][4] = {
#include "shaders/post_processing/gen7/pl2_to_pl3.g7b"
};

static const uint32_t pp_pl3_load_save_nv12_gen7[][4] = {
#include "shaders/post_processing/gen7/pl3_to_pl2.g7b"
};

static const uint32_t pp_pl3_load_save_pl3_gen7[][4] = {
#include "shaders/post_processing/gen7/pl3_to_pl3.g7b"
};

static const uint32_t pp_nv12_scaling_gen7[][4] = {
#include "shaders/post_processing/gen7/avs.g7b"
};

static const uint32_t pp_nv12_avs_gen7[][4] = {
#include "shaders/post_processing/gen7/avs.g7b"
};

static const uint32_t pp_nv12_dndi_gen7[][4] = {
#include "shaders/post_processing/gen7/dndi.g7b"
};

static const uint32_t pp_nv12_dn_gen7[][4] = {
#include "shaders/post_processing/gen7/nv12_dn_nv12.g7b"
};
static const uint32_t pp_nv12_load_save_pa_gen7[][4] = {
#include "shaders/post_processing/gen7/pl2_to_pa.g7b"
};
static const uint32_t pp_pl3_load_save_pa_gen7[][4] = {
#include "shaders/post_processing/gen7/pl3_to_pa.g7b"
};
static const uint32_t pp_pa_load_save_nv12_gen7[][4] = {
#include "shaders/post_processing/gen7/pa_to_pl2.g7b"
};
static const uint32_t pp_pa_load_save_pl3_gen7[][4] = {
#include "shaders/post_processing/gen7/pa_to_pl3.g7b"
};
static const uint32_t pp_pa_load_save_pa_gen7[][4] = {
#include "shaders/post_processing/gen7/pa_to_pa.g7b"
};
static const uint32_t pp_rgbx_load_save_nv12_gen7[][4] = {
#include "shaders/post_processing/gen7/rgbx_to_nv12.g7b"
};
static const uint32_t pp_nv12_load_save_rgbx_gen7[][4] = {
#include "shaders/post_processing/gen7/pl2_to_rgbx.g7b"
};

static VAStatus gen7_pp_plx_avs_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                           const struct i965_surface *src_surface,
                                           const VARectangle *src_rect,
                                           struct i965_surface *dst_surface,
                                           const VARectangle *dst_rect,
                                           void *filter_param);
static VAStatus gen7_pp_nv12_dndi_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                             const struct i965_surface *src_surface,
                                             const VARectangle *src_rect,
                                             struct i965_surface *dst_surface,
                                             const VARectangle *dst_rect,
                                             void *filter_param);
static VAStatus gen7_pp_nv12_dn_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                           const struct i965_surface *src_surface,
                                           const VARectangle *src_rect,
                                           struct i965_surface *dst_surface,
                                           const VARectangle *dst_rect,
                                           void *filter_param);

static struct pp_module pp_modules_gen7[] = {
    {
        {
            "NULL module (for testing)",
            PP_NULL,
            pp_null_gen7,
            sizeof(pp_null_gen7),
            NULL,
        },

        pp_null_initialize,
    },

    {
        {
            "NV12_NV12",
            PP_NV12_LOAD_SAVE_N12,
            pp_nv12_load_save_nv12_gen7,
            sizeof(pp_nv12_load_save_nv12_gen7),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "NV12_PL3",
            PP_NV12_LOAD_SAVE_PL3,
            pp_nv12_load_save_pl3_gen7,
            sizeof(pp_nv12_load_save_pl3_gen7),
            NULL,
        },
        
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PL3_NV12",
            PP_PL3_LOAD_SAVE_N12,
            pp_pl3_load_save_nv12_gen7,
            sizeof(pp_pl3_load_save_nv12_gen7),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PL3_PL3",
            PP_PL3_LOAD_SAVE_PL3,
            pp_pl3_load_save_pl3_gen7,
            sizeof(pp_pl3_load_save_pl3_gen7),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "NV12 Scaling module",
            PP_NV12_SCALING,
            pp_nv12_scaling_gen7,
            sizeof(pp_nv12_scaling_gen7),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "NV12 AVS module",
            PP_NV12_AVS,
            pp_nv12_avs_gen7,
            sizeof(pp_nv12_avs_gen7),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "NV12 DNDI module",
            PP_NV12_DNDI,
            pp_nv12_dndi_gen7,
            sizeof(pp_nv12_dndi_gen7),
            NULL,
        },

        gen7_pp_nv12_dndi_initialize,
    },

    {
        {
            "NV12 DN module",
            PP_NV12_DN,
            pp_nv12_dn_gen7,
            sizeof(pp_nv12_dn_gen7),
            NULL,
        },

        gen7_pp_nv12_dn_initialize,
    },
    {
        {
            "NV12_PA module",
            PP_NV12_LOAD_SAVE_PA,
            pp_nv12_load_save_pa_gen7,
            sizeof(pp_nv12_load_save_pa_gen7),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PL3_PA module",
            PP_PL3_LOAD_SAVE_PA,
            pp_pl3_load_save_pa_gen7,
            sizeof(pp_pl3_load_save_pa_gen7),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PA_NV12 module",
            PP_PA_LOAD_SAVE_NV12,
            pp_pa_load_save_nv12_gen7,
            sizeof(pp_pa_load_save_nv12_gen7),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PA_PL3 module",
            PP_PA_LOAD_SAVE_PL3,
            pp_pa_load_save_pl3_gen7,
            sizeof(pp_pa_load_save_pl3_gen7),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PA_PA module",
            PP_PA_LOAD_SAVE_PA,
            pp_pa_load_save_pa_gen7,
            sizeof(pp_pa_load_save_pa_gen7),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "RGBX_NV12 module",
            PP_RGBX_LOAD_SAVE_NV12,
            pp_rgbx_load_save_nv12_gen7,
            sizeof(pp_rgbx_load_save_nv12_gen7),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "NV12_RGBX module",
            PP_NV12_LOAD_SAVE_RGBX,
            pp_nv12_load_save_rgbx_gen7,
            sizeof(pp_nv12_load_save_rgbx_gen7),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },
            
};

static const uint32_t pp_null_gen75[][4] = {
};

static const uint32_t pp_nv12_load_save_nv12_gen75[][4] = {
#include "shaders/post_processing/gen7/pl2_to_pl2.g75b"
};

static const uint32_t pp_nv12_load_save_pl3_gen75[][4] = {
#include "shaders/post_processing/gen7/pl2_to_pl3.g75b"
};

static const uint32_t pp_pl3_load_save_nv12_gen75[][4] = {
#include "shaders/post_processing/gen7/pl3_to_pl2.g75b"
};

static const uint32_t pp_pl3_load_save_pl3_gen75[][4] = {
#include "shaders/post_processing/gen7/pl3_to_pl3.g75b"
};

static const uint32_t pp_nv12_scaling_gen75[][4] = {
#include "shaders/post_processing/gen7/avs.g75b"
};

static const uint32_t pp_nv12_avs_gen75[][4] = {
#include "shaders/post_processing/gen7/avs.g75b"
};

static const uint32_t pp_nv12_dndi_gen75[][4] = {
// #include "shaders/post_processing/gen7/dndi.g75b"
};

static const uint32_t pp_nv12_dn_gen75[][4] = {
// #include "shaders/post_processing/gen7/nv12_dn_nv12.g75b"
};
static const uint32_t pp_nv12_load_save_pa_gen75[][4] = {
#include "shaders/post_processing/gen7/pl2_to_pa.g75b"
};
static const uint32_t pp_pl3_load_save_pa_gen75[][4] = {
#include "shaders/post_processing/gen7/pl3_to_pa.g75b"
};
static const uint32_t pp_pa_load_save_nv12_gen75[][4] = {
#include "shaders/post_processing/gen7/pa_to_pl2.g75b"
};
static const uint32_t pp_pa_load_save_pl3_gen75[][4] = {
#include "shaders/post_processing/gen7/pa_to_pl3.g75b"
};
static const uint32_t pp_pa_load_save_pa_gen75[][4] = {
#include "shaders/post_processing/gen7/pa_to_pa.g75b"
};
static const uint32_t pp_rgbx_load_save_nv12_gen75[][4] = {
#include "shaders/post_processing/gen7/rgbx_to_nv12.g75b"
};
static const uint32_t pp_nv12_load_save_rgbx_gen75[][4] = {
#include "shaders/post_processing/gen7/pl2_to_rgbx.g75b"
};

static struct pp_module pp_modules_gen75[] = {
    {
        {
            "NULL module (for testing)",
            PP_NULL,
            pp_null_gen75,
            sizeof(pp_null_gen75),
            NULL,
        },

        pp_null_initialize,
    },

    {
        {
            "NV12_NV12",
            PP_NV12_LOAD_SAVE_N12,
            pp_nv12_load_save_nv12_gen75,
            sizeof(pp_nv12_load_save_nv12_gen75),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "NV12_PL3",
            PP_NV12_LOAD_SAVE_PL3,
            pp_nv12_load_save_pl3_gen75,
            sizeof(pp_nv12_load_save_pl3_gen75),
            NULL,
        },
        
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PL3_NV12",
            PP_PL3_LOAD_SAVE_N12,
            pp_pl3_load_save_nv12_gen75,
            sizeof(pp_pl3_load_save_nv12_gen75),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PL3_PL3",
            PP_PL3_LOAD_SAVE_PL3,
            pp_pl3_load_save_pl3_gen75,
            sizeof(pp_pl3_load_save_pl3_gen75),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "NV12 Scaling module",
            PP_NV12_SCALING,
            pp_nv12_scaling_gen75,
            sizeof(pp_nv12_scaling_gen75),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "NV12 AVS module",
            PP_NV12_AVS,
            pp_nv12_avs_gen75,
            sizeof(pp_nv12_avs_gen75),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "NV12 DNDI module",
            PP_NV12_DNDI,
            pp_nv12_dndi_gen75,
            sizeof(pp_nv12_dndi_gen75),
            NULL,
        },

        gen7_pp_nv12_dn_initialize,
    },

    {
        {
            "NV12 DN module",
            PP_NV12_DN,
            pp_nv12_dn_gen75,
            sizeof(pp_nv12_dn_gen75),
            NULL,
        },

        gen7_pp_nv12_dn_initialize,
    },

    {
        {
            "NV12_PA module",
            PP_NV12_LOAD_SAVE_PA,
            pp_nv12_load_save_pa_gen75,
            sizeof(pp_nv12_load_save_pa_gen75),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PL3_PA module",
            PP_PL3_LOAD_SAVE_PA,
            pp_pl3_load_save_pa_gen75,
            sizeof(pp_pl3_load_save_pa_gen75),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PA_NV12 module",
            PP_PA_LOAD_SAVE_NV12,
            pp_pa_load_save_nv12_gen75,
            sizeof(pp_pa_load_save_nv12_gen75),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PA_PL3 module",
            PP_PA_LOAD_SAVE_PL3,
            pp_pa_load_save_pl3_gen75,
            sizeof(pp_pa_load_save_pl3_gen75),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "PA_PA module",
            PP_PA_LOAD_SAVE_PA,
            pp_pa_load_save_pa_gen75,
            sizeof(pp_pa_load_save_pa_gen75),
            NULL,
        },

        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "RGBX_NV12 module",
            PP_RGBX_LOAD_SAVE_NV12,
            pp_rgbx_load_save_nv12_gen75,
            sizeof(pp_rgbx_load_save_nv12_gen75),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },

    {
        {
            "NV12_RGBX module",
            PP_NV12_LOAD_SAVE_RGBX,
            pp_nv12_load_save_rgbx_gen75,
            sizeof(pp_nv12_load_save_rgbx_gen75),
            NULL,
        },
    
        gen7_pp_plx_avs_initialize,
    },
            
};

static int
pp_get_surface_fourcc(VADriverContextP ctx, const struct i965_surface *surface)
{
    int fourcc;

    if (surface->type == I965_SURFACE_TYPE_IMAGE) {
        struct object_image *obj_image = (struct object_image *)surface->base;
        fourcc = obj_image->image.format.fourcc;
    } else {
        struct object_surface *obj_surface = (struct object_surface *)surface->base;
        fourcc = obj_surface->fourcc;
    }

    return fourcc;
}

static void
pp_get_surface_size(VADriverContextP ctx, const struct i965_surface *surface, int *width, int *height)
{
    if (surface->type == I965_SURFACE_TYPE_IMAGE) {
        struct object_image *obj_image = (struct object_image *)surface->base;

        *width = obj_image->image.width;
        *height = obj_image->image.height;
    } else {
        struct object_surface *obj_surface = (struct object_surface *)surface->base;

        *width = obj_surface->orig_width;
        *height = obj_surface->orig_height;
    }
}

static void
pp_set_surface_tiling(struct i965_surface_state *ss, unsigned int tiling)
{
    switch (tiling) {
    case I915_TILING_NONE:
        ss->ss3.tiled_surface = 0;
        ss->ss3.tile_walk = 0;
        break;
    case I915_TILING_X:
        ss->ss3.tiled_surface = 1;
        ss->ss3.tile_walk = I965_TILEWALK_XMAJOR;
        break;
    case I915_TILING_Y:
        ss->ss3.tiled_surface = 1;
        ss->ss3.tile_walk = I965_TILEWALK_YMAJOR;
        break;
    }
}

static void
pp_set_surface2_tiling(struct i965_surface_state2 *ss, unsigned int tiling)
{
    switch (tiling) {
    case I915_TILING_NONE:
        ss->ss2.tiled_surface = 0;
        ss->ss2.tile_walk = 0;
        break;
    case I915_TILING_X:
        ss->ss2.tiled_surface = 1;
        ss->ss2.tile_walk = I965_TILEWALK_XMAJOR;
        break;
    case I915_TILING_Y:
        ss->ss2.tiled_surface = 1;
        ss->ss2.tile_walk = I965_TILEWALK_YMAJOR;
        break;
    }
}

static void
gen7_pp_set_surface_tiling(struct gen7_surface_state *ss, unsigned int tiling)
{
    switch (tiling) {
    case I915_TILING_NONE:
        ss->ss0.tiled_surface = 0;
        ss->ss0.tile_walk = 0;
        break;
    case I915_TILING_X:
        ss->ss0.tiled_surface = 1;
        ss->ss0.tile_walk = I965_TILEWALK_XMAJOR;
        break;
    case I915_TILING_Y:
        ss->ss0.tiled_surface = 1;
        ss->ss0.tile_walk = I965_TILEWALK_YMAJOR;
        break;
    }
}

static void
gen7_pp_set_surface2_tiling(struct gen7_surface_state2 *ss, unsigned int tiling)
{
    switch (tiling) {
    case I915_TILING_NONE:
        ss->ss2.tiled_surface = 0;
        ss->ss2.tile_walk = 0;
        break;
    case I915_TILING_X:
        ss->ss2.tiled_surface = 1;
        ss->ss2.tile_walk = I965_TILEWALK_XMAJOR;
        break;
    case I915_TILING_Y:
        ss->ss2.tiled_surface = 1;
        ss->ss2.tile_walk = I965_TILEWALK_YMAJOR;
        break;
    }
}

static void
ironlake_pp_interface_descriptor_table(struct i965_post_processing_context *pp_context)
{
    struct i965_interface_descriptor *desc;
    dri_bo *bo;
    int pp_index = pp_context->current_pp;

    bo = pp_context->idrt.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);
    desc = bo->virtual;
    memset(desc, 0, sizeof(*desc));
    desc->desc0.grf_reg_blocks = 10;
    desc->desc0.kernel_start_pointer = pp_context->pp_modules[pp_index].kernel.bo->offset >> 6; /* reloc */
    desc->desc1.const_urb_entry_read_offset = 0;
    desc->desc1.const_urb_entry_read_len = 4; /* grf 1-4 */
    desc->desc2.sampler_state_pointer = pp_context->sampler_state_table.bo->offset >> 5;
    desc->desc2.sampler_count = 0;
    desc->desc3.binding_table_entry_count = 0;
    desc->desc3.binding_table_pointer = (BINDING_TABLE_OFFSET >> 5);

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      desc->desc0.grf_reg_blocks,
                      offsetof(struct i965_interface_descriptor, desc0),
                      pp_context->pp_modules[pp_index].kernel.bo);

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      desc->desc2.sampler_count << 2,
                      offsetof(struct i965_interface_descriptor, desc2),
                      pp_context->sampler_state_table.bo);

    dri_bo_unmap(bo);
    pp_context->idrt.num_interface_descriptors++;
}

static void
ironlake_pp_vfe_state(struct i965_post_processing_context *pp_context)
{
    struct i965_vfe_state *vfe_state;
    dri_bo *bo;

    bo = pp_context->vfe_state.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);
    vfe_state = bo->virtual;
    memset(vfe_state, 0, sizeof(*vfe_state));
    vfe_state->vfe1.max_threads = pp_context->urb.num_vfe_entries - 1;
    vfe_state->vfe1.urb_entry_alloc_size = pp_context->urb.size_vfe_entry - 1;
    vfe_state->vfe1.num_urb_entries = pp_context->urb.num_vfe_entries;
    vfe_state->vfe1.vfe_mode = VFE_GENERIC_MODE;
    vfe_state->vfe1.children_present = 0;
    vfe_state->vfe2.interface_descriptor_base = 
        pp_context->idrt.bo->offset >> 4; /* reloc */
    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      0,
                      offsetof(struct i965_vfe_state, vfe2),
                      pp_context->idrt.bo);
    dri_bo_unmap(bo);
}

static void
ironlake_pp_upload_constants(struct i965_post_processing_context *pp_context)
{
    unsigned char *constant_buffer;
    struct pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;

    assert(sizeof(*pp_static_parameter) == 128);
    dri_bo_map(pp_context->curbe.bo, 1);
    assert(pp_context->curbe.bo->virtual);
    constant_buffer = pp_context->curbe.bo->virtual;
    memcpy(constant_buffer, pp_static_parameter, sizeof(*pp_static_parameter));
    dri_bo_unmap(pp_context->curbe.bo);
}

static void
ironlake_pp_states_setup(VADriverContextP ctx,
                         struct i965_post_processing_context *pp_context)
{
    ironlake_pp_interface_descriptor_table(pp_context);
    ironlake_pp_vfe_state(pp_context);
    ironlake_pp_upload_constants(pp_context);
}

static void
ironlake_pp_pipeline_select(VADriverContextP ctx,
                            struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 1);
    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_MEDIA);
    ADVANCE_BATCH(batch);
}

static void
ironlake_pp_urb_layout(VADriverContextP ctx,
                       struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;
    unsigned int vfe_fence, cs_fence;

    vfe_fence = pp_context->urb.cs_start;
    cs_fence = pp_context->urb.size;

    BEGIN_BATCH(batch, 3);
    OUT_BATCH(batch, CMD_URB_FENCE | UF0_VFE_REALLOC | UF0_CS_REALLOC | 1);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 
              (vfe_fence << UF2_VFE_FENCE_SHIFT) |      /* VFE_SIZE */
              (cs_fence << UF2_CS_FENCE_SHIFT));        /* CS_SIZE */
    ADVANCE_BATCH(batch);
}

static void
ironlake_pp_state_base_address(VADriverContextP ctx,
                               struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 8);
    OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | 6);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_RELOC(batch, pp_context->surface_state_binding_table.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    ADVANCE_BATCH(batch);
}

static void
ironlake_pp_state_pointers(VADriverContextP ctx,
                           struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 3);
    OUT_BATCH(batch, CMD_MEDIA_STATE_POINTERS | 1);
    OUT_BATCH(batch, 0);
    OUT_RELOC(batch, pp_context->vfe_state.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
    ADVANCE_BATCH(batch);
}

static void 
ironlake_pp_cs_urb_layout(VADriverContextP ctx,
                          struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, CMD_CS_URB_STATE | 0);
    OUT_BATCH(batch,
              ((pp_context->urb.size_cs_entry - 1) << 4) |     /* URB Entry Allocation Size */
              (pp_context->urb.num_cs_entries << 0));          /* Number of URB Entries */
    ADVANCE_BATCH(batch);
}

static void
ironlake_pp_constant_buffer(VADriverContextP ctx,
                            struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, CMD_CONSTANT_BUFFER | (1 << 8) | (2 - 2));
    OUT_RELOC(batch, pp_context->curbe.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              pp_context->urb.size_cs_entry - 1);
    ADVANCE_BATCH(batch);    
}

static void
ironlake_pp_object_walker(VADriverContextP ctx,
                          struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;
    int x, x_steps, y, y_steps;
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;

    x_steps = pp_context->pp_x_steps(pp_context->private_context);
    y_steps = pp_context->pp_y_steps(pp_context->private_context);

    for (y = 0; y < y_steps; y++) {
        for (x = 0; x < x_steps; x++) {
            if (!pp_context->pp_set_block_parameter(pp_context, x, y)) {
                BEGIN_BATCH(batch, 20);
                OUT_BATCH(batch, CMD_MEDIA_OBJECT | 18);
                OUT_BATCH(batch, 0);
                OUT_BATCH(batch, 0); /* no indirect data */
                OUT_BATCH(batch, 0);

                /* inline data grf 5-6 */
                assert(sizeof(*pp_inline_parameter) == 64);
                intel_batchbuffer_data(batch, pp_inline_parameter, sizeof(*pp_inline_parameter));

                ADVANCE_BATCH(batch);
            }
        }
    }
}

static void
ironlake_pp_pipeline_setup(VADriverContextP ctx,
                           struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    intel_batchbuffer_start_atomic(batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(batch);
    ironlake_pp_pipeline_select(ctx, pp_context);
    ironlake_pp_state_base_address(ctx, pp_context);
    ironlake_pp_state_pointers(ctx, pp_context);
    ironlake_pp_urb_layout(ctx, pp_context);
    ironlake_pp_cs_urb_layout(ctx, pp_context);
    ironlake_pp_constant_buffer(ctx, pp_context);
    ironlake_pp_object_walker(ctx, pp_context);
    intel_batchbuffer_end_atomic(batch);
}

// update u/v offset when the surface format are packed yuv
static void i965_update_src_surface_static_parameter(
    VADriverContextP    ctx, 
    struct i965_post_processing_context *pp_context,
    const struct i965_surface *surface)
{
    struct pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    int fourcc = pp_get_surface_fourcc(ctx, surface);

    switch (fourcc) {
    case VA_FOURCC_YUY2:
        pp_static_parameter->grf1.source_packed_u_offset = 1;
        pp_static_parameter->grf1.source_packed_v_offset = 3;
        break;
    case VA_FOURCC_UYVY:
        pp_static_parameter->grf1.source_packed_y_offset = 1;
        pp_static_parameter->grf1.source_packed_v_offset = 2;
        break;
    case VA_FOURCC_BGRX:
    case VA_FOURCC_BGRA:
        pp_static_parameter->grf1.source_rgb_layout = 0;
        break;
    case VA_FOURCC_RGBX:
    case VA_FOURCC_RGBA:
        pp_static_parameter->grf1.source_rgb_layout = 1;
        break;
    default:
        break;
    }
    
}

static void i965_update_dst_surface_static_parameter(
    VADriverContextP    ctx, 
    struct i965_post_processing_context *pp_context,
    const struct i965_surface *surface)
{
    struct pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    int fourcc = pp_get_surface_fourcc(ctx, surface);

    switch (fourcc) {
    case VA_FOURCC_YUY2:
        pp_static_parameter->grf1.r1_2.load_and_save.destination_packed_u_offset = 1;
        pp_static_parameter->grf1.r1_2.load_and_save.destination_packed_v_offset = 3;
        break;
    case VA_FOURCC_UYVY:
        pp_static_parameter->grf1.r1_2.load_and_save.destination_packed_y_offset = 1;
        pp_static_parameter->grf1.r1_2.load_and_save.destination_packed_v_offset = 2;
        break;
    case VA_FOURCC_BGRX:
    case VA_FOURCC_BGRA:
        pp_static_parameter->grf1.r1_2.csc.destination_rgb_layout = 0;
        break;
    case VA_FOURCC_RGBX:
    case VA_FOURCC_RGBA:
        pp_static_parameter->grf1.r1_2.csc.destination_rgb_layout = 1;
        break;
    default:
        break;
    }
    
}

static void
i965_pp_set_surface_state(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                          dri_bo *surf_bo, unsigned long surf_bo_offset,
                          int width, int height, int pitch, int format, 
                          int index, int is_target)
{
    struct i965_surface_state *ss;
    dri_bo *ss_bo;
    unsigned int tiling;
    unsigned int swizzle;

    dri_bo_get_tiling(surf_bo, &tiling, &swizzle);
    ss_bo = pp_context->surface_state_binding_table.bo;
    assert(ss_bo);

    dri_bo_map(ss_bo, True);
    assert(ss_bo->virtual);
    ss = (struct i965_surface_state *)((char *)ss_bo->virtual + SURFACE_STATE_OFFSET(index));
    memset(ss, 0, sizeof(*ss));
    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = format;
    ss->ss1.base_addr = surf_bo->offset + surf_bo_offset;
    ss->ss2.width = width - 1;
    ss->ss2.height = height - 1;
    ss->ss3.pitch = pitch - 1;
    pp_set_surface_tiling(ss, tiling);
    dri_bo_emit_reloc(ss_bo,
                      I915_GEM_DOMAIN_RENDER, is_target ? I915_GEM_DOMAIN_RENDER : 0,
                      surf_bo_offset,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct i965_surface_state, ss1),
                      surf_bo);
    ((unsigned int *)((char *)ss_bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(ss_bo);
}

static void
i965_pp_set_surface2_state(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                           dri_bo *surf_bo, unsigned long surf_bo_offset,
                           int width, int height, int wpitch,
                           int xoffset, int yoffset,
                           int format, int interleave_chroma,
                           int index)
{
    struct i965_surface_state2 *ss2;
    dri_bo *ss2_bo;
    unsigned int tiling;
    unsigned int swizzle;

    dri_bo_get_tiling(surf_bo, &tiling, &swizzle);
    ss2_bo = pp_context->surface_state_binding_table.bo;
    assert(ss2_bo);

    dri_bo_map(ss2_bo, True);
    assert(ss2_bo->virtual);
    ss2 = (struct i965_surface_state2 *)((char *)ss2_bo->virtual + SURFACE_STATE_OFFSET(index));
    memset(ss2, 0, sizeof(*ss2));
    ss2->ss0.surface_base_address = surf_bo->offset + surf_bo_offset;
    ss2->ss1.cbcr_pixel_offset_v_direction = 0;
    ss2->ss1.width = width - 1;
    ss2->ss1.height = height - 1;
    ss2->ss2.pitch = wpitch - 1;
    ss2->ss2.interleave_chroma = interleave_chroma;
    ss2->ss2.surface_format = format;
    ss2->ss3.x_offset_for_cb = xoffset;
    ss2->ss3.y_offset_for_cb = yoffset;
    pp_set_surface2_tiling(ss2, tiling);
    dri_bo_emit_reloc(ss2_bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      surf_bo_offset,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct i965_surface_state2, ss0),
                      surf_bo);
    ((unsigned int *)((char *)ss2_bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(ss2_bo);
}

static void
gen7_pp_set_surface_state(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                          dri_bo *surf_bo, unsigned long surf_bo_offset,
                          int width, int height, int pitch, int format, 
                          int index, int is_target)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);  
    struct gen7_surface_state *ss;
    dri_bo *ss_bo;
    unsigned int tiling;
    unsigned int swizzle;

    dri_bo_get_tiling(surf_bo, &tiling, &swizzle);
    ss_bo = pp_context->surface_state_binding_table.bo;
    assert(ss_bo);

    dri_bo_map(ss_bo, True);
    assert(ss_bo->virtual);
    ss = (struct gen7_surface_state *)((char *)ss_bo->virtual + SURFACE_STATE_OFFSET(index));
    memset(ss, 0, sizeof(*ss));
    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = format;
    ss->ss1.base_addr = surf_bo->offset + surf_bo_offset;
    ss->ss2.width = width - 1;
    ss->ss2.height = height - 1;
    ss->ss3.pitch = pitch - 1;
    gen7_pp_set_surface_tiling(ss, tiling);
    if (IS_HASWELL(i965->intel.device_info))
        gen7_render_set_surface_scs(ss);
    dri_bo_emit_reloc(ss_bo,
                      I915_GEM_DOMAIN_RENDER, is_target ? I915_GEM_DOMAIN_RENDER : 0,
                      surf_bo_offset,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct gen7_surface_state, ss1),
                      surf_bo);
    ((unsigned int *)((char *)ss_bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(ss_bo);
}

static void
gen7_pp_set_surface2_state(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                           dri_bo *surf_bo, unsigned long surf_bo_offset,
                           int width, int height, int wpitch,
                           int xoffset, int yoffset,
                           int format, int interleave_chroma,
                           int index)
{
    struct gen7_surface_state2 *ss2;
    dri_bo *ss2_bo;
    unsigned int tiling;
    unsigned int swizzle;

    dri_bo_get_tiling(surf_bo, &tiling, &swizzle);
    ss2_bo = pp_context->surface_state_binding_table.bo;
    assert(ss2_bo);

    dri_bo_map(ss2_bo, True);
    assert(ss2_bo->virtual);
    ss2 = (struct gen7_surface_state2 *)((char *)ss2_bo->virtual + SURFACE_STATE_OFFSET(index));
    memset(ss2, 0, sizeof(*ss2));
    ss2->ss0.surface_base_address = surf_bo->offset + surf_bo_offset;
    ss2->ss1.cbcr_pixel_offset_v_direction = 0;
    ss2->ss1.width = width - 1;
    ss2->ss1.height = height - 1;
    ss2->ss2.pitch = wpitch - 1;
    ss2->ss2.interleave_chroma = interleave_chroma;
    ss2->ss2.surface_format = format;
    ss2->ss3.x_offset_for_cb = xoffset;
    ss2->ss3.y_offset_for_cb = yoffset;
    gen7_pp_set_surface2_tiling(ss2, tiling);
    dri_bo_emit_reloc(ss2_bo,
                      I915_GEM_DOMAIN_RENDER, 0,
                      surf_bo_offset,
                      SURFACE_STATE_OFFSET(index) + offsetof(struct gen7_surface_state2, ss0),
                      surf_bo);
    ((unsigned int *)((char *)ss2_bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(ss2_bo);
}

static void 
pp_set_media_rw_message_surface(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                const struct i965_surface *surface, 
                                int base_index, int is_target,
                                int *width, int *height, int *pitch, int *offset)
{
    struct object_surface *obj_surface;
    struct object_image *obj_image;
    dri_bo *bo;
    int fourcc = pp_get_surface_fourcc(ctx, surface);
    const int Y = 0;
    const int U = ((fourcc == VA_FOURCC_YV12) ||
                   (fourcc == VA_FOURCC_YV16))
                   ? 2 : 1;
    const int V = ((fourcc == VA_FOURCC_YV12) ||
                   (fourcc == VA_FOURCC_YV16))
                   ? 1 : 2;
    const int UV = 1;
    int interleaved_uv = fourcc == VA_FOURCC_NV12;
    int packed_yuv = (fourcc == VA_FOURCC_YUY2 || fourcc == VA_FOURCC_UYVY);
    int full_packed_format = (fourcc == VA_FOURCC_RGBA ||
                              fourcc == VA_FOURCC_RGBX ||
                              fourcc == VA_FOURCC_BGRA ||
                              fourcc == VA_FOURCC_BGRX);
    int scale_factor_of_1st_plane_width_in_byte = 1;
                              
    if (surface->type == I965_SURFACE_TYPE_SURFACE) {
        obj_surface = (struct object_surface *)surface->base;
        bo = obj_surface->bo;
        width[0] = obj_surface->orig_width;
        height[0] = obj_surface->orig_height;
        pitch[0] = obj_surface->width;
        offset[0] = 0;

        if (full_packed_format) {
            scale_factor_of_1st_plane_width_in_byte = 4; 
        }
        else if (packed_yuv ) {
            scale_factor_of_1st_plane_width_in_byte =  2; 
        }
        else if (interleaved_uv) {
            width[1] = obj_surface->orig_width;
            height[1] = obj_surface->orig_height / 2;
            pitch[1] = obj_surface->width;
            offset[1] = offset[0] + obj_surface->width * obj_surface->height;
        } else {
            width[1] = obj_surface->orig_width / 2;
            height[1] = obj_surface->orig_height / 2;
            pitch[1] = obj_surface->width / 2;
            offset[1] = offset[0] + obj_surface->width * obj_surface->height;
            width[2] = obj_surface->orig_width / 2;
            height[2] = obj_surface->orig_height / 2;
            pitch[2] = obj_surface->width / 2;
            offset[2] = offset[1] + (obj_surface->width / 2) * (obj_surface->height / 2);
        }
    } else {
        obj_image = (struct object_image *)surface->base;
        bo = obj_image->bo;
        width[0] = obj_image->image.width;
        height[0] = obj_image->image.height;
        pitch[0] = obj_image->image.pitches[0];
        offset[0] = obj_image->image.offsets[0];

        if (full_packed_format) {
            scale_factor_of_1st_plane_width_in_byte = 4;
        }
        else if (packed_yuv ) {
            scale_factor_of_1st_plane_width_in_byte = 2;
        }
        else if (interleaved_uv) {
            width[1] = obj_image->image.width;
            height[1] = obj_image->image.height / 2;
            pitch[1] = obj_image->image.pitches[1];
            offset[1] = obj_image->image.offsets[1];
        } else {
            width[1] = obj_image->image.width / 2;
            height[1] = obj_image->image.height / 2;
            pitch[1] = obj_image->image.pitches[1];
            offset[1] = obj_image->image.offsets[1];
            width[2] = obj_image->image.width / 2;
            height[2] = obj_image->image.height / 2;
            pitch[2] = obj_image->image.pitches[2];
            offset[2] = obj_image->image.offsets[2];
            if (fourcc == VA_FOURCC_YV16) {
                width[1] = obj_image->image.width / 2;
                height[1] = obj_image->image.height;
                width[2] = obj_image->image.width / 2;
                height[2] = obj_image->image.height;
            }
        }
    }

    /* Y surface */
    i965_pp_set_surface_state(ctx, pp_context,
                              bo, offset[Y],
                              width[Y] *scale_factor_of_1st_plane_width_in_byte / 4, height[Y], pitch[Y], I965_SURFACEFORMAT_R8_UNORM,
                              base_index, is_target);

    if (!packed_yuv && !full_packed_format) {
        if (interleaved_uv) {
            i965_pp_set_surface_state(ctx, pp_context,
                                      bo, offset[UV],
                                      width[UV] / 4, height[UV], pitch[UV], I965_SURFACEFORMAT_R8_UNORM,
                                      base_index + 1, is_target);
        } else {
            /* U surface */
            i965_pp_set_surface_state(ctx, pp_context,
                                      bo, offset[U],
                                      width[U] / 4, height[U], pitch[U], I965_SURFACEFORMAT_R8_UNORM,
                                      base_index + 1, is_target);

            /* V surface */
            i965_pp_set_surface_state(ctx, pp_context,
                                      bo, offset[V],
                                      width[V] / 4, height[V], pitch[V], I965_SURFACEFORMAT_R8_UNORM,
                                      base_index + 2, is_target);
        }
    }

}

static void 
gen7_pp_set_media_rw_message_surface(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                     const struct i965_surface *surface, 
                                     int base_index, int is_target,
                                     const VARectangle *rect,
                                     int *width, int *height, int *pitch, int *offset)
{
    struct object_surface *obj_surface;
    struct object_image *obj_image;
    dri_bo *bo;
    int fourcc = pp_get_surface_fourcc(ctx, surface);
    const i965_fourcc_info *fourcc_info = get_fourcc_info(fourcc);

    if (fourcc_info == NULL)
        return;

    if (surface->type == I965_SURFACE_TYPE_SURFACE) {
        obj_surface = (struct object_surface *)surface->base;
        bo = obj_surface->bo;
        width[0] = MIN(rect->x + rect->width, obj_surface->orig_width);
        height[0] = MIN(rect->y + rect->height, obj_surface->orig_height);
        pitch[0] = obj_surface->width;
        offset[0] = 0;

        if (fourcc_info->num_planes == 1 && is_target)
            width[0] = width[0] * (fourcc_info->bpp[0] / 8); /* surface format is R8 */

        width[1] = MIN(rect->x / fourcc_info->hfactor + rect->width / fourcc_info->hfactor, obj_surface->cb_cr_width);
        height[1] = MIN(rect->y / fourcc_info->vfactor + rect->height / fourcc_info->vfactor, obj_surface->cb_cr_height);
        pitch[1] = obj_surface->cb_cr_pitch;
        offset[1] = obj_surface->y_cb_offset * obj_surface->width;

        width[2] = MIN(rect->x / fourcc_info->hfactor + rect->width / fourcc_info->hfactor, obj_surface->cb_cr_width);
        height[2] = MIN(rect->y / fourcc_info->vfactor + rect->height / fourcc_info->vfactor, obj_surface->cb_cr_height);
        pitch[2] = obj_surface->cb_cr_pitch;
        offset[2] = obj_surface->y_cr_offset * obj_surface->width;
    } else {
        int U = 0, V = 0;

        /* FIXME: add support for ARGB/ABGR image */
        obj_image = (struct object_image *)surface->base;
        bo = obj_image->bo;
        width[0] = MIN(rect->x + rect->width, obj_image->image.width);
        height[0] = MIN(rect->y + rect->height, obj_image->image.height);
        pitch[0] = obj_image->image.pitches[0];
        offset[0] = obj_image->image.offsets[0];

        if (fourcc_info->num_planes == 1) {
            if (is_target)
                width[0] = width[0] * (fourcc_info->bpp[0] / 8); /* surface format is R8 */
        } else if (fourcc_info->num_planes == 2) {
            U = 1, V = 1;
        } else {
            assert(fourcc_info->num_components == 3);

            U = fourcc_info->components[1].plane;
            V = fourcc_info->components[2].plane;
            assert((U == 1 && V == 2) ||
                   (U == 2 && V == 1));
        }

        /* Always set width/height although they aren't used for fourcc_info->num_planes == 1 */
        width[1] = MIN(rect->x / fourcc_info->hfactor + rect->width / fourcc_info->hfactor, obj_image->image.width / fourcc_info->hfactor);
        height[1] = MIN(rect->y / fourcc_info->vfactor + rect->height / fourcc_info->vfactor, obj_image->image.height / fourcc_info->vfactor);
        pitch[1] = obj_image->image.pitches[U];
        offset[1] = obj_image->image.offsets[U];

        width[2] = MIN(rect->x / fourcc_info->hfactor + rect->width / fourcc_info->hfactor, obj_image->image.width / fourcc_info->hfactor);
        height[2] = MIN(rect->y / fourcc_info->vfactor + rect->height / fourcc_info->vfactor, obj_image->image.height / fourcc_info->vfactor);
        pitch[2] = obj_image->image.pitches[V];
        offset[2] = obj_image->image.offsets[V];
    }

    if (is_target) {
        gen7_pp_set_surface_state(ctx, pp_context,
                                  bo, 0,
                                  width[0] / 4, height[0], pitch[0],
                                  I965_SURFACEFORMAT_R8_UINT,
                                  base_index, 1);

        if (fourcc_info->num_planes == 2) {
            gen7_pp_set_surface_state(ctx, pp_context,
                                      bo, offset[1],
                                      width[1] / 2, height[1], pitch[1],
                                      I965_SURFACEFORMAT_R8G8_SINT,
                                      base_index + 1, 1);
        } else if (fourcc_info->num_planes == 3) {
            gen7_pp_set_surface_state(ctx, pp_context,
                                      bo, offset[1],
                                      width[1] / 4, height[1], pitch[1],
                                      I965_SURFACEFORMAT_R8_SINT,
                                      base_index + 1, 1);
            gen7_pp_set_surface_state(ctx, pp_context,
                                      bo, offset[2],
                                      width[2] / 4, height[2], pitch[2],
                                      I965_SURFACEFORMAT_R8_SINT,
                                      base_index + 2, 1);
        }

        if (fourcc_info->format == I965_COLOR_RGB) {
            struct gen7_pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
            /* the format is MSB: X-B-G-R */
            pp_static_parameter->grf2.save_avs_rgb_swap = 0;
            if ((fourcc == VA_FOURCC_BGRA) ||
                (fourcc == VA_FOURCC_BGRX)) {
                /* It is stored as MSB: X-R-G-B */
                pp_static_parameter->grf2.save_avs_rgb_swap = 1;
            }
        }
    } else {
        int format0 = SURFACE_FORMAT_Y8_UNORM;

        switch (fourcc) {
        case VA_FOURCC_YUY2:
            format0 = SURFACE_FORMAT_YCRCB_NORMAL;
            break;

        case VA_FOURCC_UYVY:
            format0 = SURFACE_FORMAT_YCRCB_SWAPY;
            break;

        default:
            break;
        }

	if (fourcc_info->format == I965_COLOR_RGB) {
    	    struct gen7_pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
	    /* Only R8G8B8A8_UNORM is supported for BGRX or RGBX */
	    format0 = SURFACE_FORMAT_R8G8B8A8_UNORM;
	    pp_static_parameter->grf2.src_avs_rgb_swap = 0;
	    if ((fourcc == VA_FOURCC_BGRA) ||
                (fourcc == VA_FOURCC_BGRX)) {
		pp_static_parameter->grf2.src_avs_rgb_swap = 1;
	    }
	}

        gen7_pp_set_surface2_state(ctx, pp_context,
                                   bo, offset[0],
                                   width[0], height[0], pitch[0],
                                   0, 0,
                                   format0, 0,
                                   base_index);

        if (fourcc_info->num_planes == 2) {
            gen7_pp_set_surface2_state(ctx, pp_context,
                                       bo, offset[1],
                                       width[1], height[1], pitch[1],
                                       0, 0,
                                       SURFACE_FORMAT_R8B8_UNORM, 0,
                                       base_index + 1);
        } else if (fourcc_info->num_planes == 3) {
            gen7_pp_set_surface2_state(ctx, pp_context,
                                       bo, offset[1],
                                       width[1], height[1], pitch[1],
                                       0, 0,
                                       SURFACE_FORMAT_R8_UNORM, 0,
                                       base_index + 1);
            gen7_pp_set_surface2_state(ctx, pp_context,
                                       bo, offset[2],
                                       width[2], height[2], pitch[2],
                                       0, 0,
                                       SURFACE_FORMAT_R8_UNORM, 0,
                                       base_index + 2);
        }
    }
}

static int
pp_null_x_steps(void *private_context)
{
    return 1;
}

static int
pp_null_y_steps(void *private_context)
{
    return 1;
}

static int
pp_null_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    return 0;
}

static VAStatus
pp_null_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                   const struct i965_surface *src_surface,
                   const VARectangle *src_rect,
                   struct i965_surface *dst_surface,
                   const VARectangle *dst_rect,
                   void *filter_param)
{
    /* private function & data */
    pp_context->pp_x_steps = pp_null_x_steps;
    pp_context->pp_y_steps = pp_null_y_steps;
    pp_context->private_context = NULL;
    pp_context->pp_set_block_parameter = pp_null_set_block_parameter;

    dst_surface->flags = src_surface->flags;

    return VA_STATUS_SUCCESS;
}

static int
pp_load_save_x_steps(void *private_context)
{
    return 1;
}

static int
pp_load_save_y_steps(void *private_context)
{
    struct pp_load_save_context *pp_load_save_context = private_context;

    return pp_load_save_context->dest_h / 8;
}

static int
pp_load_save_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;
    struct pp_load_save_context *pp_load_save_context = (struct pp_load_save_context *)pp_context->private_context;

    pp_inline_parameter->grf5.destination_block_horizontal_origin = x * 16 + pp_load_save_context->dest_x;
    pp_inline_parameter->grf5.destination_block_vertical_origin = y * 8 + pp_load_save_context->dest_y;

    return 0;
}

static void calculate_boundary_block_mask(struct i965_post_processing_context *pp_context, const VARectangle *dst_rect)
{
    int i;
    /* x offset of dest surface must be dword aligned.
     * so we have to extend dst surface on left edge, and mask out pixels not interested
     */
    if (dst_rect->x%GPU_ASM_X_OFFSET_ALIGNMENT) {
        pp_context->block_horizontal_mask_left = 0;
        for (i=dst_rect->x%GPU_ASM_X_OFFSET_ALIGNMENT; i<GPU_ASM_BLOCK_WIDTH; i++)
        {
            pp_context->block_horizontal_mask_left |= 1<<i;
        }
    }
    else {
        pp_context->block_horizontal_mask_left = 0xffff;
    }
    
    int dst_width_adjust = dst_rect->width + dst_rect->x%GPU_ASM_X_OFFSET_ALIGNMENT; 
    if (dst_width_adjust%GPU_ASM_BLOCK_WIDTH){
        pp_context->block_horizontal_mask_right = (1 << (dst_width_adjust%GPU_ASM_BLOCK_WIDTH)) - 1;
    }
    else {
        pp_context->block_horizontal_mask_right = 0xffff;
    }
    
    if (dst_rect->height%GPU_ASM_BLOCK_HEIGHT){
        pp_context->block_vertical_mask_bottom = (1 << (dst_rect->height%GPU_ASM_BLOCK_HEIGHT)) - 1;
    }
    else {
        pp_context->block_vertical_mask_bottom = 0xff;
    }

}
static VAStatus
pp_plx_load_save_plx_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                const struct i965_surface *src_surface,
                                const VARectangle *src_rect,
                                struct i965_surface *dst_surface,
                                const VARectangle *dst_rect,
                                void *filter_param)
{
    struct pp_load_save_context *pp_load_save_context = (struct pp_load_save_context *)&pp_context->pp_load_save_context;
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;
    struct pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    int width[3], height[3], pitch[3], offset[3];

    /* source surface */
    pp_set_media_rw_message_surface(ctx, pp_context, src_surface, 1, 0,
                                    width, height, pitch, offset);

    /* destination surface */
    pp_set_media_rw_message_surface(ctx, pp_context, dst_surface, 7, 1,
                                    width, height, pitch, offset);

    /* private function & data */
    pp_context->pp_x_steps = pp_load_save_x_steps;
    pp_context->pp_y_steps = pp_load_save_y_steps;
    pp_context->private_context = &pp_context->pp_load_save_context;
    pp_context->pp_set_block_parameter = pp_load_save_set_block_parameter;

    int dst_left_edge_extend = dst_rect->x%GPU_ASM_X_OFFSET_ALIGNMENT;;
    pp_load_save_context->dest_x = dst_rect->x - dst_left_edge_extend;
    pp_load_save_context->dest_y = dst_rect->y;
    pp_load_save_context->dest_h = ALIGN(dst_rect->height, 8);
    pp_load_save_context->dest_w = ALIGN(dst_rect->width+dst_left_edge_extend, 16);

    pp_inline_parameter->grf5.block_count_x = pp_load_save_context->dest_w / 16;   /* 1 x N */
    pp_inline_parameter->grf5.number_blocks = pp_load_save_context->dest_w / 16;

    pp_static_parameter->grf3.horizontal_origin_offset = src_rect->x;
    pp_static_parameter->grf3.vertical_origin_offset = src_rect->y;

    // update u/v offset for packed yuv
    i965_update_src_surface_static_parameter (ctx, pp_context, src_surface);
    i965_update_dst_surface_static_parameter (ctx, pp_context, dst_surface);

    dst_surface->flags = src_surface->flags;

    return VA_STATUS_SUCCESS;
}

static int
pp_scaling_x_steps(void *private_context)
{
    return 1;
}

static int
pp_scaling_y_steps(void *private_context)
{
    struct pp_scaling_context *pp_scaling_context = private_context;

    return pp_scaling_context->dest_h / 8;
}

static int
pp_scaling_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    struct pp_scaling_context *pp_scaling_context = (struct pp_scaling_context *)pp_context->private_context;
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;
    struct pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    float src_x_steping = pp_inline_parameter->grf5.normalized_video_x_scaling_step;
    float src_y_steping = pp_static_parameter->grf1.r1_6.normalized_video_y_scaling_step;

    pp_inline_parameter->grf5.r5_1.source_surface_block_normalized_horizontal_origin = src_x_steping * x * 16 + pp_scaling_context->src_normalized_x;
    pp_inline_parameter->grf5.source_surface_block_normalized_vertical_origin = src_y_steping * y * 8 + pp_scaling_context->src_normalized_y;
    pp_inline_parameter->grf5.destination_block_horizontal_origin = x * 16 + pp_scaling_context->dest_x;
    pp_inline_parameter->grf5.destination_block_vertical_origin = y * 8 + pp_scaling_context->dest_y;
    
    return 0;
}

static VAStatus
pp_nv12_scaling_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                           const struct i965_surface *src_surface,
                           const VARectangle *src_rect,
                           struct i965_surface *dst_surface,
                           const VARectangle *dst_rect,
                           void *filter_param)
{
    struct pp_scaling_context *pp_scaling_context = (struct pp_scaling_context *)&pp_context->pp_scaling_context;
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;
    struct pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    struct object_surface *obj_surface;
    struct i965_sampler_state *sampler_state;
    int in_w, in_h, in_wpitch, in_hpitch;
    int out_w, out_h, out_wpitch, out_hpitch;

    /* source surface */
    obj_surface = (struct object_surface *)src_surface->base;
    in_w = obj_surface->orig_width;
    in_h = obj_surface->orig_height;
    in_wpitch = obj_surface->width;
    in_hpitch = obj_surface->height;

    /* source Y surface index 1 */
    i965_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, 0,
                              in_w, in_h, in_wpitch, I965_SURFACEFORMAT_R8_UNORM,
                              1, 0);

    /* source UV surface index 2 */
    i965_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, in_wpitch * in_hpitch,
                              in_w / 2, in_h / 2, in_wpitch, I965_SURFACEFORMAT_R8G8_UNORM,
                              2, 0);

    /* destination surface */
    obj_surface = (struct object_surface *)dst_surface->base;
    out_w = obj_surface->orig_width;
    out_h = obj_surface->orig_height;
    out_wpitch = obj_surface->width;
    out_hpitch = obj_surface->height;

    /* destination Y surface index 7 */
    i965_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, 0,
                              out_w / 4, out_h, out_wpitch, I965_SURFACEFORMAT_R8_UNORM,
                              7, 1);

    /* destination UV surface index 8 */
    i965_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, out_wpitch * out_hpitch,
                              out_w / 4, out_h / 2, out_wpitch, I965_SURFACEFORMAT_R8G8_UNORM,
                              8, 1);

    /* sampler state */
    dri_bo_map(pp_context->sampler_state_table.bo, True);
    assert(pp_context->sampler_state_table.bo->virtual);
    sampler_state = pp_context->sampler_state_table.bo->virtual;

    /* SIMD16 Y index 1 */
    sampler_state[1].ss0.min_filter = I965_MAPFILTER_LINEAR;
    sampler_state[1].ss0.mag_filter = I965_MAPFILTER_LINEAR;
    sampler_state[1].ss1.r_wrap_mode = I965_TEXCOORDMODE_CLAMP;
    sampler_state[1].ss1.s_wrap_mode = I965_TEXCOORDMODE_CLAMP;
    sampler_state[1].ss1.t_wrap_mode = I965_TEXCOORDMODE_CLAMP;

    /* SIMD16 UV index 2 */
    sampler_state[2].ss0.min_filter = I965_MAPFILTER_LINEAR;
    sampler_state[2].ss0.mag_filter = I965_MAPFILTER_LINEAR;
    sampler_state[2].ss1.r_wrap_mode = I965_TEXCOORDMODE_CLAMP;
    sampler_state[2].ss1.s_wrap_mode = I965_TEXCOORDMODE_CLAMP;
    sampler_state[2].ss1.t_wrap_mode = I965_TEXCOORDMODE_CLAMP;

    dri_bo_unmap(pp_context->sampler_state_table.bo);

    /* private function & data */
    pp_context->pp_x_steps = pp_scaling_x_steps;
    pp_context->pp_y_steps = pp_scaling_y_steps;
    pp_context->private_context = &pp_context->pp_scaling_context;
    pp_context->pp_set_block_parameter = pp_scaling_set_block_parameter;

    int dst_left_edge_extend = dst_rect->x%GPU_ASM_X_OFFSET_ALIGNMENT;
    float src_left_edge_extend = (float)dst_left_edge_extend*src_rect->width/dst_rect->width;
    pp_scaling_context->dest_x = dst_rect->x - dst_left_edge_extend;
    pp_scaling_context->dest_y = dst_rect->y;
    pp_scaling_context->dest_w = ALIGN(dst_rect->width + dst_left_edge_extend, 16);
    pp_scaling_context->dest_h = ALIGN(dst_rect->height, 8);
    pp_scaling_context->src_normalized_x = (float)(src_rect->x - src_left_edge_extend)/ in_w;
    pp_scaling_context->src_normalized_y = (float)src_rect->y / in_h;

    pp_static_parameter->grf1.r1_6.normalized_video_y_scaling_step = (float) src_rect->height / in_h / dst_rect->height;

    pp_inline_parameter->grf5.normalized_video_x_scaling_step = (float) (src_rect->width + src_left_edge_extend)/ in_w / (dst_rect->width + dst_left_edge_extend);
    pp_inline_parameter->grf5.block_count_x = pp_scaling_context->dest_w / 16;   /* 1 x N */
    pp_inline_parameter->grf5.number_blocks = pp_scaling_context->dest_w / 16;

    dst_surface->flags = src_surface->flags;

    return VA_STATUS_SUCCESS;
}

static int
pp_avs_x_steps(void *private_context)
{
    struct pp_avs_context *pp_avs_context = private_context;

    return pp_avs_context->dest_w / 16;
}

static int
pp_avs_y_steps(void *private_context)
{
    return 1;
}

static int
pp_avs_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    struct pp_avs_context *pp_avs_context = (struct pp_avs_context *)pp_context->private_context;
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;
    struct pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    float src_x_steping, src_y_steping, video_step_delta;
    int tmp_w = ALIGN(pp_avs_context->dest_h * pp_avs_context->src_w / pp_avs_context->src_h, 16);

    if (pp_static_parameter->grf4.r4_2.avs.nlas == 0) {
        src_x_steping = pp_inline_parameter->grf5.normalized_video_x_scaling_step;
        pp_inline_parameter->grf5.r5_1.source_surface_block_normalized_horizontal_origin = src_x_steping * x * 16 + pp_avs_context->src_normalized_x;
    } else if (tmp_w >= pp_avs_context->dest_w) {
        pp_inline_parameter->grf5.normalized_video_x_scaling_step = 1.0 / tmp_w;
        pp_inline_parameter->grf6.video_step_delta = 0;
        
        if (x == 0) {
            pp_inline_parameter->grf5.r5_1.source_surface_block_normalized_horizontal_origin = (float)(tmp_w - pp_avs_context->dest_w) / tmp_w / 2 +
                pp_avs_context->src_normalized_x;
        } else {
            src_x_steping = pp_inline_parameter->grf5.normalized_video_x_scaling_step;
            video_step_delta = pp_inline_parameter->grf6.video_step_delta;
            pp_inline_parameter->grf5.r5_1.source_surface_block_normalized_horizontal_origin += src_x_steping * 16 +
                16 * 15 * video_step_delta / 2;
        }
    } else {
        int n0, n1, n2, nls_left, nls_right;
        int factor_a = 5, factor_b = 4;
        float f;

        n0 = (pp_avs_context->dest_w - tmp_w) / (16 * 2);
        n1 = (pp_avs_context->dest_w - tmp_w) / 16 - n0;
        n2 = tmp_w / (16 * factor_a);
        nls_left = n0 + n2;
        nls_right = n1 + n2;
        f = (float) n2 * 16 / tmp_w;
        
        if (n0 < 5) {
            pp_inline_parameter->grf6.video_step_delta = 0.0;

            if (x == 0) {
                pp_inline_parameter->grf5.normalized_video_x_scaling_step = 1.0 / pp_avs_context->dest_w;
                pp_inline_parameter->grf5.r5_1.source_surface_block_normalized_horizontal_origin = pp_avs_context->src_normalized_x;
            } else {
                src_x_steping = pp_inline_parameter->grf5.normalized_video_x_scaling_step;
                video_step_delta = pp_inline_parameter->grf6.video_step_delta;
                pp_inline_parameter->grf5.r5_1.source_surface_block_normalized_horizontal_origin += src_x_steping * 16 +
                    16 * 15 * video_step_delta / 2;
            }
        } else {
            if (x < nls_left) {
                /* f = a * nls_left * 16 + b * nls_left * 16 * (nls_left * 16 - 1) / 2 */
                float a = f / (nls_left * 16 * factor_b);
                float b = (f - nls_left * 16 * a) * 2 / (nls_left * 16 * (nls_left * 16 - 1));
                
                pp_inline_parameter->grf6.video_step_delta = b;

                if (x == 0) {
                    pp_inline_parameter->grf5.r5_1.source_surface_block_normalized_horizontal_origin = pp_avs_context->src_normalized_x;
                    pp_inline_parameter->grf5.normalized_video_x_scaling_step = a;
                } else {
                    src_x_steping = pp_inline_parameter->grf5.normalized_video_x_scaling_step;
                    video_step_delta = pp_inline_parameter->grf6.video_step_delta;
                    pp_inline_parameter->grf5.r5_1.source_surface_block_normalized_horizontal_origin += src_x_steping * 16 +
                        16 * 15 * video_step_delta / 2;
                    pp_inline_parameter->grf5.normalized_video_x_scaling_step += 16 * b;
                }
            } else if (x < (pp_avs_context->dest_w / 16 - nls_right)) {
                /* scale the center linearly */
                src_x_steping = pp_inline_parameter->grf5.normalized_video_x_scaling_step;
                video_step_delta = pp_inline_parameter->grf6.video_step_delta;
                pp_inline_parameter->grf5.r5_1.source_surface_block_normalized_horizontal_origin += src_x_steping * 16 +
                    16 * 15 * video_step_delta / 2;
                pp_inline_parameter->grf6.video_step_delta = 0.0;
                pp_inline_parameter->grf5.normalized_video_x_scaling_step = 1.0 / tmp_w;
            } else {
                float a = f / (nls_right * 16 * factor_b);
                float b = (f - nls_right * 16 * a) * 2 / (nls_right * 16 * (nls_right * 16 - 1));

                src_x_steping = pp_inline_parameter->grf5.normalized_video_x_scaling_step;
                video_step_delta = pp_inline_parameter->grf6.video_step_delta;
                pp_inline_parameter->grf5.r5_1.source_surface_block_normalized_horizontal_origin += src_x_steping * 16 +
                    16 * 15 * video_step_delta / 2;
                pp_inline_parameter->grf6.video_step_delta = -b;

                if (x == (pp_avs_context->dest_w / 16 - nls_right))
                    pp_inline_parameter->grf5.normalized_video_x_scaling_step = a + (nls_right * 16  - 1) * b;
                else
                    pp_inline_parameter->grf5.normalized_video_x_scaling_step -= b * 16;
            }
        }
    }

    src_y_steping = pp_static_parameter->grf1.r1_6.normalized_video_y_scaling_step;
    pp_inline_parameter->grf5.source_surface_block_normalized_vertical_origin = src_y_steping * y * 8 + pp_avs_context->src_normalized_y;
    pp_inline_parameter->grf5.destination_block_horizontal_origin = x * 16 + pp_avs_context->dest_x;
    pp_inline_parameter->grf5.destination_block_vertical_origin = y * 8 + pp_avs_context->dest_y;

    return 0;
}

static VAStatus
pp_nv12_avs_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                       const struct i965_surface *src_surface,
                       const VARectangle *src_rect,
                       struct i965_surface *dst_surface,
                       const VARectangle *dst_rect,
                       void *filter_param,
                       int nlas)
{
    struct pp_avs_context *pp_avs_context = (struct pp_avs_context *)&pp_context->pp_avs_context;
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;
    struct pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    struct object_surface *obj_surface;
    struct i965_sampler_8x8 *sampler_8x8;
    struct i965_sampler_8x8_state *sampler_8x8_state;
    int index;
    int in_w, in_h, in_wpitch, in_hpitch;
    int out_w, out_h, out_wpitch, out_hpitch;
    int i;

    /* surface */
    obj_surface = (struct object_surface *)src_surface->base;
    in_w = obj_surface->orig_width;
    in_h = obj_surface->orig_height;
    in_wpitch = obj_surface->width;
    in_hpitch = obj_surface->height;

    /* source Y surface index 1 */
    i965_pp_set_surface2_state(ctx, pp_context,
                               obj_surface->bo, 0,
                               in_w, in_h, in_wpitch,
                               0, 0,
                               SURFACE_FORMAT_Y8_UNORM, 0,
                               1);

    /* source UV surface index 2 */
    i965_pp_set_surface2_state(ctx, pp_context,
                               obj_surface->bo, in_wpitch * in_hpitch,
                               in_w / 2, in_h / 2, in_wpitch,
                               0, 0,
                               SURFACE_FORMAT_R8B8_UNORM, 0,
                               2);

    /* destination surface */
    obj_surface = (struct object_surface *)dst_surface->base;
    out_w = obj_surface->orig_width;
    out_h = obj_surface->orig_height;
    out_wpitch = obj_surface->width;
    out_hpitch = obj_surface->height;
    assert(out_w <= out_wpitch && out_h <= out_hpitch);

    /* destination Y surface index 7 */
    i965_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, 0,
                              out_w / 4, out_h, out_wpitch, I965_SURFACEFORMAT_R8_UNORM,
                              7, 1);

    /* destination UV surface index 8 */
    i965_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, out_wpitch * out_hpitch,
                              out_w / 4, out_h / 2, out_wpitch, I965_SURFACEFORMAT_R8G8_UNORM,
                              8, 1);

    /* sampler 8x8 state */
    dri_bo_map(pp_context->sampler_state_table.bo_8x8, True);
    assert(pp_context->sampler_state_table.bo_8x8->virtual);
    assert(sizeof(*sampler_8x8_state) == sizeof(int) * 138);
    sampler_8x8_state = pp_context->sampler_state_table.bo_8x8->virtual;
    memset(sampler_8x8_state, 0, sizeof(*sampler_8x8_state));

    for (i = 0; i < 17; i++) {
        /* for Y channel, currently ignore */
        sampler_8x8_state->coefficients[i].dw0.table_0x_filter_c0 = 0x00;
        sampler_8x8_state->coefficients[i].dw0.table_0x_filter_c1 = 0x00;
        sampler_8x8_state->coefficients[i].dw0.table_0x_filter_c2 = 0x08;
        sampler_8x8_state->coefficients[i].dw0.table_0x_filter_c3 = 0x18;
        sampler_8x8_state->coefficients[i].dw1.table_0x_filter_c4 = 0x18;
        sampler_8x8_state->coefficients[i].dw1.table_0x_filter_c5 = 0x08;
        sampler_8x8_state->coefficients[i].dw1.table_0x_filter_c6 = 0x00;
        sampler_8x8_state->coefficients[i].dw1.table_0x_filter_c7 = 0x00;
        sampler_8x8_state->coefficients[i].dw2.table_0y_filter_c0 = 0x00;
        sampler_8x8_state->coefficients[i].dw2.table_0y_filter_c1 = 0x00;
        sampler_8x8_state->coefficients[i].dw2.table_0y_filter_c2 = 0x10;
        sampler_8x8_state->coefficients[i].dw2.table_0y_filter_c3 = 0x10;
        sampler_8x8_state->coefficients[i].dw3.table_0y_filter_c4 = 0x10;
        sampler_8x8_state->coefficients[i].dw3.table_0y_filter_c5 = 0x10;
        sampler_8x8_state->coefficients[i].dw3.table_0y_filter_c6 = 0x00;
        sampler_8x8_state->coefficients[i].dw3.table_0y_filter_c7 = 0x00;
        /* for U/V channel, 0.25 */
        sampler_8x8_state->coefficients[i].dw4.table_1x_filter_c0 = 0x0;
        sampler_8x8_state->coefficients[i].dw4.table_1x_filter_c1 = 0x0;
        sampler_8x8_state->coefficients[i].dw4.table_1x_filter_c2 = 0x10;
        sampler_8x8_state->coefficients[i].dw4.table_1x_filter_c3 = 0x10;
        sampler_8x8_state->coefficients[i].dw5.table_1x_filter_c4 = 0x10;
        sampler_8x8_state->coefficients[i].dw5.table_1x_filter_c5 = 0x10;
        sampler_8x8_state->coefficients[i].dw5.table_1x_filter_c6 = 0x0;
        sampler_8x8_state->coefficients[i].dw5.table_1x_filter_c7 = 0x0;
        sampler_8x8_state->coefficients[i].dw6.table_1y_filter_c0 = 0x0;
        sampler_8x8_state->coefficients[i].dw6.table_1y_filter_c1 = 0x0;
        sampler_8x8_state->coefficients[i].dw6.table_1y_filter_c2 = 0x10;
        sampler_8x8_state->coefficients[i].dw6.table_1y_filter_c3 = 0x10;
        sampler_8x8_state->coefficients[i].dw7.table_1y_filter_c4 = 0x10;
        sampler_8x8_state->coefficients[i].dw7.table_1y_filter_c5 = 0x10;
        sampler_8x8_state->coefficients[i].dw7.table_1y_filter_c6 = 0x0;
        sampler_8x8_state->coefficients[i].dw7.table_1y_filter_c7 = 0x0;
    }

    sampler_8x8_state->dw136.default_sharpness_level = 0;
    sampler_8x8_state->dw137.adaptive_filter_for_all_channel = 1;
    sampler_8x8_state->dw137.bypass_y_adaptive_filtering = 1;
    sampler_8x8_state->dw137.bypass_x_adaptive_filtering = 1;
    dri_bo_unmap(pp_context->sampler_state_table.bo_8x8);

    /* sampler 8x8 */
    dri_bo_map(pp_context->sampler_state_table.bo, True);
    assert(pp_context->sampler_state_table.bo->virtual);
    assert(sizeof(*sampler_8x8) == sizeof(int) * 16);
    sampler_8x8 = pp_context->sampler_state_table.bo->virtual;

    /* sample_8x8 Y index 1 */
    index = 1;
    memset(&sampler_8x8[index], 0, sizeof(*sampler_8x8));
    sampler_8x8[index].dw0.avs_filter_type = AVS_FILTER_ADAPTIVE_8_TAP;
    sampler_8x8[index].dw0.ief_bypass = 1;
    sampler_8x8[index].dw0.ief_filter_type = IEF_FILTER_DETAIL;
    sampler_8x8[index].dw0.ief_filter_size = IEF_FILTER_SIZE_5X5;
    sampler_8x8[index].dw1.sampler_8x8_state_pointer = pp_context->sampler_state_table.bo_8x8->offset >> 5;
    sampler_8x8[index].dw2.global_noise_estimation = 22;
    sampler_8x8[index].dw2.strong_edge_threshold = 8;
    sampler_8x8[index].dw2.weak_edge_threshold = 1;
    sampler_8x8[index].dw3.strong_edge_weight = 7;
    sampler_8x8[index].dw3.regular_weight = 2;
    sampler_8x8[index].dw3.non_edge_weight = 0;
    sampler_8x8[index].dw3.gain_factor = 40;
    sampler_8x8[index].dw4.steepness_boost = 0;
    sampler_8x8[index].dw4.steepness_threshold = 0;
    sampler_8x8[index].dw4.mr_boost = 0;
    sampler_8x8[index].dw4.mr_threshold = 5;
    sampler_8x8[index].dw5.pwl1_point_1 = 4;
    sampler_8x8[index].dw5.pwl1_point_2 = 12;
    sampler_8x8[index].dw5.pwl1_point_3 = 16;
    sampler_8x8[index].dw5.pwl1_point_4 = 26;
    sampler_8x8[index].dw6.pwl1_point_5 = 40;
    sampler_8x8[index].dw6.pwl1_point_6 = 160;
    sampler_8x8[index].dw6.pwl1_r3_bias_0 = 127;
    sampler_8x8[index].dw6.pwl1_r3_bias_1 = 98;
    sampler_8x8[index].dw7.pwl1_r3_bias_2 = 88;
    sampler_8x8[index].dw7.pwl1_r3_bias_3 = 64;
    sampler_8x8[index].dw7.pwl1_r3_bias_4 = 44;
    sampler_8x8[index].dw7.pwl1_r3_bias_5 = 0;
    sampler_8x8[index].dw8.pwl1_r3_bias_6 = 0;
    sampler_8x8[index].dw8.pwl1_r5_bias_0 = 3;
    sampler_8x8[index].dw8.pwl1_r5_bias_1 = 32;
    sampler_8x8[index].dw8.pwl1_r5_bias_2 = 32;
    sampler_8x8[index].dw9.pwl1_r5_bias_3 = 58;
    sampler_8x8[index].dw9.pwl1_r5_bias_4 = 100;
    sampler_8x8[index].dw9.pwl1_r5_bias_5 = 108;
    sampler_8x8[index].dw9.pwl1_r5_bias_6 = 88;
    sampler_8x8[index].dw10.pwl1_r3_slope_0 = -116;
    sampler_8x8[index].dw10.pwl1_r3_slope_1 = -20;
    sampler_8x8[index].dw10.pwl1_r3_slope_2 = -96;
    sampler_8x8[index].dw10.pwl1_r3_slope_3 = -32;
    sampler_8x8[index].dw11.pwl1_r3_slope_4 = -50;
    sampler_8x8[index].dw11.pwl1_r3_slope_5 = 0;
    sampler_8x8[index].dw11.pwl1_r3_slope_6 = 0;
    sampler_8x8[index].dw11.pwl1_r5_slope_0 = 116;
    sampler_8x8[index].dw12.pwl1_r5_slope_1 = 0;
    sampler_8x8[index].dw12.pwl1_r5_slope_2 = 114;
    sampler_8x8[index].dw12.pwl1_r5_slope_3 = 67;
    sampler_8x8[index].dw12.pwl1_r5_slope_4 = 9;
    sampler_8x8[index].dw13.pwl1_r5_slope_5 = -3;
    sampler_8x8[index].dw13.pwl1_r5_slope_6 = -15;
    sampler_8x8[index].dw13.limiter_boost = 0;
    sampler_8x8[index].dw13.minimum_limiter = 10;
    sampler_8x8[index].dw13.maximum_limiter = 11;
    sampler_8x8[index].dw14.clip_limiter = 130;
    dri_bo_emit_reloc(pp_context->sampler_state_table.bo,
                      I915_GEM_DOMAIN_RENDER, 
                      0,
                      0,
                      sizeof(*sampler_8x8) * index + offsetof(struct i965_sampler_8x8, dw1),
                      pp_context->sampler_state_table.bo_8x8);

    /* sample_8x8 UV index 2 */
    index = 2;
    memset(&sampler_8x8[index], 0, sizeof(*sampler_8x8));
    sampler_8x8[index].dw0.avs_filter_type = AVS_FILTER_ADAPTIVE_8_TAP;
    sampler_8x8[index].dw0.ief_bypass = 1;
    sampler_8x8[index].dw0.ief_filter_type = IEF_FILTER_DETAIL;
    sampler_8x8[index].dw0.ief_filter_size = IEF_FILTER_SIZE_5X5;
    sampler_8x8[index].dw1.sampler_8x8_state_pointer = pp_context->sampler_state_table.bo_8x8->offset >> 5;
    sampler_8x8[index].dw2.global_noise_estimation = 22;
    sampler_8x8[index].dw2.strong_edge_threshold = 8;
    sampler_8x8[index].dw2.weak_edge_threshold = 1;
    sampler_8x8[index].dw3.strong_edge_weight = 7;
    sampler_8x8[index].dw3.regular_weight = 2;
    sampler_8x8[index].dw3.non_edge_weight = 0;
    sampler_8x8[index].dw3.gain_factor = 40;
    sampler_8x8[index].dw4.steepness_boost = 0;
    sampler_8x8[index].dw4.steepness_threshold = 0;
    sampler_8x8[index].dw4.mr_boost = 0;
    sampler_8x8[index].dw4.mr_threshold = 5;
    sampler_8x8[index].dw5.pwl1_point_1 = 4;
    sampler_8x8[index].dw5.pwl1_point_2 = 12;
    sampler_8x8[index].dw5.pwl1_point_3 = 16;
    sampler_8x8[index].dw5.pwl1_point_4 = 26;
    sampler_8x8[index].dw6.pwl1_point_5 = 40;
    sampler_8x8[index].dw6.pwl1_point_6 = 160;
    sampler_8x8[index].dw6.pwl1_r3_bias_0 = 127;
    sampler_8x8[index].dw6.pwl1_r3_bias_1 = 98;
    sampler_8x8[index].dw7.pwl1_r3_bias_2 = 88;
    sampler_8x8[index].dw7.pwl1_r3_bias_3 = 64;
    sampler_8x8[index].dw7.pwl1_r3_bias_4 = 44;
    sampler_8x8[index].dw7.pwl1_r3_bias_5 = 0;
    sampler_8x8[index].dw8.pwl1_r3_bias_6 = 0;
    sampler_8x8[index].dw8.pwl1_r5_bias_0 = 3;
    sampler_8x8[index].dw8.pwl1_r5_bias_1 = 32;
    sampler_8x8[index].dw8.pwl1_r5_bias_2 = 32;
    sampler_8x8[index].dw9.pwl1_r5_bias_3 = 58;
    sampler_8x8[index].dw9.pwl1_r5_bias_4 = 100;
    sampler_8x8[index].dw9.pwl1_r5_bias_5 = 108;
    sampler_8x8[index].dw9.pwl1_r5_bias_6 = 88;
    sampler_8x8[index].dw10.pwl1_r3_slope_0 = -116;
    sampler_8x8[index].dw10.pwl1_r3_slope_1 = -20;
    sampler_8x8[index].dw10.pwl1_r3_slope_2 = -96;
    sampler_8x8[index].dw10.pwl1_r3_slope_3 = -32;
    sampler_8x8[index].dw11.pwl1_r3_slope_4 = -50;
    sampler_8x8[index].dw11.pwl1_r3_slope_5 = 0;
    sampler_8x8[index].dw11.pwl1_r3_slope_6 = 0;
    sampler_8x8[index].dw11.pwl1_r5_slope_0 = 116;
    sampler_8x8[index].dw12.pwl1_r5_slope_1 = 0;
    sampler_8x8[index].dw12.pwl1_r5_slope_2 = 114;
    sampler_8x8[index].dw12.pwl1_r5_slope_3 = 67;
    sampler_8x8[index].dw12.pwl1_r5_slope_4 = 9;
    sampler_8x8[index].dw13.pwl1_r5_slope_5 = -3;
    sampler_8x8[index].dw13.pwl1_r5_slope_6 = -15;
    sampler_8x8[index].dw13.limiter_boost = 0;
    sampler_8x8[index].dw13.minimum_limiter = 10;
    sampler_8x8[index].dw13.maximum_limiter = 11;
    sampler_8x8[index].dw14.clip_limiter = 130;
    dri_bo_emit_reloc(pp_context->sampler_state_table.bo,
                      I915_GEM_DOMAIN_RENDER, 
                      0,
                      0,
                      sizeof(*sampler_8x8) * index + offsetof(struct i965_sampler_8x8, dw1),
                      pp_context->sampler_state_table.bo_8x8);

    dri_bo_unmap(pp_context->sampler_state_table.bo);

    /* private function & data */
    pp_context->pp_x_steps = pp_avs_x_steps;
    pp_context->pp_y_steps = pp_avs_y_steps;
    pp_context->private_context = &pp_context->pp_avs_context;
    pp_context->pp_set_block_parameter = pp_avs_set_block_parameter;

    int dst_left_edge_extend = dst_rect->x%GPU_ASM_X_OFFSET_ALIGNMENT;
    float src_left_edge_extend = (float)dst_left_edge_extend*src_rect->width/dst_rect->width;
    pp_avs_context->dest_x = dst_rect->x - dst_left_edge_extend;
    pp_avs_context->dest_y = dst_rect->y;
    pp_avs_context->dest_w = ALIGN(dst_rect->width + dst_left_edge_extend, 16);
    pp_avs_context->dest_h = ALIGN(dst_rect->height, 8);
    pp_avs_context->src_normalized_x = (float)(src_rect->x - src_left_edge_extend)/ in_w;
    pp_avs_context->src_normalized_y = (float)src_rect->y / in_h;
    pp_avs_context->src_w = src_rect->width + src_left_edge_extend;
    pp_avs_context->src_h = src_rect->height;

    pp_static_parameter->grf4.r4_2.avs.nlas = nlas;
    pp_static_parameter->grf1.r1_6.normalized_video_y_scaling_step = (float) src_rect->height / in_h / dst_rect->height;

    pp_inline_parameter->grf5.normalized_video_x_scaling_step = (float) (src_rect->width + src_left_edge_extend)/ in_w / (dst_rect->width + dst_left_edge_extend);
    pp_inline_parameter->grf5.block_count_x = 1;        /* M x 1 */
    pp_inline_parameter->grf5.number_blocks = pp_avs_context->dest_h / 8;
    pp_inline_parameter->grf6.video_step_delta = 0.0;

    dst_surface->flags = src_surface->flags;

    return VA_STATUS_SUCCESS;
}

static VAStatus
pp_nv12_avs_initialize_nlas(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                            const struct i965_surface *src_surface,
                            const VARectangle *src_rect,
                            struct i965_surface *dst_surface,
                            const VARectangle *dst_rect,
                            void *filter_param)
{
    return pp_nv12_avs_initialize(ctx, pp_context,
                                  src_surface,
                                  src_rect,
                                  dst_surface,
                                  dst_rect,
                                  filter_param,
                                  1);
}

static VAStatus
gen6_nv12_scaling_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                             const struct i965_surface *src_surface,
                             const VARectangle *src_rect,
                             struct i965_surface *dst_surface,
                             const VARectangle *dst_rect,
                             void *filter_param)
{
    return pp_nv12_avs_initialize(ctx, pp_context,
                                  src_surface,
                                  src_rect,
                                  dst_surface,
                                  dst_rect,
                                  filter_param,
                                  0);    
}

static int
gen7_pp_avs_x_steps(void *private_context)
{
    struct pp_avs_context *pp_avs_context = private_context;

    return pp_avs_context->dest_w / 16;
}

static int
gen7_pp_avs_y_steps(void *private_context)
{
    struct pp_avs_context *pp_avs_context = private_context;

    return pp_avs_context->dest_h / 16;
}

static int
gen7_pp_avs_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    struct pp_avs_context *pp_avs_context = (struct pp_avs_context *)pp_context->private_context;
    struct gen7_pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;

    pp_inline_parameter->grf7.destination_block_horizontal_origin = x * 16 + pp_avs_context->dest_x;
    pp_inline_parameter->grf7.destination_block_vertical_origin = y * 16 + pp_avs_context->dest_y;
    pp_inline_parameter->grf7.constant_0 = 0xffffffff;
    pp_inline_parameter->grf7.sampler_load_main_video_x_scaling_step = pp_avs_context->horiz_range / pp_avs_context->src_w;

    return 0;
}

static void gen7_update_src_surface_uv_offset(VADriverContextP    ctx, 
                                              struct i965_post_processing_context *pp_context,
                                              const struct i965_surface *surface)
{
    struct gen7_pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    int fourcc = pp_get_surface_fourcc(ctx, surface);
    
    if (fourcc == VA_FOURCC_YUY2) {
        pp_static_parameter->grf2.di_destination_packed_y_component_offset = 0;
        pp_static_parameter->grf2.di_destination_packed_u_component_offset = 1;
        pp_static_parameter->grf2.di_destination_packed_v_component_offset = 3;
    } else if (fourcc == VA_FOURCC_UYVY) {
        pp_static_parameter->grf2.di_destination_packed_y_component_offset = 1;
        pp_static_parameter->grf2.di_destination_packed_u_component_offset = 0;
        pp_static_parameter->grf2.di_destination_packed_v_component_offset = 2;
    }
}

static VAStatus
gen7_pp_plx_avs_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                           const struct i965_surface *src_surface,
                           const VARectangle *src_rect,
                           struct i965_surface *dst_surface,
                           const VARectangle *dst_rect,
                           void *filter_param)
{
    struct pp_avs_context *pp_avs_context = (struct pp_avs_context *)&pp_context->pp_avs_context;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen7_pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    struct gen7_sampler_8x8 *sampler_8x8;
    struct i965_sampler_8x8_state *sampler_8x8_state;
    int index, i;
    int width[3], height[3], pitch[3], offset[3];
    int src_width, src_height;

    /* source surface */
    gen7_pp_set_media_rw_message_surface(ctx, pp_context, src_surface, 0, 0,
                                         src_rect,
                                         width, height, pitch, offset);
    src_width = width[0];
    src_height = height[0];

    /* destination surface */
    gen7_pp_set_media_rw_message_surface(ctx, pp_context, dst_surface, 24, 1,
                                         dst_rect,
                                         width, height, pitch, offset);

    /* sampler 8x8 state */
    dri_bo_map(pp_context->sampler_state_table.bo_8x8, True);
    assert(pp_context->sampler_state_table.bo_8x8->virtual);
    assert(sizeof(*sampler_8x8_state) == sizeof(int) * 138);
    sampler_8x8_state = pp_context->sampler_state_table.bo_8x8->virtual;
    memset(sampler_8x8_state, 0, sizeof(*sampler_8x8_state));

    for (i = 0; i < 17; i++) {
	float coff;
	coff = i;
	coff = coff / 16;
        /* for Y channel, currently ignore */
        sampler_8x8_state->coefficients[i].dw0.table_0x_filter_c0 = 0x0;
        sampler_8x8_state->coefficients[i].dw0.table_0x_filter_c1 = 0x0;
        sampler_8x8_state->coefficients[i].dw0.table_0x_filter_c2 = 0x0;
        sampler_8x8_state->coefficients[i].dw0.table_0x_filter_c3 = intel_format_convert(1 - coff, 1, 6,0);
        sampler_8x8_state->coefficients[i].dw1.table_0x_filter_c4 = intel_format_convert(coff, 1, 6, 0);
        sampler_8x8_state->coefficients[i].dw1.table_0x_filter_c5 = 0x0;
        sampler_8x8_state->coefficients[i].dw1.table_0x_filter_c6 = 0x0;
        sampler_8x8_state->coefficients[i].dw1.table_0x_filter_c7 = 0x0;
        sampler_8x8_state->coefficients[i].dw2.table_0y_filter_c0 = 0x0;
        sampler_8x8_state->coefficients[i].dw2.table_0y_filter_c1 = 0x0;
        sampler_8x8_state->coefficients[i].dw2.table_0y_filter_c2 = 0x0;
        sampler_8x8_state->coefficients[i].dw2.table_0y_filter_c3 = intel_format_convert(1 - coff, 1, 6, 0);
        sampler_8x8_state->coefficients[i].dw3.table_0y_filter_c4 = intel_format_convert(coff, 1, 6, 0);
        sampler_8x8_state->coefficients[i].dw3.table_0y_filter_c5 = 0x0;
        sampler_8x8_state->coefficients[i].dw3.table_0y_filter_c6 = 0x0;
        sampler_8x8_state->coefficients[i].dw3.table_0y_filter_c7 = 0x0;
        /* for U/V channel, 0.25 */
        sampler_8x8_state->coefficients[i].dw4.table_1x_filter_c0 = 0x0;
        sampler_8x8_state->coefficients[i].dw4.table_1x_filter_c1 = 0x0;
        sampler_8x8_state->coefficients[i].dw4.table_1x_filter_c2 = 0x0;
        sampler_8x8_state->coefficients[i].dw4.table_1x_filter_c3 = intel_format_convert(1 - coff, 1, 6, 0);
        sampler_8x8_state->coefficients[i].dw5.table_1x_filter_c4 = intel_format_convert(coff, 1, 6, 0);
        sampler_8x8_state->coefficients[i].dw5.table_1x_filter_c5 = 0;
        sampler_8x8_state->coefficients[i].dw5.table_1x_filter_c6 = 0x0;
        sampler_8x8_state->coefficients[i].dw5.table_1x_filter_c7 = 0x0;
        sampler_8x8_state->coefficients[i].dw6.table_1y_filter_c0 = 0x0;
        sampler_8x8_state->coefficients[i].dw6.table_1y_filter_c1 = 0x0;
        sampler_8x8_state->coefficients[i].dw6.table_1y_filter_c2 = 0x0;
        sampler_8x8_state->coefficients[i].dw6.table_1y_filter_c3 = intel_format_convert(1 - coff, 1, 6, 0);
        sampler_8x8_state->coefficients[i].dw7.table_1y_filter_c4 = intel_format_convert(coff, 1, 6, 0);
        sampler_8x8_state->coefficients[i].dw7.table_1y_filter_c5 = 0x0;
        sampler_8x8_state->coefficients[i].dw7.table_1y_filter_c6 = 0x0;
        sampler_8x8_state->coefficients[i].dw7.table_1y_filter_c7 = 0x0;
    }

    sampler_8x8_state->dw136.default_sharpness_level = 0;
    sampler_8x8_state->dw137.adaptive_filter_for_all_channel = 1;
    sampler_8x8_state->dw137.bypass_y_adaptive_filtering = 1;
    sampler_8x8_state->dw137.bypass_x_adaptive_filtering = 1;
    dri_bo_unmap(pp_context->sampler_state_table.bo_8x8);

    /* sampler 8x8 */
    dri_bo_map(pp_context->sampler_state_table.bo, True);
    assert(pp_context->sampler_state_table.bo->virtual);
    assert(sizeof(*sampler_8x8) == sizeof(int) * 4);
    sampler_8x8 = pp_context->sampler_state_table.bo->virtual;

    /* sample_8x8 Y index 4 */
    index = 4;
    memset(&sampler_8x8[index], 0, sizeof(*sampler_8x8));
    sampler_8x8[index].dw0.global_noise_estimation = 255;
    sampler_8x8[index].dw0.ief_bypass = 1;

    sampler_8x8[index].dw1.sampler_8x8_state_pointer = pp_context->sampler_state_table.bo_8x8->offset >> 5;

    sampler_8x8[index].dw2.weak_edge_threshold = 1;
    sampler_8x8[index].dw2.strong_edge_threshold = 8;
    sampler_8x8[index].dw2.r5x_coefficient = 9;
    sampler_8x8[index].dw2.r5cx_coefficient = 8;
    sampler_8x8[index].dw2.r5c_coefficient = 3;

    sampler_8x8[index].dw3.r3x_coefficient = 27;
    sampler_8x8[index].dw3.r3c_coefficient = 5;
    sampler_8x8[index].dw3.gain_factor = 40;
    sampler_8x8[index].dw3.non_edge_weight = 1;
    sampler_8x8[index].dw3.regular_weight = 2;
    sampler_8x8[index].dw3.strong_edge_weight = 7;
    sampler_8x8[index].dw3.ief4_smooth_enable = 0;

    dri_bo_emit_reloc(pp_context->sampler_state_table.bo,
                      I915_GEM_DOMAIN_RENDER, 
                      0,
                      0,
                      sizeof(*sampler_8x8) * index + offsetof(struct i965_sampler_8x8, dw1),
                      pp_context->sampler_state_table.bo_8x8);

    /* sample_8x8 UV index 8 */
    index = 8;
    memset(&sampler_8x8[index], 0, sizeof(*sampler_8x8));
    sampler_8x8[index].dw0.disable_8x8_filter = 0;
    sampler_8x8[index].dw0.global_noise_estimation = 255;
    sampler_8x8[index].dw0.ief_bypass = 1;
    sampler_8x8[index].dw1.sampler_8x8_state_pointer = pp_context->sampler_state_table.bo_8x8->offset >> 5;
    sampler_8x8[index].dw2.weak_edge_threshold = 1;
    sampler_8x8[index].dw2.strong_edge_threshold = 8;
    sampler_8x8[index].dw2.r5x_coefficient = 9;
    sampler_8x8[index].dw2.r5cx_coefficient = 8;
    sampler_8x8[index].dw2.r5c_coefficient = 3;
    sampler_8x8[index].dw3.r3x_coefficient = 27;
    sampler_8x8[index].dw3.r3c_coefficient = 5;
    sampler_8x8[index].dw3.gain_factor = 40;
    sampler_8x8[index].dw3.non_edge_weight = 1;
    sampler_8x8[index].dw3.regular_weight = 2;
    sampler_8x8[index].dw3.strong_edge_weight = 7;
    sampler_8x8[index].dw3.ief4_smooth_enable = 0;

    dri_bo_emit_reloc(pp_context->sampler_state_table.bo,
                      I915_GEM_DOMAIN_RENDER, 
                      0,
                      0,
                      sizeof(*sampler_8x8) * index + offsetof(struct i965_sampler_8x8, dw1),
                      pp_context->sampler_state_table.bo_8x8);

    /* sampler_8x8 V, index 12 */
    index = 12;
    memset(&sampler_8x8[index], 0, sizeof(*sampler_8x8));
    sampler_8x8[index].dw0.disable_8x8_filter = 0;
    sampler_8x8[index].dw0.global_noise_estimation = 255;
    sampler_8x8[index].dw0.ief_bypass = 1;
    sampler_8x8[index].dw1.sampler_8x8_state_pointer = pp_context->sampler_state_table.bo_8x8->offset >> 5;
    sampler_8x8[index].dw2.weak_edge_threshold = 1;
    sampler_8x8[index].dw2.strong_edge_threshold = 8;
    sampler_8x8[index].dw2.r5x_coefficient = 9;
    sampler_8x8[index].dw2.r5cx_coefficient = 8;
    sampler_8x8[index].dw2.r5c_coefficient = 3;
    sampler_8x8[index].dw3.r3x_coefficient = 27;
    sampler_8x8[index].dw3.r3c_coefficient = 5;
    sampler_8x8[index].dw3.gain_factor = 40;
    sampler_8x8[index].dw3.non_edge_weight = 1;
    sampler_8x8[index].dw3.regular_weight = 2;
    sampler_8x8[index].dw3.strong_edge_weight = 7;
    sampler_8x8[index].dw3.ief4_smooth_enable = 0;

    dri_bo_emit_reloc(pp_context->sampler_state_table.bo,
                      I915_GEM_DOMAIN_RENDER, 
                      0,
                      0,
                      sizeof(*sampler_8x8) * index + offsetof(struct i965_sampler_8x8, dw1),
                      pp_context->sampler_state_table.bo_8x8);

    dri_bo_unmap(pp_context->sampler_state_table.bo);

    /* private function & data */
    pp_context->pp_x_steps = gen7_pp_avs_x_steps;
    pp_context->pp_y_steps = gen7_pp_avs_y_steps;
    pp_context->private_context = &pp_context->pp_avs_context;
    pp_context->pp_set_block_parameter = gen7_pp_avs_set_block_parameter;

    pp_avs_context->dest_x = dst_rect->x;
    pp_avs_context->dest_y = dst_rect->y;
    pp_avs_context->dest_w = ALIGN(dst_rect->width, 16);
    pp_avs_context->dest_h = ALIGN(dst_rect->height, 16);
    pp_avs_context->src_w = src_rect->width;
    pp_avs_context->src_h = src_rect->height;
    pp_avs_context->horiz_range = (float)src_rect->width / src_width;

    int dw = (pp_avs_context->src_w - 1) / 16 + 1;
    dw = MAX(dw, dst_rect->width);

    pp_static_parameter->grf1.pointer_to_inline_parameter = 7;
    pp_static_parameter->grf2.avs_wa_enable = 1; /* must be set for GEN7 */
    if (IS_HASWELL(i965->intel.device_info))
    	pp_static_parameter->grf2.avs_wa_enable = 0; /* HSW don't use the WA */

    if (pp_static_parameter->grf2.avs_wa_enable) {
        int src_fourcc = pp_get_surface_fourcc(ctx, src_surface);
        if ((src_fourcc == VA_FOURCC_RGBA) ||
            (src_fourcc == VA_FOURCC_RGBX) ||
            (src_fourcc == VA_FOURCC_BGRA) ||
            (src_fourcc == VA_FOURCC_BGRX)) {
            pp_static_parameter->grf2.avs_wa_enable = 0;
        }
    }
	
    pp_static_parameter->grf2.avs_wa_width = src_width;
    pp_static_parameter->grf2.avs_wa_one_div_256_width = (float) 1.0 / (256 * src_width);
    pp_static_parameter->grf2.avs_wa_five_div_256_width = (float) 5.0 / (256 * src_width);
    pp_static_parameter->grf2.alpha = 255;

    pp_static_parameter->grf3.sampler_load_horizontal_scaling_step_ratio = (float) pp_avs_context->src_w / dw;
    pp_static_parameter->grf4.sampler_load_vertical_scaling_step = (float) src_rect->height / src_height / dst_rect->height;
    pp_static_parameter->grf5.sampler_load_vertical_frame_origin = (float) src_rect->y / src_height -
        (float) pp_avs_context->dest_y * pp_static_parameter->grf4.sampler_load_vertical_scaling_step;
    pp_static_parameter->grf6.sampler_load_horizontal_frame_origin = (float) src_rect->x / src_width -
        (float) pp_avs_context->dest_x * pp_avs_context->horiz_range / dw;

    gen7_update_src_surface_uv_offset(ctx, pp_context, dst_surface);

    dst_surface->flags = src_surface->flags;

    return VA_STATUS_SUCCESS;
}

static int
pp_dndi_x_steps(void *private_context)
{
    return 1;
}

static int
pp_dndi_y_steps(void *private_context)
{
    struct pp_dndi_context *pp_dndi_context = private_context;

    return pp_dndi_context->dest_h / 4;
}

static int
pp_dndi_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;

    pp_inline_parameter->grf5.destination_block_horizontal_origin = x * 16;
    pp_inline_parameter->grf5.destination_block_vertical_origin = y * 4;

    return 0;
}

static VAStatus
pp_nv12_dndi_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                        const struct i965_surface *src_surface,
                        const VARectangle *src_rect,
                        struct i965_surface *dst_surface,
                        const VARectangle *dst_rect,
                        void *filter_param)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct pp_dndi_context *pp_dndi_context = (struct pp_dndi_context *)&pp_context->pp_dndi_context;
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;
    struct pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    struct object_surface *previous_in_obj_surface, *current_in_obj_surface, *previous_out_obj_surface, *current_out_obj_surface;
    struct i965_sampler_dndi *sampler_dndi;
    int index;
    int w, h;
    int orig_w, orig_h;
    int dndi_top_first = 1;
    VAProcFilterParameterBufferDeinterlacing *di_filter_param = (VAProcFilterParameterBufferDeinterlacing *)filter_param;
    int is_first_frame = (pp_dndi_context->frame_order == -1);

    if (di_filter_param->flags & VA_DEINTERLACING_BOTTOM_FIELD)
        dndi_top_first = 0;
    else
        dndi_top_first = 1;

    /* surface */
    current_in_obj_surface = (struct object_surface *)src_surface->base;

    if (di_filter_param->algorithm == VAProcDeinterlacingBob) {
        previous_in_obj_surface = current_in_obj_surface;
        is_first_frame = 1;
    } else if (di_filter_param->algorithm == VAProcDeinterlacingMotionAdaptive) {
        if (pp_dndi_context->frame_order == 0) {
            VAProcPipelineParameterBuffer *pipeline_param = pp_context->pipeline_param;
            if (!pipeline_param ||
                !pipeline_param->num_forward_references ||
                pipeline_param->forward_references[0] == VA_INVALID_ID) {
                WARN_ONCE("A forward temporal reference is needed for Motion adaptive deinterlacing !!!\n");

                return VA_STATUS_ERROR_INVALID_PARAMETER;
            } else {
                previous_in_obj_surface = SURFACE(pipeline_param->forward_references[0]);
                assert(previous_in_obj_surface && previous_in_obj_surface->bo);

                is_first_frame = 0;
            }
        } else if (pp_dndi_context->frame_order == 1) {
            vpp_surface_convert(ctx,
                                pp_dndi_context->current_out_obj_surface,
                                (struct object_surface *)dst_surface->base);
            pp_dndi_context->frame_order = (pp_dndi_context->frame_order + 1) % 2;
            is_first_frame = 0;

            return VA_STATUS_SUCCESS_1;
        } else {
            previous_in_obj_surface = current_in_obj_surface;
            is_first_frame = 1;
        }
    } else {
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    /* source (temporal reference) YUV surface index 5 */
    orig_w = previous_in_obj_surface->orig_width;
    orig_h = previous_in_obj_surface->orig_height;
    w = previous_in_obj_surface->width;
    h = previous_in_obj_surface->height;
    i965_pp_set_surface2_state(ctx, pp_context,
                               previous_in_obj_surface->bo, 0,
                               orig_w, orig_h, w,
                               0, h,
                               SURFACE_FORMAT_PLANAR_420_8, 1,
                               5);

    /* source surface */
    orig_w = current_in_obj_surface->orig_width;
    orig_h = current_in_obj_surface->orig_height;
    w = current_in_obj_surface->width;
    h = current_in_obj_surface->height;

    /* source UV surface index 2 */
    i965_pp_set_surface_state(ctx, pp_context,
                              current_in_obj_surface->bo, w * h,
                              orig_w / 4, orig_h / 2, w, I965_SURFACEFORMAT_R8G8_UNORM,
                              2, 0);

    /* source YUV surface index 4 */
    i965_pp_set_surface2_state(ctx, pp_context,
                               current_in_obj_surface->bo, 0,
                               orig_w, orig_h, w,
                               0, h,
                               SURFACE_FORMAT_PLANAR_420_8, 1,
                               4);

    /* source STMM surface index 6 */
    if (pp_dndi_context->stmm_bo == NULL) {
        pp_dndi_context->stmm_bo = dri_bo_alloc(i965->intel.bufmgr,
                                                "STMM surface",
                                                w * h,
                                                4096);
        assert(pp_dndi_context->stmm_bo);
    }

    i965_pp_set_surface_state(ctx, pp_context,
                              pp_dndi_context->stmm_bo, 0,
                              orig_w, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              6, 0);

    /* destination (Previous frame) */
    previous_out_obj_surface = (struct object_surface *)dst_surface->base;
    orig_w = previous_out_obj_surface->orig_width;
    orig_h = previous_out_obj_surface->orig_height;
    w = previous_out_obj_surface->width;
    h = previous_out_obj_surface->height;

    if (is_first_frame) {
        current_out_obj_surface = previous_out_obj_surface;
    } else {
        VAStatus va_status;

        if (pp_dndi_context->current_out_surface == VA_INVALID_SURFACE) {
            unsigned int tiling = 0, swizzle = 0;
            dri_bo_get_tiling(previous_out_obj_surface->bo, &tiling, &swizzle);

            va_status = i965_CreateSurfaces(ctx,
                                            orig_w,
                                            orig_h,
                                            VA_RT_FORMAT_YUV420,
                                            1,
                                            &pp_dndi_context->current_out_surface);
            assert(va_status == VA_STATUS_SUCCESS);
            pp_dndi_context->current_out_obj_surface = SURFACE(pp_dndi_context->current_out_surface);
            assert(pp_dndi_context->current_out_obj_surface);
            i965_check_alloc_surface_bo(ctx,
                                        pp_dndi_context->current_out_obj_surface,
                                        tiling != I915_TILING_NONE,
                                        VA_FOURCC_NV12,
                                        SUBSAMPLE_YUV420);
        }

        current_out_obj_surface = pp_dndi_context->current_out_obj_surface;
    }

    /* destination (Previous frame) Y surface index 7 */
    i965_pp_set_surface_state(ctx, pp_context,
                              previous_out_obj_surface->bo, 0,
                              orig_w / 4, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              7, 1);

    /* destination (Previous frame) UV surface index 8 */
    i965_pp_set_surface_state(ctx, pp_context,
                              previous_out_obj_surface->bo, w * h,
                              orig_w / 4, orig_h / 2, w, I965_SURFACEFORMAT_R8G8_UNORM,
                              8, 1);

    /* destination(Current frame) */
    orig_w = current_out_obj_surface->orig_width;
    orig_h = current_out_obj_surface->orig_height;
    w = current_out_obj_surface->width;
    h = current_out_obj_surface->height;

    /* destination (Current frame) Y surface index xxx */
    i965_pp_set_surface_state(ctx, pp_context,
                              current_out_obj_surface->bo, 0,
                              orig_w / 4, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              10, 1);

    /* destination (Current frame) UV surface index xxx */
    i965_pp_set_surface_state(ctx, pp_context,
                              current_out_obj_surface->bo, w * h,
                              orig_w / 4, orig_h / 2, w, I965_SURFACEFORMAT_R8G8_UNORM,
                              11, 1);

    /* STMM output surface, index 20 */
    i965_pp_set_surface_state(ctx, pp_context,
                              pp_dndi_context->stmm_bo, 0,
                              orig_w, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              20, 1);

    /* sampler dndi */
    dri_bo_map(pp_context->sampler_state_table.bo, True);
    assert(pp_context->sampler_state_table.bo->virtual);
    assert(sizeof(*sampler_dndi) == sizeof(int) * 8);
    sampler_dndi = pp_context->sampler_state_table.bo->virtual;

    /* sample dndi index 1 */
    index = 0;
    sampler_dndi[index].dw0.denoise_asd_threshold = 38;
    sampler_dndi[index].dw0.denoise_history_delta = 7;          // 0-15, default is 8
    sampler_dndi[index].dw0.denoise_maximum_history = 192;      // 128-240
    sampler_dndi[index].dw0.denoise_stad_threshold = 140;

    sampler_dndi[index].dw1.denoise_threshold_for_sum_of_complexity_measure = 38;
    sampler_dndi[index].dw1.denoise_moving_pixel_threshold = 1;
    sampler_dndi[index].dw1.stmm_c2 = 1;
    sampler_dndi[index].dw1.low_temporal_difference_threshold = 0;
    sampler_dndi[index].dw1.temporal_difference_threshold = 0;

    sampler_dndi[index].dw2.block_noise_estimate_noise_threshold = 20;   // 0-31
    sampler_dndi[index].dw2.block_noise_estimate_edge_threshold = 1;    // 0-15
    sampler_dndi[index].dw2.denoise_edge_threshold = 7;                 // 0-15
    sampler_dndi[index].dw2.good_neighbor_threshold = 12;                // 0-63

    sampler_dndi[index].dw3.maximum_stmm = 150;
    sampler_dndi[index].dw3.multipler_for_vecm = 30;
    sampler_dndi[index].dw3.blending_constant_across_time_for_small_values_of_stmm = 125;
    sampler_dndi[index].dw3.blending_constant_across_time_for_large_values_of_stmm = 64;
    sampler_dndi[index].dw3.stmm_blending_constant_select = 0;

    sampler_dndi[index].dw4.sdi_delta = 5;
    sampler_dndi[index].dw4.sdi_threshold = 100;
    sampler_dndi[index].dw4.stmm_output_shift = 5;                      // stmm_max - stmm_min = 2 ^ stmm_output_shift
    sampler_dndi[index].dw4.stmm_shift_up = 1;
    sampler_dndi[index].dw4.stmm_shift_down = 0;
    sampler_dndi[index].dw4.minimum_stmm = 118;

    sampler_dndi[index].dw5.fmd_temporal_difference_threshold = 175;
    sampler_dndi[index].dw5.sdi_fallback_mode_2_constant = 37;
    sampler_dndi[index].dw5.sdi_fallback_mode_1_t2_constant = 100;
    sampler_dndi[index].dw5.sdi_fallback_mode_1_t1_constant = 50;

    sampler_dndi[index].dw6.dn_enable = 1;
    sampler_dndi[index].dw6.di_enable = 1;
    sampler_dndi[index].dw6.di_partial = 0;
    sampler_dndi[index].dw6.dndi_top_first = dndi_top_first;
    sampler_dndi[index].dw6.dndi_stream_id = 0;
    sampler_dndi[index].dw6.dndi_first_frame = is_first_frame;
    sampler_dndi[index].dw6.progressive_dn = 0;
    sampler_dndi[index].dw6.fmd_tear_threshold = 2;
    sampler_dndi[index].dw6.fmd2_vertical_difference_threshold = 100;
    sampler_dndi[index].dw6.fmd1_vertical_difference_threshold = 16;

    sampler_dndi[index].dw7.fmd_for_1st_field_of_current_frame = 0;
    sampler_dndi[index].dw7.fmd_for_2nd_field_of_previous_frame = 0;
    sampler_dndi[index].dw7.vdi_walker_enable = 0;
    sampler_dndi[index].dw7.column_width_minus1 = w / 16;

    dri_bo_unmap(pp_context->sampler_state_table.bo);

    /* private function & data */
    pp_context->pp_x_steps = pp_dndi_x_steps;
    pp_context->pp_y_steps = pp_dndi_y_steps;
    pp_context->private_context = &pp_context->pp_dndi_context;
    pp_context->pp_set_block_parameter = pp_dndi_set_block_parameter;

    pp_static_parameter->grf1.statistics_surface_picth = w / 2;
    pp_static_parameter->grf1.r1_6.di.top_field_first = dndi_top_first;
    pp_static_parameter->grf4.r4_2.di.motion_history_coefficient_m2 = 0;
    pp_static_parameter->grf4.r4_2.di.motion_history_coefficient_m1 = 0;

    pp_inline_parameter->grf5.block_count_x = w / 16;   /* 1 x N */
    pp_inline_parameter->grf5.number_blocks = w / 16;
    pp_inline_parameter->grf5.block_vertical_mask = 0xff;
    pp_inline_parameter->grf5.block_horizontal_mask = 0xffff;

    pp_dndi_context->dest_w = w;
    pp_dndi_context->dest_h = h;

    dst_surface->flags = I965_SURFACE_FLAG_FRAME;

    pp_dndi_context->frame_order = (pp_dndi_context->frame_order + 1) % 2;

    return VA_STATUS_SUCCESS;
}

static int
pp_dn_x_steps(void *private_context)
{
    return 1;
}

static int
pp_dn_y_steps(void *private_context)
{
    struct pp_dn_context *pp_dn_context = private_context;

    return pp_dn_context->dest_h / 8;
}

static int
pp_dn_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;

    pp_inline_parameter->grf5.destination_block_horizontal_origin = x * 16;
    pp_inline_parameter->grf5.destination_block_vertical_origin = y * 8;

    return 0;
}

static VAStatus
pp_nv12_dn_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                      const struct i965_surface *src_surface,
                      const VARectangle *src_rect,
                      struct i965_surface *dst_surface,
                      const VARectangle *dst_rect,
                      void *filter_param)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct pp_dn_context *pp_dn_context = (struct pp_dn_context *)&pp_context->pp_dn_context;
    struct object_surface *obj_surface;
    struct i965_sampler_dndi *sampler_dndi;
    struct pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;
    VAProcFilterParameterBuffer *dn_filter_param = filter_param; /* FIXME: parameter */
    int index;
    int w, h;
    int orig_w, orig_h;
    int dn_strength = 15;
    int dndi_top_first = 1;
    int dn_progressive = 0;

    if (src_surface->flags == I965_SURFACE_FLAG_FRAME) {
        dndi_top_first = 1;
        dn_progressive = 1;
    } else if (src_surface->flags == I965_SURFACE_FLAG_TOP_FIELD_FIRST) {
        dndi_top_first = 1;
        dn_progressive = 0;
    } else {
        dndi_top_first = 0;
        dn_progressive = 0;
    }

    if (dn_filter_param) {
        float value = dn_filter_param->value;
        
        if (value > 1.0)
            value = 1.0;
        
        if (value < 0.0)
            value = 0.0;

        dn_strength = (int)(value * 31.0F);
    }

    /* surface */
    obj_surface = (struct object_surface *)src_surface->base;
    orig_w = obj_surface->orig_width;
    orig_h = obj_surface->orig_height;
    w = obj_surface->width;
    h = obj_surface->height;

    if (pp_dn_context->stmm_bo == NULL) {
        pp_dn_context->stmm_bo = dri_bo_alloc(i965->intel.bufmgr,
                                              "STMM surface",
                                              w * h,
                                              4096);
        assert(pp_dn_context->stmm_bo);
    }

    /* source UV surface index 2 */
    i965_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, w * h,
                              orig_w / 4, orig_h / 2, w, I965_SURFACEFORMAT_R8G8_UNORM,
                              2, 0);

    /* source YUV surface index 4 */
    i965_pp_set_surface2_state(ctx, pp_context,
                               obj_surface->bo, 0,
                               orig_w, orig_h, w,
                               0, h,
                               SURFACE_FORMAT_PLANAR_420_8, 1,
                               4);

    /* source STMM surface index 20 */
    i965_pp_set_surface_state(ctx, pp_context,
                              pp_dn_context->stmm_bo, 0,
                              orig_w, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              20, 1);

    /* destination surface */
    obj_surface = (struct object_surface *)dst_surface->base;
    orig_w = obj_surface->orig_width;
    orig_h = obj_surface->orig_height;
    w = obj_surface->width;
    h = obj_surface->height;

    /* destination Y surface index 7 */
    i965_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, 0,
                              orig_w / 4, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              7, 1);

    /* destination UV surface index 8 */
    i965_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, w * h,
                              orig_w / 4, orig_h / 2, w, I965_SURFACEFORMAT_R8G8_UNORM,
                              8, 1);
    /* sampler dn */
    dri_bo_map(pp_context->sampler_state_table.bo, True);
    assert(pp_context->sampler_state_table.bo->virtual);
    assert(sizeof(*sampler_dndi) == sizeof(int) * 8);
    sampler_dndi = pp_context->sampler_state_table.bo->virtual;

    /* sample dndi index 1 */
    index = 0;
    sampler_dndi[index].dw0.denoise_asd_threshold = 0;
    sampler_dndi[index].dw0.denoise_history_delta = 8;          // 0-15, default is 8
    sampler_dndi[index].dw0.denoise_maximum_history = 128;      // 128-240
    sampler_dndi[index].dw0.denoise_stad_threshold = 0;

    sampler_dndi[index].dw1.denoise_threshold_for_sum_of_complexity_measure = 64;
    sampler_dndi[index].dw1.denoise_moving_pixel_threshold = 0;
    sampler_dndi[index].dw1.stmm_c2 = 0;
    sampler_dndi[index].dw1.low_temporal_difference_threshold = 8;
    sampler_dndi[index].dw1.temporal_difference_threshold = 16;

    sampler_dndi[index].dw2.block_noise_estimate_noise_threshold = dn_strength;   // 0-31
    sampler_dndi[index].dw2.block_noise_estimate_edge_threshold = 7;    // 0-15
    sampler_dndi[index].dw2.denoise_edge_threshold = 7;                 // 0-15
    sampler_dndi[index].dw2.good_neighbor_threshold = 7;                // 0-63

    sampler_dndi[index].dw3.maximum_stmm = 128;
    sampler_dndi[index].dw3.multipler_for_vecm = 2;
    sampler_dndi[index].dw3.blending_constant_across_time_for_small_values_of_stmm = 0;
    sampler_dndi[index].dw3.blending_constant_across_time_for_large_values_of_stmm = 64;
    sampler_dndi[index].dw3.stmm_blending_constant_select = 0;

    sampler_dndi[index].dw4.sdi_delta = 8;
    sampler_dndi[index].dw4.sdi_threshold = 128;
    sampler_dndi[index].dw4.stmm_output_shift = 7;                      // stmm_max - stmm_min = 2 ^ stmm_output_shift
    sampler_dndi[index].dw4.stmm_shift_up = 0;
    sampler_dndi[index].dw4.stmm_shift_down = 0;
    sampler_dndi[index].dw4.minimum_stmm = 0;

    sampler_dndi[index].dw5.fmd_temporal_difference_threshold = 0;
    sampler_dndi[index].dw5.sdi_fallback_mode_2_constant = 0;
    sampler_dndi[index].dw5.sdi_fallback_mode_1_t2_constant = 0;
    sampler_dndi[index].dw5.sdi_fallback_mode_1_t1_constant = 0;

    sampler_dndi[index].dw6.dn_enable = 1;
    sampler_dndi[index].dw6.di_enable = 0;
    sampler_dndi[index].dw6.di_partial = 0;
    sampler_dndi[index].dw6.dndi_top_first = dndi_top_first;
    sampler_dndi[index].dw6.dndi_stream_id = 1;
    sampler_dndi[index].dw6.dndi_first_frame = 1;
    sampler_dndi[index].dw6.progressive_dn = dn_progressive;
    sampler_dndi[index].dw6.fmd_tear_threshold = 32;
    sampler_dndi[index].dw6.fmd2_vertical_difference_threshold = 32;
    sampler_dndi[index].dw6.fmd1_vertical_difference_threshold = 32;

    sampler_dndi[index].dw7.fmd_for_1st_field_of_current_frame = 2;
    sampler_dndi[index].dw7.fmd_for_2nd_field_of_previous_frame = 1;
    sampler_dndi[index].dw7.vdi_walker_enable = 0;
    sampler_dndi[index].dw7.column_width_minus1 = w / 16;

    dri_bo_unmap(pp_context->sampler_state_table.bo);

    /* private function & data */
    pp_context->pp_x_steps = pp_dn_x_steps;
    pp_context->pp_y_steps = pp_dn_y_steps;
    pp_context->private_context = &pp_context->pp_dn_context;
    pp_context->pp_set_block_parameter = pp_dn_set_block_parameter;

    pp_static_parameter->grf1.statistics_surface_picth = w / 2;
    pp_static_parameter->grf1.r1_6.di.top_field_first = 0;
    pp_static_parameter->grf4.r4_2.di.motion_history_coefficient_m2 = 64;
    pp_static_parameter->grf4.r4_2.di.motion_history_coefficient_m1 = 192;

    pp_inline_parameter->grf5.block_count_x = w / 16;   /* 1 x N */
    pp_inline_parameter->grf5.number_blocks = w / 16;
    pp_inline_parameter->grf5.block_vertical_mask = 0xff;
    pp_inline_parameter->grf5.block_horizontal_mask = 0xffff;

    pp_dn_context->dest_w = w;
    pp_dn_context->dest_h = h;

    dst_surface->flags = src_surface->flags;
    
    return VA_STATUS_SUCCESS;
}

static int
gen7_pp_dndi_x_steps(void *private_context)
{
    struct pp_dndi_context *pp_dndi_context = private_context;

    return pp_dndi_context->dest_w / 16;
}

static int
gen7_pp_dndi_y_steps(void *private_context)
{
    struct pp_dndi_context *pp_dndi_context = private_context;

    return pp_dndi_context->dest_h / 4;
}

static int
gen7_pp_dndi_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    struct gen7_pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;

    pp_inline_parameter->grf7.destination_block_horizontal_origin = x * 16;
    pp_inline_parameter->grf7.destination_block_vertical_origin = y * 4;

    return 0;
}

static VAStatus
gen7_pp_nv12_dndi_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                             const struct i965_surface *src_surface,
                             const VARectangle *src_rect,
                             struct i965_surface *dst_surface,
                             const VARectangle *dst_rect,
                             void *filter_param)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct pp_dndi_context *pp_dndi_context = (struct pp_dndi_context *)&pp_context->pp_dndi_context;
    struct gen7_pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    struct object_surface *previous_in_obj_surface, *current_in_obj_surface, *previous_out_obj_surface, *current_out_obj_surface;
    struct gen7_sampler_dndi *sampler_dndi;
    int index;
    int w, h;
    int orig_w, orig_h;
    int dndi_top_first = 1;
    VAProcFilterParameterBufferDeinterlacing *di_filter_param = (VAProcFilterParameterBufferDeinterlacing *)filter_param;
    int is_first_frame = (pp_dndi_context->frame_order == -1);

    if (di_filter_param->flags & VA_DEINTERLACING_BOTTOM_FIELD)
        dndi_top_first = 0;
    else
        dndi_top_first = 1;

    /* surface */
    current_in_obj_surface = (struct object_surface *)src_surface->base;

    if (di_filter_param->algorithm == VAProcDeinterlacingBob) {
        previous_in_obj_surface = current_in_obj_surface;
        is_first_frame = 1;
    } else if (di_filter_param->algorithm == VAProcDeinterlacingMotionAdaptive) {
        if (pp_dndi_context->frame_order == 0) {
            VAProcPipelineParameterBuffer *pipeline_param = pp_context->pipeline_param;
            if (!pipeline_param ||
                !pipeline_param->num_forward_references ||
                pipeline_param->forward_references[0] == VA_INVALID_ID) {
                WARN_ONCE("A forward temporal reference is needed for Motion adaptive deinterlacing !!!\n");

                return VA_STATUS_ERROR_INVALID_PARAMETER;
            } else {
                previous_in_obj_surface = SURFACE(pipeline_param->forward_references[0]);
                assert(previous_in_obj_surface && previous_in_obj_surface->bo);

                is_first_frame = 0;
            }
        } else if (pp_dndi_context->frame_order == 1) {
            vpp_surface_convert(ctx,
                                pp_dndi_context->current_out_obj_surface,
                                (struct object_surface *)dst_surface->base);
            pp_dndi_context->frame_order = (pp_dndi_context->frame_order + 1) % 2;
            is_first_frame = 0;

            return VA_STATUS_SUCCESS_1;
        } else {
            previous_in_obj_surface = current_in_obj_surface;
            is_first_frame = 1;
        }
    } else {
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    /* source (temporal reference) YUV surface index 4 */
    orig_w = previous_in_obj_surface->orig_width;
    orig_h = previous_in_obj_surface->orig_height;
    w = previous_in_obj_surface->width;
    h = previous_in_obj_surface->height;
    gen7_pp_set_surface2_state(ctx, pp_context,
                               previous_in_obj_surface->bo, 0,
                               orig_w, orig_h, w,
                               0, h,
                               SURFACE_FORMAT_PLANAR_420_8, 1,
                               4);

    /* source surface */
    orig_w = current_in_obj_surface->orig_width;
    orig_h = current_in_obj_surface->orig_height;
    w = current_in_obj_surface->width;
    h = current_in_obj_surface->height;

    /* source UV surface index 1 */
    gen7_pp_set_surface_state(ctx, pp_context,
                              current_in_obj_surface->bo, w * h,
                              orig_w / 4, orig_h / 2, w, I965_SURFACEFORMAT_R8G8_UNORM,
                              1, 0);

    /* source YUV surface index 3 */
    gen7_pp_set_surface2_state(ctx, pp_context,
                               current_in_obj_surface->bo, 0,
                               orig_w, orig_h, w,
                               0, h,
                               SURFACE_FORMAT_PLANAR_420_8, 1,
                               3);

    /* STMM / History Statistics input surface, index 5 */
    if (pp_dndi_context->stmm_bo == NULL) {
        pp_dndi_context->stmm_bo = dri_bo_alloc(i965->intel.bufmgr,
                                                "STMM surface",
                                                w * h,
                                                4096);
        assert(pp_dndi_context->stmm_bo);
    }

    gen7_pp_set_surface_state(ctx, pp_context,
                              pp_dndi_context->stmm_bo, 0,
                              orig_w, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              5, 1);

    /* destination surface */
    previous_out_obj_surface = (struct object_surface *)dst_surface->base;
    orig_w = previous_out_obj_surface->orig_width;
    orig_h = previous_out_obj_surface->orig_height;
    w = previous_out_obj_surface->width;
    h = previous_out_obj_surface->height;

    if (is_first_frame) {
        current_out_obj_surface = previous_out_obj_surface;
    } else {
        VAStatus va_status;

        if (pp_dndi_context->current_out_surface == VA_INVALID_SURFACE) {
            unsigned int tiling = 0, swizzle = 0;
            dri_bo_get_tiling(previous_out_obj_surface->bo, &tiling, &swizzle);

            va_status = i965_CreateSurfaces(ctx,
                                            orig_w,
                                            orig_h,
                                            VA_RT_FORMAT_YUV420,
                                            1,
                                            &pp_dndi_context->current_out_surface);
            assert(va_status == VA_STATUS_SUCCESS);
            pp_dndi_context->current_out_obj_surface = SURFACE(pp_dndi_context->current_out_surface);
            assert(pp_dndi_context->current_out_obj_surface);
            i965_check_alloc_surface_bo(ctx,
                                        pp_dndi_context->current_out_obj_surface,
                                        tiling != I915_TILING_NONE,
                                        VA_FOURCC_NV12,
                                        SUBSAMPLE_YUV420);
        }

        current_out_obj_surface = pp_dndi_context->current_out_obj_surface;
    }

    /* destination(Previous frame) Y surface index 27 */
    gen7_pp_set_surface_state(ctx, pp_context,
                              previous_out_obj_surface->bo, 0,
                              orig_w / 4, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              27, 1);

    /* destination(Previous frame) UV surface index 28 */
    gen7_pp_set_surface_state(ctx, pp_context,
                              previous_out_obj_surface->bo, w * h,
                              orig_w / 4, orig_h / 2, w, I965_SURFACEFORMAT_R8G8_UNORM,
                              28, 1);

    /* destination(Current frame) Y surface index 30 */
    gen7_pp_set_surface_state(ctx, pp_context,
                              current_out_obj_surface->bo, 0,
                              orig_w / 4, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              30, 1);

    /* destination(Current frame) UV surface index 31 */
    orig_w = current_out_obj_surface->orig_width;
    orig_h = current_out_obj_surface->orig_height;
    w = current_out_obj_surface->width;
    h = current_out_obj_surface->height;

    gen7_pp_set_surface_state(ctx, pp_context,
                              current_out_obj_surface->bo, w * h,
                              orig_w / 4, orig_h / 2, w, I965_SURFACEFORMAT_R8G8_UNORM,
                              31, 1);

    /* STMM output surface, index 33 */
    gen7_pp_set_surface_state(ctx, pp_context,
                              pp_dndi_context->stmm_bo, 0,
                              orig_w, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              33, 1);


    /* sampler dndi */
    dri_bo_map(pp_context->sampler_state_table.bo, True);
    assert(pp_context->sampler_state_table.bo->virtual);
    assert(sizeof(*sampler_dndi) == sizeof(int) * 8);
    sampler_dndi = pp_context->sampler_state_table.bo->virtual;

    /* sample dndi index 0 */
    index = 0;
    sampler_dndi[index].dw0.denoise_asd_threshold = 38;
    sampler_dndi[index].dw0.dnmh_delt = 7;
    sampler_dndi[index].dw0.vdi_walker_y_stride = 0;
    sampler_dndi[index].dw0.vdi_walker_frame_sharing_enable = 0;
    sampler_dndi[index].dw0.denoise_maximum_history = 192;      // 128-240
    sampler_dndi[index].dw0.denoise_stad_threshold = 140;

    sampler_dndi[index].dw1.denoise_threshold_for_sum_of_complexity_measure = 38;
    sampler_dndi[index].dw1.denoise_moving_pixel_threshold = 1;
    sampler_dndi[index].dw1.stmm_c2 = 2;
    sampler_dndi[index].dw1.low_temporal_difference_threshold = 0;
    sampler_dndi[index].dw1.temporal_difference_threshold = 0;

    sampler_dndi[index].dw2.block_noise_estimate_noise_threshold = 20;   // 0-31
    sampler_dndi[index].dw2.bne_edge_th = 1;
    sampler_dndi[index].dw2.smooth_mv_th = 0;
    sampler_dndi[index].dw2.sad_tight_th = 5;
    sampler_dndi[index].dw2.cat_slope_minus1 = 9;
    sampler_dndi[index].dw2.good_neighbor_th = 12;

    sampler_dndi[index].dw3.maximum_stmm = 150;
    sampler_dndi[index].dw3.multipler_for_vecm = 30;
    sampler_dndi[index].dw3.blending_constant_across_time_for_small_values_of_stmm = 125;
    sampler_dndi[index].dw3.blending_constant_across_time_for_large_values_of_stmm = 64;
    sampler_dndi[index].dw3.stmm_blending_constant_select = 0;

    sampler_dndi[index].dw4.sdi_delta = 5;
    sampler_dndi[index].dw4.sdi_threshold = 100;
    sampler_dndi[index].dw4.stmm_output_shift = 5;                      // stmm_max - stmm_min = 2 ^ stmm_output_shift
    sampler_dndi[index].dw4.stmm_shift_up = 1;
    sampler_dndi[index].dw4.stmm_shift_down = 0;
    sampler_dndi[index].dw4.minimum_stmm = 118;

    sampler_dndi[index].dw5.fmd_temporal_difference_threshold = 175;
    sampler_dndi[index].dw5.sdi_fallback_mode_2_constant = 37;
    sampler_dndi[index].dw5.sdi_fallback_mode_1_t2_constant = 100;
    sampler_dndi[index].dw5.sdi_fallback_mode_1_t1_constant = 50;
    sampler_dndi[index].dw6.dn_enable = 0;
    sampler_dndi[index].dw6.di_enable = 1;
    sampler_dndi[index].dw6.di_partial = 0;
    sampler_dndi[index].dw6.dndi_top_first = dndi_top_first;
    sampler_dndi[index].dw6.dndi_stream_id = 1;
    sampler_dndi[index].dw6.dndi_first_frame = is_first_frame;
    sampler_dndi[index].dw6.progressive_dn = 0;
    sampler_dndi[index].dw6.mcdi_enable = 0;
    sampler_dndi[index].dw6.fmd_tear_threshold = 2;
    sampler_dndi[index].dw6.cat_th1 = 0;
    sampler_dndi[index].dw6.fmd2_vertical_difference_threshold = 100;
    sampler_dndi[index].dw6.fmd1_vertical_difference_threshold = 16;

    sampler_dndi[index].dw7.sad_tha = 5;
    sampler_dndi[index].dw7.sad_thb = 10;
    sampler_dndi[index].dw7.fmd_for_1st_field_of_current_frame = 0;
    sampler_dndi[index].dw7.mc_pixel_consistency_th = 25;
    sampler_dndi[index].dw7.fmd_for_2nd_field_of_previous_frame = 0;
    sampler_dndi[index].dw7.vdi_walker_enable = 0;
    sampler_dndi[index].dw7.neighborpixel_th = 10;
    sampler_dndi[index].dw7.column_width_minus1 = w / 16;

    dri_bo_unmap(pp_context->sampler_state_table.bo);

    /* private function & data */
    pp_context->pp_x_steps = gen7_pp_dndi_x_steps;
    pp_context->pp_y_steps = gen7_pp_dndi_y_steps;
    pp_context->private_context = &pp_context->pp_dndi_context;
    pp_context->pp_set_block_parameter = gen7_pp_dndi_set_block_parameter;

    pp_static_parameter->grf1.di_statistics_surface_pitch_div2 = w / 2;
    pp_static_parameter->grf1.di_statistics_surface_height_div4 = h / 4;
    pp_static_parameter->grf1.di_top_field_first = 0;
    pp_static_parameter->grf1.pointer_to_inline_parameter = 7;

    pp_static_parameter->grf2.di_destination_packed_y_component_offset = 0;
    pp_static_parameter->grf2.di_destination_packed_u_component_offset = 1;
    pp_static_parameter->grf2.di_destination_packed_v_component_offset = 3;

    pp_static_parameter->grf4.di_hoffset_svf_from_dvf = 0;
    pp_static_parameter->grf4.di_voffset_svf_from_dvf = 0;

    pp_dndi_context->dest_w = w;
    pp_dndi_context->dest_h = h;

    dst_surface->flags = I965_SURFACE_FLAG_FRAME;

    pp_dndi_context->frame_order = (pp_dndi_context->frame_order + 1) % 2;

    return VA_STATUS_SUCCESS;
}

static int
gen7_pp_dn_x_steps(void *private_context)
{
    struct pp_dn_context *pp_dn_context = private_context;

    return pp_dn_context->dest_w / 16;
}

static int
gen7_pp_dn_y_steps(void *private_context)
{
    struct pp_dn_context *pp_dn_context = private_context;

    return pp_dn_context->dest_h / 4;
}

static int
gen7_pp_dn_set_block_parameter(struct i965_post_processing_context *pp_context, int x, int y)
{
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;

    pp_inline_parameter->grf5.destination_block_horizontal_origin = x * 16;
    pp_inline_parameter->grf5.destination_block_vertical_origin = y * 4;

    return 0;
}

static VAStatus
gen7_pp_nv12_dn_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                           const struct i965_surface *src_surface,
                           const VARectangle *src_rect,
                           struct i965_surface *dst_surface,
                           const VARectangle *dst_rect,
                           void *filter_param)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct pp_dn_context *pp_dn_context = (struct pp_dn_context *)&pp_context->pp_dn_context;
    struct gen7_pp_static_parameter *pp_static_parameter = pp_context->pp_static_parameter;
    struct object_surface *obj_surface;
    struct gen7_sampler_dndi *sampler_dn;
    VAProcFilterParameterBuffer *dn_filter_param = filter_param; /* FIXME: parameter */
    int index;
    int w, h;
    int orig_w, orig_h;
    int dn_strength = 15;
    int dndi_top_first = 1;
    int dn_progressive = 0;

    if (src_surface->flags == I965_SURFACE_FLAG_FRAME) {
        dndi_top_first = 1;
        dn_progressive = 1;
    } else if (src_surface->flags == I965_SURFACE_FLAG_TOP_FIELD_FIRST) {
        dndi_top_first = 1;
        dn_progressive = 0;
    } else {
        dndi_top_first = 0;
        dn_progressive = 0;
    }

    if (dn_filter_param) {
        float value = dn_filter_param->value;
        
        if (value > 1.0)
            value = 1.0;
        
        if (value < 0.0)
            value = 0.0;

        dn_strength = (int)(value * 31.0F);
    }

    /* surface */
    obj_surface = (struct object_surface *)src_surface->base;
    orig_w = obj_surface->orig_width;
    orig_h = obj_surface->orig_height;
    w = obj_surface->width;
    h = obj_surface->height;

    if (pp_dn_context->stmm_bo == NULL) {
        pp_dn_context->stmm_bo= dri_bo_alloc(i965->intel.bufmgr,
                                             "STMM surface",
                                             w * h,
                                             4096);
        assert(pp_dn_context->stmm_bo);
    }

    /* source UV surface index 1 */
    gen7_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, w * h,
                              orig_w / 4, orig_h / 2, w, I965_SURFACEFORMAT_R8G8_UNORM,
                              1, 0);

    /* source YUV surface index 3 */
    gen7_pp_set_surface2_state(ctx, pp_context,
                               obj_surface->bo, 0,
                               orig_w, orig_h, w,
                               0, h,
                               SURFACE_FORMAT_PLANAR_420_8, 1,
                               3);

    /* source (temporal reference) YUV surface index 4 */
    gen7_pp_set_surface2_state(ctx, pp_context,
                               obj_surface->bo, 0,
                               orig_w, orig_h, w,
                               0, h,
                               SURFACE_FORMAT_PLANAR_420_8, 1,
                               4);

    /* STMM / History Statistics input surface, index 5 */
    gen7_pp_set_surface_state(ctx, pp_context,
                              pp_dn_context->stmm_bo, 0,
                              orig_w, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              33, 1);

    /* destination surface */
    obj_surface = (struct object_surface *)dst_surface->base;
    orig_w = obj_surface->orig_width;
    orig_h = obj_surface->orig_height;
    w = obj_surface->width;
    h = obj_surface->height;

    /* destination Y surface index 24 */
    gen7_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, 0,
                              orig_w / 4, orig_h, w, I965_SURFACEFORMAT_R8_UNORM,
                              24, 1);

    /* destination UV surface index 25 */
    gen7_pp_set_surface_state(ctx, pp_context,
                              obj_surface->bo, w * h,
                              orig_w / 4, orig_h / 2, w, I965_SURFACEFORMAT_R8G8_UNORM,
                              25, 1);

    /* sampler dn */
    dri_bo_map(pp_context->sampler_state_table.bo, True);
    assert(pp_context->sampler_state_table.bo->virtual);
    assert(sizeof(*sampler_dn) == sizeof(int) * 8);
    sampler_dn = pp_context->sampler_state_table.bo->virtual;

    /* sample dn index 1 */
    index = 0;
    sampler_dn[index].dw0.denoise_asd_threshold = 0;
    sampler_dn[index].dw0.dnmh_delt = 8;
    sampler_dn[index].dw0.vdi_walker_y_stride = 0;
    sampler_dn[index].dw0.vdi_walker_frame_sharing_enable = 0;
    sampler_dn[index].dw0.denoise_maximum_history = 128;      // 128-240
    sampler_dn[index].dw0.denoise_stad_threshold = 0;

    sampler_dn[index].dw1.denoise_threshold_for_sum_of_complexity_measure = 64;
    sampler_dn[index].dw1.denoise_moving_pixel_threshold = 0;
    sampler_dn[index].dw1.stmm_c2 = 0;
    sampler_dn[index].dw1.low_temporal_difference_threshold = 8;
    sampler_dn[index].dw1.temporal_difference_threshold = 16;

    sampler_dn[index].dw2.block_noise_estimate_noise_threshold = dn_strength;   // 0-31
    sampler_dn[index].dw2.bne_edge_th = 1;
    sampler_dn[index].dw2.smooth_mv_th = 0;
    sampler_dn[index].dw2.sad_tight_th = 5;
    sampler_dn[index].dw2.cat_slope_minus1 = 9;
    sampler_dn[index].dw2.good_neighbor_th = 4;

    sampler_dn[index].dw3.maximum_stmm = 128;
    sampler_dn[index].dw3.multipler_for_vecm = 2;
    sampler_dn[index].dw3.blending_constant_across_time_for_small_values_of_stmm = 0;
    sampler_dn[index].dw3.blending_constant_across_time_for_large_values_of_stmm = 64;
    sampler_dn[index].dw3.stmm_blending_constant_select = 0;

    sampler_dn[index].dw4.sdi_delta = 8;
    sampler_dn[index].dw4.sdi_threshold = 128;
    sampler_dn[index].dw4.stmm_output_shift = 7;                      // stmm_max - stmm_min = 2 ^ stmm_output_shift
    sampler_dn[index].dw4.stmm_shift_up = 0;
    sampler_dn[index].dw4.stmm_shift_down = 0;
    sampler_dn[index].dw4.minimum_stmm = 0;

    sampler_dn[index].dw5.fmd_temporal_difference_threshold = 0;
    sampler_dn[index].dw5.sdi_fallback_mode_2_constant = 0;
    sampler_dn[index].dw5.sdi_fallback_mode_1_t2_constant = 0;
    sampler_dn[index].dw5.sdi_fallback_mode_1_t1_constant = 0;

    sampler_dn[index].dw6.dn_enable = 1;
    sampler_dn[index].dw6.di_enable = 0;
    sampler_dn[index].dw6.di_partial = 0;
    sampler_dn[index].dw6.dndi_top_first = dndi_top_first;
    sampler_dn[index].dw6.dndi_stream_id = 1;
    sampler_dn[index].dw6.dndi_first_frame = 1;
    sampler_dn[index].dw6.progressive_dn = dn_progressive;
    sampler_dn[index].dw6.mcdi_enable = 0;
    sampler_dn[index].dw6.fmd_tear_threshold = 32;
    sampler_dn[index].dw6.cat_th1 = 0;
    sampler_dn[index].dw6.fmd2_vertical_difference_threshold = 32;
    sampler_dn[index].dw6.fmd1_vertical_difference_threshold = 32;

    sampler_dn[index].dw7.sad_tha = 5;
    sampler_dn[index].dw7.sad_thb = 10;
    sampler_dn[index].dw7.fmd_for_1st_field_of_current_frame = 2;
    sampler_dn[index].dw7.mc_pixel_consistency_th = 25;
    sampler_dn[index].dw7.fmd_for_2nd_field_of_previous_frame = 1;
    sampler_dn[index].dw7.vdi_walker_enable = 0;
    sampler_dn[index].dw7.neighborpixel_th = 10;
    sampler_dn[index].dw7.column_width_minus1 = w / 16;

    dri_bo_unmap(pp_context->sampler_state_table.bo);

    /* private function & data */
    pp_context->pp_x_steps = gen7_pp_dn_x_steps;
    pp_context->pp_y_steps = gen7_pp_dn_y_steps;
    pp_context->private_context = &pp_context->pp_dn_context;
    pp_context->pp_set_block_parameter = gen7_pp_dn_set_block_parameter;

    pp_static_parameter->grf1.di_statistics_surface_pitch_div2 = w / 2;
    pp_static_parameter->grf1.di_statistics_surface_height_div4 = h / 4;
    pp_static_parameter->grf1.di_top_field_first = 0;
    pp_static_parameter->grf1.pointer_to_inline_parameter = 7;

    pp_static_parameter->grf2.di_destination_packed_y_component_offset = 0;
    pp_static_parameter->grf2.di_destination_packed_u_component_offset = 1;
    pp_static_parameter->grf2.di_destination_packed_v_component_offset = 3;

    pp_static_parameter->grf4.di_hoffset_svf_from_dvf = 0;
    pp_static_parameter->grf4.di_voffset_svf_from_dvf = 0;

    pp_dn_context->dest_w = w;
    pp_dn_context->dest_h = h;

    dst_surface->flags = src_surface->flags;

    return VA_STATUS_SUCCESS;
}

static VAStatus
ironlake_pp_initialize(
    VADriverContextP ctx,
    struct i965_post_processing_context *pp_context,
    const struct i965_surface *src_surface,
    const VARectangle *src_rect,
    struct i965_surface *dst_surface,
    const VARectangle *dst_rect,
    int pp_index,
    void *filter_param
)
{
    VAStatus va_status;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct pp_module *pp_module;
    dri_bo *bo;
    int static_param_size, inline_param_size;

    dri_bo_unreference(pp_context->surface_state_binding_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      (SURFACE_STATE_PADDED_SIZE + sizeof(unsigned int)) * MAX_PP_SURFACES,
                      4096);
    assert(bo);
    pp_context->surface_state_binding_table.bo = bo;

    dri_bo_unreference(pp_context->curbe.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "constant buffer",
                      4096, 
                      4096);
    assert(bo);
    pp_context->curbe.bo = bo;

    dri_bo_unreference(pp_context->idrt.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "interface discriptor", 
                      sizeof(struct i965_interface_descriptor), 
                      4096);
    assert(bo);
    pp_context->idrt.bo = bo;
    pp_context->idrt.num_interface_descriptors = 0;

    dri_bo_unreference(pp_context->sampler_state_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "sampler state table", 
                      4096,
                      4096);
    assert(bo);
    dri_bo_map(bo, True);
    memset(bo->virtual, 0, bo->size);
    dri_bo_unmap(bo);
    pp_context->sampler_state_table.bo = bo;

    dri_bo_unreference(pp_context->sampler_state_table.bo_8x8);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "sampler 8x8 state ",
                      4096,
                      4096);
    assert(bo);
    pp_context->sampler_state_table.bo_8x8 = bo;

    dri_bo_unreference(pp_context->sampler_state_table.bo_8x8_uv);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "sampler 8x8 state ",
                      4096,
                      4096);
    assert(bo);
    pp_context->sampler_state_table.bo_8x8_uv = bo;

    dri_bo_unreference(pp_context->vfe_state.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "vfe state", 
                      sizeof(struct i965_vfe_state), 
                      4096);
    assert(bo);
    pp_context->vfe_state.bo = bo;

    static_param_size = sizeof(struct pp_static_parameter);
    inline_param_size = sizeof(struct pp_inline_parameter);

    memset(pp_context->pp_static_parameter, 0, static_param_size);
    memset(pp_context->pp_inline_parameter, 0, inline_param_size);
    
    assert(pp_index >= PP_NULL && pp_index < NUM_PP_MODULES);
    pp_context->current_pp = pp_index;
    pp_module = &pp_context->pp_modules[pp_index];
    
    if (pp_module->initialize)
        va_status = pp_module->initialize(ctx, pp_context,
                                          src_surface,
                                          src_rect,
                                          dst_surface,
                                          dst_rect,
                                          filter_param);
    else
        va_status = VA_STATUS_ERROR_UNIMPLEMENTED;

    return va_status;
}

static VAStatus
ironlake_post_processing(
    VADriverContextP   ctx,
    struct i965_post_processing_context *pp_context,
    const struct i965_surface *src_surface,
    const VARectangle *src_rect,
    struct i965_surface *dst_surface,
    const VARectangle *dst_rect,
    int                pp_index,
    void *filter_param
)
{
    VAStatus va_status;

    va_status = ironlake_pp_initialize(ctx, pp_context,
                                       src_surface,
                                       src_rect,
                                       dst_surface,
                                       dst_rect,
                                       pp_index,
                                       filter_param);

    if (va_status == VA_STATUS_SUCCESS) {
        ironlake_pp_states_setup(ctx, pp_context);
        ironlake_pp_pipeline_setup(ctx, pp_context);
    }

    return va_status;
}

static VAStatus
gen6_pp_initialize(
    VADriverContextP ctx,
    struct i965_post_processing_context *pp_context,
    const struct i965_surface *src_surface,
    const VARectangle *src_rect,
    struct i965_surface *dst_surface,
    const VARectangle *dst_rect,
    int pp_index,
    void *filter_param
)
{
    VAStatus va_status;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct pp_module *pp_module;
    dri_bo *bo;
    int static_param_size, inline_param_size;

    dri_bo_unreference(pp_context->surface_state_binding_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      (SURFACE_STATE_PADDED_SIZE + sizeof(unsigned int)) * MAX_PP_SURFACES,
                      4096);
    assert(bo);
    pp_context->surface_state_binding_table.bo = bo;

    dri_bo_unreference(pp_context->curbe.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "constant buffer",
                      4096, 
                      4096);
    assert(bo);
    pp_context->curbe.bo = bo;

    dri_bo_unreference(pp_context->idrt.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "interface discriptor", 
                      sizeof(struct gen6_interface_descriptor_data), 
                      4096);
    assert(bo);
    pp_context->idrt.bo = bo;
    pp_context->idrt.num_interface_descriptors = 0;

    dri_bo_unreference(pp_context->sampler_state_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "sampler state table", 
                      4096,
                      4096);
    assert(bo);
    dri_bo_map(bo, True);
    memset(bo->virtual, 0, bo->size);
    dri_bo_unmap(bo);
    pp_context->sampler_state_table.bo = bo;

    dri_bo_unreference(pp_context->sampler_state_table.bo_8x8);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "sampler 8x8 state ",
                      4096,
                      4096);
    assert(bo);
    pp_context->sampler_state_table.bo_8x8 = bo;

    dri_bo_unreference(pp_context->sampler_state_table.bo_8x8_uv);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "sampler 8x8 state ",
                      4096,
                      4096);
    assert(bo);
    pp_context->sampler_state_table.bo_8x8_uv = bo;

    dri_bo_unreference(pp_context->vfe_state.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr, 
                      "vfe state", 
                      sizeof(struct i965_vfe_state), 
                      4096);
    assert(bo);
    pp_context->vfe_state.bo = bo;
    
    if (IS_GEN7(i965->intel.device_info)) {
        static_param_size = sizeof(struct gen7_pp_static_parameter);
        inline_param_size = sizeof(struct gen7_pp_inline_parameter);
    } else {
        static_param_size = sizeof(struct pp_static_parameter);
        inline_param_size = sizeof(struct pp_inline_parameter);
    }

    memset(pp_context->pp_static_parameter, 0, static_param_size);
    memset(pp_context->pp_inline_parameter, 0, inline_param_size);

    assert(pp_index >= PP_NULL && pp_index < NUM_PP_MODULES);
    pp_context->current_pp = pp_index;
    pp_module = &pp_context->pp_modules[pp_index];
    
    if (pp_module->initialize)
        va_status = pp_module->initialize(ctx, pp_context,
                                          src_surface,
                                          src_rect,
                                          dst_surface,
                                          dst_rect,
                                          filter_param);
    else
        va_status = VA_STATUS_ERROR_UNIMPLEMENTED;

    calculate_boundary_block_mask(pp_context, dst_rect);

    return va_status;
}


static void
gen6_pp_interface_descriptor_table(VADriverContextP   ctx,
                                   struct i965_post_processing_context *pp_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_interface_descriptor_data *desc;
    dri_bo *bo;
    int pp_index = pp_context->current_pp;

    bo = pp_context->idrt.bo;
    dri_bo_map(bo, True);
    assert(bo->virtual);
    desc = bo->virtual;
    memset(desc, 0, sizeof(*desc));
    desc->desc0.kernel_start_pointer = 
        pp_context->pp_modules[pp_index].kernel.bo->offset >> 6; /* reloc */
    desc->desc1.single_program_flow = 1;
    desc->desc1.floating_point_mode = FLOATING_POINT_IEEE_754;
    desc->desc2.sampler_count = 1;      /* 1 - 4 samplers used */
    desc->desc2.sampler_state_pointer = 
        pp_context->sampler_state_table.bo->offset >> 5;
    desc->desc3.binding_table_entry_count = 0;
    desc->desc3.binding_table_pointer = (BINDING_TABLE_OFFSET >> 5);
    desc->desc4.constant_urb_entry_read_offset = 0;

    if (IS_GEN7(i965->intel.device_info))
        desc->desc4.constant_urb_entry_read_length = 6; /* grf 1-6 */
    else
        desc->desc4.constant_urb_entry_read_length = 4; /* grf 1-4 */

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      0,
                      offsetof(struct gen6_interface_descriptor_data, desc0),
                      pp_context->pp_modules[pp_index].kernel.bo);

    dri_bo_emit_reloc(bo,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      desc->desc2.sampler_count << 2,
                      offsetof(struct gen6_interface_descriptor_data, desc2),
                      pp_context->sampler_state_table.bo);

    dri_bo_unmap(bo);
    pp_context->idrt.num_interface_descriptors++;
}

static void
gen6_pp_upload_constants(VADriverContextP ctx,
                         struct i965_post_processing_context *pp_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    unsigned char *constant_buffer;
    int param_size;

    assert(sizeof(struct pp_static_parameter) == 128);
    assert(sizeof(struct gen7_pp_static_parameter) == 192);

    if (IS_GEN7(i965->intel.device_info))
        param_size = sizeof(struct gen7_pp_static_parameter);
    else
        param_size = sizeof(struct pp_static_parameter);

    dri_bo_map(pp_context->curbe.bo, 1);
    assert(pp_context->curbe.bo->virtual);
    constant_buffer = pp_context->curbe.bo->virtual;
    memcpy(constant_buffer, pp_context->pp_static_parameter, param_size);
    dri_bo_unmap(pp_context->curbe.bo);
}

static void
gen6_pp_states_setup(VADriverContextP ctx,
                     struct i965_post_processing_context *pp_context)
{
    gen6_pp_interface_descriptor_table(ctx, pp_context);
    gen6_pp_upload_constants(ctx, pp_context);
}

static void
gen6_pp_pipeline_select(VADriverContextP ctx,
                        struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 1);
    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_MEDIA);
    ADVANCE_BATCH(batch);
}

static void
gen6_pp_state_base_address(VADriverContextP ctx,
                           struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 10);
    OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | (10 - 2));
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_RELOC(batch, pp_context->surface_state_binding_table.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY); /* Surface state base address */
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
    ADVANCE_BATCH(batch);
}

static void
gen6_pp_vfe_state(VADriverContextP ctx,
                  struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 8);
    OUT_BATCH(batch, CMD_MEDIA_VFE_STATE | (8 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch,
              (pp_context->vfe_gpu_state.max_num_threads - 1) << 16 |
              pp_context->vfe_gpu_state.num_urb_entries << 8);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch,
              (pp_context->vfe_gpu_state.urb_entry_size) << 16 |  
		/* URB Entry Allocation Size, in 256 bits unit */
              (pp_context->vfe_gpu_state.curbe_allocation_size));
		/* CURBE Allocation Size, in 256 bits unit */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen6_pp_curbe_load(VADriverContextP ctx,
                   struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int param_size;

    if (IS_GEN7(i965->intel.device_info))
        param_size = sizeof(struct gen7_pp_static_parameter);
    else
        param_size = sizeof(struct pp_static_parameter);

    BEGIN_BATCH(batch, 4);
    OUT_BATCH(batch, CMD_MEDIA_CURBE_LOAD | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch,
              param_size);
    OUT_RELOC(batch, 
              pp_context->curbe.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              0);
    ADVANCE_BATCH(batch);
}

static void
gen6_interface_descriptor_load(VADriverContextP ctx,
                               struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    BEGIN_BATCH(batch, 4);
    OUT_BATCH(batch, CMD_MEDIA_INTERFACE_DESCRIPTOR_LOAD | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch,
              pp_context->idrt.num_interface_descriptors * sizeof(struct gen6_interface_descriptor_data));
    OUT_RELOC(batch, 
              pp_context->idrt.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              0);
    ADVANCE_BATCH(batch);
}

static void update_block_mask_parameter(struct i965_post_processing_context *pp_context, int x, int y, int x_steps, int y_steps) 
{
    struct pp_inline_parameter *pp_inline_parameter = pp_context->pp_inline_parameter;

    pp_inline_parameter->grf5.block_vertical_mask = 0xff;
    pp_inline_parameter->grf6.block_vertical_mask_bottom = pp_context->block_vertical_mask_bottom;
    // for the first block, it always on the left edge. the second block will reload horizontal_mask from grf6.block_horizontal_mask_middle
    pp_inline_parameter->grf5.block_horizontal_mask = pp_context->block_horizontal_mask_left;
    pp_inline_parameter->grf6.block_horizontal_mask_middle = 0xffff;
    pp_inline_parameter->grf6.block_horizontal_mask_right = pp_context->block_horizontal_mask_right;

    /* 1 x N */
    if (x_steps == 1) {
        if (y == y_steps-1) {
            pp_inline_parameter->grf5.block_vertical_mask = pp_context->block_vertical_mask_bottom;
        }
        else {
            pp_inline_parameter->grf6.block_vertical_mask_bottom = 0xff;
        }
    }

    /* M x 1 */
    if (y_steps == 1) {
        if (x == 0) { // all blocks in this group are on the left edge
            pp_inline_parameter->grf6.block_horizontal_mask_middle = pp_context->block_horizontal_mask_left;
            pp_inline_parameter->grf6.block_horizontal_mask_right = pp_context->block_horizontal_mask_left; 
        }
        else if (x == x_steps-1) {
            pp_inline_parameter->grf5.block_horizontal_mask = pp_context->block_horizontal_mask_right;
            pp_inline_parameter->grf6.block_horizontal_mask_middle = pp_context->block_horizontal_mask_right;
        }
        else {
            pp_inline_parameter->grf5.block_horizontal_mask = 0xffff;
            pp_inline_parameter->grf6.block_horizontal_mask_middle = 0xffff;
            pp_inline_parameter->grf6.block_horizontal_mask_right = 0xffff;
        }
    }

}

static void
gen6_pp_object_walker(VADriverContextP ctx,
                      struct i965_post_processing_context *pp_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = pp_context->batch;
    int x, x_steps, y, y_steps;
    int param_size, command_length_in_dws;
    dri_bo *command_buffer;
    unsigned int *command_ptr;

    if (IS_GEN7(i965->intel.device_info))
        param_size = sizeof(struct gen7_pp_inline_parameter);
    else
        param_size = sizeof(struct pp_inline_parameter);

    x_steps = pp_context->pp_x_steps(pp_context->private_context);
    y_steps = pp_context->pp_y_steps(pp_context->private_context);
    command_length_in_dws = 6 + (param_size >> 2);
    command_buffer = dri_bo_alloc(i965->intel.bufmgr,
                                  "command objects buffer",
                                  command_length_in_dws * 4 * x_steps * y_steps + 8,
                                  4096);

    dri_bo_map(command_buffer, 1);
    command_ptr = command_buffer->virtual;

    for (y = 0; y < y_steps; y++) {
        for (x = 0; x < x_steps; x++) {
            if (!pp_context->pp_set_block_parameter(pp_context, x, y)) {
                // some common block parameter update goes here, apply to all pp functions
                if (IS_GEN6(i965->intel.device_info))
                    update_block_mask_parameter (pp_context, x, y, x_steps, y_steps);
                
                *command_ptr++ = (CMD_MEDIA_OBJECT | (command_length_in_dws - 2));
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                *command_ptr++ = 0;
                memcpy(command_ptr, pp_context->pp_inline_parameter, param_size);
                command_ptr += (param_size >> 2);
            }
        }
    }

    if (command_length_in_dws * x_steps * y_steps % 2 == 0)
        *command_ptr++ = 0;

    *command_ptr = MI_BATCH_BUFFER_END;

    dri_bo_unmap(command_buffer);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, MI_BATCH_BUFFER_START | (1 << 8));
    OUT_RELOC(batch, command_buffer,
              I915_GEM_DOMAIN_COMMAND, 0,
              0);
    ADVANCE_BATCH(batch);

    dri_bo_unreference(command_buffer);

    /* Have to execute the batch buffer here becuase MI_BATCH_BUFFER_END
     * will cause control to pass back to ring buffer 
     */
    intel_batchbuffer_end_atomic(batch);
    intel_batchbuffer_flush(batch);
    intel_batchbuffer_start_atomic(batch, 0x1000);
}

static void
gen6_pp_pipeline_setup(VADriverContextP ctx,
                       struct i965_post_processing_context *pp_context)
{
    struct intel_batchbuffer *batch = pp_context->batch;

    intel_batchbuffer_start_atomic(batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(batch);
    gen6_pp_pipeline_select(ctx, pp_context);
    gen6_pp_state_base_address(ctx, pp_context);
    gen6_pp_vfe_state(ctx, pp_context);
    gen6_pp_curbe_load(ctx, pp_context);
    gen6_interface_descriptor_load(ctx, pp_context);
    gen6_pp_object_walker(ctx, pp_context);
    intel_batchbuffer_end_atomic(batch);
}

static VAStatus
gen6_post_processing(
    VADriverContextP ctx,
    struct i965_post_processing_context *pp_context,
    const struct i965_surface *src_surface,
    const VARectangle *src_rect,
    struct i965_surface *dst_surface,
    const VARectangle *dst_rect,
    int pp_index,
    void *filter_param
)
{
    VAStatus va_status;
    
    va_status = gen6_pp_initialize(ctx, pp_context,
                                   src_surface,
                                   src_rect,
                                   dst_surface,
                                   dst_rect,
                                   pp_index,
                                   filter_param);

    if (va_status == VA_STATUS_SUCCESS) {
        gen6_pp_states_setup(ctx, pp_context);
        gen6_pp_pipeline_setup(ctx, pp_context);
    }

    if (va_status == VA_STATUS_SUCCESS_1)
        va_status = VA_STATUS_SUCCESS;

    return va_status;
}

static VAStatus
i965_post_processing_internal(
    VADriverContextP   ctx,
    struct i965_post_processing_context *pp_context,
    const struct i965_surface *src_surface,
    const VARectangle *src_rect,
    struct i965_surface *dst_surface,
    const VARectangle *dst_rect,
    int                pp_index,
    void *filter_param
)
{
    VAStatus va_status;
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    if (pp_context && pp_context->intel_post_processing) {
        va_status = (pp_context->intel_post_processing)(ctx, pp_context,
                          src_surface, src_rect,
                          dst_surface, dst_rect,
                          pp_index, filter_param);
    } else {
        va_status = VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    return va_status;
}

static void
rgb_to_yuv(unsigned int argb,
           unsigned char *y,
           unsigned char *u,
           unsigned char *v,
           unsigned char *a)
{
    int r = ((argb >> 16) & 0xff);
    int g = ((argb >> 8) & 0xff);
    int b = ((argb >> 0) & 0xff);
    
    *y = (257 * r + 504 * g + 98 * b) / 1000 + 16;
    *v = (439 * r - 368 * g - 71 * b) / 1000 + 128;
    *u = (-148 * r - 291 * g + 439 * b) / 1000 + 128;
    *a = ((argb >> 24) & 0xff);
}

static void 
i965_vpp_clear_surface(VADriverContextP ctx,
                       struct i965_post_processing_context *pp_context,
                       struct object_surface *obj_surface,
                       unsigned int color)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = pp_context->batch;
    unsigned int blt_cmd, br13;
    unsigned int tiling = 0, swizzle = 0;
    int pitch;
    unsigned char y, u, v, a = 0;
    int region_width, region_height;

    /* Currently only support NV12 surface */
    if (!obj_surface || obj_surface->fourcc != VA_FOURCC_NV12)
        return;

    rgb_to_yuv(color, &y, &u, &v, &a);

    if (a == 0)
        return;

    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);
    blt_cmd = XY_COLOR_BLT_CMD;
    pitch = obj_surface->width;

    if (tiling != I915_TILING_NONE) {
        assert(tiling == I915_TILING_Y);
        // blt_cmd |= XY_COLOR_BLT_DST_TILED;
        // pitch >>= 2;
    }

    br13 = 0xf0 << 16;
    br13 |= BR13_8;
    br13 |= pitch;

    if (IS_IRONLAKE(i965->intel.device_info)) {
        intel_batchbuffer_start_atomic(batch, 48);
        BEGIN_BATCH(batch, 12);
    } else {
        /* Will double-check the command if the new chipset is added */
        intel_batchbuffer_start_atomic_blt(batch, 48);
        BEGIN_BLT_BATCH(batch, 12);
    }

    region_width = obj_surface->width;
    region_height = obj_surface->height;

    OUT_BATCH(batch, blt_cmd);
    OUT_BATCH(batch, br13);
    OUT_BATCH(batch,
              0 << 16 |
              0);
    OUT_BATCH(batch,
              region_height << 16 |
              region_width);
    OUT_RELOC(batch, obj_surface->bo, 
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
              0);
    OUT_BATCH(batch, y);

    br13 = 0xf0 << 16;
    br13 |= BR13_565;
    br13 |= pitch;

    region_width = obj_surface->width / 2;
    region_height = obj_surface->height / 2;

    if (tiling == I915_TILING_Y) {
        region_height = ALIGN(obj_surface->height / 2, 32);
    }

    OUT_BATCH(batch, blt_cmd);
    OUT_BATCH(batch, br13);
    OUT_BATCH(batch,
              0 << 16 |
              0);
    OUT_BATCH(batch,
              region_height << 16 |
              region_width);
    OUT_RELOC(batch, obj_surface->bo, 
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
              obj_surface->width * obj_surface->y_cb_offset);
    OUT_BATCH(batch, v << 8 | u);

    ADVANCE_BATCH(batch);
    intel_batchbuffer_end_atomic(batch);
}

VAStatus
i965_scaling_processing(
    VADriverContextP   ctx,
    struct object_surface *src_surface_obj,
    const VARectangle *src_rect,
    struct object_surface *dst_surface_obj,
    const VARectangle *dst_rect,
    unsigned int       flags)
{
    VAStatus va_status = VA_STATUS_SUCCESS;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
 
    assert(src_surface_obj->fourcc == VA_FOURCC_NV12);
    assert(dst_surface_obj->fourcc == VA_FOURCC_NV12);

    if (HAS_VPP(i965) && (flags & I965_PP_FLAG_AVS)) {
        struct i965_surface src_surface;
        struct i965_surface dst_surface;

         _i965LockMutex(&i965->pp_mutex);

         src_surface.base = (struct object_base *)src_surface_obj;
         src_surface.type = I965_SURFACE_TYPE_SURFACE;
         src_surface.flags = I965_SURFACE_FLAG_FRAME;
         dst_surface.base = (struct object_base *)dst_surface_obj;
         dst_surface.type = I965_SURFACE_TYPE_SURFACE;
         dst_surface.flags = I965_SURFACE_FLAG_FRAME;

         va_status = i965_post_processing_internal(ctx, i965->pp_context,
                                                   &src_surface,
                                                   src_rect,
                                                   &dst_surface,
                                                   dst_rect,
                                                   PP_NV12_AVS,
                                                   NULL);

         _i965UnlockMutex(&i965->pp_mutex);
    }

    return va_status;
}

VASurfaceID
i965_post_processing(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags,
    int               *has_done_scaling  
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VASurfaceID out_surface_id = VA_INVALID_ID;
    VASurfaceID tmp_id = VA_INVALID_ID;
    
    *has_done_scaling = 0;

    if (HAS_VPP(i965)) {
        VAStatus status;
        struct i965_surface src_surface;
        struct i965_surface dst_surface;

        /* Currently only support post processing for NV12 surface */
        if (obj_surface->fourcc != VA_FOURCC_NV12)
            return out_surface_id;

        _i965LockMutex(&i965->pp_mutex);

        if (flags & I965_PP_FLAG_MCDI) {
            src_surface.base = (struct object_base *)obj_surface;
            src_surface.type = I965_SURFACE_TYPE_SURFACE;
            src_surface.flags = (flags & I965_PP_FLAG_TOP_FIELD) ? 
                I965_SURFACE_FLAG_TOP_FIELD_FIRST : I965_SURFACE_FLAG_BOTTOME_FIELD_FIRST;

            status = i965_CreateSurfaces(ctx,
                                         obj_surface->orig_width,
                                         obj_surface->orig_height,
                                         VA_RT_FORMAT_YUV420,
                                         1,
                                         &out_surface_id);
            assert(status == VA_STATUS_SUCCESS);
            obj_surface = SURFACE(out_surface_id);
            assert(obj_surface);
            i965_check_alloc_surface_bo(ctx, obj_surface, 0, VA_FOURCC_NV12, SUBSAMPLE_YUV420);
            i965_vpp_clear_surface(ctx, i965->pp_context, obj_surface, 0); 

            dst_surface.base = (struct object_base *)obj_surface;
            dst_surface.type = I965_SURFACE_TYPE_SURFACE;
            dst_surface.flags = I965_SURFACE_FLAG_FRAME;

            i965_post_processing_internal(ctx, i965->pp_context,
                                          &src_surface,
                                          src_rect,
                                          &dst_surface,
                                          dst_rect,
                                          PP_NV12_DNDI,
                                          NULL);
        }

        if (flags & I965_PP_FLAG_AVS) {
            struct i965_render_state *render_state = &i965->render_state;
            struct intel_region *dest_region = render_state->draw_region;

            if (out_surface_id != VA_INVALID_ID)
                tmp_id = out_surface_id;

            src_surface.base = (struct object_base *)obj_surface;
            src_surface.type = I965_SURFACE_TYPE_SURFACE;
            src_surface.flags = I965_SURFACE_FLAG_FRAME;

            status = i965_CreateSurfaces(ctx,
                                         dest_region->width,
                                         dest_region->height,
                                         VA_RT_FORMAT_YUV420,
                                         1,
                                         &out_surface_id);
            assert(status == VA_STATUS_SUCCESS);
            obj_surface = SURFACE(out_surface_id);
            assert(obj_surface);
            i965_check_alloc_surface_bo(ctx, obj_surface, 0, VA_FOURCC_NV12, SUBSAMPLE_YUV420);
            i965_vpp_clear_surface(ctx, i965->pp_context, obj_surface, 0); 

            dst_surface.base = (struct object_base *)obj_surface;
            dst_surface.type = I965_SURFACE_TYPE_SURFACE;
            dst_surface.flags = I965_SURFACE_FLAG_FRAME;

            i965_post_processing_internal(ctx, i965->pp_context,
                                          &src_surface,
                                          src_rect,
                                          &dst_surface,
                                          dst_rect,
                                          PP_NV12_AVS,
                                          NULL);

            if (tmp_id != VA_INVALID_ID)
                i965_DestroySurfaces(ctx, &tmp_id, 1);
                
            *has_done_scaling = 1;
        }

        _i965UnlockMutex(&i965->pp_mutex);
    }

    return out_surface_id;
}       

static VAStatus
i965_image_pl2_processing(VADriverContextP ctx,
                          const struct i965_surface *src_surface,
                          const VARectangle *src_rect,
                          struct i965_surface *dst_surface,
                          const VARectangle *dst_rect);

static VAStatus
i965_image_plx_nv12_plx_processing(VADriverContextP ctx,
                                   VAStatus (*i965_image_plx_nv12_processing)(
                                       VADriverContextP,
                                       const struct i965_surface *,
                                       const VARectangle *,
                                       struct i965_surface *,
                                       const VARectangle *),
                                   const struct i965_surface *src_surface,
                                   const VARectangle *src_rect,
                                   struct i965_surface *dst_surface,
                                   const VARectangle *dst_rect)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAStatus status;
    VASurfaceID tmp_surface_id = VA_INVALID_SURFACE;
    struct object_surface *obj_surface = NULL;
    struct i965_surface tmp_surface;
    int width, height;

    pp_get_surface_size(ctx, dst_surface, &width, &height);
    status = i965_CreateSurfaces(ctx,
                                 width,
                                 height,
                                 VA_RT_FORMAT_YUV420,
                                 1,
                                 &tmp_surface_id);
    assert(status == VA_STATUS_SUCCESS);
    obj_surface = SURFACE(tmp_surface_id);
    assert(obj_surface);
    i965_check_alloc_surface_bo(ctx, obj_surface, 0, VA_FOURCC_NV12, SUBSAMPLE_YUV420);

    tmp_surface.base = (struct object_base *)obj_surface;
    tmp_surface.type = I965_SURFACE_TYPE_SURFACE;
    tmp_surface.flags = I965_SURFACE_FLAG_FRAME;

    status = i965_image_plx_nv12_processing(ctx,
                                            src_surface,
                                            src_rect,
                                            &tmp_surface,
                                            dst_rect);

    if (status == VA_STATUS_SUCCESS)
        status = i965_image_pl2_processing(ctx,
                                           &tmp_surface,
                                           dst_rect,
                                           dst_surface,
                                           dst_rect);

    i965_DestroySurfaces(ctx,
                         &tmp_surface_id,
                         1);

    return status;
}


static VAStatus
i965_image_pl1_rgbx_processing(VADriverContextP ctx,
                               const struct i965_surface *src_surface,
                               const VARectangle *src_rect,
                               struct i965_surface *dst_surface,
                               const VARectangle *dst_rect)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_post_processing_context *pp_context = i965->pp_context;
    int fourcc = pp_get_surface_fourcc(ctx, dst_surface);
    VAStatus vaStatus;

    switch (fourcc) {
    case VA_FOURCC_NV12:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_RGBX_LOAD_SAVE_NV12,
                                                 NULL);
        intel_batchbuffer_flush(pp_context->batch);
        break;

    default:
        vaStatus = i965_image_plx_nv12_plx_processing(ctx,
                                                      i965_image_pl1_rgbx_processing,
                                                      src_surface,
                                                      src_rect,
                                                      dst_surface,
                                                      dst_rect);
        break;
    }

    return vaStatus;
}

static VAStatus
i965_image_pl3_processing(VADriverContextP ctx,
                          const struct i965_surface *src_surface,
                          const VARectangle *src_rect,
                          struct i965_surface *dst_surface,
                          const VARectangle *dst_rect)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_post_processing_context *pp_context = i965->pp_context;
    int fourcc = pp_get_surface_fourcc(ctx, dst_surface);
    VAStatus vaStatus = VA_STATUS_ERROR_UNIMPLEMENTED;

    switch (fourcc) {
    case VA_FOURCC_NV12:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_PL3_LOAD_SAVE_N12,
                                                 NULL);
        intel_batchbuffer_flush(pp_context->batch);
        break;

    case VA_FOURCC_IMC1:
    case VA_FOURCC_IMC3:
    case VA_FOURCC_YV12:
    case VA_FOURCC_I420:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_PL3_LOAD_SAVE_PL3,
                                                 NULL);
        intel_batchbuffer_flush(pp_context->batch);
        break;

    case VA_FOURCC_YUY2:
    case VA_FOURCC_UYVY:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_PL3_LOAD_SAVE_PA,
                                                 NULL);
        intel_batchbuffer_flush(pp_context->batch);
        break;

    default:
        vaStatus = i965_image_plx_nv12_plx_processing(ctx,
                                                      i965_image_pl3_processing,
                                                      src_surface,
                                                      src_rect,
                                                      dst_surface,
                                                      dst_rect);
        break;
    }

    return vaStatus;
}

static VAStatus
i965_image_pl2_processing(VADriverContextP ctx,
                          const struct i965_surface *src_surface,
                          const VARectangle *src_rect,
                          struct i965_surface *dst_surface,
                          const VARectangle *dst_rect)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_post_processing_context *pp_context = i965->pp_context;
    int fourcc = pp_get_surface_fourcc(ctx, dst_surface);
    VAStatus vaStatus = VA_STATUS_ERROR_UNIMPLEMENTED;

    switch (fourcc) {
    case VA_FOURCC_NV12:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_NV12_LOAD_SAVE_N12,
                                                 NULL);
        break;

    case VA_FOURCC_IMC1:
    case VA_FOURCC_IMC3:
    case VA_FOURCC_YV12:
    case VA_FOURCC_I420:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_NV12_LOAD_SAVE_PL3,
                                                 NULL);
        break;

    case VA_FOURCC_YUY2:
    case VA_FOURCC_UYVY:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_NV12_LOAD_SAVE_PA,
                                                 NULL);
        break;

    case VA_FOURCC_BGRX:
    case VA_FOURCC_BGRA:
    case VA_FOURCC_RGBX:
    case VA_FOURCC_RGBA:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_NV12_LOAD_SAVE_RGBX,
                                                 NULL);
        break;

    default:
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    intel_batchbuffer_flush(pp_context->batch);

    return vaStatus;
}

static VAStatus
i965_image_pl1_processing(VADriverContextP ctx,
                          const struct i965_surface *src_surface,
                          const VARectangle *src_rect,
                          struct i965_surface *dst_surface,
                          const VARectangle *dst_rect)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_post_processing_context *pp_context = i965->pp_context;
    int fourcc = pp_get_surface_fourcc(ctx, dst_surface);
    VAStatus vaStatus;

    switch (fourcc) {
    case VA_FOURCC_NV12:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_PA_LOAD_SAVE_NV12,
                                                 NULL);
        intel_batchbuffer_flush(pp_context->batch);
        break;

    case VA_FOURCC_YV12:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_PA_LOAD_SAVE_PL3,
                                                 NULL);
        intel_batchbuffer_flush(pp_context->batch);
        break;

    case VA_FOURCC_YUY2:
    case VA_FOURCC_UYVY:
        vaStatus = i965_post_processing_internal(ctx, i965->pp_context,
                                                 src_surface,
                                                 src_rect,
                                                 dst_surface,
                                                 dst_rect,
                                                 PP_PA_LOAD_SAVE_PA,
                                                 NULL);
        intel_batchbuffer_flush(pp_context->batch);
        break;

    default:
        vaStatus = i965_image_plx_nv12_plx_processing(ctx,
                                                      i965_image_pl1_processing,
                                                      src_surface,
                                                      src_rect,
                                                      dst_surface,
                                                      dst_rect);
        break;
    }

    return vaStatus;
}

VAStatus
i965_image_processing(VADriverContextP ctx,
                      const struct i965_surface *src_surface,
                      const VARectangle *src_rect,
                      struct i965_surface *dst_surface,
                      const VARectangle *dst_rect)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAStatus status = VA_STATUS_ERROR_UNIMPLEMENTED;

    if (HAS_VPP(i965)) {
        int fourcc = pp_get_surface_fourcc(ctx, src_surface);

        _i965LockMutex(&i965->pp_mutex);

        switch (fourcc) {
        case VA_FOURCC_YV12:
        case VA_FOURCC_I420:
        case VA_FOURCC_IMC1:
        case VA_FOURCC_IMC3:
        case VA_FOURCC_422H:
        case VA_FOURCC_422V:
        case VA_FOURCC_411P:
        case VA_FOURCC_444P:
        case VA_FOURCC_YV16:
            status = i965_image_pl3_processing(ctx,
                                               src_surface,
                                               src_rect,
                                               dst_surface,
                                               dst_rect);
            break;

        case  VA_FOURCC_NV12:
            status = i965_image_pl2_processing(ctx,
                                               src_surface,
                                               src_rect,
                                               dst_surface,
                                               dst_rect);
            break;
        case VA_FOURCC_YUY2:
        case VA_FOURCC_UYVY:
            status = i965_image_pl1_processing(ctx,
                                               src_surface,
                                               src_rect,
                                               dst_surface,
                                               dst_rect);
            break;
        case VA_FOURCC_BGRA:
        case VA_FOURCC_BGRX:
        case VA_FOURCC_RGBA:
        case VA_FOURCC_RGBX:
            status = i965_image_pl1_rgbx_processing(ctx,
                                               src_surface,
                                               src_rect,
                                               dst_surface,
                                               dst_rect);
            break;
        default:
            status = VA_STATUS_ERROR_UNIMPLEMENTED;
            break;
        }
        
        _i965UnlockMutex(&i965->pp_mutex);
    }

    return status;
}       

static void
i965_post_processing_context_finalize(struct i965_post_processing_context *pp_context)
{
    int i;

    dri_bo_unreference(pp_context->surface_state_binding_table.bo);
    pp_context->surface_state_binding_table.bo = NULL;

    dri_bo_unreference(pp_context->curbe.bo);
    pp_context->curbe.bo = NULL;

    dri_bo_unreference(pp_context->sampler_state_table.bo);
    pp_context->sampler_state_table.bo = NULL;

    dri_bo_unreference(pp_context->sampler_state_table.bo_8x8);
    pp_context->sampler_state_table.bo_8x8 = NULL;

    dri_bo_unreference(pp_context->sampler_state_table.bo_8x8_uv);
    pp_context->sampler_state_table.bo_8x8_uv = NULL;

    dri_bo_unreference(pp_context->idrt.bo);
    pp_context->idrt.bo = NULL;
    pp_context->idrt.num_interface_descriptors = 0;

    dri_bo_unreference(pp_context->vfe_state.bo);
    pp_context->vfe_state.bo = NULL;

    dri_bo_unreference(pp_context->pp_dndi_context.stmm_bo);
    pp_context->pp_dndi_context.stmm_bo = NULL;

    dri_bo_unreference(pp_context->pp_dn_context.stmm_bo);
    pp_context->pp_dn_context.stmm_bo = NULL;

    for (i = 0; i < NUM_PP_MODULES; i++) {
        struct pp_module *pp_module = &pp_context->pp_modules[i];

        dri_bo_unreference(pp_module->kernel.bo);
        pp_module->kernel.bo = NULL;
    }

    free(pp_context->pp_static_parameter);
    free(pp_context->pp_inline_parameter);
    pp_context->pp_static_parameter = NULL;
    pp_context->pp_inline_parameter = NULL;
}

void
i965_post_processing_terminate(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_post_processing_context *pp_context = i965->pp_context;

    if (pp_context) {
        pp_context->finalize(pp_context);
        free(pp_context);
    }

    i965->pp_context = NULL;
}

#define VPP_CURBE_ALLOCATION_SIZE	32

void
i965_post_processing_context_init(VADriverContextP ctx,
                                  void *data,
                                  struct intel_batchbuffer *batch)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i;
    struct i965_post_processing_context *pp_context = data;

    if (IS_IRONLAKE(i965->intel.device_info)) {
	pp_context->urb.size = i965->intel.device_info->urb_size;
	pp_context->urb.num_vfe_entries = 32;
	pp_context->urb.size_vfe_entry = 1;     /* in 512 bits unit */
	pp_context->urb.num_cs_entries = 1;
        pp_context->urb.size_cs_entry = 2;
	pp_context->urb.vfe_start = 0;
	pp_context->urb.cs_start = pp_context->urb.vfe_start + 
            pp_context->urb.num_vfe_entries * pp_context->urb.size_vfe_entry;
	assert(pp_context->urb.cs_start +
           pp_context->urb.num_cs_entries * pp_context->urb.size_cs_entry <= i965->intel.device_info->urb_size);
        pp_context->intel_post_processing = ironlake_post_processing;
    } else {
	pp_context->vfe_gpu_state.max_num_threads = 60;
	pp_context->vfe_gpu_state.num_urb_entries = 59;
	pp_context->vfe_gpu_state.gpgpu_mode = 0;
	pp_context->vfe_gpu_state.urb_entry_size = 16 - 1;
	pp_context->vfe_gpu_state.curbe_allocation_size = VPP_CURBE_ALLOCATION_SIZE;
        pp_context->intel_post_processing = gen6_post_processing;
    }

    pp_context->finalize = i965_post_processing_context_finalize;

    assert(NUM_PP_MODULES == ARRAY_ELEMS(pp_modules_gen5));
    assert(NUM_PP_MODULES == ARRAY_ELEMS(pp_modules_gen6));
    assert(NUM_PP_MODULES == ARRAY_ELEMS(pp_modules_gen7));
    assert(NUM_PP_MODULES == ARRAY_ELEMS(pp_modules_gen75));

    if (IS_HASWELL(i965->intel.device_info))
        memcpy(pp_context->pp_modules, pp_modules_gen75, sizeof(pp_context->pp_modules));
    else if (IS_GEN7(i965->intel.device_info))
        memcpy(pp_context->pp_modules, pp_modules_gen7, sizeof(pp_context->pp_modules));
    else if (IS_GEN6(i965->intel.device_info))
        memcpy(pp_context->pp_modules, pp_modules_gen6, sizeof(pp_context->pp_modules));
    else if (IS_IRONLAKE(i965->intel.device_info))
        memcpy(pp_context->pp_modules, pp_modules_gen5, sizeof(pp_context->pp_modules));

    for (i = 0; i < NUM_PP_MODULES; i++) {
        struct pp_module *pp_module = &pp_context->pp_modules[i];
        dri_bo_unreference(pp_module->kernel.bo);
        if (pp_module->kernel.bin && pp_module->kernel.size) {
            pp_module->kernel.bo = dri_bo_alloc(i965->intel.bufmgr,
                                                pp_module->kernel.name,
                                                pp_module->kernel.size,
                                                4096);
            assert(pp_module->kernel.bo);
            dri_bo_subdata(pp_module->kernel.bo, 0, pp_module->kernel.size, pp_module->kernel.bin);
        } else {
            pp_module->kernel.bo = NULL;
        }
    }

    /* static & inline parameters */
    if (IS_GEN7(i965->intel.device_info)) {
        pp_context->pp_static_parameter = calloc(sizeof(struct gen7_pp_static_parameter), 1);
        pp_context->pp_inline_parameter = calloc(sizeof(struct gen7_pp_inline_parameter), 1);
    } else {
        pp_context->pp_static_parameter = calloc(sizeof(struct pp_static_parameter), 1);
        pp_context->pp_inline_parameter = calloc(sizeof(struct pp_inline_parameter), 1);
    }

    pp_context->pp_dndi_context.current_out_surface = VA_INVALID_SURFACE;
    pp_context->pp_dndi_context.current_out_obj_surface = NULL;
    pp_context->pp_dndi_context.frame_order = -1;
    pp_context->batch = batch;
}

bool
i965_post_processing_init(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_post_processing_context *pp_context = i965->pp_context;

    if (HAS_VPP(i965)) {
        if (pp_context == NULL) {
            pp_context = calloc(1, sizeof(*pp_context));
            i965->codec_info->post_processing_context_init(ctx, pp_context, i965->pp_batch);
            i965->pp_context = pp_context;
        }
    }

    return true;
}

static const int procfilter_to_pp_flag[VAProcFilterCount] = {
    PP_NULL,    /* VAProcFilterNone */
    PP_NV12_DN, /* VAProcFilterNoiseReduction */
    PP_NV12_DNDI, /* VAProcFilterDeinterlacing */
    PP_NULL,    /* VAProcFilterSharpening */
    PP_NULL,    /* VAProcFilterColorBalance */
};

static const int proc_frame_to_pp_frame[3] = {
    I965_SURFACE_FLAG_FRAME,
    I965_SURFACE_FLAG_TOP_FIELD_FIRST,
    I965_SURFACE_FLAG_BOTTOME_FIELD_FIRST
};

VAStatus 
i965_proc_picture(VADriverContextP ctx, 
                  VAProfile profile, 
                  union codec_state *codec_state,
                  struct hw_context *hw_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_proc_context *proc_context = (struct i965_proc_context *)hw_context;
    struct proc_state *proc_state = &codec_state->proc;
    VAProcPipelineParameterBuffer *pipeline_param = (VAProcPipelineParameterBuffer *)proc_state->pipeline_param->buffer;
    struct object_surface *obj_surface;
    struct i965_surface src_surface, dst_surface;
    VARectangle src_rect, dst_rect;
    VAStatus status;
    int i;
    VASurfaceID tmp_surfaces[VAProcFilterCount + 4];
    int num_tmp_surfaces = 0;
    unsigned int tiling = 0, swizzle = 0;
    int in_width, in_height;

    if (pipeline_param->surface == VA_INVALID_ID ||
        proc_state->current_render_target == VA_INVALID_ID) {
        status = VA_STATUS_ERROR_INVALID_SURFACE;
        goto error;
    }

    obj_surface = SURFACE(pipeline_param->surface);

    if (!obj_surface) {
        status = VA_STATUS_ERROR_INVALID_SURFACE;
        goto error;
    }

    if (!obj_surface->bo) {
        status = VA_STATUS_ERROR_INVALID_VALUE; /* The input surface is created without valid content */
        goto error;
    }

    if (pipeline_param->num_filters && !pipeline_param->filters) {
        status = VA_STATUS_ERROR_INVALID_PARAMETER;
        goto error;
    }

    in_width = obj_surface->orig_width;
    in_height = obj_surface->orig_height;
    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);

    src_surface.base = (struct object_base *)obj_surface;
    src_surface.type = I965_SURFACE_TYPE_SURFACE;
    src_surface.flags = proc_frame_to_pp_frame[pipeline_param->filter_flags & 0x3];

    VASurfaceID out_surface_id = VA_INVALID_ID;
    if (obj_surface->fourcc != VA_FOURCC_NV12) {
        src_surface.base = (struct object_base *)obj_surface;
        src_surface.type = I965_SURFACE_TYPE_SURFACE;
        src_surface.flags = I965_SURFACE_FLAG_FRAME;
        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.width = in_width;
        src_rect.height = in_height;

        status = i965_CreateSurfaces(ctx,
                                     in_width,
                                     in_height,
                                     VA_RT_FORMAT_YUV420,
                                     1,
                                     &out_surface_id);
        assert(status == VA_STATUS_SUCCESS);
        tmp_surfaces[num_tmp_surfaces++] = out_surface_id;
        obj_surface = SURFACE(out_surface_id);
        assert(obj_surface);
        i965_check_alloc_surface_bo(ctx, obj_surface, !!tiling, VA_FOURCC_NV12, SUBSAMPLE_YUV420);

        dst_surface.base = (struct object_base *)obj_surface;
        dst_surface.type = I965_SURFACE_TYPE_SURFACE;
        dst_surface.flags = I965_SURFACE_FLAG_FRAME;
        dst_rect.x = 0;
        dst_rect.y = 0;
        dst_rect.width = in_width;
        dst_rect.height = in_height;

        status = i965_image_processing(ctx,
                                       &src_surface,
                                       &src_rect,
                                       &dst_surface,
                                       &dst_rect);
        assert(status == VA_STATUS_SUCCESS);

        src_surface.base = (struct object_base *)obj_surface;
        src_surface.type = I965_SURFACE_TYPE_SURFACE;
        src_surface.flags = proc_frame_to_pp_frame[pipeline_param->filter_flags & 0x3];
    }

    if (pipeline_param->surface_region) {
        src_rect.x = pipeline_param->surface_region->x;
        src_rect.y = pipeline_param->surface_region->y;
        src_rect.width = pipeline_param->surface_region->width;
        src_rect.height = pipeline_param->surface_region->height;
    } else {
        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.width = in_width;
        src_rect.height = in_height;
    }

    if (pipeline_param->output_region) {
        dst_rect.x = pipeline_param->output_region->x;
        dst_rect.y = pipeline_param->output_region->y;
        dst_rect.width = pipeline_param->output_region->width;
        dst_rect.height = pipeline_param->output_region->height;
    } else {
        dst_rect.x = 0;
        dst_rect.y = 0;
        dst_rect.width = in_width;
        dst_rect.height = in_height;
    }

    proc_context->pp_context.pipeline_param = pipeline_param;

    for (i = 0; i < pipeline_param->num_filters; i++) {
        struct object_buffer *obj_buffer = BUFFER(pipeline_param->filters[i]);
        VAProcFilterParameterBufferBase *filter_param = NULL;
        VAProcFilterType filter_type;
        int kernel_index;

        if (!obj_buffer ||
            !obj_buffer->buffer_store ||
            !obj_buffer->buffer_store->buffer) {
            status = VA_STATUS_ERROR_INVALID_FILTER_CHAIN;
            goto error;
        }

        out_surface_id = VA_INVALID_ID;
        filter_param = (VAProcFilterParameterBufferBase *)obj_buffer->buffer_store->buffer;
        filter_type = filter_param->type;
        kernel_index = procfilter_to_pp_flag[filter_type];

        if (kernel_index != PP_NULL &&
            proc_context->pp_context.pp_modules[kernel_index].kernel.bo != NULL) {
            status = i965_CreateSurfaces(ctx,
                                         in_width,
                                         in_height,
                                         VA_RT_FORMAT_YUV420,
                                         1,
                                         &out_surface_id);
            assert(status == VA_STATUS_SUCCESS);
            tmp_surfaces[num_tmp_surfaces++] = out_surface_id;
            obj_surface = SURFACE(out_surface_id);
            assert(obj_surface);
            i965_check_alloc_surface_bo(ctx, obj_surface, !!tiling, VA_FOURCC_NV12, SUBSAMPLE_YUV420);
            dst_surface.base = (struct object_base *)obj_surface;
            dst_surface.type = I965_SURFACE_TYPE_SURFACE;
            status = i965_post_processing_internal(ctx, &proc_context->pp_context,
                                                   &src_surface,
                                                   &src_rect,
                                                   &dst_surface,
                                                   &src_rect,
                                                   kernel_index,
                                                   filter_param);

            if (status == VA_STATUS_SUCCESS) {
                src_surface.base = dst_surface.base;
                src_surface.type = dst_surface.type;
                src_surface.flags = dst_surface.flags;
            }
        }
    }

    proc_context->pp_context.pipeline_param = NULL;
    obj_surface = SURFACE(proc_state->current_render_target);
    
    if (!obj_surface) {
        status = VA_STATUS_ERROR_INVALID_SURFACE;
        goto error;
    }

    int csc_needed = 0;
    if (obj_surface->fourcc && obj_surface->fourcc !=  VA_FOURCC_NV12){
        csc_needed = 1;
        out_surface_id = VA_INVALID_ID;
        status = i965_CreateSurfaces(ctx,
                                     obj_surface->orig_width,
                                     obj_surface->orig_height,
                                     VA_RT_FORMAT_YUV420, 
                                     1,
                                     &out_surface_id);
        assert(status == VA_STATUS_SUCCESS);
        tmp_surfaces[num_tmp_surfaces++] = out_surface_id;
        struct object_surface *csc_surface = SURFACE(out_surface_id);
        assert(csc_surface);
        i965_check_alloc_surface_bo(ctx, csc_surface, !!tiling, VA_FOURCC_NV12, SUBSAMPLE_YUV420);
        dst_surface.base = (struct object_base *)csc_surface;
    } else {
        i965_check_alloc_surface_bo(ctx, obj_surface, !!tiling, VA_FOURCC_NV12, SUBSAMPLE_YUV420);
        dst_surface.base = (struct object_base *)obj_surface;
    }

    dst_surface.type = I965_SURFACE_TYPE_SURFACE;
    i965_vpp_clear_surface(ctx, &proc_context->pp_context, obj_surface, pipeline_param->output_background_color); 

    // load/save doesn't support different origin offset for src and dst surface
    if (src_rect.width == dst_rect.width &&
        src_rect.height == dst_rect.height &&
        src_rect.x == dst_rect.x &&
        src_rect.y == dst_rect.y) {
        i965_post_processing_internal(ctx, &proc_context->pp_context,
                                      &src_surface,
                                      &src_rect,
                                      &dst_surface,
                                      &dst_rect,
                                      PP_NV12_LOAD_SAVE_N12,
                                      NULL);
    } else {

        i965_post_processing_internal(ctx, &proc_context->pp_context,
                                      &src_surface,
                                      &src_rect,
                                      &dst_surface,
                                      &dst_rect,
                                      (pipeline_param->filter_flags & VA_FILTER_SCALING_MASK) == VA_FILTER_SCALING_NL_ANAMORPHIC ?
                                      PP_NV12_AVS : PP_NV12_SCALING,
                                      NULL);
    }

    if (csc_needed) {
        src_surface.base = dst_surface.base;
        src_surface.type = dst_surface.type;
        src_surface.flags = dst_surface.flags;
        dst_surface.base = (struct object_base *)obj_surface;
        dst_surface.type = I965_SURFACE_TYPE_SURFACE;
        i965_image_processing(ctx, &src_surface, &dst_rect, &dst_surface, &dst_rect);
    }
    
    if (num_tmp_surfaces)
        i965_DestroySurfaces(ctx,
                             tmp_surfaces,
                             num_tmp_surfaces);

    intel_batchbuffer_flush(hw_context->batch);

    return VA_STATUS_SUCCESS;

error:
    if (num_tmp_surfaces)
        i965_DestroySurfaces(ctx,
                             tmp_surfaces,
                             num_tmp_surfaces);

    return status;
}

static void
i965_proc_context_destroy(void *hw_context)
{
    struct i965_proc_context *proc_context = (struct i965_proc_context *)hw_context;

    i965_post_processing_context_finalize(&proc_context->pp_context);
    intel_batchbuffer_free(proc_context->base.batch);
    free(proc_context);
}

struct hw_context *
i965_proc_context_init(VADriverContextP ctx, struct object_config *obj_config)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_driver_data *intel = intel_driver_data(ctx);
    struct i965_proc_context *proc_context = calloc(1, sizeof(struct i965_proc_context));

    proc_context->base.destroy = i965_proc_context_destroy;
    proc_context->base.run = i965_proc_picture;
    proc_context->base.batch = intel_batchbuffer_new(intel, I915_EXEC_RENDER, 0);
    i965->codec_info->post_processing_context_init(ctx, &proc_context->pp_context, proc_context->base.batch);

    return (struct hw_context *)proc_context;
}


