
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
};


static display_t *os_display;

u32_t cmd_buffer;
u32_t cmd_offset;

int init_cursor(cursor_t *cursor);
static cursor_t*  __stdcall select_cursor_kms(cursor_t *cursor);
static void       __stdcall move_cursor_kms(cursor_t *cursor, int x, int y);

void __stdcall restore_cursor(int x, int y)
{};

void disable_mouse(void)
{};

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

int init_display_kms(struct drm_device *dev)
{
    struct drm_connector    *connector;
    struct drm_connector_helper_funcs *connector_funcs;
    struct drm_encoder      *encoder;
    struct drm_crtc         *crtc = NULL;
    struct drm_framebuffer  *fb;

    cursor_t  *cursor;
    u32_t      ifl;

    ENTER();

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        if( connector->status != connector_status_connected)
            continue;

        connector_funcs = connector->helper_private;
        encoder = connector_funcs->best_encoder(connector);
        if( encoder == NULL)
        {
            dbgprintf("CONNECTOR %x ID: %d no active encoders\n",
                      connector, connector->base.id);
            continue;
        }
        connector->encoder = encoder;

        dbgprintf("CONNECTOR %x ID:  %d status %d encoder %x\n crtc %x\n",
               connector, connector->base.id,
               connector->status, connector->encoder,
               encoder->crtc);

        crtc = encoder->crtc;
        break;
    };

    if(connector == NULL)
    {
        dbgprintf("No active connectors!\n");
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
        dbgprintf("No CRTC for encoder %d\n", encoder->base.id);
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

    {
#define XY_COLOR_BLT        ((2<<29)|(0x50<<22)|(0x4))
#define BLT_WRITE_ALPHA     (1<<21)
#define BLT_WRITE_RGB       (1<<20)

        drm_i915_private_t *dev_priv = dev->dev_private;
        struct drm_i915_gem_object *obj;
        struct intel_ring_buffer *ring;

        u32_t br13, cmd, *b;

        int      n=0;

        cmd =  XY_COLOR_BLT | BLT_WRITE_ALPHA | BLT_WRITE_RGB;
        br13 = os_display->pitch;
        br13 |= 0xF0 << 16;
        br13 |= 3 << 24;

        obj = i915_gem_alloc_object(dev, 4096);
        i915_gem_object_pin(obj, 4096, true);

        cmd_buffer = MapIoMem(obj->pages[0], 4096, PG_SW|PG_NOCACHE);
        cmd_offset = obj->gtt_offset;

        b = (u32_t*)cmd_buffer;
        b[n++] = cmd;
        b[n++] = br13;
        b[n++] = 0; // top, left
        b[n++] = (128 << 16) | 128; // bottom, right
        b[n++] = 0; // dst
        b[n++] = 0x0000FF00;
        b[n++] = MI_BATCH_BUFFER_END;
        if( n & 1)
            b[n++] = MI_NOOP;

//        cmd_buffer = (u32_t)&b[n];
//        i915_gem_object_set_to_gtt_domain(obj, false);


        ring = &dev_priv->ring[BCS];
        ring->dispatch_execbuffer(ring,cmd_offset, n*4);

    };

    LEAVE();

    return 0;
};


