
#include <ddk.h>
#include <linux/errno.h>
#include <mutex.h>
#include <pci.h>
#include <syscall.h>

#include "acpi.h"
#include "acpi_bus.h"


#define PREFIX "ACPI: "


static int acpi_pci_unbind(struct acpi_device *device)
{
    struct pci_dev *dev;

    dev = acpi_get_pci_dev(device->handle);
    if (!dev)
        goto out;

//    device_set_run_wake(&dev->dev, false);
//    pci_acpi_remove_pm_notifier(device);

    if (!dev->subordinate)
        goto out;

//    acpi_pci_irq_del_prt(dev->subordinate);

    device->ops.bind = NULL;
    device->ops.unbind = NULL;

out:
//    pci_dev_put(dev);
    return 0;
}

static int acpi_pci_bind(struct acpi_device *device)
{
    ACPI_STATUS status;
    ACPI_HANDLE handle;
    struct pci_bus *bus;
    struct pci_dev *dev;

    dev = acpi_get_pci_dev(device->handle);
    if (!dev)
        return 0;

//    pci_acpi_add_pm_notifier(device, dev);
//    if (device->wakeup.flags.run_wake)
//        device_set_run_wake(&dev->dev, true);

    /*
     * Install the 'bind' function to facilitate callbacks for
     * children of the P2P bridge.
     */
    if (dev->subordinate) {
        ACPI_DEBUG_PRINT((ACPI_DB_INFO,
                  "Device %04x:%02x:%02x.%d is a PCI bridge\n",
                  pci_domain_nr(dev->bus), dev->bus->number,
                  PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn)));
        device->ops.bind = acpi_pci_bind;
        device->ops.unbind = acpi_pci_unbind;
    }

    /*
     * Evaluate and parse _PRT, if exists.  This code allows parsing of
     * _PRT objects within the scope of non-bridge devices.  Note that
     * _PRTs within the scope of a PCI bridge assume the bridge's
     * subordinate bus number.
     *
     * TBD: Can _PRTs exist within the scope of non-bridge PCI devices?
     */
    status = AcpiGetHandle(device->handle, METHOD_NAME__PRT, &handle);
    if (ACPI_FAILURE(status))
        goto out;

    if (dev->subordinate)
        bus = dev->subordinate;
    else
        bus = dev->bus;

    acpi_pci_irq_add_prt(device->handle, bus);

out:
//    pci_dev_put(dev);
    return 0;
}

int acpi_pci_bind_root(struct acpi_device *device)
{
    device->ops.bind = acpi_pci_bind;
    device->ops.unbind = acpi_pci_unbind;

    return 0;
}

