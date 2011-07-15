
#include <drm/drmP.h>
#include <drm.h>
#include <drm_mm.h>
#include "radeon_drm.h"
#include "radeon.h"
#include "radeon_object.h"
#include "display.h"

#include "r100d.h"


display_t *rdisplay;

static cursor_t*  __stdcall select_cursor(cursor_t *cursor);
static void       __stdcall move_cursor(cursor_t *cursor, int x, int y);

extern void destroy_cursor(void);

void disable_mouse(void)
{};

int init_cursor(cursor_t *cursor)
{
    struct radeon_device *rdev;

    uint32_t *bits;
    uint32_t *src;

    int       i,j;
    int       r;

    rdev = (struct radeon_device *)rdisplay->ddev->dev_private;

    r = radeon_bo_create(rdev, CURSOR_WIDTH*CURSOR_HEIGHT*4,
                     PAGE_SIZE, false, RADEON_GEM_DOMAIN_VRAM, &cursor->robj);

    if (unlikely(r != 0))
        return r;

    r = radeon_bo_reserve(cursor->robj, false);
    if (unlikely(r != 0))
        return r;

    r = radeon_bo_pin(cursor->robj, RADEON_GEM_DOMAIN_VRAM, NULL);
    if (unlikely(r != 0))
        return r;

    r = radeon_bo_kmap(cursor->robj, (void**)&bits);
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

    radeon_bo_kunmap(cursor->robj);

 //   cursor->header.destroy = destroy_cursor;

    return 0;
};

void fini_cursor(cursor_t *cursor)
{
    list_del(&cursor->list);
    radeon_bo_unpin(cursor->robj);
    KernelFree(cursor->data);
    __DestroyObject(cursor);
};

