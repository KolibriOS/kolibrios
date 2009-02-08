
#define R300_TEST

#include "r5xx_regs.h"


#define R5XX_LOOP_COUNT 2000000

#define RADEON_CLOCK_CNTL_DATA              0x000c

#define RADEON_CLOCK_CNTL_INDEX             0x0008
#       define RADEON_PLL_WR_EN             (1 << 7)
#       define RADEON_PLL_DIV_SEL           (3 << 8)
#       define RADEON_PLL2_DIV_SEL_MASK     ~(3 << 8)

#define RADEON_MCLK_CNTL                    0x0012 /* PLL */
#       define RADEON_FORCEON_MCLKA         (1 << 16)
#       define RADEON_FORCEON_MCLKB         (1 << 17)
#       define RADEON_FORCEON_YCLKA         (1 << 18)
#       define RADEON_FORCEON_YCLKB         (1 << 19)
#       define RADEON_FORCEON_MC            (1 << 20)
#       define RADEON_FORCEON_AIC           (1 << 21)
#       define R300_DISABLE_MC_MCLKA        (1 << 21)
#       define R300_DISABLE_MC_MCLKB        (1 << 21)




/*
 * Flush all dirty data in the Pixel Cache to memory.
 */

static Bool
R5xx2DFlush()
{
    int i;

    MASKREG(R5XX_DSTCACHE_CTLSTAT,
                R5XX_DSTCACHE_FLUSH_ALL, R5XX_DSTCACHE_FLUSH_ALL);

    for (i = 0; i < R5XX_LOOP_COUNT; i++)
        if (!(INREG(R5XX_DSTCACHE_CTLSTAT) & R5XX_DSTCACHE_BUSY))
            return TRUE;

    dbgprintf("%s: Timeout 0x%08x.\n", __func__,
         (unsigned int)INREG(R5XX_DSTCACHE_CTLSTAT));
    return FALSE;
}

static Bool
R5xx2DIdleLocal()                                //R100-R500
{
    int i;

      /* wait for fifo to clear */
    for (i = 0; i < R5XX_LOOP_COUNT; i++)
        if (64 == (INREG(R5XX_RBBM_STATUS) & R5XX_RBBM_FIFOCNT_MASK))
            break;

    if (i == R5XX_LOOP_COUNT) {
        dbgprintf("%s: FIFO Timeout 0x%08X.\n", __func__,INREG(R5XX_RBBM_STATUS));
        return FALSE;
    }

      /* wait for engine to go idle */
    for (i = 0; i < R5XX_LOOP_COUNT; i++) {
        if (!(INREG(R5XX_RBBM_STATUS) & R5XX_RBBM_ACTIVE)) {
            R5xx2DFlush();
            return TRUE;
        }
    }
    dbgprintf("%s: Idle Timeout 0x%08X.\n", __func__,INREG(R5XX_RBBM_STATUS));
    return FALSE;
}


