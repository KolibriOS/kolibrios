#include <syscall.h>
#include <linux/pci.h>

LIST_HEAD(devices);

/* PCI control bits.  Shares IORESOURCE_BITS with above PCI ROM.  */
#define IORESOURCE_PCI_FIXED            (1<<4)  /* Do not move resource */

#define LEGACY_IO_RESOURCE      (IORESOURCE_IO | IORESOURCE_PCI_FIXED)

#define IORESOURCE_ROM_COPY             (1<<2)  /* ROM is alloc'd copy, resource field overlaid */
#define IORESOURCE_ROM_BIOS_COPY        (1<<3)  /* ROM is BIOS copy, resource field overlaid */


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

  /* if( pci_scan_filter(id, busnr, devfn) == 0)
        return NULL;*/

    hdr = PciRead8(busnr, devfn, PCI_HEADER_TYPE);

    dev = (pci_dev_t*)KernelZeroAlloc(sizeof(pci_dev_t));
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


int enum_pci_devices(void)
{
    pci_dev_t  *dev;
    u32       last_bus;
    u32       bus = 0 , devfn = 0;

    last_bus = PciApi(1);

    if( unlikely(last_bus == -1))
        return -1;

    for(;bus <= last_bus; bus++)
    {
        for (devfn = 0; devfn < 0x100; devfn += 8){
            _pci_scan_slot(bus, devfn);
        }
    }
    dev = (pci_dev_t*)devices.next;
    
    while(&dev->link != &devices)
    {
        /*printk("PCI device %x:%x bus:%x devfn:%x\n",
                dev->pci_dev.vendor,
                dev->pci_dev.device,
                dev->pci_dev.busnr,
                dev->pci_dev.devfn);*/
        dev = (pci_dev_t*)dev->link.next;
    }
    return 0;
}

void free_pci_devices(void) 
{
    pci_dev_t *dev = (pci_dev_t*)devices.next;
    while(&dev->link != &devices) {
        pci_dev_t *temp = dev;
        dev = (pci_dev_t*)dev->link.next;
        KernelFree(temp);
    }
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
        if((dev->pci_dev.vendor != vendor) && (vendor != PCI_ANY_ID))
                continue;

        if((dev->pci_dev.device == device || device == PCI_ANY_ID))
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


int pci_bus_read_config_byte (struct pci_bus *bus, u32 devfn, int pos, u8 *value)
{
//    raw_spin_lock_irqsave(&pci_lock, flags);
    *value = PciRead8(bus->number, devfn, pos);
//    raw_spin_unlock_irqrestore(&pci_lock, flags);
    return 0;
}

int pci_bus_read_config_word (struct pci_bus *bus, u32 devfn, int pos, u16 *value)
{
    if ( pos & 1)
        return PCIBIOS_BAD_REGISTER_NUMBER;
//    raw_spin_lock_irqsave(&pci_lock, flags);
    *value = PciRead16(bus->number, devfn, pos);
//    raw_spin_unlock_irqrestore(&pci_lock, flags);
    return 0;
}


int pci_bus_read_config_dword (struct pci_bus *bus, u32 devfn, int pos, u32 *value)
{
    if ( pos & 3)
        return PCIBIOS_BAD_REGISTER_NUMBER;
    *value = PciRead32(bus->number, devfn, pos);
    return 0;
}

int pci_bus_write_config_dword(struct pci_bus *bus, unsigned int devfn, int where, u32 val)
{
    if ( where & 3)
        return PCIBIOS_BAD_REGISTER_NUMBER;
    PciWrite32(bus->number, devfn,where, val);
    return 0;
}