static void radeon_show_cursor()
{
    struct radeon_device *rdev = (struct radeon_device *)rdisplay->ddev->dev_private;


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

    rdev = (struct radeon_device *)rdisplay->ddev->dev_private;

    old = rdisplay->cursor;

    rdisplay->cursor = cursor;
    gpu_addr = radeon_bo_gpu_offset(cursor->robj);

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

    rdev = (struct radeon_device *)rdisplay->ddev->dev_private;

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
    rdev = (struct radeon_device *)rdisplay->ddev->dev_private;

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

        gpu_addr = radeon_bo_gpu_offset(cursor->robj);

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

    cursor_t            *cursor;
    bool                 retval = true;
    u32_t                ifl;

    ENTER();

    rdisplay = GetDisplay();

    dev = rdisplay->ddev = rdev->ddev;

    ifl = safe_cli();
    {
        list_for_each_entry(cursor, &rdisplay->cursors, list)
        {
            init_cursor(cursor);
        };

        rdisplay->restore_cursor(0,0);
        rdisplay->init_cursor    = init_cursor;
        rdisplay->select_cursor  = select_cursor;
        rdisplay->show_cursor    = NULL;
        rdisplay->move_cursor    = move_cursor;
        rdisplay->restore_cursor = restore_cursor;
        rdisplay->disable_mouse  = disable_mouse;

        select_cursor(rdisplay->cursor);
        radeon_show_cursor();
    };
    safe_sti(ifl);

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

#if 0

#define PACKET3_PAINT_MULTI             0x9A
#       define R5XX_GMC_CLR_CMP_CNTL_DIS        (1    << 28)
#       define R5XX_GMC_WR_MSK_DIS              (1    << 30)
#       define R5XX_ROP3_P                0x00f00000

#define R5XX_SC_TOP_LEFT                  0x16ec
#define R5XX_SC_BOTTOM_RIGHT              0x16f0
#       define R5XX_SC_SIGN_MASK_LO       0x8000
#       define R5XX_SC_SIGN_MASK_HI       0x80000000

#define R5XX_DEFAULT_SC_BOTTOM_RIGHT      0x16e8
#       define R5XX_DEFAULT_SC_RIGHT_MAX  (0x1fff <<  0)
#       define R5XX_DEFAULT_SC_BOTTOM_MAX (0x1fff << 16)


int r100_2D_test(struct radeon_device *rdev)
{

    uint32_t   pitch;
    uint32_t   offset;

    int        r;

    ENTER();

    pitch  = (1024*4)/64;
    offset = rdev->mc.vram_start;

    r = radeon_ring_lock(rdev, 16);
    if (r) {
        DRM_ERROR("radeon: cp failed to lock ring (%d).\n", r);
        return r;
    }
    radeon_ring_write(rdev, PACKET0(R5XX_SC_TOP_LEFT, 0));
    radeon_ring_write(rdev, 0);

    radeon_ring_write(rdev, PACKET0(R5XX_SC_BOTTOM_RIGHT, 0));
    radeon_ring_write(rdev, RADEON_DEFAULT_SC_RIGHT_MAX |
                            RADEON_DEFAULT_SC_BOTTOM_MAX);

    radeon_ring_write(rdev, PACKET0(R5XX_DEFAULT_SC_BOTTOM_RIGHT, 0));
    radeon_ring_write(rdev, RADEON_DEFAULT_SC_RIGHT_MAX |
                            RADEON_DEFAULT_SC_BOTTOM_MAX);

    radeon_ring_write(rdev, PACKET3(PACKET3_PAINT_MULTI, 4));
    radeon_ring_write(rdev, RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                            RADEON_GMC_BRUSH_SOLID_COLOR      |
                            RADEON_GMC_DST_32BPP              |
                            RADEON_GMC_SRC_DATATYPE_COLOR     |
                            R5XX_GMC_CLR_CMP_CNTL_DIS         |
                            R5XX_GMC_WR_MSK_DIS               |
                            R5XX_ROP3_P);

    radeon_ring_write(rdev, (pitch<<22)|(offset>>10));
    radeon_ring_write(rdev, 0x0000FF00);
    radeon_ring_write(rdev, (64<<16)|64);
    radeon_ring_write(rdev, (128<<16)|128);

    radeon_ring_write(rdev, PACKET0(RADEON_DSTCACHE_CTLSTAT, 0));
    radeon_ring_write(rdev, RADEON_RB2D_DC_FLUSH_ALL);
    radeon_ring_write(rdev, PACKET0(RADEON_WAIT_UNTIL, 0));
    radeon_ring_write(rdev, RADEON_WAIT_2D_IDLECLEAN |
                            RADEON_WAIT_HOST_IDLECLEAN |
                            RADEON_WAIT_DMA_GUI_IDLE);

    radeon_ring_unlock_commit(rdev);

    LEAVE();
    return r;
}


#include "r600_reg_auto_r6xx.h"
#include "r600_reg_r6xx.h"
#include "r600d.h"

const u32 r6xx_default_state[] =
{
    0xc0002400,
    0x00000000,
    0xc0012800,
    0x80000000,
    0x80000000,
    0xc0004600,
    0x00000016,
    0xc0016800,
    0x00000010,
    0x00028000,
    0xc0016800,
    0x00000010,
    0x00008000,
    0xc0016800,
    0x00000542,
    0x07000003,
    0xc0016800,
    0x000005c5,
    0x00000000,
    0xc0016800,
    0x00000363,
    0x00000000,
    0xc0016800,
    0x0000060c,
    0x82000000,
    0xc0016800,
    0x0000060e,
    0x01020204,
    0xc0016f00,
    0x00000000,
    0x00000000,
    0xc0016f00,
    0x00000001,
    0x00000000,
    0xc0096900,
    0x0000022a,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0xc0016900,
    0x00000004,
    0x00000000,
    0xc0016900,
    0x0000000a,
    0x00000000,
    0xc0016900,
    0x0000000b,
    0x00000000,
    0xc0016900,
    0x0000010c,
    0x00000000,
    0xc0016900,
    0x0000010d,
    0x00000000,
    0xc0016900,
    0x00000200,
    0x00000000,
    0xc0016900,
    0x00000343,
    0x00000060,
    0xc0016900,
    0x00000344,
    0x00000040,
    0xc0016900,
    0x00000351,
    0x0000aa00,
    0xc0016900,
    0x00000104,
    0x00000000,
    0xc0016900,
    0x0000010e,
    0x00000000,
    0xc0046900,
    0x00000105,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0xc0036900,
    0x00000109,
    0x00000000,
    0x00000000,
    0x00000000,
    0xc0046900,
    0x0000030c,
    0x01000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0xc0046900,
    0x00000048,
    0x3f800000,
    0x00000000,
    0x3f800000,
    0x3f800000,
    0xc0016900,
    0x0000008e,
    0x0000000f,
    0xc0016900,
    0x00000080,
    0x00000000,
    0xc0016900,
    0x00000083,
    0x0000ffff,
    0xc0016900,
    0x00000084,
    0x00000000,
    0xc0016900,
    0x00000085,
    0x20002000,
    0xc0016900,
    0x00000086,
    0x00000000,
    0xc0016900,
    0x00000087,
    0x20002000,
    0xc0016900,
    0x00000088,
    0x00000000,
    0xc0016900,
    0x00000089,
    0x20002000,
    0xc0016900,
    0x0000008a,
    0x00000000,
    0xc0016900,
    0x0000008b,
    0x20002000,
    0xc0016900,
    0x0000008c,
    0x00000000,
    0xc0016900,
    0x00000094,
    0x80000000,
    0xc0016900,
    0x00000095,
    0x20002000,
    0xc0026900,
    0x000000b4,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x00000096,
    0x80000000,
    0xc0016900,
    0x00000097,
    0x20002000,
    0xc0026900,
    0x000000b6,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x00000098,
    0x80000000,
    0xc0016900,
    0x00000099,
    0x20002000,
    0xc0026900,
    0x000000b8,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x0000009a,
    0x80000000,
    0xc0016900,
    0x0000009b,
    0x20002000,
    0xc0026900,
    0x000000ba,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x0000009c,
    0x80000000,
    0xc0016900,
    0x0000009d,
    0x20002000,
    0xc0026900,
    0x000000bc,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x0000009e,
    0x80000000,
    0xc0016900,
    0x0000009f,
    0x20002000,
    0xc0026900,
    0x000000be,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000a0,
    0x80000000,
    0xc0016900,
    0x000000a1,
    0x20002000,
    0xc0026900,
    0x000000c0,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000a2,
    0x80000000,
    0xc0016900,
    0x000000a3,
    0x20002000,
    0xc0026900,
    0x000000c2,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000a4,
    0x80000000,
    0xc0016900,
    0x000000a5,
    0x20002000,
    0xc0026900,
    0x000000c4,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000a6,
    0x80000000,
    0xc0016900,
    0x000000a7,
    0x20002000,
    0xc0026900,
    0x000000c6,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000a8,
    0x80000000,
    0xc0016900,
    0x000000a9,
    0x20002000,
    0xc0026900,
    0x000000c8,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000aa,
    0x80000000,
    0xc0016900,
    0x000000ab,
    0x20002000,
    0xc0026900,
    0x000000ca,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000ac,
    0x80000000,
    0xc0016900,
    0x000000ad,
    0x20002000,
    0xc0026900,
    0x000000cc,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000ae,
    0x80000000,
    0xc0016900,
    0x000000af,
    0x20002000,
    0xc0026900,
    0x000000ce,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000b0,
    0x80000000,
    0xc0016900,
    0x000000b1,
    0x20002000,
    0xc0026900,
    0x000000d0,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000b2,
    0x80000000,
    0xc0016900,
    0x000000b3,
    0x20002000,
    0xc0026900,
    0x000000d2,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x00000293,
    0x00004010,
    0xc0016900,
    0x00000300,
    0x00000000,
    0xc0016900,
    0x00000301,
    0x00000000,
    0xc0016900,
    0x00000312,
    0xffffffff,
    0xc0016900,
    0x00000307,
    0x00000000,
    0xc0016900,
    0x00000308,
    0x00000000,
    0xc0016900,
    0x00000283,
    0x00000000,
    0xc0016900,
    0x00000292,
    0x00000000,
    0xc0066900,
    0x0000010f,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0xc0016900,
    0x00000206,
    0x00000000,
    0xc0016900,
    0x00000207,
    0x00000000,
    0xc0016900,
    0x00000208,
    0x00000000,
    0xc0046900,
    0x00000303,
    0x3f800000,
    0x3f800000,
    0x3f800000,
    0x3f800000,
    0xc0016900,
    0x00000205,
    0x00000004,
    0xc0016900,
    0x00000280,
    0x00000000,
    0xc0016900,
    0x00000281,
    0x00000000,
    0xc0016900,
    0x0000037e,
    0x00000000,
    0xc0016900,
    0x00000382,
    0x00000000,
    0xc0016900,
    0x00000380,
    0x00000000,
    0xc0016900,
    0x00000383,
    0x00000000,
    0xc0016900,
    0x00000381,
    0x00000000,
    0xc0016900,
    0x00000282,
    0x00000008,
    0xc0016900,
    0x00000302,
    0x0000002d,
    0xc0016900,
    0x0000037f,
    0x00000000,
    0xc0016900,
    0x000001b2,
    0x00000000,
    0xc0016900,
    0x000001b6,
    0x00000000,
    0xc0016900,
    0x000001b7,
    0x00000000,
    0xc0016900,
    0x000001b8,
    0x00000000,
    0xc0016900,
    0x000001b9,
    0x00000000,
    0xc0016900,
    0x00000225,
    0x00000000,
    0xc0016900,
    0x00000229,
    0x00000000,
    0xc0016900,
    0x00000237,
    0x00000000,
    0xc0016900,
    0x00000100,
    0x00000800,
    0xc0016900,
    0x00000101,
    0x00000000,
    0xc0016900,
    0x00000102,
    0x00000000,
    0xc0016900,
    0x000002a8,
    0x00000000,
    0xc0016900,
    0x000002a9,
    0x00000000,
    0xc0016900,
    0x00000103,
    0x00000000,
    0xc0016900,
    0x00000284,
    0x00000000,
    0xc0016900,
    0x00000290,
    0x00000000,
    0xc0016900,
    0x00000285,
    0x00000000,
    0xc0016900,
    0x00000286,
    0x00000000,
    0xc0016900,
    0x00000287,
    0x00000000,
    0xc0016900,
    0x00000288,
    0x00000000,
    0xc0016900,
    0x00000289,
    0x00000000,
    0xc0016900,
    0x0000028a,
    0x00000000,
    0xc0016900,
    0x0000028b,
    0x00000000,
    0xc0016900,
    0x0000028c,
    0x00000000,
    0xc0016900,
    0x0000028d,
    0x00000000,
    0xc0016900,
    0x0000028e,
    0x00000000,
    0xc0016900,
    0x0000028f,
    0x00000000,
    0xc0016900,
    0x000002a1,
    0x00000000,
    0xc0016900,
    0x000002a5,
    0x00000000,
    0xc0016900,
    0x000002ac,
    0x00000000,
    0xc0016900,
    0x000002ad,
    0x00000000,
    0xc0016900,
    0x000002ae,
    0x00000000,
    0xc0016900,
    0x000002c8,
    0x00000000,
    0xc0016900,
    0x00000206,
    0x00000100,
    0xc0016900,
    0x00000204,
    0x00010000,
    0xc0036e00,
    0x00000000,
    0x00000012,
    0x00000000,
    0x00000000,
    0xc0016900,
    0x0000008f,
    0x0000000f,
    0xc0016900,
    0x000001e8,
    0x00000001,
    0xc0016900,
    0x00000202,
    0x00cc0000,
    0xc0016900,
    0x00000205,
    0x00000244,
    0xc0016900,
    0x00000203,
    0x00000210,
    0xc0016900,
    0x000001b1,
    0x00000000,
    0xc0016900,
    0x00000185,
    0x00000000,
    0xc0016900,
    0x000001b3,
    0x00000001,
    0xc0016900,
    0x000001b4,
    0x00000000,
    0xc0016900,
    0x00000191,
    0x00000b00,
    0xc0016900,
    0x000001b5,
    0x00000000,
};



const u32 r7xx_default_state[] =
{
    0xc0012800,
    0x80000000,
    0x80000000,
    0xc0004600,
    0x00000016,
    0xc0016800,
    0x00000010,
    0x00028000,
    0xc0016800,
    0x00000010,
    0x00008000,
    0xc0016800,
    0x00000542,
    0x07000002,
    0xc0016800,
    0x000005c5,
    0x00000000,
    0xc0016800,
    0x00000363,
    0x00004000,
    0xc0016800,
    0x0000060c,
    0x00000000,
    0xc0016800,
    0x0000060e,
    0x00420204,
    0xc0016f00,
    0x00000000,
    0x00000000,
    0xc0016f00,
    0x00000001,
    0x00000000,
    0xc0096900,
    0x0000022a,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0xc0016900,
    0x00000004,
    0x00000000,
    0xc0016900,
    0x0000000a,
    0x00000000,
    0xc0016900,
    0x0000000b,
    0x00000000,
    0xc0016900,
    0x0000010c,
    0x00000000,
    0xc0016900,
    0x0000010d,
    0x00000000,
    0xc0016900,
    0x00000200,
    0x00000000,
    0xc0016900,
    0x00000343,
    0x00000060,
    0xc0016900,
    0x00000344,
    0x00000000,
    0xc0016900,
    0x00000351,
    0x0000aa00,
    0xc0016900,
    0x00000104,
    0x00000000,
    0xc0016900,
    0x0000010e,
    0x00000000,
    0xc0046900,
    0x00000105,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0xc0046900,
    0x0000030c,
    0x01000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0xc0016900,
    0x0000008e,
    0x0000000f,
    0xc0016900,
    0x00000080,
    0x00000000,
    0xc0016900,
    0x00000083,
    0x0000ffff,
    0xc0016900,
    0x00000084,
    0x00000000,
    0xc0016900,
    0x00000085,
    0x20002000,
    0xc0016900,
    0x00000086,
    0x00000000,
    0xc0016900,
    0x00000087,
    0x20002000,
    0xc0016900,
    0x00000088,
    0x00000000,
    0xc0016900,
    0x00000089,
    0x20002000,
    0xc0016900,
    0x0000008a,
    0x00000000,
    0xc0016900,
    0x0000008b,
    0x20002000,
    0xc0016900,
    0x0000008c,
    0xaaaaaaaa,
    0xc0016900,
    0x00000094,
    0x80000000,
    0xc0016900,
    0x00000095,
    0x20002000,
    0xc0026900,
    0x000000b4,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x00000096,
    0x80000000,
    0xc0016900,
    0x00000097,
    0x20002000,
    0xc0026900,
    0x000000b6,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x00000098,
    0x80000000,
    0xc0016900,
    0x00000099,
    0x20002000,
    0xc0026900,
    0x000000b8,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x0000009a,
    0x80000000,
    0xc0016900,
    0x0000009b,
    0x20002000,
    0xc0026900,
    0x000000ba,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x0000009c,
    0x80000000,
    0xc0016900,
    0x0000009d,
    0x20002000,
    0xc0026900,
    0x000000bc,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x0000009e,
    0x80000000,
    0xc0016900,
    0x0000009f,
    0x20002000,
    0xc0026900,
    0x000000be,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000a0,
    0x80000000,
    0xc0016900,
    0x000000a1,
    0x20002000,
    0xc0026900,
    0x000000c0,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000a2,
    0x80000000,
    0xc0016900,
    0x000000a3,
    0x20002000,
    0xc0026900,
    0x000000c2,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000a4,
    0x80000000,
    0xc0016900,
    0x000000a5,
    0x20002000,
    0xc0026900,
    0x000000c4,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000a6,
    0x80000000,
    0xc0016900,
    0x000000a7,
    0x20002000,
    0xc0026900,
    0x000000c6,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000a8,
    0x80000000,
    0xc0016900,
    0x000000a9,
    0x20002000,
    0xc0026900,
    0x000000c8,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000aa,
    0x80000000,
    0xc0016900,
    0x000000ab,
    0x20002000,
    0xc0026900,
    0x000000ca,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000ac,
    0x80000000,
    0xc0016900,
    0x000000ad,
    0x20002000,
    0xc0026900,
    0x000000cc,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000ae,
    0x80000000,
    0xc0016900,
    0x000000af,
    0x20002000,
    0xc0026900,
    0x000000ce,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000b0,
    0x80000000,
    0xc0016900,
    0x000000b1,
    0x20002000,
    0xc0026900,
    0x000000d0,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x000000b2,
    0x80000000,
    0xc0016900,
    0x000000b3,
    0x20002000,
    0xc0026900,
    0x000000d2,
    0x00000000,
    0x3f800000,
    0xc0016900,
    0x00000293,
    0x00514000,
    0xc0016900,
    0x00000300,
    0x00000000,
    0xc0016900,
    0x00000301,
    0x00000000,
    0xc0016900,
    0x00000312,
    0xffffffff,
    0xc0016900,
    0x00000307,
    0x00000000,
    0xc0016900,
    0x00000308,
    0x00000000,
    0xc0016900,
    0x00000283,
    0x00000000,
    0xc0016900,
    0x00000292,
    0x00000000,
    0xc0066900,
    0x0000010f,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0xc0016900,
    0x00000206,
    0x00000000,
    0xc0016900,
    0x00000207,
    0x00000000,
    0xc0016900,
    0x00000208,
    0x00000000,
    0xc0046900,
    0x00000303,
    0x3f800000,
    0x3f800000,
    0x3f800000,
    0x3f800000,
    0xc0016900,
    0x00000205,
    0x00000004,
    0xc0016900,
    0x00000280,
    0x00000000,
    0xc0016900,
    0x00000281,
    0x00000000,
    0xc0016900,
    0x0000037e,
    0x00000000,
    0xc0016900,
    0x00000382,
    0x00000000,
    0xc0016900,
    0x00000380,
    0x00000000,
    0xc0016900,
    0x00000383,
    0x00000000,
    0xc0016900,
    0x00000381,
    0x00000000,
    0xc0016900,
    0x00000282,
    0x00000008,
    0xc0016900,
    0x00000302,
    0x0000002d,
    0xc0016900,
    0x0000037f,
    0x00000000,
    0xc0016900,
    0x000001b2,
    0x00000001,
    0xc0016900,
    0x000001b6,
    0x00000000,
    0xc0016900,
    0x000001b7,
    0x00000000,
    0xc0016900,
    0x000001b8,
    0x00000000,
    0xc0016900,
    0x000001b9,
    0x00000000,
    0xc0016900,
    0x00000225,
    0x00000000,
    0xc0016900,
    0x00000229,
    0x00000000,
    0xc0016900,
    0x00000237,
    0x00000000,
    0xc0016900,
    0x00000100,
    0x00000800,
    0xc0016900,
    0x00000101,
    0x00000000,
    0xc0016900,
    0x00000102,
    0x00000000,
    0xc0016900,
    0x000002a8,
    0x00000000,
    0xc0016900,
    0x000002a9,
    0x00000000,
    0xc0016900,
    0x00000103,
    0x00000000,
    0xc0016900,
    0x00000284,
    0x00000000,
    0xc0016900,
    0x00000290,
    0x00000000,
    0xc0016900,
    0x00000285,
    0x00000000,
    0xc0016900,
    0x00000286,
    0x00000000,
    0xc0016900,
    0x00000287,
    0x00000000,
    0xc0016900,
    0x00000288,
    0x00000000,
    0xc0016900,
    0x00000289,
    0x00000000,
    0xc0016900,
    0x0000028a,
    0x00000000,
    0xc0016900,
    0x0000028b,
    0x00000000,
    0xc0016900,
    0x0000028c,
    0x00000000,
    0xc0016900,
    0x0000028d,
    0x00000000,
    0xc0016900,
    0x0000028e,
    0x00000000,
    0xc0016900,
    0x0000028f,
    0x00000000,
    0xc0016900,
    0x000002a1,
    0x00000000,
    0xc0016900,
    0x000002a5,
    0x00000000,
    0xc0016900,
    0x000002ac,
    0x00000000,
    0xc0016900,
    0x000002ad,
    0x00000000,
    0xc0016900,
    0x000002ae,
    0x00000000,
    0xc0016900,
    0x000002c8,
    0x00000000,
    0xc0016900,
    0x00000206,
    0x00000100,
    0xc0016900,
    0x00000204,
    0x00010000,
    0xc0036e00,
    0x00000000,
    0x00000012,
    0x00000000,
    0x00000000,
    0xc0016900,
    0x0000008f,
    0x0000000f,
    0xc0016900,
    0x000001e8,
    0x00000001,
    0xc0016900,
    0x00000202,
    0x00cc0000,
    0xc0016900,
    0x00000205,
    0x00000244,
    0xc0016900,
    0x00000203,
    0x00000210,
    0xc0016900,
    0x000001b1,
    0x00000000,
    0xc0016900,
    0x00000185,
    0x00000000,
    0xc0016900,
    0x000001b3,
    0x00000001,
    0xc0016900,
    0x000001b4,
    0x00000000,
    0xc0016900,
    0x00000191,
    0x00000b00,
    0xc0016900,
    0x000001b5,
    0x00000000,
};

const u32 r6xx_default_size = ARRAY_SIZE(r6xx_default_state);
const u32 r7xx_default_size = ARRAY_SIZE(r7xx_default_state);


int R600_solid_ps(struct radeon_device *rdev, uint32_t* shader);
int R600_solid_vs(struct radeon_device *rdev, uint32_t* shader);

#define COLOR_8_8_8_8         0x1a

/* emits 21 on rv770+, 23 on r600 */
static void
set_render_target(struct radeon_device *rdev, int format,
          int w, int h, u64 gpu_addr)
{
    u32 cb_color_info;
    int pitch, slice;

    h = (h + 7) & ~7;
    if (h < 8)
        h = 8;

    cb_color_info = ((format << 2) | (1 << 27));
    pitch = (w / 8) - 1;
    slice = ((w * h) / 64) - 1;

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (CB_COLOR0_BASE - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, gpu_addr >> 8);

    if (rdev->family > CHIP_R600 && rdev->family < CHIP_RV770) {
        radeon_ring_write(rdev, PACKET3(PACKET3_SURFACE_BASE_UPDATE, 0));
        radeon_ring_write(rdev, 2 << 0);
    }

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (CB_COLOR0_SIZE - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, (pitch << 0) | (slice << 10));

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (CB_COLOR0_VIEW - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, 0);

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (CB_COLOR0_INFO - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, cb_color_info);

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (CB_COLOR0_TILE - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, 0);

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (CB_COLOR0_FRAG - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, 0);

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (CB_COLOR0_MASK - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, 0);
}


/* emits 5dw */
static void
cp_set_surface_sync(struct radeon_device *rdev,
            u32 sync_type, u32 size,
            u64 mc_addr)
{
    u32 cp_coher_size;

    if (size == 0xffffffff)
        cp_coher_size = 0xffffffff;
    else
        cp_coher_size = ((size + 255) >> 8);

    radeon_ring_write(rdev, PACKET3(PACKET3_SURFACE_SYNC, 3));
    radeon_ring_write(rdev, sync_type);
    radeon_ring_write(rdev, cp_coher_size);
    radeon_ring_write(rdev, mc_addr >> 8);
    radeon_ring_write(rdev, 10); /* poll interval */
}

/* emits 14 */
static void
set_default_state(struct radeon_device *rdev,
                  u64 state_gpu_addr, u32 state_len)
{
    u32 sq_config, sq_gpr_resource_mgmt_1, sq_gpr_resource_mgmt_2;
    u32 sq_thread_resource_mgmt, sq_stack_resource_mgmt_1, sq_stack_resource_mgmt_2;
    int num_ps_gprs, num_vs_gprs, num_temp_gprs, num_gs_gprs, num_es_gprs;
    int num_ps_threads, num_vs_threads, num_gs_threads, num_es_threads;
    int num_ps_stack_entries, num_vs_stack_entries, num_gs_stack_entries, num_es_stack_entries;
    u64 gpu_addr;
    int dwords;

    switch (rdev->family) {
    case CHIP_R600:
        num_ps_gprs = 192;
        num_vs_gprs = 56;
        num_temp_gprs = 4;
        num_gs_gprs = 0;
        num_es_gprs = 0;
        num_ps_threads = 136;
        num_vs_threads = 48;
        num_gs_threads = 4;
        num_es_threads = 4;
        num_ps_stack_entries = 128;
        num_vs_stack_entries = 128;
        num_gs_stack_entries = 0;
        num_es_stack_entries = 0;
        break;
    case CHIP_RV630:
    case CHIP_RV635:
        num_ps_gprs = 84;
        num_vs_gprs = 36;
        num_temp_gprs = 4;
        num_gs_gprs = 0;
        num_es_gprs = 0;
        num_ps_threads = 144;
        num_vs_threads = 40;
        num_gs_threads = 4;
        num_es_threads = 4;
        num_ps_stack_entries = 40;
        num_vs_stack_entries = 40;
        num_gs_stack_entries = 32;
        num_es_stack_entries = 16;
        break;
    case CHIP_RV610:
    case CHIP_RV620:
    case CHIP_RS780:
    case CHIP_RS880:
    default:
        num_ps_gprs = 84;
        num_vs_gprs = 36;
        num_temp_gprs = 4;
        num_gs_gprs = 0;
        num_es_gprs = 0;
        num_ps_threads = 136;
        num_vs_threads = 48;
        num_gs_threads = 4;
        num_es_threads = 4;
        num_ps_stack_entries = 40;
        num_vs_stack_entries = 40;
        num_gs_stack_entries = 32;
        num_es_stack_entries = 16;
        break;
    case CHIP_RV670:
        num_ps_gprs = 144;
        num_vs_gprs = 40;
        num_temp_gprs = 4;
        num_gs_gprs = 0;
        num_es_gprs = 0;
        num_ps_threads = 136;
        num_vs_threads = 48;
        num_gs_threads = 4;
        num_es_threads = 4;
        num_ps_stack_entries = 40;
        num_vs_stack_entries = 40;
        num_gs_stack_entries = 32;
        num_es_stack_entries = 16;
        break;
    case CHIP_RV770:
        num_ps_gprs = 192;
        num_vs_gprs = 56;
        num_temp_gprs = 4;
        num_gs_gprs = 0;
        num_es_gprs = 0;
        num_ps_threads = 188;
        num_vs_threads = 60;
        num_gs_threads = 0;
        num_es_threads = 0;
        num_ps_stack_entries = 256;
        num_vs_stack_entries = 256;
        num_gs_stack_entries = 0;
        num_es_stack_entries = 0;
        break;
    case CHIP_RV730:
    case CHIP_RV740:
        num_ps_gprs = 84;
        num_vs_gprs = 36;
        num_temp_gprs = 4;
        num_gs_gprs = 0;
        num_es_gprs = 0;
        num_ps_threads = 188;
        num_vs_threads = 60;
        num_gs_threads = 0;
        num_es_threads = 0;
        num_ps_stack_entries = 128;
        num_vs_stack_entries = 128;
        num_gs_stack_entries = 0;
        num_es_stack_entries = 0;
        break;
    case CHIP_RV710:
        num_ps_gprs = 192;
        num_vs_gprs = 56;
        num_temp_gprs = 4;
        num_gs_gprs = 0;
        num_es_gprs = 0;
        num_ps_threads = 144;
        num_vs_threads = 48;
        num_gs_threads = 0;
        num_es_threads = 0;
        num_ps_stack_entries = 128;
        num_vs_stack_entries = 128;
        num_gs_stack_entries = 0;
        num_es_stack_entries = 0;
        break;
    }

    if ((rdev->family == CHIP_RV610) ||
        (rdev->family == CHIP_RV620) ||
        (rdev->family == CHIP_RS780) ||
        (rdev->family == CHIP_RS880) ||
        (rdev->family == CHIP_RV710))
        sq_config = 0;
    else
        sq_config = VC_ENABLE;

    sq_config |= (DX9_CONSTS |
              ALU_INST_PREFER_VECTOR |
              PS_PRIO(0) |
              VS_PRIO(1) |
              GS_PRIO(2) |
              ES_PRIO(3));

    sq_gpr_resource_mgmt_1 = (NUM_PS_GPRS(num_ps_gprs) |
                  NUM_VS_GPRS(num_vs_gprs) |
                  NUM_CLAUSE_TEMP_GPRS(num_temp_gprs));
    sq_gpr_resource_mgmt_2 = (NUM_GS_GPRS(num_gs_gprs) |
                  NUM_ES_GPRS(num_es_gprs));
    sq_thread_resource_mgmt = (NUM_PS_THREADS(num_ps_threads) |
                   NUM_VS_THREADS(num_vs_threads) |
                   NUM_GS_THREADS(num_gs_threads) |
                   NUM_ES_THREADS(num_es_threads));
    sq_stack_resource_mgmt_1 = (NUM_PS_STACK_ENTRIES(num_ps_stack_entries) |
                    NUM_VS_STACK_ENTRIES(num_vs_stack_entries));
    sq_stack_resource_mgmt_2 = (NUM_GS_STACK_ENTRIES(num_gs_stack_entries) |
                    NUM_ES_STACK_ENTRIES(num_es_stack_entries));

    /* emit an IB pointing at default state */
    dwords   = (state_len + 0xf) & ~0xf;
    gpu_addr = state_gpu_addr;

    radeon_ring_write(rdev, PACKET3(PACKET3_INDIRECT_BUFFER, 2));
    radeon_ring_write(rdev, gpu_addr & 0xFFFFFFFC);
    radeon_ring_write(rdev, upper_32_bits(gpu_addr) & 0xFF);
    radeon_ring_write(rdev, dwords);

    radeon_ring_write(rdev, PACKET3(PACKET3_EVENT_WRITE, 0));
    radeon_ring_write(rdev, CACHE_FLUSH_AND_INV_EVENT);
    /* SQ config */
    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONFIG_REG, 6));
    radeon_ring_write(rdev, (SQ_CONFIG - PACKET3_SET_CONFIG_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, sq_config);
    radeon_ring_write(rdev, sq_gpr_resource_mgmt_1);
    radeon_ring_write(rdev, sq_gpr_resource_mgmt_2);
    radeon_ring_write(rdev, sq_thread_resource_mgmt);
    radeon_ring_write(rdev, sq_stack_resource_mgmt_1);
    radeon_ring_write(rdev, sq_stack_resource_mgmt_2);
}

