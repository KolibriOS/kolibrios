
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

#include "hmm.h"
#include "bitmap.h"

extern struct drm_device *main_device;


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

    main_device = dev;

    err = init_bitmaps();

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

        ret = i915_gem_object_pin(obj, CURSOR_WIDTH*CURSOR_HEIGHT*4, true, true);
        if (ret) {
            drm_gem_object_unreference(&obj->base);
            return ret;
        }

/* You don't need to worry about fragmentation issues.
 * GTT space is continuous. I guarantee it.                           */

        mapped = bits = (u32*)MapIoMem(dev_priv->mm.gtt->gma_bus_addr + obj->gtt_offset,
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
       intel_crtc->cursor_addr = cursor->cobj->gtt_offset;
    else
        intel_crtc->cursor_addr = (addr_t)cursor->cobj;

    intel_crtc->cursor_width = 32;
    intel_crtc->cursor_height = 32;

    move_cursor_kms(cursor, intel_crtc->cursor_x, intel_crtc->cursor_y);
    return old;
};





extern struct hmm bm_mm;


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

#define XY_COLOR_BLT                ((2<<29)|(0x50<<22)|(0x4))
#define XY_SRC_COPY_BLT_CMD         ((2<<29)|(0x53<<22)|6)
#define XY_SRC_COPY_CHROMA_CMD     ((2<<29)|(0x73<<22)|8)
#define ROP_COPY_SRC               0xCC
#define FORMAT8888                 3

#define BLT_WRITE_ALPHA             (1<<21)
#define BLT_WRITE_RGB               (1<<20)



typedef int v4si __attribute__ ((vector_size (16)));



static void
i915_gem_execbuffer_retire_commands(struct drm_device *dev,
                    struct drm_file *file,
                    struct intel_ring_buffer *ring)
{
    /* Unconditionally force add_request to emit a full flush. */
    ring->gpu_caches_dirty = true;

    /* Add a breadcrumb for the completion of the batch buffer */
    (void)i915_add_request(ring, file, NULL);
}

int blit_video(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h)
{
    drm_i915_private_t *dev_priv = main_device->dev_private;
    struct intel_ring_buffer *ring;
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
    int      n=0;
    int        ret;

    if(unlikely(hbitmap==0))
        return -1;

    bitmap = (bitmap_t*)hmm_get_data(&bm_mm, hbitmap);

    if(unlikely(bitmap==NULL))
        return -1;

    context = get_context(main_device);
    if(unlikely(context == NULL))
        return -1;

    GetWindowRect(&winrc);

    dst_clip.xmin   = 0;
    dst_clip.ymin   = 0;
    dst_clip.xmax   = winrc.right-winrc.left-1;
    dst_clip.ymax   = winrc.bottom -winrc.top-1;

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
        u32 ifl;

        src_offset = (u8*)(src_y*bitmap->pitch + src_x*4);
        src_offset += (u32)bitmap->uaddr;

        dst_offset = (u8*)(dst_y*os_display->width + dst_x);
        dst_offset+= get_display_map();

        u32_t tmp_h = height;

      ifl = safe_cli();
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
      safe_sti(ifl);
    }
#endif

    if((context->cmd_buffer & 0xFC0)==0xFC0)
        context->cmd_buffer&= 0xFFFFF000;

    b = (u32_t*)ALIGN(context->cmd_buffer,16);

    offset = context->cmd_offset + ((u32_t)b & 0xFFF);

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

    context->cmd_buffer+= n*4;

    context->obj->base.pending_read_domains |= I915_GEM_DOMAIN_COMMAND;


    mutex_lock(&main_device->struct_mutex);

    i915_gem_object_set_to_gtt_domain(bitmap->obj, false);

    if (HAS_BLT(main_device))
    {
        u32 seqno;
        int i;

        ring = &dev_priv->ring[BCS];
//        printf("dispatch...  ");

        i915_gem_object_sync(bitmap->obj, ring);
        intel_ring_invalidate_all_caches(ring);

        seqno = i915_gem_next_request_seqno(ring);
//        printf("seqno = %d\n", seqno);

        for (i = 0; i < ARRAY_SIZE(ring->sync_seqno); i++) {
            if (seqno < ring->sync_seqno[i]) {
            /* The GPU can not handle its semaphore value wrapping,
             * so every billion or so execbuffers, we need to stall
             * the GPU in order to reset the counters.
             */
                DRM_DEBUG("wrap seqno\n");

                ret = i915_gpu_idle(main_device);
                if (ret)
                    goto fail;
                i915_gem_retire_requests(main_device);

                BUG_ON(ring->sync_seqno[i]);
            }
        }

        ret = ring->dispatch_execbuffer(ring, offset, n*4);
        if (ret)
            goto fail;
//        printf("done\n");

        bitmap->obj->base.read_domains = bitmap->obj->base.pending_read_domains;
        bitmap->obj->base.write_domain = bitmap->obj->base.pending_write_domain;
        bitmap->obj->fenced_gpu_access = bitmap->obj->pending_fenced_gpu_access;

        i915_gem_object_move_to_active(bitmap->obj, ring, seqno);

        i915_gem_execbuffer_retire_commands(main_device, NULL, ring);
//        printf("retire\n");
    }
    else
    {
        ring = &dev_priv->ring[RCS];
        ring->dispatch_execbuffer(ring, offset, n*4);
        ring->flush(ring, 0, I915_GEM_DOMAIN_RENDER);
    };

