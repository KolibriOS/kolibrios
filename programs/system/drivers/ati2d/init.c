

static Bool rhdMapMMIO(RHDPtr rhdPtr)
{
     rhdPtr->MMIOMapSize = 1 << rhdPtr->memsize[RHD_MMIO_BAR];
     rhdPtr->MMIOBase = MapIoMem((void*)rhdPtr->memBase[RHD_MMIO_BAR],
                                 rhdPtr->MMIOMapSize,PG_SW+PG_NOCACHE);
     if( rhdPtr->MMIOBase==0)
        return 0;

     DBG(dbgprintf("Mapped IO at %x (size %x)\n", rhdPtr->MMIOBase, rhdPtr->MMIOMapSize));
     return 1;
}

/* Read MC register */
unsigned INMC(RHDPtr info, int addr)
{
    u32_t       data;

    if ((info->ChipFamily == CHIP_FAMILY_RS690) ||
       (info->ChipFamily == CHIP_FAMILY_RS740)) {
       OUTREG(RS690_MC_INDEX, (addr & RS690_MC_INDEX_MASK));
       data = INREG(RS690_MC_DATA);
    } else if (info->ChipFamily == CHIP_FAMILY_RS600) {
       OUTREG(RS600_MC_INDEX, (addr & RS600_MC_INDEX_MASK));
       data = INREG(RS600_MC_DATA);
    } else if (IS_AVIVO_VARIANT) {
       OUTREG(AVIVO_MC_INDEX, (addr & 0xff) | 0x7f0000);
       (void)INREG(AVIVO_MC_INDEX);
       data = INREG(AVIVO_MC_DATA);

       OUTREG(AVIVO_MC_INDEX, 0);
       (void)INREG(AVIVO_MC_INDEX);
    } else {
       OUTREG(R300_MC_IND_INDEX, addr & 0x3f);
       (void)INREG(R300_MC_IND_INDEX);
       data = INREG(R300_MC_IND_DATA);

       OUTREG(R300_MC_IND_INDEX, 0);
       (void)INREG(R300_MC_IND_INDEX);
    }

    return data;
}

     /* Write MC information */
void OUTMC(RHDPtr info, int addr, u32_t data)
{
     if ((info->ChipFamily == CHIP_FAMILY_RS690) ||
         (info->ChipFamily == CHIP_FAMILY_RS740)) {
        OUTREG(RS690_MC_INDEX, ((addr & RS690_MC_INDEX_MASK) | RS690_MC_INDEX_WR_EN));
        OUTREG(RS690_MC_DATA, data);
        OUTREG(RS690_MC_INDEX, RS690_MC_INDEX_WR_ACK);
     }
     else if (info->ChipFamily == CHIP_FAMILY_RS600) {
        OUTREG(RS600_MC_INDEX, ((addr & RS600_MC_INDEX_MASK) | RS600_MC_INDEX_WR_EN));
        OUTREG(RS600_MC_DATA, data);
        OUTREG(RS600_MC_INDEX, RS600_MC_INDEX_WR_ACK);
     }
     else if (IS_AVIVO_VARIANT) {
        OUTREG(AVIVO_MC_INDEX, (addr & 0xff) | 0xff0000);
        (void)INREG(AVIVO_MC_INDEX);
        OUTREG(AVIVO_MC_DATA, data);
        OUTREG(AVIVO_MC_INDEX, 0);
        (void)INREG(AVIVO_MC_INDEX);
     }
     else {
        OUTREG(R300_MC_IND_INDEX, (((addr) & 0x3f) | R300_MC_IND_WR_EN));
        (void)INREG(R300_MC_IND_INDEX);
        OUTREG(R300_MC_IND_DATA, data);
        OUTREG(R300_MC_IND_INDEX, 0);
        (void)INREG(R300_MC_IND_INDEX);
     }
}

static Bool avivo_get_mc_idle(RHDPtr info)
{

     if (info->ChipFamily >= CHIP_FAMILY_R600) {
	/* no idea where this is on r600 yet */
        return TRUE;
     }
     else if (info->ChipFamily == CHIP_FAMILY_RV515) {
        if (INMC(info, RV515_MC_STATUS) & RV515_MC_STATUS_IDLE)
           return TRUE;
        else
           return FALSE;
     }
     else if (info->ChipFamily == CHIP_FAMILY_RS600)
     {
        if (INMC(info, RS600_MC_STATUS) & RS600_MC_STATUS_IDLE)
           return TRUE;
        else
           return FALSE;
     }
     else if ((info->ChipFamily == CHIP_FAMILY_RS690) ||
	       (info->ChipFamily == CHIP_FAMILY_RS740)) {
        if (INMC(info, RS690_MC_STATUS) & RS690_MC_STATUS_IDLE)
           return TRUE;
        else
           return FALSE;
    }
    else {
       if (INMC(info, R520_MC_STATUS) & R520_MC_STATUS_IDLE)
           return TRUE;
       else
           return FALSE;
    }
}

#define LOC_FB 0x1
#define LOC_AGP 0x2