void
R5xx2DSetup()
{

    /* Setup engine location. This shouldn't be necessary since we
     * set them appropriately before any accel ops, but let's avoid
     * random bogus DMA in case we inadvertently trigger the engine
     * in the wrong place (happened). */
    R5xxFIFOWaitLocal(2);
    OUTREG(R5XX_DST_PITCH_OFFSET,rhd.dst_pitch_offset);
    OUTREG(R5XX_SRC_PITCH_OFFSET,rhd.dst_pitch_offset);

    R5xxFIFOWaitLocal(1);
    MASKREG(R5XX_DP_DATATYPE, 0, R5XX_HOST_BIG_ENDIAN_EN);

    OUTREG(R5XX_SURFACE_CNTL, rhd.surface_cntl);

    R5xxFIFOWaitLocal(3);
    OUTREG(R5XX_SC_TOP_LEFT, 0);
    OUTREG(R5XX_SC_BOTTOM_RIGHT,
           RADEON_DEFAULT_SC_RIGHT_MAX | RADEON_DEFAULT_SC_BOTTOM_MAX);
    OUTREG(R5XX_DEFAULT_SC_BOTTOM_RIGHT,
           RADEON_DEFAULT_SC_RIGHT_MAX | RADEON_DEFAULT_SC_BOTTOM_MAX);

    R5xxFIFOWaitLocal(1);
//    OUTREG(R5XX_DP_GUI_MASTER_CNTL, rhd.gui_control |
//           R5XX_GMC_BRUSH_SOLID_COLOR | R5XX_GMC_SRC_DATATYPE_COLOR);
    OUTREG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT | R5XX_DST_Y_TOP_TO_BOTTOM);

    R5xxFIFOWaitLocal(5);
    OUTREG(R5XX_DP_BRUSH_FRGD_CLR, 0xFFFFFFFF);
    OUTREG(R5XX_DP_BRUSH_BKGD_CLR, 0x00000000);
    OUTREG(R5XX_DP_SRC_FRGD_CLR, 0xFFFFFFFF);
    OUTREG(R5XX_DP_SRC_BKGD_CLR, 0x00000000);
    OUTREG(R5XX_DP_WRITE_MASK, 0xFFFFFFFF);

    R5xx2DIdleLocal();
}

void R5xxFIFOWait(u32_t required)
{
    if (!R5xxFIFOWaitLocal(required)) {
 //     R5xx2DReset();
      R5xx2DSetup();
    }
}

void R5xx2DIdle()
{
    if (!R5xx2DIdleLocal()) {
  //    R5xx2DReset();
      R5xx2DSetup();
    }
}



void  R5xx2DInit()
{
    u32_t base;
    int screensize;
    int screenpitch;

    screensize = GetScreenSize();
    screenpitch = GetScreenPitch();

    rhd.displayWidth  = screensize >> 16;
    rhd.displayHeight = screensize & 0xFFFF;

    rhd.__xmin = 0;
    rhd.__ymin = 0;
    rhd.__xmax = rhd.displayWidth  - 1;
    rhd.__ymax = rhd.displayHeight - 1;

    clip.xmin = 0;
    clip.ymin = 0;
    clip.xmax = rhd.displayWidth  - 1;
    clip.ymax = rhd.displayHeight - 1;

    dbgprintf("screen width  %d height %d\n",
               rhd.displayWidth, rhd.displayHeight);

    rhd.gui_control = ((6 << RADEON_GMC_DST_DATATYPE_SHIFT)
                      | RADEON_GMC_CLR_CMP_CNTL_DIS
                      | RADEON_GMC_DST_PITCH_OFFSET_CNTL);

    dbgprintf("gui_control %x \n", rhd.gui_control);

    rhd.surface_cntl = 0;

    rhd.dst_pitch_offset = (((rhd.displayWidth * 4 / 64)<< 22) |
                               (rhd.fbLocation  >> 10));


    dbgprintf("dst_pitch_offset %x \n", rhd.dst_pitch_offset);

    scr_pixmap.width  = rhd.displayWidth;
    scr_pixmap.height = rhd.displayHeight;
    scr_pixmap.format = PICT_a8r8g8b8;
    scr_pixmap.flags  = PX_MEM_LOCAL;
    scr_pixmap.pitch  = rhd.displayWidth * 4     ;//screenpitch;
    scr_pixmap.local  = rhd.fbLocation;
    scr_pixmap.pitch_offset =  rhd.dst_pitch_offset;
    scr_pixmap.mapped = 0;

    R5xxFIFOWaitLocal(2);
    OUTREG(R5XX_DST_PITCH_OFFSET,rhd.dst_pitch_offset);
    OUTREG(R5XX_SRC_PITCH_OFFSET,rhd.dst_pitch_offset);

    R5xxFIFOWaitLocal(1);
    MASKREG(R5XX_DP_DATATYPE, 0, R5XX_HOST_BIG_ENDIAN_EN);

    OUTREG(R5XX_SURFACE_CNTL, rhd.surface_cntl);

#if !R300_PIO

    init_cp(&rhd);

#endif

    R5xx2DSetup();

}