bool set_mode(struct drm_device *dev, struct drm_connector *connector,
              videomode_t *reqmode, bool strict)
{
    struct drm_display_mode  *mode = NULL, *tmpmode;
    drm_i915_private_t *dev_priv = dev->dev_private;
    struct drm_fb_helper *fb_helper = &dev_priv->fbdev->helper;

    bool ret = false;

    ENTER();

    dbgprintf("width %d height %d vrefresh %d\n",
               reqmode->width, reqmode->height, reqmode->freq);

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

do_set:

    if( mode != NULL )
    {
        struct drm_framebuffer   *fb;
        struct drm_encoder       *encoder;
        struct drm_crtc          *crtc;

        char *con_name;
        char *enc_name;

        encoder = connector->encoder;
        crtc = encoder->crtc;

        con_name = drm_get_connector_name(connector);
        enc_name = drm_get_encoder_name(encoder);

        dbgprintf("set mode %d %d connector %s encoder %s\n",
                   reqmode->width, reqmode->height, con_name, enc_name);

        fb = fb_helper->fb;

        fb->width  = reqmode->width;
        fb->height = reqmode->height;
        fb->pitch  = ALIGN(reqmode->width * 4, 64);
        fb->bits_per_pixel = 32;
        fb->depth == 24;

        crtc->fb = fb;
        crtc->enabled = true;
        os_display->crtc = crtc;

        ret = drm_crtc_helper_set_mode(crtc, mode, 0, 0, fb);

//        select_cursor_kms(rdisplay->cursor);
//        radeon_show_cursor_kms(crtc);

        if (ret == true)
        {
            os_display->width    = fb->width;
            os_display->height   = fb->height;
            os_display->pitch    = fb->pitch;
            os_display->vrefresh = drm_mode_vrefresh(mode);

            sysSetScreen(fb->width, fb->height, fb->pitch);

            dbgprintf("new mode %d x %d pitch %d\n",
                       fb->width, fb->height, fb->pitch);
        }
        else
            DRM_ERROR("failed to set mode %d_%d on crtc %p\n",
                       fb->width, fb->height, crtc);
    }

    LEAVE();
    return ret;
};



int get_videomodes(videomode_t *mode, int *count)
{
    int err = -1;

    ENTER();

    dbgprintf("mode %x count %d\n", mode, *count);

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
    LEAVE();
    return err;
};

int set_user_mode(videomode_t *mode)
{
    int err = -1;

    ENTER();

    dbgprintf("width %d height %d vrefresh %d\n",
               mode->width, mode->height, mode->freq);

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

    LEAVE();
    return err;
};

void __attribute__((regparm(1))) destroy_cursor(cursor_t *cursor)
{
    list_del(&cursor->list);
//    radeon_bo_unpin(cursor->robj);
//    KernelFree(cursor->data);
    __DestroyObject(cursor);
};

