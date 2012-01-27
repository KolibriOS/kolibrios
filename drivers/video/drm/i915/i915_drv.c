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

//#include <linux/device.h>
#include "drmP.h"
#include "drm.h"
#include "i915_drm.h"
#include "i915_drv.h"
#include "intel_drv.h"


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <errno-base.h>
#include <linux/pci.h>

#include <syscall.h>

#define __read_mostly

int init_display_kms(struct drm_device *dev);


int i915_panel_ignore_lid __read_mostly         =  0;

unsigned int i915_powersave  __read_mostly      =  0;

unsigned int i915_enable_rc6 __read_mostly      =  0;

unsigned int i915_enable_fbc __read_mostly      =  0;

unsigned int i915_lvds_downclock  __read_mostly =  0;

unsigned int i915_panel_use_ssc __read_mostly   =  1;

int i915_vbt_sdvo_panel_type __read_mostly      = -1;

#define PCI_VENDOR_ID_INTEL        0x8086

#define INTEL_VGA_DEVICE(id, info) {        \
    .class = PCI_CLASS_DISPLAY_VGA << 8,    \
    .class_mask = 0xff0000,                 \
    .vendor = 0x8086,                       \
    .device = id,                           \
    .subvendor = PCI_ANY_ID,                \
    .subdevice = PCI_ANY_ID,                \
    .driver_data = (unsigned long) info }

static const struct intel_device_info intel_i830_info = {
	.gen = 2, .is_mobile = 1, .cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
};

static const struct intel_device_info intel_845g_info = {
	.gen = 2,
	.has_overlay = 1, .overlay_needs_physical = 1,
};

static const struct intel_device_info intel_i85x_info = {
	.gen = 2, .is_i85x = 1, .is_mobile = 1,
	.cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
};

static const struct intel_device_info intel_i865g_info = {
	.gen = 2,
	.has_overlay = 1, .overlay_needs_physical = 1,
};

static const struct intel_device_info intel_i915g_info = {
	.gen = 3, .is_i915g = 1, .cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
};
static const struct intel_device_info intel_i915gm_info = {
	.gen = 3, .is_mobile = 1,
	.cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.supports_tv = 1,
};
static const struct intel_device_info intel_i945g_info = {
	.gen = 3, .has_hotplug = 1, .cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
};
static const struct intel_device_info intel_i945gm_info = {
	.gen = 3, .is_i945gm = 1, .is_mobile = 1,
	.has_hotplug = 1, .cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.supports_tv = 1,
};

static const struct intel_device_info intel_i965g_info = {
	.gen = 4, .is_broadwater = 1,
	.has_hotplug = 1,
	.has_overlay = 1,
};

static const struct intel_device_info intel_i965gm_info = {
	.gen = 4, .is_crestline = 1,
	.is_mobile = 1, .has_fbc = 1, .has_hotplug = 1,
	.has_overlay = 1,
	.supports_tv = 1,
};

static const struct intel_device_info intel_g33_info = {
	.gen = 3, .is_g33 = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_overlay = 1,
};

static const struct intel_device_info intel_g45_info = {
	.gen = 4, .is_g4x = 1, .need_gfx_hws = 1,
	.has_pipe_cxsr = 1, .has_hotplug = 1,
	.has_bsd_ring = 1,
};

static const struct intel_device_info intel_gm45_info = {
	.gen = 4, .is_g4x = 1,
	.is_mobile = 1, .need_gfx_hws = 1, .has_fbc = 1,
	.has_pipe_cxsr = 1, .has_hotplug = 1,
	.supports_tv = 1,
	.has_bsd_ring = 1,
};

static const struct intel_device_info intel_pineview_info = {
	.gen = 3, .is_g33 = 1, .is_pineview = 1, .is_mobile = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_overlay = 1,
};

static const struct intel_device_info intel_ironlake_d_info = {
	.gen = 5,
	.need_gfx_hws = 1, .has_pipe_cxsr = 1, .has_hotplug = 1,
	.has_bsd_ring = 1,
};

static const struct intel_device_info intel_ironlake_m_info = {
	.gen = 5, .is_mobile = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_fbc = 1,
	.has_bsd_ring = 1,
};

static const struct intel_device_info intel_sandybridge_d_info = {
    .gen = 6,
	.need_gfx_hws = 1, .has_hotplug = 1,
    .has_bsd_ring = 1,
    .has_blt_ring = 1,
};

static const struct intel_device_info intel_sandybridge_m_info = {
	.gen = 6, .is_mobile = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
    .has_fbc      = 1,
    .has_bsd_ring = 1,
    .has_blt_ring = 1,
};

