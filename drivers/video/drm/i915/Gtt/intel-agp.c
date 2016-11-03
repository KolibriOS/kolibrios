/*
 * Intel AGPGART routines.
 */

#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/gfp.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/agp_backend.h>
#include "agp.h"
#include "intel-agp.h"
#include <drm/intel-gtt.h>

#include <linux/spinlock.h>



#include <syscall.h>

#define __devinit
#define PCI_VENDOR_ID_INTEL             0x8086
#define PCI_DEVICE_ID_INTEL_82915G_HB   0x2580
#define PCI_DEVICE_ID_INTEL_82915GM_HB  0x2590
#define PCI_DEVICE_ID_INTEL_82945G_HB   0x2770
#define PCI_DEVICE_ID_INTEL_82945GM_HB  0x27A0


int intel_gmch_probe(struct pci_dev *bridge_pdev, struct pci_dev *gpu_pdev,
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

	if (intel_gmch_probe(pdev, NULL, bridge))

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
	ID(PCI_DEVICE_ID_INTEL_E7221_HB),
	ID(PCI_DEVICE_ID_INTEL_82915G_HB),
	ID(PCI_DEVICE_ID_INTEL_82915GM_HB),
	ID(PCI_DEVICE_ID_INTEL_82945G_HB),
	ID(PCI_DEVICE_ID_INTEL_82945GM_HB),
	ID(PCI_DEVICE_ID_INTEL_82945GME_HB),
	ID(PCI_DEVICE_ID_INTEL_PINEVIEW_M_HB),
	ID(PCI_DEVICE_ID_INTEL_PINEVIEW_HB),
	ID(PCI_DEVICE_ID_INTEL_82946GZ_HB),
	ID(PCI_DEVICE_ID_INTEL_82G35_HB),
	ID(PCI_DEVICE_ID_INTEL_82965Q_HB),
	ID(PCI_DEVICE_ID_INTEL_82965G_HB),
	ID(PCI_DEVICE_ID_INTEL_82965GM_HB),
	ID(PCI_DEVICE_ID_INTEL_82965GME_HB),
	ID(PCI_DEVICE_ID_INTEL_G33_HB),
	ID(PCI_DEVICE_ID_INTEL_Q35_HB),
	ID(PCI_DEVICE_ID_INTEL_Q33_HB),
	ID(PCI_DEVICE_ID_INTEL_GM45_HB),
	ID(PCI_DEVICE_ID_INTEL_EAGLELAKE_HB),
	ID(PCI_DEVICE_ID_INTEL_Q45_HB),
	ID(PCI_DEVICE_ID_INTEL_G45_HB),
	ID(PCI_DEVICE_ID_INTEL_G41_HB),
	ID(PCI_DEVICE_ID_INTEL_B43_HB),
	ID(PCI_DEVICE_ID_INTEL_B43_1_HB),
	ID(PCI_DEVICE_ID_INTEL_IRONLAKE_D_HB),
	ID(PCI_DEVICE_ID_INTEL_IRONLAKE_D2_HB),
	ID(PCI_DEVICE_ID_INTEL_IRONLAKE_M_HB),
	ID(PCI_DEVICE_ID_INTEL_IRONLAKE_MA_HB),
	ID(PCI_DEVICE_ID_INTEL_IRONLAKE_MC2_HB),
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


