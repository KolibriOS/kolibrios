
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

