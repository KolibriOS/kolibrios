
#include <types.h>
#include <link.h>

#ifndef __PCI_H__
#define __PCI_H__

#define PCI_ANY_ID (~0)


#define PCI_CLASS_NOT_DEFINED           0x0000
#define PCI_CLASS_NOT_DEFINED_VGA       0x0001

#define PCI_BASE_CLASS_STORAGE          0x01
#define PCI_CLASS_STORAGE_SCSI          0x0100
#define PCI_CLASS_STORAGE_IDE           0x0101
#define PCI_CLASS_STORAGE_FLOPPY        0x0102
#define PCI_CLASS_STORAGE_IPI           0x0103
#define PCI_CLASS_STORAGE_RAID          0x0104
#define PCI_CLASS_STORAGE_SATA          0x0106
#define PCI_CLASS_STORAGE_SATA_AHCI     0x010601
#define PCI_CLASS_STORAGE_SAS           0x0107
#define PCI_CLASS_STORAGE_OTHER         0x0180

#define PCI_BASE_CLASS_NETWORK          0x02
#define PCI_CLASS_NETWORK_ETHERNET      0x0200
#define PCI_CLASS_NETWORK_TOKEN_RING    0x0201
#define PCI_CLASS_NETWORK_FDDI          0x0202
#define PCI_CLASS_NETWORK_ATM           0x0203
#define PCI_CLASS_NETWORK_OTHER         0x0280

#define PCI_BASE_CLASS_DISPLAY          0x03
#define PCI_CLASS_DISPLAY_VGA           0x0300
#define PCI_CLASS_DISPLAY_XGA           0x0301
#define PCI_CLASS_DISPLAY_3D            0x0302
#define PCI_CLASS_DISPLAY_OTHER         0x0380

#define PCI_BASE_CLASS_MULTIMEDIA       0x04
#define PCI_CLASS_MULTIMEDIA_VIDEO      0x0400
#define PCI_CLASS_MULTIMEDIA_AUDIO      0x0401
#define PCI_CLASS_MULTIMEDIA_PHONE      0x0402
#define PCI_CLASS_MULTIMEDIA_OTHER      0x0480

#define PCI_BASE_CLASS_MEMORY           0x05
#define PCI_CLASS_MEMORY_RAM            0x0500
#define PCI_CLASS_MEMORY_FLASH          0x0501
#define PCI_CLASS_MEMORY_OTHER          0x0580

#define PCI_BASE_CLASS_BRIDGE           0x06
#define PCI_CLASS_BRIDGE_HOST           0x0600
#define PCI_CLASS_BRIDGE_ISA            0x0601
#define PCI_CLASS_BRIDGE_EISA           0x0602
#define PCI_CLASS_BRIDGE_MC             0x0603
#define PCI_CLASS_BRIDGE_PCI            0x0604
#define PCI_CLASS_BRIDGE_PCMCIA         0x0605
#define PCI_CLASS_BRIDGE_NUBUS          0x0606
#define PCI_CLASS_BRIDGE_CARDBUS        0x0607
#define PCI_CLASS_BRIDGE_RACEWAY        0x0608
#define PCI_CLASS_BRIDGE_OTHER          0x0680

#define PCI_BASE_CLASS_COMMUNICATION    0x07
#define PCI_CLASS_COMMUNICATION_SERIAL  0x0700
#define PCI_CLASS_COMMUNICATION_PARALLEL 0x0701
#define PCI_CLASS_COMMUNICATION_MULTISERIAL 0x0702
#define PCI_CLASS_COMMUNICATION_MODEM   0x0703
#define PCI_CLASS_COMMUNICATION_OTHER   0x0780

#define PCI_BASE_CLASS_SYSTEM           0x08
#define PCI_CLASS_SYSTEM_PIC            0x0800
#define PCI_CLASS_SYSTEM_PIC_IOAPIC     0x080010
#define PCI_CLASS_SYSTEM_PIC_IOXAPIC    0x080020
#define PCI_CLASS_SYSTEM_DMA            0x0801
#define PCI_CLASS_SYSTEM_TIMER          0x0802
#define PCI_CLASS_SYSTEM_RTC            0x0803
#define PCI_CLASS_SYSTEM_PCI_HOTPLUG    0x0804
#define PCI_CLASS_SYSTEM_SDHCI          0x0805
#define PCI_CLASS_SYSTEM_OTHER          0x0880