static void radeon_read_mc_fb_agp_location(RHDPtr info, int mask,
                                u32_t *fb_loc, u32_t *agp_loc, u32_t *agp_loc_hi)
{

    if (info->ChipFamily >= CHIP_FAMILY_RV770) {
       if (mask & LOC_FB)
	    *fb_loc = INREG(R700_MC_VM_FB_LOCATION);
	if (mask & LOC_AGP) {
	    *agp_loc = INREG(R600_MC_VM_AGP_BOT);
	    *agp_loc_hi = INREG(R600_MC_VM_AGP_TOP);
	}
    } else if (info->ChipFamily >= CHIP_FAMILY_R600) {
	if (mask & LOC_FB)
	    *fb_loc = INREG(R600_MC_VM_FB_LOCATION);
	if (mask & LOC_AGP) {
	    *agp_loc = INREG(R600_MC_VM_AGP_BOT);
	    *agp_loc_hi = INREG(R600_MC_VM_AGP_TOP);
	}
    } else if (info->ChipFamily == CHIP_FAMILY_RV515) {
	if (mask & LOC_FB)
        *fb_loc = INMC(info, RV515_MC_FB_LOCATION);
	if (mask & LOC_AGP) {
        *agp_loc = INMC(info, RV515_MC_AGP_LOCATION);
	    *agp_loc_hi = 0;
	}
    } else if (info->ChipFamily == CHIP_FAMILY_RS600) {
	if (mask & LOC_FB)
        *fb_loc = INMC(info, RS600_MC_FB_LOCATION);
	if (mask & LOC_AGP) {
	    *agp_loc = 0;//INMC(pScrn, RS600_MC_AGP_LOCATION);
	    *agp_loc_hi = 0;
	}
    } else if ((info->ChipFamily == CHIP_FAMILY_RS690) ||
	       (info->ChipFamily == CHIP_FAMILY_RS740)) {
	if (mask & LOC_FB)
        *fb_loc = INMC(info, RS690_MC_FB_LOCATION);
	if (mask & LOC_AGP) {
        *agp_loc = INMC(info, RS690_MC_AGP_LOCATION);
	    *agp_loc_hi = 0;
	}
    } else if (info->ChipFamily >= CHIP_FAMILY_R520) {
	if (mask & LOC_FB)
        *fb_loc = INMC(info, R520_MC_FB_LOCATION);
	if (mask & LOC_AGP) {
        *agp_loc = INMC(info, R520_MC_AGP_LOCATION);
	    *agp_loc_hi = 0;
	}
    } else {
	if (mask & LOC_FB)
	    *fb_loc = INREG(RADEON_MC_FB_LOCATION);
	if (mask & LOC_AGP)
	    *agp_loc = INREG(RADEON_MC_AGP_LOCATION);
    }
}

static void radeon_write_mc_fb_agp_location(RHDPtr info, int mask, u32_t fb_loc,
                      u32_t agp_loc, u32_t agp_loc_hi)
{

     if (info->ChipFamily >= CHIP_FAMILY_RV770) {
        if (mask & LOC_FB)
           OUTREG(R700_MC_VM_FB_LOCATION, fb_loc);
        if (mask & LOC_AGP) {
           OUTREG(R600_MC_VM_AGP_BOT, agp_loc);
           OUTREG(R600_MC_VM_AGP_TOP, agp_loc_hi);
        }
     }
     else if (info->ChipFamily >= CHIP_FAMILY_R600)
     {
        if (mask & LOC_FB)
           OUTREG(R600_MC_VM_FB_LOCATION, fb_loc);
        if (mask & LOC_AGP) {
           OUTREG(R600_MC_VM_AGP_BOT, agp_loc);
           OUTREG(R600_MC_VM_AGP_TOP, agp_loc_hi);
        }
     }
     else if (info->ChipFamily == CHIP_FAMILY_RV515)
     {
        if (mask & LOC_FB)
           OUTMC(info, RV515_MC_FB_LOCATION, fb_loc);
        if (mask & LOC_AGP)
           OUTMC(info, RV515_MC_AGP_LOCATION, agp_loc);
           (void)INMC(info, RV515_MC_AGP_LOCATION);
     }
     else if (info->ChipFamily == CHIP_FAMILY_RS600)
     {
     if (mask & LOC_FB)
        OUTMC(info, RS600_MC_FB_LOCATION, fb_loc);
	/*	if (mask & LOC_AGP)
		OUTMC(pScrn, RS600_MC_AGP_LOCATION, agp_loc);*/
     }
     else if ((info->ChipFamily == CHIP_FAMILY_RS690) ||
              (info->ChipFamily == CHIP_FAMILY_RS740))
     {
        if (mask & LOC_FB)
           OUTMC(info, RS690_MC_FB_LOCATION, fb_loc);
        if (mask & LOC_AGP)
           OUTMC(info, RS690_MC_AGP_LOCATION, agp_loc);
     }
     else if (info->ChipFamily >= CHIP_FAMILY_R520)
     {
        if (mask & LOC_FB)
           OUTMC(info, R520_MC_FB_LOCATION, fb_loc);
        if (mask & LOC_AGP)
           OUTMC(info, R520_MC_AGP_LOCATION, agp_loc);
           (void)INMC(info, R520_MC_FB_LOCATION);
     }
     else {
        if (mask & LOC_FB)
           OUTREG(RADEON_MC_FB_LOCATION, fb_loc);
        if (mask & LOC_AGP)
           OUTREG(RADEON_MC_AGP_LOCATION, agp_loc);
    }
}


