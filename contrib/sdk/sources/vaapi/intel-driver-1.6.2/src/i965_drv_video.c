/*
 * Copyright ?2009 Intel Corporation
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
 *    Zou Nan hai <nanhai.zou@intel.com>
 *
 */

#include "sysdeps.h"
#include <unistd.h>

#ifdef HAVE_VA_X11
# include "i965_output_dri.h"
#endif

#ifdef HAVE_VA_WAYLAND
# include "i965_output_wayland.h"
#endif

#include "intel_version.h"
#include "intel_driver.h"
#include "intel_memman.h"
#include "intel_batchbuffer.h"
#include "i965_defines.h"
#include "i965_drv_video.h"
#include "i965_decoder.h"
#include "i965_encoder.h"

#define CONFIG_ID_OFFSET                0x01000000
#define CONTEXT_ID_OFFSET               0x02000000
#define SURFACE_ID_OFFSET               0x04000000
#define BUFFER_ID_OFFSET                0x08000000
#define IMAGE_ID_OFFSET                 0x0a000000
#define SUBPIC_ID_OFFSET                0x10000000

#define HAS_MPEG2_DECODING(ctx)  ((ctx)->codec_info->has_mpeg2_decoding && \
                                  (ctx)->intel.has_bsd)

#define HAS_MPEG2_ENCODING(ctx)  ((ctx)->codec_info->has_mpeg2_encoding && \
                                  (ctx)->intel.has_bsd)

#define HAS_H264_DECODING(ctx)  ((ctx)->codec_info->has_h264_decoding && \
                                 (ctx)->intel.has_bsd)

#define HAS_H264_ENCODING(ctx)  ((ctx)->codec_info->has_h264_encoding && \
                                 (ctx)->intel.has_bsd)

#define HAS_VC1_DECODING(ctx)   ((ctx)->codec_info->has_vc1_decoding && \
                                 (ctx)->intel.has_bsd)

#define HAS_JPEG_DECODING(ctx)  ((ctx)->codec_info->has_jpeg_decoding && \
                                 (ctx)->intel.has_bsd)

#define HAS_JPEG_ENCODING(ctx)  ((ctx)->codec_info->has_jpeg_encoding && \
                                 (ctx)->intel.has_bsd)

#define HAS_VPP(ctx)    ((ctx)->codec_info->has_vpp)

#define HAS_ACCELERATED_GETIMAGE(ctx)   ((ctx)->codec_info->has_accelerated_getimage)

#define HAS_ACCELERATED_PUTIMAGE(ctx)   ((ctx)->codec_info->has_accelerated_putimage)

#define HAS_TILED_SURFACE(ctx) ((ctx)->codec_info->has_tiled_surface)

#define HAS_VP8_DECODING(ctx)   ((ctx)->codec_info->has_vp8_decoding && \
                                 (ctx)->intel.has_bsd)

#define HAS_VP8_ENCODING(ctx)   ((ctx)->codec_info->has_vp8_encoding && \
                                 (ctx)->intel.has_bsd)

#define HAS_H264_MVC_DECODING(ctx) \
    (HAS_H264_DECODING(ctx) && (ctx)->codec_info->h264_mvc_dec_profiles)

#define HAS_H264_MVC_DECODING_PROFILE(ctx, profile)                     \
    (HAS_H264_MVC_DECODING(ctx) &&                                      \
     ((ctx)->codec_info->h264_mvc_dec_profiles & (1U << profile)))

#define HAS_H264_MVC_ENCODING(ctx)  ((ctx)->codec_info->has_h264_mvc_encoding && \
                                     (ctx)->intel.has_bsd)

#define HAS_HEVC_DECODING(ctx)          ((ctx)->codec_info->has_hevc_decoding && \
                                         (ctx)->intel.has_bsd)

#define HAS_HEVC_ENCODING(ctx)          ((ctx)->codec_info->has_hevc_encoding && \
                                         (ctx)->intel.has_bsd)

#define HAS_VP9_DECODING(ctx)          ((ctx)->codec_info->has_vp9_decoding && \
                                         (ctx)->intel.has_bsd)

#define HAS_HEVC10_DECODING(ctx)        ((ctx)->codec_info->has_hevc10_decoding && \
                                         (ctx)->intel.has_bsd)

static int get_sampling_from_fourcc(unsigned int fourcc);

/* Check whether we are rendering to X11 (VA/X11 or VA/GLX API) */
#define IS_VA_X11(ctx) \
    (((ctx)->display_type & VA_DISPLAY_MAJOR_MASK) == VA_DISPLAY_X11)

/* Check whether we are rendering to Wayland */
#define IS_VA_WAYLAND(ctx) \
    (((ctx)->display_type & VA_DISPLAY_MAJOR_MASK) == VA_DISPLAY_WAYLAND)

#define I965_BIT        1
#define I965_2BITS      (I965_BIT << 1)
#define I965_4BITS      (I965_BIT << 2)
#define I965_8BITS      (I965_BIT << 3)
#define I965_16BITS     (I965_BIT << 4)
#define I965_32BITS     (I965_BIT << 5)

#define PLANE_0         0
#define PLANE_1         1
#define PLANE_2         2

#define OFFSET_0        0
#define OFFSET_4        4
#define OFFSET_8        8
#define OFFSET_16       16
#define OFFSET_24       24

/* hfactor, vfactor, num_planes, bpp[], num_components, components[] */
#define I_NV12  2, 2, 2, {I965_8BITS, I965_4BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_1, OFFSET_0}, {PLANE_1, OFFSET_8} }
#define I_I420  2, 2, 3, {I965_8BITS, I965_2BITS, I965_2BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_1, OFFSET_0}, {PLANE_2, OFFSET_0} }
#define I_IYUV  I_I420
#define I_IMC3  I_I420
#define I_YV12  2, 2, 3, {I965_8BITS, I965_2BITS, I965_2BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_2, OFFSET_0}, {PLANE_1, OFFSET_0} }
#define I_IMC1  I_YV12

#define I_P010  2, 2, 2, {I965_16BITS, I965_8BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_1, OFFSET_0}, {PLANE_1, OFFSET_16} }

#define I_422H  2, 1, 3, {I965_8BITS, I965_4BITS, I965_4BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_1, OFFSET_0}, {PLANE_2, OFFSET_0} }
#define I_422V  1, 2, 3, {I965_8BITS, I965_4BITS, I965_4BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_1, OFFSET_0}, {PLANE_2, OFFSET_0} }
#define I_YV16  2, 1, 3, {I965_8BITS, I965_4BITS, I965_4BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_2, OFFSET_0}, {PLANE_1, OFFSET_0} }
#define I_YUY2  2, 1, 1, {I965_16BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_0, OFFSET_8}, {PLANE_0, OFFSET_24} }
#define I_UYVY  2, 1, 1, {I965_16BITS}, 3, { {PLANE_0, OFFSET_8}, {PLANE_0, OFFSET_0}, {PLANE_0, OFFSET_16} }

#define I_444P  1, 1, 3, {I965_8BITS, I965_8BITS, I965_8BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_1, OFFSET_0}, {PLANE_2, OFFSET_0} }

#define I_411P  4, 1, 3, {I965_8BITS, I965_2BITS, I965_2BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_1, OFFSET_0}, {PLANE_2, OFFSET_0} }

#define I_Y800  1, 1, 1, {I965_8BITS}, 1, { {PLANE_0, OFFSET_0} }

#define I_RGBA  1, 1, 1, {I965_32BITS}, 4, { {PLANE_0, OFFSET_0}, {PLANE_0, OFFSET_8}, {PLANE_0, OFFSET_16}, {PLANE_0, OFFSET_24} }
#define I_RGBX  1, 1, 1, {I965_32BITS}, 3, { {PLANE_0, OFFSET_0}, {PLANE_0, OFFSET_8}, {PLANE_0, OFFSET_16} }
#define I_BGRA  1, 1, 1, {I965_32BITS}, 4, { {PLANE_0, OFFSET_16}, {PLANE_0, OFFSET_8}, {PLANE_0, OFFSET_0}, {PLANE_0, OFFSET_24} }
#define I_BGRX  1, 1, 1, {I965_32BITS}, 3, { {PLANE_0, OFFSET_16}, {PLANE_0, OFFSET_8}, {PLANE_0, OFFSET_0} }

#define I_ARGB  1, 1, 1, {I965_32BITS}, 4, { {PLANE_0, OFFSET_8}, {PLANE_0, OFFSET_16}, {PLANE_0, OFFSET_24}, {PLANE_0, OFFSET_0} }
#define I_ABGR  1, 1, 1, {I965_32BITS}, 4, { {PLANE_0, OFFSET_24}, {PLANE_0, OFFSET_16}, {PLANE_0, OFFSET_8}, {PLANE_0, OFFSET_0} }

#define I_IA88  1, 1, 1, {I965_16BITS}, 2, { {PLANE_0, OFFSET_0}, {PLANE_0, OFFSET_8} }
#define I_AI88  1, 1, 1, {I965_16BITS}, 2, { {PLANE_0, OFFSET_8}, {PLANE_0, OFFSET_0} }

#define I_IA44  1, 1, 1, {I965_8BITS}, 2, { {PLANE_0, OFFSET_0}, {PLANE_0, OFFSET_4} }
#define I_AI44  1, 1, 1, {I965_8BITS}, 2, { {PLANE_0, OFFSET_4}, {PLANE_0, OFFSET_0} }

/* flag */
#define I_S             1
#define I_I             2
#define I_SI            (I_S | I_I)

#define DEF_FOUCC_INFO(FOURCC, FORMAT, SUB, FLAG)       { VA_FOURCC_##FOURCC, I965_COLOR_##FORMAT, SUBSAMPLE_##SUB, FLAG, I_##FOURCC }
#define DEF_YUV(FOURCC, SUB, FLAG)                      DEF_FOUCC_INFO(FOURCC, YUV, SUB, FLAG)
#define DEF_RGB(FOURCC, SUB, FLAG)                      DEF_FOUCC_INFO(FOURCC, RGB, SUB, FLAG)
#define DEF_INDEX(FOURCC, SUB, FLAG)                    DEF_FOUCC_INFO(FOURCC, INDEX, SUB, FLAG)

static const i965_fourcc_info i965_fourcc_infos[] = {
    DEF_YUV(NV12, YUV420, I_SI),
    DEF_YUV(I420, YUV420, I_SI),
    DEF_YUV(IYUV, YUV420, I_S),
    DEF_YUV(IMC3, YUV420, I_S),
    DEF_YUV(YV12, YUV420, I_SI),
    DEF_YUV(IMC1, YUV420, I_S),

    DEF_YUV(P010, YUV420, I_SI),

    DEF_YUV(422H, YUV422H, I_SI),
    DEF_YUV(422V, YUV422V, I_S),
    DEF_YUV(YV16, YUV422H, I_S),
    DEF_YUV(YUY2, YUV422H, I_SI),
    DEF_YUV(UYVY, YUV422H, I_SI),

    DEF_YUV(444P, YUV444, I_S),

    DEF_YUV(411P, YUV411, I_S),

    DEF_YUV(Y800, YUV400, I_S),

    DEF_RGB(RGBA, RGBX, I_SI),
    DEF_RGB(RGBX, RGBX, I_SI),
    DEF_RGB(BGRA, RGBX, I_SI),
    DEF_RGB(BGRX, RGBX, I_SI),

    DEF_RGB(ARGB, RGBX, I_I),
    DEF_RGB(ABGR, RGBX, I_I),

    DEF_INDEX(IA88, RGBX, I_I),
    DEF_INDEX(AI88, RGBX, I_I),

    DEF_INDEX(IA44, RGBX, I_I),
    DEF_INDEX(AI44, RGBX, I_I)
};

const i965_fourcc_info *
get_fourcc_info(unsigned int fourcc)
{
    unsigned int i;

    for (i = 0; ARRAY_ELEMS(i965_fourcc_infos); i++) {
        const i965_fourcc_info * const info = &i965_fourcc_infos[i];

        if (info->fourcc == fourcc)
            return info;
    }

    return NULL;
}

static int
get_bpp_from_fourcc(unsigned int fourcc)
{
    const i965_fourcc_info *info = get_fourcc_info(fourcc);
    unsigned int i = 0;
    unsigned int bpp = 0;

    if (!info)
        return 0;

    for (i = 0; i < info->num_planes; i++)
        bpp += info->bpp[i];

    return bpp;
}

enum {
    I965_SURFACETYPE_RGBA = 1,
    I965_SURFACETYPE_YUV,
    I965_SURFACETYPE_INDEXED
};

/* List of supported display attributes */
static const VADisplayAttribute i965_display_attributes[] = {
    {
        VADisplayAttribBrightness,
        -100, 100, DEFAULT_BRIGHTNESS,
        VA_DISPLAY_ATTRIB_GETTABLE | VA_DISPLAY_ATTRIB_SETTABLE
    },

    {
        VADisplayAttribContrast,
        0, 100, DEFAULT_CONTRAST,
        VA_DISPLAY_ATTRIB_GETTABLE | VA_DISPLAY_ATTRIB_SETTABLE
    },

    {
        VADisplayAttribHue,
        -180, 180, DEFAULT_HUE,
        VA_DISPLAY_ATTRIB_GETTABLE | VA_DISPLAY_ATTRIB_SETTABLE
    },

    {
        VADisplayAttribSaturation,
        0, 100, DEFAULT_SATURATION,
        VA_DISPLAY_ATTRIB_GETTABLE | VA_DISPLAY_ATTRIB_SETTABLE
    },

    {
        VADisplayAttribRotation,
        0, 3, VA_ROTATION_NONE,
        VA_DISPLAY_ATTRIB_GETTABLE|VA_DISPLAY_ATTRIB_SETTABLE
    },
};

/* List of supported image formats */
typedef struct {
    unsigned int        type;
    VAImageFormat       va_format;
} i965_image_format_map_t;

static const i965_image_format_map_t
i965_image_formats_map[I965_MAX_IMAGE_FORMATS + 1] = {
    { I965_SURFACETYPE_YUV,
      { VA_FOURCC_YV12, VA_LSB_FIRST, 12, } },
    { I965_SURFACETYPE_YUV,
      { VA_FOURCC_I420, VA_LSB_FIRST, 12, } },
    { I965_SURFACETYPE_YUV,
      { VA_FOURCC_NV12, VA_LSB_FIRST, 12, } },
    { I965_SURFACETYPE_YUV,
      { VA_FOURCC_YUY2, VA_LSB_FIRST, 16, } },
    { I965_SURFACETYPE_YUV,
      { VA_FOURCC_UYVY, VA_LSB_FIRST, 16, } },
    { I965_SURFACETYPE_YUV,
      { VA_FOURCC_422H, VA_LSB_FIRST, 16, } },
    { I965_SURFACETYPE_RGBA,
      { VA_FOURCC_RGBX, VA_LSB_FIRST, 32, 24, 0x000000ff, 0x0000ff00, 0x00ff0000 } },
    { I965_SURFACETYPE_RGBA,
      { VA_FOURCC_BGRX, VA_LSB_FIRST, 32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff } },
};

/* List of supported subpicture formats */
typedef struct {
    unsigned int        type;
    unsigned int        format;
    VAImageFormat       va_format;
    unsigned int        va_flags;
} i965_subpic_format_map_t;

#define COMMON_SUBPICTURE_FLAGS                 \
    (VA_SUBPICTURE_DESTINATION_IS_SCREEN_COORD| \
     VA_SUBPICTURE_GLOBAL_ALPHA)

static const i965_subpic_format_map_t
i965_subpic_formats_map[I965_MAX_SUBPIC_FORMATS + 1] = {
    { I965_SURFACETYPE_INDEXED, I965_SURFACEFORMAT_P4A4_UNORM,
      { VA_FOURCC_IA44, VA_MSB_FIRST, 8, },
      COMMON_SUBPICTURE_FLAGS },
    { I965_SURFACETYPE_INDEXED, I965_SURFACEFORMAT_A4P4_UNORM,
      { VA_FOURCC_AI44, VA_MSB_FIRST, 8, },
      COMMON_SUBPICTURE_FLAGS },
    { I965_SURFACETYPE_INDEXED, I965_SURFACEFORMAT_P8A8_UNORM,
      { VA_FOURCC_IA88, VA_MSB_FIRST, 16, },
      COMMON_SUBPICTURE_FLAGS },
    { I965_SURFACETYPE_INDEXED, I965_SURFACEFORMAT_A8P8_UNORM,
      { VA_FOURCC_AI88, VA_MSB_FIRST, 16, },
      COMMON_SUBPICTURE_FLAGS },
     { I965_SURFACETYPE_RGBA, I965_SURFACEFORMAT_B8G8R8A8_UNORM,
      { VA_FOURCC_BGRA, VA_LSB_FIRST, 32,
        32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 },
      COMMON_SUBPICTURE_FLAGS },
    { I965_SURFACETYPE_RGBA, I965_SURFACEFORMAT_R8G8B8A8_UNORM,
      { VA_FOURCC_RGBA, VA_LSB_FIRST, 32,
        32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 },
      COMMON_SUBPICTURE_FLAGS },
};

static const i965_subpic_format_map_t *
get_subpic_format(const VAImageFormat *va_format)
{
    unsigned int i;
    for (i = 0; i965_subpic_formats_map[i].type != 0; i++) {
        const i965_subpic_format_map_t * const m = &i965_subpic_formats_map[i];
        if (m->va_format.fourcc == va_format->fourcc &&
            (m->type == I965_SURFACETYPE_RGBA ?
             (m->va_format.byte_order == va_format->byte_order &&
              m->va_format.red_mask   == va_format->red_mask   &&
              m->va_format.green_mask == va_format->green_mask &&
              m->va_format.blue_mask  == va_format->blue_mask  &&
              m->va_format.alpha_mask == va_format->alpha_mask) : 1))
            return m;
    }
    return NULL;
}

/* Checks whether the surface is in busy state */
static bool
is_surface_busy(struct i965_driver_data *i965,
    struct object_surface *obj_surface)
{
    assert(obj_surface != NULL);

    if (obj_surface->locked_image_id != VA_INVALID_ID)
        return true;
    if (obj_surface->derived_image_id != VA_INVALID_ID)
        return true;
    return false;
}

/* Checks whether the image is in busy state */
static bool
is_image_busy(struct i965_driver_data *i965, struct object_image *obj_image, VASurfaceID surface)
{
    struct object_buffer *obj_buffer;

    assert(obj_image != NULL);

    if (obj_image->derived_surface != VA_INVALID_ID &&
        obj_image->derived_surface == surface)
        return true;

    obj_buffer = BUFFER(obj_image->image.buf);
    if (obj_buffer && obj_buffer->export_refcount > 0)
        return true;
    return false;
}

#define I965_PACKED_HEADER_BASE         0
#define I965_SEQ_PACKED_HEADER_BASE     0
#define I965_SEQ_PACKED_HEADER_END      2
#define I965_PIC_PACKED_HEADER_BASE     2
#define I965_PACKED_MISC_HEADER_BASE    4

int
va_enc_packed_type_to_idx(int packed_type)
{
    int idx = 0;

    if (packed_type & VAEncPackedHeaderMiscMask) {
        idx = I965_PACKED_MISC_HEADER_BASE;
        packed_type = (~VAEncPackedHeaderMiscMask & packed_type);
        ASSERT_RET(packed_type > 0, 0);
        idx += (packed_type - 1);
    } else {
        idx = I965_PACKED_HEADER_BASE;

        switch (packed_type) {
        case VAEncPackedHeaderSequence:
            idx = I965_SEQ_PACKED_HEADER_BASE + 0;
            break;

        case VAEncPackedHeaderPicture:
            idx = I965_PIC_PACKED_HEADER_BASE + 0;
            break;

        case VAEncPackedHeaderSlice:
            idx = I965_PIC_PACKED_HEADER_BASE + 1;
            break;

        default:
            /* Should not get here */
            ASSERT_RET(0, 0);
            break;
        }
    }

    ASSERT_RET(idx < 5, 0);
    return idx;
}

#define CALL_VTABLE(vawr, status, param) status = (vawr->vtable->param)

static VAStatus
i965_surface_wrapper(VADriverContextP ctx, VASurfaceID surface)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_surface *obj_surface = SURFACE(surface);
    VAStatus va_status = VA_STATUS_SUCCESS;

    if (!obj_surface) {
        return VA_STATUS_ERROR_INVALID_SURFACE;
    }

    if (obj_surface->wrapper_surface != VA_INVALID_ID) {
        /* the wrapped surface already exists. just return it */
       return va_status;
    }

    if (obj_surface->fourcc == 0)
        i965_check_alloc_surface_bo(ctx, obj_surface,
                                    1, VA_FOURCC_NV12, SUBSAMPLE_YUV420);

    /*
     * TBD: Support more surface formats.
     * Currently only NV12 is support as NV12 is used by decoding.
     */
    if (obj_surface->fourcc != VA_FOURCC_NV12 )
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    if ((i965->wrapper_pdrvctx == NULL) ||
        (obj_surface->bo == NULL))
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    {
        int fd_handle;
        VASurfaceAttrib attrib_list[2];
        VASurfaceAttribExternalBuffers buffer_descriptor;
        VAGenericID wrapper_surface;

//        if (drm_intel_bo_gem_export_to_prime(obj_surface->bo, &fd_handle) != 0)
            return VA_STATUS_ERROR_OPERATION_FAILED;

        obj_surface->exported_primefd = fd_handle;

        memset(&attrib_list, 0, sizeof(attrib_list));
        memset(&buffer_descriptor, 0, sizeof(buffer_descriptor));

        attrib_list[0].type = VASurfaceAttribExternalBufferDescriptor;
        attrib_list[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
        attrib_list[0].value.value.p = &buffer_descriptor;
        attrib_list[0].value.type = VAGenericValueTypePointer;

        attrib_list[1].type = VASurfaceAttribMemoryType;
        attrib_list[1].flags = VA_SURFACE_ATTRIB_SETTABLE;
        attrib_list[1].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
        attrib_list[1].value.type = VAGenericValueTypeInteger;

        buffer_descriptor.num_buffers = 1;
        buffer_descriptor.num_planes = 2;
        buffer_descriptor.width = obj_surface->orig_width;
        buffer_descriptor.height = obj_surface->orig_height;
        buffer_descriptor.pixel_format = obj_surface->fourcc;
        buffer_descriptor.data_size = obj_surface->size;
        buffer_descriptor.pitches[0] = obj_surface->width;
        buffer_descriptor.pitches[1] = obj_surface->cb_cr_pitch;
        buffer_descriptor.offsets[0] = 0;
        buffer_descriptor.offsets[1] = obj_surface->width * obj_surface->height;
        buffer_descriptor.buffers = (void *)&fd_handle;

        CALL_VTABLE(i965->wrapper_pdrvctx, va_status,
                    vaCreateSurfaces2(i965->wrapper_pdrvctx,
                                      VA_RT_FORMAT_YUV420,
                                      obj_surface->orig_width,
                                      obj_surface->orig_height,
                                      &wrapper_surface, 1,
                                      attrib_list, 2));

        if (va_status == VA_STATUS_SUCCESS) {
            obj_surface->wrapper_surface = wrapper_surface;
        } else {
            /* This needs to be checked */
            va_status = VA_STATUS_ERROR_OPERATION_FAILED;
        }
        return va_status;
    }

}

VAStatus
i965_QueryConfigProfiles(VADriverContextP ctx,
                         VAProfile *profile_list,       /* out */
                         int *num_profiles)             /* out */
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    int i = 0;

    if (HAS_MPEG2_DECODING(i965) ||
        HAS_MPEG2_ENCODING(i965)) {
        profile_list[i++] = VAProfileMPEG2Simple;
        profile_list[i++] = VAProfileMPEG2Main;
    }

    if (HAS_H264_DECODING(i965) ||
        HAS_H264_ENCODING(i965)) {
        profile_list[i++] = VAProfileH264ConstrainedBaseline;
        profile_list[i++] = VAProfileH264Main;
        profile_list[i++] = VAProfileH264High;
    }
    if (HAS_H264_MVC_DECODING_PROFILE(i965, VAProfileH264MultiviewHigh))
        profile_list[i++] = VAProfileH264MultiviewHigh;
    if (HAS_H264_MVC_DECODING_PROFILE(i965, VAProfileH264StereoHigh))
        profile_list[i++] = VAProfileH264StereoHigh;

    if (HAS_VC1_DECODING(i965)) {
        profile_list[i++] = VAProfileVC1Simple;
        profile_list[i++] = VAProfileVC1Main;
        profile_list[i++] = VAProfileVC1Advanced;
    }

    if (HAS_VPP(i965)) {
        profile_list[i++] = VAProfileNone;
    }

    if (HAS_JPEG_DECODING(i965) ||
        HAS_JPEG_ENCODING(i965)) {
        profile_list[i++] = VAProfileJPEGBaseline;
    }

    if (HAS_VP8_DECODING(i965) ||
        HAS_VP8_ENCODING(i965)) {
        profile_list[i++] = VAProfileVP8Version0_3;
    }

    if (HAS_H264_MVC_ENCODING(i965)) {
        profile_list[i++] = VAProfileH264MultiviewHigh;
        profile_list[i++] = VAProfileH264StereoHigh;
    }

    if (HAS_HEVC_DECODING(i965)||
        HAS_HEVC_ENCODING(i965)) {
        profile_list[i++] = VAProfileHEVCMain;
    }

    if (HAS_HEVC10_DECODING(i965)) {
        profile_list[i++] = VAProfileHEVCMain10;
    }

    if(HAS_VP9_DECODING(i965)) {
        profile_list[i++] = VAProfileVP9Profile0;
    }

    if (i965->wrapper_pdrvctx) {
        VAProfile wrapper_list[4];
        int wrapper_num;
        VADriverContextP pdrvctx;
        VAStatus va_status;

        pdrvctx = i965->wrapper_pdrvctx;
        CALL_VTABLE(pdrvctx, va_status,
                    vaQueryConfigProfiles(pdrvctx,
                                          wrapper_list, &wrapper_num));

        if (va_status == VA_STATUS_SUCCESS) {
            int j;
            for (j = 0; j < wrapper_num; j++)
                if (wrapper_list[j] != VAProfileNone)
                    profile_list[i++] = wrapper_list[j];
        }
    }

    /* If the assert fails then I965_MAX_PROFILES needs to be bigger */
    ASSERT_RET(i <= I965_MAX_PROFILES, VA_STATUS_ERROR_OPERATION_FAILED);
    *num_profiles = i;

    return VA_STATUS_SUCCESS;
}

VAStatus
i965_QueryConfigEntrypoints(VADriverContextP ctx,
                            VAProfile profile,
                            VAEntrypoint *entrypoint_list,      /* out */
                            int *num_entrypoints)               /* out */
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    int n = 0;

    switch (profile) {
    case VAProfileMPEG2Simple:
    case VAProfileMPEG2Main:
        if (HAS_MPEG2_DECODING(i965))
            entrypoint_list[n++] = VAEntrypointVLD;

        if (HAS_MPEG2_ENCODING(i965))
            entrypoint_list[n++] = VAEntrypointEncSlice;

        break;

    case VAProfileH264ConstrainedBaseline:
    case VAProfileH264Main:
    case VAProfileH264High:
        if (HAS_H264_DECODING(i965))
            entrypoint_list[n++] = VAEntrypointVLD;

        if (HAS_H264_ENCODING(i965))
            entrypoint_list[n++] = VAEntrypointEncSlice;

        break;
   case VAProfileH264MultiviewHigh:
   case VAProfileH264StereoHigh:
       if (HAS_H264_MVC_DECODING_PROFILE(i965, profile))
            entrypoint_list[n++] = VAEntrypointVLD;

       if (HAS_H264_MVC_ENCODING(i965))
            entrypoint_list[n++] = VAEntrypointEncSlice;
        break;

    case VAProfileVC1Simple:
    case VAProfileVC1Main:
    case VAProfileVC1Advanced:
        if (HAS_VC1_DECODING(i965))
            entrypoint_list[n++] = VAEntrypointVLD;
        break;

    case VAProfileNone:
        if (HAS_VPP(i965))
            entrypoint_list[n++] = VAEntrypointVideoProc;
        break;

    case VAProfileJPEGBaseline:
        if (HAS_JPEG_DECODING(i965))
            entrypoint_list[n++] = VAEntrypointVLD;

        if (HAS_JPEG_ENCODING(i965))
            entrypoint_list[n++] = VAEntrypointEncPicture;
        break;

    case VAProfileVP8Version0_3:
        if (HAS_VP8_DECODING(i965))
            entrypoint_list[n++] = VAEntrypointVLD;

        if (HAS_VP8_ENCODING(i965))
            entrypoint_list[n++] = VAEntrypointEncSlice;

        break;

    case VAProfileHEVCMain:
        if (HAS_HEVC_DECODING(i965))
            entrypoint_list[n++] = VAEntrypointVLD;

        if (HAS_HEVC_ENCODING(i965))
            entrypoint_list[n++] = VAEntrypointEncSlice;

        break;

    case VAProfileHEVCMain10:
        if (HAS_HEVC10_DECODING(i965))
            entrypoint_list[n++] = VAEntrypointVLD;

        break;

    case VAProfileVP9Profile0:
        if(HAS_VP9_DECODING(i965))
            entrypoint_list[n++] = VAEntrypointVLD;

        if (i965->wrapper_pdrvctx) {
            VAStatus va_status = VA_STATUS_SUCCESS;
            VADriverContextP pdrvctx = i965->wrapper_pdrvctx;

            CALL_VTABLE(pdrvctx, va_status,
                        vaQueryConfigEntrypoints(pdrvctx, profile,
                                                 entrypoint_list,
                                                 num_entrypoints));
            return va_status;
        }

        break;

    default:
        break;
    }

    /* If the assert fails then I965_MAX_ENTRYPOINTS needs to be bigger */
    ASSERT_RET(n <= I965_MAX_ENTRYPOINTS, VA_STATUS_ERROR_OPERATION_FAILED);
    *num_entrypoints = n;
    return n > 0 ? VA_STATUS_SUCCESS : VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
}

