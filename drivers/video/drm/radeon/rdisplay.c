
#include <stdint.h>
#include <drm/drmP.h>
#include <drm.h>
#include <drm_mm.h>
#include "radeon_drm.h"
#include "radeon.h"
#include "radeon_object.h"

#define CURSOR_WIDTH 64
#define CURSOR_HEIGHT 64

typedef struct tag_object  kobj_t;
typedef struct tag_display display_t;

struct tag_object
{
    uint32_t   magic;
    void      *destroy;
    kobj_t    *fd;
    kobj_t    *bk;
    uint32_t   pid;
};

typedef struct
{
    kobj_t     header;

    uint32_t  *data;
    uint32_t   hot_x;
    uint32_t   hot_y;

    struct list_head      list;
    struct radeon_object *robj;
}cursor_t;

int        init_cursor(cursor_t *cursor);
cursor_t*  __stdcall select_cursor(cursor_t *cursor);
void       __stdcall move_cursor(cursor_t *cursor, int x, int y);
void       __stdcall restore_cursor(int x, int y);

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

    struct drm_device *ddev;
    struct drm_crtc   *crtc;

    struct list_head   cursors;

    cursor_t   *cursor;
    int       (*init_cursor)(cursor_t*);
    cursor_t* (__stdcall *select_cursor)(cursor_t*);
    void      (*show_cursor)(int show);
    void      (__stdcall *move_cursor)(cursor_t *cursor, int x, int y);
    void      (__stdcall *restore_cursor)(int x, int y);

};

int radeon_align_pitch(struct radeon_device *rdev, int width, int bpp, bool tiled);

static display_t *rdisplay;


void set_crtc(struct drm_crtc *crtc)
{
    ENTER();
    rdisplay->crtc = crtc;
    LEAVE();
}

