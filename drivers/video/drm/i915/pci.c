#include <syscall.h>

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/slab.h>
#include <linux/pm.h>

#include <linux/pci.h>

extern int pci_scan_filter(u32 id, u32 busnr, u32 devfn);

static LIST_HEAD(devices);

/* PCI control bits.  Shares IORESOURCE_BITS with above PCI ROM.  */
#define IORESOURCE_PCI_FIXED            (1<<4)  /* Do not move resource */

#define LEGACY_IO_RESOURCE      (IORESOURCE_IO | IORESOURCE_PCI_FIXED)

#define IORESOURCE_ROM_COPY             (1<<2)  /* ROM is alloc'd copy, resource field overlaid */
#define IORESOURCE_ROM_BIOS_COPY        (1<<3)  /* ROM is BIOS copy, resource field overlaid */

/*
 * Translate the low bits of the PCI base
 * to the resource type
 */
static inline unsigned int pci_calc_resource_flags(unsigned int flags)
{
    if (flags & PCI_BASE_ADDRESS_SPACE_IO)
        return IORESOURCE_IO;

    if (flags & PCI_BASE_ADDRESS_MEM_PREFETCH)
        return IORESOURCE_MEM | IORESOURCE_PREFETCH;

    return IORESOURCE_MEM;
}


static u32 pci_size(u32 base, u32 maxbase, u32 mask)
{
    u32 size = mask & maxbase;      /* Find the significant bits */

    if (!size)
        return 0;

    /* Get the lowest of them to find the decode size, and
       from that the extent.  */
    size = (size & ~(size-1)) - 1;

    /* base == maxbase can be valid only if the BAR has
       already been programmed with all 1s.  */
    if (base == maxbase && ((base | size) & mask) != mask)
        return 0;

    return size;
}

static u64 pci_size64(u64 base, u64 maxbase, u64 mask)
{
    u64 size = mask & maxbase;      /* Find the significant bits */

    if (!size)
        return 0;

    /* Get the lowest of them to find the decode size, and
       from that the extent.  */
    size = (size & ~(size-1)) - 1;

    /* base == maxbase can be valid only if the BAR has
       already been programmed with all 1s.  */
    if (base == maxbase && ((base | size) & mask) != mask)
        return 0;

    return size;
}

static inline int is_64bit_memory(u32 mask)
{
    if ((mask & (PCI_BASE_ADDRESS_SPACE|PCI_BASE_ADDRESS_MEM_TYPE_MASK)) ==
        (PCI_BASE_ADDRESS_SPACE_MEMORY|PCI_BASE_ADDRESS_MEM_TYPE_64))
        return 1;
    return 0;
}