/* emits 12 */
static void
set_scissors(struct radeon_device *rdev, int x1, int y1,
         int x2, int y2)
{
    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 2));
    radeon_ring_write(rdev, (PA_SC_SCREEN_SCISSOR_TL - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, (x1 << 0) | (y1 << 16));
    radeon_ring_write(rdev, (x2 << 0) | (y2 << 16));

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 2));
    radeon_ring_write(rdev, (PA_SC_GENERIC_SCISSOR_TL - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, (x1 << 0) | (y1 << 16) | (1 << 31));
    radeon_ring_write(rdev, (x2 << 0) | (y2 << 16));

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 2));
    radeon_ring_write(rdev, (PA_SC_WINDOW_SCISSOR_TL - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, (x1 << 0) | (y1 << 16) | (1 << 31));
    radeon_ring_write(rdev, (x2 << 0) | (y2 << 16));
}

static void
draw_auto(struct radeon_device *rdev)
{
    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONFIG_REG, 1));
    radeon_ring_write(rdev, (VGT_PRIMITIVE_TYPE - PACKET3_SET_CONFIG_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, DI_PT_RECTLIST);

    radeon_ring_write(rdev, PACKET3(PACKET3_INDEX_TYPE, 0));
    radeon_ring_write(rdev, DI_INDEX_SIZE_16_BIT);

    radeon_ring_write(rdev, PACKET3(PACKET3_NUM_INSTANCES, 0));
    radeon_ring_write(rdev, 1);

    radeon_ring_write(rdev, PACKET3(PACKET3_DRAW_INDEX_AUTO, 1));
    radeon_ring_write(rdev, 3);
    radeon_ring_write(rdev, DI_SRC_SEL_AUTO_INDEX);

}

