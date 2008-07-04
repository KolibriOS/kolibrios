
//#define R300_TEST

#include "r5xx_regs.h"

#define RADEON_BUS_CNTL                 0x0030
#	define RADEON_BUS_MASTER_DIS		(1 << 6)


#define RADEON_SCRATCH_UMSK             0x0770
#define RADEON_SCRATCH_ADDR		0x0774

#define RADEON_CP_ME_RAM_ADDR           0x07d4
#define RADEON_CP_ME_RAM_RADDR		0x07d8
#define RADEON_CP_ME_RAM_DATAH		0x07dc
#define RADEON_CP_ME_RAM_DATAL		0x07e0

#define RADEON_AIC_CNTL                 0x01d0
#define RADEON_PCIGART_TRANSLATE_EN      (1 << 0)


#define RADEON_CP_RB_BASE               0x0700
#define RADEON_CP_RB_CNTL		0x0704
#	define RADEON_BUF_SWAP_32BIT		(2 << 16)
#	define RADEON_RB_NO_UPDATE		(1 << 27)
#define RADEON_CP_RB_RPTR_ADDR		0x070c
#define RADEON_CP_RB_RPTR		0x0710
#define RADEON_CP_RB_WPTR		0x0714

#define RADEON_CP_RB_WPTR_DELAY		0x0718
#	define RADEON_PRE_WRITE_TIMER_SHIFT	0
#	define RADEON_PRE_WRITE_LIMIT_SHIFT	23

#define RADEON_CP_IB_BASE		0x0738

#define RADEON_CP_CSQ_CNTL		0x0740
#	define RADEON_CSQ_CNT_PRIMARY_MASK	(0xff << 0)
#	define RADEON_CSQ_PRIDIS_INDDIS		(0 << 28)
#	define RADEON_CSQ_PRIPIO_INDDIS		(1 << 28)
#	define RADEON_CSQ_PRIBM_INDDIS		(2 << 28)
#	define RADEON_CSQ_PRIPIO_INDBM		(3 << 28)
#	define RADEON_CSQ_PRIBM_INDBM		(4 << 28)
#	define RADEON_CSQ_PRIPIO_INDPIO		(15 << 28)

#define RADEON_ISYNC_CNTL               0x1724
#	define RADEON_ISYNC_ANY2D_IDLE3D	(1 << 0)
#	define RADEON_ISYNC_ANY3D_IDLE2D	(1 << 1)
#	define RADEON_ISYNC_TRIG2D_IDLE3D	(1 << 2)
#	define RADEON_ISYNC_TRIG3D_IDLE2D	(1 << 3)
#	define RADEON_ISYNC_WAIT_IDLEGUI	(1 << 4)
#	define RADEON_ISYNC_CPSCRATCH_IDLEGUI	(1 << 5)

#define R5XX_LOOP_COUNT 2000000

#include "microcode.h"

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


void RADEONPllErrataAfterData()
{

    /* This function is required to workaround a hardware bug in some (all?)
     * revisions of the R300.  This workaround should be called after every
     * CLOCK_CNTL_INDEX register access.  If not, register reads afterward
     * may not be correct.
     */
    if (rhd.ChipSet <= RHD_RV380)
    {
        u32_t save, tmp;

	save = INREG(RADEON_CLOCK_CNTL_INDEX);
	tmp = save & ~(0x3f | RADEON_PLL_WR_EN);
	OUTREG(RADEON_CLOCK_CNTL_INDEX, tmp);
	tmp = INREG(RADEON_CLOCK_CNTL_DATA);
	OUTREG(RADEON_CLOCK_CNTL_INDEX, save);
    }
}


/* Read PLL register */
u32_t RADEONINPLL(int addr)
{
    u32_t       data;

    OUTREG8(RADEON_CLOCK_CNTL_INDEX, addr & 0x3f);
    //RADEONPllErrataAfterIndex();
    data = INREG(RADEON_CLOCK_CNTL_DATA);
    RADEONPllErrataAfterData();

    return data;
};

