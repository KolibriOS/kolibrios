
SymTabRec RHDChipsets[] = {
    /* R500 */
    { RHD_RV505, "RV505" },
    { RHD_RV515, "RV515" },
    { RHD_RV516, "RV516" },
    { RHD_R520,  "R520" },
    { RHD_RV530, "RV530" },
    { RHD_RV535, "RV535" },
    { RHD_RV550, "RV550" },
    { RHD_RV560, "RV560" },
    { RHD_RV570, "RV570" },
    { RHD_R580,  "R580" },
    /* R500 Mobility */
    { RHD_M52,   "M52" },
    { RHD_M54,   "M54" },
    { RHD_M56,   "M56" },
    { RHD_M58,   "M58" },
    { RHD_M62,   "M62" },
    { RHD_M64,   "M64" },
    { RHD_M66,   "M66" },
    { RHD_M68,   "M68" },
    { RHD_M71,   "M71" },
    /* R500 integrated */
    { RHD_RS600, "RS600" },
    { RHD_RS690, "RS690" },
    { RHD_RS740, "RS740" },
    /* R600 */
    { RHD_R600,  "R600" },
    { RHD_RV610, "RV610" },
    { RHD_RV630, "RV630" },
    /* R600 Mobility */
    { RHD_M72,   "M72" },
    { RHD_M74,   "M74" },
    { RHD_M76,   "M76" },
    /* RV670 came into existence after RV6x0 and M7x */
    { RHD_RV670, "RV670" },
    { RHD_R680,  "R680"  },
    { RHD_RV620, "RV620" },
    { RHD_RV635, "RV635" },
    { -1,      NULL }
};

# define RHD_DEVICE_MATCH(d, i) { (d),(i) }
# define PCI_ID_LIST PciChipset_t RHDPCIchipsets[]
# define LIST_END { 0,  0}

