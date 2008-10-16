

static Bool
rhdMapMMIO(RHDPtr rhdPtr)
{
  rhdPtr->MMIOMapSize = 1 << rhdPtr->memsize[RHD_MMIO_BAR];
  rhdPtr->MMIOBase = MapIoMem((void*)rhdPtr->memBase[RHD_MMIO_BAR],
                          rhdPtr->MMIOMapSize,PG_SW+PG_NOCACHE);
  if( rhdPtr->MMIOBase==0)
    return 0;

  DBG(dbgprintf("Mapped IO at %x (size %x)\n", rhdPtr->MMIOBase, rhdPtr->MMIOMapSize));
  return 1;
}

#define RADEON_NB_TOM             0x15c

static size_t rhdGetVideoRamSize(RHDPtr rhdPtr)
{
  size_t RamSize, BARSize;

  if (rhdPtr->ChipSet == RHD_RS690)
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
      if (rhdPtr->ChipSet < RHD_R600)
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
  if (rhdPtr->ChipSet < RHD_R600)
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

Bool RHDPreInit()
{
    /* We need access to IO space already */
  if (!rhdMapMMIO(&rhd)) {
    dbgprintf("Failed to map MMIO.\n");
    return FALSE;
  };

  rhd.videoRam = rhdGetVideoRamSize(&rhd);
  if (!rhd.videoRam)
  {
    dbgprintf("No Video RAM detected.\n");
    goto error1;
	}
  dbgprintf("VideoRAM: %d kByte\n",rhd.videoRam);

  rhd.FbFreeStart = 0;
  rhd.FbFreeSize = rhd.videoRam << 10;

  if( !rhdMapFB(&rhd))
    return FALSE;

  rhd.FbScanoutStart = 0;
  rhd.FbScanoutSize  = 8*1024*1024;
  rhd.FbFreeStart    = 10*1024*1024;
  rhd.FbFreeSize     = rhd.FbMapSize - 10*1024*1024;

  rhdInitHeap(&rhd);
  return TRUE;

error1:
  return FALSE;
};



