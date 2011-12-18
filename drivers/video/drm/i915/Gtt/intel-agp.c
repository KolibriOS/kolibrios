/*
 * Intel AGPGART routines.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <errno-base.h>
#include <linux/pci.h>

//#include <linux/agp_backend.h>
//#include <asm/smp.h>
#include <linux/spinlock.h>

#include "agp.h"
#include "intel-agp.h"

#include <syscall.h>

#define __devinit
#define PCI_VENDOR_ID_INTEL             0x8086


int intel_gmch_probe(struct pci_dev *pdev,
                      struct agp_bridge_data *bridge);

int intel_agp_enabled;

struct agp_bridge_data *agp_alloc_bridge(void)
{
    struct agp_bridge_data *bridge;

    bridge = kzalloc(sizeof(*bridge), GFP_KERNEL);
    if (!bridge)
        return NULL;

    atomic_set(&bridge->agp_in_use, 0);
    atomic_set(&bridge->current_memory_agp, 0);

//    if (list_empty(&agp_bridges))
//      agp_bridge = bridge;

    return bridge;
}

static int __devinit agp_intel_probe(struct pci_dev *pdev,
                     const struct pci_device_id *ent)
{
    struct agp_bridge_data *bridge;
    u8 cap_ptr = 0;
    int err = -ENODEV;

    cap_ptr = pci_find_capability(pdev, PCI_CAP_ID_AGP);

    bridge = agp_alloc_bridge();
    if (!bridge)
        return -ENOMEM;

    bridge->capndx = cap_ptr;

    if (intel_gmch_probe(pdev, bridge))
    {
//        pci_set_drvdata(pdev, bridge);
//        err = agp_add_bridge(bridge);
//        if (!err)
        intel_agp_enabled = 1;
        err = 0;
    }

    return err;
}

static struct pci_device_id agp_intel_pci_table[] = {
#define ID(x)                       \
    {                       \
    .class      = (PCI_CLASS_BRIDGE_HOST << 8), \
    .class_mask = ~0,               \
    .vendor     = PCI_VENDOR_ID_INTEL,      \
    .device     = x,                \
    .subvendor  = PCI_ANY_ID,           \
    .subdevice  = PCI_ANY_ID,           \
    }
    ID(PCI_DEVICE_ID_INTEL_SANDYBRIDGE_HB),
    ID(PCI_DEVICE_ID_INTEL_SANDYBRIDGE_M_HB),
    ID(PCI_DEVICE_ID_INTEL_SANDYBRIDGE_S_HB),
    { }
};

static pci_dev_t agp_device;

int init_agp(void)
{
    const struct pci_device_id  *ent;

    ent = find_pci_device(&agp_device, agp_intel_pci_table);

    if( unlikely(ent == NULL) )
    {
        dbgprintf("host controller not found\n");
        return -ENODEV;
    };

    return agp_intel_probe(&agp_device.pci_dev, ent);
}


