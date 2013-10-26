
#define iowrite32(v, addr)      writel((v), (addr))

#include "drmP.h"
#include "drm.h"
#include "i915_drm.h"
#include "i915_drv.h"
#include "intel_drv.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <errno-base.h>
#include <linux/pci.h>

#include <syscall.h>

#include "bitmap.h"

typedef struct
{
    kobj_t     header;

    uint32_t  *data;
    uint32_t   hot_x;
    uint32_t   hot_y;

    struct list_head   list;
    struct drm_i915_gem_object  *cobj;
}cursor_t;

#define CURSOR_WIDTH 64
#define CURSOR_HEIGHT 64


struct tag_display
{
    int  x;
    int  y;
    int  width;
    int  height;
    int  bpp;
    int  vrefresh;
    int  pitch;
    int  lfb;

    int  supported_modes;
    struct drm_device    *ddev;
    struct drm_connector *connector;
    struct drm_crtc      *crtc;

    struct list_head   cursors;

    cursor_t   *cursor;
    int       (*init_cursor)(cursor_t*);
    cursor_t* (__stdcall *select_cursor)(cursor_t*);
    void      (*show_cursor)(int show);
    void      (__stdcall *move_cursor)(cursor_t *cursor, int x, int y);
    void      (__stdcall *restore_cursor)(int x, int y);
    void      (*disable_mouse)(void);
    u32  mask_seqno;
    u32  check_mouse;
    u32  check_m_pixel;

};


static display_t *os_display;

u32_t cmd_buffer;
u32_t cmd_offset;

void init_render();
int  sna_init();

int init_cursor(cursor_t *cursor);
static cursor_t*  __stdcall select_cursor_kms(cursor_t *cursor);
static void       __stdcall move_cursor_kms(cursor_t *cursor, int x, int y);

void __stdcall restore_cursor(int x, int y)
{};

void disable_mouse(void)
{};

static char *manufacturer_name(unsigned char *x)
{
    static char name[4];

    name[0] = ((x[0] & 0x7C) >> 2) + '@';
    name[1] = ((x[0] & 0x03) << 3) + ((x[1] & 0xE0) >> 5) + '@';
    name[2] = (x[1] & 0x1F) + '@';
    name[3] = 0;

    return name;
}

