
#include <ddk.h>
#include <linux/errno.h>
#include <mutex.h>
#include <pci.h>
#include <syscall.h>

#include "acpi.h"
#include "acpi_bus.h"


#define PREFIX "ACPI: "


struct acpi_handle_node {
    struct list_head node;
    ACPI_HANDLE handle;
};

static const struct acpi_device_ids root_device_ids[] = {
    {"PNP0A03", 0},
    {"", 0},
};

static LIST_HEAD(acpi_pci_roots);


/**
 * acpi_is_root_bridge - determine whether an ACPI CA node is a PCI root bridge
 * @handle - the ACPI CA node in question.
 *
 * Note: we could make this API take a struct acpi_device * instead, but
 * for now, it's more convenient to operate on an acpi_handle.
 */
int acpi_is_root_bridge(ACPI_HANDLE handle)
{
    int ret;
    struct acpi_device *device;

    ret = acpi_bus_get_device(handle, &device);
    if (ret)
        return 0;

    ret = acpi_match_device_ids(device, root_device_ids);
    if (ret)
        return 0;
    else
        return 1;
}


struct acpi_pci_root *acpi_pci_find_root(ACPI_HANDLE handle)
{
    struct acpi_pci_root *root;

    list_for_each_entry(root, &acpi_pci_roots, node) {
        if (root->device->handle == handle)
            return root;
    }
    return NULL;
}


/**
 * acpi_get_pci_dev - convert ACPI CA handle to struct pci_dev
 * @handle: the handle in question
 *
 * Given an ACPI CA handle, the desired PCI device is located in the
 * list of PCI devices.
 *
 * If the device is found, its reference count is increased and this
 * function returns a pointer to its data structure.  The caller must
 * decrement the reference count by calling pci_dev_put().
 * If no device is found, %NULL is returned.
 */
struct pci_dev *acpi_get_pci_dev(ACPI_HANDLE handle)
{
    int dev, fn;
    unsigned long long adr;
    ACPI_STATUS status;
    ACPI_HANDLE phandle;
    struct pci_bus *pbus;
    struct pci_dev *pdev = NULL;
    struct acpi_handle_node *node, *tmp;
    struct acpi_pci_root *root;
    LIST_HEAD(device_list);

    /*
     * Walk up the ACPI CA namespace until we reach a PCI root bridge.
     */
    phandle = handle;
    while (!acpi_is_root_bridge(phandle)) {
        node = kzalloc(sizeof(struct acpi_handle_node), GFP_KERNEL);
        if (!node)
            goto out;

        INIT_LIST_HEAD(&node->node);
        node->handle = phandle;
        list_add(&node->node, &device_list);

        status = AcpiGetParent(phandle, &phandle);
        if (ACPI_FAILURE(status))
            goto out;
    }

    root = acpi_pci_find_root(phandle);
    if (!root)
        goto out;

    pbus = root->bus;

    /*
     * Now, walk back down the PCI device tree until we return to our
     * original handle. Assumes that everything between the PCI root
     * bridge and the device we're looking for must be a P2P bridge.
     */
    list_for_each_entry(node, &device_list, node) {
        ACPI_HANDLE hnd = node->handle;
        status = acpi_evaluate_integer(hnd, "_ADR", NULL, &adr);
        if (ACPI_FAILURE(status))
            goto out;
        dev = (adr >> 16) & 0xffff;
        fn  = adr & 0xffff;

        pdev = pci_get_slot(pbus, PCI_DEVFN(dev, fn));
        if (!pdev || hnd == handle)
            break;

        pbus = pdev->subordinate;
//        pci_dev_put(pdev);

        /*
         * This function may be called for a non-PCI device that has a
         * PCI parent (eg. a disk under a PCI SATA controller).  In that
         * case pdev->subordinate will be NULL for the parent.
         */
        if (!pbus) {
            dbgprintf("Not a PCI-to-PCI bridge\n");
            pdev = NULL;
            break;
        }
    }
out:
    list_for_each_entry_safe(node, tmp, &device_list, node)
        kfree(node);

    return pdev;
}

