
#include <drm/drmP.h>
#include <drm/radeon_drm.h>
#include "radeon.h"
#include "radeon_object.h"
#include "drm_fb_helper.h"
#include "hmm.h"
#include <display.h>

extern struct drm_framebuffer *main_fb;
extern struct drm_gem_object  *main_fb_obj;

display_t *os_display;

static cursor_t*  __stdcall select_cursor_kms(cursor_t *cursor);
static void       __stdcall move_cursor_kms(cursor_t *cursor, int x, int y);

extern int init_cursor(cursor_t *cursor);
extern void __stdcall restore_cursor(int x, int y);

int radeon_align_pitch(struct radeon_device *rdev, int width, int bpp, bool tiled);

void disable_mouse(void);

static void radeon_show_cursor_kms(struct drm_crtc *crtc)
{
    struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);
    struct radeon_device *rdev = crtc->dev->dev_private;

    if (ASIC_IS_DCE4(rdev)) {
        WREG32(RADEON_MM_INDEX, EVERGREEN_CUR_CONTROL + radeon_crtc->crtc_offset);
        WREG32(RADEON_MM_DATA, EVERGREEN_CURSOR_EN |
               EVERGREEN_CURSOR_MODE(EVERGREEN_CURSOR_24_8_PRE_MULT));
    } else if (ASIC_IS_AVIVO(rdev)) {
        WREG32(RADEON_MM_INDEX, AVIVO_D1CUR_CONTROL + radeon_crtc->crtc_offset);
        WREG32(RADEON_MM_DATA, AVIVO_D1CURSOR_EN |
                 (AVIVO_D1CURSOR_MODE_24BPP << AVIVO_D1CURSOR_MODE_SHIFT));
    } else {
        switch (radeon_crtc->crtc_id) {
        case 0:
            WREG32(RADEON_MM_INDEX, RADEON_CRTC_GEN_CNTL);
            break;
        case 1:
            WREG32(RADEON_MM_INDEX, RADEON_CRTC2_GEN_CNTL);
            break;
        default:
            return;
        }

        WREG32_P(RADEON_MM_DATA, (RADEON_CRTC_CUR_EN |
                      (RADEON_CRTC_CUR_MODE_24BPP << RADEON_CRTC_CUR_MODE_SHIFT)),
             ~(RADEON_CRTC_CUR_EN | RADEON_CRTC_CUR_MODE_MASK));
    }
}

static void radeon_lock_cursor_kms(struct drm_crtc *crtc, bool lock)
{
    struct radeon_device *rdev = crtc->dev->dev_private;
    struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);
    uint32_t cur_lock;

    if (ASIC_IS_DCE4(rdev)) {
        cur_lock = RREG32(EVERGREEN_CUR_UPDATE + radeon_crtc->crtc_offset);
        if (lock)
            cur_lock |= EVERGREEN_CURSOR_UPDATE_LOCK;
        else
            cur_lock &= ~EVERGREEN_CURSOR_UPDATE_LOCK;
        WREG32(EVERGREEN_CUR_UPDATE + radeon_crtc->crtc_offset, cur_lock);
    } else if (ASIC_IS_AVIVO(rdev)) {
        cur_lock = RREG32(AVIVO_D1CUR_UPDATE + radeon_crtc->crtc_offset);
        if (lock)
            cur_lock |= AVIVO_D1CURSOR_UPDATE_LOCK;
        else
            cur_lock &= ~AVIVO_D1CURSOR_UPDATE_LOCK;
        WREG32(AVIVO_D1CUR_UPDATE + radeon_crtc->crtc_offset, cur_lock);
    } else {
        cur_lock = RREG32(RADEON_CUR_OFFSET + radeon_crtc->crtc_offset);
        if (lock)
            cur_lock |= RADEON_CUR_LOCK;
        else
            cur_lock &= ~RADEON_CUR_LOCK;
        WREG32(RADEON_CUR_OFFSET + radeon_crtc->crtc_offset, cur_lock);
    }
}