//    bitmap->obj->base.read_domains = I915_GEM_DOMAIN_CPU;
//    bitmap->obj->base.write_domain = I915_GEM_DOMAIN_CPU;

    mutex_unlock(&main_device->struct_mutex);
fail:
    return ret;
};


#if 0

    i915_gem_execbuffer_retire_commands(dev, ring);
/* For display hotplug interrupt */
static void
ironlake_enable_display_irq(drm_i915_private_t *dev_priv, u32 mask)
{
    if ((dev_priv->irq_mask & mask) != 0) {
        dev_priv->irq_mask &= ~mask;
        I915_WRITE(DEIMR, dev_priv->irq_mask);
        POSTING_READ(DEIMR);
    }
}

static int ironlake_enable_vblank(struct drm_device *dev, int pipe)
{
    drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
    unsigned long irqflags;

//    if (!i915_pipe_enabled(dev, pipe))
//        return -EINVAL;

    spin_lock_irqsave(&dev_priv->irq_lock, irqflags);
    ironlake_enable_display_irq(dev_priv, (pipe == 0) ?
                    DE_PIPEA_VBLANK : DE_PIPEB_VBLANK);
    spin_unlock_irqrestore(&dev_priv->irq_lock, irqflags);

    return 0;
}



static int i915_interrupt_info(struct drm_device *dev)
{
    drm_i915_private_t *dev_priv = dev->dev_private;
    int ret, i, pipe;

    if (!HAS_PCH_SPLIT(dev)) {
        dbgprintf("Interrupt enable:    %08x\n",
               I915_READ(IER));
        dbgprintf("Interrupt identity:  %08x\n",
               I915_READ(IIR));
        dbgprintf("Interrupt mask:      %08x\n",
               I915_READ(IMR));
        for_each_pipe(pipe)
            dbgprintf("Pipe %c stat:         %08x\n",
                   pipe_name(pipe),
                   I915_READ(PIPESTAT(pipe)));
    } else {
        dbgprintf("North Display Interrupt enable:      %08x\n",
           I915_READ(DEIER));
        dbgprintf("North Display Interrupt identity:    %08x\n",
           I915_READ(DEIIR));
        dbgprintf("North Display Interrupt mask:        %08x\n",
           I915_READ(DEIMR));
        dbgprintf("South Display Interrupt enable:      %08x\n",
           I915_READ(SDEIER));
        dbgprintf("South Display Interrupt identity:    %08x\n",
           I915_READ(SDEIIR));
        dbgprintf("South Display Interrupt mask:        %08x\n",
           I915_READ(SDEIMR));
        dbgprintf("Graphics Interrupt enable:           %08x\n",
           I915_READ(GTIER));
        dbgprintf("Graphics Interrupt identity:         %08x\n",
           I915_READ(GTIIR));
        dbgprintf("Graphics Interrupt mask:             %08x\n",
               I915_READ(GTIMR));
    }
    dbgprintf("Interrupts received: %d\n",
           atomic_read(&dev_priv->irq_received));
    for (i = 0; i < I915_NUM_RINGS; i++) {
        if (IS_GEN6(dev) || IS_GEN7(dev)) {
            printf("Graphics Interrupt mask (%s):       %08x\n",
                   dev_priv->ring[i].name,
                   I915_READ_IMR(&dev_priv->ring[i]));
        }
//        i915_ring_seqno_info(m, &dev_priv->ring[i]);
    }

    return 0;
}