#define PCI_BASE_CLASS_INPUT            0x09
#define PCI_CLASS_INPUT_KEYBOARD        0x0900
#define PCI_CLASS_INPUT_PEN             0x0901
#define PCI_CLASS_INPUT_MOUSE           0x0902
#define PCI_CLASS_INPUT_SCANNER         0x0903
#define PCI_CLASS_INPUT_GAMEPORT        0x0904
#define PCI_CLASS_INPUT_OTHER           0x0980

#define PCI_BASE_CLASS_DOCKING          0x0a
#define PCI_CLASS_DOCKING_GENERIC       0x0a00
#define PCI_CLASS_DOCKING_OTHER         0x0a80

#define PCI_BASE_CLASS_PROCESSOR        0x0b
#define PCI_CLASS_PROCESSOR_386         0x0b00
#define PCI_CLASS_PROCESSOR_486         0x0b01
#define PCI_CLASS_PROCESSOR_PENTIUM     0x0b02
#define PCI_CLASS_PROCESSOR_ALPHA       0x0b10
#define PCI_CLASS_PROCESSOR_POWERPC     0x0b20
#define PCI_CLASS_PROCESSOR_MIPS        0x0b30
#define PCI_CLASS_PROCESSOR_CO          0x0b40

#define PCI_BASE_CLASS_SERIAL           0x0c
#define PCI_CLASS_SERIAL_FIREWIRE       0x0c00
#define PCI_CLASS_SERIAL_FIREWIRE_OHCI  0x0c0010
#define PCI_CLASS_SERIAL_ACCESS         0x0c01
#define PCI_CLASS_SERIAL_SSA            0x0c02
#define PCI_CLASS_SERIAL_USB            0x0c03
#define PCI_CLASS_SERIAL_USB_UHCI       0x0c0300
#define PCI_CLASS_SERIAL_USB_OHCI       0x0c0310
#define PCI_CLASS_SERIAL_USB_EHCI       0x0c0320
#define PCI_CLASS_SERIAL_FIBER          0x0c04
#define PCI_CLASS_SERIAL_SMBUS          0x0c05

#define PCI_BASE_CLASS_WIRELESS                 0x0d
#define PCI_CLASS_WIRELESS_RF_CONTROLLER        0x0d10
#define PCI_CLASS_WIRELESS_WHCI                 0x0d1010

#define PCI_BASE_CLASS_INTELLIGENT      0x0e
#define PCI_CLASS_INTELLIGENT_I2O       0x0e00

#define PCI_BASE_CLASS_SATELLITE        0x0f
#define PCI_CLASS_SATELLITE_TV          0x0f00
#define PCI_CLASS_SATELLITE_AUDIO       0x0f01
#define PCI_CLASS_SATELLITE_VOICE       0x0f03
#define PCI_CLASS_SATELLITE_DATA        0x0f04

#define PCI_BASE_CLASS_CRYPT            0x10
#define PCI_CLASS_CRYPT_NETWORK         0x1000
#define PCI_CLASS_CRYPT_ENTERTAINMENT   0x1001
#define PCI_CLASS_CRYPT_OTHER           0x1080

#define PCI_BASE_CLASS_SIGNAL_PROCESSING 0x11
#define PCI_CLASS_SP_DPIO               0x1100
#define PCI_CLASS_SP_OTHER              0x1180

#define PCI_CLASS_OTHERS                0xff


/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#define PCI_VENDOR_ID                   0x000    /* 16 bits */
#define PCI_DEVICE_ID                   0x002    /* 16 bits */
#define PCI_COMMAND                     0x004    /* 16 bits */
#define  PCI_COMMAND_IO                 0x001    /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY             0x002    /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER             0x004    /* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL            0x008    /* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE         0x010    /* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE        0x020    /* Enable palette snooping */
#define  PCI_COMMAND_PARITY             0x040    /* Enable parity checking */
#define  PCI_COMMAND_WAIT               0x080    /* Enable address/data stepping */
#define  PCI_COMMAND_SERR               0x100    /* Enable SERR */
#define  PCI_COMMAND_FAST_BACK          0x200    /* Enable back-to-back writes */
#define  PCI_COMMAND_INTX_DISABLE       0x400    /* INTx Emulation Disable */