static void pci_read_bases(struct pci_dev *dev, unsigned int howmany, int rom)
{
    u32  pos, reg, next;
    u32  l, sz;
    struct resource *res;

    for(pos=0; pos < howmany; pos = next)
    {
        u64  l64;
        u64  sz64;
        u32  raw_sz;

        next = pos + 1;

        res  = &dev->resource[pos];

        reg = PCI_BASE_ADDRESS_0 + (pos << 2);
        l = PciRead32(dev->busnr, dev->devfn, reg);
        PciWrite32(dev->busnr, dev->devfn, reg, ~0);
        sz = PciRead32(dev->busnr, dev->devfn, reg);
        PciWrite32(dev->busnr, dev->devfn, reg, l);

        if (!sz || sz == 0xffffffff)
            continue;

        if (l == 0xffffffff)
            l = 0;

        raw_sz = sz;
        if ((l & PCI_BASE_ADDRESS_SPACE) ==
                        PCI_BASE_ADDRESS_SPACE_MEMORY)
        {
            sz = pci_size(l, sz, (u32)PCI_BASE_ADDRESS_MEM_MASK);
            /*
             * For 64bit prefetchable memory sz could be 0, if the
             * real size is bigger than 4G, so we need to check
             * szhi for that.
             */
            if (!is_64bit_memory(l) && !sz)
                    continue;
            res->start = l & PCI_BASE_ADDRESS_MEM_MASK;
            res->flags |= l & ~PCI_BASE_ADDRESS_MEM_MASK;
        }
        else {
            sz = pci_size(l, sz, PCI_BASE_ADDRESS_IO_MASK & 0xffff);
            if (!sz)
                continue;
            res->start = l & PCI_BASE_ADDRESS_IO_MASK;
            res->flags |= l & ~PCI_BASE_ADDRESS_IO_MASK;
        }
        res->end = res->start + (unsigned long) sz;
        res->flags |= pci_calc_resource_flags(l);
        if (is_64bit_memory(l))
        {
            u32 szhi, lhi;

            lhi = PciRead32(dev->busnr, dev->devfn, reg+4);
            PciWrite32(dev->busnr, dev->devfn, reg+4, ~0);
            szhi = PciRead32(dev->busnr, dev->devfn, reg+4);
            PciWrite32(dev->busnr, dev->devfn, reg+4, lhi);
            sz64 = ((u64)szhi << 32) | raw_sz;
            l64 = ((u64)lhi << 32) | l;
            sz64 = pci_size64(l64, sz64, PCI_BASE_ADDRESS_MEM_MASK);
            next++;

#if BITS_PER_LONG == 64
            if (!sz64) {
                res->start = 0;
                res->end = 0;
                res->flags = 0;
                continue;
            }
            res->start = l64 & PCI_BASE_ADDRESS_MEM_MASK;
            res->end = res->start + sz64;
#else
            if (sz64 > 0x100000000ULL) {
                printk(KERN_ERR "PCI: Unable to handle 64-bit "
                                "BAR for device %s\n", pci_name(dev));
                res->start = 0;
                res->flags = 0;
            }
            else if (lhi)
            {
                /* 64-bit wide address, treat as disabled */
                PciWrite32(dev->busnr, dev->devfn, reg,
                        l & ~(u32)PCI_BASE_ADDRESS_MEM_MASK);
                PciWrite32(dev->busnr, dev->devfn, reg+4, 0);
                res->start = 0;
                res->end = sz;
            }
#endif
        }
    }

    if ( rom )
    {
        dev->rom_base_reg = rom;
        res = &dev->resource[PCI_ROM_RESOURCE];

        l = PciRead32(dev->busnr, dev->devfn, rom);
        PciWrite32(dev->busnr, dev->devfn, rom, ~PCI_ROM_ADDRESS_ENABLE);
        sz = PciRead32(dev->busnr, dev->devfn, rom);
        PciWrite32(dev->busnr, dev->devfn, rom, l);

        if (l == 0xffffffff)
            l = 0;

        if (sz && sz != 0xffffffff)
        {
            sz = pci_size(l, sz, (u32)PCI_ROM_ADDRESS_MASK);

            if (sz)
            {
                res->flags = (l & IORESOURCE_ROM_ENABLE) |
                                  IORESOURCE_MEM | IORESOURCE_PREFETCH |
                                  IORESOURCE_READONLY | IORESOURCE_CACHEABLE;
                res->start = l & PCI_ROM_ADDRESS_MASK;
                res->end = res->start + (unsigned long) sz;
            }
        }
    }
}

static void pci_read_irq(struct pci_dev *dev)
{
    u8 irq;

    irq = PciRead8(dev->busnr, dev->devfn, PCI_INTERRUPT_PIN);
    dev->pin = irq;
    if (irq)
        irq = PciRead8(dev->busnr, dev->devfn, PCI_INTERRUPT_LINE);
    dev->irq = irq;
};


