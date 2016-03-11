
#include <drm/drmP.h>
#include <drm/radeon_drm.h>
#include "radeon.h"
#include "radeon_object.h"

#include <display.h>

#include "r100d.h"


display_t *os_display;

static cursor_t*  __stdcall select_cursor(cursor_t *cursor);
static void       __stdcall move_cursor(cursor_t *cursor, int x, int y);

extern void __attribute__((regparm(1))) destroy_cursor(cursor_t *cursor);

void disable_mouse(void)
{};

int init_cursor(cursor_t *cursor)
{
    struct radeon_device *rdev;

    uint32_t *bits;
    uint32_t *src;

    int       i,j;
    int       r;

    rdev = (struct radeon_device *)os_display->ddev->dev_private;

    r = radeon_bo_create(rdev, CURSOR_WIDTH*CURSOR_HEIGHT*4,
                     4096, false, RADEON_GEM_DOMAIN_VRAM, 0, NULL, NULL, (struct radeon_bo**)&cursor->cobj);

    if (unlikely(r != 0))
        return r;

    r = radeon_bo_reserve(cursor->cobj, false);
    if (unlikely(r != 0))
        return r;

    r = radeon_bo_pin(cursor->cobj, RADEON_GEM_DOMAIN_VRAM, NULL);
    if (unlikely(r != 0))
        return r;

    r = radeon_bo_kmap(cursor->cobj, (void**)&bits);
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

    radeon_bo_kunmap(cursor->cobj);

 //   cursor->header.destroy = destroy_cursor;

    return 0;
};

void __attribute__((regparm(1))) destroy_cursor(cursor_t *cursor)
{
    list_del(&cursor->list);
    radeon_bo_unpin(cursor->cobj);
    KernelFree(cursor->data);
    __DestroyObject(cursor);
};

static void radeon_show_cursor()
{
    struct radeon_device *rdev = (struct radeon_device *)os_display->ddev->dev_private;


    if (ASIC_IS_DCE4(rdev)) {
        WREG32(RADEON_MM_INDEX, EVERGREEN_CUR_CONTROL);
        WREG32(RADEON_MM_DATA, EVERGREEN_CURSOR_EN |
               EVERGREEN_CURSOR_MODE(EVERGREEN_CURSOR_24_8_PRE_MULT));
    } else if (ASIC_IS_AVIVO(rdev)) {
        WREG32(RADEON_MM_INDEX, AVIVO_D1CUR_CONTROL);
        WREG32(RADEON_MM_DATA, AVIVO_D1CURSOR_EN |
                 (AVIVO_D1CURSOR_MODE_24BPP << AVIVO_D1CURSOR_MODE_SHIFT));
    } else {
        WREG32(RADEON_MM_INDEX, RADEON_CRTC_GEN_CNTL);
        WREG32_P(RADEON_MM_DATA, (RADEON_CRTC_CUR_EN |
                      (RADEON_CRTC_CUR_MODE_24BPP << RADEON_CRTC_CUR_MODE_SHIFT)),
             ~(RADEON_CRTC_CUR_EN | RADEON_CRTC_CUR_MODE_MASK));
    }
}

cursor_t* __stdcall select_cursor(cursor_t *cursor)
{
    struct radeon_device *rdev;
    cursor_t *old;
    uint32_t  gpu_addr;

    rdev = (struct radeon_device *)os_display->ddev->dev_private;

    old = os_display->cursor;

    os_display->cursor = cursor;
    gpu_addr = radeon_bo_gpu_offset(cursor->cobj);

    if (ASIC_IS_DCE4(rdev))
    {
        WREG32(EVERGREEN_CUR_SURFACE_ADDRESS_HIGH, 0);
        WREG32(EVERGREEN_CUR_SURFACE_ADDRESS, gpu_addr);
    }
    else if (ASIC_IS_AVIVO(rdev))
    {
        if (rdev->family >= CHIP_RV770)
            WREG32(R700_D1CUR_SURFACE_ADDRESS_HIGH, 0);
        WREG32(AVIVO_D1CUR_SURFACE_ADDRESS,  gpu_addr);
    }
    else {
        WREG32(RADEON_CUR_OFFSET, gpu_addr - rdev->mc.vram_start);
    }

    return old;
};

