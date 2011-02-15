
#include <ddk.h>
#include <linux/errno.h>
#include <mutex.h>
#include <linux/spinlock.h>
#include <pci.h>
#include <syscall.h>

#include "acpi.h"
#include "acpi_bus.h"

#define PREFIX "ACPI: "

struct acpi_prt_entry
{
    struct list_head    list;
    ACPI_PCI_ID         id;
    u8                  pin;
    ACPI_HANDLE         link;
    u32                 index;      /* GSI, or link _CRS index */
};

static LIST_HEAD(acpi_prt_list);
static DEFINE_SPINLOCK(acpi_prt_lock);

static inline char pin_name(int pin)
{
    return 'A' + pin - 1;
}


/* --------------------------------------------------------------------------
                         PCI IRQ Routing Table (PRT) Support
   -------------------------------------------------------------------------- */

static struct acpi_prt_entry *acpi_pci_irq_find_prt_entry(struct pci_dev *dev,
                              int pin)
{
    struct acpi_prt_entry *entry;
    int segment = pci_domain_nr(dev->bus);
    int bus = dev->bus->number;
    int device = PCI_SLOT(dev->devfn);

    spin_lock(&acpi_prt_lock);
    list_for_each_entry(entry, &acpi_prt_list, list) {
        if ((segment == entry->id.Segment)
            && (bus == entry->id.Bus)
            && (device == entry->id.Device)
            && (pin == entry->pin)) {
            spin_unlock(&acpi_prt_lock);
            return entry;
        }
    }
    spin_unlock(&acpi_prt_lock);
    return NULL;
}

static int acpi_pci_irq_add_entry(ACPI_HANDLE handle, struct pci_bus *bus,
                  struct acpi_pci_routing_table *prt)
{
    struct acpi_prt_entry *entry;

    entry = kzalloc(sizeof(struct acpi_prt_entry), GFP_KERNEL);
    if (!entry)
        return -ENOMEM;

    /*
     * Note that the _PRT uses 0=INTA, 1=INTB, etc, while PCI uses
     * 1=INTA, 2=INTB.  We use the PCI encoding throughout, so convert
     * it here.
     */
    entry->id.Segment = pci_domain_nr(bus);
    entry->id.Bus = bus->number;
    entry->id.Device = (prt->Address >> 16) & 0xFFFF;
    entry->pin = prt->Pin + 1;

//    do_prt_fixups(entry, prt);

    entry->index = prt->SourceIndex;

    /*
     * Type 1: Dynamic
     * ---------------
     * The 'source' field specifies the PCI interrupt link device used to
     * configure the IRQ assigned to this slot|dev|pin.  The 'source_index'
     * indicates which resource descriptor in the resource template (of
     * the link device) this interrupt is allocated from.
     *
     * NOTE: Don't query the Link Device for IRQ information at this time
     *       because Link Device enumeration may not have occurred yet
     *       (e.g. exists somewhere 'below' this _PRT entry in the ACPI
     *       namespace).
     */
    if (prt->Source[0])
        AcpiGetHandle(handle, prt->Source, &entry->link);

    /*
     * Type 2: Static
     * --------------
     * The 'source' field is NULL, and the 'source_index' field specifies
     * the IRQ value, which is hardwired to specific interrupt inputs on
     * the interrupt controller.
     */

    dbgprintf(PREFIX "      %04x:%02x:%02x[%c] -> %s[%d]\n",
                  entry->id.Segment, entry->id.Bus,
                  entry->id.Device, pin_name(entry->pin),
                  prt->Source, entry->index);

    spin_lock(&acpi_prt_lock);
    list_add_tail(&entry->list, &acpi_prt_list);
    spin_unlock(&acpi_prt_lock);

    return 0;
}