/* Write PLL information */
void RADEONOUTPLL(int addr, u32_t data)
{
    OUTREG8(RADEON_CLOCK_CNTL_INDEX, (((addr) & 0x3f) |
				      RADEON_PLL_WR_EN));
//    RADEONPllErrataAfterIndex(info);
    OUTREG(RADEON_CLOCK_CNTL_DATA, data);
    RADEONPllErrataAfterData();
}



static Bool
R5xxFIFOWaitLocal(CARD32 required)             //R100-R500
{
  int i;

  for (i = 0; i < R5XX_LOOP_COUNT; i++)
    if (required <= (INREG(R5XX_RBBM_STATUS) & R5XX_RBBM_FIFOCNT_MASK))
	    return TRUE;

  dbgprintf("%s: Timeout 0x%08X.\n", __func__,
         (unsigned int) INREG(R5XX_RBBM_STATUS));
  return FALSE;
}

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

static void
R5xx2DReset()
{
    CARD32 save, tmp;
    u32_t       clock_cntl_index;
    u32_t       mclk_cntl;

      /* The following RBBM_SOFT_RESET sequence can help un-wedge
       * an R300 after the command processor got stuck. */
    save = INREG(R5XX_RBBM_SOFT_RESET);
    tmp = save | R5XX_SOFT_RESET_CP |
      R5XX_SOFT_RESET_HI | R5XX_SOFT_RESET_SE |
      R5XX_SOFT_RESET_RE | R5XX_SOFT_RESET_PP |
      R5XX_SOFT_RESET_E2 | R5XX_SOFT_RESET_RB;
    OUTREG(R5XX_RBBM_SOFT_RESET, tmp);

    INREG(R5XX_RBBM_SOFT_RESET);
    tmp &= ~(R5XX_SOFT_RESET_CP | R5XX_SOFT_RESET_HI |
         R5XX_SOFT_RESET_SE | R5XX_SOFT_RESET_RE |
         R5XX_SOFT_RESET_PP | R5XX_SOFT_RESET_E2 |
         R5XX_SOFT_RESET_RB);
    OUTREG(R5XX_RBBM_SOFT_RESET, tmp);

    INREG(R5XX_RBBM_SOFT_RESET);
    OUTREG(R5XX_RBBM_SOFT_RESET, save);
    INREG(R5XX_RBBM_SOFT_RESET);

    R5xx2DFlush();

#if 0
    clock_cntl_index = INREG(RADEON_CLOCK_CNTL_INDEX);
    RADEONPllErrataAfterIndex(info);

    mclk_cntl = RADEONINPLL(RADEON_MCLK_CNTL);

    RADEONOUTPLL(RADEON_MCLK_CNTL, (mclk_cntl |
			      RADEON_FORCEON_MCLKA |
			      RADEON_FORCEON_MCLKB |
			      RADEON_FORCEON_YCLKA |
			      RADEON_FORCEON_YCLKB |
			      RADEON_FORCEON_MC |
			      RADEON_FORCEON_AIC));
#endif

      /* Soft resetting HDP thru RBBM_SOFT_RESET register can cause some
       * unexpected behaviour on some machines.  Here we use
       * R5XX_HOST_PATH_CNTL to reset it. */
    save = INREG(R5XX_HOST_PATH_CNTL);

    tmp = INREG(R5XX_RBBM_SOFT_RESET);
    tmp |= R5XX_SOFT_RESET_CP | R5XX_SOFT_RESET_HI | R5XX_SOFT_RESET_E2;
    OUTREG(R5XX_RBBM_SOFT_RESET, tmp);

    INREG(R5XX_RBBM_SOFT_RESET);
    OUTREG(R5XX_RBBM_SOFT_RESET, 0);

    MASKREG(R5XX_RB2D_DSTCACHE_MODE,
	       R5XX_RB2D_DC_AUTOFLUSH_ENABLE | R5XX_RB2D_DC_DISABLE_IGNORE_PE,
	       R5XX_RB2D_DC_AUTOFLUSH_ENABLE | R5XX_RB2D_DC_DISABLE_IGNORE_PE);

    OUTREG(R5XX_HOST_PATH_CNTL, save | R5XX_HDP_SOFT_RESET);
    INREG(R5XX_HOST_PATH_CNTL);
    OUTREG(R5XX_HOST_PATH_CNTL, save);

#if 0
    OUTREG(RADEON_CLOCK_CNTL_INDEX, clock_cntl_index);
    RADEONPllErrataAfterIndex(info);
    RADEONOUTPLL(RADEON_MCLK_CNTL, mclk_cntl);
#endif
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

    R5xxFIFOWaitLocal(1);
    OUTREG(R5XX_DEFAULT_SC_BOTTOM_RIGHT,
           R5XX_DEFAULT_SC_RIGHT_MAX | R5XX_DEFAULT_SC_BOTTOM_MAX);
    R5xxFIFOWaitLocal(1);
    OUTREG(R5XX_DP_GUI_MASTER_CNTL, rhd.gui_control |
           R5XX_GMC_BRUSH_SOLID_COLOR | R5XX_GMC_SRC_DATATYPE_COLOR);

    R5xxFIFOWaitLocal(5);
    OUTREG(R5XX_DP_BRUSH_FRGD_CLR, 0xFFFFFFFF);
    OUTREG(R5XX_DP_BRUSH_BKGD_CLR, 0x00000000);
    OUTREG(R5XX_DP_SRC_FRGD_CLR, 0xFFFFFFFF);
    OUTREG(R5XX_DP_SRC_BKGD_CLR, 0x00000000);
    OUTREG(R5XX_DP_WRITE_MASK, 0xFFFFFFFF);

    R5xx2DIdleLocal();
}

