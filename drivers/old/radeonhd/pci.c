
#include "common.h"
#include "pci.h"


int
pciGetBaseSize(int bus, int devfn, int index, Bool destructive, Bool *min)
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

/*
    int       chipRev;
    int       subsysVendor;
    int       subsysCard;
    int       bus;
    int       devfn;
//    int       func;
    int       class;
    int       subclass;
    int       interface;
    memType   memBase[6];
    memType   ioBase[6];
    int       size[6];
    unsigned char	type[6];
    memType   biosBase;
    int       biosSize;
*/

int pciGetInfo(pciVideoPtr pci)
{
  CARD32 reg0,reg2C;
  int i;

  reg0  = PciRead32(pci->bus,pci->devfn, 0);
  reg2C = PciRead32(pci->bus,pci->devfn, 0x2C);

  pci->vendor  = reg0 & 0xFFFF;
  pci->devtype = reg0 >> 16;

  pci->subsysVendor = reg2C & 0xFFFF;
  pci->subsysCard = reg2C >> 16;

  for (i = 0; i < 6; i++)
  {
    CARD32 base;

    base = PciRead32(pci->bus,pci->devfn, PCI_MAP_REG_START + (i << 2));
    if(base)
    {
      if (base & PCI_MAP_IO)
      {
        pci->ioBase[i] = (memType)PCIGETIO(base);
        pci->type[i] = base & PCI_MAP_IO_ATTR_MASK;
      }
      else
      {
        pci->type[i] = base & PCI_MAP_MEMORY_ATTR_MASK;
        pci->memBase[i] = (memType)PCIGETMEMORY(base);
      }
		}

    pci->size[i] =
      pciGetBaseSize(pci->bus,pci->devfn, i, TRUE, &pci->validSize);
  }

}

