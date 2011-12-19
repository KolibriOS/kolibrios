
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

#include "i915_drv.h"
#include <syscall.h>

#define PCI_VENDOR_ID_INTEL        0x8086

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

#define INTEL_PCH_DEVICE_ID_MASK        0xff00
#define INTEL_PCH_IBX_DEVICE_ID_TYPE    0x3b00
#define INTEL_PCH_CPT_DEVICE_ID_TYPE    0x1c00
#define INTEL_PCH_PPT_DEVICE_ID_TYPE    0x1e00

void intel_detect_pch (struct drm_device *dev)
{
    struct drm_i915_private *dev_priv = dev->dev_private;
    struct pci_dev *pch;

    /*
     * The reason to probe ISA bridge instead of Dev31:Fun0 is to
     * make graphics device passthrough work easy for VMM, that only
     * need to expose ISA bridge to let driver know the real hardware
     * underneath. This is a requirement from virtualization team.
     */
    pch = pci_get_class(PCI_CLASS_BRIDGE_ISA << 8, NULL);
    if (pch) {
        if (pch->vendor == PCI_VENDOR_ID_INTEL) {
            int id;
            id = pch->device & INTEL_PCH_DEVICE_ID_MASK;

            if (id == INTEL_PCH_IBX_DEVICE_ID_TYPE) {
                dev_priv->pch_type = PCH_IBX;
                DRM_DEBUG_KMS("Found Ibex Peak PCH\n");
            } else if (id == INTEL_PCH_CPT_DEVICE_ID_TYPE) {
                dev_priv->pch_type = PCH_CPT;
                DRM_DEBUG_KMS("Found CougarPoint PCH\n");
            } else if (id == INTEL_PCH_PPT_DEVICE_ID_TYPE) {
                /* PantherPoint is CPT compatible */
                dev_priv->pch_type = PCH_CPT;
                DRM_DEBUG_KMS("Found PatherPoint PCH\n");
            }
        }
    }
}

static void __gen6_gt_force_wake_get(struct drm_i915_private *dev_priv)
{
    int count;

    count = 0;
    while (count++ < 50 && (I915_READ_NOTRACE(FORCEWAKE_ACK) & 1))
        udelay(10);

    I915_WRITE_NOTRACE(FORCEWAKE, 1);
    POSTING_READ(FORCEWAKE);

    count = 0;
    while (count++ < 50 && (I915_READ_NOTRACE(FORCEWAKE_ACK) & 1) == 0)
        udelay(10);
}

/*
 * Generally this is called implicitly by the register read function. However,
 * if some sequence requires the GT to not power down then this function should
 * be called at the beginning of the sequence followed by a call to
 * gen6_gt_force_wake_put() at the end of the sequence.
 */
void gen6_gt_force_wake_get(struct drm_i915_private *dev_priv)
{
//    WARN_ON(!mutex_is_locked(&dev_priv->dev->struct_mutex));

    /* Forcewake is atomic in case we get in here without the lock */
    if (atomic_add_return(1, &dev_priv->forcewake_count) == 1)
        __gen6_gt_force_wake_get(dev_priv);
}

static void __gen6_gt_force_wake_put(struct drm_i915_private *dev_priv)
{
    I915_WRITE_NOTRACE(FORCEWAKE, 0);
    POSTING_READ(FORCEWAKE);
}

/*
 * see gen6_gt_force_wake_get()
 */
void gen6_gt_force_wake_put(struct drm_i915_private *dev_priv)
{
//    WARN_ON(!mutex_is_locked(&dev_priv->dev->struct_mutex));

    if (atomic_dec_and_test(&dev_priv->forcewake_count))
        __gen6_gt_force_wake_put(dev_priv);
}

void __gen6_gt_wait_for_fifo(struct drm_i915_private *dev_priv)
{
    if (dev_priv->gt_fifo_count < GT_FIFO_NUM_RESERVED_ENTRIES ) {
        int loop = 500;
        u32 fifo = I915_READ_NOTRACE(GT_FIFO_FREE_ENTRIES);
        while (fifo <= GT_FIFO_NUM_RESERVED_ENTRIES && loop--) {
            udelay(10);
            fifo = I915_READ_NOTRACE(GT_FIFO_FREE_ENTRIES);
        }
//        WARN_ON(loop < 0 && fifo <= GT_FIFO_NUM_RESERVED_ENTRIES);
        dev_priv->gt_fifo_count = fifo;
    }
    dev_priv->gt_fifo_count--;
}





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

    ret = i915_driver_load(dev, ent->driver_data );
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