int pci_setup_device(struct pci_dev *dev)
{
    u32  class;

    class = PciRead32(dev->busnr, dev->devfn, PCI_CLASS_REVISION);
    dev->revision = class & 0xff;
    class >>= 8;                                /* upper 3 bytes */
    dev->class = class;

    /* "Unknown power state" */
//    dev->current_state = PCI_UNKNOWN;

    /* Early fixups, before probing the BARs */
 //   pci_fixup_device(pci_fixup_early, dev);
    class = dev->class >> 8;

    switch (dev->hdr_type)
    {
        case PCI_HEADER_TYPE_NORMAL:                /* standard header */
            if (class == PCI_CLASS_BRIDGE_PCI)
                goto bad;
            pci_read_irq(dev);
            pci_read_bases(dev, 6, PCI_ROM_ADDRESS);
            dev->subsystem_vendor = PciRead16(dev->busnr, dev->devfn,PCI_SUBSYSTEM_VENDOR_ID);
            dev->subsystem_device = PciRead16(dev->busnr, dev->devfn, PCI_SUBSYSTEM_ID);

            /*
             *      Do the ugly legacy mode stuff here rather than broken chip
             *      quirk code. Legacy mode ATA controllers have fixed
             *      addresses. These are not always echoed in BAR0-3, and
             *      BAR0-3 in a few cases contain junk!
             */
            if (class == PCI_CLASS_STORAGE_IDE)
            {
                u8 progif;

                progif = PciRead8(dev->busnr, dev->devfn,PCI_CLASS_PROG);
                if ((progif & 1) == 0)
                {
                    dev->resource[0].start = 0x1F0;
                    dev->resource[0].end = 0x1F7;
                    dev->resource[0].flags = LEGACY_IO_RESOURCE;
                    dev->resource[1].start = 0x3F6;
                    dev->resource[1].end = 0x3F6;
                    dev->resource[1].flags = LEGACY_IO_RESOURCE;
                }
                if ((progif & 4) == 0)
                {
                    dev->resource[2].start = 0x170;
                    dev->resource[2].end = 0x177;
                    dev->resource[2].flags = LEGACY_IO_RESOURCE;
                    dev->resource[3].start = 0x376;
                    dev->resource[3].end = 0x376;
                    dev->resource[3].flags = LEGACY_IO_RESOURCE;
                };
            }
            break;

        case PCI_HEADER_TYPE_BRIDGE:                /* bridge header */
                if (class != PCI_CLASS_BRIDGE_PCI)
                        goto bad;
                /* The PCI-to-PCI bridge spec requires that subtractive
                   decoding (i.e. transparent) bridge must have programming
                   interface code of 0x01. */
                pci_read_irq(dev);
                dev->transparent = ((dev->class & 0xff) == 1);
                pci_read_bases(dev, 2, PCI_ROM_ADDRESS1);
                break;

        case PCI_HEADER_TYPE_CARDBUS:               /* CardBus bridge header */
                if (class != PCI_CLASS_BRIDGE_CARDBUS)
                        goto bad;
                pci_read_irq(dev);
                pci_read_bases(dev, 1, 0);
                dev->subsystem_vendor = PciRead16(dev->busnr,
                                                  dev->devfn,
                                                  PCI_CB_SUBSYSTEM_VENDOR_ID);

                dev->subsystem_device = PciRead16(dev->busnr,
                                                  dev->devfn,
                                                  PCI_CB_SUBSYSTEM_ID);
                break;

        default:                                    /* unknown header */
                printk(KERN_ERR "PCI: device %s has unknown header type %02x, ignoring.\n",
                        pci_name(dev), dev->hdr_type);
                return -1;

        bad:
                printk(KERN_ERR "PCI: %s: class %x doesn't match header type %02x. Ignoring class.\n",
                       pci_name(dev), class, dev->hdr_type);
                dev->class = PCI_CLASS_NOT_DEFINED;
    }

    /* We found a fine healthy device, go go go... */

    return 0;
};

static pci_dev_t* pci_scan_device(u32 busnr, int devfn)
{
    pci_dev_t  *dev;

    u32   id;
    u8    hdr;

    int     timeout = 10;

    id = PciRead32(busnr, devfn, PCI_VENDOR_ID);

    /* some broken boards return 0 or ~0 if a slot is empty: */
    if (id == 0xffffffff || id == 0x00000000 ||
        id == 0x0000ffff || id == 0xffff0000)
        return NULL;

    while (id == 0xffff0001)
    {

        delay(timeout/10);
        timeout *= 2;

        id = PciRead32(busnr, devfn, PCI_VENDOR_ID);

        /* Card hasn't responded in 60 seconds?  Must be stuck. */
        if (timeout > 60 * 100)
        {
            printk(KERN_WARNING "Device %04x:%02x:%02x.%d not "
                   "responding\n", busnr,PCI_SLOT(devfn),PCI_FUNC(devfn));
            return NULL;
        }
    };

    if( pci_scan_filter(id, busnr, devfn) == 0)
        return NULL;

    hdr = PciRead8(busnr, devfn, PCI_HEADER_TYPE);

    dev = (pci_dev_t*)kzalloc(sizeof(pci_dev_t), 0);
    if(unlikely(dev == NULL))
        return NULL;

    INIT_LIST_HEAD(&dev->link);


    dev->pci_dev.busnr    = busnr;
    dev->pci_dev.devfn    = devfn;
    dev->pci_dev.hdr_type = hdr & 0x7f;
    dev->pci_dev.multifunction    = !!(hdr & 0x80);
    dev->pci_dev.vendor   = id & 0xffff;
    dev->pci_dev.device   = (id >> 16) & 0xffff;

    pci_setup_device(&dev->pci_dev);

    return dev;

};




