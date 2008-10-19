

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

static void RADEONInitMemoryMap(RHDPtr info)
{
     u32_t       mem_size;
     u32_t       aper_size;

     radeon_read_mc_fb_agp_location(info, LOC_FB | LOC_AGP, &info->mc_fb_location,
                    &info->mc_agp_location, &info->mc_agp_location_hi);

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

     if (IS_AVIVO_VARIANT) {
        if (info->ChipFamily >= CHIP_FAMILY_R600) {
           OUTREG(R600_HDP_NONSURFACE_BASE, (info->mc_fb_location << 16) & 0xff0000);
     }
     else {
        OUTREG(AVIVO_HDP_FB_LOCATION, info->mc_fb_location);
     }
         info->mc_agp_location = 0x003f0000;
     }
     else
         info->mc_agp_location = 0xffffffc0;

      dbgprintf("RADEONInitMemoryMap() : \n");
      dbgprintf("  mem_size         : 0x%08x\n", (unsigned)mem_size);
      dbgprintf("  MC_FB_LOCATION   : 0x%08x\n", (unsigned)info->mc_fb_location);
      dbgprintf("  MC_AGP_LOCATION  : 0x%08x\n", (unsigned)info->mc_agp_location);
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

///    if (!info->HasCRTC2) {
///        info->RamWidth /= 4;
///        info->IsDDR = TRUE;
///    }
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
//    PCI_READ_BYTE(info->PciInfo, &byte, 0xe);
//    if (byte & 0x80) {
//    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
//           "Generation 1 PCI interface in multifunction mode"
//           ", accessible memory limited to one aperture\n");
//    return aper_size;
//    }

    /* Single function older card. We read HDP_APER_CNTL to see how the BIOS
     * have set it up. We don't write this as it's broken on some ASICs but
     * we expect the BIOS to have done the right thing (might be too optimistic...)
     */
//    if (INREG(RADEON_HOST_PATH_CNTL) & RADEON_HDP_APER_CNTL)
//        return aper_size * 2;

    return aper_size;
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

     /* if the card is PCI Express reserve the last 32k for the gart table */

//    if (info->cardType == CARD_PCIE )
      /* work out the size of pcie aperture */
//        info->FbSecureSize = RADEONDRIGetPciAperTableSize(info);
//    else
//    info->FbSecureSize = 0;

    return TRUE;
}

/*

#define RADEON_NB_TOM             0x15c

static size_t rhdGetVideoRamSize(RHDPtr rhdPtr)
{
  size_t RamSize, BARSize;

  if (rhdPtr->ChipFamily == CHIP_FAMILY_RS690)
    RamSize = (_RHDRegRead(rhdPtr, R5XX_CONFIG_MEMSIZE))>>10;
  else
    if (rhdPtr->IsIGP)
    {
      u32_t tom = _RHDRegRead(rhdPtr, RADEON_NB_TOM);
      RamSize = (((tom >> 16) - (tom & 0xffff) + 1) << 6);
      _RHDRegWrite(rhdPtr,R5XX_CONFIG_MEMSIZE, RamSize<<10);
    }
    else
    {
      if (rhdPtr->ChipFamily < CHIP_FAMILY_R600)
      {
        RamSize = (_RHDRegRead(rhdPtr, R5XX_CONFIG_MEMSIZE)) >> 10;
        if(RamSize==0) RamSize=8192;
      }
      else
        RamSize = (_RHDRegRead(rhdPtr, R6XX_CONFIG_MEMSIZE)) >> 10;
    };

  BARSize = 1 << (rhdPtr->memsize[RHD_FB_BAR] - 10);
  if(BARSize==0)
    BARSize = 0x20000;

  if (RamSize > BARSize) {
    DBG(dbgprintf("The detected amount of videoram"
           " exceeds the PCI BAR aperture.\n"));
    DBG(dbgprintf("Using only %dkB of the total "
           "%dkB.\n", (int) BARSize, (int) RamSize));
    return BARSize;
  }
  else return RamSize;
}
*/

#if 0
static Bool
rhdMapFB(RHDPtr rhdPtr)
{
  rhdPtr->FbMapSize = 1 << rhdPtr->memsize[RHD_FB_BAR];
  rhdPtr->PhisBase = rhdPtr->memBase[RHD_FB_BAR];

 // rhdPtr->FbBase = MapIoMem(rhdPtr->PhisBase, rhdPtr->FbMapSize,PG_SW+PG_NOCACHE);

 //  if (!rhdPtr->FbBase)
 //   return FALSE;

    /* These devices have an internal address reference, which some other
     * address registers in there also use. This can be different from the
     * address in the BAR */
  if (rhdPtr->ChipFamily < CHIP_FAMILY_R600)
    rhdPtr->FbIntAddress = _RHDRegRead(rhdPtr, HDP_FB_LOCATION)<< 16;
  else
    rhdPtr->FbIntAddress = _RHDRegRead(rhdPtr, R6XX_CONFIG_FB_BASE);

//    rhdPtr->FbIntAddress = _RHDRegRead(rhdPtr, 0x6110);
//    dbgprintf("rhdPtr->FbIntAddress %x\n",rhdPtr->FbIntAddress);

  if (rhdPtr->FbIntAddress != rhdPtr->PhisBase)
    dbgprintf("PCI FB Address (BAR) is at "
              "0x%08X while card Internal Address is 0x%08X\n",
              (unsigned int) rhdPtr->PhisBase,rhdPtr->FbIntAddress);
 // dbgprintf("Mapped FB at %p (size 0x%08X)\n",rhdPtr->FbBase, rhdPtr->FbMapSize);
  return TRUE;
}
#endif

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

Bool RHDPreInit()
{
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

     rhd.FbFreeStart = 0;
     rhd.FbFreeSize = rhd.videoRam << 10;

 // if( !rhdMapFB(&rhd))
//    return FALSE;

//  rhd.FbScanoutStart = 0;
//  rhd.FbScanoutSize  = 8*1024*1024;

  rhd.FbFreeStart    = 10*1024*1024;
  rhd.FbFreeSize     = rhd.FbMapSize - rhd.FbFreeStart;

  rhdInitHeap(&rhd);
  return TRUE;

error1:
  return FALSE;
};