const PCI_ID_LIST = {
    RHD_DEVICE_MATCH(  0x7100, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x7101, RHD_M58   ), /* Mobility Radeon X1800 XT */
    RHD_DEVICE_MATCH(  0x7102, RHD_M58   ), /* Mobility Radeon X1800 */
    RHD_DEVICE_MATCH(  0x7103, RHD_M58   ), /* Mobility FireGL V7200 */
    RHD_DEVICE_MATCH(  0x7104, RHD_R520  ), /* FireGL V7200 */
    RHD_DEVICE_MATCH(  0x7105, RHD_R520  ), /* FireGL V5300 */
    RHD_DEVICE_MATCH(  0x7106, RHD_M58   ), /* Mobility FireGL V7100 */
    RHD_DEVICE_MATCH(  0x7108, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x7109, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x710A, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x710B, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x710C, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x710E, RHD_R520  ), /* FireGL V7300 */
    RHD_DEVICE_MATCH(  0x710F, RHD_R520  ), /* FireGL V7350 */
    RHD_DEVICE_MATCH(  0x7140, RHD_RV515 ), /* Radeon X1600/X1550 */
    RHD_DEVICE_MATCH(  0x7141, RHD_RV505 ), /* RV505 */
    RHD_DEVICE_MATCH(  0x7142, RHD_RV515 ), /* Radeon X1300/X1550 */
    RHD_DEVICE_MATCH(  0x7143, RHD_RV505 ), /* Radeon X1550 */
    RHD_DEVICE_MATCH(  0x7144, RHD_M54   ), /* M54-GL */
    RHD_DEVICE_MATCH(  0x7145, RHD_M54   ), /* Mobility Radeon X1400 */
    RHD_DEVICE_MATCH(  0x7146, RHD_RV515 ), /* Radeon X1300/X1550 */
    RHD_DEVICE_MATCH(  0x7147, RHD_RV505 ), /* Radeon X1550 64-bit */
    RHD_DEVICE_MATCH(  0x7149, RHD_M52   ), /* Mobility Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714A, RHD_M52   ), /* Mobility Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714B, RHD_M52   ), /* Mobility Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714C, RHD_M52   ), /* Mobility Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714D, RHD_RV515 ), /* Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714E, RHD_RV515 ), /* Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714F, RHD_RV505 ), /* RV505 */
    RHD_DEVICE_MATCH(  0x7151, RHD_RV505 ), /* RV505 */
    RHD_DEVICE_MATCH(  0x7152, RHD_RV515 ), /* FireGL V3300 */
    RHD_DEVICE_MATCH(  0x7153, RHD_RV515 ), /* FireGL V3350 */
    RHD_DEVICE_MATCH(  0x715E, RHD_RV515 ), /* Radeon X1300 */
    RHD_DEVICE_MATCH(  0x715F, RHD_RV505 ), /* Radeon X1550 64-bit */
    RHD_DEVICE_MATCH(  0x7180, RHD_RV516 ), /* Radeon X1300/X1550 */
    RHD_DEVICE_MATCH(  0x7181, RHD_RV516 ), /* Radeon X1600 */
    RHD_DEVICE_MATCH(  0x7183, RHD_RV516 ), /* Radeon X1300/X1550 */
    RHD_DEVICE_MATCH(  0x7186, RHD_M64   ), /* Mobility Radeon X1450 */
    RHD_DEVICE_MATCH(  0x7187, RHD_RV516 ), /* Radeon X1300/X1550 */
    RHD_DEVICE_MATCH(  0x7188, RHD_M64   ), /* Mobility Radeon X2300 */
    RHD_DEVICE_MATCH(  0x718A, RHD_M64   ), /* Mobility Radeon X2300 */
    RHD_DEVICE_MATCH(  0x718B, RHD_M62   ), /* Mobility Radeon X1350 */
    RHD_DEVICE_MATCH(  0x718C, RHD_M62   ), /* Mobility Radeon X1350 */
    RHD_DEVICE_MATCH(  0x718D, RHD_M64   ), /* Mobility Radeon X1450 */
    RHD_DEVICE_MATCH(  0x718F, RHD_RV516 ), /* Radeon X1300 */
    RHD_DEVICE_MATCH(  0x7193, RHD_RV516 ), /* Radeon X1550 */
    RHD_DEVICE_MATCH(  0x7196, RHD_M62   ), /* Mobility Radeon X1350 */
    RHD_DEVICE_MATCH(  0x719B, RHD_RV516 ), /* FireMV 2250 */
    RHD_DEVICE_MATCH(  0x719F, RHD_RV516 ), /* Radeon X1550 64-bit */
    RHD_DEVICE_MATCH(  0x71C0, RHD_RV530 ), /* Radeon X1600 */
    RHD_DEVICE_MATCH(  0x71C1, RHD_RV535 ), /* Radeon X1650 */
    RHD_DEVICE_MATCH(  0x71C2, RHD_RV530 ), /* Radeon X1600 */
    RHD_DEVICE_MATCH(  0x71C3, RHD_RV535 ), /* Radeon X1600 */
    RHD_DEVICE_MATCH(  0x71C4, RHD_M56   ), /* Mobility FireGL V5200 */
    RHD_DEVICE_MATCH(  0x71C5, RHD_M56   ), /* Mobility Radeon X1600 */
    RHD_DEVICE_MATCH(  0x71C6, RHD_RV530 ), /* Radeon X1650 */
    RHD_DEVICE_MATCH(  0x71C7, RHD_RV535 ), /* Radeon X1650 */
    RHD_DEVICE_MATCH(  0x71CD, RHD_RV530 ), /* Radeon X1600 */
    RHD_DEVICE_MATCH(  0x71CE, RHD_RV530 ), /* Radeon X1300 XT/X1600 Pro */
    RHD_DEVICE_MATCH(  0x71D2, RHD_RV530 ), /* FireGL V3400 */
    RHD_DEVICE_MATCH(  0x71D4, RHD_M66   ), /* Mobility FireGL V5250 */
    RHD_DEVICE_MATCH(  0x71D5, RHD_M66   ), /* Mobility Radeon X1700 */
    RHD_DEVICE_MATCH(  0x71D6, RHD_M66   ), /* Mobility Radeon X1700 XT */
    RHD_DEVICE_MATCH(  0x71DA, RHD_RV530 ), /* FireGL V5200 */
    RHD_DEVICE_MATCH(  0x71DE, RHD_M66   ), /* Mobility Radeon X1700 */
    RHD_DEVICE_MATCH(  0x7200, RHD_RV550 ), /*  Radeon X2300HD  */
    RHD_DEVICE_MATCH(  0x7210, RHD_M71   ), /* Mobility Radeon HD 2300 */
    RHD_DEVICE_MATCH(  0x7211, RHD_M71   ), /* Mobility Radeon HD 2300 */
    RHD_DEVICE_MATCH(  0x7240, RHD_R580  ), /* Radeon X1950 */
    RHD_DEVICE_MATCH(  0x7243, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7244, RHD_R580  ), /* Radeon X1950 */
    RHD_DEVICE_MATCH(  0x7245, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7246, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7247, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7248, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7249, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x724A, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x724B, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x724C, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x724D, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x724E, RHD_R580  ), /* AMD Stream Processor */
    RHD_DEVICE_MATCH(  0x724F, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7280, RHD_RV570 ), /* Radeon X1950 */
    RHD_DEVICE_MATCH(  0x7281, RHD_RV560 ), /* RV560 */
    RHD_DEVICE_MATCH(  0x7283, RHD_RV560 ), /* RV560 */
    RHD_DEVICE_MATCH(  0x7284, RHD_M68   ), /* Mobility Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7287, RHD_RV560 ), /* RV560 */
    RHD_DEVICE_MATCH(  0x7288, RHD_RV570 ), /* Radeon X1950 GT */
    RHD_DEVICE_MATCH(  0x7289, RHD_RV570 ), /* RV570 */
    RHD_DEVICE_MATCH(  0x728B, RHD_RV570 ), /* RV570 */
    RHD_DEVICE_MATCH(  0x728C, RHD_RV570 ), /* ATI FireGL V7400  */
    RHD_DEVICE_MATCH(  0x7290, RHD_RV560 ), /* RV560 */
    RHD_DEVICE_MATCH(  0x7291, RHD_RV560 ), /* Radeon X1650 */
    RHD_DEVICE_MATCH(  0x7293, RHD_RV560 ), /* Radeon X1650 */
    RHD_DEVICE_MATCH(  0x7297, RHD_RV560 ), /* RV560 */
    RHD_DEVICE_MATCH(  0x791E, RHD_RS690 ), /* Radeon X1200 */
    RHD_DEVICE_MATCH(  0x791F, RHD_RS690 ), /* Radeon X1200 */
    RHD_DEVICE_MATCH(  0x793F, RHD_RS600 ), /* Radeon Xpress 1200 */
    RHD_DEVICE_MATCH(  0x7941, RHD_RS600 ), /* Radeon Xpress 1200 */
    RHD_DEVICE_MATCH(  0x7942, RHD_RS600 ), /* Radeon Xpress 1200 (M) */
    RHD_DEVICE_MATCH(  0x796C, RHD_RS740 ), /* RS740 */
    RHD_DEVICE_MATCH(  0x796D, RHD_RS740 ), /* RS740M */
    RHD_DEVICE_MATCH(  0x796E, RHD_RS740 ), /* ATI Radeon 2100 RS740 */
    RHD_DEVICE_MATCH(  0x796F, RHD_RS740 ), /* RS740M */
    RHD_DEVICE_MATCH(  0x9400, RHD_R600  ), /* Radeon HD 2900 XT */
    RHD_DEVICE_MATCH(  0x9401, RHD_R600  ), /* Radeon HD 2900 XT */
    RHD_DEVICE_MATCH(  0x9402, RHD_R600  ), /* Radeon HD 2900 XT */
    RHD_DEVICE_MATCH(  0x9403, RHD_R600  ), /* Radeon HD 2900 Pro */
    RHD_DEVICE_MATCH(  0x9405, RHD_R600  ), /* Radeon HD 2900 GT */
    RHD_DEVICE_MATCH(  0x940A, RHD_R600  ), /* FireGL V8650 */
    RHD_DEVICE_MATCH(  0x940B, RHD_R600  ), /* FireGL V8600 */
    RHD_DEVICE_MATCH(  0x940F, RHD_R600  ), /* FireGL V7600 */
    RHD_DEVICE_MATCH(  0x94C0, RHD_RV610 ), /* RV610 */
    RHD_DEVICE_MATCH(  0x94C1, RHD_RV610 ), /* Radeon HD 2400 XT */
    RHD_DEVICE_MATCH(  0x94C3, RHD_RV610 ), /* Radeon HD 2400 Pro */
    RHD_DEVICE_MATCH(  0x94C4, RHD_RV610 ), /* ATI Radeon HD 2400 PRO AGP */
    RHD_DEVICE_MATCH(  0x94C5, RHD_RV610 ), /* FireGL V4000 */
    RHD_DEVICE_MATCH(  0x94C6, RHD_RV610 ), /* RV610 */
    RHD_DEVICE_MATCH(  0x94C7, RHD_RV610 ), /* ATI Radeon HD 2350 */
    RHD_DEVICE_MATCH(  0x94C8, RHD_M74   ), /* Mobility Radeon HD 2400 XT */
    RHD_DEVICE_MATCH(  0x94C9, RHD_M72   ), /* Mobility Radeon HD 2400 */
    RHD_DEVICE_MATCH(  0x94CB, RHD_M72   ), /* ATI RADEON E2400 */
    RHD_DEVICE_MATCH(  0x94CC, RHD_RV610 ), /* ATI Radeon HD 2400 */
    RHD_DEVICE_MATCH(  0x9500, RHD_RV670 ), /* RV670 */
    RHD_DEVICE_MATCH(  0x9501, RHD_RV670 ), /* ATI Radeon HD3870 */
    RHD_DEVICE_MATCH(  0x9505, RHD_RV670 ), /* ATI Radeon HD3850 */
    RHD_DEVICE_MATCH(  0x9507, RHD_RV670 ), /* RV670 */
    RHD_DEVICE_MATCH(  0x950F, RHD_R680  ), /* ATI Radeon HD3870 X2 */
    RHD_DEVICE_MATCH(  0x9511, RHD_RV670 ), /* ATI FireGL V7700 */
    RHD_DEVICE_MATCH(  0x9515, RHD_RV670 ), /* ATI Radeon HD 3850 AGP */
    RHD_DEVICE_MATCH(  0x9580, RHD_RV630 ), /* RV630 */
    RHD_DEVICE_MATCH(  0x9581, RHD_M76   ), /* Mobility Radeon HD 2600 */
    RHD_DEVICE_MATCH(  0x9583, RHD_M76   ), /* Mobility Radeon HD 2600 XT */
    RHD_DEVICE_MATCH(  0x9586, RHD_RV630 ), /* ATI Radeon HD 2600 XT AGP */
    RHD_DEVICE_MATCH(  0x9587, RHD_RV630 ), /* ATI Radeon HD 2600 Pro AGP */
    RHD_DEVICE_MATCH(  0x9588, RHD_RV630 ), /* Radeon HD 2600 XT */
    RHD_DEVICE_MATCH(  0x9589, RHD_RV630 ), /* Radeon HD 2600 Pro */
    RHD_DEVICE_MATCH(  0x958A, RHD_RV630 ), /* Gemini RV630 */
    RHD_DEVICE_MATCH(  0x958B, RHD_M76   ), /* Gemini ATI Mobility Radeon HD 2600 XT */
    RHD_DEVICE_MATCH(  0x958C, RHD_RV630 ), /* FireGL V5600 */
    RHD_DEVICE_MATCH(  0x958D, RHD_RV630 ), /* FireGL V3600 */
    RHD_DEVICE_MATCH(  0x958E, RHD_RV630 ), /* ATI Radeon HD 2600 LE */
    RHD_DEVICE_MATCH(  0x9590, RHD_RV635 ), /* ATI Radeon HD 3600 Series */
    RHD_DEVICE_MATCH(  0x9591, RHD_RV635 ), /* ATI Mobility Radeon HD 3650 */
    RHD_DEVICE_MATCH(  0x9596, RHD_RV635 ), /* ATI Radeon HD 3650 AGP */
    RHD_DEVICE_MATCH(  0x9597, RHD_RV635 ), /* ATI Radeon HD 3600 Series */
    RHD_DEVICE_MATCH(  0x9598, RHD_RV635 ), /* ATI Radeon HD 3670 */
    RHD_DEVICE_MATCH(  0x9599, RHD_RV635 ), /* ATI Radeon HD 3600 Series */
    RHD_DEVICE_MATCH(  0x95C0, RHD_RV620 ), /* ATI Radeon HD 3470 */
    RHD_DEVICE_MATCH(  0x95C2, RHD_M82   ), /* ATI Mobility Radeon HD 3430 (M82) */
    RHD_DEVICE_MATCH(  0x95C4, RHD_M82 ), /* ATI Mobility Radeon HD 3400 Series (M82)  */
    RHD_DEVICE_MATCH(  0x95C5, RHD_RV620 ), /* ATI Radeon HD 3450 */
    RHD_DEVICE_MATCH(  0x95C7, RHD_RV620 ), /* ATI Radeon HD 3430 */
    RHD_DEVICE_MATCH(  0x95CD, RHD_RV620 ), /* ATI FireMV 2450  */
    RHD_DEVICE_MATCH(  0x95CE, RHD_RV620 ), /* ATI FireMV 2260  */
    RHD_DEVICE_MATCH(  0x95CF, RHD_RV620 ), /* ATI FireMV 2260  */
    LIST_END
};

