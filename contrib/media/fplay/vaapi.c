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
#include <kos32sys.h>
#include "winlib/winlib.h"
#include "fplay.h"

struct hw_profile
{
    enum AVCodecID av_codec;
    int ff_profile;
    uint64_t va_profile;
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

static int drm_fd = 0;
static struct vaapi_context *v_context;

static VASurfaceID v_surface_id[4];

#define HAS_HEVC VA_CHECK_VERSION(0, 38, 0)
#define HAS_VP9 (VA_CHECK_VERSION(0, 38, 1) && defined(FF_PROFILE_VP9_0))

#define PE(av_codec_id, ff_profile, vdp_profile)                \
    {AV_CODEC_ID_ ## av_codec_id, FF_PROFILE_ ## ff_profile,    \
     VAProfile ## vdp_profile}

static const struct hw_profile profiles[] = {
    PE(MPEG2VIDEO,  MPEG2_MAIN,         MPEG2Main),
    PE(MPEG2VIDEO,  MPEG2_SIMPLE,       MPEG2Simple),
    PE(MPEG4,       MPEG4_ADVANCED_SIMPLE, MPEG4AdvancedSimple),
    PE(MPEG4,       MPEG4_MAIN,         MPEG4Main),
    PE(MPEG4,       MPEG4_SIMPLE,       MPEG4Simple),
    PE(H264,        H264_HIGH,          H264High),
    PE(H264,        H264_MAIN,          H264Main),
    PE(H264,        H264_BASELINE,      H264Baseline),
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
    for (int n = 0; profiles[n].av_codec; n++) {
        if (profiles[n].av_codec == id)
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

VADisplay va_open_display(void)
{
    VADisplay va_dpy;

    drm_fd = get_service("DISPLAY");
    if (drm_fd == 0)
        return NULL;

    va_dpy = vaGetDisplayDRM(drm_fd);
    if (va_dpy)
        return va_dpy;

    drm_fd = 0;
    return NULL;
};

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
    vaapi->display               = display;
    vaapi->config_id             = VA_INVALID_ID;
    vaapi->context_id            = VA_INVALID_ID;

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
    VAStatus   status;
    int i;

    profiles = calloc(vaMaxNumProfiles(vaapi->display), sizeof(profiles[0]));

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
    VAStatus      status;
    int i;

    entrypoints = calloc(vaMaxNumEntrypoints(vaapi->display), sizeof(entrypoints[0]));

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

static int vaapi_init_decoder(VAProfile    profile,
                       VAEntrypoint entrypoint,
                       unsigned int picture_width,
                       unsigned int picture_height)
{
    struct vaapi_context* const vaapi = v_context;
    VAConfigAttrib attrib;
    VAConfigID  config_id = VA_INVALID_ID;
    VAContextID context_id = VA_INVALID_ID;
    VAStatus status;

ENTER();
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
                              v_surface_id,4,NULL,0);
    printf("v_surface_id_3 %x\n", v_surface_id[3]);
    if (!vaapi_check_status(status, "vaCreateSurfaces()"))
    {
        FAIL();
        return -1;
    };
    {
        VAImage vaimage;
        VABufferInfo info = {0};

        vaDeriveImage(vaapi->display,v_surface_id[0],&vaimage);
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
                             v_surface_id, 4,
                             &context_id);
    if (!vaapi_check_status(status, "vaCreateContext()"))
    {
        FAIL();
        return -1;
    };

    vaapi->config_id      = config_id;
    vaapi->context_id     = context_id;
    LEAVE();
    return 0;
}


static enum PixelFormat get_format(struct AVCodecContext *avctx,
                                   const enum AVPixelFormat *fmt)
{
    int i, profile;

    ENTER();

//    for (int i = 0; fmt[i] != AV_PIX_FMT_NONE; i++)
//        printf(" %s", av_get_pix_fmt_name(fmt[i]));

    for (i = 0; fmt[i] != PIX_FMT_NONE; i++) {
        printf("pixformat %x\n", fmt[i]);
        if (fmt[i] != AV_PIX_FMT_VAAPI_VLD)
            continue;

        switch (avctx->codec_id)
        {
        case CODEC_ID_MPEG2VIDEO:
            profile = VAProfileMPEG2Main;
            break;
        case CODEC_ID_MPEG4:
        case CODEC_ID_H263:
            profile = VAProfileMPEG4AdvancedSimple;
            break;
        case CODEC_ID_H264:
            profile = VAProfileH264High;
            break;
        case CODEC_ID_WMV3:
            profile = VAProfileVC1Main;
            break;
        case CODEC_ID_VC1:
            profile = VAProfileVC1Advanced;
            break;
        default:
            profile = -1;
            break;
        }
        if (profile >= 0) {
            if (vaapi_init_decoder(profile, VAEntrypointVLD, avctx->width, avctx->height) == 0)
            {
                avctx->hwaccel_context = v_context;
                LEAVE();
                return fmt[i]; ;
            }
        }
    }
    FAIL();
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
    struct av_surface surface = *(struct av_surface*)data;
//    VDPAUContext *ctx = opaque;

//    ctx->video_surface_destroy(surface);
    av_freep(&data);
}

static int get_buffer2(AVCodecContext *avctx, AVFrame *pic, int flags)
{
    vst_t *vst = (vst_t*)avctx->opaque;
    void *surface = (void *)(uintptr_t)v_surface_id[vst->dfx];

    pic->data[3] = surface;

    struct av_surface *avsurface;
    surface = av_malloc(sizeof(*avsurface));
    if (!surface)
        return AVERROR(ENOMEM);

    pic->buf[0] = av_buffer_create((uint8_t*)avsurface, sizeof(*avsurface),
                                     av_release_buffer, avctx,
                                     AV_BUFFER_FLAG_READONLY);
    return 0;
}

struct vaapi_context va_context_storage;

int fplay_init_context(vst_t *vst)
{
    AVCodecContext *vCtx = vst->vCtx;

    if(va_check_codec_support(vCtx->codec_id))
    {
        VADisplay dpy;

        dpy = va_open_display();
        vst->hwCtx = vaapi_init(dpy);

        if(vst->hwCtx != NULL)
        {
            for(int i = 0; i < 4; i++)
            {
                int ret;

                ret = avpicture_alloc(&vst->vframe[i].picture, AV_PIX_FMT_BGRA,
                                      vst->vCtx->width, vst->vCtx->height);
                if ( ret != 0 )
                {
                    printf("Cannot alloc video buffer\n\r");
                    return ret;
                };
                vst->vframe[i].format = AV_PIX_FMT_BGRA;
                vst->vframe[i].pts   = 0;
                vst->vframe[i].ready  = 0;
            };

            vst->hwdec         = 1;
            vCtx->opaque       = vst;
            vCtx->thread_count = 1;
            vCtx->get_format   = get_format;
            vCtx->get_buffer2  = get_buffer2;
            return 0;
        };
    };

    vst->hwdec = 0;

    for(int i = 0; i < 4; i++)
    {
        int ret;

        ret = avpicture_alloc(&vst->vframe[i].picture, vst->vCtx->pix_fmt,
                               vst->vCtx->width, vst->vCtx->height);
        if ( ret != 0 )
        {
            printf("Cannot alloc video buffer\n\r");
            return ret;
        };
        vst->vframe[i].format = vst->vCtx->pix_fmt;
        vst->vframe[i].pts    = 0;
        vst->vframe[i].ready  = 0;
    };

    return 0;
}


struct SwsContext *vacvt_ctx;

void va_convert_picture(vst_t *vst, int width, int height, AVPicture *pic)
{
    uint8_t  *src_data[4];
    int       src_linesize[4];
    VAImage vaimage;
    VAStatus status;
    uint8_t *vdata;
    struct vaapi_context* const vaapi = v_context;

    vaSyncSurface(vaapi->display,v_surface_id[vst->dfx]);

    status = vaDeriveImage(vaapi->display,v_surface_id[vst->dfx],&vaimage);
    if (!vaapi_check_status(status, "vaDeriveImage()"))
    {
        FAIL();
        return;
    };

    static int once = 2;

    if(once && vst->dfx == 0)
    {
        VABufferInfo info = {0};

        printf("vaDeriveImage: %x  fourcc: %x\n"
               "offset0: %d pitch0: %d\n"
               "offset1: %d pitch1: %d\n"
               "offset2: %d pitch2: %d\n",
                vaimage.buf, vaimage.format.fourcc,
                vaimage.offsets[0],vaimage.pitches[0],
                vaimage.offsets[1],vaimage.pitches[1],
                vaimage.offsets[2],vaimage.pitches[2]);

        info.mem_type = VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM;
        status = vaAcquireBufferHandle(vaapi->display, vaimage.buf, &info);
        if (vaapi_check_status(status, "vaAcquireBufferHandle()"))
        {
            printf("vaAcquireBufferHandle: %x type: %x\n"
                    "mem type: %x mem size: %d\n",
                    info.handle, info.type, info.mem_type, info.mem_size);

            vaReleaseBufferHandle(vaapi->display, vaimage.buf);
        }
        once--;
    };

    src_linesize[0] = vaimage.pitches[0];
    src_linesize[1] = vaimage.pitches[1];
    src_linesize[2] = vaimage.pitches[2];
    src_linesize[3] = 0;

    status = vaMapBuffer(vaapi->display,vaimage.buf,(void **)&vdata);
    if (!vaapi_check_status(status, "vaMapBuffer()"))
    {
        FAIL();
        return;
    };

//    printf("vdata: %x offset0: %d offset1: %d offset2: %d\n", vdata,
//            vaimage.offsets[0],
//            vaimage.offsets[1],
//            vaimage.offsets[2]);

    src_data[0] = vdata + vaimage.offsets[0];
    src_data[1] = vdata + vaimage.offsets[1];
    src_data[2] = vdata + vaimage.offsets[2];
    src_data[3] = 0;

    vacvt_ctx = sws_getCachedContext(vacvt_ctx, width, height, AV_PIX_FMT_NV12,
              width, height, AV_PIX_FMT_BGRA,
              SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if(vacvt_ctx == NULL)
    {
        printf("Cannot initialize the conversion context!\n");
        return ;
    };

    sws_scale(vacvt_ctx, (const uint8_t* const *)src_data, src_linesize, 0, height, pic->data, pic->linesize);

    vaUnmapBuffer (vaapi->display, vaimage.buf);
    vaDestroyImage(vaapi->display, vaimage.image_id);
}
