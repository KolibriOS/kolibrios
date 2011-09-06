/*
 *	pci.h
 *
 *	PCI defines and function prototypes
 *	Copyright 1994, Drew Eckhardt
 *	Copyright 1997--1999 Martin Mares <mj@ucw.cz>
 *
 *	For more information, please consult the following manuals (look at
 *	http://www.pcisig.com/ for how to get them):
 *
 *	PCI BIOS Specification
 *	PCI Local Bus Specification
 *	PCI to PCI Bridge Specification
 *	PCI System Design Guide
 */

#ifndef LINUX_PCI_H
#define LINUX_PCI_H

#include <types.h>
#include <list.h>
#include <linux/pci_regs.h>	/* The pci register defines */
#include <ioport.h>


#define PCI_CFG_SPACE_SIZE      256
#define PCI_CFG_SPACE_EXP_SIZE  4096


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

/*
 * The PCI interface treats multi-function devices as independent
 * devices.  The slot/function address of each device is encoded
 * in a single byte as follows:
 *
 *	7:3 = slot
 *	2:0 = function
 */
#define PCI_DEVFN(slot, func)  ((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_SLOT(devfn)        (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)        ((devfn) & 0x07)



typedef unsigned int PCITAG;

extern inline PCITAG
pciTag(int busnum, int devnum, int funcnum)
{
    return(PCI_MAKE_TAG(busnum,devnum,funcnum));
}

/* pci_slot represents a physical slot */
struct pci_slot {
	struct pci_bus *bus;		/* The bus this slot is on */
	struct list_head list;		/* node in list of slots on this bus */
	struct hotplug_slot *hotplug;	/* Hotplug info (migrate over time) */
	unsigned char number;		/* PCI_SLOT(pci_dev->devfn) */
};

/* File state for mmap()s on /proc/bus/pci/X/Y */
enum pci_mmap_state {
	pci_mmap_io,
	pci_mmap_mem
};

/* This defines the direction arg to the DMA mapping routines. */
#define PCI_DMA_BIDIRECTIONAL	0
#define PCI_DMA_TODEVICE	1
#define PCI_DMA_FROMDEVICE	2
#define PCI_DMA_NONE		3

/*
 *  For PCI devices, the region numbers are assigned this way:
 */
enum {
    /* #0-5: standard PCI resources */
    PCI_STD_RESOURCES,
    PCI_STD_RESOURCE_END = 5,

    /* #6: expansion ROM resource */
    PCI_ROM_RESOURCE,

    /* device specific resources */
#ifdef CONFIG_PCI_IOV
    PCI_IOV_RESOURCES,
    PCI_IOV_RESOURCE_END = PCI_IOV_RESOURCES + PCI_SRIOV_NUM_BARS - 1,
#endif

    /* resources assigned to buses behind the bridge */
#define PCI_BRIDGE_RESOURCE_NUM 4

    PCI_BRIDGE_RESOURCES,
    PCI_BRIDGE_RESOURCE_END = PCI_BRIDGE_RESOURCES +
                  PCI_BRIDGE_RESOURCE_NUM - 1,

    /* total resources associated with a PCI device */
    PCI_NUM_RESOURCES,

    /* preserve this for compatibility */
    DEVICE_COUNT_RESOURCE
};

typedef int __bitwise pci_power_t;

#define PCI_D0		((pci_power_t __force) 0)
#define PCI_D1		((pci_power_t __force) 1)
#define PCI_D2		((pci_power_t __force) 2)
#define PCI_D3hot	((pci_power_t __force) 3)
#define PCI_D3cold	((pci_power_t __force) 4)
#define PCI_UNKNOWN	((pci_power_t __force) 5)
#define PCI_POWER_ERROR	((pci_power_t __force) -1)
/** The pci_channel state describes connectivity between the CPU and
 *  the pci device.  If some PCI bus between here and the pci device
 *  has crashed or locked up, this info is reflected here.
 */
typedef unsigned int __bitwise pci_channel_state_t;

enum pci_channel_state {
	/* I/O channel is in normal state */
	pci_channel_io_normal = (__force pci_channel_state_t) 1,