#define PCI_STATUS                      0x006    /* 16 bits */
#define  PCI_STATUS_CAP_LIST            0x010    /* Support Capability List */
#define  PCI_STATUS_66MHZ               0x020    /* Support 66 Mhz PCI 2.1 bus */
#define  PCI_STATUS_UDF                 0x040    /* Support User Definable Features [obsolete] */
#define  PCI_STATUS_FAST_BACK           0x080    /* Accept fast-back to back */
#define  PCI_STATUS_PARITY              0x100    /* Detected parity error */
#define  PCI_STATUS_DEVSEL_MASK         0x600    /* DEVSEL timing */
#define  PCI_STATUS_DEVSEL_FAST         0x000
#define  PCI_STATUS_DEVSEL_MEDIUM       0x200
#define  PCI_STATUS_DEVSEL_SLOW         0x400
#define  PCI_STATUS_SIG_TARGET_ABORT    0x800    /* Set on target abort */
#define  PCI_STATUS_REC_TARGET_ABORT    0x1000   /* Master ack of " */
#define  PCI_STATUS_REC_MASTER_ABORT    0x2000   /* Set on master abort */
#define  PCI_STATUS_SIG_SYSTEM_ERROR    0x4000   /* Set when we drive SERR */
#define  PCI_STATUS_DETECTED_PARITY     0x8000   /* Set on parity error */

#define PCI_CLASS_REVISION               0x08    /* High 24 bits are class, low 8 revision */
#define PCI_REVISION_ID                  0x08    /* Revision ID */
#define PCI_CLASS_PROG                   0x09    /* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE                 0x0a    /* Device class */

#define PCI_CACHE_LINE_SIZE              0x0c    /* 8 bits */
#define PCI_LATENCY_TIMER                0x0d    /* 8 bits */
#define PCI_HEADER_TYPE                  0x0e    /* 8 bits */
#define  PCI_HEADER_TYPE_NORMAL             0
#define  PCI_HEADER_TYPE_BRIDGE             1
#define  PCI_HEADER_TYPE_CARDBUS            2

#define PCI_BIST                         0x0f    /* 8 bits */
#define  PCI_BIST_CODE_MASK              0x0f    /* Return result */
#define  PCI_BIST_START                  0x40    /* 1 to start BIST, 2 secs or less */
#define  PCI_BIST_CAPABLE                0x80    /* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#define  PCI_BASE_ADDRESS_0             0x10    /* 32 bits */
#define  PCI_BASE_ADDRESS_1             0x14    /* 32 bits [htype 0,1 only] */
#define  PCI_BASE_ADDRESS_2             0x18    /* 32 bits [htype 0 only] */
#define  PCI_BASE_ADDRESS_3             0x1c    /* 32 bits */
#define  PCI_BASE_ADDRESS_4             0x20    /* 32 bits */
#define  PCI_BASE_ADDRESS_5             0x24    /* 32 bits */
#define  PCI_BASE_ADDRESS_SPACE         0x01    /* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO      0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY  0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_MASK 0x06
#define  PCI_BASE_ADDRESS_MEM_TYPE_32   0x00    /* 32 bit address */
#define  PCI_BASE_ADDRESS_MEM_TYPE_1M   0x02    /* Below 1M [obsolete] */
#define  PCI_BASE_ADDRESS_MEM_TYPE_64   0x04    /* 64 bit address */
#define  PCI_BASE_ADDRESS_MEM_PREFETCH  0x08    /* prefetchable? */
#define  PCI_BASE_ADDRESS_MEM_MASK      (~0x0fUL)
#define  PCI_BASE_ADDRESS_IO_MASK       (~0x03UL)
/* bit 1 is reserved if address_space = 1 */

#define PCI_ROM_ADDRESS1                0x38    /* Same as PCI_ROM_ADDRESS, but for htype 1 */