static void radeon_lock_cursor(bool lock)
{
    struct radeon_device *rdev;

    rdev = (struct radeon_device *)os_display->ddev->dev_private;

    uint32_t cur_lock;

	if (ASIC_IS_DCE4(rdev)) {
		cur_lock = RREG32(EVERGREEN_CUR_UPDATE);
		if (lock)
			cur_lock |= EVERGREEN_CURSOR_UPDATE_LOCK;
		else
			cur_lock &= ~EVERGREEN_CURSOR_UPDATE_LOCK;
        WREG32(EVERGREEN_CUR_UPDATE, cur_lock);
	} else if (ASIC_IS_AVIVO(rdev)) {
        cur_lock = RREG32(AVIVO_D1CUR_UPDATE);
        if (lock)
            cur_lock |= AVIVO_D1CURSOR_UPDATE_LOCK;
        else
            cur_lock &= ~AVIVO_D1CURSOR_UPDATE_LOCK;
        WREG32(AVIVO_D1CUR_UPDATE, cur_lock);
    } else {
        cur_lock = RREG32(RADEON_CUR_OFFSET);
        if (lock)
            cur_lock |= RADEON_CUR_LOCK;
        else
            cur_lock &= ~RADEON_CUR_LOCK;
        WREG32(RADEON_CUR_OFFSET, cur_lock);
    }
}


void __stdcall move_cursor(cursor_t *cursor, int x, int y)
{
    struct radeon_device *rdev;
    rdev = (struct radeon_device *)os_display->ddev->dev_private;

    int hot_x = cursor->hot_x;
    int hot_y = cursor->hot_y;
    int w     = 32;

    radeon_lock_cursor(true);

    if (ASIC_IS_DCE4(rdev)) {
        WREG32(EVERGREEN_CUR_POSITION,(x << 16) | y);
        WREG32(EVERGREEN_CUR_HOT_SPOT, (hot_x << 16) | hot_y);
        WREG32(EVERGREEN_CUR_SIZE, ((w - 1) << 16) | 31);
    } else if (ASIC_IS_AVIVO(rdev)) {
        WREG32(AVIVO_D1CUR_POSITION, (x << 16) | y);
        WREG32(AVIVO_D1CUR_HOT_SPOT, (hot_x << 16) | hot_y);
        WREG32(AVIVO_D1CUR_SIZE, ((w - 1) << 16) | 31);
    } else {

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
    radeon_lock_cursor(false);
}

void __stdcall restore_cursor(int x, int y)
{
};


bool init_display(struct radeon_device *rdev, videomode_t *usermode)
{
    struct drm_device   *dev;

    cursor_t *cursor;
    bool      retval = true;
    u32       ifl;

    ENTER();

    os_display = GetDisplay();

    dev = os_display->ddev = rdev->ddev;

    ifl = safe_cli();
    {
        list_for_each_entry(cursor, &os_display->cursors, list)
        {
            init_cursor(cursor);
        };

        os_display->restore_cursor(0,0);
        os_display->init_cursor    = init_cursor;
        os_display->select_cursor  = select_cursor;
        os_display->show_cursor    = NULL;
        os_display->move_cursor    = move_cursor;
        os_display->restore_cursor = restore_cursor;
        os_display->disable_mouse  = disable_mouse;

        select_cursor(os_display->cursor);
        radeon_show_cursor();
    };
    safe_sti(ifl);

//    init_bitmaps();

    LEAVE();

    return retval;
};


struct fb_info *framebuffer_alloc(size_t size, struct device *dev)
{
#define BYTES_PER_LONG (BITS_PER_LONG/8)
#define PADDING (BYTES_PER_LONG - (sizeof(struct fb_info) % BYTES_PER_LONG))
    int fb_info_size = sizeof(struct fb_info);
    struct fb_info *info;
    char *p;

    if (size)
        fb_info_size += PADDING;

    p = kzalloc(fb_info_size + size, GFP_KERNEL);

    if (!p)
        return NULL;

    info = (struct fb_info *) p;

    if (size)
        info->par = p + fb_info_size;

    return info;
#undef PADDING
#undef BYTES_PER_LONG
}

void framebuffer_release(struct fb_info *info)
{
    kfree(info);
}


/* 23 bits of float fractional data */
#define I2F_FRAC_BITS  23
#define I2F_MASK ((1 << I2F_FRAC_BITS) - 1)

/*
 * Converts unsigned integer into 32-bit IEEE floating point representation.
 * Will be exact from 0 to 2^24.  Above that, we round towards zero
 * as the fractional bits will not fit in a float.  (It would be better to
 * round towards even as the fpu does, but that is slower.)
 */
__pure uint32_t int2float(uint32_t x)
{
    uint32_t msb, exponent, fraction;

    /* Zero is special */
    if (!x) return 0;

    /* Get location of the most significant bit */
    msb = __fls(x);

    /*
     * Use a rotate instead of a shift because that works both leftwards
     * and rightwards due to the mod(32) behaviour.  This means we don't
     * need to check to see if we are above 2^24 or not.
     */
    fraction = ror32(x, (msb - I2F_FRAC_BITS) & 0x1f) & I2F_MASK;
    exponent = (127 + msb) << I2F_FRAC_BITS;

    return fraction + exponent;
}