const char *
xf86TokenToString(SymTabPtr table, int token)
{
    int i;

    for (i = 0; table[i].token >= 0 && table[i].token != token; i++){};

    if (table[i].token < 0)
      return NULL;
    else
      return(table[i].name);
}

RHDPtr FindPciDevice()
{
  const PciChipset_t *dev;
  u32 bus, last_bus;

  if( (last_bus = PciApi(1))==-1)
    return 0;

  for(bus=0;bus<=last_bus;bus++)
  {
    u32 devfn;

    for(devfn=0;devfn<256;devfn++)
    {
      u32 id;
      id = PciRead32(bus,devfn, 0);

      if( (CARD16)id != VENDOR_ATI)
        continue;

      if( (dev=PciDevMatch(id>>16,RHDPCIchipsets))!=NULL)
      {
        CARD32 reg2C;
        int i;

        rhd.PciDeviceID = (id>>16);

        rhd.bus = bus;
        rhd.devfn = devfn;
        rhd.PciTag = pciTag(bus,(devfn>>3)&0x1F,devfn&0x7);

        rhd.ChipSet = dev->ChipSet;

        reg2C = PciRead32(bus,devfn, 0x2C);

        rhd.subvendor_id = reg2C & 0xFFFF;;
        rhd.subdevice_id = reg2C >> 16;

        for (i = 0; i < 6; i++)
        {
          CARD32 base;
          Bool validSize;

          base = PciRead32(bus,devfn, PCI_MAP_REG_START + (i << 2));
          if(base)
          {
            if (base & PCI_MAP_IO)
            {
              rhd.ioBase[i] = (CARD32)PCIGETIO(base);
              rhd.memtype[i]   = base & PCI_MAP_IO_ATTR_MASK;
            }
            else
            {
              rhd.memBase[i] = (CARD32)PCIGETMEMORY(base);
              rhd.memtype[i] = base & PCI_MAP_MEMORY_ATTR_MASK;
            }
          }
          rhd.memsize[i] = pciGetBaseSize(bus,devfn, i, TRUE, &validSize);
        }
        rhd.ChipName = (char*)xf86TokenToString(RHDChipsets, rhd.PciDeviceID);

        return &rhd;
      }
    };
  };
  return NULL;
}