void execute_buffer (struct drm_i915_gem_object *buffer, uint32_t offset,
                     int size)
{
    struct intel_ring_buffer *ring;
    drm_i915_private_t *dev_priv = main_device->dev_private;
    u32 invalidate;
    u32 seqno = 2;

    offset += buffer->gtt_offset;
//    dbgprintf("execute %x size %d\n", offset, size);

//    asm volatile(
//    "mfence \n"
//    "wbinvd \n"
//    "mfence  \n"
//    :::"memory");

    ring = &dev_priv->ring[RCS];
    ring->dispatch_execbuffer(ring, offset, size);

    invalidate = I915_GEM_DOMAIN_COMMAND;
    if (INTEL_INFO(main_device)->gen >= 4)
        invalidate |= I915_GEM_DOMAIN_SAMPLER;
    if (ring->flush(ring, invalidate, 0))
        i915_gem_next_request_seqno(ring);

    ring->irq_get(ring);

    ring->add_request(ring, &seqno);

//    i915_interrupt_info(main_device);

};


int blit_textured(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h)
{
    drm_i915_private_t *dev_priv = main_device->dev_private;

    bitmap_t  *src_bitmap, *dst_bitmap;
    bitmap_t   screen;

    rect_t     winrc;

//    dbgprintf("  handle: %d dx %d dy %d sx %d sy %d w %d h %d\n",
//              hbitmap, dst_x, dst_y, src_x, src_y, w, h);

    if(unlikely(hbitmap==0))
        return -1;

    src_bitmap = (bitmap_t*)hman_get_data(&bm_man, hbitmap);
//    dbgprintf("bitmap %x\n", src_bitmap);

    if(unlikely(src_bitmap==NULL))
        return -1;

    GetWindowRect(&winrc);

    screen.pitch  = os_display->pitch;
    screen.gaddr  = 0;
    screen.width  = os_display->width;
    screen.height = os_display->height;
    screen.obj    = (void*)-1;

    dst_bitmap = &screen;

    dst_x+= winrc.left;
    dst_y+= winrc.top;

    sna_blit_copy(dst_bitmap, dst_x, dst_y, w, h, src_bitmap, src_x, src_y);

};

int sna_blit_tex(bitmap_t *dst_bitmap, int dst_x, int dst_y,
                  int w, int h, bitmap_t *src_bitmap, int src_x, int src_y,
                  bitmap_t *mask_bitmap);


int blit_tex(u32 hbitmap, int  dst_x, int dst_y,
             int src_x, int src_y, u32 w, u32 h)
{
    drm_i915_private_t *dev_priv = main_device->dev_private;
    struct context *ctx;

    bitmap_t  *src_bitmap, *dst_bitmap;
    bitmap_t   screen;
    int        ret;

    bitmap_t *mask_bitmap;
    rect_t     winrc;

//    dbgprintf("  handle: %d dx %d dy %d sx %d sy %d w %d h %d\n",
//              hbitmap, dst_x, dst_y, src_x, src_y, w, h);

    if(unlikely(hbitmap==0))
        return -1;

    src_bitmap = (bitmap_t*)hman_get_data(&bm_man, hbitmap);
//    dbgprintf("bitmap %x\n", src_bitmap);

    if(unlikely(src_bitmap==NULL))
        return -1;

    ctx = get_context();
    if(unlikely(ctx==NULL))
    {
        ret = create_context();
        if(ret!=0)
            return -1;

        ctx = get_context();
    };

    mask_bitmap = ctx->mask;

    GetWindowRect(&winrc);
    dst_x+= winrc.left;
    dst_y+= winrc.top;


    if(ctx->seqno != os_display->mask_seqno)
    {
        u8* src_offset;
        u8* dst_offset;
        u32 slot;
        u32 ifl;

        ret = gem_object_lock(mask_bitmap->obj);
        if(ret !=0 )
        {
            dbgprintf("%s fail\n", __FUNCTION__);
            return ret;
        };

//        printf("width %d height %d\n", winrc.right, winrc.bottom);

        mask_bitmap->width  = winrc.right;
        mask_bitmap->height = winrc.bottom;
        mask_bitmap->pitch =  ALIGN(w,64);

        slot = *((u8*)CURRENT_TASK);
//        slot = 0x01;

        slot|= (slot<<8)|(slot<<16)|(slot<<24);


        __asm__ __volatile__ (
        "movd       %[slot],   %%xmm6    \n"
        "punpckldq  %%xmm6, %%xmm6            \n"
        "punpcklqdq %%xmm6, %%xmm6            \n"
        :: [slot]  "m" (slot)
        :"xmm6");

        src_offset = mask_bitmap->uaddr;

        dst_offset = (u8*)(dst_y*os_display->width + dst_x);
        dst_offset+= get_display_map();

        u32_t tmp_h = mask_bitmap->height;

      ifl = safe_cli();
        while( tmp_h--)
        {
            int tmp_w = mask_bitmap->width;

            u8* tmp_src = src_offset;
            u8* tmp_dst = dst_offset;

            src_offset+= mask_bitmap->pitch;
            dst_offset+= os_display->width;

//            while( tmp_w--)
//            {
//                *(tmp_src) = (*tmp_dst==slot)?0x1:0x00;
//                tmp_src++;
//                tmp_dst++;
//            };
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

                :: "r" (tmp_dst), "D" (tmp_src)
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

                :: "r" (tmp_dst), "D" (tmp_src)
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
                :: "r" (tmp_dst), "D" (tmp_src)
                :"xmm0");
                tmp_w -= 16;
                tmp_src += 16;
                tmp_dst += 16;
            }
        };
      safe_sti(ifl);
      ctx->seqno = os_display->mask_seqno;
    }

    screen.pitch  = os_display->pitch;
    screen.gaddr  = 0;
    screen.width  = os_display->width;
    screen.height = os_display->height;
    screen.obj    = (void*)-1;

    dst_bitmap = &screen;


    sna_blit_tex(dst_bitmap, dst_x, dst_y, w, h, src_bitmap, src_x, src_y,
                 mask_bitmap);