bool set_mode(struct drm_device *dev, struct drm_connector *connector,
              videomode_t *reqmode, bool strict)
{
    drm_i915_private_t      *dev_priv   = dev->dev_private;
    struct drm_fb_helper    *fb_helper  = &dev_priv->fbdev->helper;

    struct drm_mode_config  *config     = &dev->mode_config;
    struct drm_display_mode *mode       = NULL, *tmpmode;
    struct drm_framebuffer  *fb         = NULL;
    struct drm_crtc         *crtc;
    struct drm_encoder      *encoder;
    struct drm_mode_set     set;
    char *con_name;
    char *enc_name;
    unsigned hdisplay, vdisplay;
    int ret;

    mutex_lock(&dev->mode_config.mutex);

    list_for_each_entry(tmpmode, &connector->modes, head)
    {
        if( (drm_mode_width(tmpmode)    == reqmode->width)  &&
            (drm_mode_height(tmpmode)   == reqmode->height) &&
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
            if( (drm_mode_width(tmpmode)  == reqmode->width)  &&
                (drm_mode_height(tmpmode) == reqmode->height) )
            {
                mode = tmpmode;
                goto do_set;
            }
        };
    };

    DRM_ERROR("%s failed\n", __FUNCTION__);

    return -1;

do_set:

    encoder = connector->encoder;
    crtc = encoder->crtc;

    con_name = drm_get_connector_name(connector);
    enc_name = drm_get_encoder_name(encoder);

    DRM_DEBUG_KMS("set mode %d %d: crtc %d connector %s encoder %s\n",
              reqmode->width, reqmode->height, crtc->base.id,
              con_name, enc_name);

    drm_mode_set_crtcinfo(mode, CRTC_INTERLACE_HALVE_V);

    hdisplay = mode->hdisplay;
    vdisplay = mode->vdisplay;

    if (crtc->invert_dimensions)
        swap(hdisplay, vdisplay);

    fb = fb_helper->fb;

    fb->width  = reqmode->width;
    fb->height = reqmode->height;
    fb->pitches[0]  = ALIGN(reqmode->width * 4, 64);
    fb->pitches[1]  = ALIGN(reqmode->width * 4, 64);
    fb->pitches[2]  = ALIGN(reqmode->width * 4, 64);
    fb->pitches[3]  = ALIGN(reqmode->width * 4, 64);

    fb->bits_per_pixel = 32;
    fb->depth = 24;

    crtc->fb = fb;
    crtc->enabled = true;
    os_display->crtc = crtc;

    set.crtc = crtc;
    set.x = 0;
    set.y = 0;
    set.mode = mode;
    set.connectors = &connector;
    set.num_connectors = 1;
    set.fb = fb;
    ret = crtc->funcs->set_config(&set);
    mutex_unlock(&dev->mode_config.mutex);

    if ( !ret )
    {
        os_display->width    = fb->width;
        os_display->height   = fb->height;
        os_display->pitch    = fb->pitches[0];
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

static struct drm_connector* get_def_connector(struct drm_device *dev)
{
    struct drm_connector  *connector;
    struct drm_connector_helper_funcs *connector_funcs;

    struct drm_connector  *def_connector = NULL;

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        struct drm_encoder  *encoder;
        struct drm_crtc     *crtc;

        if( connector->status != connector_status_connected)
            continue;

        connector_funcs = connector->helper_private;
        encoder = connector_funcs->best_encoder(connector);
        if( encoder == NULL)
            continue;

        connector->encoder = encoder;

        crtc = encoder->crtc;

        DRM_DEBUG_KMS("CONNECTOR %x ID:  %d status %d encoder %x\n crtc %x",
                   connector, connector->base.id,
                   connector->status, connector->encoder,
                   crtc);

//        if (crtc == NULL)
//            continue;

        def_connector = connector;

        break;
    };

    return def_connector;
};


int init_display_kms(struct drm_device *dev)
{
    struct drm_connector    *connector;
    struct drm_connector_helper_funcs *connector_funcs;
    struct drm_encoder      *encoder;
    struct drm_crtc         *crtc = NULL;
    struct drm_framebuffer  *fb;

    cursor_t  *cursor;
    u32_t      ifl;
    int        err;

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        if( connector->status != connector_status_connected)
            continue;

        connector_funcs = connector->helper_private;
        encoder = connector_funcs->best_encoder(connector);
        if( encoder == NULL)
        {
            DRM_DEBUG_KMS("CONNECTOR %x ID: %d no active encoders\n",
                      connector, connector->base.id);
            continue;
        }
        connector->encoder = encoder;
        crtc = encoder->crtc;

        DRM_DEBUG_KMS("CONNECTOR %x ID:%d status:%d ENCODER %x CRTC %x ID:%d\n",
               connector, connector->base.id,
               connector->status, connector->encoder,
               crtc, crtc->base.id );

        break;
    };

    if(connector == NULL)
    {
        DRM_ERROR("No active connectors!\n");
        return -1;
    };

    if(crtc == NULL)
    {
        struct drm_crtc *tmp_crtc;
        int crtc_mask = 1;

        list_for_each_entry(tmp_crtc, &dev->mode_config.crtc_list, head)
        {
            if (encoder->possible_crtcs & crtc_mask)
            {
                crtc = tmp_crtc;
                encoder->crtc = crtc;
                break;
            };
            crtc_mask <<= 1;
        };
    };

    if(crtc == NULL)
    {
        DRM_ERROR("No CRTC for encoder %d\n", encoder->base.id);
        return -1;
    };


    DRM_DEBUG_KMS("[Select CRTC:%d]\n", crtc->base.id);

    os_display = GetDisplay();
    os_display->ddev = dev;
    os_display->connector = connector;
    os_display->crtc = crtc;

    os_display->supported_modes = count_connector_modes(connector);


    ifl = safe_cli();
    {
        struct intel_crtc *intel_crtc = to_intel_crtc(os_display->crtc);

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

        intel_crtc->cursor_x = os_display->width/2;
        intel_crtc->cursor_y = os_display->height/2;

        select_cursor_kms(os_display->cursor);
    };
    safe_sti(ifl);

#ifdef __HWA__
    err = init_bitmaps();
#endif

    return 0;
};