cursor_t* __stdcall select_cursor_kms(cursor_t *cursor)
{
    struct radeon_device *rdev;
    struct radeon_crtc   *radeon_crtc;
    cursor_t *old;
    uint32_t  gpu_addr;

    rdev = (struct radeon_device *)os_display->ddev->dev_private;
    radeon_crtc = to_radeon_crtc(os_display->crtc);

    old = os_display->cursor;

    os_display->cursor = cursor;
    gpu_addr = radeon_bo_gpu_offset(cursor->cobj);

    if (ASIC_IS_DCE4(rdev)) {
        WREG32(EVERGREEN_CUR_SURFACE_ADDRESS_HIGH + radeon_crtc->crtc_offset,
               0);
        WREG32(EVERGREEN_CUR_SURFACE_ADDRESS + radeon_crtc->crtc_offset,
               gpu_addr);
    } else if (ASIC_IS_AVIVO(rdev)) {
        if (rdev->family >= CHIP_RV770)
            WREG32(R700_D1CUR_SURFACE_ADDRESS_HIGH, 0);
        WREG32(AVIVO_D1CUR_SURFACE_ADDRESS + radeon_crtc->crtc_offset, gpu_addr);
    }
    else {
        radeon_crtc->legacy_display_base_addr = gpu_addr - rdev->mc.vram_start;
        /* offset is from DISP(2)_BASE_ADDRESS */
        WREG32(RADEON_CUR_OFFSET + radeon_crtc->crtc_offset, radeon_crtc->legacy_display_base_addr);
    }

    return old;
};

void __stdcall move_cursor_kms(cursor_t *cursor, int x, int y)
{
    struct radeon_device *rdev;
    rdev = (struct radeon_device *)os_display->ddev->dev_private;
    struct drm_crtc *crtc = os_display->crtc;
    struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);

    int hot_x = cursor->hot_x;
    int hot_y = cursor->hot_y;
    int w = 32;

    radeon_lock_cursor_kms(crtc, true);

    if (ASIC_IS_DCE4(rdev)) {
        WREG32(EVERGREEN_CUR_POSITION + radeon_crtc->crtc_offset,
               (x << 16) | y);
        WREG32(EVERGREEN_CUR_HOT_SPOT + radeon_crtc->crtc_offset,
               (hot_x << 16) | hot_y);
        WREG32(EVERGREEN_CUR_SIZE + radeon_crtc->crtc_offset,
               ((w - 1) << 16) | 31);
    } else if (ASIC_IS_AVIVO(rdev)) {
        WREG32(AVIVO_D1CUR_POSITION + radeon_crtc->crtc_offset,
               (x << 16) | y);
        WREG32(AVIVO_D1CUR_HOT_SPOT + radeon_crtc->crtc_offset,
               (hot_x << 16) | hot_y);
        WREG32(AVIVO_D1CUR_SIZE + radeon_crtc->crtc_offset,
               ((w - 1) << 16) | 31);
    } else {
        if (crtc->mode.flags & DRM_MODE_FLAG_DBLSCAN)
            y *= 2;

        uint32_t  gpu_addr;
        int       xorg =0, yorg=0;

        x = x - hot_x;
        y = y - hot_y;

        if( x < 0 )
        {
            xorg = -x + 1;
            x = 0;
        }

        if( y < 0 )
        {
            yorg = -hot_y + 1;
            y = 0;
        };

        WREG32(RADEON_CUR_HORZ_VERT_OFF,
               (RADEON_CUR_LOCK | (xorg << 16) | yorg ));
        WREG32(RADEON_CUR_HORZ_VERT_POSN,
               (RADEON_CUR_LOCK | (x << 16) | y));

        gpu_addr = radeon_bo_gpu_offset(cursor->cobj);

        /* offset is from DISP(2)_BASE_ADDRESS */
        WREG32(RADEON_CUR_OFFSET,
         (gpu_addr - rdev->mc.vram_start + (yorg * 256)));
    }
    radeon_lock_cursor_kms(crtc, false);
}

static char *manufacturer_name(unsigned char *x)
{
    static char name[4];

    name[0] = ((x[0] & 0x7C) >> 2) + '@';
    name[1] = ((x[0] & 0x03) << 3) + ((x[1] & 0xE0) >> 5) + '@';
    name[2] = (x[1] & 0x1F) + '@';
    name[3] = 0;

    return name;
}

static int set_mode(struct drm_device *dev, struct drm_connector *connector,
                    struct drm_crtc *crtc, videomode_t *reqmode, bool strict)
{
    struct drm_display_mode  *mode = NULL, *tmpmode;
    struct drm_framebuffer  *fb         = NULL;
    struct drm_mode_set     set;
    const char *con_name;
    unsigned hdisplay, vdisplay;
    int ret;