//    asm volatile ("int3");
};


#endif








void __stdcall run_workqueue(struct workqueue_struct *cwq)
{
    unsigned long irqflags;

//    dbgprintf("wq: %x head %x, next %x\n",
//               cwq, &cwq->worklist, cwq->worklist.next);

    spin_lock_irqsave(&cwq->lock, irqflags);

    while (!list_empty(&cwq->worklist))
    {
        struct work_struct *work = list_entry(cwq->worklist.next,
                                        struct work_struct, entry);
        work_func_t f = work->func;
        list_del_init(cwq->worklist.next);
//        dbgprintf("head %x, next %x\n",
//                  &cwq->worklist, cwq->worklist.next);

        spin_unlock_irqrestore(&cwq->lock, irqflags);
        f(work);
        spin_lock_irqsave(&cwq->lock, irqflags);
    }

    spin_unlock_irqrestore(&cwq->lock, irqflags);
}


static inline
int __queue_work(struct workqueue_struct *wq,
                         struct work_struct *work)
{
    unsigned long flags;

//    dbgprintf("wq: %x, work: %x\n",
//               wq, work );

    if(!list_empty(&work->entry))
        return 0;

    spin_lock_irqsave(&wq->lock, flags);

    if(list_empty(&wq->worklist))
        TimerHs(0,0, run_workqueue, wq);

    list_add_tail(&work->entry, &wq->worklist);

    spin_unlock_irqrestore(&wq->lock, flags);
//    dbgprintf("wq: %x head %x, next %x\n",
//               wq, &wq->worklist, wq->worklist.next);

    return 1;
};

void __stdcall delayed_work_timer_fn(unsigned long __data)
{
    struct delayed_work *dwork = (struct delayed_work *)__data;
    struct workqueue_struct *wq = dwork->work.data;

//    dbgprintf("wq: %x, work: %x\n",
//               wq, &dwork->work );

    __queue_work(wq, &dwork->work);
}


int queue_delayed_work_on(struct workqueue_struct *wq,
                        struct delayed_work *dwork, unsigned long delay)
{
    struct work_struct *work = &dwork->work;

    work->data = wq;
    TimerHs(0,0, delayed_work_timer_fn, dwork);
    return 1;
}

int queue_delayed_work(struct workqueue_struct *wq,
                        struct delayed_work *dwork, unsigned long delay)
{
    u32  flags;

//    dbgprintf("wq: %x, work: %x\n",
//               wq, &dwork->work );

    if (delay == 0)
        return __queue_work(wq, &dwork->work);

    return queue_delayed_work_on(wq, dwork, delay);
}


struct workqueue_struct *alloc_workqueue(const char *fmt,
                           unsigned int flags,
                           int max_active)
{
    struct workqueue_struct *wq;

    wq = kzalloc(sizeof(*wq),0);
    if (!wq)
        goto err;

    INIT_LIST_HEAD(&wq->worklist);

    return wq;
err:
    return NULL;
}

#define NSEC_PER_SEC    1000000000L

void getrawmonotonic(struct timespec *ts)
{
    u32 tmp = GetTimerTicks();

    ts->tv_sec  = tmp/100;
    ts->tv_nsec = (tmp - ts->tv_sec*100)*10000000;
}

void set_normalized_timespec(struct timespec *ts, time_t sec, long nsec)
{
        while (nsec >= NSEC_PER_SEC) {
                nsec -= NSEC_PER_SEC;
                ++sec;
        }
        while (nsec < 0) {
                nsec += NSEC_PER_SEC;
                --sec;
        }
        ts->tv_sec = sec;
        ts->tv_nsec = nsec;
}