static void RADEONUpdateMemMapRegisters(RHDPtr info)
{
     u32_t timeout;

     u32_t mc_fb_loc, mc_agp_loc, mc_agp_loc_hi;

     radeon_read_mc_fb_agp_location(info, LOC_FB | LOC_AGP, &mc_fb_loc,
				   &mc_agp_loc, &mc_agp_loc_hi);

     if (IS_AVIVO_VARIANT)
     {

        if (mc_fb_loc  != info->mc_fb_location  ||
            mc_agp_loc != info->mc_agp_location)
        {
           u32_t d1crtc, d2crtc;
           u32_t tmp;
//           RADEONWaitForIdleMMIO(pScrn);

           OUTREG(AVIVO_D1VGA_CONTROL, INREG(AVIVO_D1VGA_CONTROL) & ~AVIVO_DVGA_CONTROL_MODE_ENABLE);
           OUTREG(AVIVO_D2VGA_CONTROL, INREG(AVIVO_D2VGA_CONTROL) & ~AVIVO_DVGA_CONTROL_MODE_ENABLE);

           /* Stop display & memory access */
           d1crtc = INREG(AVIVO_D1CRTC_CONTROL);
           OUTREG(AVIVO_D1CRTC_CONTROL, d1crtc & ~AVIVO_CRTC_EN);

           d2crtc = INREG(AVIVO_D2CRTC_CONTROL);
           OUTREG(AVIVO_D2CRTC_CONTROL, d2crtc & ~AVIVO_CRTC_EN);

           tmp = INREG(AVIVO_D2CRTC_CONTROL);

           usleep(10000);
           timeout = 0;
           while (!(avivo_get_mc_idle(info)))
           {
              if (++timeout > 1000000)
              {
                 dbgprintf("Timeout trying to update memory controller settings !\n");
                 dbgprintf("You will probably crash now ... \n");
               /* Nothing we can do except maybe try to kill the server,
                * let's wait 2 seconds to leave the above message a chance
                * to maybe hit the disk and continue trying to setup despite
                * the MC being non-idle
                */
                 usleep(2000000);
              }
              usleep(10);
           }

           radeon_write_mc_fb_agp_location(info, LOC_FB | LOC_AGP,
                           info->mc_fb_location,
                           info->mc_agp_location,
                           info->mc_agp_location_hi);

           if (info->ChipFamily < CHIP_FAMILY_R600) {
              OUTREG(AVIVO_HDP_FB_LOCATION, info->mc_fb_location);
           }
           else {
              OUTREG(R600_HDP_NONSURFACE_BASE, (info->mc_fb_location << 16) & 0xff0000);
           }

           OUTREG(AVIVO_D1CRTC_CONTROL, d1crtc );

           OUTREG(AVIVO_D2CRTC_CONTROL, d2crtc );

           tmp = INREG(AVIVO_D2CRTC_CONTROL);

           /* Reset the engine and HDP */
//           RADEONEngineReset(pScrn);
       }
     }
     else
     {

	/* Write memory mapping registers only if their value change
	 * since we must ensure no access is done while they are
	 * reprogrammed
	 */
        if ( mc_fb_loc != info->mc_fb_location   ||
             mc_agp_loc != info->mc_agp_location)
        {
           u32_t crtc_ext_cntl, crtc_gen_cntl, crtc2_gen_cntl=0, ov0_scale_cntl;
           u32_t old_mc_status, status_idle;

           dbgprintf("  Map Changed ! Applying ...\n");

	    /* Make sure engine is idle. We assume the CCE is stopped
	     * at this point
	     */
   //        RADEONWaitForIdleMMIO(info);

           if (info->IsIGP)
              goto igp_no_mcfb;

	    /* Capture MC_STATUS in case things go wrong ... */
           old_mc_status = INREG(RADEON_MC_STATUS);

	    /* Stop display & memory access */
           ov0_scale_cntl = INREG(RADEON_OV0_SCALE_CNTL);
           OUTREG(RADEON_OV0_SCALE_CNTL, ov0_scale_cntl & ~RADEON_SCALER_ENABLE);
           crtc_ext_cntl = INREG(RADEON_CRTC_EXT_CNTL);
           OUTREG(RADEON_CRTC_EXT_CNTL, crtc_ext_cntl | RADEON_CRTC_DISPLAY_DIS);
           crtc_gen_cntl = INREG(RADEON_CRTC_GEN_CNTL);
//           RADEONWaitForVerticalSync(pScrn);
           OUTREG(RADEON_CRTC_GEN_CNTL,
                 (crtc_gen_cntl  & ~(RADEON_CRTC_CUR_EN | RADEON_CRTC_ICON_EN))
                 | RADEON_CRTC_DISP_REQ_EN_B | RADEON_CRTC_EXT_DISP_EN);

           if (info->HasCRTC2)
           {
              crtc2_gen_cntl = INREG(RADEON_CRTC2_GEN_CNTL);
//              RADEONWaitForVerticalSync2(pScrn);
              OUTREG(RADEON_CRTC2_GEN_CNTL, (crtc2_gen_cntl
                     & ~(RADEON_CRTC2_CUR_EN | RADEON_CRTC2_ICON_EN))
                    | RADEON_CRTC2_DISP_REQ_EN_B);
           }

	    /* Make sure the chip settles down (paranoid !) */
           usleep(1000);

	    /* Wait for MC idle */
           if (IS_R300_VARIANT)
              status_idle = R300_MC_IDLE;
           else
              status_idle = RADEON_MC_IDLE;

           timeout = 0;
           while (!(INREG(RADEON_MC_STATUS) & status_idle))
           {
              if (++timeout > 1000000)
              {
                 dbgprintf("Timeout trying to update memory controller settings !\n");
                 dbgprintf("MC_STATUS = 0x%08x (on entry = 0x%08x)\n",
                           INREG(RADEON_MC_STATUS), old_mc_status);
                 dbgprintf("You will probably crash now ... \n");
		    /* Nothing we can do except maybe try to kill the server,
		     * let's wait 2 seconds to leave the above message a chance
		     * to maybe hit the disk and continue trying to setup despite
		     * the MC being non-idle
		     */
                 usleep(20000);
              }
              usleep(10);
           }

	    /* Update maps, first clearing out AGP to make sure we don't get
	     * a temporary overlap
	     */
           OUTREG(RADEON_MC_AGP_LOCATION, 0xfffffffc);
           OUTREG(RADEON_MC_FB_LOCATION, info->mc_fb_location);
           radeon_write_mc_fb_agp_location(info, LOC_FB | LOC_AGP, info->mc_fb_location,
                           0xfffffffc, 0);

           OUTREG(RADEON_CRTC_GEN_CNTL,crtc_gen_cntl );
           OUTREG(RADEON_CRTC_EXT_CNTL, crtc_ext_cntl);
           OUTREG(RADEON_OV0_SCALE_CNTL, ov0_scale_cntl );


 igp_no_mcfb:
           radeon_write_mc_fb_agp_location(info, LOC_AGP, 0,
                        info->mc_agp_location, 0);
	    /* Make sure map fully reached the chip */
           (void)INREG(RADEON_MC_FB_LOCATION);

           dbgprintf("  Map applied, resetting engine ...\n");

	    /* Reset the engine and HDP */
//        RADEONEngineReset(pScrn);

	    /* Make sure we have sane offsets before re-enabling the CRTCs, disable
	     * stereo, clear offsets, and wait for offsets to catch up with hw
	     */

           OUTREG(RADEON_CRTC_OFFSET_CNTL, RADEON_CRTC_OFFSET_FLIP_CNTL);
           OUTREG(RADEON_CRTC_OFFSET, 0);
           OUTREG(RADEON_CUR_OFFSET, 0);
           timeout = 0;
           while(INREG(RADEON_CRTC_OFFSET) & RADEON_CRTC_OFFSET__GUI_TRIG_OFFSET)
           {
              if (timeout++ > 1000000) {
                 dbgprintf("Timeout waiting for CRTC offset to update !\n");
                 break;
              }
              usleep(1000);
           }
           if (info->HasCRTC2)
           {
              OUTREG(RADEON_CRTC2_OFFSET_CNTL, RADEON_CRTC2_OFFSET_FLIP_CNTL);
              OUTREG(RADEON_CRTC2_OFFSET, 0);
              OUTREG(RADEON_CUR2_OFFSET, 0);
              timeout = 0;
              while(INREG(RADEON_CRTC2_OFFSET) & RADEON_CRTC2_OFFSET__GUI_TRIG_OFFSET)
              {
                 if (timeout++ > 1000000) {
                    dbgprintf("Timeout waiting for CRTC2 offset to update !\n");
                    break;
                 }
                 usleep(1000);
              }
           }
        }

        dbgprintf("Updating display base addresses...\n");

        OUTREG(RADEON_DISPLAY_BASE_ADDR, info->fbLocation);
        if (info->HasCRTC2)
           OUTREG(RADEON_DISPLAY2_BASE_ADDR, info->fbLocation);
        OUTREG(RADEON_OV0_BASE_ADDR, info->fbLocation);
        (void)INREG(RADEON_OV0_BASE_ADDR);

	/* More paranoia delays, wait 100ms */
        usleep(1000);

        dbgprintf("Memory map updated.\n");
     };
};