    drm_modeset_lock_all(dev);

    list_for_each_entry(tmpmode, &connector->modes, head)
    {
        if( (tmpmode->hdisplay == reqmode->width)  &&
            (tmpmode->vdisplay == reqmode->height) &&
            (drm_mode_vrefresh(tmpmode) == reqmode->freq) )
        {
            mode = tmpmode;
            goto do_set;
        }
	};

    if( (mode == NULL) && (strict == false) )
    {
        list_for_each_entry(tmpmode, &connector->modes, head)
        {
            if( (tmpmode->hdisplay == reqmode->width)  &&
                (tmpmode->vdisplay == reqmode->height) )
            {
                mode = tmpmode;
                goto do_set;
            }
       };
    };

    DRM_ERROR("%s failed\n", __FUNCTION__);

    return -1;

do_set:

    con_name = connector->name;

    DRM_DEBUG_KMS("set mode %d %d: crtc %d connector %s\n",
              reqmode->width, reqmode->height, crtc->base.id,
              con_name);

    drm_mode_set_crtcinfo(mode, CRTC_INTERLACE_HALVE_V);

    hdisplay = mode->hdisplay;
    vdisplay = mode->vdisplay;

    fb = main_fb;

    fb->width  = reqmode->width;
    fb->height = reqmode->height;

    fb->pitches[0] =
    fb->pitches[1] =
    fb->pitches[2] =
    fb->pitches[3] = radeon_align_pitch(dev->dev_private, reqmode->width, 32, false) * ((32 + 1) / 8);
    fb->bits_per_pixel = 32;
    fb->depth = 24;

    crtc->enabled = true;
    os_display->crtc = crtc;

    set.crtc = crtc;
    set.x = 0;
    set.y = 0;
    set.mode = mode;
    set.connectors = &connector;
    set.num_connectors = 1;
    set.fb = fb;

    ret = drm_mode_set_config_internal(&set);

    drm_modeset_unlock_all(dev);

    select_cursor_kms(os_display->cursor);
    radeon_show_cursor_kms(crtc);

    if ( !ret )
    {
        os_display->width    = fb->width;
        os_display->height   = fb->height;
        os_display->vrefresh = drm_mode_vrefresh(mode);

        sysSetScreen(fb->width, fb->height, fb->pitches[0]);

        DRM_DEBUG_KMS("new mode %d x %d pitch %d\n",
                       fb->width, fb->height, fb->pitches[0]);
    }
    else
        DRM_ERROR("failed to set mode %d_%d on crtc %p\n",
                   fb->width, fb->height, crtc);

    return ret;
}

static int count_connector_modes(struct drm_connector* connector)
{
    struct drm_display_mode  *mode;
    int count = 0;

    list_for_each_entry(mode, &connector->modes, head)
    {
        count++;
    };
    return count;
};

static struct drm_crtc *get_possible_crtc(struct drm_device *dev, struct drm_encoder *encoder)
{
    struct drm_crtc *tmp_crtc;
    int crtc_mask = 1;

    list_for_each_entry(tmp_crtc, &dev->mode_config.crtc_list, head)
    {
        if (encoder->possible_crtcs & crtc_mask)
        {
            encoder->crtc = tmp_crtc;
            DRM_DEBUG_KMS("use CRTC %p ID %d\n", tmp_crtc, tmp_crtc->base.id);
            return tmp_crtc;
        };
        crtc_mask <<= 1;
    };
    return NULL;
};

static int choose_config(struct drm_device *dev, struct drm_connector **boot_connector,
                  struct drm_crtc **boot_crtc)
{
    struct drm_connector_helper_funcs *connector_funcs;
    struct drm_connector *connector;
    struct drm_encoder   *encoder;
    struct drm_crtc      *crtc;

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        if( connector->status != connector_status_connected)
            continue;

        encoder = connector->encoder;

        if(encoder == NULL)
        {
            connector_funcs = connector->helper_private;
            encoder = connector_funcs->best_encoder(connector);

            if( encoder == NULL)
            {
                DRM_DEBUG_KMS("CONNECTOR %x ID: %d no active encoders\n",
                        connector, connector->base.id);
                continue;
            };
        }