int _pci_scan_slot(u32 bus, int devfn)
{
    int  func, nr = 0;

    for (func = 0; func < 8; func++, devfn++)
    {
        pci_dev_t  *dev;

        dev = pci_scan_device(bus, devfn);
        if( dev )
        {
            list_add(&dev->link, &devices);

            nr++;

            /*
             * If this is a single function device,
             * don't scan past the first function.
             */
            if (!dev->pci_dev.multifunction)
            {
                if (func > 0) {
                    dev->pci_dev.multifunction = 1;
                }
                else {
                    break;
                }
             }
        }
        else {
            if (func == 0)
                break;
        }
    };

    return nr;
};

#define PCI_FIND_CAP_TTL    48

static int __pci_find_next_cap_ttl(unsigned int bus, unsigned int devfn,
                   u8 pos, int cap, int *ttl)
{
    u8 id;

    while ((*ttl)--) {
        pos = PciRead8(bus, devfn, pos);
        if (pos < 0x40)
            break;
        pos &= ~3;
        id = PciRead8(bus, devfn, pos + PCI_CAP_LIST_ID);
        if (id == 0xff)
            break;
        if (id == cap)
            return pos;
        pos += PCI_CAP_LIST_NEXT;
    }
    return 0;
}

static int __pci_find_next_cap(unsigned int bus, unsigned int devfn,
                   u8 pos, int cap)
{
    int ttl = PCI_FIND_CAP_TTL;

    return __pci_find_next_cap_ttl(bus, devfn, pos, cap, &ttl);
}

static int __pci_bus_find_cap_start(unsigned int bus,
                    unsigned int devfn, u8 hdr_type)
{
    u16 status;

    status = PciRead16(bus, devfn, PCI_STATUS);
    if (!(status & PCI_STATUS_CAP_LIST))
        return 0;

    switch (hdr_type) {
    case PCI_HEADER_TYPE_NORMAL:
    case PCI_HEADER_TYPE_BRIDGE:
        return PCI_CAPABILITY_LIST;
    case PCI_HEADER_TYPE_CARDBUS:
        return PCI_CB_CAPABILITY_LIST;
    default:
        return 0;
    }

    return 0;
}


int pci_find_capability(struct pci_dev *dev, int cap)
{
    int pos;

    pos = __pci_bus_find_cap_start(dev->busnr, dev->devfn, dev->hdr_type);
    if (pos)
        pos = __pci_find_next_cap(dev->busnr, dev->devfn, pos, cap);

    return pos;
}




int enum_pci_devices()
{
    pci_dev_t  *dev;
    u32       last_bus;
    u32       bus = 0 , devfn = 0;


    last_bus = PciApi(1);


    if( unlikely(last_bus == -1))
        return -1;

    for(;bus <= last_bus; bus++)
    {
        for (devfn = 0; devfn < 0x100; devfn += 8)
            _pci_scan_slot(bus, devfn);


    }
    for(dev = (pci_dev_t*)devices.next;
        &dev->link != &devices;
        dev = (pci_dev_t*)dev->link.next)
    {
        dbgprintf("PCI device %x:%x bus:%x devfn:%x\n",
                dev->pci_dev.vendor,
                dev->pci_dev.device,
                dev->pci_dev.busnr,
                dev->pci_dev.devfn);

    }
    return 0;
}

const struct pci_device_id* find_pci_device(pci_dev_t* pdev, const struct pci_device_id *idlist)
{
    pci_dev_t *dev;
    const struct pci_device_id *ent;

    for(dev = (pci_dev_t*)devices.next;
        &dev->link != &devices;
        dev = (pci_dev_t*)dev->link.next)
    {
        if( dev->pci_dev.vendor != idlist->vendor )
            continue;

        for(ent = idlist; ent->vendor != 0; ent++)
        {
            if(unlikely(ent->device == dev->pci_dev.device))
            {
                pdev->pci_dev = dev->pci_dev;
                return  ent;
            }
        };
    }

    return NULL;
};

