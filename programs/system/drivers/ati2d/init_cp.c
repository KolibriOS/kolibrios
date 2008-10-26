#define RADEON_SCRATCH_REG0     0x15e0
#define RADEON_SCRATCH_REG1		0x15e4
#define RADEON_SCRATCH_REG2		0x15e8
#define RADEON_SCRATCH_REG3		0x15ec
#define RADEON_SCRATCH_REG4		0x15f0
#define RADEON_SCRATCH_REG5		0x15f4
#define RADEON_SCRATCH_UMSK		0x0770
#define RADEON_SCRATCH_ADDR		0x0774

#   define RS400_BUS_MASTER_DIS      (1 << 14)
//#   define RADEON_BUS_MASTER_DIS     (1 <<  6)

#define RADEON_ISYNC_CNTL       0x1724
#	define RADEON_ISYNC_ANY2D_IDLE3D	(1 << 0)
#	define RADEON_ISYNC_ANY3D_IDLE2D	(1 << 1)
#	define RADEON_ISYNC_TRIG2D_IDLE3D	(1 << 2)
#	define RADEON_ISYNC_TRIG3D_IDLE2D	(1 << 3)
#	define RADEON_ISYNC_WAIT_IDLEGUI	(1 << 4)
#	define RADEON_ISYNC_CPSCRATCH_IDLEGUI	(1 << 5)


#define RADEON_IDLE_RETRY      16 /* Fall out of idle loops after this count */
#define RADEON_TIMEOUT    2000000 /* Fall out of wait loops after this count */


void RADEONPllErrataAfterIndex()
{
    if (!(rhd.ChipErrata & CHIP_ERRATA_PLL_DUMMYREADS))
       return;

    /* This workaround is necessary on rv200 and RS200 or PLL
     * reads may return garbage (among others...)
     */
    (void)INREG(RADEON_CLOCK_CNTL_DATA);
    (void)INREG(RADEON_CRTC_GEN_CNTL);
}


void RADEONPllErrataAfterData()
{

    /* This function is required to workaround a hardware bug in some (all?)
     * revisions of the R300.  This workaround should be called after every
     * CLOCK_CNTL_INDEX register access.  If not, register reads afterward
     * may not be correct.
     */
    if (rhd.ChipFamily <= CHIP_FAMILY_RV380)
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
    RADEONPllErrataAfterIndex();
    data = INREG(RADEON_CLOCK_CNTL_DATA);
    RADEONPllErrataAfterData();

    return data;
};

/* Write PLL information */
void RADEONOUTPLL(int addr, u32_t data)
{
    OUTREG8(RADEON_CLOCK_CNTL_INDEX, (((addr) & 0x3f) |
				      RADEON_PLL_WR_EN));
    RADEONPllErrataAfterIndex();
    OUTREG(RADEON_CLOCK_CNTL_DATA, data);
    RADEONPllErrataAfterData();
}

void RADEONEngineFlush(RHDPtr info)
{
     int i;

     if (info->ChipFamily <= CHIP_FAMILY_RV280)
     {
        MASKREG(RADEON_RB3D_DSTCACHE_CTLSTAT,RADEON_RB3D_DC_FLUSH_ALL,
                                             ~RADEON_RB3D_DC_FLUSH_ALL);
        for (i = 0; i < RADEON_TIMEOUT; i++) {
           if (!(INREG(RADEON_RB3D_DSTCACHE_CTLSTAT) & RADEON_RB3D_DC_BUSY))
            break;
        }
        if (i == RADEON_TIMEOUT) {
           dbgprintf("DC flush timeout: %x\n",
                     (u32_t)INREG(RADEON_RB3D_DSTCACHE_CTLSTAT));
        }
     }
     else
     {
//        MASKREG(R300_DSTCACHE_CTLSTAT,R300_RB2D_DC_FLUSH_ALL,
//                                      ~R300_RB2D_DC_FLUSH_ALL);
//        for (i = 0; i < RADEON_TIMEOUT; i++) {
//            if (!(INREG(R300_DSTCACHE_CTLSTAT) & R300_RB2D_DC_BUSY))
//            break;
//        }
//        if (i == RADEON_TIMEOUT) {
//           dbgprintf("DC flush timeout: %x\n",
//                     (u32_t)INREG(R300_DSTCACHE_CTLSTAT));
//        }
     }
}