static VAStatus
i965_validate_config(VADriverContextP ctx, VAProfile profile,
    VAEntrypoint entrypoint)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    VAStatus va_status;

    /* Validate profile & entrypoint */
    switch (profile) {
    case VAProfileMPEG2Simple:
    case VAProfileMPEG2Main:
        if ((HAS_MPEG2_DECODING(i965) && entrypoint == VAEntrypointVLD) ||
            (HAS_MPEG2_ENCODING(i965) && entrypoint == VAEntrypointEncSlice)) {
            va_status = VA_STATUS_SUCCESS;
        } else {
            va_status = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        }
        break;

    case VAProfileH264ConstrainedBaseline:
    case VAProfileH264Main:
    case VAProfileH264High:
        if ((HAS_H264_DECODING(i965) && entrypoint == VAEntrypointVLD) ||
            (HAS_H264_ENCODING(i965) && entrypoint == VAEntrypointEncSlice)) {
            va_status = VA_STATUS_SUCCESS;
        } else {
            va_status = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        }
        break;

    case VAProfileVC1Simple:
    case VAProfileVC1Main:
    case VAProfileVC1Advanced:
        if (HAS_VC1_DECODING(i965) && entrypoint == VAEntrypointVLD) {
            va_status = VA_STATUS_SUCCESS;
        } else {
            va_status = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        }
        break;

    case VAProfileNone:
        if (HAS_VPP(i965) && VAEntrypointVideoProc == entrypoint) {
            va_status = VA_STATUS_SUCCESS;
        } else {
            va_status = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        }
        break;

    case VAProfileJPEGBaseline:
        if ((HAS_JPEG_DECODING(i965) && entrypoint == VAEntrypointVLD) ||
            (HAS_JPEG_ENCODING(i965) && entrypoint == VAEntrypointEncPicture)) {
            va_status = VA_STATUS_SUCCESS;
        } else {
            va_status = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        }
        break;

    case VAProfileVP8Version0_3:
        if ((HAS_VP8_DECODING(i965) && entrypoint == VAEntrypointVLD) ||
            (HAS_VP8_ENCODING(i965) && entrypoint == VAEntrypointEncSlice)) {
            va_status = VA_STATUS_SUCCESS;
        } else {
            va_status = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        }
        break;

    case VAProfileH264MultiviewHigh:
    case VAProfileH264StereoHigh:
        if ((HAS_H264_MVC_DECODING_PROFILE(i965, profile) &&
             entrypoint == VAEntrypointVLD) ||
            (HAS_H264_MVC_ENCODING(i965) && entrypoint == VAEntrypointEncSlice)) {
            va_status = VA_STATUS_SUCCESS;
        } else {
            va_status = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        }

        break;

    case VAProfileHEVCMain:
        if ((HAS_HEVC_DECODING(i965) && (entrypoint == VAEntrypointVLD))||
            (HAS_HEVC_ENCODING(i965) && (entrypoint == VAEntrypointEncSlice)))
            va_status = VA_STATUS_SUCCESS;
        else
            va_status = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;

        break;

    case VAProfileHEVCMain10:
        if (HAS_HEVC10_DECODING(i965) && (entrypoint == VAEntrypointVLD))
            va_status = VA_STATUS_SUCCESS;
        else
            va_status = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;

        break;

    case VAProfileVP9Profile0:
        if ((HAS_VP9_DECODING(i965)) && (entrypoint == VAEntrypointVLD))
            va_status = VA_STATUS_SUCCESS;
        else if (i965->wrapper_pdrvctx)
            va_status = VA_STATUS_SUCCESS;
        else
            va_status = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        break;

    default:
        va_status = VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        break;
    }
    return va_status;
}

static uint32_t
i965_get_default_chroma_formats(VADriverContextP ctx, VAProfile profile,
    VAEntrypoint entrypoint)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    uint32_t chroma_formats = VA_RT_FORMAT_YUV420;

    switch (profile) {
    case VAProfileH264ConstrainedBaseline:
    case VAProfileH264Main:
    case VAProfileH264High:
        if (HAS_H264_DECODING(i965) && entrypoint == VAEntrypointVLD)
            chroma_formats |= i965->codec_info->h264_dec_chroma_formats;
        break;

    case VAProfileH264MultiviewHigh:
    case VAProfileH264StereoHigh:
        if (HAS_H264_MVC_DECODING(i965) && entrypoint == VAEntrypointVLD)
            chroma_formats |= i965->codec_info->h264_dec_chroma_formats;
        break;

    case VAProfileJPEGBaseline:
        if (HAS_JPEG_DECODING(i965) && entrypoint == VAEntrypointVLD)
            chroma_formats |= i965->codec_info->jpeg_dec_chroma_formats;
        if (HAS_JPEG_ENCODING(i965) && entrypoint == VAEntrypointEncPicture)
            chroma_formats |= i965->codec_info->jpeg_enc_chroma_formats;
        break;

    case VAProfileHEVCMain10:
        chroma_formats = 0; // clear YUV420 8bits format support
        if (HAS_HEVC10_DECODING(i965) && entrypoint == VAEntrypointVLD)
            chroma_formats |= i965->codec_info->hevc_dec_chroma_formats;
        break;
    default:
        break;
    }
    return chroma_formats;
}

VAStatus
i965_GetConfigAttributes(VADriverContextP ctx,
                         VAProfile profile,
                         VAEntrypoint entrypoint,
                         VAConfigAttrib *attrib_list,  /* in/out */
                         int num_attribs)
{
    VAStatus va_status;
    int i;

    va_status = i965_validate_config(ctx, profile, entrypoint);
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    /* Other attributes don't seem to be defined */
    /* What to do if we don't know the attribute? */
    for (i = 0; i < num_attribs; i++) {
        attrib_list[i].value = VA_ATTRIB_NOT_SUPPORTED;
        switch (attrib_list[i].type) {
        case VAConfigAttribRTFormat:
            attrib_list[i].value = i965_get_default_chroma_formats(ctx,
                profile, entrypoint);
            break;

        case VAConfigAttribRateControl:
            if (entrypoint == VAEntrypointEncSlice) {
                attrib_list[i].value = VA_RC_CQP;

                if (profile != VAProfileMPEG2Main &&
                    profile != VAProfileMPEG2Simple)
                    attrib_list[i].value |= VA_RC_CBR;
                break;
            }
            break;

        case VAConfigAttribEncPackedHeaders:
            if (entrypoint == VAEntrypointEncSlice) {
                attrib_list[i].value = VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_MISC;
                if (profile == VAProfileH264ConstrainedBaseline ||
                    profile == VAProfileH264Main ||
                    profile == VAProfileH264High ||
                    profile == VAProfileH264StereoHigh ||
                    profile == VAProfileH264MultiviewHigh ||
                    profile == VAProfileHEVCMain) {
                    attrib_list[i].value |= (VA_ENC_PACKED_HEADER_RAW_DATA |
                                             VA_ENC_PACKED_HEADER_SLICE);
                }
                break;
            }
            else if (entrypoint == VAEntrypointEncPicture) {
                if (profile == VAProfileJPEGBaseline)
                    attrib_list[i].value = VA_ENC_PACKED_HEADER_RAW_DATA;
            }
            break;

        case VAConfigAttribEncMaxRefFrames:
            if (entrypoint == VAEntrypointEncSlice) {
                attrib_list[i].value = (1 << 16) | (1 << 0);
                break;
            }
            break;

        case VAConfigAttribEncQualityRange:
            if (entrypoint == VAEntrypointEncSlice) {
                attrib_list[i].value = 1;
                if (profile == VAProfileH264ConstrainedBaseline ||
                    profile == VAProfileH264Main ||
                    profile == VAProfileH264High )
                    attrib_list[i].value = ENCODER_QUALITY_RANGE;
                break;
            }
            break;

        case VAConfigAttribEncJPEG:
            if( entrypoint == VAEntrypointEncPicture) {
                VAConfigAttribValEncJPEG *configVal = (VAConfigAttribValEncJPEG*)&(attrib_list[i].value);
                (configVal->bits).arithmatic_coding_mode = 0; // Huffman coding is used
                (configVal->bits).progressive_dct_mode = 0;   // Only Sequential DCT is supported
                (configVal->bits).non_interleaved_mode = 1;   // Support both interleaved and non-interleaved
                (configVal->bits).differential_mode = 0;      // Baseline DCT is non-differential
                (configVal->bits).max_num_components = 3;     // Only 3 components supported
                (configVal->bits).max_num_scans = 1;          // Only 1 scan per frame
                (configVal->bits).max_num_huffman_tables = 3; // Max 3 huffman tables
                (configVal->bits).max_num_quantization_tables = 3; // Max 3 quantization tables
            }
            break;

        case VAConfigAttribDecSliceMode:
            attrib_list[i].value = VA_DEC_SLICE_MODE_NORMAL;
            break;

        default:
            /* Do nothing */
            attrib_list[i].value = VA_ATTRIB_NOT_SUPPORTED;
            break;
        }
    }

    return VA_STATUS_SUCCESS;
}

static void
i965_destroy_config(struct object_heap *heap, struct object_base *obj)
{
    object_heap_free(heap, obj);
}

static VAConfigAttrib *
i965_lookup_config_attribute(struct object_config *obj_config,
    VAConfigAttribType type)
{
    int i;

    for (i = 0; i < obj_config->num_attribs; i++) {
        VAConfigAttrib * const attrib = &obj_config->attrib_list[i];
        if (attrib->type == type)
            return attrib;
    }
    return NULL;
}

static VAStatus
i965_append_config_attribute(struct object_config *obj_config,
    const VAConfigAttrib *new_attrib)
{
    VAConfigAttrib *attrib;

    if (obj_config->num_attribs >= I965_MAX_CONFIG_ATTRIBUTES)
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;

    attrib = &obj_config->attrib_list[obj_config->num_attribs++];
    attrib->type = new_attrib->type;
    attrib->value = new_attrib->value;
    return VA_STATUS_SUCCESS;
}

static VAStatus
i965_ensure_config_attribute(struct object_config *obj_config,
    const VAConfigAttrib *new_attrib)
{
    VAConfigAttrib *attrib;

    /* Check for existing attributes */
    attrib = i965_lookup_config_attribute(obj_config, new_attrib->type);
    if (attrib) {
        /* Update existing attribute */
        attrib->value = new_attrib->value;
        return VA_STATUS_SUCCESS;
    }
    return i965_append_config_attribute(obj_config, new_attrib);
}

VAStatus
i965_CreateConfig(VADriverContextP ctx,
                  VAProfile profile,
                  VAEntrypoint entrypoint,
                  VAConfigAttrib *attrib_list,
                  int num_attribs,
                  VAConfigID *config_id)        /* out */
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    struct object_config *obj_config;
    int configID;
    int i;
    VAStatus vaStatus;

    vaStatus = i965_validate_config(ctx, profile, entrypoint);

    if (VA_STATUS_SUCCESS != vaStatus) {
        return vaStatus;
    }

    configID = NEW_CONFIG_ID();
    obj_config = CONFIG(configID);

    if (NULL == obj_config) {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    obj_config->profile = profile;
    obj_config->entrypoint = entrypoint;
    obj_config->num_attribs = 0;
    obj_config->wrapper_config = VA_INVALID_ID;

    for (i = 0; i < num_attribs; i++) {
        vaStatus = i965_ensure_config_attribute(obj_config, &attrib_list[i]);
        if (vaStatus != VA_STATUS_SUCCESS)
            break;
    }

    if (vaStatus == VA_STATUS_SUCCESS) {
        VAConfigAttrib attrib, *attrib_found;
        attrib.type = VAConfigAttribRTFormat;
        attrib.value = i965_get_default_chroma_formats(ctx, profile, entrypoint);
        attrib_found = i965_lookup_config_attribute(obj_config, attrib.type);
        if (!attrib_found || !attrib_found->value)
            vaStatus = i965_append_config_attribute(obj_config, &attrib);
        else if (!(attrib_found->value & attrib.value))
            vaStatus = VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    if ((vaStatus == VA_STATUS_SUCCESS) &&
        (profile == VAProfileVP9Profile0)) {

        if (i965->wrapper_pdrvctx) {
            VAGenericID wrapper_config;

            CALL_VTABLE(i965->wrapper_pdrvctx, vaStatus,
                        vaCreateConfig(i965->wrapper_pdrvctx, profile,
                                       entrypoint, attrib_list,
                                       num_attribs, &wrapper_config));

            if (vaStatus == VA_STATUS_SUCCESS)
                obj_config->wrapper_config = wrapper_config;
        }
    }

    /* Error recovery */
    if (VA_STATUS_SUCCESS != vaStatus) {
        i965_destroy_config(&i965->config_heap, (struct object_base *)obj_config);
    } else {
        *config_id = configID;
    }

    return vaStatus;
}

VAStatus
i965_DestroyConfig(VADriverContextP ctx, VAConfigID config_id)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_config *obj_config = CONFIG(config_id);
    VAStatus vaStatus;

    if (NULL == obj_config) {
        vaStatus = VA_STATUS_ERROR_INVALID_CONFIG;
        return vaStatus;
    }

    if ((obj_config->wrapper_config != VA_INVALID_ID) &&
        i965->wrapper_pdrvctx) {
        CALL_VTABLE(i965->wrapper_pdrvctx, vaStatus,
                    vaDestroyConfig(i965->wrapper_pdrvctx,
                                    obj_config->wrapper_config));
        obj_config->wrapper_config = VA_INVALID_ID;
    }

    i965_destroy_config(&i965->config_heap, (struct object_base *)obj_config);
    return VA_STATUS_SUCCESS;
}

VAStatus i965_QueryConfigAttributes(VADriverContextP ctx,
                                    VAConfigID config_id,
                                    VAProfile *profile,                 /* out */
                                    VAEntrypoint *entrypoint,           /* out */
                                    VAConfigAttrib *attrib_list,        /* out */
                                    int *num_attribs)                   /* out */
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_config *obj_config = CONFIG(config_id);
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int i;

    ASSERT_RET(obj_config, VA_STATUS_ERROR_INVALID_CONFIG);
    *profile = obj_config->profile;
    *entrypoint = obj_config->entrypoint;
    *num_attribs = obj_config->num_attribs;

    for(i = 0; i < obj_config->num_attribs; i++) {
        attrib_list[i] = obj_config->attrib_list[i];
    }

    return vaStatus;
}

void
i965_destroy_surface_storage(struct object_surface *obj_surface)
{
    if (!obj_surface)
        return;

    dri_bo_unreference(obj_surface->bo);
    obj_surface->bo = NULL;

    if (obj_surface->free_private_data != NULL) {
        obj_surface->free_private_data(&obj_surface->private_data);
        obj_surface->private_data = NULL;
    }
}

static void
i965_destroy_surface(struct object_heap *heap, struct object_base *obj)
{
    struct object_surface *obj_surface = (struct object_surface *)obj;

    i965_destroy_surface_storage(obj_surface);
    object_heap_free(heap, obj);
}

static VAStatus
i965_surface_native_memory(VADriverContextP ctx,
                           struct object_surface *obj_surface,
                           int format,
                           int expected_fourcc)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int tiling = HAS_TILED_SURFACE(i965);

    if (!expected_fourcc)
        return VA_STATUS_SUCCESS;

    // todo, should we disable tiling for 422 format?
    if (expected_fourcc == VA_FOURCC_I420 ||
        expected_fourcc == VA_FOURCC_IYUV ||
        expected_fourcc == VA_FOURCC_YV12 ||
        expected_fourcc == VA_FOURCC_YV16)
        tiling = 0;

    return i965_check_alloc_surface_bo(ctx, obj_surface, tiling, expected_fourcc, get_sampling_from_fourcc(expected_fourcc));
}

static VAStatus
i965_suface_external_memory(VADriverContextP ctx,
                            struct object_surface *obj_surface,
                            int external_memory_type,
                            VASurfaceAttribExternalBuffers *memory_attibute,
                            int index)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    if (!memory_attibute ||
        !memory_attibute->buffers ||
        index > memory_attibute->num_buffers)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    ASSERT_RET(obj_surface->orig_width == memory_attibute->width, VA_STATUS_ERROR_INVALID_PARAMETER);
    ASSERT_RET(obj_surface->orig_height == memory_attibute->height, VA_STATUS_ERROR_INVALID_PARAMETER);
    ASSERT_RET(memory_attibute->num_planes >= 1, VA_STATUS_ERROR_INVALID_PARAMETER);

    obj_surface->fourcc = memory_attibute->pixel_format;
    obj_surface->width = memory_attibute->pitches[0];
    obj_surface->size = memory_attibute->data_size;

    if (memory_attibute->num_planes == 1)
        obj_surface->height = memory_attibute->data_size / obj_surface->width;
    else
        obj_surface->height = memory_attibute->offsets[1] / obj_surface->width;

    obj_surface->x_cb_offset = 0; /* X offset is always 0 */
    obj_surface->x_cr_offset = 0;

    switch (obj_surface->fourcc) {
    case VA_FOURCC_NV12:
    case VA_FOURCC_P010:
        ASSERT_RET(memory_attibute->num_planes == 2, VA_STATUS_ERROR_INVALID_PARAMETER);
        ASSERT_RET(memory_attibute->pitches[0] == memory_attibute->pitches[1], VA_STATUS_ERROR_INVALID_PARAMETER);

        obj_surface->subsampling = SUBSAMPLE_YUV420;
        obj_surface->y_cb_offset = obj_surface->height;
        obj_surface->y_cr_offset = obj_surface->height;
        obj_surface->cb_cr_width = obj_surface->orig_width / 2;
        obj_surface->cb_cr_height = obj_surface->orig_height / 2;
        obj_surface->cb_cr_pitch = memory_attibute->pitches[1];

        break;

    case VA_FOURCC_YV12:
    case VA_FOURCC_IMC1:
        ASSERT_RET(memory_attibute->num_planes == 3, VA_STATUS_ERROR_INVALID_PARAMETER);
        ASSERT_RET(memory_attibute->pitches[1] == memory_attibute->pitches[2], VA_STATUS_ERROR_INVALID_PARAMETER);

        obj_surface->subsampling = SUBSAMPLE_YUV420;
        obj_surface->y_cr_offset = obj_surface->height;
        obj_surface->y_cb_offset = memory_attibute->offsets[2] / obj_surface->width;
        obj_surface->cb_cr_width = obj_surface->orig_width / 2;
        obj_surface->cb_cr_height = obj_surface->orig_height / 2;
        obj_surface->cb_cr_pitch = memory_attibute->pitches[1];

        break;

    case VA_FOURCC_I420:
    case VA_FOURCC_IYUV:
    case VA_FOURCC_IMC3:
        ASSERT_RET(memory_attibute->num_planes == 3, VA_STATUS_ERROR_INVALID_PARAMETER);
        ASSERT_RET(memory_attibute->pitches[1] == memory_attibute->pitches[2], VA_STATUS_ERROR_INVALID_PARAMETER);

        obj_surface->subsampling = SUBSAMPLE_YUV420;
        obj_surface->y_cb_offset = obj_surface->height;
        obj_surface->y_cr_offset = memory_attibute->offsets[2] / obj_surface->width;
        obj_surface->cb_cr_width = obj_surface->orig_width / 2;
        obj_surface->cb_cr_height = obj_surface->orig_height / 2;
        obj_surface->cb_cr_pitch = memory_attibute->pitches[1];

        break;

    case VA_FOURCC_YUY2:
    case VA_FOURCC_UYVY:
        ASSERT_RET(memory_attibute->num_planes == 1, VA_STATUS_ERROR_INVALID_PARAMETER);

        obj_surface->subsampling = SUBSAMPLE_YUV422H;
        obj_surface->y_cb_offset = 0;
        obj_surface->y_cr_offset = 0;
        obj_surface->cb_cr_width = obj_surface->orig_width / 2;
        obj_surface->cb_cr_height = obj_surface->orig_height;
        obj_surface->cb_cr_pitch = memory_attibute->pitches[0];

        break;

    case VA_FOURCC_RGBA:
    case VA_FOURCC_RGBX:
    case VA_FOURCC_BGRA:
    case VA_FOURCC_BGRX:
        ASSERT_RET(memory_attibute->num_planes == 1, VA_STATUS_ERROR_INVALID_PARAMETER);

        obj_surface->subsampling = SUBSAMPLE_RGBX;
        obj_surface->y_cb_offset = 0;
        obj_surface->y_cr_offset = 0;
        obj_surface->cb_cr_width = 0;
        obj_surface->cb_cr_height = 0;
        obj_surface->cb_cr_pitch = 0;

        break;

    case VA_FOURCC_Y800: /* monochrome surface */
        ASSERT_RET(memory_attibute->num_planes == 1, VA_STATUS_ERROR_INVALID_PARAMETER);

        obj_surface->subsampling = SUBSAMPLE_YUV400;
        obj_surface->y_cb_offset = 0;
        obj_surface->y_cr_offset = 0;
        obj_surface->cb_cr_width = 0;
        obj_surface->cb_cr_height = 0;
        obj_surface->cb_cr_pitch = 0;

        break;

    case VA_FOURCC_411P:
        ASSERT_RET(memory_attibute->num_planes == 3, VA_STATUS_ERROR_INVALID_PARAMETER);
        ASSERT_RET(memory_attibute->pitches[1] == memory_attibute->pitches[2], VA_STATUS_ERROR_INVALID_PARAMETER);

        obj_surface->subsampling = SUBSAMPLE_YUV411;
        obj_surface->y_cb_offset = 0;
        obj_surface->y_cr_offset = 0;
        obj_surface->cb_cr_width = obj_surface->orig_width / 4;
        obj_surface->cb_cr_height = obj_surface->orig_height;
        obj_surface->cb_cr_pitch = memory_attibute->pitches[1];

        break;

    case VA_FOURCC_422H:
        ASSERT_RET(memory_attibute->num_planes == 3, VA_STATUS_ERROR_INVALID_PARAMETER);
        ASSERT_RET(memory_attibute->pitches[1] == memory_attibute->pitches[2], VA_STATUS_ERROR_INVALID_PARAMETER);

        obj_surface->subsampling = SUBSAMPLE_YUV422H;
        obj_surface->y_cb_offset = obj_surface->height;
        obj_surface->y_cr_offset = memory_attibute->offsets[2] / obj_surface->width;
        obj_surface->cb_cr_width = obj_surface->orig_width / 2;
        obj_surface->cb_cr_height = obj_surface->orig_height;
        obj_surface->cb_cr_pitch = memory_attibute->pitches[1];

        break;

    case VA_FOURCC_YV16:
        assert(memory_attibute->num_planes == 3);
        assert(memory_attibute->pitches[1] == memory_attibute->pitches[2]);

        obj_surface->subsampling = SUBSAMPLE_YUV422H;
        obj_surface->y_cr_offset = memory_attibute->offsets[1] / obj_surface->width;
        obj_surface->y_cb_offset = memory_attibute->offsets[2] / obj_surface->width;
        obj_surface->cb_cr_width = obj_surface->orig_width / 2;
        obj_surface->cb_cr_height = obj_surface->orig_height;
        obj_surface->cb_cr_pitch = memory_attibute->pitches[1];

        break;

    case VA_FOURCC_422V:
        ASSERT_RET(memory_attibute->num_planes == 3, VA_STATUS_ERROR_INVALID_PARAMETER);
        ASSERT_RET(memory_attibute->pitches[1] == memory_attibute->pitches[2], VA_STATUS_ERROR_INVALID_PARAMETER);

        obj_surface->subsampling = SUBSAMPLE_YUV422H;
        obj_surface->y_cb_offset = obj_surface->height;
        obj_surface->y_cr_offset = memory_attibute->offsets[2] / obj_surface->width;
        obj_surface->cb_cr_width = obj_surface->orig_width;
        obj_surface->cb_cr_height = obj_surface->orig_height / 2;
        obj_surface->cb_cr_pitch = memory_attibute->pitches[1];

        break;

    case VA_FOURCC_444P:
        ASSERT_RET(memory_attibute->num_planes == 3, VA_STATUS_ERROR_INVALID_PARAMETER);
        ASSERT_RET(memory_attibute->pitches[1] == memory_attibute->pitches[2], VA_STATUS_ERROR_INVALID_PARAMETER);

        obj_surface->subsampling = SUBSAMPLE_YUV444;
        obj_surface->y_cb_offset = obj_surface->height;
        obj_surface->y_cr_offset = memory_attibute->offsets[2] / obj_surface->width;
        obj_surface->cb_cr_width = obj_surface->orig_width;
        obj_surface->cb_cr_height = obj_surface->orig_height;
        obj_surface->cb_cr_pitch = memory_attibute->pitches[1];

        break;

    default:

        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (external_memory_type == I965_SURFACE_MEM_GEM_FLINK)
        obj_surface->bo = drm_intel_bo_gem_create_from_name(i965->intel.bufmgr,
                                                            "gem flinked vaapi surface",
                                                            memory_attibute->buffers[index]);
//    else if (external_memory_type == I965_SURFACE_MEM_DRM_PRIME)
//        obj_surface->bo = drm_intel_bo_gem_create_from_prime(i965->intel.bufmgr,
//                                                             memory_attibute->buffers[index],
//                                                             obj_surface->size);

    if (!obj_surface->bo)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    return VA_STATUS_SUCCESS;
}

/* byte-per-pixel of the first plane */
static int
bpp_1stplane_by_fourcc(unsigned int fourcc)
{
    const i965_fourcc_info *info = get_fourcc_info(fourcc);

    if (info && (info->flag & I_S))
        return info->bpp[0] / 8;
    else
        return 0;
}