        crtc = encoder->crtc;
        if(crtc == NULL)
            crtc = get_possible_crtc(dev, encoder);

        if(crtc != NULL)
        {
            *boot_connector = connector;
            *boot_crtc = crtc;
            connector->encoder = encoder;
            DRM_DEBUG_KMS("CONNECTOR %p ID:%d status:%d ENCODER %p ID: %d CRTC %p ID:%d\n",
                           connector, connector->base.id, connector->status,
                           encoder, encoder->base.id, crtc, crtc->base.id );
            return 0;
        }
        else
            DRM_DEBUG_KMS("No CRTC for encoder %d\n", encoder->base.id);

    };

    return -ENOENT;
};

static int get_boot_mode(struct drm_connector *connector, videomode_t *usermode)
{
    struct drm_display_mode *mode;

    list_for_each_entry(mode, &connector->modes, head)
    {
        DRM_DEBUG_KMS("check mode w:%d h:%d %dHz\n",
                mode->hdisplay, mode->vdisplay,
                drm_mode_vrefresh(mode));

        if( os_display->width  == mode->hdisplay &&
            os_display->height == mode->vdisplay &&
            drm_mode_vrefresh(mode) == 60)
        {
            usermode->width  = os_display->width;
            usermode->height = os_display->height;
            usermode->freq   = 60;
            return 1;
        }
    }
    return 0;
}

int init_display_kms(struct drm_device *dev, videomode_t *usermode)
{
    struct drm_connector_helper_funcs *connector_funcs;
    struct drm_connector    *connector = NULL;
    struct drm_crtc         *crtc = NULL;
    struct drm_framebuffer  *fb;

    cursor_t  *cursor;
    u32        ifl;
    int        ret;

    mutex_lock(&dev->mode_config.mutex);

    ret = choose_config(dev, &connector, &crtc);
    if(ret)
    {
        DRM_DEBUG_KMS("No active connectors!\n");
        mutex_unlock(&dev->mode_config.mutex);
        return -1;
    };

    {
        struct drm_display_mode *tmp, *native = NULL;
        struct radeon_device *rdev = dev->dev_private;

        list_for_each_entry(tmp, &connector->modes, head) {
            if (tmp->hdisplay > 16384 ||
                tmp->vdisplay > 16384)
                continue;
            if (tmp->type & DRM_MODE_TYPE_PREFERRED)
            {
                native = tmp;
                break;
            };
        }

        if( ASIC_IS_AVIVO(rdev) && native )
        {
            struct radeon_encoder *radeon_encoder = to_radeon_encoder(connector->encoder);
            radeon_encoder->rmx_type = RMX_FULL;
            radeon_encoder->native_mode = *native;
        };
    }

#if 0
    mutex_lock(&dev->object_name_lock);
    idr_preload(GFP_KERNEL);

    if (!main_fb_obj->name) {
        ret = idr_alloc(&dev->object_name_idr, &main_fb_obj, 1, 0, GFP_NOWAIT);

        main_fb_obj->name = ret;

        /* Allocate a reference for the name table.  */
        drm_gem_object_reference(main_fb_obj);

        DRM_DEBUG_KMS("%s allocate fb name %d\n", __FUNCTION__, main_fb_obj->name );
    }

    idr_preload_end();
    mutex_unlock(&dev->object_name_lock);
    drm_gem_object_unreference(main_fb_obj);
#endif

    os_display = GetDisplay();
    os_display->ddev = dev;
    os_display->connector = connector;
    os_display->crtc = crtc;

    os_display->supported_modes = count_connector_modes(connector);

    ifl = safe_cli();
    {
        list_for_each_entry(cursor, &os_display->cursors, list)
        {
            init_cursor(cursor);
        };

        os_display->restore_cursor(0,0);
        os_display->init_cursor    = init_cursor;
        os_display->select_cursor  = select_cursor_kms;
        os_display->show_cursor    = NULL;
        os_display->move_cursor    = move_cursor_kms;
        os_display->restore_cursor = restore_cursor;
        os_display->disable_mouse  = disable_mouse;
        select_cursor_kms(os_display->cursor);
    };
    safe_sti(ifl);


//    dbgprintf("current mode %d x %d x %d\n",
//              os_display->width, os_display->height, os_display->vrefresh);
//    dbgprintf("user mode mode %d x %d x %d\n",
//              usermode->width, usermode->height, usermode->freq);

    if( (usermode->width == 0) ||
        (usermode->height == 0))
    {
        if( !get_boot_mode(connector, usermode))
        {
            struct drm_display_mode *mode;

            mode = list_entry(connector->modes.next, typeof(*mode), head);
            usermode->width  = mode->hdisplay;
            usermode->height = mode->vdisplay;
            usermode->freq   = drm_mode_vrefresh(mode);
        };
    };

    mutex_unlock(&dev->mode_config.mutex);

    set_mode(dev, os_display->connector, os_display->crtc, usermode, false);

    radeon_show_cursor_kms(os_display->crtc);

    return 0;
};


