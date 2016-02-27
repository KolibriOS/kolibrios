/*
 * Copyright 2003 José Fonseca.
 * Copyright 2003 Leif Delgass.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/export.h>
#include <drm/drmP.h>
#include "drm_internal.h"
#include "drm_legacy.h"

#include <syscall.h>
/**
 * drm_pci_alloc - Allocate a PCI consistent memory block, for DMA.
 * @dev: DRM device
 * @size: size of block to allocate
 * @align: alignment of block
 *
 * Return: A handle to the allocated memory block on success or NULL on
 * failure.
 */
drm_dma_handle_t *drm_pci_alloc(struct drm_device * dev, size_t size, size_t align)
{
	drm_dma_handle_t *dmah;
	unsigned long addr;
	size_t sz;

	/* pci_alloc_consistent only guarantees alignment to the smallest
	 * PAGE_SIZE order which is greater than or equal to the requested size.
	 * Return NULL here for now to make sure nobody tries for larger alignment
	 */
	if (align > size)
		return NULL;

	dmah = kmalloc(sizeof(drm_dma_handle_t), GFP_KERNEL);
	if (!dmah)
		return NULL;

	dmah->size = size;
	dmah->vaddr = dma_alloc_coherent(&dev->pdev->dev, size, &dmah->busaddr, GFP_KERNEL | __GFP_COMP);

	if (dmah->vaddr == NULL) {
		kfree(dmah);
		return NULL;
	}

	memset(dmah->vaddr, 0, size);

	return dmah;
}

EXPORT_SYMBOL(drm_pci_alloc);

/*
 * Free a PCI consistent memory block without freeing its descriptor.
 *
 * This function is for internal use in the Linux-specific DRM core code.
 */
void __drm_legacy_pci_free(struct drm_device * dev, drm_dma_handle_t * dmah)
{
	unsigned long addr;
	size_t sz;

	if (dmah->vaddr) {
		KernelFree(dmah->vaddr);
	}
}

/**
 * drm_pci_free - Free a PCI consistent memory block
 * @dev: DRM device
 * @dmah: handle to memory block
 */
void drm_pci_free(struct drm_device * dev, drm_dma_handle_t * dmah)
{
	__drm_legacy_pci_free(dev, dmah);
	kfree(dmah);
}

EXPORT_SYMBOL(drm_pci_free);

#if 0

static int drm_get_pci_domain(struct drm_device *dev)
{
#ifndef __alpha__
	/* For historical reasons, drm_get_pci_domain() is busticated
	 * on most archs and has to remain so for userspace interface
	 * < 1.4, except on alpha which was right from the beginning
	 */
	if (dev->if_version < 0x10004)
		return 0;
#endif /* __alpha__ */

	return pci_domain_nr(dev->pdev->bus);
}

int drm_pci_set_busid(struct drm_device *dev, struct drm_master *master)
{
	master->unique = kasprintf(GFP_KERNEL, "pci:%04x:%02x:%02x.%d",
					drm_get_pci_domain(dev),
					dev->pdev->bus->number,
					PCI_SLOT(dev->pdev->devfn),
					PCI_FUNC(dev->pdev->devfn));
	if (!master->unique)
		return -ENOMEM;

	master->unique_len = strlen(master->unique);
	return 0;
}
EXPORT_SYMBOL(drm_pci_set_busid);

int drm_pci_set_unique(struct drm_device *dev,
		       struct drm_master *master,
		       struct drm_unique *u)
{
	int domain, bus, slot, func, ret;

	master->unique_len = u->unique_len;
	master->unique = kmalloc(master->unique_len + 1, GFP_KERNEL);
	if (!master->unique) {
		ret = -ENOMEM;
		goto err;
	}

	if (copy_from_user(master->unique, u->unique, master->unique_len)) {
		ret = -EFAULT;
		goto err;
	}

	master->unique[master->unique_len] = '\0';

	/* Return error if the busid submitted doesn't match the device's actual
	 * busid.
	 */
	ret = sscanf(master->unique, "PCI:%d:%d:%d", &bus, &slot, &func);
	if (ret != 3) {
		ret = -EINVAL;
		goto err;
	}

	domain = bus >> 8;
	bus &= 0xff;

	if ((domain != drm_get_pci_domain(dev)) ||
	    (bus != dev->pdev->bus->number) ||
	    (slot != PCI_SLOT(dev->pdev->devfn)) ||
	    (func != PCI_FUNC(dev->pdev->devfn))) {
		ret = -EINVAL;
		goto err;
	}
	return 0;
err:
	return ret;
}

static int drm_pci_irq_by_busid(struct drm_device *dev, struct drm_irq_busid *p)
{
	if ((p->busnum >> 8) != drm_get_pci_domain(dev) ||
	    (p->busnum & 0xff) != dev->pdev->bus->number ||
	    p->devnum != PCI_SLOT(dev->pdev->devfn) || p->funcnum != PCI_FUNC(dev->pdev->devfn))
		return -EINVAL;

	p->irq = dev->pdev->irq;

	DRM_DEBUG("%d:%d:%d => IRQ %d\n", p->busnum, p->devnum, p->funcnum,
		  p->irq);
	return 0;
}