struct pci_dev *
pci_get_device(unsigned int vendor, unsigned int device, struct pci_dev *from)
{
    pci_dev_t *dev;

    dev = (pci_dev_t*)devices.next;

    if(from != NULL)
    {
        for(; &dev->link != &devices;
            dev = (pci_dev_t*)dev->link.next)
        {
            if( &dev->pci_dev == from)
            {
                dev = (pci_dev_t*)dev->link.next;
                break;
            };
        }
    };

    for(; &dev->link != &devices;
        dev = (pci_dev_t*)dev->link.next)
    {
        if( dev->pci_dev.vendor != vendor )
                continue;

        if(dev->pci_dev.device == device)
        {
            return &dev->pci_dev;
        }
    }
    return NULL;
};


struct pci_dev * _pci_get_bus_and_slot(unsigned int bus, unsigned int devfn)
{
    pci_dev_t *dev;

    for(dev = (pci_dev_t*)devices.next;
        &dev->link != &devices;
        dev = (pci_dev_t*)dev->link.next)
    {
        if ( dev->pci_dev.busnr == bus && dev->pci_dev.devfn == devfn)
            return &dev->pci_dev;
    }
    return NULL;
}

struct pci_dev *pci_get_class(unsigned int class, struct pci_dev *from)
{
    pci_dev_t *dev;

    dev = (pci_dev_t*)devices.next;

    if(from != NULL)
    {
        for(; &dev->link != &devices;
            dev = (pci_dev_t*)dev->link.next)
        {
            if( &dev->pci_dev == from)
            {
                dev = (pci_dev_t*)dev->link.next;
                break;
            };
        }
    };

    for(; &dev->link != &devices;
        dev = (pci_dev_t*)dev->link.next)
    {
        if( dev->pci_dev.class == class)
        {
            return &dev->pci_dev;
        }
    }

   return NULL;
}


#define PIO_OFFSET      0x10000UL
#define PIO_MASK        0x0ffffUL
#define PIO_RESERVED    0x40000UL

#define IO_COND(addr, is_pio, is_mmio) do {            \
    unsigned long port = (unsigned long __force)addr;  \
    if (port >= PIO_RESERVED) {                        \
        is_mmio;                                       \
    } else if (port > PIO_OFFSET) {                    \
        port &= PIO_MASK;                              \
        is_pio;                                        \
    };                                                 \
} while (0)

/* Create a virtual mapping cookie for an IO port range */
void __iomem *ioport_map(unsigned long port, unsigned int nr)
{
    if (port > PIO_MASK)
        return NULL;
    return (void __iomem *) (unsigned long) (port + PIO_OFFSET);
}

void __iomem *pci_iomap(struct pci_dev *dev, int bar, unsigned long maxlen)
{
    resource_size_t start = pci_resource_start(dev, bar);
    resource_size_t len = pci_resource_len(dev, bar);
    unsigned long flags = pci_resource_flags(dev, bar);

    if (!len || !start)
        return NULL;
    if (maxlen && len > maxlen)
        len = maxlen;
    if (flags & IORESOURCE_IO)
        return ioport_map(start, len);
    if (flags & IORESOURCE_MEM) {
        return ioremap(start, len);
    }
    /* What? */
    return NULL;
}

void pci_iounmap(struct pci_dev *dev, void __iomem * addr)
{
    IO_COND(addr, /* nothing */, iounmap(addr));
}




int pci_enable_rom(struct pci_dev *pdev)
{
    struct resource *res = pdev->resource + PCI_ROM_RESOURCE;
    struct pci_bus_region region;
    u32 rom_addr;

    if (!res->flags)
            return -1;

    _pcibios_resource_to_bus(pdev, &region, res);
    pci_read_config_dword(pdev, pdev->rom_base_reg, &rom_addr);
    rom_addr &= ~PCI_ROM_ADDRESS_MASK;
    rom_addr |= region.start | PCI_ROM_ADDRESS_ENABLE;
    pci_write_config_dword(pdev, pdev->rom_base_reg, rom_addr);
    return 0;
}

