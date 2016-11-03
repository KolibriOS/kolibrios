#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/vaapi.h>
#include <va/va.h>
#include <va/va_drmcommon.h>
#include <va/drm/va_drm.h>
#include <alloca.h>
#include <kos32sys.h>
#include "winlib/winlib.h"
#include "fplay.h"

struct hw_profile
{
    enum AVCodecID av_codec;
    int ff_profile;
    VAProfile va_profile;
};


#define ENTER()   printf("enter %s\n",__FUNCTION__)
#define LEAVE()   printf("leave %s\n",__FUNCTION__)
#define FAIL()    printf("fail %s\n",__FUNCTION__)


#if DEBUG
# define D(x) x
# define bug printf
#else
# define D(x)
#endif

#undef  ARRAY_ELEMS
#define ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

struct va_decoder
{
    struct decoder decoder;
    VADisplay      dpy;
    void          *hwctx;
    VASurfaceID    v_surface_id[16];
};

static struct va_decoder va_decoder;

static int drm_fd = 0;
static struct vaapi_context *v_context;

#define HAS_HEVC VA_CHECK_VERSION(0, 38, 0)
#define HAS_VP9 (VA_CHECK_VERSION(0, 38, 1) && defined(FF_PROFILE_VP9_0))

#define PE(av_codec_id, ff_profile, vdp_profile)                \
    {AV_CODEC_ID_ ## av_codec_id, FF_PROFILE_ ## ff_profile,    \
     VAProfile ## vdp_profile}

static const struct hw_profile hw_profiles[] = {
    PE(MPEG2VIDEO,  MPEG2_MAIN,         MPEG2Main),
    PE(MPEG2VIDEO,  MPEG2_SIMPLE,       MPEG2Simple),
    PE(MPEG4,       MPEG4_ADVANCED_SIMPLE, MPEG4AdvancedSimple),
    PE(MPEG4,       MPEG4_MAIN,         MPEG4Main),
    PE(MPEG4,       MPEG4_SIMPLE,       MPEG4Simple),
    PE(H264,        H264_HIGH,          H264High),
    PE(H264,        H264_MAIN,          H264Main),
    PE(H264,        H264_CONSTRAINED_BASELINE, H264ConstrainedBaseline),
    PE(VC1,         VC1_ADVANCED,       VC1Advanced),
    PE(VC1,         VC1_MAIN,           VC1Main),
    PE(VC1,         VC1_SIMPLE,         VC1Simple),
    PE(WMV3,        VC1_ADVANCED,       VC1Advanced),
    PE(WMV3,        VC1_MAIN,           VC1Main),
    PE(WMV3,        VC1_SIMPLE,         VC1Simple),
#if HAS_HEVC
    PE(HEVC,        HEVC_MAIN,          HEVCMain),
    PE(HEVC,        HEVC_MAIN_10,       HEVCMain10),
#endif
#if HAS_VP9
    PE(VP9,         VP9_0,              VP9Profile0),
#endif
    {0}
};

int va_check_codec_support(enum AVCodecID id)
{
    for (int n = 0; hw_profiles[n].av_codec; n++) {
        if (hw_profiles[n].av_codec == id)
            return 1;
    }
    return 0;
}

static int vaapi_check_status(VAStatus status, const char *msg)
{
    if (status != VA_STATUS_SUCCESS) {
        fprintf(stderr, "[%s] %s: %s\n", PACKAGE_NAME, msg, vaErrorStr(status));
        return 0;
    }
    return 1;
};

static const char *string_of_VADisplayAttribType(VADisplayAttribType type)
{
    switch (type) {
#define TYPE(type) \
        case VADisplayAttrib##type: return "VADisplayAttrib" #type
        TYPE(Brightness);
        TYPE(Contrast);
        TYPE(Hue);
        TYPE(Saturation);
        TYPE(BackgroundColor);
#if !VA_CHECK_VERSION(0,34,0)
        TYPE(DirectSurface);
#endif
#if VA_CHECK_VERSION(0,32,0)
        TYPE(Rotation);
#endif
#undef TYPE
    default: break;
    }
    return "<unknown>";
}

static const char *string_of_VAProfile(VAProfile profile)
{
    switch (profile) {
#define PROFILE(profile) \
        case VAProfile##profile: return "VAProfile" #profile
        PROFILE(MPEG2Simple);
        PROFILE(MPEG2Main);
        PROFILE(MPEG4Simple);
        PROFILE(MPEG4AdvancedSimple);
        PROFILE(MPEG4Main);
#if VA_CHECK_VERSION(0,32,0)
        PROFILE(JPEGBaseline);
        PROFILE(H263Baseline);
        PROFILE(H264ConstrainedBaseline);
#endif
        PROFILE(H264Baseline);
        PROFILE(H264Main);
        PROFILE(H264High);
        PROFILE(VC1Simple);
        PROFILE(VC1Main);
        PROFILE(VC1Advanced);
#undef PROFILE
    default: break;
    }
    return "<unknown>";
}

static const char *string_of_VAEntrypoint(VAEntrypoint entrypoint)
{
    switch (entrypoint) {
#define ENTRYPOINT(entrypoint) \
        case VAEntrypoint##entrypoint: return "VAEntrypoint" #entrypoint
        ENTRYPOINT(VLD);
        ENTRYPOINT(IZZ);
        ENTRYPOINT(IDCT);
        ENTRYPOINT(MoComp);
        ENTRYPOINT(Deblocking);
#if VA_CHECK_VERSION(0,32,0)
        ENTRYPOINT(EncSlice);
        ENTRYPOINT(EncPicture);
#endif
#undef ENTRYPOINT
    default: break;
    }
    return "<unknown>";
}


void *vaapi_init(VADisplay display)
{
    struct vaapi_context *vaapi;
    int major_version, minor_version;
    int i, num_display_attrs, max_display_attrs;
    VADisplayAttribute *display_attrs = NULL;
    VAStatus status;

    if (v_context)
        return 0;

    if (!display)
        goto error;
    D(bug("VA display %p\n", display));

    status = vaInitialize(display, &major_version, &minor_version);
    if (!vaapi_check_status(status, "vaInitialize()"))
        goto error;
    D(bug("VA API version %d.%d\n", major_version, minor_version));

    max_display_attrs = vaMaxNumDisplayAttributes(display);
    display_attrs = malloc(max_display_attrs * sizeof(display_attrs[0]));
    if (!display_attrs)
        goto error;

    num_display_attrs = 0; /* XXX: workaround old GMA500 bug */
    status = vaQueryDisplayAttributes(display, display_attrs, &num_display_attrs);
    if (!vaapi_check_status(status, "vaQueryDisplayAttributes()"))
        goto error;
    D(bug("%d display attributes available\n", num_display_attrs));
    for (i = 0; i < num_display_attrs; i++) {
        VADisplayAttribute * const display_attr = &display_attrs[i];
        D(bug("  %-32s (%s/%s) min %d max %d value 0x%x\n",
              string_of_VADisplayAttribType(display_attr->type),
              (display_attr->flags & VA_DISPLAY_ATTRIB_GETTABLE) ? "get" : "---",
              (display_attr->flags & VA_DISPLAY_ATTRIB_SETTABLE) ? "set" : "---",
              display_attr->min_value,
              display_attr->max_value,
              display_attr->value));
    }

    if ((vaapi = calloc(1, sizeof(*vaapi))) == NULL)
        goto error;
    vaapi->display    = display;
    vaapi->config_id  = VA_INVALID_ID;
    vaapi->context_id = VA_INVALID_ID;

    v_context = vaapi;

    return vaapi;

error:
    free(display_attrs);
    return NULL;
}

static int has_profile(struct vaapi_context *vaapi, VAProfile profile)
{
    VAProfile *profiles;
    int        n_profiles;
    size_t size;
    VAStatus   status;
    int i;

    size = vaMaxNumProfiles(vaapi->display) * sizeof(VAProfile);

    profiles = alloca(size);
    memset(profiles, 0, size);

    status = vaQueryConfigProfiles(vaapi->display,profiles,&n_profiles);

    if (!vaapi_check_status(status, "vaQueryConfigProfiles()"))
        return 0;

    D(bug("%d profiles available\n", n_profiles));

    for (i = 0; i < n_profiles; i++)
    {
        if (profiles[i] == profile)
            return 1;
    }
    return 0;
}

static int has_entrypoint(struct vaapi_context *vaapi, VAProfile profile, VAEntrypoint entrypoint)
{
    VAEntrypoint *entrypoints;
    int           n_entrypoints;
    size_t size;
    VAStatus      status;
    int i;

    size = vaMaxNumEntrypoints(vaapi->display) * sizeof(VAEntrypoint);

    entrypoints = alloca(size);
    memset(entrypoints, 0, size);

    status = vaQueryConfigEntrypoints(vaapi->display, profile,
                                      entrypoints, &n_entrypoints);
    if (!vaapi_check_status(status, "vaQueryConfigEntrypoints()"))
        return 0;

    D(bug("%d entrypoints available for %s\n", n_entrypoints,
          string_of_VAProfile(profile)));

    for (i = 0; i < n_entrypoints; i++)
    {
        if (entrypoints[i] == entrypoint)
            return 1;
    }
    return 0;
}

static int vaapi_init_decoder(vst_t *vst,VAProfile profile,
                              VAEntrypoint entrypoint,
                              unsigned int picture_width,
                              unsigned int picture_height)
{
    struct vaapi_context* const vaapi = v_context;
    struct va_decoder *hw_decoder = (struct va_decoder*)vst->decoder;
    VAConfigAttrib attrib;
    VAConfigID  config_id = VA_INVALID_ID;
    VAContextID context_id = VA_INVALID_ID;
    VAStatus status;

ENTER();

    printf("%s profile %x width:%d height:%d\n",
            __FUNCTION__, profile, picture_width, picture_height);

    if (!vaapi)
    {
        FAIL();
        return -1;
    };

    if (!has_profile(vaapi, profile))
    {
        FAIL();
        return -1;
    };

    if (!has_entrypoint(vaapi, profile, entrypoint))
    {
        FAIL();
        return -1;
    };

    if (vaapi->context_id != VA_INVALID_ID)
        vaDestroyContext(vaapi->display, vaapi->context_id);

    if (hw_decoder->decoder.has_surfaces)
    {
        vaDestroySurfaces(vaapi->display,hw_decoder->v_surface_id,hw_decoder->decoder.nframes);
        hw_decoder->decoder.has_surfaces = 0;
    }

    if (vaapi->config_id != VA_INVALID_ID)
        vaDestroyConfig(vaapi->display, vaapi->config_id);


    attrib.type = VAConfigAttribRTFormat;

    printf("vaGetConfigAttributes\n");
    status = vaGetConfigAttributes(vaapi->display, profile, entrypoint,
                                   &attrib, 1);
    if (!vaapi_check_status(status, "vaGetConfigAttributes()"))
    {
        FAIL();
        return -1;
    }

    if ((attrib.value & VA_RT_FORMAT_YUV420) == 0)
    {
        printf("Chroma format not supported.\n");
        FAIL();
        return -1;
    };

    printf("vaCreateConfig\n");
    status = vaCreateConfig(vaapi->display, profile, entrypoint,
                            &attrib, 1, &config_id);
    if (!vaapi_check_status(status, "vaCreateConfig()"))
    {
        FAIL();
        return -1;
    }

    printf("vaCreateSurfaces %dx%d\n",picture_width,picture_height);
    status = vaCreateSurfaces(vaapi->display, VA_RT_FORMAT_YUV420, picture_width, picture_height,
                              hw_decoder->v_surface_id,hw_decoder->decoder.nframes,NULL,0);
    if (!vaapi_check_status(status, "vaCreateSurfaces()"))
    {
        FAIL();
        return -1;
    };

    hw_decoder->decoder.has_surfaces = 1;

    {
        VAImage vaimage;
        VABufferInfo info = {0};

        vaDeriveImage(vaapi->display,hw_decoder->v_surface_id[0],&vaimage);
        printf("vaDeriveImage: %x  fourcc: %x\n"
               "offset0: %d pitch0: %d\n"
               "offset1: %d pitch1: %d\n"
               "offset2: %d pitch2: %d\n",
                vaimage.buf, vaimage.format.fourcc,
                vaimage.offsets[0],vaimage.pitches[0],
                vaimage.offsets[1],vaimage.pitches[1],
                vaimage.offsets[2],vaimage.pitches[2]);

        info.mem_type = VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM;
        vaAcquireBufferHandle(vaapi->display, vaimage.buf, &info);
        printf("vaAcquireBufferHandle: %x type: %x\n"
                "mem type: %x mem size: %x\n",
                info.handle, info.type, info.mem_type, info.mem_size);

        vaReleaseBufferHandle(vaapi->display, vaimage.buf);

        vaDestroyImage(vaapi->display,vaimage.image_id);
    };

    printf("vaCreateContext %dx%d\n",picture_width,picture_height);
    status = vaCreateContext(vaapi->display, config_id,
                             picture_width, picture_height,
                             VA_PROGRESSIVE,
                             hw_decoder->v_surface_id, vst->decoder->nframes,
                             &context_id);
    if (!vaapi_check_status(status, "vaCreateContext()"))
    {
        FAIL();
        return -1;
    };

    vaapi->config_id  = config_id;
    vaapi->context_id = context_id;
    LEAVE();
    return 0;
}


static enum PixelFormat get_format(struct AVCodecContext *avctx,
                                   const enum AVPixelFormat *fmt)
{
    vst_t *vst = (vst_t*)avctx->opaque;
    struct va_decoder* hw_decoder = (struct va_decoder*)vst->decoder;
    struct decoder* decoder = &hw_decoder->decoder;
    VAProfile profile = avctx->profile;
    enum AVCodecID codec = avctx->codec_id;

    if(avctx->hwaccel_context != NULL)
    {
        if(decoder->codec_id != avctx->codec_id ||
           decoder->profile  != avctx->profile)
    {
        printf("\n%s codec changed!!!\n"
               "old id %d profile %x new id %d profile %x\n",
                    __FUNCTION__, decoder->codec_id, decoder->profile,
                codec, profile);

        for(int i = 0; i < decoder->nframes; i++)
        {
            vframe_t *vframe = &decoder->vframes[i];
            vframe->format   = AV_PIX_FMT_NONE;
        };
    }
        else
            return AV_PIX_FMT_VAAPI_VLD;
    }

    if (codec == AV_CODEC_ID_H264)
    {
        if(profile == FF_PROFILE_H264_BASELINE)
            profile = FF_PROFILE_H264_CONSTRAINED_BASELINE;
    };

    printf("\n%s codec %d profile %x\n", __FUNCTION__,avctx->codec_id, avctx->profile);

    for (int i = 0; fmt[i] != PIX_FMT_NONE; i++)
    {
        if (fmt[i] != AV_PIX_FMT_VAAPI_VLD)
            continue;

        for (int n = 0; hw_profiles[n].av_codec; n++)
        {
            if (hw_profiles[n].av_codec   == codec &&
                hw_profiles[n].ff_profile == profile)
            {
                profile = hw_profiles[n].va_profile;
                if (vaapi_init_decoder(vst, profile, VAEntrypointVLD, avctx->width, avctx->height) == 0)
                {
                    avctx->hwaccel_context = v_context;
                    printf("%s format: %x\n",__FUNCTION__, fmt[i]);
                    return fmt[i];
                }
            }
        }

    }
    printf("%s format PIX_FMT_NONE\n",__FUNCTION__);
    return PIX_FMT_NONE;
}

struct av_surface
{
    int         w;
    int         h;
    VASurfaceID id;
};

static void av_release_buffer(void *opaque, uint8_t *data)
{
}

static int get_buffer2(AVCodecContext *avctx, AVFrame *pic, int flags)
{
    static struct av_surface avsurface;
    vst_t *vst = (vst_t*)avctx->opaque;
    struct va_decoder* hw_decoder = (struct va_decoder*)vst->decoder;
    void *surface;

    surface = (void *)(uintptr_t)hw_decoder->v_surface_id[vst->decoder->active_frame->index];

    pic->data[3] = surface;

    pic->buf[0] = av_buffer_create((uint8_t*)&avsurface, sizeof(avsurface),
                                    av_release_buffer, avctx,
                                    AV_BUFFER_FLAG_READONLY);
    return 0;
}


#define EGL_TEXTURE_Y_U_V_WL            0x31D7
#define EGL_TEXTURE_Y_UV_WL             0x31D8
#define EGL_TEXTURE_Y_XUXV_WL           0x31D9

enum wl_drm_format {
    WL_DRM_FORMAT_C8 = 0x20203843,
    WL_DRM_FORMAT_RGB332 = 0x38424752,
    WL_DRM_FORMAT_BGR233 = 0x38524742,
    WL_DRM_FORMAT_XRGB4444 = 0x32315258,
    WL_DRM_FORMAT_XBGR4444 = 0x32314258,
    WL_DRM_FORMAT_RGBX4444 = 0x32315852,
    WL_DRM_FORMAT_BGRX4444 = 0x32315842,
    WL_DRM_FORMAT_ARGB4444 = 0x32315241,
    WL_DRM_FORMAT_ABGR4444 = 0x32314241,
    WL_DRM_FORMAT_RGBA4444 = 0x32314152,
    WL_DRM_FORMAT_BGRA4444 = 0x32314142,
    WL_DRM_FORMAT_XRGB1555 = 0x35315258,
    WL_DRM_FORMAT_XBGR1555 = 0x35314258,
    WL_DRM_FORMAT_RGBX5551 = 0x35315852,
    WL_DRM_FORMAT_BGRX5551 = 0x35315842,
    WL_DRM_FORMAT_ARGB1555 = 0x35315241,
    WL_DRM_FORMAT_ABGR1555 = 0x35314241,
    WL_DRM_FORMAT_RGBA5551 = 0x35314152,
    WL_DRM_FORMAT_BGRA5551 = 0x35314142,
    WL_DRM_FORMAT_RGB565 = 0x36314752,
    WL_DRM_FORMAT_BGR565 = 0x36314742,
    WL_DRM_FORMAT_RGB888 = 0x34324752,
    WL_DRM_FORMAT_BGR888 = 0x34324742,
    WL_DRM_FORMAT_XRGB8888 = 0x34325258,
    WL_DRM_FORMAT_XBGR8888 = 0x34324258,
    WL_DRM_FORMAT_RGBX8888 = 0x34325852,
    WL_DRM_FORMAT_BGRX8888 = 0x34325842,
    WL_DRM_FORMAT_ARGB8888 = 0x34325241,
    WL_DRM_FORMAT_ABGR8888 = 0x34324241,
    WL_DRM_FORMAT_RGBA8888 = 0x34324152,
    WL_DRM_FORMAT_BGRA8888 = 0x34324142,
    WL_DRM_FORMAT_XRGB2101010 = 0x30335258,
    WL_DRM_FORMAT_XBGR2101010 = 0x30334258,
    WL_DRM_FORMAT_RGBX1010102 = 0x30335852,
    WL_DRM_FORMAT_BGRX1010102 = 0x30335842,
    WL_DRM_FORMAT_ARGB2101010 = 0x30335241,
    WL_DRM_FORMAT_ABGR2101010 = 0x30334241,
    WL_DRM_FORMAT_RGBA1010102 = 0x30334152,
    WL_DRM_FORMAT_BGRA1010102 = 0x30334142,
    WL_DRM_FORMAT_YUYV = 0x56595559,
    WL_DRM_FORMAT_YVYU = 0x55595659,
    WL_DRM_FORMAT_UYVY = 0x59565955,
    WL_DRM_FORMAT_VYUY = 0x59555956,
    WL_DRM_FORMAT_AYUV = 0x56555941,
    WL_DRM_FORMAT_NV12 = 0x3231564e,
    WL_DRM_FORMAT_NV21 = 0x3132564e,
    WL_DRM_FORMAT_NV16 = 0x3631564e,
    WL_DRM_FORMAT_NV61 = 0x3136564e,
    WL_DRM_FORMAT_YUV410 = 0x39565559,
    WL_DRM_FORMAT_YVU410 = 0x39555659,
    WL_DRM_FORMAT_YUV411 = 0x31315559,
    WL_DRM_FORMAT_YVU411 = 0x31315659,
    WL_DRM_FORMAT_YUV420 = 0x32315559,
    WL_DRM_FORMAT_YVU420 = 0x32315659,
    WL_DRM_FORMAT_YUV422 = 0x36315559,
    WL_DRM_FORMAT_YVU422 = 0x36315659,
    WL_DRM_FORMAT_YUV444 = 0x34325559,
    WL_DRM_FORMAT_YVU444 = 0x34325659,
};

void va_create_planar(vst_t *vst, vframe_t *vframe)
{
    struct vaapi_context* const vaapi = v_context;
    struct va_decoder* hw_decoder = (struct va_decoder*)vst->decoder;
    VABufferInfo info = {0};

    VAImage vaimage;
    VAStatus status;
    planar_t *planar;

    vaSyncSurface(vaapi->display,hw_decoder->v_surface_id[vframe->index]);

    if(vframe->format != AV_PIX_FMT_NONE)
        return;

    if(vframe->planar != NULL)
    {
        pxDestroyPlanar(vframe->planar);
        vframe->planar = NULL;
    };

    status = vaDeriveImage(vaapi->display,hw_decoder->v_surface_id[vframe->index],&vaimage);
    if (!vaapi_check_status(status, "vaDeriveImage()"))
    {
        FAIL();
        return;
    };
/*
    printf("vaDeriveImage: %x  fourcc: %x\n"
           "offset0: %d pitch0: %d\n"
           "offset1: %d pitch1: %d\n"
           "offset2: %d pitch2: %d\n",
            vaimage.buf, vaimage.format.fourcc,
            vaimage.offsets[0],vaimage.pitches[0],
            vaimage.offsets[1],vaimage.pitches[1],
            vaimage.offsets[2],vaimage.pitches[2]);
*/
    info.mem_type = VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM;
    status = vaAcquireBufferHandle(vaapi->display, vaimage.buf, &info);
    if (!vaapi_check_status(status, "vaAcquireBufferHandle()"))
    {
        vaDestroyImage(vaapi->display, vaimage.image_id);
        FAIL();
        return;
    };
/*
    printf("vaAcquireBufferHandle: %x type: %x\n"
            "mem type: %x mem size: %d\n",
            info.handle, info.type, info.mem_type, info.mem_size);
*/
    planar = pxCreatePlanar(info.handle, WL_DRM_FORMAT_NV12,
                            vaimage.width, vaimage.height,
                            vaimage.offsets[0],vaimage.pitches[0],
                            vaimage.offsets[1],vaimage.pitches[1],
                            vaimage.offsets[2],vaimage.pitches[2]);
    if(planar != NULL)
    {
        vframe->planar = planar;
        vframe->format = AV_PIX_FMT_NV12;
    };

    vaReleaseBufferHandle(vaapi->display, vaimage.buf);
    vaDestroyImage(vaapi->display, vaimage.image_id);

}

static enum AVCodecID profile_to_codec(VAProfile profile)
{
    enum AVCodecID id;

    switch(profile)
    {
        case VAProfileMPEG2Simple:
        case VAProfileMPEG2Main:
            id = AV_CODEC_ID_MPEG2VIDEO;
            break;

        case VAProfileMPEG4Simple:
        case VAProfileMPEG4AdvancedSimple:
        case VAProfileMPEG4Main:
            id = AV_CODEC_ID_MPEG4;
            break;

        case VAProfileH264Baseline:
        case VAProfileH264Main:
        case VAProfileH264High:
        case VAProfileH264ConstrainedBaseline:
        case VAProfileH264MultiviewHigh:
        case VAProfileH264StereoHigh:
            id = AV_CODEC_ID_H264;
            break;

        case VAProfileVC1Simple:
        case VAProfileVC1Main:
        case VAProfileVC1Advanced:
            id = AV_CODEC_ID_VC1;
            break;

        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
            id = AV_CODEC_ID_HEVC;
            break;

        default:
            id = AV_CODEC_ID_NONE;
    };
    return id;
};

static VAProfile get_profile(VADisplay dpy, enum AVCodecID codec_id)
{
    VAProfile profile = VAProfileNone, *profile_list = NULL;
    int num_profiles, max_num_profiles;
    enum AVCodecID ff_id;
    VAStatus va_status;
    int i;

    max_num_profiles = vaMaxNumProfiles(dpy);

    profile_list = alloca(max_num_profiles * sizeof(VAProfile));
    if (!profile_list)
    {
        printf("Failed to allocate memory for profile list\n");
        goto err_0;
    }

    va_status = vaQueryConfigProfiles(dpy, profile_list, &num_profiles);
    if(!vaapi_check_status(va_status, "vaQueryConfigProfiles()"))
        goto err_0;

    if(codec_id == AV_CODEC_ID_H263)
        ff_id = AV_CODEC_ID_H264;
    else if(codec_id == AV_CODEC_ID_WMV3)
        ff_id = AV_CODEC_ID_VC1;
    else
        ff_id = codec_id;

    for (i = 0; i < num_profiles; i++)
    {
        if(ff_id == profile_to_codec(profile_list[i]))
        {
            profile = profile_list[i];
            break;
        }
    };

err_0:
    return profile;
}

static void fini_va_decoder(vst_t *vst)
{
    struct vaapi_context* const vaapi = v_context;
    struct va_decoder *hw_decoder = (struct va_decoder*)vst->decoder;
ENTER();
    for(int i = 0; i < hw_decoder->decoder.nframes; i++)
    {
        vframe_t *vframe = &hw_decoder->decoder.vframes[i];
        if(vframe->planar != NULL)
        {
            printf("destroy planar %d\n", i);
            pxDestroyPlanar(vframe->planar);
            vframe->planar = NULL;
        };
    };

    av_frame_free(&hw_decoder->decoder.Frame);

    if (vaapi->context_id != VA_INVALID_ID)
        vaDestroyContext(vaapi->display, vaapi->context_id);

    if (hw_decoder->decoder.has_surfaces)
        vaDestroySurfaces(vaapi->display,hw_decoder->v_surface_id,hw_decoder->decoder.nframes);

    if (vaapi->config_id != VA_INVALID_ID)
        vaDestroyConfig(vaapi->display, vaapi->config_id);

    vaTerminate(hw_decoder->dpy);
LEAVE();
};


struct decoder* init_va_decoder(vst_t *vst)
{
    AVCodecContext *vCtx = vst->vCtx;
    struct va_decoder *hw_decoder = &va_decoder;
    struct decoder *decoder = &hw_decoder->decoder;

    drm_fd = get_service("DISPLAY");
    if (drm_fd == 0)
        return NULL;

    hw_decoder->dpy = vaGetDisplayDRM(drm_fd);
    if (hw_decoder->dpy == NULL)
        goto err_0;

    hw_decoder->hwctx = vaapi_init(hw_decoder->dpy);
    if(hw_decoder->hwctx == NULL)
        goto err_1;

    if(get_profile(hw_decoder->dpy, vCtx->codec_id) == VAProfileNone)
        goto err_1;

    decoder->Frame = av_frame_alloc();
    if(decoder->Frame == NULL)
        goto err_1;

    if(vCtx->codec_id == AV_CODEC_ID_H264)
        decoder->nframes = 16;
    else
        decoder->nframes = 4;

    for(int i = 0; i < decoder->nframes; i++)
    {
        vframe_t *vframe = &decoder->vframes[i];

        vframe->format    = AV_PIX_FMT_NONE;
        vframe->is_hw_pic = 1;
        vframe->index     = i;
        list_add_tail(&vframe->list, &vst->input_list);
    };

    vCtx->opaque       = vst;
    vCtx->thread_count = 1;
    vCtx->get_format   = get_format;
    vCtx->get_buffer2  = get_buffer2;

    if(avcodec_open2(vst->vCtx, vst->vCodec, NULL) < 0)
    {
        printf("Error while opening codec for input stream %d\n",
                vst->vStream);
        goto err_2;
    };

    decoder->name     = vst->vCodec->name;
    decoder->codec_id = vCtx->codec_id;
    decoder->profile  = vCtx->profile;
    decoder->pix_fmt  = vCtx->pix_fmt;
    decoder->width    = vCtx->width;
    decoder->height   = vCtx->height;
    decoder->is_hw    = 1;
    decoder->frame_reorder = 1;
    decoder->fini     = fini_va_decoder;

    return (struct decoder*)decoder;

err_2:
    av_frame_free(&decoder->Frame);
err_1:
    vaTerminate(hw_decoder->dpy);
err_0:
    drm_fd = 0;
    return NULL;
}