static void RADEONInitMemoryMap(RHDPtr info)
{
     u32_t       mem_size;
     u32_t       aper_size;

     radeon_read_mc_fb_agp_location(info, LOC_FB | LOC_AGP, &info->mc_fb_location,
                    &info->mc_agp_location, &info->mc_agp_location_hi);

      dbgprintf("  MC_FB_LOCATION   : 0x%08x\n", (unsigned)info->mc_fb_location);
      dbgprintf("  MC_AGP_LOCATION  : 0x%08x\n", (unsigned)info->mc_agp_location);


     /* We shouldn't use info->videoRam here which might have been clipped
      * but the real video RAM instead
      */
     if (info->ChipFamily >= CHIP_FAMILY_R600){
        mem_size = INREG(R600_CONFIG_MEMSIZE);
        aper_size = INREG(R600_CONFIG_APER_SIZE);
     }
     else {
        mem_size = INREG(RADEON_CONFIG_MEMSIZE);
        aper_size = INREG(RADEON_CONFIG_APER_SIZE);
     }

     if (mem_size == 0)
        mem_size = 0x800000;

     /* Fix for RN50, M6, M7 with 8/16/32(??) MBs of VRAM -
        Novell bug 204882 + along with lots of ubuntu ones */
     if (aper_size > mem_size)
        mem_size = aper_size;


     if ( (info->ChipFamily != CHIP_FAMILY_RS600) &&
          (info->ChipFamily != CHIP_FAMILY_RS690) &&
          (info->ChipFamily != CHIP_FAMILY_RS740)) {
        if (info->IsIGP)
           info->mc_fb_location = INREG(RADEON_NB_TOM);
        else
        {
           u32_t aper0_base;

           if (info->ChipFamily >= CHIP_FAMILY_R600) {
              aper0_base = INREG(R600_CONFIG_F0_BASE);
           }
           else {
              aper0_base = INREG(RADEON_CONFIG_APER_0_BASE);
           }
           dbgprintf("aper0 base %x\n", aper0_base );

         /* Recent chips have an "issue" with the memory controller, the
          * location must be aligned to the size. We just align it down,
          * too bad if we walk over the top of system memory, we don't
          * use DMA without a remapped anyway.
          * Affected chips are rv280, all r3xx, and all r4xx, but not IGP
          */
           if ( info->ChipFamily == CHIP_FAMILY_RV280 ||
                info->ChipFamily == CHIP_FAMILY_R300 ||
                info->ChipFamily == CHIP_FAMILY_R350 ||
                info->ChipFamily == CHIP_FAMILY_RV350 ||
                info->ChipFamily == CHIP_FAMILY_RV380 ||
                info->ChipFamily == CHIP_FAMILY_R420 ||
                info->ChipFamily == CHIP_FAMILY_RV410)
              aper0_base &= ~(mem_size - 1);

           if ( info->ChipFamily >= CHIP_FAMILY_R600) {
              info->mc_fb_location = (aper0_base >> 24) |
              (((aper0_base + mem_size - 1) & 0xff000000U) >> 8);
              dbgprintf("mc fb loc is %08x\n", (unsigned int)info->mc_fb_location);
           }
           else {
              info->mc_fb_location = (aper0_base >> 16) |
              ((aper0_base + mem_size - 1) & 0xffff0000U);
              dbgprintf("mc fb loc is %08x\n", (unsigned int)info->mc_fb_location);
           }
        }
     }
     if (info->ChipFamily >= CHIP_FAMILY_R600) {
        info->fbLocation = (info->mc_fb_location & 0xffff) << 24;
     }
     else {
        info->fbLocation = (info->mc_fb_location & 0xffff) << 16;
     }
     /* Just disable the damn AGP apertures for now, it may be
      * re-enabled later by the DRM
      */

//     if (IS_AVIVO_VARIANT) {
//        if (info->ChipFamily >= CHIP_FAMILY_R600) {
//           OUTREG(R600_HDP_NONSURFACE_BASE, (info->mc_fb_location << 16) & 0xff0000);
//        }
//        else {
//           OUTREG(AVIVO_HDP_FB_LOCATION, info->mc_fb_location);
//        }
//        info->mc_agp_location = 0x003f0000;
//     }
//     else
//         info->mc_agp_location = 0xffffffc0;

      dbgprintf("RADEONInitMemoryMap() : \n");
      dbgprintf("  mem_size         : 0x%08x\n", (u32_t)mem_size);
      dbgprintf("  MC_FB_LOCATION   : 0x%08x\n", (unsigned)info->mc_fb_location);
      dbgprintf("  MC_AGP_LOCATION  : 0x%08x\n", (unsigned)info->mc_agp_location);
      dbgprintf("  FB_LOCATION   : 0x%08x\n", (unsigned)info->fbLocation);

      RADEONUpdateMemMapRegisters(info);
}

