

u32_t pciGetBaseSize(int bus, int devfn, int index,
                     Bool destructive, Bool *min)
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