int init_cursor(cursor_t *cursor)
{
    struct drm_i915_private *dev_priv = os_display->ddev->dev_private;
    struct drm_i915_gem_object *obj;
    uint32_t *bits;
    uint32_t *src;

    int       i,j;
    int       ret;

    ENTER();

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

        ret = i915_gem_object_pin(obj, CURSOR_WIDTH*CURSOR_HEIGHT*4, true);
        if (ret) {
//           drm_gem_object_unreference(&obj->base);
            return ret;
        }

/* You don't need to worry about fragmentation issues.
 * GTT space is continuous. I guarantee it.                           */

        bits = (u32*)MapIoMem(get_bus_addr() + obj->gtt_offset,
                    CURSOR_WIDTH*CURSOR_HEIGHT*4, PG_SW);

        if (unlikely(bits == NULL))
        {
//          i915_gem_object_unpin(obj);
//           drm_gem_object_unreference(&obj->base);
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

// release old cursor

    KernelFree(cursor->data);

    cursor->data = bits;

    cursor->header.destroy = destroy_cursor;
    LEAVE();

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
       intel_crtc->cursor_addr = cursor->cobj->gtt_offset;
    else
        intel_crtc->cursor_addr = cursor->cobj;

    intel_crtc->cursor_width = 32;
    intel_crtc->cursor_height = 32;

    move_cursor_kms(cursor, intel_crtc->cursor_x, intel_crtc->cursor_y);
    return old;
};

extern struct drm_device *main_device;

#define XY_SRC_COPY_BLT_CMD     ((2<<29)|(0x53<<22)|6)

int video_blit(uint64_t src_offset, int  x, int y,
                    int w, int h, int pitch)
{

    drm_i915_private_t *dev_priv = main_device->dev_private;
    struct intel_ring_buffer *ring;

    u32_t br13, cmd, *b;
    u32_t offset;

    int      n=0;

//    if( cmd_buffer & 0xF80 )
//        cmd_buffer&= 0xFFFFF000;

//    b = (u32_t*)ALIGN(cmd_buffer,16);

//    offset = cmd_offset + ((u32_t)b & 0xFFF);

    b = cmd_buffer;

    cmd =  XY_SRC_COPY_BLT_CMD | BLT_WRITE_RGB;
    br13 = os_display->pitch;
    br13 |= 0xCC << 16;
    br13 |= 3 << 24;

    b[n++] = cmd;
    b[n++] = br13;
    b[n++] = (y << 16) | x;
    b[n++] = ( (y+h) << 16) | (x+w); // bottom, right
    b[n++] = 0; // dst_offset
    b[n++] = 0; //src_top|src_left

    b[n++] = pitch;
    b[n++] = (u32_t)src_offset;

    b[n++] = MI_BATCH_BUFFER_END;
    if( n & 1)
        b[n++] = MI_NOOP;

//    i915_gem_object_set_to_gtt_domain(obj, false);

    ring = &dev_priv->ring[BCS];
    ring->dispatch_execbuffer(ring, cmd_offset, n*4);

    intel_ring_begin(ring, 4);
//    if (ret)
//        return ret;

//    cmd = MI_FLUSH_DW;
//    if (invalidate & I915_GEM_GPU_DOMAINS)
//        cmd |= MI_INVALIDATE_TLB | MI_INVALIDATE_BSD;
    intel_ring_emit(ring, MI_FLUSH_DW);
    intel_ring_emit(ring, 0);
    intel_ring_emit(ring, 0);
    intel_ring_emit(ring, MI_NOOP);
    intel_ring_advance(ring);


fail:
    return -1;
};


int blit_video(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h)
{
    drm_i915_private_t *dev_priv = main_device->dev_private;
    struct intel_ring_buffer *ring;

    bitmap_t  *bitmap;
    u32_t br13, cmd, *b;
    u32_t offset;

    int      n=0;

    if(unlikely(hbitmap==0))
        return -1;

    bitmap = hman_get_data(&bm_man, hbitmap);

    if(unlikely(bitmap==NULL))
        return -1;

//    if( cmd_buffer & 0xF80 )
//        cmd_buffer&= 0xFFFFF000;

//    b = (u32_t*)ALIGN(cmd_buffer,16);

//    offset = cmd_offset + ((u32_t)b & 0xFFF);

    b = cmd_buffer;

    cmd =  XY_SRC_COPY_BLT_CMD | BLT_WRITE_RGB;
    br13 = os_display->pitch;
    br13 |= 0xCC << 16;
    br13 |= 3 << 24;

    b[n++] = cmd;
    b[n++] = br13;
    b[n++] = (dst_y << 16) | dst_x;
    b[n++] = ( (dst_y+h) << 16) | (dst_x+w); // bottom, right
    b[n++] = 0; // dst_offset
    b[n++] = (src_y << 16) | src_x;

    b[n++] = bitmap->pitch;
    b[n++] = bitmap->gaddr;

    b[n++] = MI_BATCH_BUFFER_END;
    if( n & 1)
        b[n++] = MI_NOOP;

//    i915_gem_object_set_to_gtt_domain(obj, false);

    ring = &dev_priv->ring[BCS];
    ring->dispatch_execbuffer(ring, cmd_offset, n*4);

    intel_ring_begin(ring, 4);
//    if (ret)
//        return ret;

    intel_ring_emit(ring, MI_FLUSH_DW);
    intel_ring_emit(ring, 0);
    intel_ring_emit(ring, 0);
    intel_ring_emit(ring, MI_NOOP);
    intel_ring_advance(ring);

    return 0;
fail:
    return -1;
};