static VAStatus
i965_CreateSurfaces2(
    VADriverContextP    ctx,
    unsigned int        format,
    unsigned int        width,
    unsigned int        height,
    VASurfaceID        *surfaces,
    unsigned int        num_surfaces,
    VASurfaceAttrib    *attrib_list,
    unsigned int        num_attribs
    )
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i,j;
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int expected_fourcc = 0;
    int memory_type = I965_SURFACE_MEM_NATIVE; /* native */
    VASurfaceAttribExternalBuffers *memory_attibute = NULL;

    for (i = 0; i < num_attribs && attrib_list; i++) {
        if ((attrib_list[i].type == VASurfaceAttribPixelFormat) &&
            (attrib_list[i].flags & VA_SURFACE_ATTRIB_SETTABLE)) {
            ASSERT_RET(attrib_list[i].value.type == VAGenericValueTypeInteger, VA_STATUS_ERROR_INVALID_PARAMETER);
            expected_fourcc = attrib_list[i].value.value.i;
        }

        if ((attrib_list[i].type == VASurfaceAttribMemoryType) &&
            (attrib_list[i].flags & VA_SURFACE_ATTRIB_SETTABLE)) {

            ASSERT_RET(attrib_list[i].value.type == VAGenericValueTypeInteger, VA_STATUS_ERROR_INVALID_PARAMETER);

            if (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM)
                memory_type = I965_SURFACE_MEM_GEM_FLINK; /* flinked GEM handle */
            else if (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME)
                memory_type = I965_SURFACE_MEM_DRM_PRIME; /* drm prime fd */
            else if (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_VA)
                memory_type = I965_SURFACE_MEM_NATIVE; /* va native memory, to be allocated */
        }

        if ((attrib_list[i].type == VASurfaceAttribExternalBufferDescriptor) &&
            (attrib_list[i].flags == VA_SURFACE_ATTRIB_SETTABLE)) {
            ASSERT_RET(attrib_list[i].value.type == VAGenericValueTypePointer, VA_STATUS_ERROR_INVALID_PARAMETER);
            memory_attibute = (VASurfaceAttribExternalBuffers *)attrib_list[i].value.value.p;
        }
    }

    /* support 420 & 422 & RGB32 format, 422 and RGB32 are only used
     * for post-processing (including color conversion) */
    if (VA_RT_FORMAT_YUV420 != format &&
        VA_RT_FORMAT_YUV420_10BPP != format &&
        VA_RT_FORMAT_YUV422 != format &&
        VA_RT_FORMAT_YUV444 != format &&
        VA_RT_FORMAT_YUV411 != format &&
        VA_RT_FORMAT_YUV400 != format &&
        VA_RT_FORMAT_RGB32  != format) {
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    for (i = 0; i < num_surfaces; i++) {
        int surfaceID = NEW_SURFACE_ID();
        struct object_surface *obj_surface = SURFACE(surfaceID);

        if (NULL == obj_surface) {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            break;
        }

        surfaces[i] = surfaceID;
        obj_surface->status = VASurfaceReady;
        obj_surface->orig_width = width;
        obj_surface->orig_height = height;
        obj_surface->user_disable_tiling = false;
        obj_surface->user_h_stride_set = false;
        obj_surface->user_v_stride_set = false;

        obj_surface->subpic_render_idx = 0;
        for(j = 0; j < I965_MAX_SUBPIC_SUM; j++){
           obj_surface->subpic[j] = VA_INVALID_ID;
           obj_surface->obj_subpic[j] = NULL;
        }

        assert(i965->codec_info->min_linear_wpitch);
        assert(i965->codec_info->min_linear_hpitch);
        obj_surface->width = ALIGN(width, i965->codec_info->min_linear_wpitch);
        obj_surface->height = ALIGN(height, i965->codec_info->min_linear_hpitch);
        obj_surface->flags = SURFACE_REFERENCED;
        obj_surface->fourcc = 0;
        obj_surface->expected_format = format;
        obj_surface->bo = NULL;
        obj_surface->locked_image_id = VA_INVALID_ID;
        obj_surface->derived_image_id = VA_INVALID_ID;
        obj_surface->private_data = NULL;
        obj_surface->free_private_data = NULL;
        obj_surface->subsampling = SUBSAMPLE_YUV420;

        obj_surface->wrapper_surface = VA_INVALID_ID;
        obj_surface->exported_primefd = -1;

        switch (memory_type) {
        case I965_SURFACE_MEM_NATIVE:
            if (memory_attibute) {
                if (!(memory_attibute->flags & VA_SURFACE_EXTBUF_DESC_ENABLE_TILING))
                    obj_surface->user_disable_tiling = true;

                if (memory_attibute->pixel_format) {
                    if (expected_fourcc)
                        ASSERT_RET(memory_attibute->pixel_format == expected_fourcc, VA_STATUS_ERROR_INVALID_PARAMETER);
                    else
                        expected_fourcc = memory_attibute->pixel_format;
                }
                ASSERT_RET(expected_fourcc, VA_STATUS_ERROR_INVALID_PARAMETER);
                if (memory_attibute->pitches[0]) {
                    int bpp_1stplane = bpp_1stplane_by_fourcc(expected_fourcc);
                    ASSERT_RET(bpp_1stplane, VA_STATUS_ERROR_INVALID_PARAMETER);
                    obj_surface->width = memory_attibute->pitches[0];
                    obj_surface->user_h_stride_set = true;
                    ASSERT_RET(IS_ALIGNED(obj_surface->width, 16), VA_STATUS_ERROR_INVALID_PARAMETER);
                    ASSERT_RET(obj_surface->width >= width * bpp_1stplane, VA_STATUS_ERROR_INVALID_PARAMETER);

                    if (memory_attibute->offsets[1]) {
                        ASSERT_RET(!memory_attibute->offsets[0], VA_STATUS_ERROR_INVALID_PARAMETER);
                        obj_surface->height = memory_attibute->offsets[1]/memory_attibute->pitches[0];
                        obj_surface->user_v_stride_set = true;
                        ASSERT_RET(IS_ALIGNED(obj_surface->height, 16), VA_STATUS_ERROR_INVALID_PARAMETER);
                        ASSERT_RET(obj_surface->height >= height, VA_STATUS_ERROR_INVALID_PARAMETER);
                    }
                }
            }
            vaStatus = i965_surface_native_memory(ctx,
                                                  obj_surface,
                                                  format,
                                                  expected_fourcc);
            break;

        case I965_SURFACE_MEM_GEM_FLINK:
        case I965_SURFACE_MEM_DRM_PRIME:
            vaStatus = i965_suface_external_memory(ctx,
                                                   obj_surface,
                                                   memory_type,
                                                   memory_attibute,
                                                   i);
            break;
        }
        if (VA_STATUS_SUCCESS != vaStatus) {
            i965_destroy_surface(&i965->surface_heap, (struct object_base *)obj_surface);
            break;
        }
    }

    /* Error recovery */
    if (VA_STATUS_SUCCESS != vaStatus) {
        /* surfaces[i-1] was the last successful allocation */
        for (; i--; ) {
            struct object_surface *obj_surface = SURFACE(surfaces[i]);

            surfaces[i] = VA_INVALID_SURFACE;
            assert(obj_surface);
            i965_destroy_surface(&i965->surface_heap, (struct object_base *)obj_surface);
        }
    }

    return vaStatus;
}

VAStatus
i965_CreateSurfaces(VADriverContextP ctx,
                    int width,
                    int height,
                    int format,
                    int num_surfaces,
                    VASurfaceID *surfaces)      /* out */
{
    return i965_CreateSurfaces2(ctx,
                                format,
                                width,
                                height,
                                surfaces,
                                num_surfaces,
                                NULL,
                                0);
}

VAStatus
i965_DestroySurfaces(VADriverContextP ctx,
                     VASurfaceID *surface_list,
                     int num_surfaces)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i;
    VAStatus va_status = VA_STATUS_SUCCESS;

    for (i = num_surfaces; i--; ) {
        struct object_surface *obj_surface = SURFACE(surface_list[i]);

        ASSERT_RET(obj_surface, VA_STATUS_ERROR_INVALID_SURFACE);

        if ((obj_surface->wrapper_surface != VA_INVALID_ID) &&
            i965->wrapper_pdrvctx) {
            CALL_VTABLE(i965->wrapper_pdrvctx, va_status,
                        vaDestroySurfaces(i965->wrapper_pdrvctx,
                                          &(obj_surface->wrapper_surface),
                                          1));
            obj_surface->wrapper_surface = VA_INVALID_ID;
        }
        if (obj_surface->exported_primefd >= 0) {
           close(obj_surface->exported_primefd);
           obj_surface->exported_primefd = -1;
        }

        i965_destroy_surface(&i965->surface_heap, (struct object_base *)obj_surface);
    }

    return va_status;
}

VAStatus
i965_QueryImageFormats(VADriverContextP ctx,
                       VAImageFormat *format_list,      /* out */
                       int *num_formats)                /* out */
{
    int n;

    for (n = 0; i965_image_formats_map[n].va_format.fourcc != 0; n++) {
        const i965_image_format_map_t * const m = &i965_image_formats_map[n];
        if (format_list)
            format_list[n] = m->va_format;
    }

    if (num_formats)
        *num_formats = n;

    return VA_STATUS_SUCCESS;
}

/*
 * Guess the format when the usage of a VA surface is unknown
 * 1. Without a valid context: YV12
 * 2. The current context is valid:
 *    a) always NV12 on GEN6 and later
 *    b) I420 for MPEG-2 and NV12 for other codec on GEN4 & GEN5
 */
static void
i965_guess_surface_format(VADriverContextP ctx,
                          VASurfaceID surface,
                          unsigned int *fourcc,
                          unsigned int *is_tiled)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_context *obj_context = NULL;
    struct object_config *obj_config = NULL;

    *fourcc = VA_FOURCC_YV12;
    *is_tiled = 0;

    if (i965->current_context_id == VA_INVALID_ID)
        return;

    obj_context = CONTEXT(i965->current_context_id);

    if (!obj_context)
        return;

    obj_config = obj_context->obj_config;
    assert(obj_config);

    if (!obj_config)
        return;

    if (IS_GEN6(i965->intel.device_info) ||
        IS_GEN7(i965->intel.device_info) ||
        IS_GEN8(i965->intel.device_info) ||
        IS_GEN9(i965->intel.device_info)) {
        *fourcc = VA_FOURCC_NV12;
        *is_tiled = 1;
        return;
    }

    switch (obj_config->profile) {
    case VAProfileMPEG2Simple:
    case VAProfileMPEG2Main:
        *fourcc = VA_FOURCC_I420;
        *is_tiled = 0;
        break;

    default:
        *fourcc = VA_FOURCC_NV12;
        *is_tiled = 0;
        break;
    }
}

VAStatus
i965_QuerySubpictureFormats(VADriverContextP ctx,
                            VAImageFormat *format_list,         /* out */
                            unsigned int *flags,                /* out */
                            unsigned int *num_formats)          /* out */
{
    int n;

    for (n = 0; i965_subpic_formats_map[n].va_format.fourcc != 0; n++) {
        const i965_subpic_format_map_t * const m = &i965_subpic_formats_map[n];
        if (format_list)
            format_list[n] = m->va_format;
        if (flags)
            flags[n] = m->va_flags;
    }

    if (num_formats)
        *num_formats = n;

    return VA_STATUS_SUCCESS;
}

static void
i965_destroy_subpic(struct object_heap *heap, struct object_base *obj)
{
    //    struct object_subpic *obj_subpic = (struct object_subpic *)obj;

    object_heap_free(heap, obj);
}

VAStatus
i965_CreateSubpicture(VADriverContextP ctx,
                      VAImageID image,
                      VASubpictureID *subpicture)         /* out */
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VASubpictureID subpicID = NEW_SUBPIC_ID()
    struct object_subpic *obj_subpic = SUBPIC(subpicID);

    if (!obj_subpic)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    struct object_image *obj_image = IMAGE(image);
    if (!obj_image)
        return VA_STATUS_ERROR_INVALID_IMAGE;

    const i965_subpic_format_map_t * const m = get_subpic_format(&obj_image->image.format);
    if (!m)
        return VA_STATUS_ERROR_UNKNOWN; /* XXX: VA_STATUS_ERROR_UNSUPPORTED_FORMAT? */

    *subpicture = subpicID;
    obj_subpic->image  = image;
    obj_subpic->obj_image = obj_image;
    obj_subpic->format = m->format;
    obj_subpic->width  = obj_image->image.width;
    obj_subpic->height = obj_image->image.height;
    obj_subpic->pitch  = obj_image->image.pitches[0];
    obj_subpic->bo     = obj_image->bo;
    obj_subpic->global_alpha = 1.0;

    return VA_STATUS_SUCCESS;
}

VAStatus
i965_DestroySubpicture(VADriverContextP ctx,
                       VASubpictureID subpicture)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_subpic *obj_subpic = SUBPIC(subpicture);

    if (!obj_subpic)
        return VA_STATUS_ERROR_INVALID_SUBPICTURE;

    ASSERT_RET(obj_subpic->obj_image, VA_STATUS_ERROR_INVALID_SUBPICTURE);
    i965_destroy_subpic(&i965->subpic_heap, (struct object_base *)obj_subpic);
    return VA_STATUS_SUCCESS;
}

VAStatus
i965_SetSubpictureImage(VADriverContextP ctx,
                        VASubpictureID subpicture,
                        VAImageID image)
{
    /* TODO */
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus
i965_SetSubpictureChromakey(VADriverContextP ctx,
                            VASubpictureID subpicture,
                            unsigned int chromakey_min,
                            unsigned int chromakey_max,
                            unsigned int chromakey_mask)
{
    /* TODO */
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus
i965_SetSubpictureGlobalAlpha(VADriverContextP ctx,
                              VASubpictureID subpicture,
                              float global_alpha)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_subpic *obj_subpic = SUBPIC(subpicture);

    if(global_alpha > 1.0 || global_alpha < 0.0){
       return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!obj_subpic)
        return VA_STATUS_ERROR_INVALID_SUBPICTURE;

    obj_subpic->global_alpha  = global_alpha;

    return VA_STATUS_SUCCESS;
}

VAStatus
i965_AssociateSubpicture(VADriverContextP ctx,
                         VASubpictureID subpicture,
                         VASurfaceID *target_surfaces,
                         int num_surfaces,
                         short src_x, /* upper left offset in subpicture */
                         short src_y,
                         unsigned short src_width,
                         unsigned short src_height,
                         short dest_x, /* upper left offset in surface */
                         short dest_y,
                         unsigned short dest_width,
                         unsigned short dest_height,
                         /*
                          * whether to enable chroma-keying or global-alpha
                          * see VA_SUBPICTURE_XXX values
                          */
                         unsigned int flags)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_subpic *obj_subpic = SUBPIC(subpicture);
    int i, j;

    if (!obj_subpic)
        return VA_STATUS_ERROR_INVALID_SUBPICTURE;

    ASSERT_RET(obj_subpic->obj_image, VA_STATUS_ERROR_INVALID_SUBPICTURE);

    obj_subpic->src_rect.x      = src_x;
    obj_subpic->src_rect.y      = src_y;
    obj_subpic->src_rect.width  = src_width;
    obj_subpic->src_rect.height = src_height;
    obj_subpic->dst_rect.x      = dest_x;
    obj_subpic->dst_rect.y      = dest_y;
    obj_subpic->dst_rect.width  = dest_width;
    obj_subpic->dst_rect.height = dest_height;
    obj_subpic->flags           = flags;

    for (i = 0; i < num_surfaces; i++) {
        struct object_surface *obj_surface = SURFACE(target_surfaces[i]);
        if (!obj_surface)
            return VA_STATUS_ERROR_INVALID_SURFACE;

        for(j = 0; j < I965_MAX_SUBPIC_SUM; j ++){
            if(obj_surface->subpic[j] == VA_INVALID_ID){
                assert(obj_surface->obj_subpic[j] == NULL);
                obj_surface->subpic[j] = subpicture;
                obj_surface->obj_subpic[j] = obj_subpic;
                break;
            }
        }

        if(j == I965_MAX_SUBPIC_SUM){
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }

    }
    return VA_STATUS_SUCCESS;
}


VAStatus
i965_DeassociateSubpicture(VADriverContextP ctx,
                           VASubpictureID subpicture,
                           VASurfaceID *target_surfaces,
                           int num_surfaces)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_subpic *obj_subpic = SUBPIC(subpicture);
    int i, j;

    if (!obj_subpic)
        return VA_STATUS_ERROR_INVALID_SUBPICTURE;

    for (i = 0; i < num_surfaces; i++) {
        struct object_surface *obj_surface = SURFACE(target_surfaces[i]);
        if (!obj_surface)
            return VA_STATUS_ERROR_INVALID_SURFACE;

        for(j = 0; j < I965_MAX_SUBPIC_SUM; j ++){
            if (obj_surface->subpic[j] == subpicture) {
                assert(obj_surface->obj_subpic[j] == obj_subpic);
                obj_surface->subpic[j] = VA_INVALID_ID;
                obj_surface->obj_subpic[j] = NULL;
                break;
            }
        }

        if(j == I965_MAX_SUBPIC_SUM){
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }
    }
    return VA_STATUS_SUCCESS;
}

void
i965_reference_buffer_store(struct buffer_store **ptr,
                            struct buffer_store *buffer_store)
{
    assert(*ptr == NULL);

    if (buffer_store) {
        buffer_store->ref_count++;
        *ptr = buffer_store;
    }
}

void
i965_release_buffer_store(struct buffer_store **ptr)
{
    struct buffer_store *buffer_store = *ptr;

    if (buffer_store == NULL)
        return;

    assert(buffer_store->bo || buffer_store->buffer);
    assert(!(buffer_store->bo && buffer_store->buffer));
    buffer_store->ref_count--;

    if (buffer_store->ref_count == 0) {
        dri_bo_unreference(buffer_store->bo);
        free(buffer_store->buffer);
        buffer_store->bo = NULL;
        buffer_store->buffer = NULL;
        free(buffer_store);
    }

    *ptr = NULL;
}

static void
i965_destroy_context(struct object_heap *heap, struct object_base *obj)
{
    struct object_context *obj_context = (struct object_context *)obj;
    int i;

    if (obj_context->hw_context) {
        obj_context->hw_context->destroy(obj_context->hw_context);
        obj_context->hw_context = NULL;
    }

    if (obj_context->codec_type == CODEC_PROC) {
        i965_release_buffer_store(&obj_context->codec_state.proc.pipeline_param);

    } else if (obj_context->codec_type == CODEC_ENC) {
        assert(obj_context->codec_state.encode.num_slice_params <= obj_context->codec_state.encode.max_slice_params);
        i965_release_buffer_store(&obj_context->codec_state.encode.pic_param);
        i965_release_buffer_store(&obj_context->codec_state.encode.seq_param);

        for (i = 0; i < obj_context->codec_state.encode.num_slice_params; i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.slice_params[i]);

        free(obj_context->codec_state.encode.slice_params);

        assert(obj_context->codec_state.encode.num_slice_params_ext <= obj_context->codec_state.encode.max_slice_params_ext);
        i965_release_buffer_store(&obj_context->codec_state.encode.pic_param_ext);
        i965_release_buffer_store(&obj_context->codec_state.encode.seq_param_ext);

        for (i = 0; i < ARRAY_ELEMS(obj_context->codec_state.encode.packed_header_param); i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.packed_header_param[i]);

        for (i = 0; i < ARRAY_ELEMS(obj_context->codec_state.encode.packed_header_data); i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.packed_header_data[i]);

        for (i = 0; i < ARRAY_ELEMS(obj_context->codec_state.encode.misc_param); i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.misc_param[i]);

        for (i = 0; i < obj_context->codec_state.encode.num_slice_params_ext; i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.slice_params_ext[i]);

        free(obj_context->codec_state.encode.slice_params_ext);
        if (obj_context->codec_state.encode.slice_rawdata_index) {
            free(obj_context->codec_state.encode.slice_rawdata_index);
            obj_context->codec_state.encode.slice_rawdata_index = NULL;
        }
        if (obj_context->codec_state.encode.slice_rawdata_count) {
            free(obj_context->codec_state.encode.slice_rawdata_count);
            obj_context->codec_state.encode.slice_rawdata_count = NULL;
        }

        if (obj_context->codec_state.encode.slice_header_index) {
            free(obj_context->codec_state.encode.slice_header_index);
            obj_context->codec_state.encode.slice_header_index = NULL;
        }

        for (i = 0; i < obj_context->codec_state.encode.num_packed_header_params_ext; i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.packed_header_params_ext[i]);
        free(obj_context->codec_state.encode.packed_header_params_ext);

        for (i = 0; i < obj_context->codec_state.encode.num_packed_header_data_ext; i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.packed_header_data_ext[i]);
        free(obj_context->codec_state.encode.packed_header_data_ext);

    } else {
        assert(obj_context->codec_state.decode.num_slice_params <= obj_context->codec_state.decode.max_slice_params);
        assert(obj_context->codec_state.decode.num_slice_datas <= obj_context->codec_state.decode.max_slice_datas);

        i965_release_buffer_store(&obj_context->codec_state.decode.pic_param);
        i965_release_buffer_store(&obj_context->codec_state.decode.iq_matrix);
        i965_release_buffer_store(&obj_context->codec_state.decode.bit_plane);

        for (i = 0; i < obj_context->codec_state.decode.num_slice_params; i++)
            i965_release_buffer_store(&obj_context->codec_state.decode.slice_params[i]);

        for (i = 0; i < obj_context->codec_state.decode.num_slice_datas; i++)
            i965_release_buffer_store(&obj_context->codec_state.decode.slice_datas[i]);

        free(obj_context->codec_state.decode.slice_params);
        free(obj_context->codec_state.decode.slice_datas);
    }

    free(obj_context->render_targets);
    object_heap_free(heap, obj);
}

VAStatus
i965_CreateContext(VADriverContextP ctx,
                   VAConfigID config_id,
                   int picture_width,
                   int picture_height,
                   int flag,
                   VASurfaceID *render_targets,
                   int num_render_targets,
                   VAContextID *context)                /* out */
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_config *obj_config = CONFIG(config_id);
    struct object_context *obj_context = NULL;
    VAConfigAttrib *attrib;
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int contextID;
    int i;

    if (NULL == obj_config) {
        vaStatus = VA_STATUS_ERROR_INVALID_CONFIG;
        return vaStatus;
    }

    if (picture_width > i965->codec_info->max_width ||
        picture_height > i965->codec_info->max_height) {
        vaStatus = VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
        return vaStatus;
    }

    /* Validate flag */
    /* Validate picture dimensions */
    contextID = NEW_CONTEXT_ID();
    obj_context = CONTEXT(contextID);

    if (NULL == obj_context) {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    *context = contextID;
    obj_context->flags = flag;
    obj_context->context_id = contextID;
    obj_context->obj_config = obj_config;
    obj_context->picture_width = picture_width;
    obj_context->picture_height = picture_height;
    obj_context->num_render_targets = num_render_targets;
    obj_context->render_targets =
        (VASurfaceID *)calloc(num_render_targets, sizeof(VASurfaceID));
    obj_context->hw_context = NULL;
    obj_context->wrapper_context = VA_INVALID_ID;

    if (!obj_context->render_targets)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    for(i = 0; i < num_render_targets; i++) {
        if (NULL == SURFACE(render_targets[i])) {
            vaStatus = VA_STATUS_ERROR_INVALID_SURFACE;
            break;
        }

        obj_context->render_targets[i] = render_targets[i];
    }

    if (VA_STATUS_SUCCESS == vaStatus) {
        if (VAEntrypointVideoProc == obj_config->entrypoint) {
            obj_context->codec_type = CODEC_PROC;
            memset(&obj_context->codec_state.proc, 0, sizeof(obj_context->codec_state.proc));
            obj_context->codec_state.proc.current_render_target = VA_INVALID_ID;
            assert(i965->codec_info->proc_hw_context_init);
            obj_context->hw_context = i965->codec_info->proc_hw_context_init(ctx, obj_config);
         } else if ((VAEntrypointEncSlice == obj_config->entrypoint) ||
                   (VAEntrypointEncPicture == obj_config->entrypoint)) { /*encode routine only*/
            VAConfigAttrib *packed_attrib;
            obj_context->codec_type = CODEC_ENC;
            memset(&obj_context->codec_state.encode, 0, sizeof(obj_context->codec_state.encode));
            obj_context->codec_state.encode.current_render_target = VA_INVALID_ID;
            obj_context->codec_state.encode.max_slice_params = NUM_SLICES;
            obj_context->codec_state.encode.slice_params = calloc(obj_context->codec_state.encode.max_slice_params,
                                                               sizeof(*obj_context->codec_state.encode.slice_params));
            obj_context->codec_state.encode.max_packed_header_params_ext = NUM_SLICES;
            obj_context->codec_state.encode.packed_header_params_ext =
                calloc(obj_context->codec_state.encode.max_packed_header_params_ext,
                       sizeof(struct buffer_store *));

            obj_context->codec_state.encode.max_packed_header_data_ext = NUM_SLICES;
            obj_context->codec_state.encode.packed_header_data_ext =
                calloc(obj_context->codec_state.encode.max_packed_header_data_ext,
                       sizeof(struct buffer_store *));

            obj_context->codec_state.encode.max_slice_num = NUM_SLICES;
            obj_context->codec_state.encode.slice_rawdata_index =
                calloc(obj_context->codec_state.encode.max_slice_num, sizeof(int));
            obj_context->codec_state.encode.slice_rawdata_count =
                calloc(obj_context->codec_state.encode.max_slice_num, sizeof(int));

            obj_context->codec_state.encode.slice_header_index =
                calloc(obj_context->codec_state.encode.max_slice_num, sizeof(int));

            obj_context->codec_state.encode.vps_sps_seq_index = 0;

            obj_context->codec_state.encode.slice_index = 0;
            packed_attrib = i965_lookup_config_attribute(obj_config, VAConfigAttribEncPackedHeaders);
            if (packed_attrib)
                obj_context->codec_state.encode.packed_header_flag = packed_attrib->value;
            else {
                /* use the default value. SPS/PPS/RAWDATA is passed from user
                 * while Slice_header data is generated by driver.
                 */
                obj_context->codec_state.encode.packed_header_flag =
                               VA_ENC_PACKED_HEADER_SEQUENCE |
                               VA_ENC_PACKED_HEADER_PICTURE |
                               VA_ENC_PACKED_HEADER_RAW_DATA;
            }
            assert(i965->codec_info->enc_hw_context_init);
            obj_context->hw_context = i965->codec_info->enc_hw_context_init(ctx, obj_config);
        } else {
            obj_context->codec_type = CODEC_DEC;
            memset(&obj_context->codec_state.decode, 0, sizeof(obj_context->codec_state.decode));
            obj_context->codec_state.decode.current_render_target = -1;
            obj_context->codec_state.decode.max_slice_params = NUM_SLICES;
            obj_context->codec_state.decode.max_slice_datas = NUM_SLICES;
            obj_context->codec_state.decode.slice_params = calloc(obj_context->codec_state.decode.max_slice_params,
                                                               sizeof(*obj_context->codec_state.decode.slice_params));
            obj_context->codec_state.decode.slice_datas = calloc(obj_context->codec_state.decode.max_slice_datas,
                                                              sizeof(*obj_context->codec_state.decode.slice_datas));

            assert(i965->codec_info->dec_hw_context_init);
            obj_context->hw_context = i965->codec_info->dec_hw_context_init(ctx, obj_config);
        }
    }

    attrib = i965_lookup_config_attribute(obj_config, VAConfigAttribRTFormat);
    if (!attrib)
        return VA_STATUS_ERROR_INVALID_CONFIG;
    obj_context->codec_state.base.chroma_formats = attrib->value;

    if (obj_config->wrapper_config != VA_INVALID_ID) {
        /* The wrapper_pdrvctx should exist when wrapper_config is valid.
         * So it won't check i965->wrapper_pdrvctx again.
         * Fixme if it is incorrect.
         */
        VAGenericID wrapper_context;

        /*
         * The render_surface is not passed when calling
         * vaCreateContext.
         * If it is needed, we must get the wrapped surface
         * for the corresponding Surface_list.
         * So the wrapped surface conversion is deferred.
         */
        CALL_VTABLE(i965->wrapper_pdrvctx, vaStatus,
                    vaCreateContext(i965->wrapper_pdrvctx,
                                    obj_config->wrapper_config,
                                    picture_width, picture_height,
                                    flag, NULL, 0,
                                    &wrapper_context));

        if (vaStatus == VA_STATUS_SUCCESS)
            obj_context->wrapper_context = wrapper_context;
    }
    /* Error recovery */
    if (VA_STATUS_SUCCESS != vaStatus) {
        i965_destroy_context(&i965->context_heap, (struct object_base *)obj_context);
    }

    i965->current_context_id = contextID;

    return vaStatus;
}

VAStatus
i965_DestroyContext(VADriverContextP ctx, VAContextID context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_context *obj_context = CONTEXT(context);
    VAStatus va_status = VA_STATUS_SUCCESS;

    ASSERT_RET(obj_context, VA_STATUS_ERROR_INVALID_CONTEXT);

    if (i965->current_context_id == context)
        i965->current_context_id = VA_INVALID_ID;

    if ((obj_context->wrapper_context != VA_INVALID_ID) &&
        i965->wrapper_pdrvctx) {
        CALL_VTABLE(i965->wrapper_pdrvctx, va_status,
                    vaDestroyContext(i965->wrapper_pdrvctx,
                                     obj_context->wrapper_context));

        obj_context->wrapper_context = VA_INVALID_ID;
    }

    i965_destroy_context(&i965->context_heap, (struct object_base *)obj_context);

    return va_status;
}

static void
i965_destroy_buffer(struct object_heap *heap, struct object_base *obj)
{
    struct object_buffer *obj_buffer = (struct object_buffer *)obj;

    assert(obj_buffer->buffer_store);
    i965_release_buffer_store(&obj_buffer->buffer_store);
    object_heap_free(heap, obj);
}