static Bool R5xxFIFOWaitLocal(u32_t required)             //R100-R500
{
     int i;

     for (i = 0; i < RADEON_TIMEOUT; i++)
        if (required <= (INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_FIFOCNT_MASK))
           return TRUE;

     dbgprintf("%s: Timeout 0x%08X.\n", __func__, (u32_t) INREG(RADEON_RBBM_STATUS));
     return FALSE;
}

static int radeon_do_wait_for_idle()
{
	int i, ret;

    ret = R5xxFIFOWaitLocal(64);
	if (ret)
		return ret;

    for (i = 0; i < RADEON_TIMEOUT; i++)
    {
        if (!(INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_ACTIVE)) {
            RADEONEngineFlush(&rhd);
			return 0;
		}
        usleep(1);
	}
    dbgprintf("wait idle failed status : 0x%08X 0x%08X\n",
               INREG(RADEON_RBBM_STATUS),
               INREG(R300_VAP_CNTL_STATUS));

    return 1;
}


static void init_pipes(RHDPtr info)
{
     u32_t gb_tile_config = 0;

     if ( (info->ChipFamily == CHIP_FAMILY_RV410) ||
          (info->ChipFamily == CHIP_FAMILY_R420)  ||
          (info->ChipFamily == CHIP_FAMILY_RS600) ||
          (info->ChipFamily == CHIP_FAMILY_RS690) ||
          (info->ChipFamily == CHIP_FAMILY_RS740) ||
          (info->ChipFamily == CHIP_FAMILY_RS400) ||
          (info->ChipFamily == CHIP_FAMILY_RS480) || IS_R500_3D)
     {
         u32_t gb_pipe_sel = INREG(R400_GB_PIPE_SELECT);

         info->num_gb_pipes = ((gb_pipe_sel >> 12) & 0x3) + 1;
         if (IS_R500_3D)
            OUTPLL(R500_DYN_SCLK_PWMEM_PIPE, (1 | ((gb_pipe_sel >> 8) & 0xf) << 4));
     }
     else
     {
        if ((info->ChipFamily == CHIP_FAMILY_R300) ||
            (info->ChipFamily == CHIP_FAMILY_R350))
        {
         /* R3xx chips */
            info->num_gb_pipes = 2;
        }
        else {
         /* RV3xx chips */
           info->num_gb_pipes = 1;
        }
     }

     if (IS_R300_3D || IS_R500_3D)
     {

        dbgprintf("num quad-pipes is %d\n", info->num_gb_pipes);

        switch(info->num_gb_pipes) {
           case 2: gb_tile_config |= R300_PIPE_COUNT_R300; break;
           case 3: gb_tile_config |= R300_PIPE_COUNT_R420_3P; break;
           case 4: gb_tile_config |= R300_PIPE_COUNT_R420; break;
           default:
           case 1: gb_tile_config |= R300_PIPE_COUNT_RV350; break;
        }

        OUTREG(R300_GB_TILE_CONFIG, gb_tile_config);
        OUTREG(RADEON_WAIT_UNTIL, RADEON_WAIT_2D_IDLECLEAN | RADEON_WAIT_3D_IDLECLEAN);
        OUTREG(R300_DST_PIPE_CONFIG, INREG(R300_DST_PIPE_CONFIG) | R300_PIPE_AUTO_CONFIG);
        OUTREG(R300_RB2D_DSTCACHE_MODE, (INREG(R300_RB2D_DSTCACHE_MODE) |
                                        R300_DC_AUTOFLUSH_ENABLE |
                                        R300_DC_DC_DISABLE_IGNORE_PE));
    }
    else
       OUTREG(RADEON_RB3D_CNTL, 0);
};

/* ================================================================
 * CP control, initialization
 */

/* Load the microcode for the CP */

#include "radeon_microcode.h"