int get_videomodes(videomode_t *mode, int *count)
{
    int err = -1;

//    dbgprintf("mode %x count %d\n", mode, *count);

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
                mode->width  = drm_mode_width(drmmode);
                mode->height = drm_mode_height(drmmode);
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

//    dbgprintf("width %d height %d vrefresh %d\n",
//               mode->width, mode->height, mode->freq);

    if( (mode->width  != 0)  &&
        (mode->height != 0)  &&
        (mode->freq   != 0 ) &&
        ( (mode->width   != os_display->width)  ||
          (mode->height  != os_display->height) ||
          (mode->freq    != os_display->vrefresh) ) )
    {
        if( set_mode(os_display->ddev, os_display->connector, mode, true) )
            err = 0;
    };

    return err;
};

void __attribute__((regparm(1))) destroy_cursor(cursor_t *cursor)
{
    list_del(&cursor->list);

    i915_gem_object_unpin(cursor->cobj);

    mutex_lock(&main_device->struct_mutex);
    drm_gem_object_unreference(&cursor->cobj->base);
    mutex_unlock(&main_device->struct_mutex);

    __DestroyObject(cursor);
};

int init_cursor(cursor_t *cursor)
{
    struct drm_i915_private *dev_priv = os_display->ddev->dev_private;
    struct drm_i915_gem_object *obj;
    uint32_t *bits;
    uint32_t *src;
    void     *mapped;

    int       i,j;
    int       ret;

    if (dev_priv->info->cursor_needs_physical)
    {
        bits = (uint32_t*)KernelAlloc(CURSOR_WIDTH*CURSOR_HEIGHT*4);
        if (unlikely(bits == NULL))
            return ENOMEM;
        cursor->cobj = (struct drm_i915_gem_object *)GetPgAddr(bits);
    }
    else
    {
        obj = i915_gem_alloc_object(os_display->ddev, CURSOR_WIDTH*CURSOR_HEIGHT*4);
        if (unlikely(obj == NULL))
            return -ENOMEM;

        ret = i915_gem_obj_ggtt_pin(obj, CURSOR_WIDTH*CURSOR_HEIGHT*4, true, true);
        if (ret) {
            drm_gem_object_unreference(&obj->base);
            return ret;
        }

        ret = i915_gem_object_set_to_gtt_domain(obj, true);
        if (ret)
        {
            i915_gem_object_unpin(obj);
            drm_gem_object_unreference(&obj->base);
            return ret;
        }
/* You don't need to worry about fragmentation issues.
 * GTT space is continuous. I guarantee it.                           */

        mapped = bits = (u32*)MapIoMem(dev_priv->gtt.mappable_base + i915_gem_obj_ggtt_offset(obj),
                    CURSOR_WIDTH*CURSOR_HEIGHT*4, PG_SW);

        if (unlikely(bits == NULL))
        {
            i915_gem_object_unpin(obj);
            drm_gem_object_unreference(&obj->base);
            return -ENOMEM;
        };
        cursor->cobj = obj;
    };

    src = cursor->data;

    for(i = 0; i < 32; i++)
    {
        for(j = 0; j < 32; j++)
            *bits++ = *src++;
        for(j = 32; j < CURSOR_WIDTH; j++)
            *bits++ = 0;
    }
    for(i = 0; i < CURSOR_WIDTH*(CURSOR_HEIGHT-32); i++)
        *bits++ = 0;

    FreeKernelSpace(mapped);

// release old cursor

    KernelFree(cursor->data);

    cursor->data = bits;

    cursor->header.destroy = destroy_cursor;

    return 0;
}