static void drm_pci_agp_init(struct drm_device *dev)
{
	if (drm_core_check_feature(dev, DRIVER_USE_AGP)) {
		if (drm_pci_device_is_agp(dev))
			dev->agp = drm_agp_init(dev);
		if (dev->agp) {
			dev->agp->agp_mtrr = arch_phys_wc_add(
				dev->agp->agp_info.aper_base,
				dev->agp->agp_info.aper_size *
				1024 * 1024);
		}
	}
}

void drm_pci_agp_destroy(struct drm_device *dev)
{
	if (dev->agp) {
		arch_phys_wc_del(dev->agp->agp_mtrr);
		drm_agp_clear(dev);
		kfree(dev->agp);
		dev->agp = NULL;
	}
}
#endif

/**
 * drm_get_pci_dev - Register a PCI device with the DRM subsystem
 * @pdev: PCI device
 * @ent: entry from the PCI ID table that matches @pdev
 * @driver: DRM device driver
 *
 * Attempt to gets inter module "drm" information. If we are first
 * then register the character device and inter module information.
 * Try and register, if we fail to register, backout previous work.
 *
 * NOTE: This function is deprecated, please use drm_dev_alloc() and
 * drm_dev_register() instead and remove your ->load() callback.
 *
 * Return: 0 on success or a negative error code on failure.
 */
int drm_get_pci_dev(struct pci_dev *pdev, const struct pci_device_id *ent,
		    struct drm_driver *driver)
{
    static struct drm_device drm_dev;
    static struct drm_file   drm_file;

	struct drm_device *dev;
    struct drm_file   *priv;

	int ret;

    dev  = &drm_dev;
    priv = &drm_file;

    drm_file_handlers[0] = priv;

 //   ret = pci_enable_device(pdev);
 //   if (ret)
 //       goto err_g1;

    pci_set_master(pdev);

    if ((ret = drm_fill_in_dev(dev, ent, driver))) {
        printk(KERN_ERR "DRM: Fill_in_dev failed.\n");
        goto err_g2;
    }

	DRM_DEBUG("\n");


	dev->pdev = pdev;
#ifdef __alpha__
	dev->hose = pdev->sysdata;
#endif


	if ((ret = drm_fill_in_dev(dev, ent, driver))) {
		printk(KERN_ERR "DRM: Fill_in_dev failed.\n");
		goto err_g2;
	}

#if 0
	if (drm_core_check_feature(dev, DRIVER_MODESET)) {
		pci_set_drvdata(pdev, dev);
		ret = drm_get_minor(dev, &dev->control, DRM_MINOR_CONTROL);
		if (ret)
			goto err_g2;
	}

	if (drm_core_check_feature(dev, DRIVER_RENDER) && drm_rnodes) {
		ret = drm_get_minor(dev, &dev->render, DRM_MINOR_RENDER);
		if (ret)
			goto err_g21;
	}

	if ((ret = drm_get_minor(dev, &dev->primary, DRM_MINOR_LEGACY)))
		goto err_g3;
#endif

	if (dev->driver->load) {
		ret = dev->driver->load(dev, ent->driver_data);
		if (ret)
			goto err_g4;
	}

    if (dev->driver->open) {
        ret = dev->driver->open(dev, priv);
        if (ret < 0)
            goto err_g4;
    }


//   mutex_unlock(&drm_global_mutex);
	return 0;

err_g4:
//   drm_put_minor(&dev->primary);
err_g3:
//   if (dev->render)
//       drm_put_minor(&dev->render);
err_g21:
//   if (drm_core_check_feature(dev, DRIVER_MODESET))
//       drm_put_minor(&dev->control);
err_g2:
//   pci_disable_device(pdev);
err_g1:
//   kfree(dev);
//   mutex_unlock(&drm_global_mutex);
	return ret;
}
EXPORT_SYMBOL(drm_get_pci_dev);

int drm_pcie_get_speed_cap_mask(struct drm_device *dev, u32 *mask)
{
	struct pci_dev *root;
	u32 lnkcap, lnkcap2;

	*mask = 0;
	if (!dev->pdev)
		return -EINVAL;


    return -EINVAL;

#if 0
	root = dev->pdev->bus->self;

	/* we've been informed via and serverworks don't make the cut */
	if (root->vendor == PCI_VENDOR_ID_VIA ||
	    root->vendor == PCI_VENDOR_ID_SERVERWORKS)
		return -EINVAL;

	pcie_capability_read_dword(root, PCI_EXP_LNKCAP, &lnkcap);
	pcie_capability_read_dword(root, PCI_EXP_LNKCAP2, &lnkcap2);

	if (lnkcap2) {	/* PCIe r3.0-compliant */
		if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_2_5GB)
			*mask |= DRM_PCIE_SPEED_25;
		if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_5_0GB)
			*mask |= DRM_PCIE_SPEED_50;
		if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_8_0GB)
			*mask |= DRM_PCIE_SPEED_80;
	} else {	/* pre-r3.0 */
		if (lnkcap & PCI_EXP_LNKCAP_SLS_2_5GB)
			*mask |= DRM_PCIE_SPEED_25;
		if (lnkcap & PCI_EXP_LNKCAP_SLS_5_0GB)
			*mask |= (DRM_PCIE_SPEED_25 | DRM_PCIE_SPEED_50);
	}

	DRM_INFO("probing gen 2 caps for device %x:%x = %x/%x\n", root->vendor, root->device, lnkcap, lnkcap2);
	return 0;
#endif

}
EXPORT_SYMBOL(drm_pcie_get_speed_cap_mask);