	/* I/O to channel is blocked */
	pci_channel_io_frozen = (__force pci_channel_state_t) 2,

	/* PCI card is dead */
	pci_channel_io_perm_failure = (__force pci_channel_state_t) 3,
};
typedef unsigned short __bitwise pci_bus_flags_t;
enum pci_bus_flags {
    PCI_BUS_FLAGS_NO_MSI   = (__force pci_bus_flags_t) 1,
    PCI_BUS_FLAGS_NO_MMRBC = (__force pci_bus_flags_t) 2,
};

/* Based on the PCI Hotplug Spec, but some values are made up by us */
enum pci_bus_speed {
	PCI_SPEED_33MHz			= 0x00,
	PCI_SPEED_66MHz			= 0x01,
	PCI_SPEED_66MHz_PCIX		= 0x02,
	PCI_SPEED_100MHz_PCIX		= 0x03,
	PCI_SPEED_133MHz_PCIX		= 0x04,
	PCI_SPEED_66MHz_PCIX_ECC	= 0x05,
	PCI_SPEED_100MHz_PCIX_ECC	= 0x06,
	PCI_SPEED_133MHz_PCIX_ECC	= 0x07,
	PCI_SPEED_66MHz_PCIX_266	= 0x09,
	PCI_SPEED_100MHz_PCIX_266	= 0x0a,
	PCI_SPEED_133MHz_PCIX_266	= 0x0b,
	AGP_UNKNOWN			= 0x0c,
	AGP_1X				= 0x0d,
	AGP_2X				= 0x0e,
	AGP_4X				= 0x0f,
	AGP_8X				= 0x10,
	PCI_SPEED_66MHz_PCIX_533	= 0x11,
	PCI_SPEED_100MHz_PCIX_533	= 0x12,
	PCI_SPEED_133MHz_PCIX_533	= 0x13,
	PCIE_SPEED_2_5GT		= 0x14,
	PCIE_SPEED_5_0GT		= 0x15,
	PCIE_SPEED_8_0GT		= 0x16,
	PCI_SPEED_UNKNOWN		= 0xff,
};

/*
 * The pci_dev structure is used to describe PCI devices.
 */
struct pci_dev {
    struct list_head bus_list;  /* node in per-bus list */
    struct pci_bus  *bus;       /* bus this device is on */
    struct pci_bus  *subordinate;   /* bus this device bridges to */

	void		*sysdata;	/* hook for sys-specific extension */
//    struct proc_dir_entry *procent; /* device entry in /proc/bus/pci */
	struct pci_slot	*slot;		/* Physical slot this device is in */
    u32_t           busnr;
	unsigned int	devfn;		/* encoded device & function index */
	unsigned short	vendor;
	unsigned short	device;
	unsigned short	subsystem_vendor;
	unsigned short	subsystem_device;
	unsigned int	class;		/* 3 bytes: (base,sub,prog-if) */
	u8		revision;	/* PCI revision, low byte of class word */
	u8		hdr_type;	/* PCI header type (`multi' flag masked out) */
	u8		pcie_cap;	/* PCI-E capability offset */
	u8		pcie_type;	/* PCI-E device/port type */
	u8		rom_base_reg;	/* which config register controls the ROM */
	u8		pin;  		/* which interrupt pin this device uses */

 //   struct pci_driver *driver;  /* which driver has allocated this device */
    uint64_t     dma_mask;   /* Mask of the bits of bus address this
                       device implements.  Normally this is
                       0xffffffff.  You only need to change
                       this if your device has broken DMA
                       or supports 64-bit transfers.  */

 //   struct device_dma_parameters dma_parms;

