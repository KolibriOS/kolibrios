
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

#define XY_COLOR_BLT        ((2<<29)|(0x50<<22)|(0x4))
#define BLT_WRITE_ALPHA     (1<<21)
#define BLT_WRITE_RGB       (1<<20)

#if 1
    {

        drm_i915_private_t *dev_priv = dev->dev_private;
        struct drm_i915_gem_object *obj;
        struct intel_ring_buffer *ring;

        obj = i915_gem_alloc_object(dev, 4096);
        i915_gem_object_pin(obj, 4096, true);

        cmd_buffer = MapIoMem(obj->pages[0], 4096, PG_SW|PG_NOCACHE);
        cmd_offset = obj->gtt_offset;
    };
#endif

    int err;

    err = init_bitmaps();
    if( !err )
    {
        printf("Initialize bitmap manager\n");
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
        fb->pitches[0]  = ALIGN(reqmode->width * 4, 64);
        fb->pitches[1]  = ALIGN(reqmode->width * 4, 64);
        fb->pitches[2]  = ALIGN(reqmode->width * 4, 64);
        fb->pitches[3]  = ALIGN(reqmode->width * 4, 64);

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
            os_display->pitch    = fb->pitches[0];
            os_display->vrefresh = drm_mode_vrefresh(mode);

            sysSetScreen(fb->width, fb->height, fb->pitches[0]);

            dbgprintf("new mode %d x %d pitch %d\n",
                       fb->width, fb->height, fb->pitches[0]);
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
/*  FIXME    synchronization */

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


typedef struct
{
    int left;
    int top;
    int right;
    int bottom;
}rect_t;


#include "clip.inc"

void  FASTCALL GetWindowRect(rect_t *rc)__asm__("GetWindowRect");

#define CURRENT_TASK             (0x80003000)

static u32_t get_display_map()
{
    u32_t   addr;

    addr = (u32_t)os_display;
    addr+= sizeof(display_t);            /*  shoot me  */
    return *(u32_t*)addr;
}

#define XY_SRC_COPY_CHROMA_CMD     ((2<<29)|(0x73<<22)|8)
#define ROP_COPY_SRC               0xCC
#define FORMAT8888                 3

typedef int v4si __attribute__ ((vector_size (16)));


int blit_video(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h)
{
    drm_i915_private_t *dev_priv = main_device->dev_private;
    struct intel_ring_buffer *ring;

    bitmap_t  *bitmap;
    rect_t     winrc;
    clip_t     dst_clip;
    clip_t     src_clip;
    u32_t      width;
    u32_t      height;

    u32_t      br13, cmd, slot_mask, *b;
    u32_t      offset;
    u8         slot;
    int      n=0;

    if(unlikely(hbitmap==0))
        return -1;

    bitmap = (bitmap_t*)hman_get_data(&bm_man, hbitmap);

    if(unlikely(bitmap==NULL))
        return -1;


    GetWindowRect(&winrc);

    dst_clip.xmin   = 0;
    dst_clip.ymin   = 0;
    dst_clip.xmax   = winrc.right-winrc.left-1;
    dst_clip.ymax   = winrc.bottom -winrc.top -1;

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
        static v4si write_mask = {0xFF000000, 0xFF000000,
                                  0xFF000000, 0xFF000000};

        u8* src_offset;
        u8* dst_offset;

        src_offset = (u8*)(src_y*bitmap->pitch + src_x*4);
        src_offset += (u32)bitmap->uaddr;

        dst_offset = (u8*)(dst_y*os_display->width + dst_x);
        dst_offset+= get_display_map();

        u32_t tmp_h = height;

        __asm__ __volatile__ (
        "movdqa     %[write_mask],  %%xmm7    \n"
        "movd       %[slot_mask],   %%xmm6    \n"
        "punpckldq  %%xmm6, %%xmm6            \n"
        "punpcklqdq %%xmm6, %%xmm6            \n"
        :: [write_mask] "m" (write_mask),
           [slot_mask]  "g" (slot_mask)
        :"xmm7", "xmm6");

        while( tmp_h--)
        {
            u32_t tmp_w = width;

            u8* tmp_src = src_offset;
            u8* tmp_dst = dst_offset;

            src_offset+= bitmap->pitch;
            dst_offset+= os_display->width;

            while( tmp_w >= 8 )
            {
                __asm__ __volatile__ (
                "movq       (%0),   %%xmm0            \n"
                "punpcklbw  %%xmm0, %%xmm0            \n"
                "movdqa     %%xmm0, %%xmm1            \n"
                "punpcklwd  %%xmm0, %%xmm0            \n"
                "punpckhwd  %%xmm1, %%xmm1            \n"
                "pcmpeqb    %%xmm6, %%xmm0            \n"
                "pcmpeqb    %%xmm6, %%xmm1            \n"
                "maskmovdqu %%xmm7, %%xmm0            \n"
                "addl       $16, %%edi                \n"
                "maskmovdqu %%xmm7, %%xmm1            \n"
                :: "r" (tmp_dst), "D" (tmp_src)
                :"xmm0", "xmm1");
                __asm__ __volatile__ ("":::"edi");
                tmp_w -= 8;
                tmp_src += 32;
                tmp_dst += 8;
            };

            if( tmp_w >= 4 )
            {
                __asm__ __volatile__ (
                "movd       (%0),   %%xmm0            \n"
                "punpcklbw  %%xmm0, %%xmm0            \n"
                "punpcklwd  %%xmm0, %%xmm0            \n"
                "pcmpeqb    %%xmm6, %%xmm0            \n"
                "maskmovdqu %%xmm7, %%xmm0            \n"
                :: "r" (tmp_dst), "D" (tmp_src)
                :"xmm0");
                tmp_w -= 4;
                tmp_src += 16;
                tmp_dst += 4;
            };

            while( tmp_w--)
            {
                *(tmp_src+3) = (*tmp_dst==slot)?0xFF:0x00;
                tmp_src+=4;
                tmp_dst++;
            };
        };
#else
        u8* src_offset;
        u8* dst_offset;

        src_offset = (u8*)(src_y*bitmap->pitch + src_x*4);
        src_offset += (u32)bitmap->uaddr;

        dst_offset = (u8*)(dst_y*os_display->width + dst_x);
        dst_offset+= get_display_map();

        u32_t tmp_h = height;

        while( tmp_h--)
        {
            u32_t tmp_w = width;

            u8* tmp_src = src_offset;
            u8* tmp_dst = dst_offset;

            src_offset+= bitmap->pitch;
            dst_offset+= os_display->width;

            while( tmp_w--)
            {
                *(tmp_src+3) = (*tmp_dst==slot)?0xFF:0x00;
                tmp_src+=4;
                tmp_dst++;
            };
        };
    }
#endif

    if((cmd_buffer & 0xFC0)==0xFC0)
        cmd_buffer&= 0xFFFFF000;

    b = (u32_t*)ALIGN(cmd_buffer,16);

    offset = cmd_offset + ((u32_t)b & 0xFFF);

    cmd = XY_SRC_COPY_CHROMA_CMD | BLT_WRITE_RGB | BLT_WRITE_ALPHA;
    cmd |= 3 << 17;

    br13 = os_display->pitch;
    br13|= ROP_COPY_SRC << 16;
    br13|= FORMAT8888   << 24;

    b[n++] = cmd;
    b[n++] = br13;
    b[n++] = (dst_y << 16) | dst_x;                   // left, top
    b[n++] = ((dst_y+height-1)<< 16)|(dst_x+width-1); // bottom, right
    b[n++] = 0;                          // destination
    b[n++] = (src_y << 16) | src_x;      // source left & top
    b[n++] = bitmap->pitch;              // source pitch
    b[n++] = bitmap->gaddr;              // source

    b[n++] = 0;                          // Transparency Color Low
    b[n++] = 0x00FFFFFF;                 // Transparency Color High

    b[n++] = MI_BATCH_BUFFER_END;
    if( n & 1)
        b[n++] = MI_NOOP;

//    i915_gem_object_set_to_gtt_domain(obj, false);

    if (HAS_BLT(main_device))
        ring = &dev_priv->ring[BCS];
    else
        ring = &dev_priv->ring[RCS];

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