static void RADEONGetVRamType(RHDPtr info)
{
     u32_t tmp;

     if (info->IsIGP || (info->ChipFamily >= CHIP_FAMILY_R300))
        info->IsDDR = TRUE;
     else if (INREG(RADEON_MEM_SDRAM_MODE_REG) & RADEON_MEM_CFG_TYPE_DDR)
        info->IsDDR = TRUE;
     else
        info->IsDDR = FALSE;

     if ( (info->ChipFamily >= CHIP_FAMILY_R600) &&
          (info->ChipFamily <= CHIP_FAMILY_RV635))
     {
        int chansize;
        /* r6xx */
        tmp = INREG(R600_RAMCFG);
        if (tmp & R600_CHANSIZE_OVERRIDE)
            chansize = 16;
        else if (tmp & R600_CHANSIZE)
            chansize = 64;
        else
            chansize = 32;
        if (info->ChipFamily == CHIP_FAMILY_R600)
            info->RamWidth = 8 * chansize;
        else if (info->ChipFamily == CHIP_FAMILY_RV670)
            info->RamWidth = 4 * chansize;
        else if ((info->ChipFamily == CHIP_FAMILY_RV610) ||
             (info->ChipFamily == CHIP_FAMILY_RV620))
            info->RamWidth = chansize;
        else if ((info->ChipFamily == CHIP_FAMILY_RV630) ||
             (info->ChipFamily == CHIP_FAMILY_RV635))
            info->RamWidth = 2 * chansize;
     }
     else if (info->ChipFamily == CHIP_FAMILY_RV515) {
     /* rv515/rv550 */
        tmp = INMC(info, RV515_MC_CNTL);
        tmp &= RV515_MEM_NUM_CHANNELS_MASK;
        switch (tmp) {
           case 0: info->RamWidth = 64; break;
           case 1: info->RamWidth = 128; break;
           default: info->RamWidth = 128; break;
        }
     }
     else if ((info->ChipFamily >= CHIP_FAMILY_R520) &&
              (info->ChipFamily <= CHIP_FAMILY_RV570)){
     /* r520/rv530/rv560/rv570/r580 */
        tmp = INMC(info, R520_MC_CNTL0);
        switch ((tmp & R520_MEM_NUM_CHANNELS_MASK) >> R520_MEM_NUM_CHANNELS_SHIFT) {
           case 0: info->RamWidth = 32; break;
           case 1: info->RamWidth = 64; break;
           case 2: info->RamWidth = 128; break;
           case 3: info->RamWidth = 256; break;
           default: info->RamWidth = 64; break;
        }
        if (tmp & R520_MC_CHANNEL_SIZE) {
           info->RamWidth *= 2;
        }
     }
     else if ((info->ChipFamily >= CHIP_FAMILY_R300) &&
              (info->ChipFamily <= CHIP_FAMILY_RV410)) {
     /* r3xx, r4xx */
        tmp = INREG(RADEON_MEM_CNTL);
        tmp &= R300_MEM_NUM_CHANNELS_MASK;
        switch (tmp) {
           case 0: info->RamWidth = 64; break;
           case 1: info->RamWidth = 128; break;
           case 2: info->RamWidth = 256; break;
           default: info->RamWidth = 128; break;
        }
     }
     else if ((info->ChipFamily == CHIP_FAMILY_RV100) ||
              (info->ChipFamily == CHIP_FAMILY_RS100) ||
              (info->ChipFamily == CHIP_FAMILY_RS200)){
     tmp = INREG(RADEON_MEM_CNTL);
     if (tmp & RV100_HALF_MODE)
        info->RamWidth = 32;
     else
        info->RamWidth = 64;

     if (!info->HasCRTC2) {
          info->RamWidth /= 4;
          info->IsDDR = TRUE;
     }
     }
     else if (info->ChipFamily <= CHIP_FAMILY_RV280) {
        tmp = INREG(RADEON_MEM_CNTL);
     if (tmp & RADEON_MEM_NUM_CHANNELS_MASK)
        info->RamWidth = 128;
     else
        info->RamWidth = 64;
     } else {
     /* newer IGPs */
        info->RamWidth = 128;
     }

     /* This may not be correct, as some cards can have half of channel disabled
      * ToDo: identify these cases
      */
}