const PciChipset_t *PciDevMatch(CARD16 dev,const PciChipset_t *list)
{
  while(list->device)
  {
    if(dev==list->device)
      return list;
    list++;
  }
  return 0;
}


CARD32 pciGetBaseSize(int bus, int devfn, int index, Bool destructive, Bool *min)
{
  int offset;
  CARD32 addr1;
  CARD32 addr2;
  CARD32 mask1;
  CARD32 mask2;
  int bits = 0;

  /*
   * silently ignore bogus index values.  Valid values are 0-6.  0-5 are
   * the 6 base address registers, and 6 is the ROM base address register.
   */
  if (index < 0 || index > 6)
    return 0;

  if (min)
    *min = destructive;

  /* Get the PCI offset */
  if (index == 6)
    offset = PCI_MAP_ROM_REG;
  else
    offset = PCI_MAP_REG_START + (index << 2);

  addr1 = PciRead32(bus, devfn, offset);
  /*
   * Check if this is the second part of a 64 bit address.
   * XXX need to check how endianness affects 64 bit addresses.
   */
  if (index > 0 && index < 6) {
    addr2 = PciRead32(bus, devfn, offset - 4);
    if (PCI_MAP_IS_MEM(addr2) && PCI_MAP_IS64BITMEM(addr2))
      return 0;
  }

  if (destructive) {
     PciWrite32(bus, devfn, offset, 0xffffffff);
     mask1 = PciRead32(bus, devfn, offset);
     PciWrite32(bus, devfn, offset, addr1);
  } else {
    mask1 = addr1;
  }

  /* Check if this is the first part of a 64 bit address. */
  if (index < 5 && PCI_MAP_IS_MEM(mask1) && PCI_MAP_IS64BITMEM(mask1))
  {
    if (PCIGETMEMORY(mask1) == 0)
    {
      addr2 = PciRead32(bus, devfn, offset + 4);
      if (destructive)
      {
        PciWrite32(bus, devfn, offset + 4, 0xffffffff);
        mask2 = PciRead32(bus, devfn, offset + 4);
        PciWrite32(bus, devfn, offset + 4, addr2);
      }
      else
     {
       mask2 = addr2;
     }
     if (mask2 == 0)
       return 0;
     bits = 32;
     while ((mask2 & 1) == 0)
     {
       bits++;
       mask2 >>= 1;
     }
     if (bits > 32)
	  return bits;
    }
  }
  if (index < 6)
    if (PCI_MAP_IS_MEM(mask1))
      mask1 = PCIGETMEMORY(mask1);
    else
      mask1 = PCIGETIO(mask1);
  else
    mask1 = PCIGETROM(mask1);
  if (mask1 == 0)
    return 0;
  bits = 0;
  while ((mask1 & 1) == 0) {
    bits++;
    mask1 >>= 1;
  }
  /* I/O maps can be no larger than 8 bits */

  if ((index < 6) && PCI_MAP_IS_IO(addr1) && bits > 8)
    bits = 8;
  /* ROM maps can be no larger than 24 bits */
  if (index == 6 && bits > 24)
    bits = 24;
  return bits;
}


