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

     rhd.ring_rp = rhd.ring_wp = INREG(RADEON_CP_RB_RPTR);
     OUTREG(RADEON_CP_RB_WPTR, rhd.ring_rp);

     radeon_cp_start(&rhd);

    return TRUE;
};