int init_cursor(cursor_t *cursor)
{
    struct radeon_device *rdev;

    uint32_t *bits;
    uint32_t *src;

    int       i,j;
    int       r;

    rdev = (struct radeon_device *)rdisplay->ddev->dev_private;

    r = radeon_object_create(rdev, NULL, CURSOR_WIDTH*CURSOR_HEIGHT*4,
                     false,
                     RADEON_GEM_DOMAIN_VRAM,
                     false, &cursor->robj);

    if (unlikely(r != 0))
        return r;

    radeon_object_pin(cursor->robj, RADEON_GEM_DOMAIN_VRAM, NULL);

    r = radeon_object_kmap(cursor->robj, &bits);
    if (r) {
         DRM_ERROR("radeon: failed to map cursor (%d).\n", r);
         return r;
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

    radeon_object_kunmap(cursor->robj);

    return 0;
};

static void radeon_show_cursor(struct drm_crtc *crtc)
{
    struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);
    struct radeon_device *rdev = crtc->dev->dev_private;

    if (ASIC_IS_AVIVO(rdev)) {
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


static void radeon_lock_cursor(struct drm_crtc *crtc, bool lock)
{
    struct radeon_device *rdev = crtc->dev->dev_private;
    struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);
    uint32_t cur_lock;

    if (ASIC_IS_AVIVO(rdev)) {
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

cursor_t* __stdcall select_cursor(cursor_t *cursor)
{
    struct radeon_device *rdev;
    struct radeon_crtc   *radeon_crtc;
    cursor_t *old;
    uint32_t  gpu_addr;

    rdev = (struct radeon_device *)rdisplay->ddev->dev_private;
    radeon_crtc = to_radeon_crtc(rdisplay->crtc);

    old = rdisplay->cursor;

    rdisplay->cursor = cursor;
    gpu_addr = cursor->robj->gpu_addr;

    if (ASIC_IS_AVIVO(rdev))
        WREG32(AVIVO_D1CUR_SURFACE_ADDRESS + radeon_crtc->crtc_offset, gpu_addr);
    else {
        radeon_crtc->legacy_cursor_offset = gpu_addr - radeon_crtc->legacy_display_base_addr;
        /* offset is from DISP(2)_BASE_ADDRESS */
        WREG32(RADEON_CUR_OFFSET + radeon_crtc->crtc_offset, radeon_crtc->legacy_cursor_offset);
    }

    return old;
};


void __stdcall move_cursor(cursor_t *cursor, int x, int y)
{
    struct radeon_device *rdev;
    rdev = (struct radeon_device *)rdisplay->ddev->dev_private;
    struct drm_crtc *crtc = rdisplay->crtc;
    struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);

    int hot_x = cursor->hot_x;
    int hot_y = cursor->hot_y;

    radeon_lock_cursor(crtc, true);
    if (ASIC_IS_AVIVO(rdev))
    {
        int w = 32;
        int i = 0;
        struct drm_crtc *crtc_p;

        /* avivo cursor are offset into the total surface */
//        x += crtc->x;
//        y += crtc->y;

//        DRM_DEBUG("x %d y %d c->x %d c->y %d\n", x, y, crtc->x, crtc->y);
#if 0
        /* avivo cursor image can't end on 128 pixel boundry or
         * go past the end of the frame if both crtcs are enabled
         */
        list_for_each_entry(crtc_p, &crtc->dev->mode_config.crtc_list, head) {
            if (crtc_p->enabled)
                i++;
        }
        if (i > 1) {
            int cursor_end, frame_end;

            cursor_end = x + w;
            frame_end = crtc->x + crtc->mode.crtc_hdisplay;
            if (cursor_end >= frame_end) {
                w = w - (cursor_end - frame_end);
                if (!(frame_end & 0x7f))
                    w--;
            } else {
                if (!(cursor_end & 0x7f))
                    w--;
            }
            if (w <= 0)
                w = 1;
        }
#endif
        WREG32(AVIVO_D1CUR_POSITION + radeon_crtc->crtc_offset,
               (x << 16) | y);
        WREG32(AVIVO_D1CUR_HOT_SPOT + radeon_crtc->crtc_offset,
               (hot_x << 16) | hot_y);
        WREG32(AVIVO_D1CUR_SIZE + radeon_crtc->crtc_offset,
               ((w - 1) << 16) | 31);
    } else {
        if (crtc->mode.flags & DRM_MODE_FLAG_DBLSCAN)
            y *= 2;

        WREG32(RADEON_CUR_HORZ_VERT_OFF + radeon_crtc->crtc_offset,
               (RADEON_CUR_LOCK | (hot_x << 16) | (hot_y << 16)));
        WREG32(RADEON_CUR_HORZ_VERT_POSN + radeon_crtc->crtc_offset,
               (RADEON_CUR_LOCK | (x << 16) | y));

        /* offset is from DISP(2)_BASE_ADDRESS */
        WREG32(RADEON_CUR_OFFSET + radeon_crtc->crtc_offset,
         (radeon_crtc->legacy_cursor_offset + (hot_y * 256)));
    }
    radeon_lock_cursor(crtc, false);
}

void __stdcall restore_cursor(int x, int y)
{
};

static char *manufacturer_name(unsigned char *x)
{
    static char name[4];

    name[0] = ((x[0] & 0x7C) >> 2) + '@';
    name[1] = ((x[0] & 0x03) << 3) + ((x[1] & 0xE0) >> 5) + '@';
    name[2] = (x[1] & 0x1F) + '@';
    name[3] = 0;

    return name;
}

bool set_mode(struct drm_device *dev, int width, int height)
{
    struct drm_connector *connector;

    bool ret = false;

    ENTER();

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        struct drm_display_mode *mode;

        struct drm_encoder  *encoder;
        struct drm_crtc     *crtc;

        if( connector->status != connector_status_connected)
            continue;

        encoder = connector->encoder;
        if( encoder == NULL)
            continue;

        crtc = encoder->crtc;

        if(crtc == NULL)
            continue;

        list_for_each_entry(mode, &connector->modes, head)
        {
            char *con_name, *enc_name;

            struct drm_framebuffer *fb;

            if (drm_mode_width(mode) == width &&
                drm_mode_height(mode) == height)
            {
                char con_edid[128];

                fb = list_first_entry(&dev->mode_config.fb_kernel_list,
                                      struct drm_framebuffer, filp_head);

                memcpy(con_edid, connector->edid_blob_ptr->data, 128);

                dbgprintf("Manufacturer: %s Model %x Serial Number %u\n",
                manufacturer_name(con_edid + 0x08),
                (unsigned short)(con_edid[0x0A] + (con_edid[0x0B] << 8)),
                (unsigned int)(con_edid[0x0C] + (con_edid[0x0D] << 8)
                    + (con_edid[0x0E] << 16) + (con_edid[0x0F] << 24)));


                con_name = drm_get_connector_name(connector);
                enc_name = drm_get_encoder_name(encoder);

                dbgprintf("set mode %d %d connector %s encoder %s\n",
                           width, height, con_name, enc_name);

                fb->width = width;
                fb->height = height;
                fb->pitch = radeon_align_pitch(dev->dev_private, width, 32, false) * ((32 + 1) / 8);

                crtc->fb = fb;
                crtc->enabled = true;
                rdisplay->crtc = crtc;

                ret = drm_crtc_helper_set_mode(crtc, mode, 0, 0, fb);

                rdisplay->width  = fb->width;
                rdisplay->height = fb->height;
                rdisplay->pitch  = fb->pitch;

                sysSetScreen(fb->width, fb->height, fb->pitch);

                if (ret == true)
                {
                    dbgprintf("new mode %d %d pitch %d\n",fb->width, fb->height, fb->pitch);
                }
                else
                {
                    DRM_ERROR("failed to set mode %d_%d on crtc %p\n",
                               fb->width, fb->height, crtc);
                };

                LEAVE();

                return ret;
            };
        }
    };
    LEAVE();
    return ret;
};


int init_display(struct radeon_device *rdev, mode_t *usermode)
{
    cursor_t  *cursor;

    ENTER();

    rdisplay = GetDisplay();

    rdisplay->ddev = rdev->ddev;

    list_for_each_entry(cursor, &rdisplay->cursors, list)
    {
        init_cursor(cursor);
    };

    if( (usermode->width != 0) &&
        (usermode->height != 0) )
    {
        set_mode(rdev->ddev, usermode->width, usermode->height);
    }
    else
        set_mode(rdev->ddev, 800, 600);

    select_cursor(rdisplay->cursor);
    radeon_show_cursor(rdisplay->crtc);

    rdisplay->init_cursor   = init_cursor;
    rdisplay->select_cursor = select_cursor;
    rdisplay->show_cursor   = NULL;
    rdisplay->move_cursor   = move_cursor;
    rdisplay->restore_cursor = restore_cursor;

    LEAVE();

    return 1;
};

static int my_atoi(char **cmd)
{
    char* p = *cmd;
    int val = 0;

    for (;; *p++) {
        switch (*p) {
        case '0' ... '9':
            val = 10*val+(*p-'0');
            break;
        default:
            *cmd = p;
            return val;
        }
    }
}

char* parse_mode(char *p, mode_t *mode)
{
    char c;

    while( (c = *p++) == ' ');

    if( c )
    {
        p--;

        mode->width = my_atoi(&p);
        p++;

        mode->height = my_atoi(&p);
        p++;

        mode->freq = my_atoi(&p);
    }

    return p;
};

char* parse_path(char *p, char *log)
{
    char  c;

    while( (c = *p++) == ' ');
    p--;
    while( (c = *log++ = *p++) && (c != ' '));
    *log = 0;

    return p;
};

void parse_cmdline(char *cmdline, mode_t *mode, char *log)
{
    char *p = cmdline;

    char c = *p++;

    while( c )
    {
        if( c == '-')
        {
            switch(*p++)
            {
                case 'm':
                    p = parse_mode(p, mode);
                    break;

                case 'l':
                    p = parse_path(p, log);
                    break;
            };
        };
        c = *p++;
    };
};