/*
 * Depending on card genertation, chipset bugs, etc... the amount of vram
 * accessible to the CPU can vary. This function is our best shot at figuring
 * it out. Returns a value in KB.
 */
static u32_t RADEONGetAccessibleVRAM(RHDPtr info)
{
    u32_t       aper_size;
    unsigned char  byte;

    if (info->ChipFamily >= CHIP_FAMILY_R600)
       aper_size = INREG(R600_CONFIG_APER_SIZE) / 1024;
    else
       aper_size = INREG(RADEON_CONFIG_APER_SIZE) / 1024;


    /* Set HDP_APER_CNTL only on cards that are known not to be broken,
     * that is has the 2nd generation multifunction PCI interface
     */
    if (info->ChipFamily == CHIP_FAMILY_RV280 ||
        info->ChipFamily == CHIP_FAMILY_RV350 ||
        info->ChipFamily == CHIP_FAMILY_RV380 ||
        info->ChipFamily == CHIP_FAMILY_R420  ||
        info->ChipFamily == CHIP_FAMILY_RV410 ||
        IS_AVIVO_VARIANT) {
        MASKREG (RADEON_HOST_PATH_CNTL, RADEON_HDP_APER_CNTL,
		     ~RADEON_HDP_APER_CNTL);
        dbgprintf("Generation 2 PCI interface, using max accessible memory\n");
	    return aper_size * 2;
    }

    /* Older cards have all sorts of funny issues to deal with. First
     * check if it's a multifunction card by reading the PCI config
     * header type... Limit those to one aperture size
     */
    byte = pciReadByte(info->PciTag, 0xe);
    if (byte & 0x80) {
       dbgprintf("Generation 1 PCI interface in multifunction mode, "
                 "accessible memory limited to one aperture\n");
       return aper_size;
    }

    /* Single function older card. We read HDP_APER_CNTL to see how the BIOS
     * have set it up. We don't write this as it's broken on some ASICs but
     * we expect the BIOS to have done the right thing (might be too optimistic...)
     */
    if (INREG(RADEON_HOST_PATH_CNTL) & RADEON_HDP_APER_CNTL)
       return aper_size * 2;

    return aper_size;
}

int RADEONDRIGetPciAperTableSize(RHDPtr info)
{
    int ret_size;
    int num_pages;

    num_pages = (info->pciAperSize * 1024 * 1024) / 4096;

    ret_size = num_pages * sizeof(unsigned int);

    return ret_size;
}