static VAStatus
i965_create_buffer_internal(VADriverContextP ctx,
                            VAContextID context,
                            VABufferType type,
                            unsigned int size,
                            unsigned int num_elements,
                            void *data,
                            dri_bo *store_bo,
                            VABufferID *buf_id)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_buffer *obj_buffer = NULL;
    struct buffer_store *buffer_store = NULL;
    int bufferID;
    VAStatus vaStatus = VA_STATUS_ERROR_UNKNOWN;
    struct object_context *obj_context = CONTEXT(context);
    int wrapper_flag = 0;

    /* Validate type */
    switch (type) {
    case VAPictureParameterBufferType:
    case VAIQMatrixBufferType:
    case VAQMatrixBufferType:
    case VABitPlaneBufferType:
    case VASliceGroupMapBufferType:
    case VASliceParameterBufferType:
    case VASliceDataBufferType:
    case VAMacroblockParameterBufferType:
    case VAResidualDataBufferType:
    case VADeblockingParameterBufferType:
    case VAImageBufferType:
    case VAEncCodedBufferType:
    case VAEncSequenceParameterBufferType:
    case VAEncPictureParameterBufferType:
    case VAEncSliceParameterBufferType:
    case VAEncPackedHeaderParameterBufferType:
    case VAEncPackedHeaderDataBufferType:
    case VAEncMiscParameterBufferType:
    case VAProcPipelineParameterBufferType:
    case VAProcFilterParameterBufferType:
    case VAHuffmanTableBufferType:
    case VAProbabilityBufferType:
        /* Ok */
        break;

    default:
        return VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
    }

    bufferID = NEW_BUFFER_ID();
    obj_buffer = BUFFER(bufferID);

    if (NULL == obj_buffer) {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (type == VAEncCodedBufferType) {
        size += I965_CODEDBUFFER_HEADER_SIZE;
        size += 0x1000; /* for upper bound check */
    }

    obj_buffer->max_num_elements = num_elements;
    obj_buffer->num_elements = num_elements;
    obj_buffer->size_element = size;
    obj_buffer->type = type;
    obj_buffer->export_refcount = 0;
    obj_buffer->buffer_store = NULL;
    obj_buffer->wrapper_buffer = VA_INVALID_ID;

    buffer_store = calloc(1, sizeof(struct buffer_store));
    assert(buffer_store);
    buffer_store->ref_count = 1;

    if (obj_context &&
        (obj_context->wrapper_context != VA_INVALID_ID) &&
        i965->wrapper_pdrvctx) {
        VAGenericID wrapper_buffer;
        VADriverContextP pdrvctx = i965->wrapper_pdrvctx;

        CALL_VTABLE(pdrvctx, vaStatus,
                    vaCreateBuffer(pdrvctx, obj_context->wrapper_context, type, size, num_elements,
                                   data, &wrapper_buffer));
        if (vaStatus == VA_STATUS_SUCCESS) {
            obj_buffer->wrapper_buffer = wrapper_buffer;
        } else {
            free(buffer_store);
            return vaStatus;
        }
        wrapper_flag = 1;
    }

    if (store_bo != NULL) {
        buffer_store->bo = store_bo;
        dri_bo_reference(buffer_store->bo);

        /* If the buffer is wrapped, the buffer_store is bogus. Unnecessary to copy it */
        if (data && !wrapper_flag)
            dri_bo_subdata(buffer_store->bo, 0, size * num_elements, data);
    } else if (type == VASliceDataBufferType ||
               type == VAImageBufferType ||
               type == VAEncCodedBufferType ||
               type == VAProbabilityBufferType) {

        /* If the buffer is wrapped, the bo/buffer of buffer_store is bogus.
         * So it is enough to allocate one 64 byte bo
         */
        if (wrapper_flag)
            buffer_store->bo = dri_bo_alloc(i965->intel.bufmgr, "Bogus buffer",
                                            64, 64);
        else
            buffer_store->bo = dri_bo_alloc(i965->intel.bufmgr,
                                            "Buffer",
                                            size * num_elements, 64);
        assert(buffer_store->bo);

        /* If the buffer is wrapped, the bo/buffer of buffer_store is bogus.
         * In fact it can be skipped. But it is still allocated and it is
         * only to follow the normal flowchart of buffer_allocation/release.
         */
        if (!wrapper_flag) {
          if (type == VAEncCodedBufferType) {
            struct i965_coded_buffer_segment *coded_buffer_segment;

            dri_bo_map(buffer_store->bo, 1);
            coded_buffer_segment = (struct i965_coded_buffer_segment *)buffer_store->bo->virtual;
            coded_buffer_segment->base.size = size - I965_CODEDBUFFER_HEADER_SIZE;
            coded_buffer_segment->base.bit_offset = 0;
            coded_buffer_segment->base.status = 0;
            coded_buffer_segment->base.buf = NULL;
            coded_buffer_segment->base.next = NULL;
            coded_buffer_segment->mapped = 0;
            coded_buffer_segment->codec = 0;
            dri_bo_unmap(buffer_store->bo);
          } else if (data) {
              dri_bo_subdata(buffer_store->bo, 0, size * num_elements, data);
          }
       }

    } else {
        int msize = size;

        if (type == VAEncPackedHeaderDataBufferType) {
            msize = ALIGN(size, 4);
        }

        /* If the buffer is wrapped, it is enough to allocate 4 bytes */
        if (wrapper_flag)
            buffer_store->buffer = malloc(4);
        else
            buffer_store->buffer = malloc(msize * num_elements);
        assert(buffer_store->buffer);

        if (data && (!wrapper_flag))
            memcpy(buffer_store->buffer, data, size * num_elements);
    }

    buffer_store->num_elements = obj_buffer->num_elements;
    i965_reference_buffer_store(&obj_buffer->buffer_store, buffer_store);
    i965_release_buffer_store(&buffer_store);
    *buf_id = bufferID;

    return VA_STATUS_SUCCESS;
}

VAStatus
i965_CreateBuffer(VADriverContextP ctx,
                  VAContextID context,          /* in */
                  VABufferType type,            /* in */
                  unsigned int size,            /* in */
                  unsigned int num_elements,    /* in */
                  void *data,                   /* in */
                  VABufferID *buf_id)           /* out */
{
    return i965_create_buffer_internal(ctx, context, type, size, num_elements, data, NULL, buf_id);
}


VAStatus
i965_BufferSetNumElements(VADriverContextP ctx,
                          VABufferID buf_id,           /* in */
                          unsigned int num_elements)   /* in */
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_buffer *obj_buffer = BUFFER(buf_id);
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    ASSERT_RET(obj_buffer, VA_STATUS_ERROR_INVALID_BUFFER);

    /* When the wrapper_buffer exists, it will wrapper to the
     * buffer allocated from backend driver.
     */
    if ((obj_buffer->wrapper_buffer != VA_INVALID_ID) &&
        i965->wrapper_pdrvctx) {
        VADriverContextP pdrvctx = i965->wrapper_pdrvctx;

        CALL_VTABLE(pdrvctx, vaStatus,
                    vaBufferSetNumElements(pdrvctx, obj_buffer->wrapper_buffer,
                                         num_elements));
        return vaStatus;
    }

    if ((num_elements < 0) ||
        (num_elements > obj_buffer->max_num_elements)) {
        vaStatus = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    } else {
        obj_buffer->num_elements = num_elements;
        if (obj_buffer->buffer_store != NULL) {
            obj_buffer->buffer_store->num_elements = num_elements;
        }
    }

    return vaStatus;
}

VAStatus
i965_MapBuffer(VADriverContextP ctx,
               VABufferID buf_id,       /* in */
               void **pbuf)             /* out */
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_buffer *obj_buffer = BUFFER(buf_id);
    VAStatus vaStatus = VA_STATUS_ERROR_UNKNOWN;

    ASSERT_RET(obj_buffer && obj_buffer->buffer_store, VA_STATUS_ERROR_INVALID_BUFFER);

    /* When the wrapper_buffer exists, it will wrapper to the
     * buffer allocated from backend driver.
     */
    if ((obj_buffer->wrapper_buffer != VA_INVALID_ID) &&
        i965->wrapper_pdrvctx) {
        VADriverContextP pdrvctx = i965->wrapper_pdrvctx;

        CALL_VTABLE(pdrvctx, vaStatus,
                    vaMapBuffer(pdrvctx, obj_buffer->wrapper_buffer, pbuf));
        return vaStatus;
    }

    ASSERT_RET(obj_buffer->buffer_store->bo || obj_buffer->buffer_store->buffer, VA_STATUS_ERROR_INVALID_BUFFER);
    ASSERT_RET(!(obj_buffer->buffer_store->bo && obj_buffer->buffer_store->buffer), VA_STATUS_ERROR_INVALID_BUFFER);

    if (obj_buffer->export_refcount > 0)
        return VA_STATUS_ERROR_INVALID_BUFFER;

    if (NULL != obj_buffer->buffer_store->bo) {
        unsigned int tiling, swizzle;

        dri_bo_get_tiling(obj_buffer->buffer_store->bo, &tiling, &swizzle);

        if (tiling != I915_TILING_NONE)
            drm_intel_gem_bo_map_gtt(obj_buffer->buffer_store->bo);
        else
            dri_bo_map(obj_buffer->buffer_store->bo, 1);

        ASSERT_RET(obj_buffer->buffer_store->bo->virtual, VA_STATUS_ERROR_OPERATION_FAILED);
        *pbuf = obj_buffer->buffer_store->bo->virtual;

        if (obj_buffer->type == VAEncCodedBufferType) {
            int i;
            unsigned char *buffer = NULL;
            unsigned int  header_offset = I965_CODEDBUFFER_HEADER_SIZE;
            struct i965_coded_buffer_segment *coded_buffer_segment = (struct i965_coded_buffer_segment *)(obj_buffer->buffer_store->bo->virtual);

            if (!coded_buffer_segment->mapped) {
                unsigned char delimiter0, delimiter1, delimiter2, delimiter3, delimiter4;

                coded_buffer_segment->base.buf = buffer = (unsigned char *)(obj_buffer->buffer_store->bo->virtual) + I965_CODEDBUFFER_HEADER_SIZE;

                if (coded_buffer_segment->codec == CODEC_H264 ||
                    coded_buffer_segment->codec == CODEC_H264_MVC) {
                    delimiter0 = H264_DELIMITER0;
                    delimiter1 = H264_DELIMITER1;
                    delimiter2 = H264_DELIMITER2;
                    delimiter3 = H264_DELIMITER3;
                    delimiter4 = H264_DELIMITER4;
                } else if (coded_buffer_segment->codec == CODEC_MPEG2) {
                    delimiter0 = MPEG2_DELIMITER0;
                    delimiter1 = MPEG2_DELIMITER1;
                    delimiter2 = MPEG2_DELIMITER2;
                    delimiter3 = MPEG2_DELIMITER3;
                    delimiter4 = MPEG2_DELIMITER4;
                } else if(coded_buffer_segment->codec == CODEC_JPEG) {
                    //In JPEG End of Image (EOI = 0xDDF9) marker can be used for delimiter.
                    delimiter0 = 0xFF;
                    delimiter1 = 0xD9;
                } else if (coded_buffer_segment->codec == CODEC_HEVC) {
                    delimiter0 = HEVC_DELIMITER0;
                    delimiter1 = HEVC_DELIMITER1;
                    delimiter2 = HEVC_DELIMITER2;
                    delimiter3 = HEVC_DELIMITER3;
                    delimiter4 = HEVC_DELIMITER4;
                } else if (coded_buffer_segment->codec != CODEC_VP8) {
                    ASSERT_RET(0, VA_STATUS_ERROR_UNSUPPORTED_PROFILE);
                }

                if(coded_buffer_segment->codec == CODEC_JPEG) {
                    for(i = 0; i <  obj_buffer->size_element - header_offset - 1 - 0x1000; i++) {
                        if( (buffer[i] == 0xFF) && (buffer[i + 1] == 0xD9)) {
                            break;
                        }
                   }
                   coded_buffer_segment->base.size = i + 2;
                } else if (coded_buffer_segment->codec != CODEC_VP8) {
                    /* vp8 coded buffer size can be told by vp8 internal statistics buffer,
                       so it don't need to traversal the coded buffer */
                    for (i = 0; i < obj_buffer->size_element - header_offset - 3 - 0x1000; i++) {
                        if ((buffer[i] == delimiter0) &&
                            (buffer[i + 1] == delimiter1) &&
                            (buffer[i + 2] == delimiter2) &&
                            (buffer[i + 3] == delimiter3) &&
                            (buffer[i + 4] == delimiter4))
                            break;
                    }

                    if (i == obj_buffer->size_element - header_offset - 3 - 0x1000) {
                        coded_buffer_segment->base.status |= VA_CODED_BUF_STATUS_SLICE_OVERFLOW_MASK;
                    }
                    coded_buffer_segment->base.size = i;
                }

                if (coded_buffer_segment->base.size >= obj_buffer->size_element - header_offset - 0x1000) {
                    coded_buffer_segment->base.status |= VA_CODED_BUF_STATUS_SLICE_OVERFLOW_MASK;
                }

                coded_buffer_segment->mapped = 1;
            } else {
                assert(coded_buffer_segment->base.buf);
            }
        }

        vaStatus = VA_STATUS_SUCCESS;
    } else if (NULL != obj_buffer->buffer_store->buffer) {
        *pbuf = obj_buffer->buffer_store->buffer;
        vaStatus = VA_STATUS_SUCCESS;
    }

    return vaStatus;
}

VAStatus
i965_UnmapBuffer(VADriverContextP ctx, VABufferID buf_id)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_buffer *obj_buffer = BUFFER(buf_id);
    VAStatus vaStatus = VA_STATUS_ERROR_UNKNOWN;

    if ((buf_id & OBJECT_HEAP_OFFSET_MASK) != BUFFER_ID_OFFSET)
        return VA_STATUS_ERROR_INVALID_BUFFER;

    ASSERT_RET(obj_buffer && obj_buffer->buffer_store, VA_STATUS_ERROR_INVALID_BUFFER);
    /* When the wrapper_buffer exists, it will wrapper to the
     * buffer allocated from backend driver.
     */
    if ((obj_buffer->wrapper_buffer != VA_INVALID_ID) &&
        i965->wrapper_pdrvctx) {
        VADriverContextP pdrvctx = i965->wrapper_pdrvctx;

        CALL_VTABLE(pdrvctx, vaStatus,
                    vaUnmapBuffer(pdrvctx, obj_buffer->wrapper_buffer));
        return vaStatus;
    }

    ASSERT_RET(obj_buffer->buffer_store->bo || obj_buffer->buffer_store->buffer, VA_STATUS_ERROR_OPERATION_FAILED);
    ASSERT_RET(!(obj_buffer->buffer_store->bo && obj_buffer->buffer_store->buffer), VA_STATUS_ERROR_OPERATION_FAILED);

    if (NULL != obj_buffer->buffer_store->bo) {
        unsigned int tiling, swizzle;

        dri_bo_get_tiling(obj_buffer->buffer_store->bo, &tiling, &swizzle);

        if (tiling != I915_TILING_NONE)
            drm_intel_gem_bo_unmap_gtt(obj_buffer->buffer_store->bo);
        else
            dri_bo_unmap(obj_buffer->buffer_store->bo);

        vaStatus = VA_STATUS_SUCCESS;
    } else if (NULL != obj_buffer->buffer_store->buffer) {
        /* Do nothing */
        vaStatus = VA_STATUS_SUCCESS;
    }

    return vaStatus;
}

VAStatus
i965_DestroyBuffer(VADriverContextP ctx, VABufferID buffer_id)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_buffer *obj_buffer = BUFFER(buffer_id);
    VAStatus va_status = VA_STATUS_SUCCESS;

    ASSERT_RET(obj_buffer, VA_STATUS_ERROR_INVALID_BUFFER);

    if ((obj_buffer->wrapper_buffer != VA_INVALID_ID) &&
        i965->wrapper_pdrvctx) {
        CALL_VTABLE(i965->wrapper_pdrvctx, va_status,
                    vaDestroyBuffer(i965->wrapper_pdrvctx,
                                    obj_buffer->wrapper_buffer));
        obj_buffer->wrapper_buffer = VA_INVALID_ID;
    }

    i965_destroy_buffer(&i965->buffer_heap, (struct object_base *)obj_buffer);

    return va_status;
}

VAStatus
i965_BeginPicture(VADriverContextP ctx,
                  VAContextID context,
                  VASurfaceID render_target)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_context *obj_context = CONTEXT(context);
    struct object_surface *obj_surface = SURFACE(render_target);
    struct object_config *obj_config;
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int i;

    ASSERT_RET(obj_context, VA_STATUS_ERROR_INVALID_CONTEXT);
    ASSERT_RET(obj_surface, VA_STATUS_ERROR_INVALID_SURFACE);
    obj_config = obj_context->obj_config;
    ASSERT_RET(obj_config, VA_STATUS_ERROR_INVALID_CONFIG);

    if (is_surface_busy(i965, obj_surface))
        return VA_STATUS_ERROR_SURFACE_BUSY;

    if (obj_context->codec_type == CODEC_PROC) {
        obj_context->codec_state.proc.current_render_target = render_target;
    } else if (obj_context->codec_type == CODEC_ENC) {
        i965_release_buffer_store(&obj_context->codec_state.encode.pic_param);

        for (i = 0; i < obj_context->codec_state.encode.num_slice_params; i++) {
            i965_release_buffer_store(&obj_context->codec_state.encode.slice_params[i]);
        }

        obj_context->codec_state.encode.num_slice_params = 0;

        /* ext */
        i965_release_buffer_store(&obj_context->codec_state.encode.pic_param_ext);

        for (i = 0; i < ARRAY_ELEMS(obj_context->codec_state.encode.packed_header_param); i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.packed_header_param[i]);

        for (i = 0; i < ARRAY_ELEMS(obj_context->codec_state.encode.packed_header_data); i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.packed_header_data[i]);

        for (i = 0; i < obj_context->codec_state.encode.num_slice_params_ext; i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.slice_params_ext[i]);

        obj_context->codec_state.encode.num_slice_params_ext = 0;
        obj_context->codec_state.encode.current_render_target = render_target;     /*This is input new frame*/
        obj_context->codec_state.encode.last_packed_header_type = 0;
        memset(obj_context->codec_state.encode.slice_rawdata_index, 0,
               sizeof(int) * obj_context->codec_state.encode.max_slice_num);
        memset(obj_context->codec_state.encode.slice_rawdata_count, 0,
               sizeof(int) * obj_context->codec_state.encode.max_slice_num);
        memset(obj_context->codec_state.encode.slice_header_index, 0,
               sizeof(int) * obj_context->codec_state.encode.max_slice_num);

        for (i = 0; i < obj_context->codec_state.encode.num_packed_header_params_ext; i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.packed_header_params_ext[i]);
        for (i = 0; i < obj_context->codec_state.encode.num_packed_header_data_ext; i++)
            i965_release_buffer_store(&obj_context->codec_state.encode.packed_header_data_ext[i]);
        obj_context->codec_state.encode.num_packed_header_params_ext = 0;
        obj_context->codec_state.encode.num_packed_header_data_ext = 0;
        obj_context->codec_state.encode.slice_index = 0;
        obj_context->codec_state.encode.vps_sps_seq_index = 0;
    } else {
        obj_context->codec_state.decode.current_render_target = render_target;
        i965_release_buffer_store(&obj_context->codec_state.decode.pic_param);
        i965_release_buffer_store(&obj_context->codec_state.decode.iq_matrix);
        i965_release_buffer_store(&obj_context->codec_state.decode.bit_plane);
        i965_release_buffer_store(&obj_context->codec_state.decode.huffman_table);

        for (i = 0; i < obj_context->codec_state.decode.num_slice_params; i++) {
            i965_release_buffer_store(&obj_context->codec_state.decode.slice_params[i]);
            i965_release_buffer_store(&obj_context->codec_state.decode.slice_datas[i]);
        }

        obj_context->codec_state.decode.num_slice_params = 0;
        obj_context->codec_state.decode.num_slice_datas = 0;

        if ((obj_context->wrapper_context != VA_INVALID_ID) &&
            i965->wrapper_pdrvctx) {
            if (obj_surface->wrapper_surface == VA_INVALID_ID)
                vaStatus = i965_surface_wrapper(ctx, render_target);

            if (vaStatus != VA_STATUS_SUCCESS)
                return vaStatus;

            CALL_VTABLE(i965->wrapper_pdrvctx, vaStatus,
                        vaBeginPicture(i965->wrapper_pdrvctx,
                                       obj_context->wrapper_context,
                                       obj_surface->wrapper_surface));
        }
    }

    return vaStatus;
}

#define I965_RENDER_BUFFER(category, name) i965_render_##category##_##name##_buffer(ctx, obj_context, obj_buffer)

#define DEF_RENDER_SINGLE_BUFFER_FUNC(category, name, member)           \
    static VAStatus                                                     \
    i965_render_##category##_##name##_buffer(VADriverContextP ctx,      \
                                             struct object_context *obj_context, \
                                             struct object_buffer *obj_buffer) \
    {                                                                   \
        struct category##_state *category = &obj_context->codec_state.category; \
        i965_release_buffer_store(&category->member);                   \
        i965_reference_buffer_store(&category->member, obj_buffer->buffer_store); \
        return VA_STATUS_SUCCESS;                                       \
    }