static void i9xx_update_cursor(struct drm_crtc *crtc, u32 base)
{
    struct drm_device *dev = crtc->dev;
    struct drm_i915_private *dev_priv = dev->dev_private;
    struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
    int pipe = intel_crtc->pipe;
    bool visible = base != 0;

    if (intel_crtc->cursor_visible != visible) {
        uint32_t cntl = I915_READ(CURCNTR(pipe));
        if (base) {
            cntl &= ~(CURSOR_MODE | MCURSOR_PIPE_SELECT);
            cntl |= CURSOR_MODE_64_ARGB_AX | MCURSOR_GAMMA_ENABLE;
            cntl |= pipe << 28; /* Connect to correct pipe */
        } else {
            cntl &= ~(CURSOR_MODE | MCURSOR_GAMMA_ENABLE);
            cntl |= CURSOR_MODE_DISABLE;
        }
        I915_WRITE(CURCNTR(pipe), cntl);

        intel_crtc->cursor_visible = visible;
    }
    /* and commit changes on next vblank */
    I915_WRITE(CURBASE(pipe), base);
}

void __stdcall move_cursor_kms(cursor_t *cursor, int x, int y)
{
    struct drm_i915_private *dev_priv = os_display->ddev->dev_private;
    struct intel_crtc *intel_crtc = to_intel_crtc(os_display->crtc);
    u32 base, pos;
    bool visible;

    int pipe = intel_crtc->pipe;

    intel_crtc->cursor_x = x;
    intel_crtc->cursor_y = y;

    x = x - cursor->hot_x;
    y = y - cursor->hot_y;


    pos = 0;

    base = intel_crtc->cursor_addr;
    if (x >= os_display->width)
        base = 0;

    if (y >= os_display->height)
        base = 0;

    if (x < 0)
    {
        if (x + intel_crtc->cursor_width < 0)
            base = 0;

        pos |= CURSOR_POS_SIGN << CURSOR_X_SHIFT;
        x = -x;
    }
    pos |= x << CURSOR_X_SHIFT;

    if (y < 0)
    {
        if (y + intel_crtc->cursor_height < 0)
            base = 0;

        pos |= CURSOR_POS_SIGN << CURSOR_Y_SHIFT;
        y = -y;
    }
    pos |= y << CURSOR_Y_SHIFT;

    visible = base != 0;
    if (!visible && !intel_crtc->cursor_visible)
        return;

    I915_WRITE(CURPOS(pipe), pos);
//    if (IS_845G(dev) || IS_I865G(dev))
//        i845_update_cursor(crtc, base);
//    else
        i9xx_update_cursor(os_display->crtc, base);

};


cursor_t* __stdcall select_cursor_kms(cursor_t *cursor)
{
    struct drm_i915_private *dev_priv = os_display->ddev->dev_private;
    struct intel_crtc *intel_crtc = to_intel_crtc(os_display->crtc);
    cursor_t *old;

    old = os_display->cursor;
    os_display->cursor = cursor;

    if (!dev_priv->info->cursor_needs_physical)
       intel_crtc->cursor_addr = i915_gem_obj_ggtt_offset(cursor->cobj);
    else
        intel_crtc->cursor_addr = (addr_t)cursor->cobj;

    intel_crtc->cursor_width = 32;
    intel_crtc->cursor_height = 32;

    move_cursor_kms(cursor, intel_crtc->cursor_x, intel_crtc->cursor_y);
    return old;
};

struct sna_fb
{
    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch;
    uint32_t  tiling;
};

int i915_fbinfo(struct sna_fb *fb)
{
    fb->width  = os_display->width;
    fb->height = os_display->height;
    fb->pitch  = os_display->pitch;
    fb->tiling = 0;

    return 0;
};

typedef struct
{
    int left;
    int top;
    int right;
    int bottom;
}rect_t;

struct drm_i915_mask {
    __u32 handle;
    __u32 width;
    __u32 height;
    __u32 bo_size;
    __u32 bo_pitch;
    __u32 bo_map;
};