static void load_microcode(RHDPtr info)
{
	int i;
    const u32_t (*microcode)[2];

     OUTREG(RADEON_CP_ME_RAM_ADDR, 0);

     if ( (info->ChipFamily  == CHIP_FAMILY_LEGACY ) ||
          (info->ChipFamily  == CHIP_FAMILY_RADEON ) ||
          (info->ChipFamily  == CHIP_FAMILY_RV100  ) ||
          (info->ChipFamily  == CHIP_FAMILY_RV200  ) ||
          (info->ChipFamily  == CHIP_FAMILY_RS100  ) ||
          (info->ChipFamily  == CHIP_FAMILY_RS200  ))
     {
        microcode = R100_cp_microcode;
        dbgprintf("Loading R100 Microcode\n");
     }
     else if ((info->ChipFamily == CHIP_FAMILY_R200 ) ||
              (info->ChipFamily == CHIP_FAMILY_RV250) ||
              (info->ChipFamily == CHIP_FAMILY_RV280) ||
              (info->ChipFamily == CHIP_FAMILY_RS300))
     {
        microcode = R200_cp_microcode;
        dbgprintf("Loading R200 Microcode\n");
     }
     else if ((info->ChipFamily == CHIP_FAMILY_R300)  ||
              (info->ChipFamily == CHIP_FAMILY_R350)  ||
              (info->ChipFamily == CHIP_FAMILY_RV350) ||
              (info->ChipFamily == CHIP_FAMILY_RV380) ||
              (info->ChipFamily == CHIP_FAMILY_RS400) ||
              (info->ChipFamily == CHIP_FAMILY_RS480))
     {
        dbgprintf("Loading R300 Microcode\n");
        microcode = R300_cp_microcode;
     }
     else if ((info->ChipFamily == CHIP_FAMILY_R420)  ||
              (info->ChipFamily == CHIP_FAMILY_RV410))
     {
        dbgprintf("Loading R400 Microcode\n");
        microcode = R420_cp_microcode;

     }
     else if ((info->ChipFamily == CHIP_FAMILY_RS600) ||
              (info->ChipFamily == CHIP_FAMILY_RS690) ||
              (info->ChipFamily == CHIP_FAMILY_RS740))
     {
        dbgprintf("Loading RS690/RS740 Microcode\n");
        microcode = RS690_cp_microcode;
     }
     else if ((info->ChipFamily == CHIP_FAMILY_RV515) ||
              (info->ChipFamily == CHIP_FAMILY_R520)  ||
              (info->ChipFamily == CHIP_FAMILY_RV530) ||
              (info->ChipFamily == CHIP_FAMILY_R580)  ||
              (info->ChipFamily == CHIP_FAMILY_RV560) ||
              (info->ChipFamily == CHIP_FAMILY_RV570))
     {
        dbgprintf("Loading R500 Microcode\n");
        microcode = R520_cp_microcode;
     }

     for (i = 0; i < 256; i++) {
       OUTREG(RADEON_CP_ME_RAM_DATAH, microcode[i][1]);
       OUTREG(RADEON_CP_ME_RAM_DATAL, microcode[i][0]);
     }
}


void init_ring_buffer(RHDPtr info)
{
     u32_t ring_base;
     u32_t tmp;

     info->ringBase = CreateRingBuffer( 64*1024, PG_SW);

     dbgprintf("create cp ring buffer %x\n", rhd.ringBase);
     ring_base = GetPgAddr(rhd.ringBase);
     dbgprintf("ring base %x\n", ring_base);

     OUTREG(RADEON_CP_RB_BASE, ring_base);

     info->ring_avail = 64*1024/4 ;

	/* Set the write pointer delay */
     OUTREG(RADEON_CP_RB_WPTR_DELAY, 0);

	/* Initialize the ring buffer's read and write pointers */
     rhd.ring_rp = rhd.ring_wp = INREG(RADEON_CP_RB_RPTR);
     rhd.host_rp =  rhd.ring_rp;

     OUTREG(RADEON_CP_RB_WPTR,rhd.ring_rp);

     tmp = (((u32_t)&rhd.host_rp) & 4095) + GetPgAddr((void*)&rhd.host_rp);

     OUTREG(RADEON_CP_RB_RPTR_ADDR, tmp); // ring buffer read pointer

	/* Set ring buffer size */
     OUTREG(RADEON_CP_RB_CNTL, (1<<27)|(0<<18)|(10<<8)|13);

	/* Initialize the scratch register pointer.  This will cause
	 * the scratch register values to be written out to memory
	 * whenever they are updated.
	 *
	 * We simply put this behind the ring read pointer, this works
	 * with PCI GART as well as (whatever kind of) AGP GART
	 */

     tmp = (((u32_t)&rhd.scratch0) & 4095) + GetPgAddr((void*)&rhd.scratch0);
     OUTREG(RADEON_SCRATCH_ADDR, tmp);

     OUTREG(RADEON_SCRATCH_UMSK, 0x0);
     //OUTREG(0x778, 1);

	/* Turn on bus mastering */
     if ( (info->ChipFamily == CHIP_FAMILY_RS400) ||
          (info->ChipFamily == CHIP_FAMILY_RS690) ||
          (info->ChipFamily == CHIP_FAMILY_RS740) )
     {
		/* rs400, rs690/rs740 */
        tmp = INREG(RADEON_BUS_CNTL) & ~RS400_BUS_MASTER_DIS;
        OUTREG(RADEON_BUS_CNTL, tmp);
     }
     else if (!((info->ChipFamily == CHIP_FAMILY_RV380) ||
                (info->ChipFamily >= CHIP_FAMILY_R420)))
     {
		/* r1xx, r2xx, r300, r(v)350, r420/r481, rs480 */
        tmp = INREG(RADEON_BUS_CNTL) & ~RADEON_BUS_MASTER_DIS;
        OUTREG(RADEON_BUS_CNTL, tmp);
	} /* PCIE cards appears to not need this */

    tmp = INREG(RADEON_BUS_CNTL) & ~RADEON_BUS_MASTER_DIS;
    OUTREG(RADEON_BUS_CNTL, tmp);

    radeon_do_wait_for_idle();

	/* Sync everything up */
    OUTREG(RADEON_ISYNC_CNTL,
          (RADEON_ISYNC_ANY2D_IDLE3D |
           RADEON_ISYNC_ANY3D_IDLE2D |
           RADEON_ISYNC_WAIT_IDLEGUI |
           RADEON_ISYNC_CPSCRATCH_IDLEGUI));
}


