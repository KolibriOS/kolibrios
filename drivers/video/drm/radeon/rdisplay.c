
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
    cursor_t* (*select_cursor)(display_t*, cursor_t*);
    void      (*show_cursor)(int show);
    void      (*move_cursor)(int x, int y);
};


display_t *rdisplay;

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
        for(j = 0; j < CURSOR_WIDTH-32; j++)
            *bits++ = 0;
    }
    for(i = 0; i < CURSOR_WIDTH*(CURSOR_HEIGHT-32); i++)
        *bits++ = 0;

    radeon_object_kunmap(cursor->robj);

    return 0;
};

int init_display(struct radeon_device *rdev)
{
    cursor_t  *cursor;

//    rdisplay = get_display();

    rdisplay->ddev = rdev->ddev;

    list_for_each_entry(cursor, &rdisplay->cursors, list)
    {
        init_cursor(cursor);
    };
    return 1;
};

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

cursor_t* select_cursor(display_t *display, cursor_t *cursor)
{
    struct radeon_device *rdev;
    struct radeon_crtc   *radeon_crtc;
    cursor_t *old;
    uint32_t  gpu_addr;

    rdev = (struct radeon_device *)rdisplay->ddev->dev_private;
    radeon_crtc = to_radeon_crtc(rdisplay->crtc);

    old = display->cursor;

    display->cursor = cursor;
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


int radeon_cursor_move(display_t *display, int x, int y)
{
    struct drm_crtc *crtc = rdisplay->crtc;
    struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);
    struct radeon_device *rdev = crtc->dev->dev_private;

    int hot_x = rdisplay->cursor->hot_x - 1;
    int hot_y = rdisplay->cursor->hot_y - 1;

    radeon_lock_cursor(crtc, true);
    if (ASIC_IS_AVIVO(rdev))
    {
        int w = 32;
        int i = 0;
        struct drm_crtc *crtc_p;

        /* avivo cursor are offset into the total surface */
        x += crtc->x;
        y += crtc->y;

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
               (hot_x << 16) | hot_y-1);
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

    return 0;
}