#define DEF_RENDER_MULTI_BUFFER_FUNC(category, name, member)            \
    static VAStatus                                                     \
    i965_render_##category##_##name##_buffer(VADriverContextP ctx,      \
                                             struct object_context *obj_context, \
                                             struct object_buffer *obj_buffer) \
    {                                                                   \
        struct category##_state *category = &obj_context->codec_state.category; \
        if (category->num_##member == category->max_##member) {         \
            category->member = realloc(category->member, (category->max_##member + NUM_SLICES) * sizeof(*category->member)); \
            memset(category->member + category->max_##member, 0, NUM_SLICES * sizeof(*category->member)); \
            category->max_##member += NUM_SLICES;                       \
        }                                                               \
        i965_release_buffer_store(&category->member[category->num_##member]); \
        i965_reference_buffer_store(&category->member[category->num_##member], obj_buffer->buffer_store); \
        category->num_##member++;                                       \
        return VA_STATUS_SUCCESS;                                       \
    }

#define I965_RENDER_DECODE_BUFFER(name) I965_RENDER_BUFFER(decode, name)

#define DEF_RENDER_DECODE_SINGLE_BUFFER_FUNC(name, member) DEF_RENDER_SINGLE_BUFFER_FUNC(decode, name, member)
DEF_RENDER_DECODE_SINGLE_BUFFER_FUNC(picture_parameter, pic_param)
DEF_RENDER_DECODE_SINGLE_BUFFER_FUNC(iq_matrix, iq_matrix)
DEF_RENDER_DECODE_SINGLE_BUFFER_FUNC(bit_plane, bit_plane)
DEF_RENDER_DECODE_SINGLE_BUFFER_FUNC(huffman_table, huffman_table)
DEF_RENDER_DECODE_SINGLE_BUFFER_FUNC(probability_data, probability_data)

#define DEF_RENDER_DECODE_MULTI_BUFFER_FUNC(name, member) DEF_RENDER_MULTI_BUFFER_FUNC(decode, name, member)
DEF_RENDER_DECODE_MULTI_BUFFER_FUNC(slice_parameter, slice_params)
DEF_RENDER_DECODE_MULTI_BUFFER_FUNC(slice_data, slice_datas)


static VAStatus
i965_decoder_vp9_wrapper_picture(VADriverContextP ctx,
                             VABufferID *buffers,
                             int num_buffers)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int i;
    VADecPictureParameterBufferVP9 *pVp9PicParams;
    VADriverContextP pdrvctx;
    struct object_buffer *obj_buffer;

    pdrvctx = i965->wrapper_pdrvctx;
    /* do the conversion of VADecPictureParameterBufferVP9 */
    for (i = 0; i < num_buffers; i++) {
        obj_buffer = BUFFER(buffers[i]);

        if (!obj_buffer)
            continue;

        if (obj_buffer->wrapper_buffer == VA_INVALID_ID)
            continue;

        if (obj_buffer->type == VAPictureParameterBufferType) {
            int j;
            VASurfaceID surface_id;
            struct object_surface *obj_surface;

            pdrvctx = i965->wrapper_pdrvctx;

            CALL_VTABLE(pdrvctx, vaStatus,
                        vaMapBuffer(pdrvctx, obj_buffer->wrapper_buffer,
                                    (void **)(&pVp9PicParams)));

            if (vaStatus != VA_STATUS_SUCCESS)
                return vaStatus;

            for (j = 0; j < 8; j++) {
                surface_id = pVp9PicParams->reference_frames[j];
                obj_surface = SURFACE(surface_id);

                if (!obj_surface)
                    continue;

                if (obj_surface->wrapper_surface == VA_INVALID_ID) {
                    vaStatus = i965_surface_wrapper(ctx, surface_id);
                    if (vaStatus != VA_STATUS_SUCCESS) {
                        pdrvctx->vtable->vaUnmapBuffer(pdrvctx,
                                    obj_buffer->wrapper_buffer);
                        goto fail_out;
                    }
                }

                pVp9PicParams->reference_frames[j] = obj_surface->wrapper_surface;
            }
            CALL_VTABLE(pdrvctx, vaStatus,
                        vaUnmapBuffer(pdrvctx, obj_buffer->wrapper_buffer));
            break;
        }
    }

    return VA_STATUS_SUCCESS;

fail_out:
    return vaStatus;
}

static VAStatus
i965_decoder_wrapper_picture(VADriverContextP ctx,
                             VAContextID context,
                             VABufferID *buffers,
                             int num_buffers)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_context *obj_context = CONTEXT(context);
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int i;
    VADriverContextP pdrvctx;
    struct object_buffer *obj_buffer;

    if (obj_context == NULL)
        return VA_STATUS_ERROR_INVALID_CONTEXT;

    /* When it is not wrapped context, continue the normal flowchart */
    if (obj_context->wrapper_context == VA_INVALID_ID)
        return vaStatus;

    if (obj_context->obj_config &&
        (obj_context->obj_config->profile == VAProfileVP9Profile0)) {
        vaStatus = i965_decoder_vp9_wrapper_picture(ctx, buffers, num_buffers);
    } else
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    pdrvctx = i965->wrapper_pdrvctx;

    for (i = 0; i < num_buffers && vaStatus == VA_STATUS_SUCCESS; i++) {
        obj_buffer = BUFFER(buffers[i]);

        if (!obj_buffer)
            continue;

        if (obj_buffer->wrapper_buffer == VA_INVALID_ID) {
            vaStatus = VA_STATUS_ERROR_INVALID_BUFFER;
            break;
        }

        CALL_VTABLE(pdrvctx, vaStatus,
                    vaRenderPicture(pdrvctx, obj_context->wrapper_context,
                                    &(obj_buffer->wrapper_buffer), 1));
    }
    return vaStatus;
}

static VAStatus
i965_decoder_render_picture(VADriverContextP ctx,
                            VAContextID context,
                            VABufferID *buffers,
                            int num_buffers)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_context *obj_context = CONTEXT(context);
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int i;

    ASSERT_RET(obj_context, VA_STATUS_ERROR_INVALID_CONTEXT);

    for (i = 0; i < num_buffers && vaStatus == VA_STATUS_SUCCESS; i++) {
        struct object_buffer *obj_buffer = BUFFER(buffers[i]);

        if (!obj_buffer)
            return VA_STATUS_ERROR_INVALID_BUFFER;

        switch (obj_buffer->type) {
        case VAPictureParameterBufferType:
            vaStatus = I965_RENDER_DECODE_BUFFER(picture_parameter);
            break;

        case VAIQMatrixBufferType:
            vaStatus = I965_RENDER_DECODE_BUFFER(iq_matrix);
            break;

        case VABitPlaneBufferType:
            vaStatus = I965_RENDER_DECODE_BUFFER(bit_plane);
            break;

        case VASliceParameterBufferType:
            vaStatus = I965_RENDER_DECODE_BUFFER(slice_parameter);
            break;

        case VASliceDataBufferType:
            vaStatus = I965_RENDER_DECODE_BUFFER(slice_data);
            break;

        case VAHuffmanTableBufferType:
            vaStatus = I965_RENDER_DECODE_BUFFER(huffman_table);
            break;

        case VAProbabilityBufferType:
            vaStatus = I965_RENDER_DECODE_BUFFER(probability_data);
            break;

        default:
            vaStatus = VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
            break;
        }
    }

    if ((vaStatus == VA_STATUS_SUCCESS) &&
        (obj_context->wrapper_context != VA_INVALID_ID))
        vaStatus = i965_decoder_wrapper_picture(ctx, context, buffers, num_buffers);

    return vaStatus;
}

#define I965_RENDER_ENCODE_BUFFER(name) I965_RENDER_BUFFER(encode, name)

#define DEF_RENDER_ENCODE_SINGLE_BUFFER_FUNC(name, member) DEF_RENDER_SINGLE_BUFFER_FUNC(encode, name, member)
// DEF_RENDER_ENCODE_SINGLE_BUFFER_FUNC(sequence_parameter, seq_param)
// DEF_RENDER_ENCODE_SINGLE_BUFFER_FUNC(picture_parameter, pic_param)
// DEF_RENDER_ENCODE_SINGLE_BUFFER_FUNC(picture_control, pic_control)
DEF_RENDER_ENCODE_SINGLE_BUFFER_FUNC(qmatrix, q_matrix)
DEF_RENDER_ENCODE_SINGLE_BUFFER_FUNC(iqmatrix, iq_matrix)
DEF_RENDER_ENCODE_SINGLE_BUFFER_FUNC(huffman_table, huffman_table)
/* extended buffer */
DEF_RENDER_ENCODE_SINGLE_BUFFER_FUNC(sequence_parameter_ext, seq_param_ext)
DEF_RENDER_ENCODE_SINGLE_BUFFER_FUNC(picture_parameter_ext, pic_param_ext)

#define DEF_RENDER_ENCODE_MULTI_BUFFER_FUNC(name, member) DEF_RENDER_MULTI_BUFFER_FUNC(encode, name, member)
// DEF_RENDER_ENCODE_MULTI_BUFFER_FUNC(slice_parameter, slice_params)
DEF_RENDER_ENCODE_MULTI_BUFFER_FUNC(slice_parameter_ext, slice_params_ext)

DEF_RENDER_ENCODE_MULTI_BUFFER_FUNC(packed_header_params_ext, packed_header_params_ext)
DEF_RENDER_ENCODE_MULTI_BUFFER_FUNC(packed_header_data_ext, packed_header_data_ext)

static VAStatus
i965_encoder_render_packed_header_parameter_buffer(VADriverContextP ctx,
                                                   struct object_context *obj_context,
                                                   struct object_buffer *obj_buffer,
                                                   int type_index)
{
    struct encode_state *encode = &obj_context->codec_state.encode;

    ASSERT_RET(obj_buffer->buffer_store->bo == NULL, VA_STATUS_ERROR_INVALID_BUFFER);
    ASSERT_RET(obj_buffer->buffer_store->buffer, VA_STATUS_ERROR_INVALID_BUFFER);
    i965_release_buffer_store(&encode->packed_header_param[type_index]);
    i965_reference_buffer_store(&encode->packed_header_param[type_index], obj_buffer->buffer_store);

    return VA_STATUS_SUCCESS;
}

static VAStatus
i965_encoder_render_packed_header_data_buffer(VADriverContextP ctx,
                                              struct object_context *obj_context,
                                              struct object_buffer *obj_buffer,
                                              int type_index)
{
    struct encode_state *encode = &obj_context->codec_state.encode;

    ASSERT_RET(obj_buffer->buffer_store->bo == NULL, VA_STATUS_ERROR_INVALID_BUFFER);
    ASSERT_RET(obj_buffer->buffer_store->buffer, VA_STATUS_ERROR_INVALID_BUFFER);
    i965_release_buffer_store(&encode->packed_header_data[type_index]);
    i965_reference_buffer_store(&encode->packed_header_data[type_index], obj_buffer->buffer_store);

    return VA_STATUS_SUCCESS;
}

static VAStatus
i965_encoder_render_misc_parameter_buffer(VADriverContextP ctx,
                                          struct object_context *obj_context,
                                          struct object_buffer *obj_buffer)
{
    struct encode_state *encode = &obj_context->codec_state.encode;
    VAEncMiscParameterBuffer *param = NULL;

    ASSERT_RET(obj_buffer->buffer_store->bo == NULL, VA_STATUS_ERROR_INVALID_BUFFER);
    ASSERT_RET(obj_buffer->buffer_store->buffer, VA_STATUS_ERROR_INVALID_BUFFER);

    param = (VAEncMiscParameterBuffer *)obj_buffer->buffer_store->buffer;

    if (param->type >= ARRAY_ELEMS(encode->misc_param))
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    i965_release_buffer_store(&encode->misc_param[param->type]);
    i965_reference_buffer_store(&encode->misc_param[param->type], obj_buffer->buffer_store);

    return VA_STATUS_SUCCESS;
}

static VAStatus
i965_encoder_render_picture(VADriverContextP ctx,
                            VAContextID context,
                            VABufferID *buffers,
                            int num_buffers)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_context *obj_context = CONTEXT(context);
    struct object_config *obj_config;
    VAStatus vaStatus = VA_STATUS_ERROR_UNKNOWN;
    struct encode_state *encode;
    int i;

    ASSERT_RET(obj_context, VA_STATUS_ERROR_INVALID_CONTEXT);
    obj_config = obj_context->obj_config;
    ASSERT_RET(obj_config, VA_STATUS_ERROR_INVALID_CONFIG);

    encode = &obj_context->codec_state.encode;
    for (i = 0; i < num_buffers; i++) {
        struct object_buffer *obj_buffer = BUFFER(buffers[i]);

        if (!obj_buffer)
            return VA_STATUS_ERROR_INVALID_BUFFER;

        switch (obj_buffer->type) {
        case VAQMatrixBufferType:
            vaStatus = I965_RENDER_ENCODE_BUFFER(qmatrix);
            break;

        case VAIQMatrixBufferType:
            vaStatus = I965_RENDER_ENCODE_BUFFER(iqmatrix);
            break;

        case VAEncSequenceParameterBufferType:
            vaStatus = I965_RENDER_ENCODE_BUFFER(sequence_parameter_ext);
            break;

        case VAEncPictureParameterBufferType:
            vaStatus = I965_RENDER_ENCODE_BUFFER(picture_parameter_ext);
            break;

        case VAHuffmanTableBufferType:
            vaStatus = I965_RENDER_ENCODE_BUFFER(huffman_table);
            break;

        case VAEncSliceParameterBufferType:
            vaStatus = I965_RENDER_ENCODE_BUFFER(slice_parameter_ext);
            if (vaStatus == VA_STATUS_SUCCESS) {
                /* When the max number of slices is updated, it also needs
                 * to reallocate the arrays that is used to store
                 * the packed data index/count for the slice
                 */
                if (!(encode->packed_header_flag & VA_ENC_PACKED_HEADER_SLICE)) {
                   encode->slice_index++;
                }
                if (encode->slice_index == encode->max_slice_num) {
                    int slice_num = encode->max_slice_num;
                    encode->slice_rawdata_index = realloc(encode->slice_rawdata_index,
                                                          (slice_num + NUM_SLICES) * sizeof(int));
                    encode->slice_rawdata_count = realloc(encode->slice_rawdata_count,
                                                          (slice_num + NUM_SLICES) * sizeof(int));
                    encode->slice_header_index = realloc(encode->slice_header_index,
                                                          (slice_num + NUM_SLICES) * sizeof(int));
                    memset(encode->slice_rawdata_index + slice_num, 0,
                        sizeof(int) * NUM_SLICES);
                    memset(encode->slice_rawdata_count + slice_num, 0,
                        sizeof(int) * NUM_SLICES);
                    memset(encode->slice_header_index + slice_num, 0,
                        sizeof(int) * NUM_SLICES);

                    encode->max_slice_num += NUM_SLICES;
                    if ((encode->slice_rawdata_index == NULL) ||
                        (encode->slice_header_index == NULL)  ||
                        (encode->slice_rawdata_count == NULL)) {
                        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
                        return vaStatus;
                    }
                }
            }
            break;

        case VAEncPackedHeaderParameterBufferType:
        {
            VAEncPackedHeaderParameterBuffer *param = (VAEncPackedHeaderParameterBuffer *)obj_buffer->buffer_store->buffer;
            encode->last_packed_header_type = param->type;

            if ((param->type == VAEncPackedHeaderRawData) ||
                (param->type == VAEncPackedHeaderSlice)) {
                vaStatus = I965_RENDER_ENCODE_BUFFER(packed_header_params_ext);
            } else if((obj_config->profile == VAProfileHEVCMain) &&
                (encode->last_packed_header_type == VAEncPackedHeaderSequence)) {
                vaStatus = i965_encoder_render_packed_header_parameter_buffer(ctx,
                                                                          obj_context,
                                                                          obj_buffer,
                                                                          va_enc_packed_type_to_idx(encode->last_packed_header_type) + encode->vps_sps_seq_index);
            } else {
                vaStatus = i965_encoder_render_packed_header_parameter_buffer(ctx,
                                                                          obj_context,
                                                                          obj_buffer,
                                                                          va_enc_packed_type_to_idx(encode->last_packed_header_type));
            }
            break;
        }

        case VAEncPackedHeaderDataBufferType:
        {
            if (encode->last_packed_header_type == 0) {
                WARN_ONCE("the packed header data is passed without type!\n");
                vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
                return vaStatus;
            }
            if (encode->last_packed_header_type == VAEncPackedHeaderRawData ||
                encode->last_packed_header_type == VAEncPackedHeaderSlice) {
                vaStatus = I965_RENDER_ENCODE_BUFFER(packed_header_data_ext);

                /* When the PACKED_SLICE_HEADER flag is passed, it will use
                 * the packed_slice_header as the delimeter to decide how
                 * the packed rawdata is inserted for the given slice.
                 * Otherwise it will use the VAEncSequenceParameterBuffer
                 * as the delimeter
                 */
                if (encode->packed_header_flag & VA_ENC_PACKED_HEADER_SLICE) {
                    /* store the first index of the packed header data for current slice */
                    if (encode->slice_rawdata_index[encode->slice_index] == 0) {
                        encode->slice_rawdata_index[encode->slice_index] =
                            SLICE_PACKED_DATA_INDEX_TYPE | (encode->num_packed_header_data_ext - 1);
                    }
                    encode->slice_rawdata_count[encode->slice_index]++;
                    if (encode->last_packed_header_type == VAEncPackedHeaderSlice) {
                        /* find one packed slice_header delimeter. And the following
                         * packed data is for the next slice
                         */
                        encode->slice_header_index[encode->slice_index] =
                            SLICE_PACKED_DATA_INDEX_TYPE | (encode->num_packed_header_data_ext - 1);
                        encode->slice_index++;
                        /* Reallocate the buffer to record the index/count of
                         * packed_data for one slice.
                         */
                        if (encode->slice_index == encode->max_slice_num) {
                            int slice_num = encode->max_slice_num;

                            encode->slice_rawdata_index = realloc(encode->slice_rawdata_index,
                                                          (slice_num + NUM_SLICES) * sizeof(int));
                            encode->slice_rawdata_count = realloc(encode->slice_rawdata_count,
                                                          (slice_num + NUM_SLICES) * sizeof(int));
                            encode->slice_header_index = realloc(encode->slice_header_index,
                                                          (slice_num + NUM_SLICES) * sizeof(int));
                            memset(encode->slice_rawdata_index + slice_num, 0,
                                   sizeof(int) * NUM_SLICES);
                            memset(encode->slice_rawdata_count + slice_num, 0,
                                   sizeof(int) * NUM_SLICES);
                            memset(encode->slice_header_index + slice_num, 0,
                                   sizeof(int) * NUM_SLICES);
                            encode->max_slice_num += NUM_SLICES;
                        }
                    }
                } else {
                    if (vaStatus == VA_STATUS_SUCCESS) {
                        /* store the first index of the packed header data for current slice */
                        if (encode->slice_rawdata_index[encode->slice_index] == 0) {
                            encode->slice_rawdata_index[encode->slice_index] =
                                SLICE_PACKED_DATA_INDEX_TYPE | (encode->num_packed_header_data_ext - 1);
                        }
                        encode->slice_rawdata_count[encode->slice_index]++;
                        if (encode->last_packed_header_type == VAEncPackedHeaderSlice) {
                            if (encode->slice_header_index[encode->slice_index] == 0) {
                                encode->slice_header_index[encode->slice_index] =
                                    SLICE_PACKED_DATA_INDEX_TYPE | (encode->num_packed_header_data_ext - 1);
                            } else {
                                WARN_ONCE("Multi slice header data is passed for"
                                      " slice %d!\n", encode->slice_index);
                            }
                        }
                    }
                }
            } else {
                ASSERT_RET(encode->last_packed_header_type == VAEncPackedHeaderSequence ||
                    encode->last_packed_header_type == VAEncPackedHeaderPicture ||
                    encode->last_packed_header_type == VAEncPackedHeaderSlice ||
                   (((encode->last_packed_header_type & VAEncPackedHeaderMiscMask) == VAEncPackedHeaderMiscMask) &&
                    ((encode->last_packed_header_type & (~VAEncPackedHeaderMiscMask)) != 0)),
                    VA_STATUS_ERROR_ENCODING_ERROR);

                if((obj_config->profile == VAProfileHEVCMain) &&
                    (encode->last_packed_header_type == VAEncPackedHeaderSequence)) {

                        vaStatus = i965_encoder_render_packed_header_data_buffer(ctx,
                            obj_context,
                            obj_buffer,
                            va_enc_packed_type_to_idx(encode->last_packed_header_type) + encode->vps_sps_seq_index);
                        encode->vps_sps_seq_index = (encode->vps_sps_seq_index + 1) % I965_SEQ_PACKED_HEADER_END;
                }else{
                    vaStatus = i965_encoder_render_packed_header_data_buffer(ctx,
                        obj_context,
                        obj_buffer,
                        va_enc_packed_type_to_idx(encode->last_packed_header_type));

                }
            }
            encode->last_packed_header_type = 0;
            break;
        }

        case VAEncMiscParameterBufferType:
            vaStatus = i965_encoder_render_misc_parameter_buffer(ctx,
                                                                 obj_context,
                                                                 obj_buffer);
            break;

        default:
            vaStatus = VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
            break;
        }
    }

    return vaStatus;
}

#define I965_RENDER_PROC_BUFFER(name) I965_RENDER_BUFFER(proc, name)

#define DEF_RENDER_PROC_SINGLE_BUFFER_FUNC(name, member) DEF_RENDER_SINGLE_BUFFER_FUNC(proc, name, member)
DEF_RENDER_PROC_SINGLE_BUFFER_FUNC(pipeline_parameter, pipeline_param)

static VAStatus
i965_proc_render_picture(VADriverContextP ctx,
                         VAContextID context,
                         VABufferID *buffers,
                         int num_buffers)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_context *obj_context = CONTEXT(context);
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int i;

    ASSERT_RET(obj_context, VA_STATUS_ERROR_INVALID_CONTEXT);

    for (i = 0; i < num_buffers && vaStatus == VA_STATUS_SUCCESS; i++) {
        struct object_buffer *obj_buffer = BUFFER(buffers[i]);

        if (!obj_buffer)
            return VA_STATUS_ERROR_INVALID_BUFFER;

        switch (obj_buffer->type) {
        case VAProcPipelineParameterBufferType:
            vaStatus = I965_RENDER_PROC_BUFFER(pipeline_parameter);
            break;

        default:
            vaStatus = VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
            break;
        }
    }

    return vaStatus;
}

VAStatus
i965_RenderPicture(VADriverContextP ctx,
                   VAContextID context,
                   VABufferID *buffers,
                   int num_buffers)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_context *obj_context;
    struct object_config *obj_config;
    VAStatus vaStatus = VA_STATUS_ERROR_UNKNOWN;

    obj_context = CONTEXT(context);
    ASSERT_RET(obj_context, VA_STATUS_ERROR_INVALID_CONTEXT);

    if (num_buffers <= 0)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    obj_config = obj_context->obj_config;
    ASSERT_RET(obj_config, VA_STATUS_ERROR_INVALID_CONFIG);

    if (VAEntrypointVideoProc == obj_config->entrypoint) {
        vaStatus = i965_proc_render_picture(ctx, context, buffers, num_buffers);
    } else if ((VAEntrypointEncSlice == obj_config->entrypoint ) ||
               (VAEntrypointEncPicture == obj_config->entrypoint)) {
        vaStatus = i965_encoder_render_picture(ctx, context, buffers, num_buffers);
    } else {
        vaStatus = i965_decoder_render_picture(ctx, context, buffers, num_buffers);
    }

    return vaStatus;
}

VAStatus
i965_EndPicture(VADriverContextP ctx, VAContextID context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_context *obj_context = CONTEXT(context);
    struct object_config *obj_config;

    ASSERT_RET(obj_context, VA_STATUS_ERROR_INVALID_CONTEXT);
    obj_config = obj_context->obj_config;
    ASSERT_RET(obj_config, VA_STATUS_ERROR_INVALID_CONFIG);

    if (obj_context->codec_type == CODEC_PROC) {
        ASSERT_RET(VAEntrypointVideoProc == obj_config->entrypoint, VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT);
    } else if (obj_context->codec_type == CODEC_ENC) {
        ASSERT_RET(((VAEntrypointEncSlice == obj_config->entrypoint) || (VAEntrypointEncPicture == obj_config->entrypoint)), VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT);

        if (obj_context->codec_state.encode.num_packed_header_params_ext !=
               obj_context->codec_state.encode.num_packed_header_data_ext) {
            WARN_ONCE("the packed header/data is not paired for encoding!\n");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (!(obj_context->codec_state.encode.pic_param ||
                obj_context->codec_state.encode.pic_param_ext)) {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (!(obj_context->codec_state.encode.seq_param ||
                obj_context->codec_state.encode.seq_param_ext) &&
                (VAEntrypointEncPicture != obj_config->entrypoint)) {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if ((obj_context->codec_state.encode.num_slice_params <=0) &&
                (obj_context->codec_state.encode.num_slice_params_ext <=0) &&
                (obj_config->profile != VAProfileVP8Version0_3)) {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        if ((obj_context->codec_state.encode.packed_header_flag & VA_ENC_PACKED_HEADER_SLICE) &&
            (obj_context->codec_state.encode.num_slice_params_ext !=
             obj_context->codec_state.encode.slice_index)) {
            WARN_ONCE("packed slice_header data is missing for some slice"
                      " under packed SLICE_HEADER mode\n");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    } else {
        if (obj_context->codec_state.decode.pic_param == NULL) {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (obj_context->codec_state.decode.num_slice_params <=0) {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (obj_context->codec_state.decode.num_slice_datas <=0) {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (obj_context->codec_state.decode.num_slice_params !=
                obj_context->codec_state.decode.num_slice_datas) {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (obj_context->wrapper_context != VA_INVALID_ID) {
            /* call the vaEndPicture of wrapped driver */
            VADriverContextP pdrvctx;
            VAStatus va_status;

            pdrvctx = i965->wrapper_pdrvctx;
            CALL_VTABLE(pdrvctx, va_status,
                        vaEndPicture(pdrvctx, obj_context->wrapper_context));

            return va_status;
        }
    }

    ASSERT_RET(obj_context->hw_context->run, VA_STATUS_ERROR_OPERATION_FAILED);
    return obj_context->hw_context->run(ctx, obj_config->profile, &obj_context->codec_state, obj_context->hw_context);
}

VAStatus
i965_SyncSurface(VADriverContextP ctx,
                 VASurfaceID render_target)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_surface *obj_surface = SURFACE(render_target);

    ASSERT_RET(obj_surface, VA_STATUS_ERROR_INVALID_SURFACE);

    if(obj_surface->bo)
        drm_intel_bo_wait_rendering(obj_surface->bo);

    return VA_STATUS_SUCCESS;
}

VAStatus
i965_QuerySurfaceStatus(VADriverContextP ctx,
                        VASurfaceID render_target,
                        VASurfaceStatus *status)        /* out */
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_surface *obj_surface = SURFACE(render_target);

    ASSERT_RET(obj_surface, VA_STATUS_ERROR_INVALID_SURFACE);

    if (obj_surface->bo) {
        if (drm_intel_bo_busy(obj_surface->bo)){
            *status = VASurfaceRendering;
        }
        else {
            *status = VASurfaceReady;
        }
    } else {
        *status = VASurfaceReady;
    }

    return VA_STATUS_SUCCESS;
}

static VADisplayAttribute *
get_display_attribute(VADriverContextP ctx, VADisplayAttribType type)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    unsigned int i;

    if (!i965->display_attributes)
        return NULL;

    for (i = 0; i < i965->num_display_attributes; i++) {
        if (i965->display_attributes[i].type == type)
            return &i965->display_attributes[i];
    }
    return NULL;
}

static void
i965_display_attributes_terminate(VADriverContextP ctx)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);

    if (i965->display_attributes) {
        free(i965->display_attributes);
        i965->display_attributes = NULL;
        i965->num_display_attributes = 0;
    }
}

static bool
i965_display_attributes_init(VADriverContextP ctx)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);

    i965->num_display_attributes = ARRAY_ELEMS(i965_display_attributes);
    i965->display_attributes = malloc(
        i965->num_display_attributes * sizeof(i965->display_attributes[0]));
    if (!i965->display_attributes)
        goto error;

    memcpy(
        i965->display_attributes,
        i965_display_attributes,
        sizeof(i965_display_attributes)
    );

    i965->rotation_attrib = get_display_attribute(ctx, VADisplayAttribRotation);
    i965->brightness_attrib = get_display_attribute(ctx, VADisplayAttribBrightness);
    i965->contrast_attrib = get_display_attribute(ctx, VADisplayAttribContrast);
    i965->hue_attrib = get_display_attribute(ctx, VADisplayAttribHue);
    i965->saturation_attrib = get_display_attribute(ctx, VADisplayAttribSaturation);

    if (!i965->rotation_attrib ||
        !i965->brightness_attrib ||
        !i965->contrast_attrib ||
        !i965->hue_attrib ||
        !i965->saturation_attrib) {
        goto error;
    }
    return true;

error:
    i965_display_attributes_terminate(ctx);
    return false;
}

/*
 * Query display attributes
 * The caller must provide a "attr_list" array that can hold at
 * least vaMaxNumDisplayAttributes() entries. The actual number of attributes
 * returned in "attr_list" is returned in "num_attributes".
 */
VAStatus
i965_QueryDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute *attribs,        /* out */
    int                *num_attribs_ptr /* out */
)
{
    const int num_attribs = ARRAY_ELEMS(i965_display_attributes);

    if (attribs && num_attribs > 0)
        memcpy(attribs, i965_display_attributes, sizeof(i965_display_attributes));

    if (num_attribs_ptr)
        *num_attribs_ptr = num_attribs;

    return VA_STATUS_SUCCESS;
}

/*
 * Get display attributes
 * This function returns the current attribute values in "attr_list".
 * Only attributes returned with VA_DISPLAY_ATTRIB_GETTABLE set in the "flags" field
 * from vaQueryDisplayAttributes() can have their values retrieved.
 */
VAStatus
i965_GetDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute *attribs,        /* inout */
    int                 num_attribs     /* in */
)
{
    int i;

    for (i = 0; i < num_attribs; i++) {
        VADisplayAttribute *src_attrib, * const dst_attrib = &attribs[i];

        src_attrib = get_display_attribute(ctx, dst_attrib->type);
        if (src_attrib && (src_attrib->flags & VA_DISPLAY_ATTRIB_GETTABLE)) {
            dst_attrib->min_value = src_attrib->min_value;
            dst_attrib->max_value = src_attrib->max_value;
            dst_attrib->value     = src_attrib->value;
        }
        else
            dst_attrib->flags = VA_DISPLAY_ATTRIB_NOT_SUPPORTED;
    }
    return VA_STATUS_SUCCESS;
}

/*
 * Set display attributes
 * Only attributes returned with VA_DISPLAY_ATTRIB_SETTABLE set in the "flags" field
 * from vaQueryDisplayAttributes() can be set.  If the attribute is not settable or
 * the value is out of range, the function returns VA_STATUS_ERROR_ATTR_NOT_SUPPORTED
 */
VAStatus
i965_SetDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute *attribs,        /* in */
    int                 num_attribs     /* in */
)
{
    int i;

    for (i = 0; i < num_attribs; i++) {
        VADisplayAttribute *dst_attrib, * const src_attrib = &attribs[i];

        dst_attrib = get_display_attribute(ctx, src_attrib->type);
        if (!dst_attrib)
            return VA_STATUS_ERROR_ATTR_NOT_SUPPORTED;

        if (!(dst_attrib->flags & VA_DISPLAY_ATTRIB_SETTABLE))
            continue;

        if (src_attrib->value < dst_attrib->min_value ||
            src_attrib->value > dst_attrib->max_value)
            return VA_STATUS_ERROR_INVALID_PARAMETER;

        dst_attrib->value = src_attrib->value;
        /* XXX: track modified attributes through timestamps */
    }
    return VA_STATUS_SUCCESS;
}

VAStatus
i965_DbgCopySurfaceToBuffer(VADriverContextP ctx,
                            VASurfaceID surface,
                            void **buffer,              /* out */
                            unsigned int *stride)       /* out */
{
    /* TODO */
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

static void
i965_destroy_heap(struct object_heap *heap,
                  void (*func)(struct object_heap *heap, struct object_base *object))
{
    struct object_base *object;
    object_heap_iterator iter;

    object = object_heap_first(heap, &iter);

    while (object) {
        if (func)
            func(heap, object);

        object = object_heap_next(heap, &iter);
    }

    object_heap_destroy(heap);
}


VAStatus
i965_DestroyImage(VADriverContextP ctx, VAImageID image);

VAStatus
i965_CreateImage(VADriverContextP ctx,
                 VAImageFormat *format,
                 int width,
                 int height,
                 VAImage *out_image)        /* out */
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_image *obj_image;
    VAStatus va_status = VA_STATUS_ERROR_OPERATION_FAILED;
    VAImageID image_id;
    unsigned int size2, size, awidth, aheight;

    out_image->image_id = VA_INVALID_ID;
    out_image->buf      = VA_INVALID_ID;

    image_id = NEW_IMAGE_ID();
    if (image_id == VA_INVALID_ID)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    obj_image = IMAGE(image_id);
    if (!obj_image)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    obj_image->bo         = NULL;
    obj_image->palette    = NULL;
    obj_image->derived_surface = VA_INVALID_ID;

    VAImage * const image = &obj_image->image;
    image->image_id       = image_id;
    image->buf            = VA_INVALID_ID;

    awidth = ALIGN(width, i965->codec_info->min_linear_wpitch);

    if ((format->fourcc == VA_FOURCC_YV12) ||
        (format->fourcc == VA_FOURCC_I420)) {
            if (awidth % 128 != 0) {
                awidth = ALIGN(width, 128);
            }
    }

    aheight = ALIGN(height, i965->codec_info->min_linear_hpitch);
    size    = awidth * aheight;
    size2    = (awidth / 2) * (aheight / 2);

    image->num_palette_entries = 0;
    image->entry_bytes         = 0;
    memset(image->component_order, 0, sizeof(image->component_order));

    switch (format->fourcc) {
    case VA_FOURCC_IA44:
    case VA_FOURCC_AI44:
        image->num_planes = 1;
        image->pitches[0] = awidth;
        image->offsets[0] = 0;
        image->data_size  = image->offsets[0] + image->pitches[0] * aheight;
        image->num_palette_entries = 16;
        image->entry_bytes         = 3;
        image->component_order[0]  = 'R';
        image->component_order[1]  = 'G';
        image->component_order[2]  = 'B';
        break;
    case VA_FOURCC_IA88:
    case VA_FOURCC_AI88:
        image->num_planes = 1;
        image->pitches[0] = awidth * 2;
        image->offsets[0] = 0;
        image->data_size  = image->offsets[0] + image->pitches[0] * aheight;
        image->num_palette_entries = 256;
        image->entry_bytes         = 3;
        image->component_order[0]  = 'R';
        image->component_order[1]  = 'G';
        image->component_order[2]  = 'B';
        break;
    case VA_FOURCC_ARGB:
    case VA_FOURCC_ABGR:
    case VA_FOURCC_BGRA:
    case VA_FOURCC_RGBA:
    case VA_FOURCC_BGRX:
    case VA_FOURCC_RGBX:
        image->num_planes = 1;
        image->pitches[0] = awidth * 4;
        image->offsets[0] = 0;
        image->data_size  = image->offsets[0] + image->pitches[0] * aheight;
        break;
    case VA_FOURCC_YV12:
        image->num_planes = 3;
        image->pitches[0] = awidth;
        image->offsets[0] = 0;
        image->pitches[1] = awidth / 2;
        image->offsets[1] = size;
        image->pitches[2] = awidth / 2;
        image->offsets[2] = size + size2;
        image->data_size  = size + 2 * size2;
        break;
    case VA_FOURCC_I420:
        image->num_planes = 3;
        image->pitches[0] = awidth;
        image->offsets[0] = 0;
        image->pitches[1] = awidth / 2;
        image->offsets[1] = size;
        image->pitches[2] = awidth / 2;
        image->offsets[2] = size + size2;
        image->data_size  = size + 2 * size2;
        break;
    case VA_FOURCC_422H:
        image->num_planes = 3;
        image->pitches[0] = awidth;
        image->offsets[0] = 0;
        image->pitches[1] = awidth / 2;
        image->offsets[1] = size;
        image->pitches[2] = awidth / 2;
        image->offsets[2] = size + (awidth / 2) * aheight;
        image->data_size  = size + 2 * ((awidth / 2) * aheight);
        break;
    case VA_FOURCC_NV12:
        image->num_planes = 2;
        image->pitches[0] = awidth;
        image->offsets[0] = 0;
        image->pitches[1] = awidth;
        image->offsets[1] = size;
        image->data_size  = size + 2 * size2;
        break;
    case VA_FOURCC_YUY2:
    case VA_FOURCC_UYVY:
        image->num_planes = 1;
        image->pitches[0] = awidth * 2;
        image->offsets[0] = 0;
        image->data_size  = size * 2;
        break;
    default:
        goto error;
    }

    va_status = i965_CreateBuffer(ctx, 0, VAImageBufferType,
                                  image->data_size, 1, NULL, &image->buf);
    if (va_status != VA_STATUS_SUCCESS)
        goto error;

    struct object_buffer *obj_buffer = BUFFER(image->buf);

    if (!obj_buffer ||
        !obj_buffer->buffer_store ||
        !obj_buffer->buffer_store->bo)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    obj_image->bo = obj_buffer->buffer_store->bo;
    dri_bo_reference(obj_image->bo);

    if (image->num_palette_entries > 0 && image->entry_bytes > 0) {
        obj_image->palette = malloc(image->num_palette_entries * sizeof(*obj_image->palette));
        if (!obj_image->palette)
            goto error;
    }

    image->image_id             = image_id;
    image->format               = *format;
    image->width                = width;
    image->height               = height;

    *out_image                  = *image;
    return VA_STATUS_SUCCESS;

 error:
    i965_DestroyImage(ctx, image_id);
    return va_status;
}