void R5xxFIFOWait(CARD32 required)
{
    if (!R5xxFIFOWaitLocal(required)) {
      R5xx2DReset();
      R5xx2DSetup();
    }
}

void R5xx2DIdle()
{
    if (!R5xx2DIdleLocal()) {
      R5xx2DReset();
      R5xx2DSetup();
    }
}

static void load_microcode()
{
  u32 ifl;
  int i;

  ifl = safe_cli();

  OUTREG(RADEON_CP_ME_RAM_ADDR,0);

  R5xx2DIdleLocal();

  switch(rhd.ChipSet)
  {
    case RHD_R300:
    case RHD_R350:
    case RHD_RV350:
    case RHD_RV370:
    case RHD_RV380:
      dbgprintf("Loading R300 microcode\n");
      for (i = 0; i < 256; i++)
      {
        OUTREG(RADEON_CP_ME_RAM_DATAH, R300_cp_microcode[i][1]);
        OUTREG(RADEON_CP_ME_RAM_DATAL, R300_cp_microcode[i][0]);
      }
      break;

    case RHD_RV505:
    case RHD_RV515:
    case RHD_RV516:
    case RHD_R520:
    case RHD_RV530:
    case RHD_RV535:
    case RHD_RV550:
    case RHD_RV560:
    case RHD_RV570:
    case RHD_R580:
      dbgprintf("Loading R500 microcode\n");
      for (i = 0; i < 256; i++)
      {
        OUTREG(RADEON_CP_ME_RAM_DATAH, R520_cp_microcode[i][1]);
        OUTREG(RADEON_CP_ME_RAM_DATAL, R520_cp_microcode[i][0]);
      }
  }
  safe_sti(ifl);
};