/* ALU clause insts */
#define SRC0_SEL(x)        (x)
#define SRC1_SEL(x)        (x)
#define SRC2_SEL(x)        (x)
/* src[0-2]_sel */
/*   0-127 GPR */
/* 128-159 kcache constants bank 0 */
/* 160-191 kcache constants bank 1 */
/* 248-255 special SQ_ALU_SRC_* (0, 1, etc.) */

#define SRC0_REL(x)        (x)
#define SRC1_REL(x)        (x)
#define SRC2_REL(x)        (x)
/* elem */
#define SRC0_ELEM(x)        (x)
#define SRC1_ELEM(x)        (x)
#define SRC2_ELEM(x)        (x)
#define ELEM_X        0
#define ELEM_Y        1
#define ELEM_Z        2
#define ELEM_W        3
/* neg */
#define SRC0_NEG(x)        (x)
#define SRC1_NEG(x)        (x)
#define SRC2_NEG(x)        (x)
/* im */
#define INDEX_MODE(x)    (x)        /* SQ_INDEX_* */
/* ps */
#define PRED_SEL(x)      (x)        /* SQ_PRED_SEL_* */
/* last */
#define LAST(x)          (x)
/* abs */
#define SRC0_ABS(x)       (x)
#define SRC1_ABS(x)       (x)
/* uem */
#define UPDATE_EXECUTE_MASK(x) (x)
/* up */
#define UPDATE_PRED(x)      (x)
/* wm */
#define WRITE_MASK(x)   (x)
/* fm */
#define FOG_MERGE(x)    (x)
/* omod */
#define OMOD(x)        (x)      /* SQ_ALU_OMOD_* */
/* alu inst */
#define ALU_INST(x)        (x)      /* SQ_ALU_INST_* */
/*bs */
#define BANK_SWIZZLE(x)        (x)  /* SQ_ALU_VEC_* */
#define DST_GPR(x)        (x)
#define DST_REL(x)        (x)
#define DST_ELEM(x)       (x)
#define CLAMP(x)          (x)