static Bool RADEONPreInitVRAM(RHDPtr info)
{
     u32_t accessible, bar_size;

     if ((!IS_AVIVO_VARIANT) && info->IsIGP)
     {
        u32_t tom = INREG(RADEON_NB_TOM);

        info->videoRam = (((tom >> 16) -
                 (tom & 0xffff) + 1) << 6);

        OUTREG(RADEON_CONFIG_MEMSIZE, info->videoRam * 1024);
     }
     else
     {
        if (info->ChipFamily >= CHIP_FAMILY_R600)
           info->videoRam = INREG(R600_CONFIG_MEMSIZE) / 1024;
        else
        {
         /* Read VRAM size from card */
           info->videoRam      = INREG(RADEON_CONFIG_MEMSIZE) / 1024;

         /* Some production boards of m6 will return 0 if it's 8 MB */
           if (info->videoRam == 0)
           {
              info->videoRam = 8192;
              OUTREG(RADEON_CONFIG_MEMSIZE, 0x800000);
           }
        }
     }

     RADEONGetVRamType(info);

     /* Get accessible memory */
     accessible = RADEONGetAccessibleVRAM(info);

     /* Crop it to the size of the PCI BAR */
//     bar_size = PCI_REGION_SIZE(info->PciInfo, 0) / 1024;

     bar_size = 1 << (info->memsize[RHD_FB_BAR] - 10);

     if (bar_size == 0)
        bar_size = 0x20000;
     if (accessible > bar_size)
        accessible = bar_size;

     dbgprintf("Detected total video RAM=%dK width=%dbit,"
                "accessible=%uK (PCI BAR=%uK)\n",
                info->videoRam, info->RamWidth,
                (unsigned)accessible, (unsigned)bar_size);

     if (info->videoRam > accessible)
        info->videoRam = accessible;

     if (!IS_AVIVO_VARIANT)
        info->MemCntl            = INREG(RADEON_SDRAM_MODE_REG);
        info->BusCntl            = INREG(RADEON_BUS_CNTL);

     info->videoRam  &= ~1023;
     info->FbMapSize  = info->videoRam * 1024;

     info->gartSize      = RADEON_DEFAULT_GART_SIZE;
     info->ringSize      = RADEON_DEFAULT_RING_SIZE;
     info->bufSize       = RADEON_DEFAULT_BUFFER_SIZE;

     info->gartTexSize   = info->gartSize - (info->ringSize + info->bufSize);

     info->pciAperSize   = RADEON_DEFAULT_PCI_APER_SIZE;
     info->CPusecTimeout = RADEON_DEFAULT_CP_TIMEOUT;



     /* if the card is PCI Express reserve the last 32k for the gart table */

 //    if (info->cardType == CARD_PCIE )
 //     /* work out the size of pcie aperture */
 //       info->FbSecureSize = RADEONDRIGetPciAperTableSize(info);
 //    else
 //       info->FbSecureSize = 0;

     return TRUE;
}


static Bool RADEONPreInitChipType(RHDPtr rhdPtr)
{
     u32_t cmd_stat;

     rhdPtr->ChipErrata = 0;

     if ( (rhdPtr->ChipFamily == CHIP_FAMILY_R300) &&
          ((_RHDRegRead(rhdPtr,RADEON_CONFIG_CNTL) & RADEON_CFG_ATI_REV_ID_MASK)
           == RADEON_CFG_ATI_REV_A11))
        rhdPtr->ChipErrata |= CHIP_ERRATA_R300_CG;

     if ( (rhdPtr->ChipFamily == CHIP_FAMILY_RV200) ||
          (rhdPtr->ChipFamily == CHIP_FAMILY_RS200) )
        rhdPtr->ChipErrata |= CHIP_ERRATA_PLL_DUMMYREADS;

     if ( (rhdPtr->ChipFamily == CHIP_FAMILY_RV100) ||
          (rhdPtr->ChipFamily == CHIP_FAMILY_RS100) ||
          (rhdPtr->ChipFamily == CHIP_FAMILY_RS200) )
        rhdPtr->ChipErrata |= CHIP_ERRATA_PLL_DELAY;

     rhdPtr->cardType = CARD_PCI;


     cmd_stat = pciReadLong(rhdPtr->PciTag, PCI_CMD_STAT_REG);

     if (cmd_stat & RADEON_CAP_LIST)
     {
        u32_t cap_ptr, cap_id;

        cap_ptr = pciReadLong(rhdPtr->PciTag, RADEON_CAPABILITIES_PTR_PCI_CONFIG);
        cap_ptr &= RADEON_CAP_PTR_MASK;

        while(cap_ptr != RADEON_CAP_ID_NULL)
        {
           cap_id = pciReadLong(rhdPtr->PciTag, cap_ptr);
           if ((cap_id & 0xff)== RADEON_CAP_ID_AGP) {
              rhdPtr->cardType = CARD_AGP;
              break;
           }
           if ((cap_id & 0xff)== RADEON_CAP_ID_EXP) {
              rhdPtr->cardType = CARD_PCIE;
              break;
           }
           cap_ptr = (cap_id >> 8) & RADEON_CAP_PTR_MASK;
        }
     }

     dbgprintf("%s card detected\n",(rhdPtr->cardType==CARD_PCI) ? "PCI" :
               (rhdPtr->cardType==CARD_PCIE) ? "PCIE" : "AGP");

    /* treat PCIE IGP cards as PCI  */
     if (rhdPtr->cardType == CARD_PCIE && rhdPtr->IsIGP)
         rhdPtr->cardType = CARD_PCI;

     if ( (rhdPtr->ChipFamily == CHIP_FAMILY_RS100) ||
          (rhdPtr->ChipFamily == CHIP_FAMILY_RS200) ||
          (rhdPtr->ChipFamily == CHIP_FAMILY_RS300) ||
          (rhdPtr->ChipFamily == CHIP_FAMILY_RS400) ||
          (rhdPtr->ChipFamily == CHIP_FAMILY_RS480) ||
          (rhdPtr->ChipFamily == CHIP_FAMILY_RS600) ||
          (rhdPtr->ChipFamily == CHIP_FAMILY_RS690) ||
          (rhdPtr->ChipFamily == CHIP_FAMILY_RS740))
        rhdPtr->has_tcl = FALSE;
     else {
        rhdPtr->has_tcl = TRUE;
     }

     rhdPtr->LinearAddr = rhdPtr->memBase[RHD_FB_BAR];

     return TRUE;
}

