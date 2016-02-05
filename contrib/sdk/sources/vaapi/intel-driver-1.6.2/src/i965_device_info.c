/*
 * Copyright © 2014 Intel Corporation
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
 */

#include <stdio.h>
#include <stdlib.h>
#include "i965_drv_video.h"

#include <string.h>
#include <strings.h>
#include <errno.h>
#include <cpuid.h>

/* Extra set of chroma formats supported for H.264 decoding (beyond YUV 4:2:0) */
#define EXTRA_H264_DEC_CHROMA_FORMATS \
    (VA_RT_FORMAT_YUV400)

/* Extra set of chroma formats supported for JPEG decoding (beyond YUV 4:2:0) */
#define EXTRA_JPEG_DEC_CHROMA_FORMATS \
    (VA_RT_FORMAT_YUV400 | VA_RT_FORMAT_YUV411 | VA_RT_FORMAT_YUV422 | \
     VA_RT_FORMAT_YUV444)

/* Extra set of chroma formats supported for JPEG encoding (beyond YUV 4:2:0) */
#define EXTRA_JPEG_ENC_CHROMA_FORMATS \
    (VA_RT_FORMAT_YUV400| VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_RGB32)

#define EXTRA_HEVC_DEC_CHROMA_FORMATS \
    (VA_RT_FORMAT_YUV420_10BPP)