void radeon_engine_reset(RHDPtr info)
{
     u32_t  clock_cntl_index;
     u32_t  mclk_cntl;
     u32_t  rbbm_soft_reset;
     u32_t  host_path_cntl;

    if (info->ChipFamily <= CHIP_FAMILY_RV410)
    {
	        /* may need something similar for newer chips */
        clock_cntl_index = INREG(RADEON_CLOCK_CNTL_INDEX);
        mclk_cntl = INPLL( RADEON_MCLK_CNTL);

        OUTPLL(RADEON_MCLK_CNTL, (mclk_cntl |
						    RADEON_FORCEON_MCLKA |
						    RADEON_FORCEON_MCLKB |
						    RADEON_FORCEON_YCLKA |
						    RADEON_FORCEON_YCLKB |
						    RADEON_FORCEON_MC |
						    RADEON_FORCEON_AIC));
	}

    rbbm_soft_reset = INREG(RADEON_RBBM_SOFT_RESET);

    OUTREG(RADEON_RBBM_SOFT_RESET, (rbbm_soft_reset |
					      RADEON_SOFT_RESET_CP |
					      RADEON_SOFT_RESET_HI |
					      RADEON_SOFT_RESET_SE |
					      RADEON_SOFT_RESET_RE |
					      RADEON_SOFT_RESET_PP |
					      RADEON_SOFT_RESET_E2 |
					      RADEON_SOFT_RESET_RB));
    INREG(RADEON_RBBM_SOFT_RESET);
    OUTREG(RADEON_RBBM_SOFT_RESET, (rbbm_soft_reset &
					      ~(RADEON_SOFT_RESET_CP |
						RADEON_SOFT_RESET_HI |
						RADEON_SOFT_RESET_SE |
						RADEON_SOFT_RESET_RE |
						RADEON_SOFT_RESET_PP |
						RADEON_SOFT_RESET_E2 |
						RADEON_SOFT_RESET_RB)));
    INREG(RADEON_RBBM_SOFT_RESET);

    if (info->ChipFamily <= CHIP_FAMILY_RV410) {
        OUTPLL(RADEON_MCLK_CNTL, mclk_cntl);
        OUTREG(RADEON_CLOCK_CNTL_INDEX, clock_cntl_index);
        OUTREG(RADEON_RBBM_SOFT_RESET, rbbm_soft_reset);
	}
 };

#define RADEON_WAIT_UNTIL_IDLE() do {                   \
    OUT_RING( CP_PACKET0( RADEON_WAIT_UNTIL, 1 ) );         \
	OUT_RING( (RADEON_WAIT_2D_IDLECLEAN |				\
		   RADEON_WAIT_3D_IDLECLEAN |				\
		   RADEON_WAIT_HOST_IDLECLEAN) );			\
} while (0)