	pci_power_t     current_state;  /* Current operating state. In ACPI-speak,
					   this is D0-D3, D0 being fully functional,
					   and D3 being off. */
	int		pm_cap;		/* PM capability offset in the
					   configuration space */
    unsigned int    pme_support:5;  /* Bitmask of states from which PME#
                       can be generated */
	unsigned int	pme_interrupt:1;
    unsigned int    d1_support:1;   /* Low power state D1 is supported */
    unsigned int    d2_support:1;   /* Low power state D2 is supported */
    unsigned int    no_d1d2:1;  /* Only allow D0 and D3 */
	unsigned int	mmio_always_on:1;	/* disallow turning off io/mem
						   decoding during bar sizing */
	unsigned int	wakeup_prepared:1;
	unsigned int	d3_delay;	/* D3->D0 transition time in ms */


	pci_channel_state_t error_state;	/* current connectivity state */
    struct  device  dev;        /* Generic device interface */
    struct acpi_device *acpi_dev;
    int     cfg_size;   /* Size of configuration space */

    /*
     * Instead of touching interrupt line and base address registers
     * directly, use the values stored here. They might be different!
     */
    unsigned int    irq;
    struct resource resource[DEVICE_COUNT_RESOURCE]; /* I/O and memory regions + expansion ROMs */
	resource_size_t	fw_addr[DEVICE_COUNT_RESOURCE]; /* FW-assigned addr */

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
	unsigned int	ari_enabled:1;	/* ARI forwarding */
    unsigned int    is_managed:1;
	unsigned int	is_pcie:1;	/* Obsolete. Will be removed.
					   Use pci_is_pcie() instead */
	unsigned int    needs_freset:1; /* Dev requires fundamental reset */
    unsigned int    state_saved:1;
    unsigned int    is_physfn:1;
    unsigned int    is_virtfn:1;
	unsigned int	reset_fn:1;
	unsigned int    is_hotplug_bridge:1;
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


struct pci_bus {
    struct list_head node;      /* node in list of buses */
    struct pci_bus  *parent;    /* parent bus this bridge is on */
    struct list_head children;  /* list of child buses */
    struct list_head devices;   /* list of devices on this bus */
    struct pci_dev  *self;      /* bridge device as seen by parent */
    struct list_head slots;     /* list of slots on this bus */
    struct resource *resource[PCI_BRIDGE_RESOURCE_NUM];
    struct list_head resources; /* address space routed to this bus */

    struct pci_ops  *ops;       /* configuration access functions */
    void        *sysdata;   /* hook for sys-specific extension */
    struct proc_dir_entry *procdir; /* directory entry in /proc/bus/pci */

    unsigned char   number;     /* bus number */
    unsigned char   primary;    /* number of primary bridge */
    unsigned char   secondary;  /* number of secondary bridge */
    unsigned char   subordinate;    /* max number of subordinate buses */
    unsigned char   max_bus_speed;  /* enum pci_bus_speed */
    unsigned char   cur_bus_speed;  /* enum pci_bus_speed */

    char        name[48];

    unsigned short  bridge_ctl; /* manage NO_ISA/FBB/et al behaviors */
    pci_bus_flags_t bus_flags;  /* Inherited by child busses */
    struct device       *bridge;
    struct device       dev;
    struct bin_attribute    *legacy_io; /* legacy I/O for this bus */
    struct bin_attribute    *legacy_mem; /* legacy mem */
    unsigned int        is_added:1;
};


#define pci_bus_b(n)    list_entry(n, struct pci_bus, node)
#define to_pci_bus(n)   container_of(n, struct pci_bus, dev)
#define pci_dev_b(n)    list_entry(n, struct pci_dev, bus_list)
#define to_pci_dev(n) container_of(n, struct pci_dev, dev)
#define for_each_pci_dev(d) while ((d = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, d)) != NULL)


/* Low-level architecture-dependent routines */

struct pci_sysdata {
        int             domain;         /* PCI domain */
        int             node;           /* NUMA node */
};


#define pci_bus_b(n)    list_entry(n, struct pci_bus, node)
#define to_pci_bus(n)   container_of(n, struct pci_bus, dev)

/*
 * Returns true if the pci bus is root (behind host-pci bridge),
 * false otherwise
 */
static inline bool pci_is_root_bus(struct pci_bus *pbus)
{
    return !(pbus->parent);
}

struct pci_bus *
pci_find_next_bus(const struct pci_bus *from);