int acpi_pci_irq_add_prt(ACPI_HANDLE handle, struct pci_bus *bus)
{
    ACPI_STATUS status;
    struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
    struct acpi_pci_routing_table *entry;

    /* 'handle' is the _PRT's parent (root bridge or PCI-PCI bridge) */
    status = AcpiGetName(handle, ACPI_FULL_PATHNAME, &buffer);
    if (ACPI_FAILURE(status))
        return -ENODEV;

    printk(KERN_DEBUG "ACPI: PCI Interrupt Routing Table [%s._PRT]\n",
           (char *) buffer.Pointer);

    kfree(buffer.Pointer);

    buffer.Length = ACPI_ALLOCATE_BUFFER;
    buffer.Pointer = NULL;

    status = AcpiGetIrqRoutingTable(handle, &buffer);
    if (ACPI_FAILURE(status))
    {
        dbgprintf("AcpiGetIrqRoutingTable failed "
                  "evaluating _PRT [%s]\n",AcpiFormatException(status));
        kfree(buffer.Pointer);
        return -ENODEV;
    }

    entry = buffer.Pointer;
    while (entry && (entry->Length > 0)) {
        acpi_pci_irq_add_entry(handle, bus, entry);
        entry = (struct acpi_pci_routing_table *)
            ((unsigned long)entry + entry->Length);
    }

    kfree(buffer.Pointer);
    return 0;
}

static struct acpi_prt_entry *acpi_pci_irq_lookup(struct pci_dev *dev, int pin)
{
    struct acpi_prt_entry *entry;
    struct pci_dev *bridge;
    u8 bridge_pin, orig_pin = pin;

    entry = acpi_pci_irq_find_prt_entry(dev, pin);
    if (entry) {
        ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Found %s[%c] _PRT entry\n",
                  pci_name(dev), pin_name(pin)));
        return entry;
    }

    /*
     * Attempt to derive an IRQ for this device from a parent bridge's
     * PCI interrupt routing entry (eg. yenta bridge and add-in card bridge).
     */
    bridge = dev->bus->self;
    while (bridge)
    {
        pin = pci_swizzle_interrupt_pin(dev, pin);

        if ((bridge->class >> 8) == PCI_CLASS_BRIDGE_CARDBUS) {
            /* PC card has the same IRQ as its cardbridge */
            bridge_pin = bridge->pin;
            if (!bridge_pin) {
                ACPI_DEBUG_PRINT((ACPI_DB_INFO,
                          "No interrupt pin configured for device %s\n",
                          pci_name(bridge)));
                return NULL;
            }
            pin = bridge_pin;
        }

        entry = acpi_pci_irq_find_prt_entry(bridge, pin);
        if (entry) {
            ACPI_DEBUG_PRINT((ACPI_DB_INFO,
                     "Derived GSI for %s INT %c from %s\n",
                     pci_name(dev), pin_name(orig_pin),
                     pci_name(bridge)));
            return entry;
        }

        dev = bridge;
        bridge = dev->bus->self;
    }

    dbgprintf("can't derive routing for PCI INT %c\n",
         pin_name(orig_pin));
    return NULL;
}