/* Header type 0 (normal devices) */
#define PCI_CARDBUS_CIS                  0x28
#define PCI_SUBSYSTEM_VENDOR_ID          0x2c
#define PCI_SUBSYSTEM_ID                 0x2e
#define PCI_ROM_ADDRESS                  0x30    /* Bits 31..11 are address, 10..1 reserved */
#define  PCI_ROM_ADDRESS_ENABLE          0x01
#define PCI_ROM_ADDRESS_MASK             (~0x7ffUL)

#define PCI_INTERRUPT_LINE               0x3c    /* 8 bits */
#define PCI_INTERRUPT_PIN                0x3d    /* 8 bits */


#define PCI_CB_SUBSYSTEM_VENDOR_ID       0x40
#define PCI_CB_SUBSYSTEM_ID              0x42

#define PCI_CAPABILITY_LIST              0x34    /* Offset of first capability list entry */
#define PCI_CB_CAPABILITY_LIST           0x14
/* Capability lists */

#define PCI_CAP_LIST_ID                  0       /* Capability ID */
#define  PCI_CAP_ID_PM                   0x01    /* Power Management */
#define  PCI_CAP_ID_AGP                  0x02    /* Accelerated Graphics Port */
#define  PCI_CAP_ID_VPD                  0x03    /* Vital Product Data */
#define  PCI_CAP_ID_SLOTID               0x04    /* Slot Identification */
#define  PCI_CAP_ID_MSI                  0x05    /* Message Signalled Interrupts */
#define  PCI_CAP_ID_CHSWP                0x06    /* CompactPCI HotSwap */
#define  PCI_CAP_ID_PCIX                 0x07    /* PCI-X */
#define  PCI_CAP_ID_HT                   0x08    /* HyperTransport */
#define  PCI_CAP_ID_VNDR                 0x09    /* Vendor specific capability */
#define  PCI_CAP_ID_SHPC                 0x0C    /* PCI Standard Hot-Plug Controller */
#define  PCI_CAP_ID_EXP                  0x10    /* PCI Express */
#define  PCI_CAP_ID_MSIX                 0x11    /* MSI-X */
#define PCI_CAP_LIST_NEXT                1       /* Next capability in the list */
#define PCI_CAP_FLAGS                    2       /* Capability defined flags (16 bits) */
#define PCI_CAP_SIZEOF                   4


/* AGP registers */

#define PCI_AGP_VERSION                     2   /* BCD version number */
#define PCI_AGP_RFU                         3   /* Rest of capability flags */
#define PCI_AGP_STATUS                      4   /* Status register */
#define  PCI_AGP_STATUS_RQ_MASK        0xff000000  /* Maximum number of requests - 1 */
#define  PCI_AGP_STATUS_SBA            0x0200   /* Sideband addressing supported */
#define  PCI_AGP_STATUS_64BIT          0x0020   /* 64-bit addressing supported */
#define  PCI_AGP_STATUS_FW             0x0010   /* FW transfers supported */
#define  PCI_AGP_STATUS_RATE4          0x0004   /* 4x transfer rate supported */
#define  PCI_AGP_STATUS_RATE2          0x0002   /* 2x transfer rate supported */
#define  PCI_AGP_STATUS_RATE1          0x0001   /* 1x transfer rate supported */
#define PCI_AGP_COMMAND                     8   /* Control register */
#define  PCI_AGP_COMMAND_RQ_MASK    0xff000000  /* Master: Maximum number of requests */
#define  PCI_AGP_COMMAND_SBA           0x0200   /* Sideband addressing enabled */
#define  PCI_AGP_COMMAND_AGP           0x0100   /* Allow processing of AGP transactions */
#define  PCI_AGP_COMMAND_64BIT         0x0020   /* Allow processing of 64-bit addresses */
#define  PCI_AGP_COMMAND_FW            0x0010   /* Force FW transfers */
#define  PCI_AGP_COMMAND_RATE4         0x0004   /* Use 4x rate */
#define  PCI_AGP_COMMAND_RATE2         0x0002   /* Use 2x rate */
#define  PCI_AGP_COMMAND_RATE1         0x0001   /* Use 1x rate */
#define PCI_AGP_SIZEOF                     12


#define PCI_MAP_REG_START                   0x10
#define PCI_MAP_REG_END                     0x28
#define PCI_MAP_ROM_REG                     0x30

#define PCI_MAP_MEMORY                0x00000000
#define PCI_MAP_IO                    0x00000001

