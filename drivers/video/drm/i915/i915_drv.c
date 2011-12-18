
/* i915_drv.c -- i830,i845,i855,i865,i915 driver -*- linux-c -*-
 */
/*
 *
 * Copyright 2003 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <drm/drmP.h>
#include <drm/drm.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <errno-base.h>
#include <linux/pci.h>


enum {
    RCS = 0x0,
    VCS,
    BCS,
    I915_NUM_RINGS,
};


#include "i915_drv.h"
#include <syscall.h>

#define INTEL_VGA_DEVICE(id, info) {        \
    .class = PCI_CLASS_DISPLAY_VGA << 8,    \
    .class_mask = 0xff0000,                 \
    .vendor = 0x8086,                       \
    .device = id,                           \
    .subvendor = PCI_ANY_ID,                \
    .subdevice = PCI_ANY_ID,                \
    .driver_data = (unsigned long) info }

static const struct intel_device_info intel_sandybridge_d_info = {
    .gen = 6,
    .need_gfx_hws = 1,
    .has_hotplug  = 1,
    .has_bsd_ring = 1,
    .has_blt_ring = 1,
};

static const struct intel_device_info intel_sandybridge_m_info = {
    .gen = 6,
    .is_mobile    = 1,
    .need_gfx_hws = 1,
    .has_hotplug  = 1,
    .has_fbc      = 1,
    .has_bsd_ring = 1,
    .has_blt_ring = 1,
};


static const struct pci_device_id pciidlist[] = {       /* aka */
    INTEL_VGA_DEVICE(0x0102, &intel_sandybridge_d_info),
    INTEL_VGA_DEVICE(0x0112, &intel_sandybridge_d_info),
    INTEL_VGA_DEVICE(0x0122, &intel_sandybridge_d_info),
    INTEL_VGA_DEVICE(0x0106, &intel_sandybridge_m_info),
    INTEL_VGA_DEVICE(0x0116, &intel_sandybridge_m_info),
    INTEL_VGA_DEVICE(0x0126, &intel_sandybridge_m_info),
    INTEL_VGA_DEVICE(0x010A, &intel_sandybridge_d_info),
    {0, 0, 0}
};


int drm_get_dev(struct pci_dev *pdev, const struct pci_device_id *ent);

int i915_init(void)
{
    static pci_dev_t device;
    const struct pci_device_id  *ent;
    int  err;

    if( init_agp() != 0)
    {
        DRM_ERROR("drm/i915 can't work without intel_agp module!\n");
        return 0;
    };

    ent = find_pci_device(&device, pciidlist);

    if( unlikely(ent == NULL) )
    {
        dbgprintf("device not found\n");
        return 0;
    };

    dbgprintf("device %x:%x\n", device.pci_dev.vendor,
                                device.pci_dev.device);

    err = drm_get_dev(&device.pci_dev, ent);

    return err;
}

int drm_get_dev(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    static struct drm_device *dev;
    int ret;

    ENTER();

    dev = kzalloc(sizeof(*dev), 0);
    if (!dev)
        return -ENOMEM;

 //   ret = pci_enable_device(pdev);
 //   if (ret)
 //       goto err_g1;

 //   pci_set_master(pdev);

 //   if ((ret = drm_fill_in_dev(dev, pdev, ent, driver))) {
 //       printk(KERN_ERR "DRM: Fill_in_dev failed.\n");
 //       goto err_g2;
 //   }

    dev->pdev = pdev;
    dev->pci_device = pdev->device;
    dev->pci_vendor = pdev->vendor;

    INIT_LIST_HEAD(&dev->filelist);
    INIT_LIST_HEAD(&dev->ctxlist);
    INIT_LIST_HEAD(&dev->vmalist);
    INIT_LIST_HEAD(&dev->maplist);

    spin_lock_init(&dev->count_lock);
    mutex_init(&dev->struct_mutex);
    mutex_init(&dev->ctxlist_mutex);

//int i915_driver_load(struct drm_device *dev, unsigned long flags)

//    ret = radeon_driver_load_kms(dev, ent->driver_data );
//    if (ret)
//        goto err_g4;

//    if( radeon_modeset )
//        init_display_kms(dev->dev_private, &usermode);
//    else
//        init_display(dev->dev_private, &usermode);

    LEAVE();

    return 0;

err_g4:
//    drm_put_minor(&dev->primary);
//err_g3:
//    if (drm_core_check_feature(dev, DRIVER_MODESET))
//        drm_put_minor(&dev->control);
//err_g2:
//    pci_disable_device(pdev);
//err_g1:
    free(dev);

    LEAVE();

    return ret;
}


