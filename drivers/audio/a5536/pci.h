
typedef int Bool;

#define TRUE   1
#define FALSE  0

#pragma pack(push, 1)
typedef struct
{
     u16_t device;
     u16_t ChipSet;
}PciChipset_t;
#pragma pack(pop)

#define VENDOR_ATI 0x1002


#define PCI_CLASS_DISPLAY_VGA      0x0300
/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#define PCI_VENDOR_ID                0x00    /* 16 bits */
#define PCI_DEVICE_ID                0x02    /* 16 bits */
#define PCI_COMMAND                  0x04    /* 16 bits */
#define  PCI_COMMAND_IO              0x01    /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY          0x02    /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER          0x04    /* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL         0x08    /* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE      0x10    /* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE     0x20    /* Enable palette snooping */
#define  PCI_COMMAND_PARITY          0x40    /* Enable parity checking */
#define  PCI_COMMAND_WAIT            0x80    /* Enable address/data stepping */
#define  PCI_COMMAND_SERR           0x100    /* Enable SERR */
#define  PCI_COMMAND_FAST_BACK      0x200    /* Enable back-to-back writes */
#define  PCI_COMMAND_INTX_DISABLE   0x400    /* INTx Emulation Disable */

#define PCI_STATUS                  0x06    /* 16 bits */
#define  PCI_STATUS_CAP_LIST        0x10    /* Support Capability List */
#define  PCI_STATUS_66MHZ           0x20    /* Support 66 Mhz PCI 2.1 bus */
#define  PCI_STATUS_UDF             0x40    /* Support User Definable Features [obsolete] */
#define  PCI_STATUS_FAST_BACK       0x80    /* Accept fast-back to back */
#define  PCI_STATUS_PARITY          0x100   /* Detected parity error */
#define  PCI_STATUS_DEVSEL_MASK     0x600   /* DEVSEL timing */
#define  PCI_STATUS_DEVSEL_FAST		0x000
#define  PCI_STATUS_DEVSEL_MEDIUM	0x200
#define  PCI_STATUS_DEVSEL_SLOW		0x400
#define  PCI_STATUS_SIG_TARGET_ABORT	0x800 /* Set on target abort */
#define  PCI_STATUS_REC_TARGET_ABORT	0x1000 /* Master ack of " */
#define  PCI_STATUS_REC_MASTER_ABORT	0x2000 /* Set on master abort */
#define  PCI_STATUS_SIG_SYSTEM_ERROR	0x4000 /* Set when we drive SERR */
#define  PCI_STATUS_DETECTED_PARITY	0x8000 /* Set on parity error */

#define PCI_CLASS_REVISION	0x08	/* High 24 bits are class, low 8 revision */
#define PCI_REVISION_ID		0x08	/* Revision ID */
#define PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE	0x0a	/* Device class */

#define PCI_CACHE_LINE_SIZE	0x0c	/* 8 bits */
#define PCI_LATENCY_TIMER	0x0d	/* 8 bits */
#define PCI_HEADER_TYPE		0x0e	/* 8 bits */
#define  PCI_HEADER_TYPE_NORMAL		0
#define  PCI_HEADER_TYPE_BRIDGE		1
#define  PCI_HEADER_TYPE_CARDBUS	2

#define PCI_BIST            0x0f    /* 8 bits */
#define  PCI_BIST_CODE_MASK	0x0f	/* Return result */
#define  PCI_BIST_START		0x40	/* 1 to start BIST, 2 secs or less */
#define  PCI_BIST_CAPABLE	0x80	/* 1 if BIST capable */

#define PCI_CAPABILITY_LIST     0x34    /* Offset of first capability list entry */
#define PCI_CB_CAPABILITY_LIST  0x14
/* Capability lists */

#define PCI_CAP_LIST_ID     0       /* Capability ID */
#define  PCI_CAP_ID_PM		0x01	/* Power Management */
#define  PCI_CAP_ID_AGP		0x02	/* Accelerated Graphics Port */
#define  PCI_CAP_ID_VPD		0x03	/* Vital Product Data */
#define  PCI_CAP_ID_SLOTID	0x04	/* Slot Identification */
#define  PCI_CAP_ID_MSI		0x05	/* Message Signalled Interrupts */
#define  PCI_CAP_ID_CHSWP	0x06	/* CompactPCI HotSwap */
#define  PCI_CAP_ID_PCIX	0x07	/* PCI-X */
#define  PCI_CAP_ID_HT		0x08	/* HyperTransport */
#define  PCI_CAP_ID_VNDR	0x09	/* Vendor specific capability */
#define  PCI_CAP_ID_SHPC 	0x0C	/* PCI Standard Hot-Plug Controller */
#define  PCI_CAP_ID_EXP 	0x10	/* PCI Express */
#define  PCI_CAP_ID_MSIX	0x11	/* MSI-X */
#define PCI_CAP_LIST_NEXT   1       /* Next capability in the list */
#define PCI_CAP_FLAGS       2       /* Capability defined flags (16 bits) */
#define PCI_CAP_SIZEOF		4


/* AGP registers */

#define PCI_AGP_VERSION          2   /* BCD version number */
#define PCI_AGP_RFU              3   /* Rest of capability flags */
#define PCI_AGP_STATUS           4   /* Status register */
#define  PCI_AGP_STATUS_RQ_MASK	0xff000000	/* Maximum number of requests - 1 */
#define  PCI_AGP_STATUS_SBA     0x0200   /* Sideband addressing supported */
#define  PCI_AGP_STATUS_64BIT   0x0020   /* 64-bit addressing supported */
#define  PCI_AGP_STATUS_FW      0x0010   /* FW transfers supported */
#define  PCI_AGP_STATUS_RATE4   0x0004   /* 4x transfer rate supported */
#define  PCI_AGP_STATUS_RATE2   0x0002   /* 2x transfer rate supported */
#define  PCI_AGP_STATUS_RATE1   0x0001   /* 1x transfer rate supported */
#define PCI_AGP_COMMAND              8   /* Control register */
#define  PCI_AGP_COMMAND_RQ_MASK 0xff000000  /* Master: Maximum number of requests */
#define  PCI_AGP_COMMAND_SBA	0x0200	/* Sideband addressing enabled */
#define  PCI_AGP_COMMAND_AGP	0x0100	/* Allow processing of AGP transactions */
#define  PCI_AGP_COMMAND_64BIT	0x0020 	/* Allow processing of 64-bit addresses */
#define  PCI_AGP_COMMAND_FW     0x0010  /* Force FW transfers */
#define  PCI_AGP_COMMAND_RATE4	0x0004	/* Use 4x rate */
#define  PCI_AGP_COMMAND_RATE2	0x0002	/* Use 2x rate */
#define  PCI_AGP_COMMAND_RATE1	0x0001	/* Use 1x rate */
#define PCI_AGP_SIZEOF		12


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

const PciChipset_t *PciDevMatch(u16_t dev,const PciChipset_t *list);
u32_t pciGetBaseSize(int bus, int devfn, int index, Bool destructive, Bool *min);

#define PCI_ANY_ID (~0)

#define for_each_pci_dev(d) while ((d = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, d))!=-1)