int acpi_get_irq(struct pci_dev *dev)
{
    struct acpi_prt_entry *entry;
    int gsi = -1;
    u8 pin;

    int triggering = ACPI_LEVEL_SENSITIVE;
    int polarity = ACPI_ACTIVE_LOW;

    char *link = NULL;
    char link_desc[16];
    int rc;

    pin = dev->pin;

    if ( !pin )
    {
        dbgprintf("No interrupt pin configured for device %s\n",
                  pci_name(dev));
        return 0;
    }

    entry = acpi_pci_irq_lookup(dev, pin);
    if (!entry) {
        /*
         * IDE legacy mode controller IRQs are magic. Why do compat
         * extensions always make such a nasty mess.
         */
        if (dev->class >> 8 == PCI_CLASS_STORAGE_IDE &&
                (dev->class & 0x05) == 0)
            return 0;
    }

    if (entry)
    {
        if (entry->link)
        {
            gsi = acpi_pci_link_allocate_irq(entry->link,
                             entry->index,
                             &triggering, &polarity,
                             &link);
//            dbgprintf("link not implemen\n");
        }
        else
            gsi = entry->index;
    } else
        gsi = -1;

#if 0

    /*
     * No IRQ known to the ACPI subsystem - maybe the BIOS /
     * driver reported one, then use it. Exit in any case.
     */
    if (gsi < 0) {
        u32 dev_gsi;
        dev_warn(&dev->dev, "PCI INT %c: no GSI", pin_name(pin));
        /* Interrupt Line values above 0xF are forbidden */
        if (dev->irq > 0 && (dev->irq <= 0xF) &&
            (acpi_isa_irq_to_gsi(dev->irq, &dev_gsi) == 0)) {
            printk(" - using ISA IRQ %d\n", dev->irq);
            acpi_register_gsi(&dev->dev, dev_gsi,
                      ACPI_LEVEL_SENSITIVE,
                      ACPI_ACTIVE_LOW);
            return 0;
        } else {
            printk("\n");
            return 0;
        }
    }

    rc = acpi_register_gsi(&dev->dev, gsi, triggering, polarity);
    if (rc < 0) {
        dev_warn(&dev->dev, "PCI INT %c: failed to register GSI\n",
             pin_name(pin));
        return rc;
    }
    dev->irq = rc;

    if (link)
        snprintf(link_desc, sizeof(link_desc), " -> Link[%s]", link);
    else
        link_desc[0] = '\0';

    dev_info(&dev->dev, "PCI INT %c%s -> GSI %u (%s, %s) -> IRQ %d\n",
         pin_name(pin), link_desc, gsi,
         (triggering == ACPI_LEVEL_SENSITIVE) ? "level" : "edge",
         (polarity == ACPI_ACTIVE_LOW) ? "low" : "high", dev->irq);
#endif
    return gsi;
}


#define ACPI_PCI_LINK_MAX_POSSIBLE  16

/*
 * If a link is initialized, we never change its active and initialized
 * later even the link is disable. Instead, we just repick the active irq
 */
struct acpi_pci_link_irq {
    u8 active;      /* Current IRQ */
    u8 triggering;      /* All IRQs */
    u8 polarity;        /* All IRQs */
    u8 resource_type;
    u8 possible_count;
    u8 possible[ACPI_PCI_LINK_MAX_POSSIBLE];
    u8 initialized:1;
    u8 reserved:7;
};

struct acpi_pci_link {
    struct list_head        list;
    struct acpi_device      *device;
    struct acpi_pci_link_irq    irq;
    int             refcnt;
};

static LIST_HEAD(acpi_link_list);
static DEFINE_MUTEX(acpi_link_lock);