#define ALU_DWORD0(src0_sel, s0r, s0e, s0n, src1_sel, s1r, s1e, s1n, im, ps, last) \
        (((src0_sel) << 0) | ((s0r) << 9) | ((s0e) << 10) | ((s0n) << 12) | \
         ((src1_sel) << 13) | ((s1r) << 22) | ((s1e) << 23) | ((s1n) << 25) | \
     ((im) << 26) | ((ps) << 29) | ((last) << 31))

/* R7xx has alu_inst at a different slot, and no fog merge any more (no fix function fog any more) */
#define R6xx_ALU_DWORD1_OP2(s0a, s1a, uem, up, wm, fm, omod, alu_inst, bs, dst_gpr, dr, de, clamp) \
        (((s0a) << 0) | ((s1a) << 1) | ((uem) << 2) | ((up) << 3) | ((wm) << 4) | \
         ((fm) << 5) | ((omod) << 6) | ((alu_inst) << 8) | ((bs) << 18) | ((dst_gpr) << 21) | \
     ((dr) << 28) | ((de) << 29) | ((clamp) << 31))

#define R7xx_ALU_DWORD1_OP2(s0a, s1a, uem, up, wm, omod, alu_inst, bs, dst_gpr, dr, de, clamp) \
        (((s0a) << 0) | ((s1a) << 1) | ((uem) << 2) | ((up) << 3) | ((wm) << 4) | \
         ((omod) << 5) | ((alu_inst) << 7) | ((bs) << 18) | ((dst_gpr) << 21) | \
     ((dr) << 28) | ((de) << 29) | ((clamp) << 31))

/* This is a general chipset macro, but due to selection by chipid typically not usable in static arrays */
/* Fog is NOT USED on R7xx, even if specified. */
#define ALU_DWORD1_OP2(chipid, s0a, s1a, uem, up, wm, fm, omod, alu_inst, bs, dst_gpr, dr, de, clamp) \
    ((chipid) < CHIP_RV770 ? \
     R6xx_ALU_DWORD1_OP2(s0a, s1a, uem, up, wm, fm, omod, alu_inst, bs, dst_gpr, dr, de, clamp) : \
     R7xx_ALU_DWORD1_OP2(s0a, s1a, uem, up, wm, omod, alu_inst, bs, dst_gpr, dr, de, clamp))

#define ALU_DWORD1_OP3(src2_sel, s2r, s2e, s2n, alu_inst, bs, dst_gpr, dr, de, clamp) \
        (((src2_sel) << 0) | ((s2r) << 9) | ((s2e) << 10) | ((s2n) << 12) | \
         ((alu_inst) << 13) | ((bs) << 18) | ((dst_gpr) << 21) | ((dr) << 28) | \
     ((de) << 29) | ((clamp) << 31))

/* CF insts */
/* addr */
#define ADDR(x)  (x)
/* pc */
#define POP_COUNT(x)      (x)
/* const */
#define CF_CONST(x)       (x)
/* cond */
#define COND(x)        (x)      /* SQ_COND_* */
/* count */
#define I_COUNT(x)        ((x) ? ((x) - 1) : 0)
/*r7xx */
#define COUNT_3(x)        (x)
/* call count */
#define CALL_COUNT(x)     (x)
/* eop */
#define END_OF_PROGRAM(x)   (x)
/* vpm */
#define VALID_PIXEL_MODE(x) (x)
/* cf inst */
#define CF_INST(x)        (x)       /* SQ_CF_INST_* */

/* wqm */
#define WHOLE_QUAD_MODE(x)  (x)
/* barrier */
#define BARRIER(x)          (x)
/*kb0 */
#define KCACHE_BANK0(x)          (x)
/*kb1 */
#define KCACHE_BANK1(x)          (x)
/* km0/1 */
#define KCACHE_MODE0(x)          (x)
#define KCACHE_MODE1(x)          (x)    /* SQ_CF_KCACHE_* */
/* */
#define KCACHE_ADDR0(x)          (x)
#define KCACHE_ADDR1(x)          (x)
/* uw */
#define USES_WATERFALL(x)        (x)

