
#include <ddk.h>
#include <linux/errno.h>
#include <mutex.h>
#include <pci.h>
#include <syscall.h>

LIST_HEAD(pci_root_buses);

#define IO_SPACE_LIMIT 0xffff

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


static inline int pci_domain_nr(struct pci_bus *bus)
{
    struct pci_sysdata *sd = bus->sysdata;
    return sd->domain;
}

static struct pci_bus * pci_alloc_bus(void)
{
    struct pci_bus *b;

    b = kzalloc(sizeof(*b), GFP_KERNEL);
    if (b) {
        INIT_LIST_HEAD(&b->node);
        INIT_LIST_HEAD(&b->children);
        INIT_LIST_HEAD(&b->devices);
        INIT_LIST_HEAD(&b->slots);
        INIT_LIST_HEAD(&b->resources);
    }
    return b;
}

struct pci_bus * pci_create_bus(int bus, struct pci_ops *ops, void *sysdata)
{
    int error;
    struct pci_bus *b, *b2;

    b = pci_alloc_bus();
    if (!b)
        return NULL;

    b->sysdata = sysdata;
    b->ops = ops;

    b2 = pci_find_bus(pci_domain_nr(b), bus);
    if (b2) {
        /* If we already got to this bus through a different bridge, ignore it */
        dbgprintf("bus already known\n");
        goto err_out;
    }

//    down_write(&pci_bus_sem);
    list_add_tail(&b->node, &pci_root_buses);
//    up_write(&pci_bus_sem);

    b->number = b->secondary = bus;
    b->resource[0] = &ioport_resource;
    b->resource[1] = &iomem_resource;

    return b;

err_out:
    kfree(b);
    return NULL;
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