void pci_disable_rom(struct pci_dev *pdev)
{
    u32 rom_addr;
    pci_read_config_dword(pdev, pdev->rom_base_reg, &rom_addr);
    rom_addr &= ~PCI_ROM_ADDRESS_ENABLE;
    pci_write_config_dword(pdev, pdev->rom_base_reg, rom_addr);
}

/**
 * pci_get_rom_size - obtain the actual size of the ROM image
 * @pdev: target PCI device
 * @rom: kernel virtual pointer to image of ROM
 * @size: size of PCI window
 *  return: size of actual ROM image
 *
 * Determine the actual length of the ROM image.
 * The PCI window size could be much larger than the
 * actual image size.
 */
size_t pci_get_rom_size(struct pci_dev *pdev, void __iomem *rom, size_t size)
{
        void __iomem *image;
        int last_image;

        image = rom;
        do {
                void __iomem *pds;
                /* Standard PCI ROMs start out with these bytes 55 AA */
                if (readb(image) != 0x55) {
                        dev_err(&pdev->dev, "Invalid ROM contents\n");
                        break;
                }
                if (readb(image + 1) != 0xAA)
                        break;
                /* get the PCI data structure and check its signature */
                pds = image + readw(image + 24);
                if (readb(pds) != 'P')
                        break;
                if (readb(pds + 1) != 'C')
                        break;
                if (readb(pds + 2) != 'I')
                        break;
                if (readb(pds + 3) != 'R')
                        break;
                last_image = readb(pds + 21) & 0x80;
                /* this length is reliable */
                image += readw(pds + 16) * 512;
        } while (!last_image);

        /* never return a size larger than the PCI resource window */
        /* there are known ROMs that get the size wrong */
        return min((size_t)(image - rom), size);
}


/**
 * pci_map_rom - map a PCI ROM to kernel space
 * @pdev: pointer to pci device struct
 * @size: pointer to receive size of pci window over ROM
 *
 * Return: kernel virtual pointer to image of ROM
 *
 * Map a PCI ROM into kernel space. If ROM is boot video ROM,
 * the shadow BIOS copy will be returned instead of the
 * actual ROM.
 */
void __iomem *pci_map_rom(struct pci_dev *pdev, size_t *size)
{
    struct resource *res = &pdev->resource[PCI_ROM_RESOURCE];
    loff_t start;
    void __iomem *rom;

    /*
     * IORESOURCE_ROM_SHADOW set on x86, x86_64 and IA64 supports legacy
     * memory map if the VGA enable bit of the Bridge Control register is
     * set for embedded VGA.
     */
    if (res->flags & IORESOURCE_ROM_SHADOW) {
        /* primary video rom always starts here */
        start = (loff_t)0xC0000;
        *size = 0x20000; /* cover C000:0 through E000:0 */
    } else {
                if (res->flags &
                        (IORESOURCE_ROM_COPY | IORESOURCE_ROM_BIOS_COPY)) {
            *size = pci_resource_len(pdev, PCI_ROM_RESOURCE);
             return (void __iomem *)(unsigned long)
             pci_resource_start(pdev, PCI_ROM_RESOURCE);
        } else {
    				start = (loff_t)0xC0000;
    				*size = 0x20000; /* cover C000:0 through E000:0 */

        }
    }

    rom = ioremap(start, *size);
    if (!rom) {
            /* restore enable if ioremap fails */
            if (!(res->flags & (IORESOURCE_ROM_ENABLE |
                                IORESOURCE_ROM_SHADOW |
                                IORESOURCE_ROM_COPY)))
                    pci_disable_rom(pdev);
            return NULL;
    }

    /*
     * Try to find the true size of the ROM since sometimes the PCI window
     * size is much larger than the actual size of the ROM.
     * True size is important if the ROM is going to be copied.
     */
    *size = pci_get_rom_size(pdev, rom, *size);
    return rom;
}

void pci_unmap_rom(struct pci_dev *pdev, void __iomem *rom)
{
    struct resource *res = &pdev->resource[PCI_ROM_RESOURCE];

    if (res->flags & (IORESOURCE_ROM_COPY | IORESOURCE_ROM_BIOS_COPY))
            return;

    iounmap(rom);

    /* Disable again before continuing, leave enabled if pci=rom */
    if (!(res->flags & (IORESOURCE_ROM_ENABLE | IORESOURCE_ROM_SHADOW)))
            pci_disable_rom(pdev);
}