/* Defines VA profile as a 32-bit unsigned integer mask */
#define VA_PROFILE_MASK(PROFILE) \
    (1U << VAProfile##PROFILE)

extern struct hw_context *i965_proc_context_init(VADriverContextP, struct object_config *);
extern struct hw_context *g4x_dec_hw_context_init(VADriverContextP, struct object_config *);
extern bool genx_render_init(VADriverContextP);

static struct hw_codec_info g4x_hw_codec_info = {
    .dec_hw_context_init = g4x_dec_hw_context_init,
    .enc_hw_context_init = NULL,
    .proc_hw_context_init = NULL,
    .render_init = genx_render_init,
    .post_processing_context_init = NULL,

    .max_width = 2048,
    .max_height = 2048,
    .min_linear_wpitch = 16,
    .min_linear_hpitch = 16,

    .has_mpeg2_decoding = 1,

    .num_filters = 0,
};

extern struct hw_context *ironlake_dec_hw_context_init(VADriverContextP, struct object_config *);
extern void i965_post_processing_context_init(VADriverContextP, void *, struct intel_batchbuffer *);

static struct hw_codec_info ilk_hw_codec_info = {
    .dec_hw_context_init = ironlake_dec_hw_context_init,
    .enc_hw_context_init = NULL,
    .proc_hw_context_init = i965_proc_context_init,
    .render_init = genx_render_init,
    .post_processing_context_init = i965_post_processing_context_init,

    .max_width = 2048,
    .max_height = 2048,
    .min_linear_wpitch = 16,
    .min_linear_hpitch = 16,

    .has_mpeg2_decoding = 1,
    .has_h264_decoding = 1,
    .has_vpp = 1,
    .has_accelerated_putimage = 1,

    .num_filters = 0,
};

static void gen6_hw_codec_preinit(VADriverContextP ctx, struct hw_codec_info *codec_info);

extern struct hw_context *gen6_dec_hw_context_init(VADriverContextP, struct object_config *);
extern struct hw_context *gen6_enc_hw_context_init(VADriverContextP, struct object_config *);
static struct hw_codec_info snb_hw_codec_info = {
    .dec_hw_context_init = gen6_dec_hw_context_init,
    .enc_hw_context_init = gen6_enc_hw_context_init,
    .proc_hw_context_init = i965_proc_context_init,
    .render_init = genx_render_init,
    .post_processing_context_init = i965_post_processing_context_init,
    .preinit_hw_codec = gen6_hw_codec_preinit,

    .max_width = 2048,
    .max_height = 2048,
    .min_linear_wpitch = 16,
    .min_linear_hpitch = 16,

    .h264_mvc_dec_profiles = VA_PROFILE_MASK(H264StereoHigh),
    .h264_dec_chroma_formats = EXTRA_H264_DEC_CHROMA_FORMATS,

    .has_mpeg2_decoding = 1,
    .has_h264_decoding = 1,
    .has_h264_encoding = 1,
    .has_vc1_decoding = 1,
    .has_vpp = 1,
    .has_accelerated_getimage = 1,
    .has_accelerated_putimage = 1,
    .has_tiled_surface = 1,
    .has_di_motion_adptive = 1,

    .num_filters = 2,
    .filters = {
        { VAProcFilterNoiseReduction, I965_RING_NULL },
        { VAProcFilterDeinterlacing, I965_RING_NULL },
    },
};

static void gen7_hw_codec_preinit(VADriverContextP ctx, struct hw_codec_info *codec_info);

extern struct hw_context *gen7_dec_hw_context_init(VADriverContextP, struct object_config *);
extern struct hw_context *gen7_enc_hw_context_init(VADriverContextP, struct object_config *);
static struct hw_codec_info ivb_hw_codec_info = {
    .dec_hw_context_init = gen7_dec_hw_context_init,
    .enc_hw_context_init = gen7_enc_hw_context_init,
    .proc_hw_context_init = i965_proc_context_init,
    .render_init = genx_render_init,
    .post_processing_context_init = i965_post_processing_context_init,
    .preinit_hw_codec = gen7_hw_codec_preinit,

    .max_width = 4096,
    .max_height = 4096,
    .min_linear_wpitch = 64,
    .min_linear_hpitch = 16,

    .h264_mvc_dec_profiles = VA_PROFILE_MASK(H264StereoHigh),
    .h264_dec_chroma_formats = EXTRA_H264_DEC_CHROMA_FORMATS,
    .jpeg_dec_chroma_formats = EXTRA_JPEG_DEC_CHROMA_FORMATS,

    .has_mpeg2_decoding = 1,
    .has_mpeg2_encoding = 1,
    .has_h264_decoding = 1,
    .has_h264_encoding = 1,
    .has_vc1_decoding = 1,
    .has_jpeg_decoding = 1,
    .has_vpp = 1,
    .has_accelerated_getimage = 1,
    .has_accelerated_putimage = 1,
    .has_tiled_surface = 1,
    .has_di_motion_adptive = 1,
    .has_di_motion_compensated = 1,

    .num_filters = 2,
    .filters = {
        { VAProcFilterNoiseReduction, I965_RING_NULL },
        { VAProcFilterDeinterlacing, I965_RING_NULL },
    },
};

static void hsw_hw_codec_preinit(VADriverContextP ctx, struct hw_codec_info *codec_info);

extern struct hw_context *gen75_dec_hw_context_init(VADriverContextP, struct object_config *);
extern struct hw_context *gen75_enc_hw_context_init(VADriverContextP, struct object_config *);
extern struct hw_context *gen75_proc_context_init(VADriverContextP, struct object_config *);
static struct hw_codec_info hsw_hw_codec_info = {
    .dec_hw_context_init = gen75_dec_hw_context_init,
    .enc_hw_context_init = gen75_enc_hw_context_init,
    .proc_hw_context_init = gen75_proc_context_init,
    .render_init = genx_render_init,
    .post_processing_context_init = i965_post_processing_context_init,
    .preinit_hw_codec = hsw_hw_codec_preinit,

    .max_width = 4096,
    .max_height = 4096,
    .min_linear_wpitch = 64,
    .min_linear_hpitch = 16,

    .h264_mvc_dec_profiles = (VA_PROFILE_MASK(H264StereoHigh) |
                              VA_PROFILE_MASK(H264MultiviewHigh)),
    .h264_dec_chroma_formats = EXTRA_H264_DEC_CHROMA_FORMATS,
    .jpeg_dec_chroma_formats = EXTRA_JPEG_DEC_CHROMA_FORMATS,

    .has_mpeg2_decoding = 1,
    .has_mpeg2_encoding = 1,
    .has_h264_decoding = 1,
    .has_h264_encoding = 1,
    .has_vc1_decoding = 1,
    .has_jpeg_decoding = 1,
    .has_vpp = 1,
    .has_accelerated_getimage = 1,
    .has_accelerated_putimage = 1,
    .has_tiled_surface = 1,
    .has_di_motion_adptive = 1,
    .has_di_motion_compensated = 1,
    .has_h264_mvc_encoding = 1,

    .num_filters = 5,
    .filters = {
        { VAProcFilterNoiseReduction, I965_RING_VEBOX },
        { VAProcFilterDeinterlacing, I965_RING_VEBOX },
        { VAProcFilterSharpening, I965_RING_NULL },
        { VAProcFilterColorBalance, I965_RING_VEBOX},
        { VAProcFilterSkinToneEnhancement, I965_RING_VEBOX},
    },
};

extern struct hw_context *gen8_dec_hw_context_init(VADriverContextP, struct object_config *);
extern struct hw_context *gen8_enc_hw_context_init(VADriverContextP, struct object_config *);
extern void gen8_post_processing_context_init(VADriverContextP, void *, struct intel_batchbuffer *);
static struct hw_codec_info bdw_hw_codec_info = {
    .dec_hw_context_init = gen8_dec_hw_context_init,
    .enc_hw_context_init = gen8_enc_hw_context_init,
    .proc_hw_context_init = gen75_proc_context_init,
    .render_init = gen8_render_init,
    .post_processing_context_init = gen8_post_processing_context_init,

    .max_width = 4096,
    .max_height = 4096,
    .min_linear_wpitch = 128,
    .min_linear_hpitch = 16,

    .h264_mvc_dec_profiles = (VA_PROFILE_MASK(H264StereoHigh) |
                              VA_PROFILE_MASK(H264MultiviewHigh)),
    .h264_dec_chroma_formats = EXTRA_H264_DEC_CHROMA_FORMATS,
    .jpeg_dec_chroma_formats = EXTRA_JPEG_DEC_CHROMA_FORMATS,

    .has_mpeg2_decoding = 1,
    .has_mpeg2_encoding = 1,
    .has_h264_decoding = 1,
    .has_h264_encoding = 1,
    .has_vc1_decoding = 1,
    .has_jpeg_decoding = 1,
    .has_vpp = 1,
    .has_accelerated_getimage = 1,
    .has_accelerated_putimage = 1,
    .has_tiled_surface = 1,
    .has_di_motion_adptive = 1,
    .has_di_motion_compensated = 1,
    .has_vp8_decoding = 1,
    .has_h264_mvc_encoding = 1,

    .num_filters = 5,
    .filters = {
        { VAProcFilterNoiseReduction, I965_RING_VEBOX },
        { VAProcFilterDeinterlacing, I965_RING_VEBOX },
        { VAProcFilterSharpening, I965_RING_NULL }, /* need to rebuild the shader for BDW */
        { VAProcFilterColorBalance, I965_RING_VEBOX},
        { VAProcFilterSkinToneEnhancement, I965_RING_VEBOX},
    },
};

extern struct hw_context *gen9_dec_hw_context_init(VADriverContextP, struct object_config *);
static struct hw_codec_info chv_hw_codec_info = {
    .dec_hw_context_init = gen9_dec_hw_context_init,
    .enc_hw_context_init = gen8_enc_hw_context_init,
    .proc_hw_context_init = gen75_proc_context_init,
    .render_init = gen8_render_init,
    .post_processing_context_init = gen8_post_processing_context_init,

    .max_width = 4096,
    .max_height = 4096,
    .min_linear_wpitch = 128,
    .min_linear_hpitch = 16,

    .h264_mvc_dec_profiles = (VA_PROFILE_MASK(H264StereoHigh) |
                              VA_PROFILE_MASK(H264MultiviewHigh)),
    .h264_dec_chroma_formats = EXTRA_H264_DEC_CHROMA_FORMATS,
    .jpeg_dec_chroma_formats = EXTRA_JPEG_DEC_CHROMA_FORMATS,
    .jpeg_enc_chroma_formats = EXTRA_JPEG_ENC_CHROMA_FORMATS,

    .has_mpeg2_decoding = 1,
    .has_mpeg2_encoding = 1,
    .has_h264_decoding = 1,
    .has_h264_encoding = 1,
    .has_vc1_decoding = 1,
    .has_jpeg_decoding = 1,
    .has_jpeg_encoding = 1,
    .has_vpp = 1,
    .has_accelerated_getimage = 1,
    .has_accelerated_putimage = 1,
    .has_tiled_surface = 1,
    .has_di_motion_adptive = 1,
    .has_di_motion_compensated = 1,
    .has_vp8_decoding = 1,
    .has_vp8_encoding = 1,
    .has_h264_mvc_encoding = 1,
    .has_hevc_decoding = 1,

    .num_filters = 5,
    .filters = {
        { VAProcFilterNoiseReduction, I965_RING_VEBOX },
        { VAProcFilterDeinterlacing, I965_RING_VEBOX },
        { VAProcFilterSharpening, I965_RING_NULL }, /* need to rebuild the shader for BDW */
        { VAProcFilterColorBalance, I965_RING_VEBOX},
        { VAProcFilterSkinToneEnhancement, I965_RING_VEBOX},
    },
};

extern struct hw_context *gen9_enc_hw_context_init(VADriverContextP, struct object_config *);
extern void gen9_post_processing_context_init(VADriverContextP, void *, struct intel_batchbuffer *);
static struct hw_codec_info skl_hw_codec_info = {
    .dec_hw_context_init = gen9_dec_hw_context_init,
    .enc_hw_context_init = gen9_enc_hw_context_init,
    .proc_hw_context_init = gen75_proc_context_init,
    .render_init = gen9_render_init,
    .post_processing_context_init = gen9_post_processing_context_init,

    .max_width = 4096,
    .max_height = 4096,
    .min_linear_wpitch = 128,
    .min_linear_hpitch = 16,

    .h264_mvc_dec_profiles = (VA_PROFILE_MASK(H264StereoHigh) |
                              VA_PROFILE_MASK(H264MultiviewHigh)),
    .h264_dec_chroma_formats = EXTRA_H264_DEC_CHROMA_FORMATS,
    .jpeg_dec_chroma_formats = EXTRA_JPEG_DEC_CHROMA_FORMATS,
    .jpeg_enc_chroma_formats = EXTRA_JPEG_ENC_CHROMA_FORMATS,

    .has_mpeg2_decoding = 1,
    .has_mpeg2_encoding = 1,
    .has_h264_decoding = 1,
    .has_h264_encoding = 1,
    .has_vc1_decoding = 1,
    .has_jpeg_decoding = 1,
    .has_jpeg_encoding = 1,
    .has_vpp = 1,
    .has_accelerated_getimage = 1,
    .has_accelerated_putimage = 1,
    .has_tiled_surface = 1,
    .has_di_motion_adptive = 1,
    .has_di_motion_compensated = 1,
    .has_vp8_decoding = 1,
    .has_vp8_encoding = 1,
    .has_h264_mvc_encoding = 1,
    .has_hevc_decoding = 1,
    .has_hevc_encoding = 1,

    .num_filters = 5,
    .filters = {
        { VAProcFilterNoiseReduction, I965_RING_VEBOX },
        { VAProcFilterDeinterlacing, I965_RING_VEBOX },
        { VAProcFilterSharpening, I965_RING_NULL }, /* need to rebuild the shader for BDW */
        { VAProcFilterColorBalance, I965_RING_VEBOX},
        { VAProcFilterSkinToneEnhancement, I965_RING_VEBOX},
    },
};


static struct hw_codec_info bxt_hw_codec_info = {
    .dec_hw_context_init = gen9_dec_hw_context_init,
    .enc_hw_context_init = gen9_enc_hw_context_init,
    .proc_hw_context_init = gen75_proc_context_init,
    .render_init = gen9_render_init,
    .post_processing_context_init = gen9_post_processing_context_init,

    .max_width = 4096,
    .max_height = 4096,
    .min_linear_wpitch = 64,
    .min_linear_hpitch = 16,

    .h264_mvc_dec_profiles = (VA_PROFILE_MASK(H264StereoHigh) |
                              VA_PROFILE_MASK(H264MultiviewHigh)),
    .h264_dec_chroma_formats = EXTRA_H264_DEC_CHROMA_FORMATS,
    .jpeg_dec_chroma_formats = EXTRA_JPEG_DEC_CHROMA_FORMATS,
    .jpeg_enc_chroma_formats = EXTRA_JPEG_ENC_CHROMA_FORMATS,
    .hevc_dec_chroma_formats = EXTRA_HEVC_DEC_CHROMA_FORMATS,

    .has_mpeg2_decoding = 1,
    .has_h264_decoding = 1,
    .has_h264_encoding = 1,
    .has_vc1_decoding = 1,
    .has_jpeg_decoding = 1,
    .has_jpeg_encoding = 1,
    .has_vpp = 1,
    .has_accelerated_getimage = 1,
    .has_accelerated_putimage = 1,
    .has_tiled_surface = 1,
    .has_di_motion_adptive = 1,
    .has_di_motion_compensated = 1,
    .has_vp8_decoding = 1,
    .has_vp8_encoding = 1,
    .has_h264_mvc_encoding = 1,
    .has_hevc_decoding = 1,
    .has_hevc_encoding = 1,
    .has_hevc10_decoding = 1,
    .has_vp9_decoding = 1,

    .num_filters = 5,
    .filters = {
        { VAProcFilterNoiseReduction, I965_RING_VEBOX },
        { VAProcFilterDeinterlacing, I965_RING_VEBOX },
        { VAProcFilterSharpening, I965_RING_NULL },
        { VAProcFilterColorBalance, I965_RING_VEBOX},
        { VAProcFilterSkinToneEnhancement, I965_RING_VEBOX},
    },
};


struct hw_codec_info *
i965_get_codec_info(int devid)
{
    switch (devid) {
#undef CHIPSET
#define CHIPSET(id, family, dev, str) case id: return &family##_hw_codec_info;
#include "i965_pciids.h"
    default:
        return NULL;
    }
}

static const struct intel_device_info g4x_device_info = {
    .gen = 4,

    .urb_size = 384,
    .max_wm_threads = 50,       /* 10 * 5 */

    .is_g4x = 1,
};

static const struct intel_device_info ilk_device_info = {
    .gen = 5,

    .urb_size = 1024,
    .max_wm_threads = 72,       /* 12 * 6 */
};

static const struct intel_device_info snb_gt1_device_info = {
    .gen = 6,
    .gt = 1,

    .urb_size = 1024,
    .max_wm_threads = 40,
};

static const struct intel_device_info snb_gt2_device_info = {
    .gen = 6,
    .gt = 2,

    .urb_size = 1024,
    .max_wm_threads = 80,
};

static const struct intel_device_info ivb_gt1_device_info = {
    .gen = 7,
    .gt = 1,

    .urb_size = 4096,
    .max_wm_threads = 48,

    .is_ivybridge = 1,
};

static const struct intel_device_info ivb_gt2_device_info = {
    .gen = 7,
    .gt = 2,

    .urb_size = 4096,
    .max_wm_threads = 172,

    .is_ivybridge = 1,
};

static const struct intel_device_info byt_device_info = {
    .gen = 7,
    .gt = 1,

    .urb_size = 4096,
    .max_wm_threads = 48,

    .is_ivybridge = 1,
    .is_baytrail = 1,
};

static const struct intel_device_info hsw_gt1_device_info = {
    .gen = 7,
    .gt = 1,

    .urb_size = 4096,
    .max_wm_threads = 102,

    .is_haswell = 1,
};

static const struct intel_device_info hsw_gt2_device_info = {
    .gen = 7,
    .gt = 2,

    .urb_size = 4096,
    .max_wm_threads = 204,

    .is_haswell = 1,
};

static const struct intel_device_info hsw_gt3_device_info = {
    .gen = 7,
    .gt = 3,

    .urb_size = 4096,
    .max_wm_threads = 408,

    .is_haswell = 1,
};

static const struct intel_device_info bdw_device_info = {
    .gen = 8,

    .urb_size = 4096,
    .max_wm_threads = 64,       /* per PSD */
};

static const struct intel_device_info chv_device_info = {
    .gen = 8,

    .urb_size = 4096,
    .max_wm_threads = 64,       /* per PSD */

    .is_cherryview = 1,
};

static const struct intel_device_info skl_device_info = {
    .gen = 9,

    .urb_size = 4096,
    .max_wm_threads = 64,       /* per PSD */
};

static const struct intel_device_info bxt_device_info = {
    .gen = 9,

    .urb_size = 4096,
    .max_wm_threads = 64,       /* per PSD */
    .is_broxton = 1,
};

const struct intel_device_info *
i965_get_device_info(int devid)
{
    switch (devid) {
#undef CHIPSET
#define CHIPSET(id, family, dev, str) case id: return &dev##_device_info;
#include "i965_pciids.h"
    default:
        return NULL;
    }
}

static void cpuid(unsigned int op,
                         uint32_t *eax, uint32_t *ebx,
                         uint32_t *ecx, uint32_t *edx)
{
    __cpuid_count(op, 0, *eax, *ebx, *ecx, *edx);
}

/*
 * This function doesn't check the length. And the caller should
 * assure that the length of input string should be greater than 48.
 */
static int intel_driver_detect_cpustring(char *model_id)
{
    uint32_t *rdata;

    if (model_id == NULL)
        return -EINVAL;

    rdata = (uint32_t *)model_id;

    /* obtain the max supported extended CPUID info */
    cpuid(0x80000000, &rdata[0], &rdata[1], &rdata[2], &rdata[3]);

    /* If the max extended CPUID info is less than 0x80000004, fail */
    if (rdata[0] < 0x80000004)
        return -EINVAL;

    /* obtain the CPUID string */
    cpuid(0x80000002, &rdata[0], &rdata[1], &rdata[2], &rdata[3]);
    cpuid(0x80000003, &rdata[4], &rdata[5], &rdata[6], &rdata[7]);
    cpuid(0x80000004, &rdata[8], &rdata[9], &rdata[10], &rdata[11]);

    *(model_id + 48) = '\0';
    return 0;
}

/*
 * the hook_list for HSW.
 * It is captured by /proc/cpuinfo and the space character is stripped.
 */
const static char *hsw_cpu_hook_list[] =  {
"Intel(R)Pentium(R)3556U",
"Intel(R)Pentium(R)3560Y",
"Intel(R)Pentium(R)3550M",
"Intel(R)Celeron(R)2980U",
"Intel(R)Celeron(R)2955U",
"Intel(R)Celeron(R)2950M",
};

static void hsw_hw_codec_preinit(VADriverContextP ctx, struct hw_codec_info *codec_info)
{
    char model_string[64];
    char *model_ptr, *tmp_ptr;
    int i, model_len, list_len;
    bool found;

    memset(model_string, 0, sizeof(model_string));

    /* If it can't detect cpu model_string, leave it alone */
    if (intel_driver_detect_cpustring(model_string))
        return;

    /* strip the cpufreq info */
    model_ptr = model_string;
    tmp_ptr = strstr(model_ptr, "@");
   
    if (tmp_ptr)
        *tmp_ptr = '\0';

    /* strip the space character and convert to the lower case */
    model_ptr = model_string;
    model_len = strlen(model_string);
    for (i = 0; i < model_len; i++) {
         if (model_string[i] != ' ') {
             *model_ptr = model_string[i];
             model_ptr++;
         }
    }
    *model_ptr = '\0';

    found = false;
    list_len = sizeof(hsw_cpu_hook_list) / sizeof(char *);
    model_len = strlen(model_string);
    for (i = 0; i < list_len; i++) {
        model_ptr = (char *)hsw_cpu_hook_list[i];

        if (strlen(model_ptr) != model_len)
            continue;

        if (strncasecmp(model_string, model_ptr, model_len) == 0) {
            found = true;
            break;
        }
    }

    if (found) {
        codec_info->has_h264_encoding = 0;
        codec_info->has_h264_mvc_encoding = 0;
        codec_info->has_mpeg2_encoding = 0;
    }
    return;
}

/*
 * the hook_list for Sandybride.
 * It is captured by /proc/cpuinfo and the space character is stripped.
 */
const static char *gen6_cpu_hook_list[] =  {
"Intel(R)Celeron(R)CPU847",
"Intel(R)Celeron(R)CPU867",
};

static void gen6_hw_codec_preinit(VADriverContextP ctx, struct hw_codec_info *codec_info)
{
    char model_string[64];
    char *model_ptr, *tmp_ptr;
    int i, model_len, list_len;
    bool found;

    memset(model_string, 0, sizeof(model_string));

    /* If it can't detect cpu model_string, leave it alone */
    if (intel_driver_detect_cpustring(model_string))
        return;

    /* strip the cpufreq info */
    model_ptr = model_string;
    tmp_ptr = strstr(model_ptr, "@");

    if (tmp_ptr)
        *tmp_ptr = '\0';

    /* strip the space character and convert to the lower case */
    model_ptr = model_string;
    model_len = strlen(model_string);
    for (i = 0; i < model_len; i++) {
         if (model_string[i] != ' ') {
             *model_ptr = model_string[i];
             model_ptr++;
         }
    }
    *model_ptr = '\0';

    found = false;
    list_len = sizeof(gen6_cpu_hook_list) / sizeof(char *);
    model_len = strlen(model_string);
    for (i = 0; i < list_len; i++) {
        model_ptr = (char *)gen6_cpu_hook_list[i];

        if (strlen(model_ptr) != model_len)
            continue;

        if (strncasecmp(model_string, model_ptr, model_len) == 0) {
            found = true;
            break;
        }
    }

    if (found) {
        codec_info->has_h264_encoding = 0;
    }
    return;
}

/*
 * the hook_list for Ivybridge.
 * It is captured by /proc/cpuinfo and the space character is stripped.
 */
const static char *gen7_cpu_hook_list[] =  {
"Intel(R)Celeron(R)CPU1007U",
};

static void gen7_hw_codec_preinit(VADriverContextP ctx, struct hw_codec_info *codec_info)
{
    char model_string[64];
    char *model_ptr, *tmp_ptr;
    int i, model_len, list_len;
    bool found;

    memset(model_string, 0, sizeof(model_string));

    /* If it can't detect cpu model_string, leave it alone */
    if (intel_driver_detect_cpustring(model_string))
        return;

    /* strip the cpufreq info */
    model_ptr = model_string;
    tmp_ptr = strstr(model_ptr, "@");

    if (tmp_ptr)
        *tmp_ptr = '\0';

    /* strip the space character and convert to the lower case */
    model_ptr = model_string;
    model_len = strlen(model_string);
    for (i = 0; i < model_len; i++) {
         if (model_string[i] != ' ') {
             *model_ptr = model_string[i];
             model_ptr++;
         }
    }
    *model_ptr = '\0';

    found = false;
    list_len = sizeof(gen7_cpu_hook_list) / sizeof(char *);
    model_len = strlen(model_string);
    for (i = 0; i < list_len; i++) {
        model_ptr = (char *)gen7_cpu_hook_list[i];

        if (strlen(model_ptr) != model_len)
            continue;

        if (strncasecmp(model_string, model_ptr, model_len) == 0) {
            found = true;
            break;
        }
    }

    if (found) {
        codec_info->has_h264_encoding = 0;
        codec_info->has_mpeg2_encoding = 0;
    }
    return;
}