#define R300_ZB_ZCACHE_CTLSTAT                  0x4f18
#   define RADEON_RB3D_ZC_FLUSH     (1 << 0)
#	define RADEON_RB3D_ZC_FREE		(1 << 2)
#	define RADEON_RB3D_ZC_FLUSH_ALL		0x5
#   define RADEON_RB3D_ZC_BUSY      (1 << 31)
#   define R300_ZC_FLUSH                (1 << 0)
#	define R300_ZC_FREE		        (1 << 1)
#   define R300_ZC_BUSY             (1 << 31)
#   define RADEON_RB3D_DC_FLUSH     (3 << 0)
#	define RADEON_RB3D_DC_FREE		(3 << 2)
#	define RADEON_RB3D_DC_FLUSH_ALL		0xf
#   define RADEON_RB3D_DC_BUSY      (1 << 31)
#   define R300_RB3D_DC_FLUSH       (2 << 0)
#	define R300_RB3D_DC_FREE		(2 << 2)
#
#define RADEON_PURGE_CACHE() do {                                    \
    if ( rhd.ChipFamily <= CHIP_FAMILY_RV280) {                      \
            OUT_RING(CP_PACKET0( RADEON_RB3D_DSTCACHE_CTLSTAT, 1));  \
            OUT_RING(RADEON_RB3D_DC_FLUSH | RADEON_RB3D_DC_FREE);    \
    } else {                                                         \
            OUT_RING(CP_PACKET0(R300_RB3D_DSTCACHE_CTLSTAT, 1));     \
            OUT_RING(R300_RB3D_DC_FLUSH | R300_RB3D_DC_FREE );       \
        }                                                            \
} while (0)

#define RADEON_FLUSH_ZCACHE() do {                                   \
    if ( rhd.ChipFamily <= CHIP_FAMILY_RV280) {     \
            OUT_RING( CP_PACKET0( RADEON_RB3D_ZCACHE_CTLSTAT, 1 ) ); \
            OUT_RING( RADEON_RB3D_ZC_FLUSH );                        \
    } else {                                                         \
            OUT_RING( CP_PACKET0( R300_ZB_ZCACHE_CTLSTAT, 1 ) );     \
            OUT_RING( R300_ZC_FLUSH );                               \
        }                                                            \
} while (0)
#define RADEON_PURGE_ZCACHE() do {                  \
    if (rhd.ChipFamily <= CHIP_FAMILY_RV280) { \
            OUT_RING(CP_PACKET0(RADEON_RB3D_ZCACHE_CTLSTAT, 1));    \
	        OUT_RING(RADEON_RB3D_ZC_FLUSH | RADEON_RB3D_ZC_FREE);	\
	} else {                                                        \
            OUT_RING(CP_PACKET0(R300_ZB_ZCACHE_CTLSTAT, 1));    \
	        OUT_RING(R300_ZC_FLUSH | R300_ZC_FREE);			\
        }                                                               \
} while (0)

static int radeon_cp_start(RHDPtr info)
{
     u32_t *ring, write;
     u32_t ifl;
     radeon_do_wait_for_idle(64);

     OUTREG(RADEON_CP_CSQ_CNTL, RADEON_CSQ_PRIBM_INDBM);

     ifl = safe_cli();

     BEGIN_RING(8);
	/* isync can only be written through cp on r5xx write it here */
       OUT_RING(CP_PACKET0(RADEON_ISYNC_CNTL, 1));
       OUT_RING(RADEON_ISYNC_ANY2D_IDLE3D |
                RADEON_ISYNC_ANY3D_IDLE2D |
                RADEON_ISYNC_WAIT_IDLEGUI |
                RADEON_ISYNC_CPSCRATCH_IDLEGUI);
       RADEON_PURGE_CACHE();
       RADEON_PURGE_ZCACHE();
       RADEON_WAIT_UNTIL_IDLE();
       ADVANCE_RING();
    COMMIT_RING();

    safe_sti(ifl);

    radeon_do_wait_for_idle();
};


Bool init_cp(RHDPtr info)
{
     load_microcode(&rhd);

     init_ring_buffer(&rhd);

     radeon_engine_reset(&rhd);

    /* setup the raster pipes */
     init_pipes(&rhd);

     rhd.ring_rp = rhd.ring_wp = INREG(RADEON_CP_RB_RPTR);
     OUTREG(RADEON_CP_RB_WPTR, rhd.ring_rp);

     radeon_cp_start(&rhd);

};