#define PCI_MAP_MEMORY_TYPE           0x00000007
#define PCI_MAP_IO_TYPE               0x00000003

#define PCI_MAP_MEMORY_TYPE_32BIT     0x00000000
#define PCI_MAP_MEMORY_TYPE_32BIT_1M  0x00000002
#define PCI_MAP_MEMORY_TYPE_64BIT     0x00000004
#define PCI_MAP_MEMORY_TYPE_MASK      0x00000006
#define PCI_MAP_MEMORY_CACHABLE       0x00000008
#define PCI_MAP_MEMORY_ATTR_MASK      0x0000000e
#define PCI_MAP_MEMORY_ADDRESS_MASK   0xfffffff0

#define PCI_MAP_IO_ATTR_MASK          0x00000003



#define PCI_MAP_IS_IO(b)  ((b) & PCI_MAP_IO)
#define PCI_MAP_IS_MEM(b)   (!PCI_MAP_IS_IO(b))

#define PCI_MAP_IS64BITMEM(b)   \
    (((b) & PCI_MAP_MEMORY_TYPE_MASK) == PCI_MAP_MEMORY_TYPE_64BIT)

#define PCIGETMEMORY(b)   ((b) & PCI_MAP_MEMORY_ADDRESS_MASK)
#define PCIGETMEMORY64HIGH(b)   (*((CARD32*)&b + 1))
#define PCIGETMEMORY64(b)   \
    (PCIGETMEMORY(b) | ((CARD64)PCIGETMEMORY64HIGH(b) << 32))

#define PCI_MAP_IO_ADDRESS_MASK       0xfffffffc

#define PCIGETIO(b)     ((b) & PCI_MAP_IO_ADDRESS_MASK)

#define PCI_MAP_ROM_DECODE_ENABLE     0x00000001
#define PCI_MAP_ROM_ADDRESS_MASK      0xfffff800

#define PCIGETROM(b)        ((b) & PCI_MAP_ROM_ADDRESS_MASK)


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

#define PCI_DEVFN(slot, func)  ((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_SLOT(devfn)        (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)        ((devfn) & 0x07)



typedef unsigned int PCITAG;

extern inline PCITAG
pciTag(int busnum, int devnum, int funcnum)
{
    return(PCI_MAKE_TAG(busnum,devnum,funcnum));
}


struct resource
{
         resource_size_t start;
         resource_size_t end;
//         const char *name;
         unsigned long flags;
//         struct resource *parent, *sibling, *child;
};

/*
 * IO resources have these defined flags.
 */
#define IORESOURCE_BITS         0x000000ff      /* Bus-specific bits */

#define IORESOURCE_IO           0x00000100      /* Resource type */
#define IORESOURCE_MEM          0x00000200
#define IORESOURCE_IRQ          0x00000400
#define IORESOURCE_DMA          0x00000800

#define IORESOURCE_PREFETCH     0x00001000      /* No side effects */
#define IORESOURCE_READONLY     0x00002000
#define IORESOURCE_CACHEABLE    0x00004000
#define IORESOURCE_RANGELENGTH  0x00008000
#define IORESOURCE_SHADOWABLE   0x00010000
#define IORESOURCE_BUS_HAS_VGA  0x00080000

#define IORESOURCE_DISABLED     0x10000000
#define IORESOURCE_UNSET        0x20000000
#define IORESOURCE_AUTO         0x40000000
#define IORESOURCE_BUSY         0x80000000      /* Driver has marked this resource busy */

/* ISA PnP IRQ specific bits (IORESOURCE_BITS) */
#define IORESOURCE_IRQ_HIGHEDGE         (1<<0)
#define IORESOURCE_IRQ_LOWEDGE          (1<<1)
#define IORESOURCE_IRQ_HIGHLEVEL        (1<<2)
#define IORESOURCE_IRQ_LOWLEVEL         (1<<3)
#define IORESOURCE_IRQ_SHAREABLE        (1<<4)

/* ISA PnP DMA specific bits (IORESOURCE_BITS) */
#define IORESOURCE_DMA_TYPE_MASK        (3<<0)
#define IORESOURCE_DMA_8BIT             (0<<0)
#define IORESOURCE_DMA_8AND16BIT        (1<<0)
#define IORESOURCE_DMA_16BIT            (2<<0)