static void __pci_set_master(struct pci_dev *dev, bool enable)
{
    u16 old_cmd, cmd;

    pci_read_config_word(dev, PCI_COMMAND, &old_cmd);
    if (enable)
        cmd = old_cmd | PCI_COMMAND_MASTER;
    else
        cmd = old_cmd & ~PCI_COMMAND_MASTER;
    if (cmd != old_cmd) {
            dbgprintf("%s bus mastering\n",
                    enable ? "enabling" : "disabling");
        pci_write_config_word(dev, PCI_COMMAND, cmd);
        }
    dev->is_busmaster = enable;
}


/* pci_set_master - enables bus-mastering for device dev
 * @dev: the PCI device to enable
 *
 * Enables bus-mastering on the device and calls pcibios_set_master()
 * to do the needed arch specific settings.
 */
void pci_set_master(struct pci_dev *dev)
{
        __pci_set_master(dev, true);
//        pcibios_set_master(dev);
}

/**
 * pci_clear_master - disables bus-mastering for device dev
 * @dev: the PCI device to disable
 */
void pci_clear_master(struct pci_dev *dev)
{
        __pci_set_master(dev, false);
}


static inline int pcie_cap_version(const struct pci_dev *dev)
{
    return dev->pcie_flags_reg & PCI_EXP_FLAGS_VERS;
}

static inline bool pcie_cap_has_devctl(const struct pci_dev *dev)
{
    return true;
}

static inline bool pcie_cap_has_lnkctl(const struct pci_dev *dev)
{
    int type = pci_pcie_type(dev);

    return pcie_cap_version(dev) > 1 ||
           type == PCI_EXP_TYPE_ROOT_PORT ||
           type == PCI_EXP_TYPE_ENDPOINT ||
           type == PCI_EXP_TYPE_LEG_END;
}

static inline bool pcie_cap_has_sltctl(const struct pci_dev *dev)
{
    int type = pci_pcie_type(dev);

    return pcie_cap_version(dev) > 1 ||
           type == PCI_EXP_TYPE_ROOT_PORT ||
           (type == PCI_EXP_TYPE_DOWNSTREAM &&
        dev->pcie_flags_reg & PCI_EXP_FLAGS_SLOT);
}

static inline bool pcie_cap_has_rtctl(const struct pci_dev *dev)
{
    int type = pci_pcie_type(dev);

    return pcie_cap_version(dev) > 1 ||
           type == PCI_EXP_TYPE_ROOT_PORT ||
           type == PCI_EXP_TYPE_RC_EC;
}

static bool pcie_capability_reg_implemented(struct pci_dev *dev, int pos)
{
    if (!pci_is_pcie(dev))
        return false;

    switch (pos) {
    case PCI_EXP_FLAGS_TYPE:
        return true;
    case PCI_EXP_DEVCAP:
    case PCI_EXP_DEVCTL:
    case PCI_EXP_DEVSTA:
        return pcie_cap_has_devctl(dev);
    case PCI_EXP_LNKCAP:
    case PCI_EXP_LNKCTL:
    case PCI_EXP_LNKSTA:
        return pcie_cap_has_lnkctl(dev);
    case PCI_EXP_SLTCAP:
    case PCI_EXP_SLTCTL:
    case PCI_EXP_SLTSTA:
        return pcie_cap_has_sltctl(dev);
    case PCI_EXP_RTCTL:
    case PCI_EXP_RTCAP:
    case PCI_EXP_RTSTA:
        return pcie_cap_has_rtctl(dev);
    case PCI_EXP_DEVCAP2:
    case PCI_EXP_DEVCTL2:
    case PCI_EXP_LNKCAP2:
    case PCI_EXP_LNKCTL2:
    case PCI_EXP_LNKSTA2:
        return pcie_cap_version(dev) > 1;
    default:
        return false;
    }
}

/*
 * Note that these accessor functions are only for the "PCI Express
 * Capability" (see PCIe spec r3.0, sec 7.8).  They do not apply to the
 * other "PCI Express Extended Capabilities" (AER, VC, ACS, MFVC, etc.)
 */