static const struct intel_device_info intel_ivybridge_d_info = {
	.is_ivybridge = 1, .gen = 7,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_bsd_ring = 1,
	.has_blt_ring = 1,
};

static const struct intel_device_info intel_ivybridge_m_info = {
	.is_ivybridge = 1, .gen = 7, .is_mobile = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_fbc = 0,	/* FBC is not enabled on Ivybridge mobile yet */
	.has_bsd_ring = 1,
	.has_blt_ring = 1,
};

static const struct pci_device_id pciidlist[] = {       /* aka */
	INTEL_VGA_DEVICE(0x2582, &intel_i915g_info),		/* I915_G */
	INTEL_VGA_DEVICE(0x258a, &intel_i915g_info),		/* E7221_G */
	INTEL_VGA_DEVICE(0x2592, &intel_i915gm_info),		/* I915_GM */
	INTEL_VGA_DEVICE(0x2772, &intel_i945g_info),		/* I945_G */
	INTEL_VGA_DEVICE(0x27a2, &intel_i945gm_info),		/* I945_GM */
	INTEL_VGA_DEVICE(0x27ae, &intel_i945gm_info),		/* I945_GME */
	INTEL_VGA_DEVICE(0x2972, &intel_i965g_info),		/* I946_GZ */
	INTEL_VGA_DEVICE(0x2982, &intel_i965g_info),		/* G35_G */
	INTEL_VGA_DEVICE(0x2992, &intel_i965g_info),		/* I965_Q */
	INTEL_VGA_DEVICE(0x29a2, &intel_i965g_info),		/* I965_G */
	INTEL_VGA_DEVICE(0x29b2, &intel_g33_info),		/* Q35_G */
	INTEL_VGA_DEVICE(0x29c2, &intel_g33_info),		/* G33_G */
	INTEL_VGA_DEVICE(0x29d2, &intel_g33_info),		/* Q33_G */
	INTEL_VGA_DEVICE(0x2a02, &intel_i965gm_info),		/* I965_GM */
	INTEL_VGA_DEVICE(0x2a12, &intel_i965gm_info),		/* I965_GME */
	INTEL_VGA_DEVICE(0x2a42, &intel_gm45_info),		/* GM45_G */
	INTEL_VGA_DEVICE(0x2e02, &intel_g45_info),		/* IGD_E_G */
	INTEL_VGA_DEVICE(0x2e12, &intel_g45_info),		/* Q45_G */
	INTEL_VGA_DEVICE(0x2e22, &intel_g45_info),		/* G45_G */
	INTEL_VGA_DEVICE(0x2e32, &intel_g45_info),		/* G41_G */
	INTEL_VGA_DEVICE(0x2e42, &intel_g45_info),		/* B43_G */
	INTEL_VGA_DEVICE(0x2e92, &intel_g45_info),		/* B43_G.1 */
	INTEL_VGA_DEVICE(0xa001, &intel_pineview_info),
	INTEL_VGA_DEVICE(0xa011, &intel_pineview_info),
	INTEL_VGA_DEVICE(0x0042, &intel_ironlake_d_info),
	INTEL_VGA_DEVICE(0x0046, &intel_ironlake_m_info),
    INTEL_VGA_DEVICE(0x0102, &intel_sandybridge_d_info),
    INTEL_VGA_DEVICE(0x0112, &intel_sandybridge_d_info),
    INTEL_VGA_DEVICE(0x0122, &intel_sandybridge_d_info),
    INTEL_VGA_DEVICE(0x0106, &intel_sandybridge_m_info),
    INTEL_VGA_DEVICE(0x0116, &intel_sandybridge_m_info),
    INTEL_VGA_DEVICE(0x0126, &intel_sandybridge_m_info),
    INTEL_VGA_DEVICE(0x010A, &intel_sandybridge_d_info),
	INTEL_VGA_DEVICE(0x0156, &intel_ivybridge_m_info), /* GT1 mobile */
	INTEL_VGA_DEVICE(0x0166, &intel_ivybridge_m_info), /* GT2 mobile */
	INTEL_VGA_DEVICE(0x0152, &intel_ivybridge_d_info), /* GT1 desktop */
	INTEL_VGA_DEVICE(0x0162, &intel_ivybridge_d_info), /* GT2 desktop */
	INTEL_VGA_DEVICE(0x015a, &intel_ivybridge_d_info), /* GT1 server */
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

    ret = i915_driver_load(dev, ent->driver_data );

    if (ret)
        goto err_g4;

    ret = init_display_kms(dev);

    if (ret)
        goto err_g4;


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