VAStatus
i965_check_alloc_surface_bo(VADriverContextP ctx,
                            struct object_surface *obj_surface,
                            int tiled,
                            unsigned int fourcc,
                            unsigned int subsampling)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int region_width, region_height;

    if (obj_surface->bo) {
        ASSERT_RET(obj_surface->fourcc, VA_STATUS_ERROR_INVALID_SURFACE);
        ASSERT_RET(obj_surface->fourcc == fourcc, VA_STATUS_ERROR_INVALID_SURFACE);
        ASSERT_RET(obj_surface->subsampling == subsampling, VA_STATUS_ERROR_INVALID_SURFACE);
        return VA_STATUS_SUCCESS;
    }

    obj_surface->x_cb_offset = 0; /* X offset is always 0 */
    obj_surface->x_cr_offset = 0;

    int bpp_1stplane = bpp_1stplane_by_fourcc(fourcc);

    if (obj_surface->user_h_stride_set) {
        ASSERT_RET(IS_ALIGNED(obj_surface->width, 128), VA_STATUS_ERROR_INVALID_PARAMETER);
    } else
        obj_surface->width = ALIGN(obj_surface->orig_width * bpp_1stplane, 128);

    if (obj_surface->user_v_stride_set) {
        ASSERT_RET(IS_ALIGNED(obj_surface->height, 32), VA_STATUS_ERROR_INVALID_PARAMETER);
    } else
        obj_surface->height = ALIGN(obj_surface->orig_height, 32);

    if ((tiled && !obj_surface->user_disable_tiling)) {
        ASSERT_RET(fourcc != VA_FOURCC_I420 &&
               fourcc != VA_FOURCC_IYUV &&
               fourcc != VA_FOURCC_YV12,
               VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT);

        region_height = obj_surface->height;

        switch (fourcc) {
        case VA_FOURCC_NV12:
        case VA_FOURCC_P010:
            assert(subsampling == SUBSAMPLE_YUV420);
            obj_surface->cb_cr_pitch = obj_surface->width;
            obj_surface->cb_cr_width = obj_surface->orig_width / 2;
            obj_surface->cb_cr_height = obj_surface->orig_height / 2;
            obj_surface->y_cb_offset = obj_surface->height;
            obj_surface->y_cr_offset = obj_surface->height;
            region_width = obj_surface->width;
            region_height = obj_surface->height + ALIGN(obj_surface->cb_cr_height, 32);

            break;

        case VA_FOURCC_IMC1:
            assert(subsampling == SUBSAMPLE_YUV420);
            obj_surface->cb_cr_pitch = obj_surface->width;
            obj_surface->cb_cr_width = obj_surface->orig_width / 2;
            obj_surface->cb_cr_height = obj_surface->orig_height / 2;
            obj_surface->y_cr_offset = obj_surface->height;
            obj_surface->y_cb_offset = obj_surface->y_cr_offset + ALIGN(obj_surface->cb_cr_height, 32);
            region_width = obj_surface->width;
            region_height = obj_surface->height + ALIGN(obj_surface->cb_cr_height, 32) * 2;

            break;

        case VA_FOURCC_IMC3:
            assert(subsampling == SUBSAMPLE_YUV420);
            obj_surface->cb_cr_pitch = obj_surface->width;
            obj_surface->cb_cr_width = obj_surface->orig_width / 2;
            obj_surface->cb_cr_height = obj_surface->orig_height / 2;
            obj_surface->y_cb_offset = obj_surface->height;
            obj_surface->y_cr_offset = obj_surface->y_cb_offset + ALIGN(obj_surface->cb_cr_height, 32);
            region_width = obj_surface->width;
            region_height = obj_surface->height + ALIGN(obj_surface->cb_cr_height, 32) * 2;

            break;

        case VA_FOURCC_422H:
            assert(subsampling == SUBSAMPLE_YUV422H);
            obj_surface->cb_cr_pitch = obj_surface->width;
            obj_surface->cb_cr_width = obj_surface->orig_width / 2;
            obj_surface->cb_cr_height = obj_surface->orig_height;
            obj_surface->y_cb_offset = obj_surface->height;
            obj_surface->y_cr_offset = obj_surface->y_cb_offset + ALIGN(obj_surface->cb_cr_height, 32);
            region_width = obj_surface->width;
            region_height = obj_surface->height + ALIGN(obj_surface->cb_cr_height, 32) * 2;

            break;

        case VA_FOURCC_422V:
            assert(subsampling == SUBSAMPLE_YUV422V);
            obj_surface->cb_cr_pitch = obj_surface->width;
            obj_surface->cb_cr_width = obj_surface->orig_width;
            obj_surface->cb_cr_height = obj_surface->orig_height / 2;
            obj_surface->y_cb_offset = obj_surface->height;
            obj_surface->y_cr_offset = obj_surface->y_cb_offset + ALIGN(obj_surface->cb_cr_height, 32);
            region_width = obj_surface->width;
            region_height = obj_surface->height + ALIGN(obj_surface->cb_cr_height, 32) * 2;

            break;

        case VA_FOURCC_411P:
            assert(subsampling == SUBSAMPLE_YUV411);
            obj_surface->cb_cr_pitch = obj_surface->width;
            obj_surface->cb_cr_width = obj_surface->orig_width / 4;
            obj_surface->cb_cr_height = obj_surface->orig_height;
            obj_surface->y_cb_offset = obj_surface->height;
            obj_surface->y_cr_offset = obj_surface->y_cb_offset + ALIGN(obj_surface->cb_cr_height, 32);
            region_width = obj_surface->width;
            region_height = obj_surface->height + ALIGN(obj_surface->cb_cr_height, 32) * 2;

            break;

        case VA_FOURCC_444P:
            assert(subsampling == SUBSAMPLE_YUV444);
            obj_surface->cb_cr_pitch = obj_surface->width;
            obj_surface->cb_cr_width = obj_surface->orig_width;
            obj_surface->cb_cr_height = obj_surface->orig_height;
            obj_surface->y_cb_offset = obj_surface->height;
            obj_surface->y_cr_offset = obj_surface->y_cb_offset + ALIGN(obj_surface->cb_cr_height, 32);
            region_width = obj_surface->width;
            region_height = obj_surface->height + ALIGN(obj_surface->cb_cr_height, 32) * 2;

            break;

        case VA_FOURCC_Y800:
            assert(subsampling == SUBSAMPLE_YUV400);
            obj_surface->cb_cr_pitch = 0;
            obj_surface->cb_cr_width = 0;
            obj_surface->cb_cr_height = 0;
            obj_surface->y_cb_offset = 0;
            obj_surface->y_cr_offset = 0;
            region_width = obj_surface->width;
            region_height = obj_surface->height;

            break;

        case VA_FOURCC_YUY2:
        case VA_FOURCC_UYVY:
            assert(subsampling == SUBSAMPLE_YUV422H);
            obj_surface->width = ALIGN(obj_surface->orig_width * 2, 128);
            obj_surface->cb_cr_pitch = obj_surface->width;
            obj_surface->y_cb_offset = 0;
            obj_surface->y_cr_offset = 0;
            obj_surface->cb_cr_width = obj_surface->orig_width / 2;
            obj_surface->cb_cr_height = obj_surface->orig_height / 2;
            region_width = obj_surface->width;
            region_height = obj_surface->height;

            break;

        case VA_FOURCC_RGBA:
        case VA_FOURCC_RGBX:
        case VA_FOURCC_BGRA:
        case VA_FOURCC_BGRX:
            assert(subsampling == SUBSAMPLE_RGBX);

            obj_surface->width = ALIGN(obj_surface->orig_width * 4, 128);
            region_width = obj_surface->width;
            region_height = obj_surface->height;
            break;

        default:
            /* Never get here */
            ASSERT_RET(0, VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT);
            break;
        }
    } else {
        assert(subsampling == SUBSAMPLE_YUV420 ||
               subsampling == SUBSAMPLE_YUV422H ||
               subsampling == SUBSAMPLE_YUV422V ||
               subsampling == SUBSAMPLE_RGBX);

        region_width = obj_surface->width;
        region_height = obj_surface->height;

        switch (fourcc) {
        case VA_FOURCC_NV12:
        case VA_FOURCC_P010:
            obj_surface->y_cb_offset = obj_surface->height;
            obj_surface->y_cr_offset = obj_surface->height;
            obj_surface->cb_cr_width = obj_surface->orig_width / 2;
            obj_surface->cb_cr_height = obj_surface->orig_height / 2;
            obj_surface->cb_cr_pitch = obj_surface->width;
            region_height = obj_surface->height + obj_surface->height / 2;
            break;

        case VA_FOURCC_YV16:
            obj_surface->cb_cr_width = obj_surface->orig_width / 2;
            obj_surface->cb_cr_height = obj_surface->orig_height;
            obj_surface->y_cr_offset = obj_surface->height;
            obj_surface->y_cb_offset = obj_surface->y_cr_offset + ALIGN(obj_surface->cb_cr_height, 32) / 2;
            obj_surface->cb_cr_pitch = obj_surface->width / 2;
            region_height = obj_surface->height + ALIGN(obj_surface->cb_cr_height, 32);
            break;

        case VA_FOURCC_YV12:
        case VA_FOURCC_I420:
            if (fourcc == VA_FOURCC_YV12) {
                obj_surface->y_cr_offset = obj_surface->height;
                obj_surface->y_cb_offset = obj_surface->height + obj_surface->height / 4;
            } else {
                obj_surface->y_cb_offset = obj_surface->height;
                obj_surface->y_cr_offset = obj_surface->height + obj_surface->height / 4;
            }

            obj_surface->cb_cr_width = obj_surface->orig_width / 2;
            obj_surface->cb_cr_height = obj_surface->orig_height / 2;
            obj_surface->cb_cr_pitch = obj_surface->width / 2;
            region_height = obj_surface->height + obj_surface->height / 2;
            break;

        case VA_FOURCC_YUY2:
        case VA_FOURCC_UYVY:
            obj_surface->width = ALIGN(obj_surface->orig_width * 2, i965->codec_info->min_linear_wpitch);
            obj_surface->y_cb_offset = 0;
            obj_surface->y_cr_offset = 0;
            obj_surface->cb_cr_width = obj_surface->orig_width / 2;
            obj_surface->cb_cr_height = obj_surface->orig_height;
            obj_surface->cb_cr_pitch = obj_surface->width;
            region_width = obj_surface->width;
            region_height = obj_surface->height;
            break;
        case VA_FOURCC_RGBA:
        case VA_FOURCC_RGBX:
        case VA_FOURCC_BGRA:
        case VA_FOURCC_BGRX:
            obj_surface->width = ALIGN(obj_surface->orig_width * 4, i965->codec_info->min_linear_wpitch);
            region_width = obj_surface->width;
            region_height = obj_surface->height;
            break;

        default:
            /* Never get here */
            ASSERT_RET(0, VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT);
            break;
        }
    }

    obj_surface->size = ALIGN(region_width * region_height, 0x1000);

    if ((tiled && !obj_surface->user_disable_tiling)) {
        uint32_t tiling_mode = I915_TILING_Y; /* always uses Y-tiled format */
        unsigned long pitch;

        obj_surface->bo = drm_intel_bo_alloc_tiled(i965->intel.bufmgr,
                                                   "vaapi surface",
                                                   region_width,
                                                   region_height,
                                                   1,
                                                   &tiling_mode,
                                                   &pitch,
                                                   0);
        assert(tiling_mode == I915_TILING_Y);
        assert(pitch == obj_surface->width);
    } else {
        obj_surface->bo = dri_bo_alloc(i965->intel.bufmgr,
                                       "vaapi surface",
                                       obj_surface->size,
                                       0x1000);
    }

    obj_surface->fourcc = fourcc;
    obj_surface->subsampling = subsampling;
    assert(obj_surface->bo);
    return VA_STATUS_SUCCESS;
}

VAStatus i965_DeriveImage(VADriverContextP ctx,
                          VASurfaceID surface,
                          VAImage *out_image)        /* out */
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_image *obj_image;
    struct object_surface *obj_surface;
    VAImageID image_id;
    unsigned int w_pitch;
    VAStatus va_status = VA_STATUS_ERROR_OPERATION_FAILED;

    out_image->image_id = VA_INVALID_ID;
    obj_surface = SURFACE(surface);

    if (!obj_surface)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    if (!obj_surface->bo) {
        unsigned int is_tiled = 0;
        unsigned int fourcc = VA_FOURCC_YV12;
        i965_guess_surface_format(ctx, surface, &fourcc, &is_tiled);
        int sampling = get_sampling_from_fourcc(fourcc);
        va_status = i965_check_alloc_surface_bo(ctx, obj_surface, is_tiled, fourcc, sampling);
        if (va_status != VA_STATUS_SUCCESS)
            return va_status;
    }

    ASSERT_RET(obj_surface->fourcc, VA_STATUS_ERROR_INVALID_SURFACE);

    w_pitch = obj_surface->width;

    image_id = NEW_IMAGE_ID();

    if (image_id == VA_INVALID_ID)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    obj_image = IMAGE(image_id);

    if (!obj_image)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    obj_image->bo = NULL;
    obj_image->palette = NULL;
    obj_image->derived_surface = VA_INVALID_ID;

    VAImage * const image = &obj_image->image;

    memset(image, 0, sizeof(*image));
    image->image_id = image_id;
    image->buf = VA_INVALID_ID;
    image->num_palette_entries = 0;
    image->entry_bytes = 0;
    image->width = obj_surface->orig_width;
    image->height = obj_surface->orig_height;
    image->data_size = obj_surface->size;

    image->format.fourcc = obj_surface->fourcc;
    image->format.byte_order = VA_LSB_FIRST;
    image->format.bits_per_pixel = get_bpp_from_fourcc(obj_surface->fourcc);

    if (!image->format.bits_per_pixel)
        goto error;

    switch (image->format.fourcc) {
    case VA_FOURCC_YV12:
        image->num_planes = 3;
        image->pitches[0] = w_pitch; /* Y */
        image->offsets[0] = 0;
        image->pitches[1] = obj_surface->cb_cr_pitch; /* V */
        image->offsets[1] = w_pitch * obj_surface->y_cr_offset;
        image->pitches[2] = obj_surface->cb_cr_pitch; /* U */
        image->offsets[2] = w_pitch * obj_surface->y_cb_offset;
        break;

    case VA_FOURCC_YV16:
        image->num_planes = 3;
        image->pitches[0] = w_pitch; /* Y */
        image->offsets[0] = 0;
        image->pitches[1] = obj_surface->cb_cr_pitch; /* V */
        image->offsets[1] = w_pitch * obj_surface->y_cr_offset;
        image->pitches[2] = obj_surface->cb_cr_pitch; /* U */
        image->offsets[2] = w_pitch * obj_surface->y_cb_offset;
        break;

    case VA_FOURCC_NV12:
    case VA_FOURCC_P010:
        image->num_planes = 2;
        image->pitches[0] = w_pitch; /* Y */
        image->offsets[0] = 0;
        image->pitches[1] = obj_surface->cb_cr_pitch; /* UV */
        image->offsets[1] = w_pitch * obj_surface->y_cb_offset;
        break;

    case VA_FOURCC_I420:
    case VA_FOURCC_422H:
    case VA_FOURCC_IMC3:
    case VA_FOURCC_444P:
    case VA_FOURCC_422V:
    case VA_FOURCC_411P:
        image->num_planes = 3;
        image->pitches[0] = w_pitch; /* Y */
        image->offsets[0] = 0;
        image->pitches[1] = obj_surface->cb_cr_pitch; /* U */
        image->offsets[1] = w_pitch * obj_surface->y_cb_offset;
        image->pitches[2] = obj_surface->cb_cr_pitch; /* V */
        image->offsets[2] = w_pitch * obj_surface->y_cr_offset;
        break;

    case VA_FOURCC_YUY2:
    case VA_FOURCC_UYVY:
    case VA_FOURCC_Y800:
        image->num_planes = 1;
        image->pitches[0] = obj_surface->width; /* Y, width is aligned already */
        image->offsets[0] = 0;
        break;
    case VA_FOURCC_RGBA:
    case VA_FOURCC_RGBX:
    case VA_FOURCC_BGRA:
    case VA_FOURCC_BGRX:
        image->num_planes = 1;
        image->pitches[0] = obj_surface->width;

        switch (image->format.fourcc) {
        case VA_FOURCC_RGBA:
        case VA_FOURCC_RGBX:
            image->format.red_mask = 0x000000ff;
            image->format.green_mask = 0x0000ff00;
            image->format.blue_mask = 0x00ff0000;
            image->format.alpha_mask = 0x00000000;
            break;
        case VA_FOURCC_BGRA:
        case VA_FOURCC_BGRX:
            image->format.red_mask = 0x00ff0000;
            image->format.green_mask = 0x0000ff00;
            image->format.blue_mask = 0x000000ff;
            image->format.alpha_mask = 0x00000000;
            break;
        default:
            goto error;
        }

        switch (image->format.fourcc) {
        case VA_FOURCC_RGBA:
        case VA_FOURCC_BGRA:
            image->format.depth = 32;
            break;
        case VA_FOURCC_RGBX:
        case VA_FOURCC_BGRX:
            image->format.depth = 24;
            break;
        default:
            goto error;
        }

        break;
    default:
        goto error;
    }

    va_status = i965_create_buffer_internal(ctx, 0, VAImageBufferType,
                                            obj_surface->size, 1, NULL, obj_surface->bo, &image->buf);
    if (va_status != VA_STATUS_SUCCESS)
        goto error;

    struct object_buffer *obj_buffer = BUFFER(image->buf);

    if (!obj_buffer ||
        !obj_buffer->buffer_store ||
        !obj_buffer->buffer_store->bo)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    obj_image->bo = obj_buffer->buffer_store->bo;
    dri_bo_reference(obj_image->bo);

    if (image->num_palette_entries > 0 && image->entry_bytes > 0) {
        obj_image->palette = malloc(image->num_palette_entries * sizeof(*obj_image->palette));
        if (!obj_image->palette) {
            va_status = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto error;
        }
    }

    *out_image = *image;
    obj_surface->flags |= SURFACE_DERIVED;
    obj_surface->derived_image_id = image_id;
    obj_image->derived_surface = surface;

    return VA_STATUS_SUCCESS;

 error:
    i965_DestroyImage(ctx, image_id);
    return va_status;
}

static void
i965_destroy_image(struct object_heap *heap, struct object_base *obj)
{
    object_heap_free(heap, obj);
}


VAStatus
i965_DestroyImage(VADriverContextP ctx, VAImageID image)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_image *obj_image = IMAGE(image);
    struct object_surface *obj_surface;

    if (!obj_image)
        return VA_STATUS_SUCCESS;

    dri_bo_unreference(obj_image->bo);
    obj_image->bo = NULL;

    if (obj_image->image.buf != VA_INVALID_ID) {
        i965_DestroyBuffer(ctx, obj_image->image.buf);
        obj_image->image.buf = VA_INVALID_ID;
    }

    if (obj_image->palette) {
        free(obj_image->palette);
        obj_image->palette = NULL;
    }

    obj_surface = SURFACE(obj_image->derived_surface);

    if (obj_surface) {
        obj_surface->flags &= ~SURFACE_DERIVED;
        obj_surface->derived_image_id = VA_INVALID_ID;
    }

    i965_destroy_image(&i965->image_heap, (struct object_base *)obj_image);

    return VA_STATUS_SUCCESS;
}

/*
 * pointer to an array holding the palette data.  The size of the array is
 * num_palette_entries * entry_bytes in size.  The order of the components
 * in the palette is described by the component_order in VASubpicture struct
 */
VAStatus
i965_SetImagePalette(VADriverContextP ctx,
                     VAImageID image,
                     unsigned char *palette)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    unsigned int i;

    struct object_image *obj_image = IMAGE(image);
    if (!obj_image)
        return VA_STATUS_ERROR_INVALID_IMAGE;

    if (!obj_image->palette)
        return VA_STATUS_ERROR_ALLOCATION_FAILED; /* XXX: unpaletted/error */

    for (i = 0; i < obj_image->image.num_palette_entries; i++)
        obj_image->palette[i] = (((unsigned int)palette[3*i + 0] << 16) |
                                 ((unsigned int)palette[3*i + 1] <<  8) |
                                 (unsigned int)palette[3*i + 2]);
    return VA_STATUS_SUCCESS;
}

static int
get_sampling_from_fourcc(unsigned int fourcc)
{
    const i965_fourcc_info *info = get_fourcc_info(fourcc);

    if (info && (info->flag & I_S))
        return info->subsampling;
    else
        return -1;
}

static inline void
memcpy_pic(uint8_t *dst, unsigned int dst_stride,
           const uint8_t *src, unsigned int src_stride,
           unsigned int len, unsigned int height)
{
    unsigned int i;

    for (i = 0; i < height; i++) {
        memcpy(dst, src, len);
        dst += dst_stride;
        src += src_stride;
    }
}

static VAStatus
get_image_i420(struct object_image *obj_image, uint8_t *image_data,
               struct object_surface *obj_surface,
               const VARectangle *rect)
{
    uint8_t *dst[3], *src[3];
    const int Y = 0;
    const int U = obj_image->image.format.fourcc == obj_surface->fourcc ? 1 : 2;
    const int V = obj_image->image.format.fourcc == obj_surface->fourcc ? 2 : 1;
    unsigned int tiling, swizzle;
    VAStatus va_status = VA_STATUS_SUCCESS;

    if (!obj_surface->bo)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    ASSERT_RET(obj_surface->fourcc, VA_STATUS_ERROR_INVALID_SURFACE);
    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_map_gtt(obj_surface->bo);
    else
        dri_bo_map(obj_surface->bo, 0);

    if (!obj_surface->bo->virtual)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    /* Dest VA image has either I420 or YV12 format.
       Source VA surface alway has I420 format */
    dst[Y] = image_data + obj_image->image.offsets[Y];
    src[0] = (uint8_t *)obj_surface->bo->virtual;
    dst[U] = image_data + obj_image->image.offsets[U];
    src[1] = src[0] + obj_surface->width * obj_surface->height;
    dst[V] = image_data + obj_image->image.offsets[V];
    src[2] = src[1] + (obj_surface->width / 2) * (obj_surface->height / 2);

    /* Y plane */
    dst[Y] += rect->y * obj_image->image.pitches[Y] + rect->x;
    src[0] += rect->y * obj_surface->width + rect->x;
    memcpy_pic(dst[Y], obj_image->image.pitches[Y],
               src[0], obj_surface->width,
               rect->width, rect->height);

    /* U plane */
    dst[U] += (rect->y / 2) * obj_image->image.pitches[U] + rect->x / 2;
    src[1] += (rect->y / 2) * obj_surface->width / 2 + rect->x / 2;
    memcpy_pic(dst[U], obj_image->image.pitches[U],
               src[1], obj_surface->width / 2,
               rect->width / 2, rect->height / 2);

    /* V plane */
    dst[V] += (rect->y / 2) * obj_image->image.pitches[V] + rect->x / 2;
    src[2] += (rect->y / 2) * obj_surface->width / 2 + rect->x / 2;
    memcpy_pic(dst[V], obj_image->image.pitches[V],
               src[2], obj_surface->width / 2,
               rect->width / 2, rect->height / 2);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_unmap_gtt(obj_surface->bo);
    else
        dri_bo_unmap(obj_surface->bo);

    return va_status;
}

static VAStatus
get_image_nv12(struct object_image *obj_image, uint8_t *image_data,
               struct object_surface *obj_surface,
               const VARectangle *rect)
{
    uint8_t *dst[2], *src[2];
    unsigned int tiling, swizzle;
    VAStatus va_status = VA_STATUS_SUCCESS;

    if (!obj_surface->bo)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    assert(obj_surface->fourcc);
    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_map_gtt(obj_surface->bo);
    else
        dri_bo_map(obj_surface->bo, 0);

    if (!obj_surface->bo->virtual)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    /* Both dest VA image and source surface have NV12 format */
    dst[0] = image_data + obj_image->image.offsets[0];
    src[0] = (uint8_t *)obj_surface->bo->virtual;
    dst[1] = image_data + obj_image->image.offsets[1];
    src[1] = src[0] + obj_surface->width * obj_surface->height;

    /* Y plane */
    dst[0] += rect->y * obj_image->image.pitches[0] + rect->x;
    src[0] += rect->y * obj_surface->width + rect->x;
    memcpy_pic(dst[0], obj_image->image.pitches[0],
               src[0], obj_surface->width,
               rect->width, rect->height);

    /* UV plane */
    dst[1] += (rect->y / 2) * obj_image->image.pitches[1] + (rect->x & -2);
    src[1] += (rect->y / 2) * obj_surface->width + (rect->x & -2);
    memcpy_pic(dst[1], obj_image->image.pitches[1],
               src[1], obj_surface->width,
               rect->width, rect->height / 2);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_unmap_gtt(obj_surface->bo);
    else
        dri_bo_unmap(obj_surface->bo);

    return va_status;
}

static VAStatus
get_image_yuy2(struct object_image *obj_image, uint8_t *image_data,
               struct object_surface *obj_surface,
               const VARectangle *rect)
{
    uint8_t *dst, *src;
    unsigned int tiling, swizzle;
    VAStatus va_status = VA_STATUS_SUCCESS;

    if (!obj_surface->bo)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    assert(obj_surface->fourcc);
    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_map_gtt(obj_surface->bo);
    else
        dri_bo_map(obj_surface->bo, 0);

    if (!obj_surface->bo->virtual)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    /* Both dest VA image and source surface have YUYV format */
    dst = image_data + obj_image->image.offsets[0];
    src = (uint8_t *)obj_surface->bo->virtual;

    /* Y plane */
    dst += rect->y * obj_image->image.pitches[0] + rect->x*2;
    src += rect->y * obj_surface->width + rect->x*2;
    memcpy_pic(dst, obj_image->image.pitches[0],
               src, obj_surface->width*2,
               rect->width*2, rect->height);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_unmap_gtt(obj_surface->bo);
    else
        dri_bo_unmap(obj_surface->bo);

    return va_status;
}

static VAStatus
i965_sw_getimage(VADriverContextP ctx,
    struct object_surface *obj_surface, struct object_image *obj_image,
    const VARectangle *rect)
{
    void *image_data = NULL;
    VAStatus va_status;

    if (obj_surface->fourcc != obj_image->image.format.fourcc)
        return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;

    va_status = i965_MapBuffer(ctx, obj_image->image.buf, &image_data);
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    switch (obj_image->image.format.fourcc) {
    case VA_FOURCC_YV12:
    case VA_FOURCC_I420:
        get_image_i420(obj_image, image_data, obj_surface, rect);
        break;
    case VA_FOURCC_NV12:
        get_image_nv12(obj_image, image_data, obj_surface, rect);
        break;
    case VA_FOURCC_YUY2:
        /* YUY2 is the format supported by overlay plane */
        get_image_yuy2(obj_image, image_data, obj_surface, rect);
        break;
    default:
        va_status = VA_STATUS_ERROR_OPERATION_FAILED;
        break;
    }
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    va_status = i965_UnmapBuffer(ctx, obj_image->image.buf);
    return va_status;
}

static VAStatus
i965_hw_getimage(VADriverContextP ctx,
    struct object_surface *obj_surface, struct object_image *obj_image,
    const VARectangle *rect)
{
    struct i965_surface src_surface;
    struct i965_surface dst_surface;

    src_surface.base = (struct object_base *)obj_surface;
    src_surface.type = I965_SURFACE_TYPE_SURFACE;
    src_surface.flags = I965_SURFACE_FLAG_FRAME;

    dst_surface.base = (struct object_base *)obj_image;
    dst_surface.type = I965_SURFACE_TYPE_IMAGE;
    dst_surface.flags = I965_SURFACE_FLAG_FRAME;

    return i965_image_processing(ctx, &src_surface, rect, &dst_surface, rect);
}

VAStatus
i965_GetImage(VADriverContextP ctx,
              VASurfaceID surface,
              int x,   /* coordinates of the upper left source pixel */
              int y,
              unsigned int width,      /* width and height of the region */
              unsigned int height,
              VAImageID image)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    struct object_surface * const obj_surface = SURFACE(surface);
    struct object_image * const obj_image = IMAGE(image);
    VARectangle rect;
    VAStatus va_status;

    if (!obj_surface)
        return VA_STATUS_ERROR_INVALID_SURFACE;
    if (!obj_surface->bo) /* don't get anything, keep previous data */
        return VA_STATUS_SUCCESS;
    if (is_surface_busy(i965, obj_surface))
        return VA_STATUS_ERROR_SURFACE_BUSY;

    if (!obj_image || !obj_image->bo)
        return VA_STATUS_ERROR_INVALID_IMAGE;
    if (is_image_busy(i965, obj_image, surface))
        return VA_STATUS_ERROR_SURFACE_BUSY;

    if (x < 0 || y < 0)
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    if (x + width > obj_surface->orig_width ||
        y + height > obj_surface->orig_height)
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    if (x + width > obj_image->image.width ||
        y + height > obj_image->image.height)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;

    if (HAS_ACCELERATED_GETIMAGE(i965))
        va_status = i965_hw_getimage(ctx, obj_surface, obj_image, &rect);
    else
        va_status = i965_sw_getimage(ctx, obj_surface, obj_image, &rect);

    return va_status;
}