void  R5xx2DInit()
{
    u32 base;

#ifdef R300_TEST
    rhd.displayWidth  = 800;
    rhd.displayHeight = 600;
#else
    rhd.displayWidth  = INREG(D1GRPH_X_END);
    rhd.displayHeight = INREG(D1GRPH_Y_END);
#endif

    rhd.__xmin = 0;
    rhd.__ymin = 0;
    rhd.__xmax = rhd.displayWidth  - 1;
    rhd.__ymax = rhd.displayHeight - 1;

    clip.xmin = 0;
    clip.ymin = 0;
    clip.xmax = rhd.displayWidth  - 1;
    clip.ymax = rhd.displayHeight - 1;

    dbgprintf("width  %d \n", rhd.displayWidth);
    dbgprintf("height %d \n", rhd.displayHeight);

    rhd.gui_control = (R5XX_DATATYPE_ARGB8888 << R5XX_GMC_DST_DATATYPE_SHIFT) |
                       R5XX_GMC_CLR_CMP_CNTL_DIS | R5XX_GMC_DST_PITCH_OFFSET_CNTL;

    dbgprintf("gui_control %x \n", rhd.gui_control);

    rhd.surface_cntl = 0;
    rhd.dst_pitch_offset = (((rhd.displayWidth * 4) / 64) << 22) |
                            ((rhd.FbIntAddress + rhd.FbScanoutStart) >> 10);

    dbgprintf("dst_pitch_offset %x \n", rhd.dst_pitch_offset);


    scr_pixmap.width  = rhd.displayWidth;
    scr_pixmap.height = rhd.displayHeight;
    scr_pixmap.format = PICT_a8r8g8b8;
    scr_pixmap.pitch  = rhd.displayWidth * 4;
    scr_pixmap.offset = rhd.FbIntAddress;
    scr_pixmap.pitch_offset =  rhd.dst_pitch_offset;
    scr_pixmap.raw    = (void*)0;


    MASKREG(R5XX_GB_TILE_CONFIG, 0, R5XX_ENABLE_TILING);
    OUTREG (R5XX_WAIT_UNTIL, R5XX_WAIT_2D_IDLECLEAN | R5XX_WAIT_3D_IDLECLEAN);
    MASKREG(R5XX_DST_PIPE_CONFIG, R5XX_PIPE_AUTO_CONFIG, R5XX_PIPE_AUTO_CONFIG);
    MASKREG(R5XX_RB2D_DSTCACHE_MODE,
            R5XX_RB2D_DC_AUTOFLUSH_ENABLE | R5XX_RB2D_DC_DISABLE_IGNORE_PE,
            R5XX_RB2D_DC_AUTOFLUSH_ENABLE | R5XX_RB2D_DC_DISABLE_IGNORE_PE);


    R5xx2DReset();
    R5xx2DSetup();

    MASKREG( RADEON_AIC_CNTL,0, RADEON_PCIGART_TRANSLATE_EN);

    load_microcode();

    rhd.ring_base = CreateRingBuffer(0x8000, PG_SW | PG_NOCACHE);
    dbgprintf("create cp ring buffer %x\n", rhd.ring_base);
    base = GetPgAddr(rhd.ring_base);

    OUTREG(RADEON_CP_RB_BASE, base);
    dbgprintf("ring base %x\n", base);

    OUTREG(RADEON_CP_RB_WPTR_DELAY, 0);

    rhd.ring_rp = rhd.ring_wp = INREG(RADEON_CP_RB_RPTR);
    OUTREG(RADEON_CP_RB_WPTR,rhd.ring_rp);

    OUTREG(RADEON_CP_RB_RPTR_ADDR, 0); // ring buffer read pointer no update

    OUTREG(RADEON_CP_RB_CNTL, RADEON_RB_NO_UPDATE | 12);
    OUTREG(RADEON_SCRATCH_UMSK, 0);          // no scratch update

    MASKREG(RADEON_BUS_CNTL,0,RADEON_BUS_MASTER_DIS);

    R5xx2DIdleLocal();

    OUTREG(RADEON_ISYNC_CNTL, RADEON_ISYNC_ANY2D_IDLE3D |
                              RADEON_ISYNC_ANY3D_IDLE2D |
                              RADEON_ISYNC_WAIT_IDLEGUI |
                              RADEON_ISYNC_CPSCRATCH_IDLEGUI);

    OUTREG(RADEON_CP_CSQ_CNTL, RADEON_CSQ_PRIBM_INDBM);    // run


 //   OUTREG(D1CUR_SIZE, (31<<16)|31);
 //   OUTREG(D1CUR_CONTROL, 0x300);
}