#define CURRENT_TASK             (0x80003000)

static u32_t get_display_map()
{
    u32_t   addr;

    addr = (u32_t)os_display;
    addr+= sizeof(display_t);            /*  shoot me  */
    return *(u32_t*)addr;
}

void  FASTCALL GetWindowRect(rect_t *rc)__asm__("GetWindowRect");

int i915_mask_update(struct drm_device *dev, void *data,
            struct drm_file *file)
{
    struct drm_i915_mask *mask = data;
    struct drm_gem_object *obj;
    static unsigned int mask_seqno[256];
    rect_t winrc;
    u32    slot;
    int    ret;

     if(mask->handle == -2)
     {
        printf("%s handle %d\n", __FUNCTION__, mask->handle);
        return 0;
     }

    obj = drm_gem_object_lookup(dev, file, mask->handle);
    if (obj == NULL)
        return -ENOENT;

    if (!obj->filp) {
        drm_gem_object_unreference_unlocked(obj);
        return -EINVAL;
    }

    GetWindowRect(&winrc);
    {
//        static warn_count;

        mask->width    = winrc.right - winrc.left + 1;
        mask->height   = winrc.bottom - winrc.top + 1;
        mask->bo_pitch = (mask->width+15) & ~15;

#if 0
        if(warn_count < 1)
        {
            printf("left %d top %d right %d bottom %d\n",
                    winrc.left, winrc.top, winrc.right, winrc.bottom);
            printf("mask pitch %d data %p\n", mask->bo_pitch, mask->bo_size);
            warn_count++;
        };
#endif

     };


    slot = *((u8*)CURRENT_TASK);