static VAStatus
put_image_i420(struct object_surface *obj_surface,
               const VARectangle *dst_rect,
               struct object_image *obj_image, uint8_t *image_data,
               const VARectangle *src_rect)
{
    uint8_t *dst[3], *src[3];
    const int Y = 0;
    const int U = obj_image->image.format.fourcc == obj_surface->fourcc ? 1 : 2;
    const int V = obj_image->image.format.fourcc == obj_surface->fourcc ? 2 : 1;
    unsigned int tiling, swizzle;
    VAStatus va_status = VA_STATUS_SUCCESS;

    ASSERT_RET(obj_surface->bo, VA_STATUS_ERROR_INVALID_SURFACE);

    ASSERT_RET(obj_surface->fourcc, VA_STATUS_ERROR_INVALID_SURFACE);
    ASSERT_RET(dst_rect->width == src_rect->width, VA_STATUS_ERROR_UNIMPLEMENTED);
    ASSERT_RET(dst_rect->height == src_rect->height, VA_STATUS_ERROR_UNIMPLEMENTED);
    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_map_gtt(obj_surface->bo);
    else
        dri_bo_map(obj_surface->bo, 0);

    if (!obj_surface->bo->virtual)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    /* Dest VA image has either I420 or YV12 format.
       Source VA surface alway has I420 format */
    dst[0] = (uint8_t *)obj_surface->bo->virtual;
    src[Y] = image_data + obj_image->image.offsets[Y];
    dst[1] = dst[0] + obj_surface->width * obj_surface->height;
    src[U] = image_data + obj_image->image.offsets[U];
    dst[2] = dst[1] + (obj_surface->width / 2) * (obj_surface->height / 2);
    src[V] = image_data + obj_image->image.offsets[V];

    /* Y plane */
    dst[0] += dst_rect->y * obj_surface->width + dst_rect->x;
    src[Y] += src_rect->y * obj_image->image.pitches[Y] + src_rect->x;
    memcpy_pic(dst[0], obj_surface->width,
               src[Y], obj_image->image.pitches[Y],
               src_rect->width, src_rect->height);

    /* U plane */
    dst[1] += (dst_rect->y / 2) * obj_surface->width / 2 + dst_rect->x / 2;
    src[U] += (src_rect->y / 2) * obj_image->image.pitches[U] + src_rect->x / 2;
    memcpy_pic(dst[1], obj_surface->width / 2,
               src[U], obj_image->image.pitches[U],
               src_rect->width / 2, src_rect->height / 2);

    /* V plane */
    dst[2] += (dst_rect->y / 2) * obj_surface->width / 2 + dst_rect->x / 2;
    src[V] += (src_rect->y / 2) * obj_image->image.pitches[V] + src_rect->x / 2;
    memcpy_pic(dst[2], obj_surface->width / 2,
               src[V], obj_image->image.pitches[V],
               src_rect->width / 2, src_rect->height / 2);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_unmap_gtt(obj_surface->bo);
    else
        dri_bo_unmap(obj_surface->bo);

    return va_status;
}

static VAStatus
put_image_nv12(struct object_surface *obj_surface,
               const VARectangle *dst_rect,
               struct object_image *obj_image, uint8_t *image_data,
               const VARectangle *src_rect)
{
    uint8_t *dst[2], *src[2];
    unsigned int tiling, swizzle;
    VAStatus va_status = VA_STATUS_SUCCESS;

    if (!obj_surface->bo)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    ASSERT_RET(obj_surface->fourcc, VA_STATUS_ERROR_INVALID_SURFACE);
    ASSERT_RET(dst_rect->width == src_rect->width, VA_STATUS_ERROR_UNIMPLEMENTED);
    ASSERT_RET(dst_rect->height == src_rect->height, VA_STATUS_ERROR_UNIMPLEMENTED);
    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_map_gtt(obj_surface->bo);
    else
        dri_bo_map(obj_surface->bo, 0);

    if (!obj_surface->bo->virtual)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    /* Both dest VA image and source surface have NV12 format */
    dst[0] = (uint8_t *)obj_surface->bo->virtual;
    src[0] = image_data + obj_image->image.offsets[0];
    dst[1] = dst[0] + obj_surface->width * obj_surface->height;
    src[1] = image_data + obj_image->image.offsets[1];

    /* Y plane */
    dst[0] += dst_rect->y * obj_surface->width + dst_rect->x;
    src[0] += src_rect->y * obj_image->image.pitches[0] + src_rect->x;
    memcpy_pic(dst[0], obj_surface->width,
               src[0], obj_image->image.pitches[0],
               src_rect->width, src_rect->height);

    /* UV plane */
    dst[1] += (dst_rect->y / 2) * obj_surface->width + (dst_rect->x & -2);
    src[1] += (src_rect->y / 2) * obj_image->image.pitches[1] + (src_rect->x & -2);
    memcpy_pic(dst[1], obj_surface->width,
               src[1], obj_image->image.pitches[1],
               src_rect->width, src_rect->height / 2);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_unmap_gtt(obj_surface->bo);
    else
        dri_bo_unmap(obj_surface->bo);

    return va_status;
}

static VAStatus
put_image_yuy2(struct object_surface *obj_surface,
               const VARectangle *dst_rect,
               struct object_image *obj_image, uint8_t *image_data,
               const VARectangle *src_rect)
{
    uint8_t *dst, *src;
    unsigned int tiling, swizzle;
    VAStatus va_status = VA_STATUS_SUCCESS;

    ASSERT_RET(obj_surface->bo, VA_STATUS_ERROR_INVALID_SURFACE);
    ASSERT_RET(obj_surface->fourcc, VA_STATUS_ERROR_INVALID_SURFACE);
    ASSERT_RET(dst_rect->width == src_rect->width, VA_STATUS_ERROR_UNIMPLEMENTED);
    ASSERT_RET(dst_rect->height == src_rect->height, VA_STATUS_ERROR_UNIMPLEMENTED);
    dri_bo_get_tiling(obj_surface->bo, &tiling, &swizzle);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_map_gtt(obj_surface->bo);
    else
        dri_bo_map(obj_surface->bo, 0);

    if (!obj_surface->bo->virtual)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    /* Both dest VA image and source surface have YUY2 format */
    dst = (uint8_t *)obj_surface->bo->virtual;
    src = image_data + obj_image->image.offsets[0];

    /* YUYV packed plane */
    dst += dst_rect->y * obj_surface->width + dst_rect->x*2;
    src += src_rect->y * obj_image->image.pitches[0] + src_rect->x*2;
    memcpy_pic(dst, obj_surface->width*2,
               src, obj_image->image.pitches[0],
               src_rect->width*2, src_rect->height);

    if (tiling != I915_TILING_NONE)
        drm_intel_gem_bo_unmap_gtt(obj_surface->bo);
    else
        dri_bo_unmap(obj_surface->bo);

    return va_status;
}

static VAStatus
i965_sw_putimage(VADriverContextP ctx,
    struct object_surface *obj_surface, struct object_image *obj_image,
    const VARectangle *src_rect, const VARectangle *dst_rect)
{
    VAStatus va_status = VA_STATUS_SUCCESS;
    void *image_data = NULL;

    /* XXX: don't allow scaling */
    if (src_rect->width != dst_rect->width ||
        src_rect->height != dst_rect->height)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    if (obj_surface->fourcc) {
        /* Don't allow format mismatch */
        if (obj_surface->fourcc != obj_image->image.format.fourcc)
            return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;
    }

    else {
        /* VA is surface not used for decoding, use same VA image format */
        va_status = i965_check_alloc_surface_bo(
            ctx,
            obj_surface,
            0, /* XXX: don't use tiled surface */
            obj_image->image.format.fourcc,
            get_sampling_from_fourcc (obj_image->image.format.fourcc));
    }

    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    va_status = i965_MapBuffer(ctx, obj_image->image.buf, &image_data);
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    switch (obj_image->image.format.fourcc) {
    case VA_FOURCC_YV12:
    case VA_FOURCC_I420:
        va_status = put_image_i420(obj_surface, dst_rect, obj_image, image_data, src_rect);
        break;
    case VA_FOURCC_NV12:
        va_status = put_image_nv12(obj_surface, dst_rect, obj_image, image_data, src_rect);
        break;
    case VA_FOURCC_YUY2:
        va_status = put_image_yuy2(obj_surface, dst_rect, obj_image, image_data, src_rect);
        break;
    default:
        va_status = VA_STATUS_ERROR_OPERATION_FAILED;
        break;
    }
    if (va_status != VA_STATUS_SUCCESS)
        return va_status;

    va_status = i965_UnmapBuffer(ctx, obj_image->image.buf);
    return va_status;
}

static VAStatus
i965_hw_putimage(VADriverContextP ctx,
    struct object_surface *obj_surface, struct object_image *obj_image,
    const VARectangle *src_rect, const VARectangle *dst_rect)
{
    struct i965_surface src_surface, dst_surface;
    VAStatus va_status = VA_STATUS_SUCCESS;

    if (!obj_surface->bo) {
        unsigned int tiling, swizzle;
        int surface_sampling = get_sampling_from_fourcc (obj_image->image.format.fourcc);;
        dri_bo_get_tiling(obj_image->bo, &tiling, &swizzle);

        i965_check_alloc_surface_bo(ctx,
                                    obj_surface,
                                    !!tiling,
                                    obj_image->image.format.fourcc,
                                    surface_sampling);
    }

    ASSERT_RET(obj_surface->fourcc, VA_STATUS_ERROR_INVALID_SURFACE);

    src_surface.base = (struct object_base *)obj_image;
    src_surface.type = I965_SURFACE_TYPE_IMAGE;
    src_surface.flags = I965_SURFACE_FLAG_FRAME;

    dst_surface.base = (struct object_base *)obj_surface;
    dst_surface.type = I965_SURFACE_TYPE_SURFACE;
    dst_surface.flags = I965_SURFACE_FLAG_FRAME;

    va_status = i965_image_processing(ctx,
                                      &src_surface,
                                      src_rect,
                                      &dst_surface,
                                      dst_rect);

    return  va_status;
}

static VAStatus
i965_PutImage(VADriverContextP ctx,
              VASurfaceID surface,
              VAImageID image,
              int src_x,
              int src_y,
              unsigned int src_width,
              unsigned int src_height,
              int dest_x,
              int dest_y,
              unsigned int dest_width,
              unsigned int dest_height)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    struct object_surface * const obj_surface = SURFACE(surface);
    struct object_image * const obj_image = IMAGE(image);
    VARectangle src_rect, dst_rect;
    VAStatus va_status;

    if (!obj_surface)
        return VA_STATUS_ERROR_INVALID_SURFACE;
    if (is_surface_busy(i965, obj_surface))
        return VA_STATUS_ERROR_SURFACE_BUSY;

    if (!obj_image || !obj_image->bo)
        return VA_STATUS_ERROR_INVALID_IMAGE;
    if (is_image_busy(i965, obj_image, surface))
        return VA_STATUS_ERROR_SURFACE_BUSY;

    if (src_x < 0 ||
        src_y < 0 ||
        src_x + src_width > obj_image->image.width ||
        src_y + src_height > obj_image->image.height)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    src_rect.x       = src_x;
    src_rect.y       = src_y;
    src_rect.width   = src_width;
    src_rect.height  = src_height;

    if (dest_x < 0 ||
        dest_y < 0 ||
        dest_x + dest_width > obj_surface->orig_width ||
        dest_y + dest_height > obj_surface->orig_height)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    dst_rect.x      = dest_x;
    dst_rect.y      = dest_y;
    dst_rect.width  = dest_width;
    dst_rect.height = dest_height;

    if (HAS_ACCELERATED_PUTIMAGE(i965))
        va_status = i965_hw_putimage(ctx, obj_surface, obj_image,
            &src_rect, &dst_rect);
    else
        va_status = i965_sw_putimage(ctx, obj_surface, obj_image,
            &src_rect, &dst_rect);

    return va_status;
}

VAStatus
i965_PutSurface(VADriverContextP ctx,
                VASurfaceID surface,
                void *draw, /* X Drawable */
                short srcx,
                short srcy,
                unsigned short srcw,
                unsigned short srch,
                short destx,
                short desty,
                unsigned short destw,
                unsigned short desth,
                VARectangle *cliprects, /* client supplied clip list */
                unsigned int number_cliprects, /* number of clip rects in the clip list */
                unsigned int flags) /* de-interlacing flags */
{
#ifdef HAVE_VA_X11
    if (IS_VA_X11(ctx)) {
        VARectangle src_rect, dst_rect;

        src_rect.x      = srcx;
        src_rect.y      = srcy;
        src_rect.width  = srcw;
        src_rect.height = srch;

        dst_rect.x      = destx;
        dst_rect.y      = desty;
        dst_rect.width  = destw;
        dst_rect.height = desth;

        return i965_put_surface_dri(ctx, surface, draw, &src_rect, &dst_rect,
                                    cliprects, number_cliprects, flags);
    }
#endif
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

static VAStatus
i965_BufferInfo(
    VADriverContextP ctx,       /* in */
    VABufferID buf_id,          /* in */
    VABufferType *type,         /* out */
    unsigned int *size,         /* out */
    unsigned int *num_elements  /* out */
)
{
    struct i965_driver_data *i965 = NULL;
    struct object_buffer *obj_buffer = NULL;

    i965 = i965_driver_data(ctx);
    obj_buffer = BUFFER(buf_id);

    ASSERT_RET(obj_buffer, VA_STATUS_ERROR_INVALID_BUFFER);

    *type = obj_buffer->type;
    *size = obj_buffer->size_element;
    *num_elements = obj_buffer->num_elements;

    return VA_STATUS_SUCCESS;
}

static VAStatus
i965_LockSurface(
    VADriverContextP ctx,           /* in */
    VASurfaceID surface,            /* in */
    unsigned int *fourcc,           /* out */
    unsigned int *luma_stride,      /* out */
    unsigned int *chroma_u_stride,  /* out */
    unsigned int *chroma_v_stride,  /* out */
    unsigned int *luma_offset,      /* out */
    unsigned int *chroma_u_offset,  /* out */
    unsigned int *chroma_v_offset,  /* out */
    unsigned int *buffer_name,      /* out */
    void **buffer                   /* out */
)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_surface *obj_surface = NULL;
    VAImage tmpImage;

    ASSERT_RET(fourcc, VA_STATUS_ERROR_INVALID_PARAMETER);
    ASSERT_RET(luma_stride, VA_STATUS_ERROR_INVALID_PARAMETER);
    ASSERT_RET(chroma_u_stride, VA_STATUS_ERROR_INVALID_PARAMETER);
    ASSERT_RET(chroma_v_stride, VA_STATUS_ERROR_INVALID_PARAMETER);
    ASSERT_RET(luma_offset, VA_STATUS_ERROR_INVALID_PARAMETER);
    ASSERT_RET(chroma_u_offset, VA_STATUS_ERROR_INVALID_PARAMETER);
    ASSERT_RET(chroma_v_offset, VA_STATUS_ERROR_INVALID_PARAMETER);
    ASSERT_RET(buffer_name, VA_STATUS_ERROR_INVALID_PARAMETER);
    ASSERT_RET(buffer, VA_STATUS_ERROR_INVALID_PARAMETER);

    tmpImage.image_id = VA_INVALID_ID;

    obj_surface = SURFACE(surface);
    if (obj_surface == NULL) {
        // Surface is absent.
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
        goto error;
    }

    // Lock functionality is absent now.
    if (obj_surface->locked_image_id != VA_INVALID_ID) {
        // Surface is locked already.
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
        goto error;
    }

    vaStatus = i965_DeriveImage(
        ctx,
        surface,
        &tmpImage);
    if (vaStatus != VA_STATUS_SUCCESS) {
        goto error;
    }

    obj_surface->locked_image_id = tmpImage.image_id;

    vaStatus = i965_MapBuffer(
        ctx,
        tmpImage.buf,
        buffer);
    if (vaStatus != VA_STATUS_SUCCESS) {
        goto error;
    }

    *fourcc = tmpImage.format.fourcc;
    *luma_offset = tmpImage.offsets[0];
    *luma_stride = tmpImage.pitches[0];
    *chroma_u_offset = tmpImage.offsets[1];
    *chroma_u_stride = tmpImage.pitches[1];
    *chroma_v_offset = tmpImage.offsets[2];
    *chroma_v_stride = tmpImage.pitches[2];
    *buffer_name = tmpImage.buf;

error:
    if (vaStatus != VA_STATUS_SUCCESS) {
        buffer = NULL;
    }

    return vaStatus;
}

static VAStatus
i965_UnlockSurface(
    VADriverContextP ctx,   /* in */
    VASurfaceID surface     /* in */
)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_image *locked_img = NULL;
    struct object_surface *obj_surface = NULL;

    obj_surface = SURFACE(surface);

    if (obj_surface == NULL) {
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;   // Surface is absent
        return vaStatus;
    }
    if (obj_surface->locked_image_id == VA_INVALID_ID) {
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;   // Surface is not locked
        return vaStatus;
    }

    locked_img = IMAGE(obj_surface->locked_image_id);
    if (locked_img == NULL || (locked_img->image.image_id == VA_INVALID_ID)) {
        // Work image was deallocated before i965_UnlockSurface()
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
        goto error;
    }

    vaStatus = i965_UnmapBuffer(
        ctx,
        locked_img->image.buf);
    if (vaStatus != VA_STATUS_SUCCESS) {
        goto error;
    }

    vaStatus = i965_DestroyImage(
        ctx,
        locked_img->image.image_id);
    if (vaStatus != VA_STATUS_SUCCESS) {
        goto error;
    }

    locked_img->image.image_id = VA_INVALID_ID;

 error:
    obj_surface->locked_image_id = VA_INVALID_ID;

    return vaStatus;
}

static VAStatus
i965_GetSurfaceAttributes(
    VADriverContextP ctx,
    VAConfigID config,
    VASurfaceAttrib *attrib_list,
    unsigned int num_attribs
    )
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_config *obj_config;
    int i;

    if (config == VA_INVALID_ID)
        return VA_STATUS_ERROR_INVALID_CONFIG;

    obj_config = CONFIG(config);

    if (obj_config == NULL)
        return VA_STATUS_ERROR_INVALID_CONFIG;

    if (attrib_list == NULL || num_attribs == 0)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    for (i = 0; i < num_attribs; i++) {
        switch (attrib_list[i].type) {
        case VASurfaceAttribPixelFormat:
            attrib_list[i].value.type = VAGenericValueTypeInteger;
            attrib_list[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;

            if (attrib_list[i].value.value.i == 0) {
                if (IS_G4X(i965->intel.device_info)) {
                    if (obj_config->profile == VAProfileMPEG2Simple ||
                        obj_config->profile == VAProfileMPEG2Main) {
                        attrib_list[i].value.value.i = VA_FOURCC_I420;
                    } else {
                        assert(0);
                        attrib_list[i].flags = VA_SURFACE_ATTRIB_NOT_SUPPORTED;
                    }
                } else if (IS_IRONLAKE(i965->intel.device_info)) {
                    if (obj_config->profile == VAProfileMPEG2Simple ||
                        obj_config->profile == VAProfileMPEG2Main) {
                        attrib_list[i].value.value.i = VA_FOURCC_I420;
                    } else if (obj_config->profile == VAProfileH264ConstrainedBaseline ||
                               obj_config->profile == VAProfileH264Main ||
                               obj_config->profile == VAProfileH264High) {
                        attrib_list[i].value.value.i = VA_FOURCC_NV12;
                    } else if (obj_config->profile == VAProfileNone) {
                        attrib_list[i].value.value.i = VA_FOURCC_NV12;
                    } else {
                        assert(0);
                        attrib_list[i].flags = VA_SURFACE_ATTRIB_NOT_SUPPORTED;
                    }
                } else if (IS_GEN6(i965->intel.device_info)) {
                    attrib_list[i].value.value.i = VA_FOURCC_NV12;
                } else if (IS_GEN7(i965->intel.device_info) ||
                           IS_GEN8(i965->intel.device_info) ||
                           IS_GEN9(i965->intel.device_info)) {
                    if (obj_config->profile == VAProfileJPEGBaseline)
                        attrib_list[i].value.value.i = 0; /* internal format */
                    else
                        attrib_list[i].value.value.i = VA_FOURCC_NV12;
                }
            } else {
                if (IS_G4X(i965->intel.device_info)) {
                    if (obj_config->profile == VAProfileMPEG2Simple ||
                        obj_config->profile == VAProfileMPEG2Main) {
                        if (attrib_list[i].value.value.i != VA_FOURCC_I420) {
                            attrib_list[i].value.value.i = 0;
                            attrib_list[i].flags &= ~VA_SURFACE_ATTRIB_SETTABLE;
                        }
                    } else {
                        assert(0);
                        attrib_list[i].flags = VA_SURFACE_ATTRIB_NOT_SUPPORTED;
                    }
                } else if (IS_IRONLAKE(i965->intel.device_info)) {
                    if (obj_config->profile == VAProfileMPEG2Simple ||
                        obj_config->profile == VAProfileMPEG2Main) {
                        if (attrib_list[i].value.value.i != VA_FOURCC_I420) {
                            attrib_list[i].value.value.i = 0;
                            attrib_list[i].flags &= ~VA_SURFACE_ATTRIB_SETTABLE;
                        }
                    } else if (obj_config->profile == VAProfileH264ConstrainedBaseline ||
                               obj_config->profile == VAProfileH264Main ||
                               obj_config->profile == VAProfileH264High) {
                        if (attrib_list[i].value.value.i != VA_FOURCC_NV12) {
                            attrib_list[i].value.value.i = 0;
                            attrib_list[i].flags &= ~VA_SURFACE_ATTRIB_SETTABLE;
                        }
                    } else if (obj_config->profile == VAProfileNone) {
                        switch (attrib_list[i].value.value.i) {
                        case VA_FOURCC_NV12:
                        case VA_FOURCC_I420:
                        case VA_FOURCC_YV12:
                        case VA_FOURCC_YUY2:
                        case VA_FOURCC_BGRA:
                        case VA_FOURCC_BGRX:
                        case VA_FOURCC_RGBX:
                        case VA_FOURCC_RGBA:
                            break;
                        default:
                            attrib_list[i].value.value.i = 0;
                            attrib_list[i].flags &= ~VA_SURFACE_ATTRIB_SETTABLE;
                            break;
                        }
                    } else {
                        assert(0);
                        attrib_list[i].flags = VA_SURFACE_ATTRIB_NOT_SUPPORTED;
                    }
                } else if (IS_GEN6(i965->intel.device_info)) {
                    if (obj_config->entrypoint == VAEntrypointEncSlice ||
                        obj_config->entrypoint == VAEntrypointVideoProc) {
                        switch (attrib_list[i].value.value.i) {
                        case VA_FOURCC_NV12:
                        case VA_FOURCC_I420:
                        case VA_FOURCC_YV12:
                        case VA_FOURCC_YUY2:
                        case VA_FOURCC_BGRA:
                        case VA_FOURCC_BGRX:
                        case VA_FOURCC_RGBX:
                        case VA_FOURCC_RGBA:
                            break;
                        default:
                            attrib_list[i].value.value.i = 0;
                            attrib_list[i].flags &= ~VA_SURFACE_ATTRIB_SETTABLE;
                            break;
                        }
                    } else {
                        if (attrib_list[i].value.value.i != VA_FOURCC_NV12) {
                            attrib_list[i].value.value.i = 0;
                            attrib_list[i].flags &= ~VA_SURFACE_ATTRIB_SETTABLE;
                        }
                    }
                } else if (IS_GEN7(i965->intel.device_info) ||
                           IS_GEN8(i965->intel.device_info) ||
                           IS_GEN9(i965->intel.device_info)) {
                    if (obj_config->entrypoint == VAEntrypointEncSlice ||
                        obj_config->entrypoint == VAEntrypointVideoProc) {
                        switch (attrib_list[i].value.value.i) {
                        case VA_FOURCC_NV12:
                        case VA_FOURCC_I420:
                        case VA_FOURCC_YV12:
                            break;
                        default:
                            attrib_list[i].value.value.i = 0;
                            attrib_list[i].flags &= ~VA_SURFACE_ATTRIB_SETTABLE;
                            break;
                        }
                    } else {
                        if (obj_config->profile == VAProfileJPEGBaseline) {
                            attrib_list[i].value.value.i = 0;   /* JPEG decoding always uses an internal format */
                            attrib_list[i].flags &= ~VA_SURFACE_ATTRIB_SETTABLE;
                        } else {
                            if (attrib_list[i].value.value.i != VA_FOURCC_NV12) {
                                attrib_list[i].value.value.i = 0;
                                attrib_list[i].flags &= ~VA_SURFACE_ATTRIB_SETTABLE;
                            }
                        }
                    }
                }
            }

            break;
        case VASurfaceAttribMinWidth:
            /* FIXME: add support for it later */
            attrib_list[i].flags = VA_SURFACE_ATTRIB_NOT_SUPPORTED;
            break;
        case VASurfaceAttribMaxWidth:
            attrib_list[i].flags = VA_SURFACE_ATTRIB_NOT_SUPPORTED;
            break;
        case VASurfaceAttribMinHeight:
            attrib_list[i].flags = VA_SURFACE_ATTRIB_NOT_SUPPORTED;
            break;
        case VASurfaceAttribMaxHeight:
            attrib_list[i].flags = VA_SURFACE_ATTRIB_NOT_SUPPORTED;
            break;
        default:
            attrib_list[i].flags = VA_SURFACE_ATTRIB_NOT_SUPPORTED;
            break;
        }
    }

    return vaStatus;
}

static VAStatus
i965_QuerySurfaceAttributes(VADriverContextP ctx,
                            VAConfigID config,
                            VASurfaceAttrib *attrib_list,
                            unsigned int *num_attribs)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_config *obj_config;
    int i = 0;
    VASurfaceAttrib *attribs = NULL;

    if (config == VA_INVALID_ID)
        return VA_STATUS_ERROR_INVALID_CONFIG;

    obj_config = CONFIG(config);

    if (obj_config == NULL)
        return VA_STATUS_ERROR_INVALID_CONFIG;

    if (!attrib_list && !num_attribs)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    if (attrib_list == NULL) {
        *num_attribs = I965_MAX_SURFACE_ATTRIBUTES;
        return VA_STATUS_SUCCESS;
    }

    attribs = malloc(I965_MAX_SURFACE_ATTRIBUTES *sizeof(*attribs));

    if (attribs == NULL)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    if (IS_G4X(i965->intel.device_info)) {
        if (obj_config->profile == VAProfileMPEG2Simple ||
            obj_config->profile == VAProfileMPEG2Main) {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_I420;
            i++;
        }
    } else if (IS_IRONLAKE(i965->intel.device_info)) {
        switch (obj_config->profile) {
        case VAProfileMPEG2Simple:
        case VAProfileMPEG2Main:
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_I420;
            i++;

            break;

        case VAProfileH264ConstrainedBaseline:
        case VAProfileH264Main:
        case VAProfileH264High:
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_NV12;
            i++;

        case VAProfileNone:
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_NV12;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_I420;
            i++;

            break;

        default:
            break;
        }
    } else if (IS_GEN6(i965->intel.device_info)) {
        if (obj_config->entrypoint == VAEntrypointVLD) { /* decode */
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_NV12;
            i++;
        } else if (obj_config->entrypoint == VAEntrypointEncSlice ||  /* encode */
                   obj_config->entrypoint == VAEntrypointVideoProc) { /* vpp */
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_NV12;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_I420;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_YV12;
            i++;

            if (obj_config->entrypoint == VAEntrypointVideoProc) {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_YUY2;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_RGBA;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_RGBX;
                i++;
            }
        }
    } else if (IS_GEN7(i965->intel.device_info)) {
        if (obj_config->entrypoint == VAEntrypointVLD) { /* decode */
            if (obj_config->profile == VAProfileJPEGBaseline) {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_IMC3;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_IMC1;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_Y800;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_411P;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_422H;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_422V;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_444P;
                i++;
            } else if (obj_config->profile == VAProfileHEVCMain10) {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_P010;
                i++;
             } else {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_NV12;
                i++;
            }
        } else if (obj_config->entrypoint == VAEntrypointEncSlice ||  /* encode */
                   obj_config->entrypoint == VAEntrypointVideoProc) { /* vpp */
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_NV12;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_I420;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_YV12;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_IMC3;
            i++;

            if (obj_config->entrypoint == VAEntrypointVideoProc) {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_YUY2;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_RGBA;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_RGBX;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_BGRA;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_BGRX;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_YV16;
                i++;
            }
        }
    } else if (IS_GEN8(i965->intel.device_info) ||
               IS_GEN9(i965->intel.device_info)) {
        if (obj_config->entrypoint == VAEntrypointVLD) { /* decode */
            if (obj_config->profile == VAProfileJPEGBaseline) {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_IMC3;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_IMC1;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_Y800;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_411P;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_422H;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_422V;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_444P;
                i++;
            } else {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_NV12;
                i++;
            }
        } else if (obj_config->entrypoint == VAEntrypointEncSlice ||  /* encode */
                   obj_config->entrypoint == VAEntrypointVideoProc) { /* vpp */

            if (obj_config->profile == VAProfileHEVCMain10) {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_P010;
                i++;
            } else {
              attribs[i].type = VASurfaceAttribPixelFormat;
              attribs[i].value.type = VAGenericValueTypeInteger;
              attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
              attribs[i].value.value.i = VA_FOURCC_NV12;
              i++;

              attribs[i].type = VASurfaceAttribPixelFormat;
              attribs[i].value.type = VAGenericValueTypeInteger;
              attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
              attribs[i].value.value.i = VA_FOURCC_I420;
              i++;

              attribs[i].type = VASurfaceAttribPixelFormat;
              attribs[i].value.type = VAGenericValueTypeInteger;
              attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
              attribs[i].value.value.i = VA_FOURCC_YV12;
              i++;

              attribs[i].type = VASurfaceAttribPixelFormat;
              attribs[i].value.type = VAGenericValueTypeInteger;
              attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
              attribs[i].value.value.i = VA_FOURCC_IMC3;
              i++;
            }

            if (obj_config->entrypoint == VAEntrypointVideoProc) {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_YUY2;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_RGBA;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_RGBX;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_BGRA;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_BGRX;
                i++;

                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_YV16;
                i++;
            }
        }
    }

    attribs[i].type = VASurfaceAttribMemoryType;
    attribs[i].value.type = VAGenericValueTypeInteger;
    attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
    attribs[i].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
        VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM |
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
    i++;

    attribs[i].type = VASurfaceAttribExternalBufferDescriptor;
    attribs[i].value.type = VAGenericValueTypePointer;
    attribs[i].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[i].value.value.p = NULL; /* ignore */
    i++;

    attribs[i].type = VASurfaceAttribMaxWidth;
    attribs[i].value.type = VAGenericValueTypeInteger;
    attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE;
    attribs[i].value.value.i = i965->codec_info->max_width;
    i++;

    attribs[i].type = VASurfaceAttribMaxHeight;
    attribs[i].value.type = VAGenericValueTypeInteger;
    attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE;
    attribs[i].value.value.i = i965->codec_info->max_height;
    i++;

    if (i > *num_attribs) {
        *num_attribs = i;
        free(attribs);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    *num_attribs = i;
    memcpy(attrib_list, attribs, i * sizeof(*attribs));
    free(attribs);

    return vaStatus;
}

