
typedef unsigned long PCITAG;

typedef struct {
    int       vendor;
    int       devtype;
    int       devRev;
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
//    pointer   thisCard;
    Bool      validSize;
//    Bool      validate;
//    CARD32    listed_class;
} pciVideoRec, *pciVideoPtr;


#define PCI_MAP_REG_START   0x10
#define PCI_MAP_REG_END			0x28
#define PCI_MAP_ROM_REG			0x30

#define PCI_MAP_MEMORY			0x00000000
#define PCI_MAP_IO			0x00000001

#define PCI_MAP_MEMORY_TYPE		0x00000007
#define PCI_MAP_IO_TYPE			0x00000003

#define PCI_MAP_MEMORY_TYPE_32BIT	0x00000000
#define PCI_MAP_MEMORY_TYPE_32BIT_1M	0x00000002
#define PCI_MAP_MEMORY_TYPE_64BIT	0x00000004
#define PCI_MAP_MEMORY_TYPE_MASK	0x00000006
#define PCI_MAP_MEMORY_CACHABLE		0x00000008
#define PCI_MAP_MEMORY_ATTR_MASK	0x0000000e
#define PCI_MAP_MEMORY_ADDRESS_MASK	0xfffffff0

#define PCI_MAP_IO_ATTR_MASK		0x00000003

#define PCI_MAP_IS_IO(b)  ((b) & PCI_MAP_IO)
#define PCI_MAP_IS_MEM(b)	(!PCI_MAP_IS_IO(b))

#define PCI_MAP_IS64BITMEM(b)	\
	(((b) & PCI_MAP_MEMORY_TYPE_MASK) == PCI_MAP_MEMORY_TYPE_64BIT)

#define PCIGETMEMORY(b)   ((b) & PCI_MAP_MEMORY_ADDRESS_MASK)
#define PCIGETMEMORY64HIGH(b)	(*((CARD32*)&b + 1))
#define PCIGETMEMORY64(b)	\
	(PCIGETMEMORY(b) | ((CARD64)PCIGETMEMORY64HIGH(b) << 32))

#define PCI_MAP_IO_ADDRESS_MASK		0xfffffffc

#define PCIGETIO(b)		((b) & PCI_MAP_IO_ADDRESS_MASK)

#define PCI_MAP_ROM_DECODE_ENABLE	0x00000001
#define PCI_MAP_ROM_ADDRESS_MASK	0xfffff800

#define PCIGETROM(b)		((b) & PCI_MAP_ROM_ADDRESS_MASK)

int pciGetBaseSize(int bus, int devfn, int index, Bool destructive, Bool *min);
int pciGetInfo(pciVideoPtr pci);