    if( mask_seqno[slot] != os_display->mask_seqno)
    {
        u8* src_offset;
        u8* dst_offset;
        u32 ifl;

        ret = i915_mutex_lock_interruptible(dev);
        if (ret)
            return ret;

        ret = i915_gem_object_set_to_cpu_domain(to_intel_bo(obj), true);
        if(ret !=0 )
        {
            dbgprintf("%s fail\n", __FUNCTION__);
            return ret;
        };

//        printf("width %d height %d\n", winrc.right, winrc.bottom);

//        slot = 0x01;


        src_offset = (u8*)( winrc.top*os_display->width + winrc.left);
        src_offset+= get_display_map();
        dst_offset = (u8*)mask->bo_map;

        u32_t tmp_h = mask->height;

        ifl = safe_cli();
        {
            mask_seqno[slot] = os_display->mask_seqno;

            slot|= (slot<<8)|(slot<<16)|(slot<<24);

            __asm__ __volatile__ (
                "movd       %[slot],   %%xmm6    \n"
            "punpckldq  %%xmm6, %%xmm6            \n"
            "punpcklqdq %%xmm6, %%xmm6            \n"
            :: [slot]  "m" (slot)
            :"xmm6");

            while( tmp_h--)
            {
                int tmp_w = mask->bo_pitch;

                u8* tmp_src = src_offset;
                u8* tmp_dst = dst_offset;

                src_offset+= os_display->width;
                dst_offset+= mask->bo_pitch;

                while(tmp_w >= 64)
                {
                    __asm__ __volatile__ (
                    "movdqu     (%0),   %%xmm0            \n"
                    "movdqu   16(%0),   %%xmm1            \n"
                    "movdqu   32(%0),   %%xmm2            \n"
                    "movdqu   48(%0),   %%xmm3            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm1            \n"
                    "pcmpeqb    %%xmm6, %%xmm2            \n"
                    "pcmpeqb    %%xmm6, %%xmm3            \n"
                    "movdqa     %%xmm0,   (%%edi)         \n"
                    "movdqa     %%xmm1, 16(%%edi)         \n"
                    "movdqa     %%xmm2, 32(%%edi)         \n"
                    "movdqa     %%xmm3, 48(%%edi)         \n"

                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0","xmm1","xmm2","xmm3");
                    tmp_w -= 64;
                    tmp_src += 64;
                    tmp_dst += 64;
                }

                if( tmp_w >= 32 )
                {
                    __asm__ __volatile__ (
                    "movdqu     (%0),   %%xmm0            \n"
                    "movdqu   16(%0),   %%xmm1            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm1            \n"
                    "movdqa     %%xmm0,   (%%edi)         \n"
                    "movdqa     %%xmm1, 16(%%edi)         \n"

                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0","xmm1");
                    tmp_w -= 32;
                    tmp_src += 32;
                    tmp_dst += 32;
                }

                while( tmp_w > 0 )
                {
                    __asm__ __volatile__ (
                    "movdqu     (%0),   %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "movdqa     %%xmm0,   (%%edi)         \n"
                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0");
                    tmp_w -= 16;
                    tmp_src += 16;
                    tmp_dst += 16;
                }
            };
        };
        safe_sti(ifl);
    }

    drm_gem_object_unreference(obj);

    mutex_unlock(&dev->struct_mutex);

    return 0;
}












#define NSEC_PER_SEC    1000000000L

void getrawmonotonic(struct timespec *ts)
{
    u32 tmp = GetTimerTicks();

    ts->tv_sec  = tmp/100;
    ts->tv_nsec = (tmp - ts->tv_sec*100)*10000000;
}

void set_normalized_timespec(struct timespec *ts, time_t sec, s64 nsec)
{
    while (nsec >= NSEC_PER_SEC) {
        /*
         * The following asm() prevents the compiler from
         * optimising this loop into a modulo operation. See
         * also __iter_div_u64_rem() in include/linux/time.h
         */
        asm("" : "+rm"(nsec));
        nsec -= NSEC_PER_SEC;
        ++sec;
    }
    while (nsec < 0) {
        asm("" : "+rm"(nsec));
        nsec += NSEC_PER_SEC;
        --sec;
    }
    ts->tv_sec = sec;
    ts->tv_nsec = nsec;
}

void
prepare_to_wait(wait_queue_head_t *q, wait_queue_t *wait, int state)
{
    unsigned long flags;

//    wait->flags &= ~WQ_FLAG_EXCLUSIVE;
    spin_lock_irqsave(&q->lock, flags);
    if (list_empty(&wait->task_list))
            __add_wait_queue(q, wait);
    spin_unlock_irqrestore(&q->lock, flags);
}

/**
 * finish_wait - clean up after waiting in a queue
 * @q: waitqueue waited on
 * @wait: wait descriptor
 *
 * Sets current thread back to running state and removes
 * the wait descriptor from the given waitqueue if still
 * queued.
 */
void finish_wait(wait_queue_head_t *q, wait_queue_t *wait)
{
    unsigned long flags;

//    __set_current_state(TASK_RUNNING);
    /*
     * We can check for list emptiness outside the lock
     * IFF:
     *  - we use the "careful" check that verifies both
     *    the next and prev pointers, so that there cannot
     *    be any half-pending updates in progress on other
     *    CPU's that we haven't seen yet (and that might
     *    still change the stack area.
     * and
     *  - all other users take the lock (ie we can only
     *    have _one_ other CPU that looks at or modifies
     *    the list).
     */
    if (!list_empty_careful(&wait->task_list)) {
            spin_lock_irqsave(&q->lock, flags);
            list_del_init(&wait->task_list);
            spin_unlock_irqrestore(&q->lock, flags);
    }

    DestroyEvent(wait->evnt);
}

int autoremove_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key)
{
    list_del_init(&wait->task_list);
    return 1;
}

unsigned int hweight16(unsigned int w)
{
    unsigned int res = w - ((w >> 1) & 0x5555);
    res = (res & 0x3333) + ((res >> 2) & 0x3333);
    res = (res + (res >> 4)) & 0x0F0F;
    return (res + (res >> 8)) & 0x00FF;
}


unsigned long round_jiffies_up_relative(unsigned long j)
{
    unsigned long j0 = GetTimerTicks();

        /* Use j0 because jiffies might change while we run */
    return round_jiffies_common(j + j0, true) - j0;
}