/* Acquires buffer handle for external API usage (internal implementation) */
static VAStatus
i965_acquire_buffer_handle(struct object_buffer *obj_buffer,
    uint32_t mem_type, VABufferInfo *out_buf_info)
{
    struct buffer_store *buffer_store;

    buffer_store = obj_buffer->buffer_store;
    if (!buffer_store || !buffer_store->bo)
        return VA_STATUS_ERROR_INVALID_BUFFER;

    /* Synchronization point */
    drm_intel_bo_wait_rendering(buffer_store->bo);

    if (obj_buffer->export_refcount > 0) {
        if (obj_buffer->export_state.mem_type != mem_type)
            return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    else {
        VABufferInfo * const buf_info = &obj_buffer->export_state;

        switch (mem_type) {
        case VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM: {
            uint32_t name;
            if (drm_intel_bo_flink(buffer_store->bo, &name) != 0)
                return VA_STATUS_ERROR_INVALID_BUFFER;
            buf_info->handle = name;
            break;
        }
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME: {
            int fd = -1;
//            if (drm_intel_bo_gem_export_to_prime(buffer_store->bo, &fd) != 0)
//                return VA_STATUS_ERROR_INVALID_BUFFER;
            buf_info->handle = (intptr_t)fd;
            break;
        }
        }

        buf_info->type = obj_buffer->type;
        buf_info->mem_type = mem_type;
        buf_info->mem_size =
            obj_buffer->num_elements * obj_buffer->size_element;
    }

    obj_buffer->export_refcount++;
    *out_buf_info = obj_buffer->export_state;
    return VA_STATUS_SUCCESS;
}

/* Releases buffer handle after usage (internal implementation) */
static VAStatus
i965_release_buffer_handle(struct object_buffer *obj_buffer)
{
    if (obj_buffer->export_refcount == 0)
        return VA_STATUS_ERROR_INVALID_BUFFER;

    if (--obj_buffer->export_refcount == 0) {
        VABufferInfo * const buf_info = &obj_buffer->export_state;

        switch (buf_info->mem_type) {
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME: {
            close((intptr_t)buf_info->handle);
            break;
        }
        }
        buf_info->mem_type = 0;
    }
    return VA_STATUS_SUCCESS;
}

/** Acquires buffer handle for external API usage */
static VAStatus
i965_AcquireBufferHandle(VADriverContextP ctx, VABufferID buf_id,
    VABufferInfo *buf_info)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    struct object_buffer * const obj_buffer = BUFFER(buf_id);
    uint32_t i, mem_type;

    /* List of supported memory types, in preferred order */
    static const uint32_t mem_types[] = {
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME,
        VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM,
        0
    };

    if (!obj_buffer)
        return VA_STATUS_ERROR_INVALID_BUFFER;
    /* XXX: only VA surface|image like buffers are supported for now */
    if (obj_buffer->type != VAImageBufferType)
        return VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;

    /*
     * As the allocated buffer by calling vaCreateBuffer is related with
     * the specific context, it is unnecessary to export it.
     * So it is not supported when the buffer is allocated from wrapped
     * backend dirver.
     */
    if (obj_buffer->wrapper_buffer != VA_INVALID_ID) {
        return VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
    }

    if (!buf_info)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    if (!buf_info->mem_type)
        mem_type = mem_types[0];
    else {
        mem_type = 0;
        for (i = 0; mem_types[i] != 0; i++) {
            if (buf_info->mem_type & mem_types[i]) {
                mem_type = buf_info->mem_type;
                break;
            }
        }
        if (!mem_type)
            return VA_STATUS_ERROR_UNSUPPORTED_MEMORY_TYPE;
    }
    return i965_acquire_buffer_handle(obj_buffer, mem_type, buf_info);
}

/** Releases buffer handle after usage from external API */
static VAStatus
i965_ReleaseBufferHandle(VADriverContextP ctx, VABufferID buf_id)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    struct object_buffer * const obj_buffer = BUFFER(buf_id);

    if (!obj_buffer)
        return VA_STATUS_ERROR_INVALID_BUFFER;

    if (obj_buffer->wrapper_buffer != VA_INVALID_ID) {
        return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    return i965_release_buffer_handle(obj_buffer);
}

static int
i965_os_has_ring_support(VADriverContextP ctx,
                         int ring)
{
    struct i965_driver_data *const i965 = i965_driver_data(ctx);

    switch (ring) {
    case I965_RING_BSD:
        return i965->intel.has_bsd;

    case I965_RING_BLT:
        return i965->intel.has_blt;

    case I965_RING_VEBOX:
        return i965->intel.has_vebox;

    case I965_RING_NULL:
        return 1; /* Always support */

    default:
        /* should never get here */
        assert(0);
        break;
    }

    return 0;
}

/*
 * Query video processing pipeline
 */
VAStatus i965_QueryVideoProcFilters(
    VADriverContextP    ctx,
    VAContextID         context,
    VAProcFilterType   *filters,
    unsigned int       *num_filters
    )
{
    struct i965_driver_data *const i965 = i965_driver_data(ctx);
    unsigned int i = 0, num = 0;

    if (!num_filters  || !filters)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    for (i = 0; i < i965->codec_info->num_filters; i++) {
        if (i965_os_has_ring_support(ctx, i965->codec_info->filters[i].ring)) {
            if (num == *num_filters) {
                *num_filters = i965->codec_info->num_filters;

                return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
            }

            filters[num++] = i965->codec_info->filters[i].type;
        }
    }

    *num_filters = num;

    return VA_STATUS_SUCCESS;
}

VAStatus i965_QueryVideoProcFilterCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VAProcFilterType    type,
    void               *filter_caps,
    unsigned int       *num_filter_caps
    )
{
    unsigned int i = 0;
    struct i965_driver_data *const i965 = i965_driver_data(ctx);

    if (!filter_caps || !num_filter_caps)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    for (i = 0; i < i965->codec_info->num_filters; i++) {
        if (type == i965->codec_info->filters[i].type &&
            i965_os_has_ring_support(ctx, i965->codec_info->filters[i].ring))
            break;
    }

    if (i == i965->codec_info->num_filters)
        return VA_STATUS_ERROR_UNSUPPORTED_FILTER;

    i = 0;

    switch (type) {
    case VAProcFilterNoiseReduction:
    case VAProcFilterSharpening:
        {
            VAProcFilterCap *cap = filter_caps;

            if (*num_filter_caps < 1) {
                *num_filter_caps = 1;
                return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
            }

            cap->range.min_value = 0.0;
            cap->range.max_value = 1.0;
            cap->range.default_value = 0.5;
            cap->range.step = 0.03125; /* 1.0 / 32 */
            i++;
        }

        break;

    case VAProcFilterDeinterlacing:
        {
            VAProcFilterCapDeinterlacing *cap = filter_caps;

            if (*num_filter_caps < VAProcDeinterlacingCount) {
                *num_filter_caps = VAProcDeinterlacingCount;
                return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
            }

            cap->type = VAProcDeinterlacingBob;
            i++;
            cap++;


            if (i965->codec_info->has_di_motion_adptive) {
                cap->type = VAProcDeinterlacingMotionAdaptive;
                i++;
                cap++;
            }

            if (i965->codec_info->has_di_motion_compensated) {
                cap->type = VAProcDeinterlacingMotionCompensated;
                i++;
                cap++;
            }
       }

        break;

    case VAProcFilterColorBalance:
        {
            VAProcFilterCapColorBalance *cap = filter_caps;

            if (*num_filter_caps < VAProcColorBalanceCount) {
                *num_filter_caps = VAProcColorBalanceCount;
                return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
            }

            cap->type = VAProcColorBalanceHue;
            cap->range.min_value = -180.0;
            cap->range.max_value = 180.0;
            cap->range.default_value = 0.0;
            cap->range.step = 1.0;
            i++;
            cap++;

            cap->type = VAProcColorBalanceSaturation;
            cap->range.min_value = 0.0;
            cap->range.max_value = 10.0;
            cap->range.default_value = 1.0;
            cap->range.step = 0.1;
            i++;
            cap++;

            cap->type = VAProcColorBalanceBrightness;
            cap->range.min_value = -100.0;
            cap->range.max_value = 100.0;
            cap->range.default_value = 0.0;
            cap->range.step = 1.0;
            i++;
            cap++;

            cap->type = VAProcColorBalanceContrast;
            cap->range.min_value = 0.0;
            cap->range.max_value = 10.0;
            cap->range.default_value = 1.0;
            cap->range.step = 0.1;
            i++;
            cap++;
        }

        break;

    default:

        break;
    }

    *num_filter_caps = i;

    return VA_STATUS_SUCCESS;
}

static VAProcColorStandardType vpp_input_color_standards[VAProcColorStandardCount] = {
    VAProcColorStandardBT601,
};

static VAProcColorStandardType vpp_output_color_standards[VAProcColorStandardCount] = {
    VAProcColorStandardBT601,
};

VAStatus i965_QueryVideoProcPipelineCaps(
    VADriverContextP ctx,
    VAContextID context,
    VABufferID *filters,
    unsigned int num_filters,
    VAProcPipelineCaps *pipeline_cap     /* out */
    )
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    unsigned int i = 0;

    pipeline_cap->pipeline_flags = 0;
    pipeline_cap->filter_flags = 0;
    pipeline_cap->num_forward_references = 0;
    pipeline_cap->num_backward_references = 0;
    pipeline_cap->num_input_color_standards = 1;
    pipeline_cap->input_color_standards = vpp_input_color_standards;
    pipeline_cap->num_output_color_standards = 1;
    pipeline_cap->output_color_standards = vpp_output_color_standards;

    for (i = 0; i < num_filters; i++) {
        struct object_buffer *obj_buffer = BUFFER(filters[i]);

        if (!obj_buffer ||
            !obj_buffer->buffer_store ||
            !obj_buffer->buffer_store->buffer)
            return VA_STATUS_ERROR_INVALID_BUFFER;

        VAProcFilterParameterBufferBase *base = (VAProcFilterParameterBufferBase *)obj_buffer->buffer_store->buffer;

        if (base->type == VAProcFilterNoiseReduction) {
            VAProcFilterParameterBuffer *denoise = (VAProcFilterParameterBuffer *)base;
            (void)denoise;
        } else if (base->type == VAProcFilterDeinterlacing) {
            VAProcFilterParameterBufferDeinterlacing *deint = (VAProcFilterParameterBufferDeinterlacing *)base;

            ASSERT_RET(deint->algorithm == VAProcDeinterlacingBob ||
                   deint->algorithm == VAProcDeinterlacingMotionAdaptive ||
                   deint->algorithm == VAProcDeinterlacingMotionCompensated,
                   VA_STATUS_ERROR_INVALID_PARAMETER);

            if (deint->algorithm == VAProcDeinterlacingMotionAdaptive ||
                deint->algorithm == VAProcDeinterlacingMotionCompensated)
                pipeline_cap->num_forward_references++;
        } else if (base->type == VAProcFilterSkinToneEnhancement) {
                VAProcFilterParameterBuffer *stde = (VAProcFilterParameterBuffer *)base;
                (void)stde;
        }
    }

    return VA_STATUS_SUCCESS;
}

extern struct hw_codec_info *i965_get_codec_info(int devid);

static bool
i965_driver_data_init(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    i965->codec_info = i965_get_codec_info(i965->intel.device_id);

    if (!i965->codec_info)
        return false;

    if (object_heap_init(&i965->config_heap,
                         sizeof(struct object_config),
                         CONFIG_ID_OFFSET))
        goto err_config_heap;
    if (object_heap_init(&i965->context_heap,
                         sizeof(struct object_context),
                         CONTEXT_ID_OFFSET))
        goto err_context_heap;

    if (object_heap_init(&i965->surface_heap,
                         sizeof(struct object_surface),
                         SURFACE_ID_OFFSET))
        goto err_surface_heap;
    if (object_heap_init(&i965->buffer_heap,
                         sizeof(struct object_buffer),
                         BUFFER_ID_OFFSET))
        goto err_buffer_heap;
    if (object_heap_init(&i965->image_heap,
                         sizeof(struct object_image),
                         IMAGE_ID_OFFSET))
        goto err_image_heap;
    if (object_heap_init(&i965->subpic_heap,
                         sizeof(struct object_subpic),
                         SUBPIC_ID_OFFSET))
        goto err_subpic_heap;

    i965->batch = intel_batchbuffer_new(&i965->intel, I915_EXEC_RENDER, 0);
    i965->pp_batch = intel_batchbuffer_new(&i965->intel, I915_EXEC_RENDER, 0);
    _i965InitMutex(&i965->render_mutex);
    _i965InitMutex(&i965->pp_mutex);

    return true;

err_subpic_heap:
    object_heap_destroy(&i965->image_heap);
err_image_heap:
    object_heap_destroy(&i965->buffer_heap);
err_buffer_heap:
    object_heap_destroy(&i965->surface_heap);
err_surface_heap:
    object_heap_destroy(&i965->context_heap);
err_context_heap:
    object_heap_destroy(&i965->config_heap);
err_config_heap:

    return false;
}

static void
i965_driver_data_terminate(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    _i965DestroyMutex(&i965->pp_mutex);
    _i965DestroyMutex(&i965->render_mutex);

    if (i965->batch)
        intel_batchbuffer_free(i965->batch);

    if (i965->pp_batch)
        intel_batchbuffer_free(i965->pp_batch);

    i965_destroy_heap(&i965->subpic_heap, i965_destroy_subpic);
    i965_destroy_heap(&i965->image_heap, i965_destroy_image);
    i965_destroy_heap(&i965->buffer_heap, i965_destroy_buffer);
    i965_destroy_heap(&i965->surface_heap, i965_destroy_surface);
    i965_destroy_heap(&i965->context_heap, i965_destroy_context);
    i965_destroy_heap(&i965->config_heap, i965_destroy_config);
}

struct {
    bool (*init)(VADriverContextP ctx);
    void (*terminate)(VADriverContextP ctx);
    int display_type;
} i965_sub_ops[] =  {
    {
        intel_driver_init,
        intel_driver_terminate,
        0,
    },

    {
        i965_driver_data_init,
        i965_driver_data_terminate,
        0,
    },

    {
        i965_display_attributes_init,
        i965_display_attributes_terminate,
        0,
    },

    {
        i965_post_processing_init,
        i965_post_processing_terminate,
        0,
    },

    {
        i965_render_init,
        i965_render_terminate,
        0,
    },

#ifdef HAVE_VA_WAYLAND
    {
        i965_output_wayland_init,
        i965_output_wayland_terminate,
        VA_DISPLAY_WAYLAND,
    },
#endif

#ifdef HAVE_VA_X11
    {
        i965_output_dri_init,
        i965_output_dri_terminate,
        VA_DISPLAY_X11,
    },
#endif
};

static bool
ensure_vendor_string(struct i965_driver_data *i965, const char *chipset)
{
    int ret, len;

    if (i965->va_vendor[0] != '\0')
        return true;

    len = 0;
    ret = snprintf(i965->va_vendor, sizeof(i965->va_vendor),
        "%s %s driver for %s - %d.%d.%d",
        INTEL_STR_DRIVER_VENDOR, INTEL_STR_DRIVER_NAME, chipset,
        INTEL_DRIVER_MAJOR_VERSION, INTEL_DRIVER_MINOR_VERSION,
        INTEL_DRIVER_MICRO_VERSION);
    if (ret < 0 || ret >= sizeof(i965->va_vendor))
        goto error;
    len = ret;

    if (INTEL_DRIVER_PRE_VERSION > 0) {
        ret = snprintf(&i965->va_vendor[len], sizeof(i965->va_vendor) - len,
            ".pre%d", INTEL_DRIVER_PRE_VERSION);
        if (ret < 0 || ret >= sizeof(i965->va_vendor))
            goto error;
        len += ret;

        ret = snprintf(&i965->va_vendor[len], sizeof(i965->va_vendor) - len,
            " (%s)", INTEL_DRIVER_GIT_VERSION);
        if (ret < 0 || ret >= sizeof(i965->va_vendor))
            goto error;
        len += ret;
    }
    return true;

error:
    i965->va_vendor[0] = '\0';
    ASSERT_RET(ret > 0 && len < sizeof(i965->va_vendor), false);
    return false;
}

/* Only when the option of "enable-wrapper" is passed, it is possible
 * to initialize/load the wrapper context of backend driver.
 * Otherwise it is not loaded.
 */
#if HAVE_HYBRID_CODEC

static VAStatus
i965_initialize_wrapper(VADriverContextP ctx, const char *driver_name)
{
#define DRIVER_EXTENSION	"_drv_video.so"

    struct i965_driver_data *i965 = i965_driver_data(ctx);

    VADriverContextP wrapper_pdrvctx;
    struct VADriverVTable *vtable;
    char *search_path, *driver_dir;
    char *saveptr;
    char driver_path[256];
    void *handle = NULL;
    VAStatus va_status = VA_STATUS_SUCCESS;
    bool driver_loaded = false;

    wrapper_pdrvctx = calloc(1, sizeof(*wrapper_pdrvctx));
    vtable = calloc(1, sizeof(*vtable));

    if (!wrapper_pdrvctx || !vtable) {
        fprintf(stderr, "Failed to allocate memory for wrapper \n");
        free(wrapper_pdrvctx);
        free(vtable);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    /* use the same drm_state with CTX */
    wrapper_pdrvctx->drm_state = ctx->drm_state;
    wrapper_pdrvctx->display_type = ctx->display_type;
    wrapper_pdrvctx->vtable = vtable;

    search_path = VA_DRIVERS_PATH;
    search_path = strdup((const char *)search_path);

    driver_dir = strtok_r(search_path, ":", &saveptr);
    while (driver_dir && !driver_loaded) {
        memset(driver_path, 0, sizeof(driver_path));
        sprintf(driver_path, "%s/%s%s", driver_dir, driver_name, DRIVER_EXTENSION);

        handle = dlopen(driver_path, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
        if (!handle) {
            fprintf(stderr, "failed to open %s\n", driver_path);
            driver_dir = strtok_r(NULL, ":", &saveptr);
            continue;
        }
        {
            VADriverInit init_func = NULL;
            char init_func_s[256];
            int i;

            static const struct {
                int major;
                int minor;
            } compatible_versions[] = {
                { VA_MAJOR_VERSION, VA_MINOR_VERSION },
                { 0, 37 },
                { 0, 36 },
                { 0, 35 },
                { 0, 34 },
                { 0, 33 },
                { 0, 32 },
                { -1, }
            };
            for (i = 0; compatible_versions[i].major >= 0; i++) {
                snprintf(init_func_s, sizeof(init_func_s),
                     "__vaDriverInit_%d_%d",
                     compatible_versions[i].major,
                     compatible_versions[i].minor);
                init_func = (VADriverInit)dlsym(handle, init_func_s);
                if (init_func) {
                    break;
                }
            }
            if (compatible_versions[i].major < 0) {
                dlclose(handle);
                fprintf(stderr, "%s has no function %s\n",
                            driver_path, init_func_s);
                driver_dir = strtok_r(NULL, ":", &saveptr);
                continue;
            }

            if (init_func)
                va_status = (*init_func)(wrapper_pdrvctx);

            if (va_status != VA_STATUS_SUCCESS) {
                dlclose(handle);
                fprintf(stderr, "%s init failed\n", driver_path);
                driver_dir = strtok_r(NULL, ":", &saveptr);
                continue;
            }

            wrapper_pdrvctx->handle = handle;
            driver_loaded = true;
        }
    }

    free(search_path);

    if (driver_loaded) {
        i965->wrapper_pdrvctx = wrapper_pdrvctx;
        return VA_STATUS_SUCCESS;
    } else {
        fprintf(stderr, "Failed to wrapper %s%s\n", driver_name, DRIVER_EXTENSION);
        free(vtable);
        free(wrapper_pdrvctx);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }
}
#endif

static VAStatus
i965_Init(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i;
    const char *chipset;

    for (i = 0; i < ARRAY_ELEMS(i965_sub_ops); i++) {
        if ((i965_sub_ops[i].display_type == 0 ||
             i965_sub_ops[i].display_type == (ctx->display_type & VA_DISPLAY_MAJOR_MASK)) &&
            !i965_sub_ops[i].init(ctx))
            break;
    }

    if (i == ARRAY_ELEMS(i965_sub_ops)) {
        switch (i965->intel.device_id) {
#undef CHIPSET
#define CHIPSET(id, family, dev, str) case id: chipset = str; break;
#include "i965_pciids.h"
        default:
            chipset = "Unknown Intel Chipset";
            break;
        }

        if (!ensure_vendor_string(i965, chipset))
            return VA_STATUS_ERROR_ALLOCATION_FAILED;

        i965->current_context_id = VA_INVALID_ID;

        if (i965->codec_info && i965->codec_info->preinit_hw_codec)
            i965->codec_info->preinit_hw_codec(ctx, i965->codec_info);

#if HAVE_HYBRID_CODEC
        i965_initialize_wrapper(ctx, "hybrid");
#endif

        return VA_STATUS_SUCCESS;
    } else {
        i--;

        for (; i >= 0; i--) {
            if (i965_sub_ops[i].display_type == 0 ||
                i965_sub_ops[i].display_type == (ctx->display_type & VA_DISPLAY_MAJOR_MASK)) {
                i965_sub_ops[i].terminate(ctx);
            }
        }

        return VA_STATUS_ERROR_UNKNOWN;
    }
}

VAStatus
i965_Terminate(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i;

    if (i965) {
        if (i965->wrapper_pdrvctx) {
            VADriverContextP pdrvctx;
            pdrvctx = i965->wrapper_pdrvctx;
            if (pdrvctx->handle) {
                pdrvctx->vtable->vaTerminate(pdrvctx);
                pdrvctx->handle = NULL;
            }
            free(pdrvctx->vtable);
            free(pdrvctx);
            i965->wrapper_pdrvctx = NULL;
        }

        for (i = ARRAY_ELEMS(i965_sub_ops); i > 0; i--)
            if (i965_sub_ops[i - 1].display_type == 0 ||
                i965_sub_ops[i - 1].display_type == (ctx->display_type & VA_DISPLAY_MAJOR_MASK)) {
                i965_sub_ops[i - 1].terminate(ctx);
            }

        free(i965);
        ctx->pDriverData = NULL;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DLL_EXPORT
VA_DRIVER_INIT_FUNC(VADriverContextP ctx);

VAStatus
VA_DRIVER_INIT_FUNC(  VADriverContextP ctx )
{
    struct VADriverVTable * const vtable = ctx->vtable;
    struct VADriverVTableVPP * const vtable_vpp = ctx->vtable_vpp;

    struct i965_driver_data *i965;
    VAStatus ret = VA_STATUS_ERROR_UNKNOWN;

    ctx->version_major = VA_MAJOR_VERSION;
    ctx->version_minor = VA_MINOR_VERSION;
    ctx->max_profiles = I965_MAX_PROFILES;
    ctx->max_entrypoints = I965_MAX_ENTRYPOINTS;
    ctx->max_attributes = I965_MAX_CONFIG_ATTRIBUTES;
    ctx->max_image_formats = I965_MAX_IMAGE_FORMATS;
    ctx->max_subpic_formats = I965_MAX_SUBPIC_FORMATS;
    ctx->max_display_attributes = 1 + ARRAY_ELEMS(i965_display_attributes);

    vtable->vaTerminate = i965_Terminate;
    vtable->vaQueryConfigEntrypoints = i965_QueryConfigEntrypoints;
    vtable->vaQueryConfigProfiles = i965_QueryConfigProfiles;
    vtable->vaQueryConfigAttributes = i965_QueryConfigAttributes;
    vtable->vaCreateConfig = i965_CreateConfig;
    vtable->vaDestroyConfig = i965_DestroyConfig;
    vtable->vaGetConfigAttributes = i965_GetConfigAttributes;
    vtable->vaCreateSurfaces = i965_CreateSurfaces;
    vtable->vaDestroySurfaces = i965_DestroySurfaces;
    vtable->vaCreateContext = i965_CreateContext;
    vtable->vaDestroyContext = i965_DestroyContext;
    vtable->vaCreateBuffer = i965_CreateBuffer;
    vtable->vaBufferSetNumElements = i965_BufferSetNumElements;
    vtable->vaMapBuffer = i965_MapBuffer;
    vtable->vaUnmapBuffer = i965_UnmapBuffer;
    vtable->vaDestroyBuffer = i965_DestroyBuffer;
    vtable->vaBeginPicture = i965_BeginPicture;
    vtable->vaRenderPicture = i965_RenderPicture;
    vtable->vaEndPicture = i965_EndPicture;
    vtable->vaSyncSurface = i965_SyncSurface;
    vtable->vaQuerySurfaceStatus = i965_QuerySurfaceStatus;
    vtable->vaPutSurface = i965_PutSurface;
    vtable->vaQueryImageFormats = i965_QueryImageFormats;
    vtable->vaCreateImage = i965_CreateImage;
    vtable->vaDeriveImage = i965_DeriveImage;
    vtable->vaDestroyImage = i965_DestroyImage;
    vtable->vaSetImagePalette = i965_SetImagePalette;
    vtable->vaGetImage = i965_GetImage;
    vtable->vaPutImage = i965_PutImage;
    vtable->vaQuerySubpictureFormats = i965_QuerySubpictureFormats;
    vtable->vaCreateSubpicture = i965_CreateSubpicture;
    vtable->vaDestroySubpicture = i965_DestroySubpicture;
    vtable->vaSetSubpictureImage = i965_SetSubpictureImage;
    vtable->vaSetSubpictureChromakey = i965_SetSubpictureChromakey;
    vtable->vaSetSubpictureGlobalAlpha = i965_SetSubpictureGlobalAlpha;
    vtable->vaAssociateSubpicture = i965_AssociateSubpicture;
    vtable->vaDeassociateSubpicture = i965_DeassociateSubpicture;
    vtable->vaQueryDisplayAttributes = i965_QueryDisplayAttributes;
    vtable->vaGetDisplayAttributes = i965_GetDisplayAttributes;
    vtable->vaSetDisplayAttributes = i965_SetDisplayAttributes;
    vtable->vaBufferInfo = i965_BufferInfo;
    vtable->vaLockSurface = i965_LockSurface;
    vtable->vaUnlockSurface = i965_UnlockSurface;
    vtable->vaGetSurfaceAttributes = i965_GetSurfaceAttributes;
    vtable->vaQuerySurfaceAttributes = i965_QuerySurfaceAttributes;
    vtable->vaCreateSurfaces2 = i965_CreateSurfaces2;

    /* 0.36.0 */
    vtable->vaAcquireBufferHandle = i965_AcquireBufferHandle;
    vtable->vaReleaseBufferHandle = i965_ReleaseBufferHandle;

    vtable_vpp->vaQueryVideoProcFilters = i965_QueryVideoProcFilters;
    vtable_vpp->vaQueryVideoProcFilterCaps = i965_QueryVideoProcFilterCaps;
    vtable_vpp->vaQueryVideoProcPipelineCaps = i965_QueryVideoProcPipelineCaps;

    i965 = (struct i965_driver_data *)calloc(1, sizeof(*i965));

    if (i965 == NULL) {
        ctx->pDriverData = NULL;

        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    i965->wrapper_pdrvctx = NULL;
    ctx->pDriverData = (void *)i965;
    ret = i965_Init(ctx);

    if (ret == VA_STATUS_SUCCESS) {
        ctx->str_vendor = i965->va_vendor;
    } else {
        free(i965);
        ctx->pDriverData = NULL;
    }

    return ret;
}
