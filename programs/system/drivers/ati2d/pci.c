
#include "ati_pciids_gen.h"
#include "radeon_chipset_gen.h"
#include "radeon_chipinfo_gen.h"



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



const RADEONCardInfo *RadeonDevMatch(u16_t dev,const RADEONCardInfo *list)
{
  while(list->pci_device_id)
  {
    if(dev == list->pci_device_id)
      return list;
    list++;
  }
  return 0;
}


RHDPtr FindPciDevice()
{
  const RADEONCardInfo *dev;
  u32_t bus, last_bus;

  if( (last_bus = PciApi(1))==-1)
    return 0;

  for(bus=0;bus<=last_bus;bus++)
  {
    u32_t devfn;

    for(devfn=0;devfn<256;devfn++)
    {
      u32_t id;
      id = PciRead32(bus,devfn, 0);

      if( (u16_t)id != VENDOR_ATI)
        continue;

      rhd.PciDeviceID = (id>>16);

      if( (dev = RadeonDevMatch(rhd.PciDeviceID, RADEONCards))!=NULL)
      {
        u32_t reg2C;
        int i;

        rhd.chipset = (char*)xf86TokenToString(RADEONChipsets, rhd.PciDeviceID);
        if (!rhd.chipset)
        {
           dbgprintf("ChipID 0x%04x is not recognized\n", rhd.PciDeviceID);
           return FALSE;
        }
        dbgprintf("Chipset: \"%s\" (ChipID = 0x%04x)\n",rhd.chipset,rhd.PciDeviceID);

        rhd.bus = bus;
        rhd.devfn = devfn;
        rhd.PciTag = pciTag(bus,(devfn>>3)&0x1F,devfn&0x7);

        rhd.ChipFamily  = dev->chip_family;
        rhd.IsMobility  = dev->mobility;
        rhd.IsIGP       = dev->igp;
        rhd.HasCRTC2    = !dev->nocrtc2;

        reg2C = PciRead32(bus,devfn, 0x2C);

        rhd.subvendor_id = reg2C & 0xFFFF;;
        rhd.subdevice_id = reg2C >> 16;

        if (rhd.ChipFamily >= CHIP_FAMILY_R600)
           dbgprintf("R600 unsupported yet.\nExit\n");


        for (i = 0; i < 6; i++)
        {
          u32_t base;
          Bool validSize;

          base = PciRead32(bus,devfn, PCI_MAP_REG_START + (i << 2));
          if(base)
          {
            if (base & PCI_MAP_IO)
            {
              rhd.ioBase[i] = (u32_t)PCIGETIO(base);
              rhd.memtype[i]   = base & PCI_MAP_IO_ATTR_MASK;
            }
            else
            {
              rhd.memBase[i] = (u32_t)PCIGETMEMORY(base);
              rhd.memtype[i] = base & PCI_MAP_MEMORY_ATTR_MASK;
            }
          }
          rhd.memsize[i] = pciGetBaseSize(bus,devfn, i, TRUE, &validSize);
        }
        return &rhd;
      }
    };
  };
  return NULL;
}



u32_t pciGetBaseSize(int bus, int devfn, int index, Bool destructive, Bool *min)
{
  int offset;
  u32_t addr1;
  u32_t addr2;
  u32_t mask1;
  u32_t mask2;
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


