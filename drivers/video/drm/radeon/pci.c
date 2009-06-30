
#include <pci.h>
#include <errno-base.h>
#include <syscall.h>

link_t  devices;

static dev_t* pci_scan_device(u32_t bus, int devfn);


/* PCI control bits.  Shares IORESOURCE_BITS with above PCI ROM.  */
#define IORESOURCE_PCI_FIXED            (1<<4)  /* Do not move resource */

#define LEGACY_IO_RESOURCE      (IORESOURCE_IO | IORESOURCE_PCI_FIXED)

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


static u32_t pci_size(u32_t base, u32_t maxbase, u32_t mask)
{
    u32_t size = mask & maxbase;      /* Find the significant bits */

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

static u64_t pci_size64(u64_t base, u64_t maxbase, u64_t mask)
{
    u64_t size = mask & maxbase;      /* Find the significant bits */

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

static inline int is_64bit_memory(u32_t mask)
{
    if ((mask & (PCI_BASE_ADDRESS_SPACE|PCI_BASE_ADDRESS_MEM_TYPE_MASK)) ==
        (PCI_BASE_ADDRESS_SPACE_MEMORY|PCI_BASE_ADDRESS_MEM_TYPE_64))
        return 1;
    return 0;
}

static void pci_read_bases(struct pci_dev *dev, unsigned int howmany, int rom)
{
    u32_t  pos, reg, next;
    u32_t  l, sz;
    struct resource *res;

    for(pos=0; pos < howmany; pos = next)
    {
        u64_t  l64;
        u64_t  sz64;
        u32_t  raw_sz;

        next = pos + 1;

        res  = &dev->resource[pos];

        reg = PCI_BASE_ADDRESS_0 + (pos << 2);
        l = PciRead32(dev->bus, dev->devfn, reg);
        PciWrite32(dev->bus, dev->devfn, reg, ~0);
        sz = PciRead32(dev->bus, dev->devfn, reg);
        PciWrite32(dev->bus, dev->devfn, reg, l);

        if (!sz || sz == 0xffffffff)
            continue;

        if (l == 0xffffffff)
            l = 0;

        raw_sz = sz;
        if ((l & PCI_BASE_ADDRESS_SPACE) ==
                        PCI_BASE_ADDRESS_SPACE_MEMORY)
        {
            sz = pci_size(l, sz, (u32_t)PCI_BASE_ADDRESS_MEM_MASK);
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
            u32_t szhi, lhi;

            lhi = PciRead32(dev->bus, dev->devfn, reg+4);
            PciWrite32(dev->bus, dev->devfn, reg+4, ~0);
            szhi = PciRead32(dev->bus, dev->devfn, reg+4);
            PciWrite32(dev->bus, dev->devfn, reg+4, lhi);
            sz64 = ((u64_t)szhi << 32) | raw_sz;
            l64 = ((u64_t)lhi << 32) | l;
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
                PciWrite32(dev->bus, dev->devfn, reg,
                        l & ~(u32_t)PCI_BASE_ADDRESS_MEM_MASK);
                PciWrite32(dev->bus, dev->devfn, reg+4, 0);
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

        l = PciRead32(dev->bus, dev->devfn, rom);
        PciWrite32(dev->bus, dev->devfn, rom, ~PCI_ROM_ADDRESS_ENABLE);
        sz = PciRead32(dev->bus, dev->devfn, rom);
        PciWrite32(dev->bus, dev->devfn, rom, l);

        if (l == 0xffffffff)
            l = 0;

        if (sz && sz != 0xffffffff)
        {
            sz = pci_size(l, sz, (u32_t)PCI_ROM_ADDRESS_MASK);

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
    u8_t irq;

    irq = PciRead8(dev->bus, dev->devfn, PCI_INTERRUPT_PIN);
    dev->pin = irq;
    if (irq)
        PciRead8(dev->bus, dev->devfn, PCI_INTERRUPT_LINE);
    dev->irq = irq;
};


static int pci_setup_device(struct pci_dev *dev)
{
    u32_t  class;

    class = PciRead32(dev->bus, dev->devfn, PCI_CLASS_REVISION);
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
            dev->subsystem_vendor = PciRead16(dev->bus, dev->devfn,PCI_SUBSYSTEM_VENDOR_ID);
            dev->subsystem_device = PciRead16(dev->bus, dev->devfn, PCI_SUBSYSTEM_ID);

            /*
             *      Do the ugly legacy mode stuff here rather than broken chip
             *      quirk code. Legacy mode ATA controllers have fixed
             *      addresses. These are not always echoed in BAR0-3, and
             *      BAR0-3 in a few cases contain junk!
             */
            if (class == PCI_CLASS_STORAGE_IDE)
            {
                u8_t progif;

                progif = PciRead8(dev->bus, dev->devfn,PCI_CLASS_PROG);
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
                dev->subsystem_vendor = PciRead16(dev->bus,
                                                  dev->devfn,
                                                  PCI_CB_SUBSYSTEM_VENDOR_ID);

                dev->subsystem_device = PciRead16(dev->bus,
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

static dev_t* pci_scan_device(u32_t bus, int devfn)
{
    dev_t  *dev;

    u32_t   id;
    u8_t    hdr;

    int     timeout = 10;

    id = PciRead32(bus,devfn, PCI_VENDOR_ID);

    /* some broken boards return 0 or ~0 if a slot is empty: */
    if (id == 0xffffffff || id == 0x00000000 ||
        id == 0x0000ffff || id == 0xffff0000)
        return NULL;

    while (id == 0xffff0001)
    {

        delay(timeout/10);
        timeout *= 2;

        id = PciRead32(bus, devfn, PCI_VENDOR_ID);

        /* Card hasn't responded in 60 seconds?  Must be stuck. */
        if (timeout > 60 * 100)
        {
            printk(KERN_WARNING "Device %04x:%02x:%02x.%d not "
                   "responding\n", bus,PCI_SLOT(devfn),PCI_FUNC(devfn));
            return NULL;
        }
    };

    hdr = PciRead8(bus, devfn, PCI_HEADER_TYPE);

    dev = (dev_t*)malloc(sizeof(dev_t));

    link_initialize(&dev->link);

    if(unlikely(dev == NULL))
        return NULL;

    dev->pci_dev.bus      = bus;
    dev->pci_dev.devfn    = devfn;
    dev->pci_dev.hdr_type = hdr & 0x7f;
    dev->pci_dev.multifunction    = !!(hdr & 0x80);
    dev->pci_dev.vendor   = id & 0xffff;
    dev->pci_dev.device   = (id >> 16) & 0xffff;

    pci_setup_device(&dev->pci_dev);

    return dev;

};

int pci_scan_slot(u32_t bus, int devfn)
{
    int  func, nr = 0;

    for (func = 0; func < 8; func++, devfn++)
    {
        dev_t  *dev;

        dev = pci_scan_device(bus, devfn);
        if( dev )
        {
            list_append(&dev->link, &devices);

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


void pci_scan_bus(u32_t bus)
{
    u32_t devfn;
    dev_t *dev;


    for (devfn = 0; devfn < 0x100; devfn += 8)
        pci_scan_slot(bus, devfn);

}

int enum_pci_devices()
{
    dev_t  *dev;
    u32_t   last_bus;
    u32_t   bus = 0 , devfn = 0;

    list_initialize(&devices);

    last_bus = PciApi(1);


    if( unlikely(last_bus == -1))
        return -1;

    for(;bus <= last_bus; bus++)
        pci_scan_bus(bus);

//    for(dev = (dev_t*)devices.next;
//        &dev->link != &devices;
//        dev = (dev_t*)dev->link.next)
//    {
//        dbgprintf("PCI device %x:%x bus:%x devfn:%x\n",
//                dev->pci_dev.vendor,
//                dev->pci_dev.device,
//                dev->pci_dev.bus,
//                dev->pci_dev.devfn);
//
//    }
    return 0;
}

#if 0
/**
 * pci_set_power_state - Set the power state of a PCI device
 * @dev: PCI device to be suspended
 * @state: PCI power state (D0, D1, D2, D3hot, D3cold) we're entering
 *
 * Transition a device to a new power state, using the Power Management
 * Capabilities in the device's config space.
 *
 * RETURN VALUE:
 * -EINVAL if trying to enter a lower state than we're already in.
 * 0 if we're already in the requested state.
 * -EIO if device does not support PCI PM.
 * 0 if we can successfully change the power state.
 */
int
pci_set_power_state(struct pci_dev *dev, pci_power_t state)
{
        int pm, need_restore = 0;
        u16 pmcsr, pmc;

        /* bound the state we're entering */
        if (state > PCI_D3hot)
                state = PCI_D3hot;

        /*
         * If the device or the parent bridge can't support PCI PM, ignore
         * the request if we're doing anything besides putting it into D0
         * (which would only happen on boot).
         */
        if ((state == PCI_D1 || state == PCI_D2) && pci_no_d1d2(dev))
                return 0;

        /* find PCI PM capability in list */
        pm = pci_find_capability(dev, PCI_CAP_ID_PM);

        /* abort if the device doesn't support PM capabilities */
        if (!pm)
                return -EIO;

        /* Validate current state:
         * Can enter D0 from any state, but if we can only go deeper
         * to sleep if we're already in a low power state
         */
        if (state != PCI_D0 && dev->current_state > state) {
                printk(KERN_ERR "%s(): %s: state=%d, current state=%d\n",
                        __FUNCTION__, pci_name(dev), state, dev->current_state);
                return -EINVAL;
        } else if (dev->current_state == state)
                return 0;        /* we're already there */


        pci_read_config_word(dev,pm + PCI_PM_PMC,&pmc);
        if ((pmc & PCI_PM_CAP_VER_MASK) > 3) {
                printk(KERN_DEBUG
                       "PCI: %s has unsupported PM cap regs version (%u)\n",
                       pci_name(dev), pmc & PCI_PM_CAP_VER_MASK);
                return -EIO;
        }

        /* check if this device supports the desired state */
        if (state == PCI_D1 && !(pmc & PCI_PM_CAP_D1))
                return -EIO;
        else if (state == PCI_D2 && !(pmc & PCI_PM_CAP_D2))
                return -EIO;

        pci_read_config_word(dev, pm + PCI_PM_CTRL, &pmcsr);

        /* If we're (effectively) in D3, force entire word to 0.
         * This doesn't affect PME_Status, disables PME_En, and
         * sets PowerState to 0.
         */
        switch (dev->current_state) {
        case PCI_D0:
        case PCI_D1:
        case PCI_D2:
                pmcsr &= ~PCI_PM_CTRL_STATE_MASK;
                pmcsr |= state;
                break;
        case PCI_UNKNOWN: /* Boot-up */
                if ((pmcsr & PCI_PM_CTRL_STATE_MASK) == PCI_D3hot
                 && !(pmcsr & PCI_PM_CTRL_NO_SOFT_RESET))
                        need_restore = 1;
                /* Fall-through: force to D0 */
        default:
                pmcsr = 0;
                break;
        }

        /* enter specified state */
        pci_write_config_word(dev, pm + PCI_PM_CTRL, pmcsr);

        /* Mandatory power management transition delays */
        /* see PCI PM 1.1 5.6.1 table 18 */
        if (state == PCI_D3hot || dev->current_state == PCI_D3hot)
                msleep(pci_pm_d3_delay);
        else if (state == PCI_D2 || dev->current_state == PCI_D2)
                udelay(200);

        /*
         * Give firmware a chance to be called, such as ACPI _PRx, _PSx
         * Firmware method after native method ?
         */
        if (platform_pci_set_power_state)
                platform_pci_set_power_state(dev, state);

        dev->current_state = state;

        /* According to section 5.4.1 of the "PCI BUS POWER MANAGEMENT
         * INTERFACE SPECIFICATION, REV. 1.2", a device transitioning
         * from D3hot to D0 _may_ perform an internal reset, thereby
         * going to "D0 Uninitialized" rather than "D0 Initialized".
         * For example, at least some versions of the 3c905B and the
         * 3c556B exhibit this behaviour.
         *
         * At least some laptop BIOSen (e.g. the Thinkpad T21) leave
         * devices in a D3hot state at boot.  Consequently, we need to
         * restore at least the BARs so that the device will be
         * accessible to its driver.
         */
        if (need_restore)
                pci_restore_bars(dev);

        return 0;
}
#endif

int pcibios_enable_resources(struct pci_dev *dev, int mask)
{
    u16_t cmd, old_cmd;
    int  idx;
    struct resource *r;

    cmd = PciRead16(dev->bus, dev->devfn, PCI_COMMAND);
    old_cmd = cmd;
    for (idx = 0; idx < PCI_NUM_RESOURCES; idx++)
    {
        /* Only set up the requested stuff */
        if (!(mask & (1 << idx)))
                continue;

        r = &dev->resource[idx];
        if (!(r->flags & (IORESOURCE_IO | IORESOURCE_MEM)))
                continue;
        if ((idx == PCI_ROM_RESOURCE) &&
                        (!(r->flags & IORESOURCE_ROM_ENABLE)))
                continue;
        if (!r->start && r->end) {
                printk(KERN_ERR "PCI: Device %s not available "
                        "because of resource %d collisions\n",
                        pci_name(dev), idx);
                return -EINVAL;
        }
        if (r->flags & IORESOURCE_IO)
                cmd |= PCI_COMMAND_IO;
        if (r->flags & IORESOURCE_MEM)
                cmd |= PCI_COMMAND_MEMORY;
    }
    if (cmd != old_cmd) {
        printk("PCI: Enabling device %s (%04x -> %04x)\n",
                pci_name(dev), old_cmd, cmd);
        PciWrite16(dev->bus, dev->devfn, PCI_COMMAND, cmd);
    }
    return 0;
}


int pcibios_enable_device(struct pci_dev *dev, int mask)
{
        int err;

        if ((err = pcibios_enable_resources(dev, mask)) < 0)
                return err;

//        if (!dev->msi_enabled)
//                return pcibios_enable_irq(dev);
        return 0;
}


static int do_pci_enable_device(struct pci_dev *dev, int bars)
{
        int err;

//        err = pci_set_power_state(dev, PCI_D0);
//        if (err < 0 && err != -EIO)
//                return err;
        err = pcibios_enable_device(dev, bars);
//        if (err < 0)
//                return err;
//        pci_fixup_device(pci_fixup_enable, dev);

        return 0;
}


static int __pci_enable_device_flags(struct pci_dev *dev,
                                     resource_size_t flags)
{
        int err;
        int i, bars = 0;

//        if (atomic_add_return(1, &dev->enable_cnt) > 1)
//                return 0;               /* already enabled */

        for (i = 0; i < DEVICE_COUNT_RESOURCE; i++)
                if (dev->resource[i].flags & flags)
                        bars |= (1 << i);

        err = do_pci_enable_device(dev, bars);
//        if (err < 0)
//                atomic_dec(&dev->enable_cnt);
        return err;
}


/**
 * pci_enable_device - Initialize device before it's used by a driver.
 * @dev: PCI device to be initialized
 *
 *  Initialize device before it's used by a driver. Ask low-level code
 *  to enable I/O and memory. Wake up the device if it was suspended.
 *  Beware, this function can fail.
 *
 *  Note we don't actually enable the device many times if we call
 *  this function repeatedly (we just increment the count).
 */
int pci_enable_device(struct pci_dev *dev)
{
        return __pci_enable_device_flags(dev, IORESOURCE_MEM | IORESOURCE_IO);
}



struct pci_device_id* find_pci_device(dev_t* pdev, struct pci_device_id *idlist)
{
    dev_t *dev;
    struct pci_device_id *ent;

    for(dev = (dev_t*)devices.next;
        &dev->link != &devices;
        dev = (dev_t*)dev->link.next)
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



/**
 * pci_map_rom - map a PCI ROM to kernel space
 * @pdev: pointer to pci device struct
 * @size: pointer to receive size of pci window over ROM
 * @return: kernel virtual pointer to image of ROM
 *
 * Map a PCI ROM into kernel space. If ROM is boot video ROM,
 * the shadow BIOS copy will be returned instead of the
 * actual ROM.
 */

#define legacyBIOSLocation 0xC0000
#define OS_BASE   0x80000000

void *pci_map_rom(struct pci_dev *pdev, size_t *size)
{
    struct resource *res = &pdev->resource[PCI_ROM_RESOURCE];
    u32_t start;
    void  *rom;

#if 0
    /*
     * IORESOURCE_ROM_SHADOW set on x86, x86_64 and IA64 supports legacy
     * memory map if the VGA enable bit of the Bridge Control register is
     * set for embedded VGA.
     */
    if (res->flags & IORESOURCE_ROM_SHADOW) {
        /* primary video rom always starts here */
        start = (u32_t)0xC0000;
        *size = 0x20000; /* cover C000:0 through E000:0 */
    } else {
        if (res->flags & (IORESOURCE_ROM_COPY | IORESOURCE_ROM_BIOS_COPY)) {
            *size = pci_resource_len(pdev, PCI_ROM_RESOURCE);
             return (void *)(unsigned long)
                     pci_resource_start(pdev, PCI_ROM_RESOURCE);
        } else {
                /* assign the ROM an address if it doesn't have one */
            //if (res->parent == NULL &&
            //     pci_assign_resource(pdev,PCI_ROM_RESOURCE))
            //         return NULL;
             start = pci_resource_start(pdev, PCI_ROM_RESOURCE);
             *size = pci_resource_len(pdev, PCI_ROM_RESOURCE);
             if (*size == 0)
                     return NULL;

             /* Enable ROM space decodes */
             if (pci_enable_rom(pdev))
                     return NULL;
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
    *size = pci_get_rom_size(rom, *size);

#endif

    unsigned char tmp[32];
    rom = NULL;

    dbgprintf("Getting BIOS copy from legacy VBIOS location\n");
    memcpy(tmp,(char*)(OS_BASE+legacyBIOSLocation), 32);
    *size = tmp[2] * 512;
    if (*size > 0x10000 )
    {
        *size = 0;
        dbgprintf("Invalid BIOS length field\n");
    }
    else
        rom = (void*)( OS_BASE+legacyBIOSLocation);

    return rom;
}

