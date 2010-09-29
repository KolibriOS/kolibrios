
#include <ddk.h>
#include <linux/errno.h>
#include <mutex.h>
#include <pci.h>
#include <syscall.h>


#define IO_SPACE_LIMIT          0xffff
#define PCIBIOS_SUCCESSFUL      0x00

struct resource ioport_resource = {
    .name   = "PCI IO",
    .start  = 0,
    .end    = IO_SPACE_LIMIT,
    .flags  = IORESOURCE_IO,
};

struct resource iomem_resource = {
    .name   = "PCI mem",
    .start  = 0,
    .end    = -1,
    .flags  = IORESOURCE_MEM,
};

#define PCI_FIND_CAP_TTL    48

static int __pci_find_next_cap_ttl(struct pci_bus *bus, unsigned int devfn,
                   u8 pos, int cap, int *ttl)
{
    u8 id;

    while ((*ttl)--) {
        pci_bus_read_config_byte(bus, devfn, pos, &pos);
        if (pos < 0x40)
            break;
        pos &= ~3;
        pci_bus_read_config_byte(bus, devfn, pos + PCI_CAP_LIST_ID,
                     &id);
        if (id == 0xff)
            break;
        if (id == cap)
            return pos;
        pos += PCI_CAP_LIST_NEXT;
    }
    return 0;
}