#define ARRAY_BASE(x)        (x)
/* export pixel */
#define CF_PIXEL_MRT0         0
#define CF_PIXEL_MRT1         1
#define CF_PIXEL_MRT2         2
#define CF_PIXEL_MRT3         3
#define CF_PIXEL_MRT4         4
#define CF_PIXEL_MRT5         5
#define CF_PIXEL_MRT6         6
#define CF_PIXEL_MRT7         7
/* *_FOG: r6xx only */
#define CF_PIXEL_MRT0_FOG     16
#define CF_PIXEL_MRT1_FOG     17
#define CF_PIXEL_MRT2_FOG     18
#define CF_PIXEL_MRT3_FOG     19
#define CF_PIXEL_MRT4_FOG     20
#define CF_PIXEL_MRT5_FOG     21
#define CF_PIXEL_MRT6_FOG     22
#define CF_PIXEL_MRT7_FOG     23
#define CF_PIXEL_Z            61
/* export pos */
#define CF_POS0               60
#define CF_POS1               61
#define CF_POS2               62
#define CF_POS3               63
/* export param */
/* 0...31 */
#define TYPE(x)              (x)    /* SQ_EXPORT_* */
#if 0
/* type export */
#define SQ_EXPORT_PIXEL              0
#define SQ_EXPORT_POS                1
#define SQ_EXPORT_PARAM              2
/* reserved 3 */
/* type mem */
#define SQ_EXPORT_WRITE              0
#define SQ_EXPORT_WRITE_IND          1
#define SQ_EXPORT_WRITE_ACK          2
#define SQ_EXPORT_WRITE_IND_ACK      3
#endif

#define RW_GPR(x)            (x)
#define RW_REL(x)            (x)
#define ABSOLUTE                  0
#define RELATIVE                  1
#define INDEX_GPR(x)            (x)
#define ELEM_SIZE(x)            (x ? (x - 1) : 0)
#define COMP_MASK(x)            (x)
#define R6xx_ELEM_LOOP(x)            (x)
#define BURST_COUNT(x)          (x ? (x - 1) : 0)

/* swiz */
#define SRC_SEL_X(x)    (x)     /* SQ_SEL_* each */
#define SRC_SEL_Y(x)    (x)
#define SRC_SEL_Z(x)    (x)
#define SRC_SEL_W(x)    (x)

#define CF_DWORD0(addr) (addr)
/* R7xx has another entry (COUNT3), but that is only used for adding a bit to count. */
/* We allow one more bit for count in the argument of the macro on R7xx instead. */
/* R6xx: [0,7]  R7xx: [1,16] */
#define CF_DWORD1(pc, cf_const, cond, count, call_count, eop, vpm, cf_inst, wqm, b) \
        (((pc) << 0) | ((cf_const) << 3) | ((cond) << 8) | (((count) & 7) << 10) | (((count) >> 3) << 19) | \
         ((call_count) << 13) | ((eop) << 21) | ((vpm) << 22) | ((cf_inst) << 23) | ((wqm) << 30) | ((b) << 31))

#define CF_ALU_DWORD0(addr, kb0, kb1, km0) (((addr) << 0) | ((kb0) << 22) | ((kb1) << 26) | ((km0) << 30))
#define CF_ALU_DWORD1(km1, kcache_addr0, kcache_addr1, count, uw, cf_inst, wqm, b) \
        (((km1) << 0) | ((kcache_addr0) << 2) | ((kcache_addr1) << 10) | \
     ((count) << 18) | ((uw) << 25) | ((cf_inst) << 26) | ((wqm) << 30) | ((b) << 31))

#define CF_ALLOC_IMP_EXP_DWORD0(array_base, type, rw_gpr, rr, index_gpr, es) \
     (((array_base) << 0) | ((type) << 13) | ((rw_gpr) << 15) | ((rr) << 22) | ((index_gpr) << 23) | \
          ((es) << 30))
/* R7xx apparently doesn't have the ELEM_LOOP entry any more */
/* We still expose it, but ELEM_LOOP is explicitely R6xx now. */
/* TODO: is this just forgotten in the docs, or really not available any more? */
#define CF_ALLOC_IMP_EXP_DWORD1_BUF(array_size, comp_mask, el, bc, eop, vpm, cf_inst, wqm, b) \
        (((array_size) << 0) | ((comp_mask) << 12) | ((el) << 16) | ((bc) << 17) | \
     ((eop) << 21) | ((vpm) << 22) | ((cf_inst) << 23) | ((wqm) << 30) | ((b) << 31))
#define CF_ALLOC_IMP_EXP_DWORD1_SWIZ(sel_x, sel_y, sel_z, sel_w, el, bc, eop, vpm, cf_inst, wqm, b) \
        (((sel_x) << 0) | ((sel_y) << 3) | ((sel_z) << 6) | ((sel_w) << 9) | ((el) << 16) | \
     ((bc) << 17) | ((eop) << 21) | ((vpm) << 22) | ((cf_inst) << 23) | \
     ((wqm) << 30) | ((b) << 31))

/* VTX clause insts */
/* vxt insts */
#define VTX_INST(x)        (x)      /* SQ_VTX_INST_* */

/* fetch type */
#define FETCH_TYPE(x)        (x)    /* SQ_VTX_FETCH_* */

#define FETCH_WHOLE_QUAD(x)        (x)
#define BUFFER_ID(x)        (x)
#define SRC_GPR(x)          (x)
#define SRC_REL(x)          (x)
#define MEGA_FETCH_COUNT(x)        ((x) ? ((x) - 1) : 0)

#define SEMANTIC_ID(x)        (x)
#define DST_SEL_X(x)          (x)
#define DST_SEL_Y(x)          (x)
#define DST_SEL_Z(x)          (x)
#define DST_SEL_W(x)          (x)
#define USE_CONST_FIELDS(x)   (x)
#define DATA_FORMAT(x)        (x)
/* num format */
#define NUM_FORMAT_ALL(x)     (x)   /* SQ_NUM_FORMAT_* */
/* format comp */
#define FORMAT_COMP_ALL(x)     (x)  /* SQ_FORMAT_COMP_* */
/* sma */
#define SRF_MODE_ALL(x)     (x)
#define SRF_MODE_ZERO_CLAMP_MINUS_ONE      0
#define SRF_MODE_NO_ZERO                   1
#define OFFSET(x)     (x)
/* endian swap */
#define ENDIAN_SWAP(x)     (x)      /* SQ_ENDIAN_* */
#define CONST_BUF_NO_STRIDE(x)     (x)
/* mf */
#define MEGA_FETCH(x)     (x)

#define VTX_DWORD0(vtx_inst, ft, fwq, buffer_id, src_gpr, sr, ssx, mfc) \
        (((vtx_inst) << 0) | ((ft) << 5) | ((fwq) << 7) | ((buffer_id) << 8) | \
     ((src_gpr) << 16) | ((sr) << 23) | ((ssx) << 24) | ((mfc) << 26))
#define VTX_DWORD1_SEM(semantic_id, dsx, dsy, dsz, dsw, ucf, data_format, nfa, fca, sma) \
        (((semantic_id) << 0) | ((dsx) << 9) | ((dsy) << 12) | ((dsz) << 15) | ((dsw) << 18) | \
     ((ucf) << 21) | ((data_format) << 22) | ((nfa) << 28) | ((fca) << 30) | ((sma) << 31))
#define VTX_DWORD1_GPR(dst_gpr, dr, dsx, dsy, dsz, dsw, ucf, data_format, nfa, fca, sma) \
        (((dst_gpr) << 0) | ((dr) << 7) | ((dsx) << 9) | ((dsy) << 12) | ((dsz) << 15) | ((dsw) << 18) | \
     ((ucf) << 21) | ((data_format) << 22) | ((nfa) << 28) | ((fca) << 30) | ((sma) << 31))
#define VTX_DWORD2(offset, es, cbns, mf) \
     (((offset) << 0) | ((es) << 16) | ((cbns) << 18) | ((mf) << 19))
#define VTX_DWORD_PAD 0x00000000