#define IORESOURCE_DMA_MASTER           (1<<2)
#define IORESOURCE_DMA_BYTE             (1<<3)
#define IORESOURCE_DMA_WORD             (1<<4)

#define IORESOURCE_DMA_SPEED_MASK       (3<<6)
#define IORESOURCE_DMA_COMPATIBLE       (0<<6)
#define IORESOURCE_DMA_TYPEA            (1<<6)
#define IORESOURCE_DMA_TYPEB            (2<<6)
#define IORESOURCE_DMA_TYPEF            (3<<6)

/* ISA PnP memory I/O specific bits (IORESOURCE_BITS) */
#define IORESOURCE_MEM_WRITEABLE        (1<<0)  /* dup: IORESOURCE_READONLY */
#define IORESOURCE_MEM_CACHEABLE        (1<<1)  /* dup: IORESOURCE_CACHEABLE */
#define IORESOURCE_MEM_RANGELENGTH      (1<<2)  /* dup: IORESOURCE_RANGELENGTH */
#define IORESOURCE_MEM_TYPE_MASK        (3<<3)
#define IORESOURCE_MEM_8BIT             (0<<3)
#define IORESOURCE_MEM_16BIT            (1<<3)
#define IORESOURCE_MEM_8AND16BIT        (2<<3)
#define IORESOURCE_MEM_32BIT            (3<<3)
#define IORESOURCE_MEM_SHADOWABLE       (1<<5)  /* dup: IORESOURCE_SHADOWABLE */
#define IORESOURCE_MEM_EXPANSIONROM     (1<<6)

/* PCI ROM control bits (IORESOURCE_BITS) */
#define IORESOURCE_ROM_ENABLE           (1<<0)  /* ROM is enabled, same as PCI_ROM_ADDRESS_ENABLE */
#define IORESOURCE_ROM_SHADOW           (1<<1)  /* ROM is copy at C000:0 */
#define IORESOURCE_ROM_COPY             (1<<2)  /* ROM is alloc'd copy, resource field overlaid */
#define IORESOURCE_ROM_BIOS_COPY        (1<<3)  /* ROM is BIOS copy, resource field overlaid */

/* PCI control bits.  Shares IORESOURCE_BITS with above PCI ROM.  */
#define IORESOURCE_PCI_FIXED            (1<<4)  /* Do not move resource */


/*
 *  For PCI devices, the region numbers are assigned this way:
 *
 *      0-5     standard PCI regions
 *      6       expansion ROM
 *      7-10    bridges: address space assigned to buses behind the bridge
 */

#define PCI_ROM_RESOURCE        6
#define PCI_BRIDGE_RESOURCES    7
#define PCI_NUM_RESOURCES       11

#ifndef PCI_BUS_NUM_RESOURCES
#define PCI_BUS_NUM_RESOURCES   8
#endif

#define DEVICE_COUNT_RESOURCE   12

/*
 * The pci_dev structure is used to describe PCI devices.
 */
struct pci_dev {
//    struct list_head bus_list;  /* node in per-bus list */
//    struct pci_bus  *bus;       /* bus this device is on */
//    struct pci_bus  *subordinate;   /* bus this device bridges to */

//    void        *sysdata;       /* hook for sys-specific extension */
//    struct proc_dir_entry *procent; /* device entry in /proc/bus/pci */
//    struct pci_slot *slot;      /* Physical slot this device is in */
    u32_t        bus;
    u32_t        devfn;          /* encoded device & function index */
    u16_t        vendor;
    u16_t        device;
    u16_t        subsystem_vendor;
    u16_t        subsystem_device;
    u32_t        class;         /* 3 bytes: (base,sub,prog-if) */
    uint8_t      revision;      /* PCI revision, low byte of class word */
    uint8_t      hdr_type;      /* PCI header type (`multi' flag masked out) */
    uint8_t      pcie_type;     /* PCI-E device/port type */
    uint8_t      rom_base_reg;   /* which config register controls the ROM */
    uint8_t      pin;           /* which interrupt pin this device uses */