  /*
 * Error values that may be returned by PCI functions.
 */
#define PCIBIOS_SUCCESSFUL      0x00
#define PCIBIOS_FUNC_NOT_SUPPORTED  0x81
#define PCIBIOS_BAD_VENDOR_ID       0x83
#define PCIBIOS_DEVICE_NOT_FOUND    0x86
#define PCIBIOS_BAD_REGISTER_NUMBER 0x87
#define PCIBIOS_SET_FAILED      0x88
#define PCIBIOS_BUFFER_TOO_SMALL    0x89

/* Low-level architecture-dependent routines */

struct pci_ops {
    int (*read)(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *val);
    int (*write)(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 val);
};


enum pci_bar_type {
    pci_bar_unknown,    /* Standard PCI BAR probe */
    pci_bar_io,     /* An io port BAR */
    pci_bar_mem32,      /* A 32-bit memory BAR */
    pci_bar_mem64,      /* A 64-bit memory BAR */
};

/*
 * PCI domain support.  Sometimes called PCI segment (eg by ACPI),
 * a PCI domain is defined to be a set of PCI busses which share
 * configuration space.
 */
#ifdef CONFIG_PCI_DOMAINS
extern int pci_domains_supported;
#else
enum { pci_domains_supported = 0 };
static inline int pci_domain_nr(struct pci_bus *bus)
{
    return 0;
}

static inline int pci_proc_domain(struct pci_bus *bus)
{
    return 0;
}
#endif /* CONFIG_PCI_DOMAINS */

/**
 * pci_pcie_cap - get the saved PCIe capability offset
 * @dev: PCI device
 *
 * PCIe capability offset is calculated at PCI device initialization
 * time and saved in the data structure. This function returns saved
 * PCIe capability offset. Using this instead of pci_find_capability()
 * reduces unnecessary search in the PCI configuration space. If you
 * need to calculate PCIe capability offset from raw device for some
 * reasons, please use pci_find_capability() instead.
 */
static inline int pci_pcie_cap(struct pci_dev *dev)
{
    return dev->pcie_cap;
}

/**
 * pci_is_pcie - check if the PCI device is PCI Express capable
 * @dev: PCI device
 *
 * Retrun true if the PCI device is PCI Express capable, false otherwise.
 */
static inline bool pci_is_pcie(struct pci_dev *dev)
{
    return !!pci_pcie_cap(dev);
}

static inline int pci_iov_init(struct pci_dev *dev)
{
    return -ENODEV;
}
static inline void pci_iov_release(struct pci_dev *dev)

{}

static inline int pci_iov_resource_bar(struct pci_dev *dev, int resno,
                       enum pci_bar_type *type)
{
    return 0;
}
static inline void pci_restore_iov_state(struct pci_dev *dev)
{
}
static inline int pci_iov_bus_range(struct pci_bus *bus)
{
    return 0;
}

static inline int pci_enable_ats(struct pci_dev *dev, int ps)
{
    return -ENODEV;
}
static inline void pci_disable_ats(struct pci_dev *dev)
{
}
static inline int pci_ats_queue_depth(struct pci_dev *dev)
{
    return -ENODEV;
}
static inline int pci_ats_enabled(struct pci_dev *dev)
{
    return 0;
}

int pci_setup_device(struct pci_dev *dev);
int __pci_read_base(struct pci_dev *dev, enum pci_bar_type type,
         struct resource *res, unsigned int reg);
int pci_resource_bar(struct pci_dev *dev, int resno,
         enum pci_bar_type *type);
int pci_bus_add_child(struct pci_bus *bus);
unsigned int pci_scan_child_bus(struct pci_bus *bus);


typedef struct
{
    struct list_head    link;
    struct pci_dev      pci_dev;
}pci_dev_t;

int enum_pci_devices(void);

struct pci_device_id*
find_pci_device(pci_dev_t* pdev, struct pci_device_id *idlist);

#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))

int pci_set_dma_mask(struct pci_dev *dev, u64 mask);


#define pci_name(x) "radeon"

#endif //__PCI__H__