static int acpi_pci_link_set(struct acpi_pci_link *link, int irq)
{
    int result;
    ACPI_STATUS status;
    struct {
        struct acpi_resource res;
        struct acpi_resource end;
    } *resource;

    ACPI_BUFFER buffer = { 0, NULL };

    if (!irq)
        return -EINVAL;

    resource = kzalloc(sizeof(*resource) + 1, GFP_KERNEL);
    if (!resource)
        return -ENOMEM;

    buffer.Length = sizeof(*resource) + 1;
    buffer.Pointer = resource;

    switch (link->irq.resource_type) {
    case ACPI_RESOURCE_TYPE_IRQ:
        resource->res.Type = ACPI_RESOURCE_TYPE_IRQ;
        resource->res.Length = sizeof(struct acpi_resource);
        resource->res.Data.Irq.Triggering = link->irq.triggering;
        resource->res.Data.Irq.Polarity =
            link->irq.polarity;
        if (link->irq.triggering == ACPI_EDGE_SENSITIVE)
            resource->res.Data.Irq.Sharable =
                ACPI_EXCLUSIVE;
        else
            resource->res.Data.Irq.Sharable = ACPI_SHARED;
        resource->res.Data.Irq.InterruptCount = 1;
        resource->res.Data.Irq.Interrupts[0] = irq;
        break;

    case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
        resource->res.Type = ACPI_RESOURCE_TYPE_EXTENDED_IRQ;
        resource->res.Length = sizeof(struct acpi_resource);
        resource->res.Data.ExtendedIrq.ProducerConsumer =
            ACPI_CONSUMER;
        resource->res.Data.ExtendedIrq.Triggering =
            link->irq.triggering;
        resource->res.Data.ExtendedIrq.Polarity =
            link->irq.polarity;
        if (link->irq.triggering == ACPI_EDGE_SENSITIVE)
            resource->res.Data.Irq.Sharable =
                ACPI_EXCLUSIVE;
        else
            resource->res.Data.Irq.Sharable = ACPI_SHARED;
        resource->res.Data.ExtendedIrq.InterruptCount = 1;
        resource->res.Data.ExtendedIrq.Interrupts[0] = irq;
        /* ignore resource_source, it's optional */
        break;
    default:
        printk(KERN_ERR PREFIX "Invalid Resource_type %d\n", link->irq.resource_type);
        result = -EINVAL;
        goto end;

    }
    resource->end.Type = ACPI_RESOURCE_TYPE_END_TAG;

#if 0
    /* Attempt to set the resource */
    status = acpi_set_current_resources(link->device->handle, &buffer);

    /* check for total failure */
    if (ACPI_FAILURE(status)) {
        dbgprintf("%s failure Evaluating _SRS", __FUNCTION__);
        result = -ENODEV;
        goto end;
    }

    /* Query _STA, set device->status */
    result = acpi_bus_get_status(link->device);
    if (result) {
        printk(KERN_ERR PREFIX "Unable to read status\n");
        goto end;
    }
    if (!link->device->status.enabled) {
        printk(KERN_WARNING PREFIX
                  "%s [%s] disabled and referenced, BIOS bug\n",
                  acpi_device_name(link->device),
                  acpi_device_bid(link->device));
    }

    /* Query _CRS, set link->irq.active */
    result = acpi_pci_link_get_current(link);
    if (result) {
        goto end;
    }

    /*
     * Is current setting not what we set?
     * set link->irq.active
     */
    if (link->irq.active != irq) {
        /*
         * policy: when _CRS doesn't return what we just _SRS
         * assume _SRS worked and override _CRS value.
         */
        printk(KERN_WARNING PREFIX
                  "%s [%s] BIOS reported IRQ %d, using IRQ %d\n",
                  acpi_device_name(link->device),
                  acpi_device_bid(link->device), link->irq.active, irq);
        link->irq.active = irq;
    }
#endif

    ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Set IRQ %d\n", link->irq.active));

end:
    kfree(resource);
    return result;
}



#define ACPI_MAX_IRQS       256
#define ACPI_MAX_ISA_IRQ    16

#define PIRQ_PENALTY_PCI_AVAILABLE  (0)
#define PIRQ_PENALTY_PCI_POSSIBLE   (16*16)
#define PIRQ_PENALTY_PCI_USING      (16*16*16)
#define PIRQ_PENALTY_ISA_TYPICAL    (16*16*16*16)
#define PIRQ_PENALTY_ISA_USED       (16*16*16*16*16)
#define PIRQ_PENALTY_ISA_ALWAYS     (16*16*16*16*16*16)

static int acpi_irq_penalty[ACPI_MAX_IRQS] = {
    PIRQ_PENALTY_ISA_ALWAYS,    /* IRQ0 timer */
    PIRQ_PENALTY_ISA_ALWAYS,    /* IRQ1 keyboard */
    PIRQ_PENALTY_ISA_ALWAYS,    /* IRQ2 cascade */
    PIRQ_PENALTY_ISA_TYPICAL,   /* IRQ3 serial */
    PIRQ_PENALTY_ISA_TYPICAL,   /* IRQ4 serial */
    PIRQ_PENALTY_ISA_TYPICAL,   /* IRQ5 sometimes SoundBlaster */
    PIRQ_PENALTY_ISA_TYPICAL,   /* IRQ6 */
    PIRQ_PENALTY_ISA_TYPICAL,   /* IRQ7 parallel, spurious */
    PIRQ_PENALTY_ISA_TYPICAL,   /* IRQ8 rtc, sometimes */
    PIRQ_PENALTY_PCI_AVAILABLE, /* IRQ9  PCI, often acpi */
    PIRQ_PENALTY_PCI_AVAILABLE, /* IRQ10 PCI */
    PIRQ_PENALTY_PCI_AVAILABLE, /* IRQ11 PCI */
    PIRQ_PENALTY_ISA_USED,      /* IRQ12 mouse */
    PIRQ_PENALTY_ISA_USED,      /* IRQ13 fpe, sometimes */
    PIRQ_PENALTY_ISA_USED,      /* IRQ14 ide0 */
    PIRQ_PENALTY_ISA_USED,      /* IRQ15 ide1 */
    /* >IRQ15 */
};