 //   struct pci_driver *driver;  /* which driver has allocated this device */
    uint64_t     dma_mask;   /* Mask of the bits of bus address this
                       device implements.  Normally this is
                       0xffffffff.  You only need to change
                       this if your device has broken DMA
                       or supports 64-bit transfers.  */

 //   struct device_dma_parameters dma_parms;

//    pci_power_t     current_state;  /* Current operating state. In ACPI-speak,
 //                      this is D0-D3, D0 being fully functional,
//                       and D3 being off. */
//    int     pm_cap;     /* PM capability offset in the
//                       configuration space */
    unsigned int    pme_support:5;  /* Bitmask of states from which PME#
                       can be generated */
    unsigned int    d1_support:1;   /* Low power state D1 is supported */
    unsigned int    d2_support:1;   /* Low power state D2 is supported */
    unsigned int    no_d1d2:1;  /* Only allow D0 and D3 */

//    pci_channel_state_t error_state;    /* current connectivity state */
//    struct  device  dev;        /* Generic device interface */

//    int     cfg_size;   /* Size of configuration space */

    /*
     * Instead of touching interrupt line and base address registers
     * directly, use the values stored here. They might be different!
     */
    unsigned int    irq;
    struct resource resource[DEVICE_COUNT_RESOURCE]; /* I/O and memory regions + expansion ROMs */

    /* These fields are used by common fixups */
    unsigned int    transparent:1;  /* Transparent PCI bridge */
    unsigned int    multifunction:1;/* Part of multi-function device */
    /* keep track of device state */
    unsigned int    is_added:1;
    unsigned int    is_busmaster:1; /* device is busmaster */
    unsigned int    no_msi:1;   /* device may not use msi */
    unsigned int    block_ucfg_access:1;    /* userspace config space access is blocked */
    unsigned int    broken_parity_status:1; /* Device generates false positive parity */
    unsigned int    irq_reroute_variant:2;  /* device needs IRQ rerouting variant */
    unsigned int    msi_enabled:1;
    unsigned int    msix_enabled:1;
    unsigned int    ari_enabled:1;  /* ARI forwarding */
    unsigned int    is_managed:1;
    unsigned int    is_pcie:1;
    unsigned int    state_saved:1;
    unsigned int    is_physfn:1;
    unsigned int    is_virtfn:1;
//    pci_dev_flags_t dev_flags;
//    atomic_t    enable_cnt;   /* pci_enable_device has been called */

//    u32     saved_config_space[16]; /* config space saved at suspend time */
//    struct hlist_head saved_cap_space;
//    struct bin_attribute *rom_attr; /* attribute descriptor for sysfs ROM entry */
//    int rom_attr_enabled;       /* has display of the rom attribute been enabled? */
//    struct bin_attribute *res_attr[DEVICE_COUNT_RESOURCE]; /* sysfs file for resources */
//    struct bin_attribute *res_attr_wc[DEVICE_COUNT_RESOURCE]; /* sysfs file for WC mapping of resources */
};

#define pci_resource_start(dev, bar)    ((dev)->resource[(bar)].start)
#define pci_resource_end(dev, bar)      ((dev)->resource[(bar)].end)
#define pci_resource_flags(dev, bar)    ((dev)->resource[(bar)].flags)
#define pci_resource_len(dev,bar) \
        ((pci_resource_start((dev), (bar)) == 0 &&      \
          pci_resource_end((dev), (bar)) ==             \
          pci_resource_start((dev), (bar))) ? 0 :       \
                                                        \
         (pci_resource_end((dev), (bar)) -              \
          pci_resource_start((dev), (bar)) + 1))

struct pci_device_id
{
    u16_t vendor, device;           /* Vendor and device ID or PCI_ANY_ID*/
    u16_t subvendor, subdevice;     /* Subsystem ID's or PCI_ANY_ID */
    u32_t class, class_mask;        /* (class,subclass,prog-if) triplet */
    u32_t driver_data;              /* Data private to the driver */
};

typedef struct
{
    link_t         link;
    struct pci_dev pci_dev;
}dev_t;

int enum_pci_devices(void);

struct pci_device_id*
find_pci_device(dev_t* pdev, struct pci_device_id *idlist);

#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))

int pci_set_dma_mask(struct pci_dev *dev, u64 mask);


#define pci_name(x) "radeon"

#endif //__PCI__H__