int R600_solid_vs(struct radeon_device *rdev, uint32_t* shader)
{
    int i=0;

    /* 0 */
    shader[i++] = CF_DWORD0(ADDR(4));
    shader[i++] = CF_DWORD1(POP_COUNT(0),
                CF_CONST(0),
                COND(SQ_CF_COND_ACTIVE),
                I_COUNT(1),
                CALL_COUNT(0),
                END_OF_PROGRAM(0),
                VALID_PIXEL_MODE(0),
                CF_INST(SQ_CF_INST_VTX),
                WHOLE_QUAD_MODE(0),
                BARRIER(1));
    /* 1 */
    shader[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
                      TYPE(SQ_EXPORT_POS),
                      RW_GPR(1),
                      RW_REL(ABSOLUTE),
                      INDEX_GPR(0),
                      ELEM_SIZE(0));
    shader[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
                           SRC_SEL_Y(SQ_SEL_Y),
                           SRC_SEL_Z(SQ_SEL_Z),
                           SRC_SEL_W(SQ_SEL_W),
                           R6xx_ELEM_LOOP(0),
                           BURST_COUNT(1),
                           END_OF_PROGRAM(0),
                           VALID_PIXEL_MODE(0),
                           CF_INST(SQ_CF_INST_EXPORT_DONE),
                           WHOLE_QUAD_MODE(0),
                           BARRIER(1));
    /* 2 - always export a param whether it's used or not */
    shader[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
                      TYPE(SQ_EXPORT_PARAM),
                      RW_GPR(0),
                      RW_REL(ABSOLUTE),
                      INDEX_GPR(0),
                      ELEM_SIZE(0));
    shader[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
                           SRC_SEL_Y(SQ_SEL_Y),
                           SRC_SEL_Z(SQ_SEL_Z),
                           SRC_SEL_W(SQ_SEL_W),
                           R6xx_ELEM_LOOP(0),
                           BURST_COUNT(0),
                           END_OF_PROGRAM(1),
                           VALID_PIXEL_MODE(0),
                           CF_INST(SQ_CF_INST_EXPORT_DONE),
                           WHOLE_QUAD_MODE(0),
                           BARRIER(0));
    /* 3 - padding */
    shader[i++] = 0x00000000;
    shader[i++] = 0x00000000;
    /* 4/5 */
    shader[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
                 FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
                 FETCH_WHOLE_QUAD(0),
                 BUFFER_ID(0),
                 SRC_GPR(0),
                 SRC_REL(ABSOLUTE),
                 SRC_SEL_X(SQ_SEL_X),
                 MEGA_FETCH_COUNT(8));
    shader[i++] = VTX_DWORD1_GPR(DST_GPR(1),
                 DST_REL(0),
                 DST_SEL_X(SQ_SEL_X),
                 DST_SEL_Y(SQ_SEL_Y),
                 DST_SEL_Z(SQ_SEL_0),
                 DST_SEL_W(SQ_SEL_1),
                 USE_CONST_FIELDS(0),
                 DATA_FORMAT(FMT_32_32_FLOAT), /* xxx */
                 NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM), /* xxx */
                 FORMAT_COMP_ALL(SQ_FORMAT_COMP_SIGNED), /* xxx */
                 SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE));
    shader[i++] = VTX_DWORD2(OFFSET(0),
                 ENDIAN_SWAP(ENDIAN_NONE),
                 CONST_BUF_NO_STRIDE(0),
                 MEGA_FETCH(1));
    shader[i++] = VTX_DWORD_PAD;

    return i;
}

int R600_solid_ps(struct radeon_device *rdev, uint32_t* shader)
{
    int i=0;

    /* 0 */
    shader[i++] = CF_ALU_DWORD0(ADDR(2),
                KCACHE_BANK0(0),
                KCACHE_BANK1(0),
                KCACHE_MODE0(SQ_CF_KCACHE_NOP));
    shader[i++] = CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
                KCACHE_ADDR0(0),
                KCACHE_ADDR1(0),
                I_COUNT(4),
                USES_WATERFALL(0),
                CF_INST(SQ_CF_INST_ALU),
                WHOLE_QUAD_MODE(0),
                BARRIER(1));
    /* 1 */
    shader[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
                      TYPE(SQ_EXPORT_PIXEL),
                      RW_GPR(0),
                      RW_REL(ABSOLUTE),
                      INDEX_GPR(0),
                      ELEM_SIZE(1));
    shader[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
                           SRC_SEL_Y(SQ_SEL_Y),
                           SRC_SEL_Z(SQ_SEL_Z),
                           SRC_SEL_W(SQ_SEL_W),
                           R6xx_ELEM_LOOP(0),
                           BURST_COUNT(1),
                           END_OF_PROGRAM(1),
                           VALID_PIXEL_MODE(0),
                           CF_INST(SQ_CF_INST_EXPORT_DONE),
                           WHOLE_QUAD_MODE(0),
                           BARRIER(1));

    /* 2 */
    shader[i++] = ALU_DWORD0(SRC0_SEL(256),
                 SRC0_REL(ABSOLUTE),
                 SRC0_ELEM(ELEM_X),
                 SRC0_NEG(0),
                 SRC1_SEL(0),
                 SRC1_REL(ABSOLUTE),
                 SRC1_ELEM(ELEM_X),
                 SRC1_NEG(0),
                 INDEX_MODE(SQ_INDEX_AR_X),
                 PRED_SEL(SQ_PRED_SEL_OFF),
                 LAST(0));
    shader[i++] = ALU_DWORD1_OP2(rdev->family,
                 SRC0_ABS(0),
                 SRC1_ABS(0),
                 UPDATE_EXECUTE_MASK(0),
                 UPDATE_PRED(0),
                 WRITE_MASK(1),
                 FOG_MERGE(0),
                 OMOD(SQ_ALU_OMOD_OFF),
                 ALU_INST(SQ_OP2_INST_MOV),
                 BANK_SWIZZLE(SQ_ALU_VEC_012),
                 DST_GPR(0),
                 DST_REL(ABSOLUTE),
                 DST_ELEM(ELEM_X),
                 CLAMP(1));
    /* 3 */
    shader[i++] = ALU_DWORD0(SRC0_SEL(256),
                 SRC0_REL(ABSOLUTE),
                 SRC0_ELEM(ELEM_Y),
                 SRC0_NEG(0),
                 SRC1_SEL(0),
                 SRC1_REL(ABSOLUTE),
                 SRC1_ELEM(ELEM_Y),
                 SRC1_NEG(0),
                 INDEX_MODE(SQ_INDEX_AR_X),
                 PRED_SEL(SQ_PRED_SEL_OFF),
                 LAST(0));
    shader[i++] = ALU_DWORD1_OP2(rdev->family,
                 SRC0_ABS(0),
                 SRC1_ABS(0),
                 UPDATE_EXECUTE_MASK(0),
                 UPDATE_PRED(0),
                 WRITE_MASK(1),
                 FOG_MERGE(0),
                 OMOD(SQ_ALU_OMOD_OFF),
                 ALU_INST(SQ_OP2_INST_MOV),
                 BANK_SWIZZLE(SQ_ALU_VEC_012),
                 DST_GPR(0),
                 DST_REL(ABSOLUTE),
                 DST_ELEM(ELEM_Y),
                 CLAMP(1));
    /* 4 */
    shader[i++] = ALU_DWORD0(SRC0_SEL(256),
                 SRC0_REL(ABSOLUTE),
                 SRC0_ELEM(ELEM_Z),
                 SRC0_NEG(0),
                 SRC1_SEL(0),
                 SRC1_REL(ABSOLUTE),
                 SRC1_ELEM(ELEM_Z),
                 SRC1_NEG(0),
                 INDEX_MODE(SQ_INDEX_AR_X),
                 PRED_SEL(SQ_PRED_SEL_OFF),
                 LAST(0));
    shader[i++] = ALU_DWORD1_OP2(rdev->family,
                 SRC0_ABS(0),
                 SRC1_ABS(0),
                 UPDATE_EXECUTE_MASK(0),
                 UPDATE_PRED(0),
                 WRITE_MASK(1),
                 FOG_MERGE(0),
                 OMOD(SQ_ALU_OMOD_OFF),
                 ALU_INST(SQ_OP2_INST_MOV),
                 BANK_SWIZZLE(SQ_ALU_VEC_012),
                 DST_GPR(0),
                 DST_REL(ABSOLUTE),
                 DST_ELEM(ELEM_Z),
                 CLAMP(1));
    /* 5 */
    shader[i++] = ALU_DWORD0(SRC0_SEL(256),
                 SRC0_REL(ABSOLUTE),
                 SRC0_ELEM(ELEM_W),
                 SRC0_NEG(0),
                 SRC1_SEL(0),
                 SRC1_REL(ABSOLUTE),
                 SRC1_ELEM(ELEM_W),
                 SRC1_NEG(0),
                 INDEX_MODE(SQ_INDEX_AR_X),
                 PRED_SEL(SQ_PRED_SEL_OFF),
                 LAST(1));
    shader[i++] = ALU_DWORD1_OP2(rdev->family,
                 SRC0_ABS(0),
                 SRC1_ABS(0),
                 UPDATE_EXECUTE_MASK(0),
                 UPDATE_PRED(0),
                 WRITE_MASK(1),
                 FOG_MERGE(0),
                 OMOD(SQ_ALU_OMOD_OFF),
                 ALU_INST(SQ_OP2_INST_MOV),
                 BANK_SWIZZLE(SQ_ALU_VEC_012),
                 DST_GPR(0),
                 DST_REL(ABSOLUTE),
                 DST_ELEM(ELEM_W),
                 CLAMP(1));

    return i;
}

