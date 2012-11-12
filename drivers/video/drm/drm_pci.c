/* drm_pci.h -- PCI DMA memory management wrappers for DRM -*- linux-c -*- */
/**
 * \file drm_pci.c
 * \brief Functions and ioctls to manage PCI memory
 *
 * \warning These interfaces aren't stable yet.
 *
 * \todo Implement the remaining ioctl's for the PCI pools.
 * \todo The wrappers here are so thin that they would be better off inlined..
 *
 * \author José Fonseca <jrfonseca@tungstengraphics.com>
 * \author Leif Delgass <ldelgass@retinalburn.net>
 */

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

//#include <linux/pci.h>
//#include <linux/slab.h>
//#include <linux/dma-mapping.h>
#include <linux/export.h>
#include <drm/drmP.h>

#include <syscall.h>

/**********************************************************************/
/** \name PCI memory */
/*@{*/

/**
 * \brief Allocate a PCI consistent memory block, for DMA.
 */
drm_dma_handle_t *drm_pci_alloc(struct drm_device * dev, size_t size, size_t align)
{
	drm_dma_handle_t *dmah;
#if 1
	unsigned long addr;
	size_t sz;
#endif

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
    dmah->vaddr = (void*)KernelAlloc(size);
    dmah->busaddr = GetPgAddr(dmah->vaddr);

	if (dmah->vaddr == NULL) {
		kfree(dmah);
		return NULL;
	}

	memset(dmah->vaddr, 0, size);

	return dmah;
}


int drm_pcie_get_speed_cap_mask(struct drm_device *dev, u32 *mask)
{
	struct pci_dev *root;
	int pos;
	u32 lnkcap, lnkcap2;

	*mask = 0;
	if (!dev->pdev)
		return -EINVAL;

	if (!pci_is_pcie(dev->pdev))
		return -EINVAL;

    return -EINVAL;

#if 0
	root = dev->pdev->bus->self;

	pos = pci_pcie_cap(root);
	if (!pos)
		return -EINVAL;

	/* we've been informed via and serverworks don't make the cut */
//   if (root->vendor == PCI_VENDOR_ID_VIA ||
//       root->vendor == PCI_VENDOR_ID_SERVERWORKS)
//       return -EINVAL;

	pci_read_config_dword(root, pos + PCI_EXP_LNKCAP, &lnkcap);
	pci_read_config_dword(root, pos + PCI_EXP_LNKCAP2, &lnkcap2);

	lnkcap &= PCI_EXP_LNKCAP_SLS;
	lnkcap2 &= 0xfe;

	if (lnkcap2) { /* PCIE GEN 3.0 */
		if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_2_5GB)
			*mask |= DRM_PCIE_SPEED_25;
		if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_5_0GB)
			*mask |= DRM_PCIE_SPEED_50;
		if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_8_0GB)
			*mask |= DRM_PCIE_SPEED_80;
	} else {
		if (lnkcap & 1)
			*mask |= DRM_PCIE_SPEED_25;
		if (lnkcap & 2)
			*mask |= DRM_PCIE_SPEED_50;
	}

	DRM_INFO("probing gen 2 caps for device %x:%x = %x/%x\n", root->vendor, root->device, lnkcap, lnkcap2);
	return 0;
#endif

}
EXPORT_SYMBOL(drm_pcie_get_speed_cap_mask);