int pcie_capability_read_word(struct pci_dev *dev, int pos, u16 *val)
{
    int ret;

    *val = 0;
    if (pos & 1)
        return -EINVAL;

    if (pcie_capability_reg_implemented(dev, pos)) {
        ret = pci_read_config_word(dev, pci_pcie_cap(dev) + pos, val);
        /*
         * Reset *val to 0 if pci_read_config_word() fails, it may
         * have been written as 0xFFFF if hardware error happens
         * during pci_read_config_word().
         */
        if (ret)
            *val = 0;
        return ret;
    }

    /*
     * For Functions that do not implement the Slot Capabilities,
     * Slot Status, and Slot Control registers, these spaces must
     * be hardwired to 0b, with the exception of the Presence Detect
     * State bit in the Slot Status register of Downstream Ports,
     * which must be hardwired to 1b.  (PCIe Base Spec 3.0, sec 7.8)
     */
    if (pci_is_pcie(dev) && pos == PCI_EXP_SLTSTA &&
         pci_pcie_type(dev) == PCI_EXP_TYPE_DOWNSTREAM) {
        *val = PCI_EXP_SLTSTA_PDS;
    }

    return 0;
}
EXPORT_SYMBOL(pcie_capability_read_word);

int pcie_capability_read_dword(struct pci_dev *dev, int pos, u32 *val)
{
    int ret;

    *val = 0;
    if (pos & 3)
        return -EINVAL;

    if (pcie_capability_reg_implemented(dev, pos)) {
        ret = pci_read_config_dword(dev, pci_pcie_cap(dev) + pos, val);
        /*
         * Reset *val to 0 if pci_read_config_dword() fails, it may
         * have been written as 0xFFFFFFFF if hardware error happens
         * during pci_read_config_dword().
         */
        if (ret)
            *val = 0;
        return ret;
    }

    if (pci_is_pcie(dev) && pos == PCI_EXP_SLTCTL &&
         pci_pcie_type(dev) == PCI_EXP_TYPE_DOWNSTREAM) {
        *val = PCI_EXP_SLTSTA_PDS;
    }

    return 0;
}
EXPORT_SYMBOL(pcie_capability_read_dword);

int pcie_capability_write_word(struct pci_dev *dev, int pos, u16 val)
{
    if (pos & 1)
        return -EINVAL;

    if (!pcie_capability_reg_implemented(dev, pos))
        return 0;

    return pci_write_config_word(dev, pci_pcie_cap(dev) + pos, val);
}
EXPORT_SYMBOL(pcie_capability_write_word);

int pcie_capability_write_dword(struct pci_dev *dev, int pos, u32 val)
{
    if (pos & 3)
        return -EINVAL;

    if (!pcie_capability_reg_implemented(dev, pos))
        return 0;

    return pci_write_config_dword(dev, pci_pcie_cap(dev) + pos, val);
}
EXPORT_SYMBOL(pcie_capability_write_dword);

int pcie_capability_clear_and_set_word(struct pci_dev *dev, int pos,
                                       u16 clear, u16 set)
{
        int ret;
        u16 val;

        ret = pcie_capability_read_word(dev, pos, &val);
        if (!ret) {
                val &= ~clear;
                val |= set;
                ret = pcie_capability_write_word(dev, pos, val);
        }

        return ret;
}



int pcie_get_readrq(struct pci_dev *dev)
{
        u16 ctl;

        pcie_capability_read_word(dev, PCI_EXP_DEVCTL, &ctl);

        return 128 << ((ctl & PCI_EXP_DEVCTL_READRQ) >> 12);
}
EXPORT_SYMBOL(pcie_get_readrq);

/**
 * pcie_set_readrq - set PCI Express maximum memory read request
 * @dev: PCI device to query
 * @rq: maximum memory read count in bytes
 *    valid values are 128, 256, 512, 1024, 2048, 4096
 *
 * If possible sets maximum memory read request in bytes
 */
int pcie_set_readrq(struct pci_dev *dev, int rq)
{
        u16 v;

        if (rq < 128 || rq > 4096 || !is_power_of_2(rq))
                return -EINVAL;

        v = (ffs(rq) - 8) << 12;

        return pcie_capability_clear_and_set_word(dev, PCI_EXP_DEVCTL,
                                                  PCI_EXP_DEVCTL_READRQ, v);
}