#if 0
static Bool RADEONSetAgpMode(RADEONInfoPtr info, ScreenPtr pScreen)
{
    unsigned char *RADEONMMIO = info->MMIO;
//    unsigned long mode   = drmAgpGetMode(info->dri->drmFD); /* Default mode */
//    unsigned int  vendor = drmAgpVendorId(info->dri->drmFD);
//    unsigned int  device = drmAgpDeviceId(info->dri->drmFD);
    /* ignore agp 3.0 mode bit from the chip as it's buggy on some cards with
       pcie-agp rialto bridge chip - use the one from bridge which must match */
    uint32_t agp_status = (INREG(RADEON_AGP_STATUS) ); // & RADEON_AGP_MODE_MASK;
    Bool is_v3 = (agp_status & RADEON_AGPv3_MODE);
    unsigned int defaultMode;

    if (is_v3) {
       defaultMode = (agp_status & RADEON_AGPv3_8X_MODE) ? 8 : 4;
    } else {
	if (agp_status & RADEON_AGP_4X_MODE) defaultMode = 4;
	else if (agp_status & RADEON_AGP_2X_MODE) defaultMode = 2;
	else defaultMode = 1;
    }

   // agpMode = defaultMode;

    dbgprintf(pScreen->myNum, from, "Using AGP %dx\n", dbgprintf);

    mode &= ~RADEON_AGP_MODE_MASK;
    if (is_v3) {
	/* only set one mode bit for AGPv3 */
    switch (defaultMode) {
	case 8:          mode |= RADEON_AGPv3_8X_MODE; break;
	case 4: default: mode |= RADEON_AGPv3_4X_MODE;
	}
	/*TODO: need to take care of other bits valid for v3 mode
	 *      currently these bits are not used in all tested cards.
	 */
    } else {
    switch (defaultMode) {
	case 4:          mode |= RADEON_AGP_4X_MODE;
	case 2:          mode |= RADEON_AGP_2X_MODE;
	case 1: default: mode |= RADEON_AGP_1X_MODE;
	}
    }

    /* AGP Fast Writes.
     * TODO: take into account that certain agp modes don't support fast
     * writes at all */
    mode &= ~RADEON_AGP_FW_MODE; /* Disable per default */

    dbgprintf("AGP Mode 0x%08lx\n", mode);

    if (drmAgpEnable(info->dri->drmFD, mode) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "[agp] AGP not enabled\n");
	drmAgpRelease(info->dri->drmFD);
	return FALSE;
    }

    /* Workaround for some hardware bugs */
    if (info->ChipFamily < CHIP_FAMILY_R200)
        OUTREG(RADEON_AGP_CNTL, INREG(RADEON_AGP_CNTL) | 0x000e0000);

				/* Modify the mode if the default mode
				 * is not appropriate for this
				 * particular combination of graphics
				 * card and AGP chipset.
				 */

    return TRUE;
}
#endif

Bool RHDPreInit()
{
    RHDPtr info;

    /* We need access to IO space already */
     if ( !rhdMapMMIO(&rhd) ) {
        dbgprintf("Failed to map MMIO.\n");
        return FALSE;
     };


     if( !RADEONPreInitChipType(&rhd))
        return FALSE;

     if (!RADEONPreInitVRAM(&rhd))
        return FALSE;

     RADEONInitMemoryMap(&rhd);

     if (!rhd.videoRam)
     {
        dbgprintf("No Video RAM detected.\n");
        goto error1;
     }
     dbgprintf("VideoRAM: %d kByte\n",rhd.videoRam);

 //    rhd.FbFreeStart = 0;
     rhd.FbFreeSize = rhd.videoRam << 10;

 // if( !rhdMapFB(&rhd))
//    return FALSE;

//  rhd.FbScanoutStart = 0;
//  rhd.FbScanoutSize  = 8*1024*1024;

  rhd.FbFreeStart    = 10*1024*1024;
  rhd.FbFreeSize     = rhd.FbMapSize - rhd.FbFreeStart - rhd.FbSecureSize;

  rhdInitHeap(&rhd);

     info = &rhd;


   return TRUE;

error1:
  return FALSE;
};