static int __pci_find_next_cap(struct pci_bus *bus, unsigned int devfn,
                   u8 pos, int cap)
{
    int ttl = PCI_FIND_CAP_TTL;

    return __pci_find_next_cap_ttl(bus, devfn, pos, cap, &ttl);
}
static int __pci_bus_find_cap_start(struct pci_bus *bus,
                    unsigned int devfn, u8 hdr_type)
{
    u16 status;

    pci_bus_read_config_word(bus, devfn, PCI_STATUS, &status);
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


/**
 * pci_find_capability - query for devices' capabilities
 * @dev: PCI device to query
 * @cap: capability code
 *
 * Tell if a device supports a given PCI capability.
 * Returns the address of the requested capability structure within the
 * device's PCI configuration space or 0 in case the device does not
 * support it.  Possible values for @cap:
 *
 *  %PCI_CAP_ID_PM           Power Management
 *  %PCI_CAP_ID_AGP          Accelerated Graphics Port
 *  %PCI_CAP_ID_VPD          Vital Product Data
 *  %PCI_CAP_ID_SLOTID       Slot Identification
 *  %PCI_CAP_ID_MSI          Message Signalled Interrupts
 *  %PCI_CAP_ID_CHSWP        CompactPCI HotSwap
 *  %PCI_CAP_ID_PCIX         PCI-X
 *  %PCI_CAP_ID_EXP          PCI Express
 */
int pci_find_capability(struct pci_dev *dev, int cap)
{
    int pos;

    pos = __pci_bus_find_cap_start(dev->bus, dev->devfn, dev->hdr_type);
    if (pos)
        pos = __pci_find_next_cap(dev->bus, dev->devfn, pos, cap);

    return pos;
}


static struct pci_bus *pci_do_find_bus(struct pci_bus *bus, unsigned char busnr)
{
    struct pci_bus* child;
    struct list_head *tmp;

    if(bus->number == busnr)
        return bus;

    list_for_each(tmp, &bus->children) {
        child = pci_do_find_bus(pci_bus_b(tmp), busnr);
        if(child)
            return child;
    }
    return NULL;
}


/**
 * pci_find_bus - locate PCI bus from a given domain and bus number
 * @domain: number of PCI domain to search
 * @busnr: number of desired PCI bus
 *
 * Given a PCI bus number and domain number, the desired PCI bus is located
 * in the global list of PCI buses.  If the bus is found, a pointer to its
 * data structure is returned.  If no bus is found, %NULL is returned.
 */
struct pci_bus * pci_find_bus(int domain, int busnr)
{
    struct pci_bus *bus = NULL;
    struct pci_bus *tmp_bus;

    while ((bus = pci_find_next_bus(bus)) != NULL)  {
        if (pci_domain_nr(bus) != domain)
            continue;
        tmp_bus = pci_do_find_bus(bus, busnr);
        if (tmp_bus)
            return tmp_bus;
    }
    return NULL;
}

/**
 * pci_find_next_bus - begin or continue searching for a PCI bus
 * @from: Previous PCI bus found, or %NULL for new search.
 *
 * Iterates through the list of known PCI busses.  A new search is
 * initiated by passing %NULL as the @from argument.  Otherwise if
 * @from is not %NULL, searches continue from next device on the
 * global list.
 */
struct pci_bus *
pci_find_next_bus(const struct pci_bus *from)
{
    struct list_head *n;
    struct pci_bus *b = NULL;

//    WARN_ON(in_interrupt());
//    down_read(&pci_bus_sem);
    n = from ? from->node.next : pci_root_buses.next;
    if (n != &pci_root_buses)
        b = pci_bus_b(n);
//    up_read(&pci_bus_sem);
    return b;
}


/**
 * pci_get_slot - locate PCI device for a given PCI slot
 * @bus: PCI bus on which desired PCI device resides
 * @devfn: encodes number of PCI slot in which the desired PCI
 * device resides and the logical device number within that slot
 * in case of multi-function devices.
 *
 * Given a PCI bus and slot/function number, the desired PCI device
 * is located in the list of PCI devices.
 * If the device is found, its reference count is increased and this
 * function returns a pointer to its data structure.  The caller must
 * decrement the reference count by calling pci_dev_put().
 * If no device is found, %NULL is returned.
 */
struct pci_dev * pci_get_slot(struct pci_bus *bus, unsigned int devfn)
{
    struct list_head *tmp;
    struct pci_dev *dev;

//    WARN_ON(in_interrupt());
//    down_read(&pci_bus_sem);

    list_for_each(tmp, &bus->devices) {
        dev = pci_dev_b(tmp);
        if (dev->devfn == devfn)
            goto out;
    }

    dev = NULL;
 out:
//    pci_dev_get(dev);
//    up_read(&pci_bus_sem);
    return dev;
}




/**
 * pci_find_ext_capability - Find an extended capability
 * @dev: PCI device to query
 * @cap: capability code
 *
 * Returns the address of the requested extended capability structure
 * within the device's PCI configuration space or 0 if the device does
 * not support it.  Possible values for @cap:
 *
 *  %PCI_EXT_CAP_ID_ERR     Advanced Error Reporting
 *  %PCI_EXT_CAP_ID_VC      Virtual Channel
 *  %PCI_EXT_CAP_ID_DSN     Device Serial Number
 *  %PCI_EXT_CAP_ID_PWR     Power Budgeting
 */
int pci_find_ext_capability(struct pci_dev *dev, int cap)
{
    u32 header;
    int ttl;
    int pos = PCI_CFG_SPACE_SIZE;

    /* minimum 8 bytes per capability */
    ttl = (PCI_CFG_SPACE_EXP_SIZE - PCI_CFG_SPACE_SIZE) / 8;

    if (dev->cfg_size <= PCI_CFG_SPACE_SIZE)
        return 0;

    if (pci_read_config_dword(dev, pos, &header) != PCIBIOS_SUCCESSFUL)
        return 0;

    /*
     * If we have no capabilities, this is indicated by cap ID,
     * cap version and next pointer all being 0.
     */
    if (header == 0)
        return 0;

    while (ttl-- > 0) {
        if (PCI_EXT_CAP_ID(header) == cap)
            return pos;

        pos = PCI_EXT_CAP_NEXT(header);
        if (pos < PCI_CFG_SPACE_SIZE)
            break;

        if (pci_read_config_dword(dev, pos, &header) != PCIBIOS_SUCCESSFUL)
            break;
    }

    return 0;
}

#if 0

u32 pci_probe = 0;

#define PCI_NOASSIGN_ROMS   0x80000
#define PCI_NOASSIGN_BARS   0x200000

static void pcibios_fixup_device_resources(struct pci_dev *dev)
{
    struct resource *rom_r = &dev->resource[PCI_ROM_RESOURCE];
    struct resource *bar_r;
    int bar;

    if (pci_probe & PCI_NOASSIGN_BARS) {
        /*
        * If the BIOS did not assign the BAR, zero out the
        * resource so the kernel doesn't attmept to assign
        * it later on in pci_assign_unassigned_resources
        */
        for (bar = 0; bar <= PCI_STD_RESOURCE_END; bar++) {
            bar_r = &dev->resource[bar];
            if (bar_r->start == 0 && bar_r->end != 0) {
                bar_r->flags = 0;
                bar_r->end = 0;
            }
        }
    }

    if (pci_probe & PCI_NOASSIGN_ROMS) {
        if (rom_r->parent)
            return;
        if (rom_r->start) {
            /* we deal with BIOS assigned ROM later */
            return;
        }
        rom_r->start = rom_r->end = rom_r->flags = 0;
    }
}

/*
 *  Called after each bus is probed, but before its children
 *  are examined.
 */

void pcibios_fixup_bus(struct pci_bus *b)
{
    struct pci_dev *dev;

    /* root bus? */
//    if (!b->parent)
//        x86_pci_root_bus_res_quirks(b);
    pci_read_bridge_bases(b);
    list_for_each_entry(dev, &b->devices, bus_list)
        pcibios_fixup_device_resources(dev);
}

#endif
