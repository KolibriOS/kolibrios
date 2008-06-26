

#pragma pack(push, 1)
typedef struct
{
  CARD16 device;
  CARD16 ChipSet;
}PciChipset_t;
#pragma pack(pop)

#define VENDOR_ATI 0x1002


#define PCI_MAP_REG_START             0x10
#define PCI_MAP_REG_END               0x28
#define PCI_MAP_ROM_REG               0x30

#define PCI_MAP_MEMORY                0x00000000
#define PCI_MAP_IO                    0x00000001

#define PCI_MAP_MEMORY_TYPE           0x00000007
#define PCI_MAP_IO_TYPE               0x00000003

#define PCI_MAP_MEMORY_TYPE_32BIT     0x00000000
#define PCI_MAP_MEMORY_TYPE_32BIT_1M	0x00000002
#define PCI_MAP_MEMORY_TYPE_64BIT     0x00000004
#define PCI_MAP_MEMORY_TYPE_MASK      0x00000006
#define PCI_MAP_MEMORY_CACHABLE       0x00000008
#define PCI_MAP_MEMORY_ATTR_MASK      0x0000000e
#define PCI_MAP_MEMORY_ADDRESS_MASK   0xfffffff0

#define PCI_MAP_IO_ATTR_MASK          0x00000003

#define PCI_MAP_IS_IO(b)  ((b) & PCI_MAP_IO)
#define PCI_MAP_IS_MEM(b)	(!PCI_MAP_IS_IO(b))

#define PCI_MAP_IS64BITMEM(b)	\
	(((b) & PCI_MAP_MEMORY_TYPE_MASK) == PCI_MAP_MEMORY_TYPE_64BIT)

#define PCIGETMEMORY(b)   ((b) & PCI_MAP_MEMORY_ADDRESS_MASK)
#define PCIGETMEMORY64HIGH(b)	(*((CARD32*)&b + 1))
#define PCIGETMEMORY64(b)	\
	(PCIGETMEMORY(b) | ((CARD64)PCIGETMEMORY64HIGH(b) << 32))

#define PCI_MAP_IO_ADDRESS_MASK       0xfffffffc

#define PCIGETIO(b)		((b) & PCI_MAP_IO_ADDRESS_MASK)

#define PCI_MAP_ROM_DECODE_ENABLE     0x00000001
#define PCI_MAP_ROM_ADDRESS_MASK      0xfffff800

#define PCIGETROM(b)		((b) & PCI_MAP_ROM_ADDRESS_MASK)


#ifndef PCI_DOM_MASK
# define PCI_DOM_MASK 0x0ffu
#endif
#define PCI_DOMBUS_MASK (((PCI_DOM_MASK) << 8) | 0x0ffu)

#define PCI_MAKE_TAG(b,d,f)  ((((b) & (PCI_DOMBUS_MASK)) << 16) | \
			      (((d) & 0x00001fu) << 11) | \
			      (((f) & 0x000007u) << 8))

#define PCI_BUS_FROM_TAG(tag)  (((tag) >> 16) & (PCI_DOMBUS_MASK))
#define PCI_DEV_FROM_TAG(tag)  (((tag) & 0x0000f800u) >> 11)
#define PCI_FUNC_FROM_TAG(tag) (((tag) & 0x00000700u) >> 8)
#define PCI_DFN_FROM_TAG(tag)  (((tag) & 0x0000ff00u) >> 8)


typedef unsigned int PCITAG;

extern inline PCITAG
pciTag(int busnum, int devnum, int funcnum)
{
	return(PCI_MAKE_TAG(busnum,devnum,funcnum));
}

const PciChipset_t *PciDevMatch(CARD16 dev,const PciChipset_t *list);
CARD32 pciGetBaseSize(int bus, int devfn, int index, Bool destructive, Bool *min);