int get_videomodes(videomode_t *mode, int *count)
{
    int err = -1;

    if( *count == 0 )
    {
        *count = os_display->supported_modes;
        err = 0;
    }
    else if( mode != NULL )
    {
        struct drm_display_mode  *drmmode;
        int i = 0;

        if( *count > os_display->supported_modes)
            *count = os_display->supported_modes;

        list_for_each_entry(drmmode, &os_display->connector->modes, head)
        {
            if( i < *count)
            {
                mode->width  = drmmode->hdisplay;
                mode->height = drmmode->vdisplay;
                mode->bpp    = 32;
                mode->freq   = drm_mode_vrefresh(drmmode);
                i++;
                mode++;
            }
            else break;
        };
        *count = i;
        err = 0;
    };
    return err;
};

int set_user_mode(videomode_t *mode)
{
    int err = -1;

    if( (mode->width  != 0)  &&
        (mode->height != 0)  &&
        (mode->freq   != 0 ) &&
        ( (mode->width   != os_display->width)  ||
          (mode->height  != os_display->height) ||
          (mode->freq    != os_display->vrefresh) ) )
    {
        return set_mode(os_display->ddev, os_display->connector, os_display->crtc, mode, true);
    };

    return -1;
};





#if 0
typedef struct
{
    int left;
    int top;
    int right;
    int bottom;
}rect_t;

extern struct hmm bm_mm;
struct drm_device *main_device;

void  FASTCALL GetWindowRect(rect_t *rc)__asm__("GetWindowRect");

#define CURRENT_TASK             (0x80003000)

static u32_t get_display_map()
{
    u32_t   addr;

    addr = (u32_t)os_display;
    addr+= sizeof(display_t);            /*  shoot me  */
    return *(u32_t*)addr;
}

#include "clip.inc"
#include "r100d.h"

# define PACKET3_BITBLT                 0x92
# define PACKET3_TRANS_BITBLT           0x9C
# define R5XX_SRC_CMP_EQ_COLOR      (4 <<  0)
# define R5XX_SRC_CMP_NEQ_COLOR     (5 <<  0)
# define R5XX_CLR_CMP_SRC_SOURCE    (1 << 24)