static int acpi_irq_balance = 0;

static int acpi_pci_link_allocate(struct acpi_pci_link *link)
{
    int irq;
    int i;

    if (link->irq.initialized) {
        if (link->refcnt == 0)
            /* This means the link is disabled but initialized */
            acpi_pci_link_set(link, link->irq.active);
        return 0;
    }

    /*
     * search for active IRQ in list of possible IRQs.
     */
    for (i = 0; i < link->irq.possible_count; ++i) {
        if (link->irq.active == link->irq.possible[i])
            break;
    }
    /*
     * forget active IRQ that is not in possible list
     */
    if (i == link->irq.possible_count) {
        dbgprintf(KERN_WARNING PREFIX "_CRS %d not found"
                      " in _PRS\n", link->irq.active);
        link->irq.active = 0;
    }

    /*
     * if active found, use it; else pick entry from end of possible list.
     */
    if (link->irq.active)
        irq = link->irq.active;
    else
        irq = link->irq.possible[link->irq.possible_count - 1];

    if (acpi_irq_balance || !link->irq.active) {
        /*
         * Select the best IRQ.  This is done in reverse to promote
         * the use of IRQs 9, 10, 11, and >15.
         */
        for (i = (link->irq.possible_count - 1); i >= 0; i--) {
            if (acpi_irq_penalty[irq] >
                acpi_irq_penalty[link->irq.possible[i]])
                irq = link->irq.possible[i];
        }
    }

    /* Attempt to enable the link device at this IRQ. */
    if (acpi_pci_link_set(link, irq)) {
        printk(KERN_ERR PREFIX "Unable to set IRQ for %s [%s]. "
                "Try pci=noacpi or acpi=off\n",
                acpi_device_name(link->device),
                acpi_device_bid(link->device));
        return -ENODEV;
    } else {
        acpi_irq_penalty[link->irq.active] += PIRQ_PENALTY_PCI_USING;
        printk(KERN_WARNING PREFIX "%s [%s] enabled at IRQ %d\n",
               acpi_device_name(link->device),
               acpi_device_bid(link->device), link->irq.active);
    }

    link->irq.initialized = 1;
    return 0;
}


/*
 * acpi_pci_link_allocate_irq
 * success: return IRQ >= 0
 * failure: return -1
 */
int acpi_pci_link_allocate_irq(ACPI_HANDLE handle, int index,
                               int *triggering, int *polarity, char **name)
{
    int result;
    struct acpi_device *device;
    struct acpi_pci_link *link;

    result = acpi_bus_get_device(handle, &device);
    if (result) {
        printk(KERN_ERR PREFIX "Invalid link device\n");
        return -1;
    }

    link = acpi_driver_data(device);
    if (!link) {
        printk(KERN_ERR PREFIX "Invalid link context\n");
        return -1;
    }

    /* TBD: Support multiple index (IRQ) entries per Link Device */
    if (index) {
        printk(KERN_ERR PREFIX "Invalid index %d\n", index);
        return -1;
    }

    mutex_lock(&acpi_link_lock);
    if (acpi_pci_link_allocate(link)) {
        mutex_unlock(&acpi_link_lock);
        return -1;
    }

    if (!link->irq.active) {
        mutex_unlock(&acpi_link_lock);
        printk(KERN_ERR PREFIX "Link active IRQ is 0!\n");
        return -1;
    }
    link->refcnt++;
    mutex_unlock(&acpi_link_lock);

    if (triggering)
        *triggering = link->irq.triggering;
    if (polarity)
        *polarity = link->irq.polarity;
    if (name)
        *name = acpi_device_bid(link->device);
    ACPI_DEBUG_PRINT((ACPI_DB_INFO,
              "Link %s is referenced\n",
              acpi_device_bid(link->device)));
    return (link->irq.active);
}