static inline void
memcpy_toio(volatile void __iomem *dst, const void *src, int count)
{
    __memcpy((void __force *)dst, src, count);
}

#define EFLOAT(val)                         \
do {                                        \
    union { float f; uint32_t d; } a;       \
    a.f = (val);                            \
    radeon_ring_write(rdev, a.d);           \
} while (0)

int r600_2D_test(struct radeon_device *rdev)
{
    uint32_t   ps_shader[16];
    uint32_t   vs_shader[16];

    u32        packet2s[16];
    int        num_packet2s = 0;

    uint32_t   pitch;
    uint32_t   offset;

    int        state_len;
    int        dwords;
    u32        obj_size;

    u32        state_offset   = 0;
    u64        state_gpu_addr = 0;

    u32        vs_offset;
    u32        ps_offset;
    u32        vb_offset;

    int        vs_size;
    int        ps_size;

    float     *vb;
    void      *ptr;

    struct radeon_bo *state_obj;

    int        r;

    ENTER();

    pitch  = (1024*4)/64;
    offset = rdev->mc.vram_start;
    ps_size = R600_solid_ps(rdev, ps_shader);
    vs_size = R600_solid_vs(rdev, vs_shader);

    if (rdev->family >= CHIP_RV770)
        state_len = r7xx_default_size;
    else
        state_len = r6xx_default_size;

    dwords = state_len;

    while (dwords & 0xf) {
        packet2s[num_packet2s++] = PACKET2(0);
        dwords++;
    }

    obj_size = dwords * 4;
    obj_size = ALIGN(obj_size, 256);

    vs_offset = obj_size;
    obj_size += vs_size * 4;
    obj_size = ALIGN(obj_size, 256);

    ps_offset = obj_size;
    obj_size += ps_size * 4;
    obj_size = ALIGN(obj_size, 256);

    vb_offset = obj_size;
    obj_size += 32*4;
    obj_size = ALIGN(obj_size, 256);

    r = radeon_bo_create(rdev, NULL, obj_size, PAGE_SIZE, true,
                         RADEON_GEM_DOMAIN_VRAM, &state_obj);
    if (r) {
        DRM_ERROR("r600 failed to allocate state buffer\n");
        return r;
    }

    DRM_DEBUG("r6xx state allocated bo %08x vs %08x ps %08x\n",
          obj_size, vs_offset, ps_offset);

    r = radeon_bo_pin(state_obj, RADEON_GEM_DOMAIN_VRAM,
                      &state_gpu_addr);
    if (r) {
        DRM_ERROR("failed to pin state object %d\n", r);
        return r;
    };

    r = radeon_bo_kmap(state_obj, &ptr);
    if (r) {
        DRM_ERROR("failed to map state object %d\n", r);
        return r;
    };

    if (rdev->family >= CHIP_RV770)
        memcpy_toio(ptr + state_offset,
                r7xx_default_state, state_len * 4);
    else
        memcpy_toio(ptr + state_offset,
                r6xx_default_state, state_len * 4);

    if (num_packet2s)
        memcpy_toio(ptr + state_offset + (state_len * 4),
                    packet2s, num_packet2s * 4);

    memcpy(ptr + vs_offset, vs_shader, vs_size * 4);
    memcpy(ptr + ps_offset, ps_shader, ps_size * 4);


    vb = (float*)(ptr + vb_offset);

    vb[0] = (float)64;
    vb[1] = (float)64;

    vb[2] = (float)64;
    vb[3] = (float)(64+128);

    vb[4] = (float)(64+128);
    vb[5] = (float)(64+128);

    int vb_index = 3;
    int vb_size = vb_index * 8;
    int vtx_num_entries = vb_size / 4;

//    radeon_bo_kunmap(state_obj);

    r = radeon_ring_lock(rdev, 1024);
    if (r) {
        DRM_ERROR("radeon: cp failed to lock ring (%d).\n", r);
        return r;
    }

    set_default_state(rdev, state_gpu_addr, state_len);


    u64 gpu_addr;
    u32 sq_pgm_resources;

    /* setup shader regs */

    /* VS */

    sq_pgm_resources = (2 << 0);
    gpu_addr = state_gpu_addr + vs_offset;

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (SQ_PGM_START_VS - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, gpu_addr >> 8);

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (SQ_PGM_RESOURCES_VS - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, sq_pgm_resources);

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (SQ_PGM_CF_OFFSET_VS - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, 0);

    /* PS */

    sq_pgm_resources = (1 << 0);
    gpu_addr = state_gpu_addr + ps_offset;

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (SQ_PGM_START_PS - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, gpu_addr >> 8);

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (SQ_PGM_RESOURCES_PS - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, sq_pgm_resources | (1 << 28));

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (SQ_PGM_EXPORTS_PS - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, 2);

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONTEXT_REG, 1));
    radeon_ring_write(rdev, (SQ_PGM_CF_OFFSET_PS - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, 0);

    gpu_addr = state_gpu_addr + vs_offset;
    cp_set_surface_sync(rdev, PACKET3_SH_ACTION_ENA, 512, gpu_addr);


    set_render_target(rdev, COLOR_8_8_8_8, 1024, 768,  /* FIXME */
                      rdev->mc.vram_start);

    set_scissors(rdev, 0, 0, 1024, 768);


    radeon_ring_write(rdev, PACKET3(PACKET3_SET_ALU_CONST, 4));
    radeon_ring_write(rdev, (SQ_ALU_CONSTANT0_0 - PACKET3_SET_ALU_CONST_OFFSET) >> 2);
    EFLOAT(0.0f);                   /* r */
    EFLOAT(1.0f);                   /* g */
    EFLOAT(0.0f);                   /* b */
    EFLOAT(1.0f);                   /* a */

    u32 sq_vtx_constant_word2;

    gpu_addr = state_gpu_addr + vb_offset;

    sq_vtx_constant_word2 = ((upper_32_bits(gpu_addr) & 0xff) | (8 << 8));

    radeon_ring_write(rdev, PACKET3(PACKET3_SET_RESOURCE, 7));
    radeon_ring_write(rdev, 0x460);
    radeon_ring_write(rdev, gpu_addr & 0xffffffff);        /* 0: BASE_ADDRESS */
    radeon_ring_write(rdev, (vtx_num_entries << 2) - 1);   /* 1: SIZE */
    radeon_ring_write(rdev, sq_vtx_constant_word2);        /* 2: BASE_HI, STRIDE, CLAMP, FORMAT, ENDIAN */
    radeon_ring_write(rdev, 1 << 0);                       /* 3: MEM_REQUEST_SIZE ?!? */
    radeon_ring_write(rdev, 0);
    radeon_ring_write(rdev, 0);
    radeon_ring_write(rdev, SQ_TEX_VTX_VALID_BUFFER << 30);

    if ((rdev->family == CHIP_RV610) ||
        (rdev->family == CHIP_RV620) ||
        (rdev->family == CHIP_RS780) ||
        (rdev->family == CHIP_RS880) ||
        (rdev->family == CHIP_RV710))
        cp_set_surface_sync(rdev,
                    PACKET3_TC_ACTION_ENA, 24, gpu_addr);
    else
        cp_set_surface_sync(rdev,
                    PACKET3_VC_ACTION_ENA, 24, gpu_addr);

    draw_auto(rdev);

    cp_set_surface_sync(rdev, PACKET3_CB_ACTION_ENA | PACKET3_CB0_DEST_BASE_ENA,
                        1024*4*512, offset);

    radeon_ring_write(rdev, PACKET3(PACKET3_EVENT_WRITE, 0));
    radeon_ring_write(rdev, CACHE_FLUSH_AND_INV_EVENT);
    /* wait for 3D idle clean */
    radeon_ring_write(rdev, PACKET3(PACKET3_SET_CONFIG_REG, 1));
    radeon_ring_write(rdev, (WAIT_UNTIL - PACKET3_SET_CONFIG_REG_OFFSET) >> 2);
    radeon_ring_write(rdev, WAIT_3D_IDLE_bit | WAIT_3D_IDLECLEAN_bit);

    radeon_ring_unlock_commit(rdev);

    r600_ring_test(rdev);

    LEAVE();
    return r;
}

#endif