int srv_blit_bitmap(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h)
{
    struct context *context;

    bitmap_t  *bitmap;
    rect_t     winrc;
    clip_t     dst_clip;
    clip_t     src_clip;
    u32_t      width;
    u32_t      height;

    u32_t      br13, cmd, slot_mask, *b;
    u32_t      offset;
    u8         slot;
    int        n=0;
    int        ret;

    if(unlikely(hbitmap==0))
        return -1;

    bitmap = (bitmap_t*)hmm_get_data(&bm_mm, hbitmap);

    if(unlikely(bitmap==NULL))
        return -1;

    context = get_context(main_drm_device);
    if(unlikely(context == NULL))
        return -1;

    GetWindowRect(&winrc);
    {
        static warn_count;

        if(warn_count < 1)
        {
            printf("left %d top %d right %d bottom %d\n",
                    winrc.left, winrc.top, winrc.right, winrc.bottom);
            printf("bitmap width %d height %d\n", w, h);
            warn_count++;
        };
    };


    dst_clip.xmin   = 0;
    dst_clip.ymin   = 0;
    dst_clip.xmax   = winrc.right-winrc.left;
    dst_clip.ymax   = winrc.bottom -winrc.top;

    src_clip.xmin   = 0;
    src_clip.ymin   = 0;
    src_clip.xmax   = bitmap->width  - 1;
    src_clip.ymax   = bitmap->height - 1;

    width  = w;
    height = h;

    if( blit_clip(&dst_clip, &dst_x, &dst_y,
                  &src_clip, &src_x, &src_y,
                  &width, &height) )
        return 0;

    dst_x+= winrc.left;
    dst_y+= winrc.top;

    slot = *((u8*)CURRENT_TASK);

    slot_mask = (u32_t)slot<<24;

    {
#if 0
#else
        u8* src_offset;
        u8* dst_offset;
        u32 color;

        u32 ifl;

        src_offset = (u8*)(src_y*bitmap->pitch + src_x*4);
        src_offset += (u32)bitmap->uaddr;

        dst_offset = (u8*)(dst_y*os_display->width + dst_x);
        dst_offset+= get_display_map();

        u32_t tmp_h = height;

      ifl = safe_cli();
        while( tmp_h--)
        {
            u32 tmp_w = width;

            u32* tmp_src = src_offset;
            u8*  tmp_dst = dst_offset;

            src_offset+= bitmap->pitch;
            dst_offset+= os_display->width;

            while( tmp_w--)
            {
                color = *tmp_src;

                if(*tmp_dst == slot)
                    color |= 0xFF000000;
                else
                    color = 0x00;

                *tmp_src = color;
                tmp_src++;
                tmp_dst++;
            };
        };
      safe_sti(ifl);
#endif
    }

    {
        static warn_count;

        if(warn_count < 1)
        {
            printf("blit width %d height %d\n",
                    width, height);
            warn_count++;
        };
    };


//    if((context->cmd_buffer & 0xFC0)==0xFC0)
//        context->cmd_buffer&= 0xFFFFF000;

//    b = (u32_t*)ALIGN(context->cmd_buffer,64);

//    offset = context->cmd_offset + ((u32_t)b & 0xFFF);


//    context->cmd_buffer+= n*4;

    struct radeon_device *rdev = main_drm_device->dev_private;
    struct radeon_ib *ib = &context->ib;

    ib->ptr[0] = PACKET0(0x15cc, 0);
    ib->ptr[1] = 0xFFFFFFFF;
    ib->ptr[2] = PACKET3(PACKET3_TRANS_BITBLT, 11);
    ib->ptr[3] =  RADEON_GMC_SRC_PITCH_OFFSET_CNTL |
                  RADEON_GMC_DST_PITCH_OFFSET_CNTL |
                  RADEON_GMC_SRC_CLIPPING |
                  RADEON_GMC_DST_CLIPPING |
                  RADEON_GMC_BRUSH_NONE |
                  (RADEON_COLOR_FORMAT_ARGB8888 << 8) |
                  RADEON_GMC_SRC_DATATYPE_COLOR |
                  RADEON_ROP3_S |
                  RADEON_DP_SRC_SOURCE_MEMORY |
                  RADEON_GMC_WR_MSK_DIS;

    ib->ptr[4] = ((bitmap->pitch/64) << 22) | (bitmap->gaddr >> 10);
    ib->ptr[5] = ((os_display->pitch/64) << 22) | (rdev->mc.vram_start >> 10);
    ib->ptr[6] = (0x1fff) | (0x1fff << 16);
    ib->ptr[7] = 0;
    ib->ptr[8] = (0x1fff) | (0x1fff << 16);

    ib->ptr[9] = R5XX_CLR_CMP_SRC_SOURCE | R5XX_SRC_CMP_EQ_COLOR;
    ib->ptr[10] = 0x00000000;
    ib->ptr[11] = 0xFFFFFFFF;

    ib->ptr[12] = (src_x << 16) | src_y;
    ib->ptr[13] = (dst_x << 16) | dst_y;
    ib->ptr[14] = (width << 16) | height;

    ib->ptr[15] = PACKET2(0);

    ib->length_dw = 16;

    ret = radeon_ib_schedule(rdev, ib, NULL);
    if (ret) {
        DRM_ERROR("radeon: failed to schedule ib (%d).\n", ret);
        goto fail;
    }

    ret = radeon_fence_wait(ib->fence, false);
    if (ret) {
        DRM_ERROR("radeon: fence wait failed (%d).\n", ret);
        goto fail;
    }

    radeon_fence_unref(&ib->fence);

fail:
    return ret;
};

#endif
