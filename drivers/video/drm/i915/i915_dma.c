/* i915_dma.c -- DMA support for the I915 -*- linux-c -*-
 */
/*
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

#include "drmP.h"
#include "drm.h"
#include "drm_crtc_helper.h"
#include "drm_fb_helper.h"
#include "intel_drv.h"
#include "i915_drm.h"
#include "i915_drv.h"
#include <drm/intel-gtt.h>
//#include "i915_trace.h"
//#include "../../../platform/x86/intel_ips.h"
#include <linux/pci.h>
//#include <linux/vgaarb.h>
//#include <linux/acpi.h>
//#include <linux/pnp.h>
//#include <linux/vga_switcheroo.h>
#include <linux/slab.h>
//#include <acpi/video.h>

void __iomem *pci_iomap(struct pci_dev *dev, int bar, unsigned long maxlen);

static inline int pci_read_config_dword(struct pci_dev *dev, int where,
                    u32 *val)
{
    *val = PciRead32(dev->busnr, dev->devfn, where);
    return 1;
}



static void i915_write_hws_pga(struct drm_device *dev)
{
    drm_i915_private_t *dev_priv = dev->dev_private;
    u32 addr;

    addr = dev_priv->status_page_dmah->busaddr;
    if (INTEL_INFO(dev)->gen >= 4)
        addr |= (dev_priv->status_page_dmah->busaddr >> 28) & 0xf0;
    I915_WRITE(HWS_PGA, addr);
}

/**
 * Sets up the hardware status page for devices that need a physical address
 * in the register.
 */
static int i915_init_phys_hws(struct drm_device *dev)
{
    drm_i915_private_t *dev_priv = dev->dev_private;

    /* Program Hardware Status Page */
    dev_priv->status_page_dmah =
        drm_pci_alloc(dev, PAGE_SIZE, PAGE_SIZE);

    if (!dev_priv->status_page_dmah) {
        DRM_ERROR("Can not allocate hardware status page\n");
        return -ENOMEM;
    }

    i915_write_hws_pga(dev);

    dbgprintf("Enabled hardware status page\n");
    return 0;
}










#define MCHBAR_I915 0x44
#define MCHBAR_I965 0x48
#define MCHBAR_SIZE (4*4096)

#define DEVEN_REG 0x54
#define   DEVEN_MCHBAR_EN (1 << 28)




/* Setup MCHBAR if possible, return true if we should disable it again */
static void
intel_setup_mchbar(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int mchbar_reg = INTEL_INFO(dev)->gen >= 4 ? MCHBAR_I965 : MCHBAR_I915;
	u32 temp;
	bool enabled;

	dev_priv->mchbar_need_disable = false;

	if (IS_I915G(dev) || IS_I915GM(dev)) {
		pci_read_config_dword(dev_priv->bridge_dev, DEVEN_REG, &temp);
		enabled = !!(temp & DEVEN_MCHBAR_EN);
	} else {
		pci_read_config_dword(dev_priv->bridge_dev, mchbar_reg, &temp);
		enabled = temp & 1;
	}

	/* If it's already enabled, don't have to do anything */
	if (enabled)
		return;

	dbgprintf("Epic fail\n");

#if 0
	if (intel_alloc_mchbar_resource(dev))
		return;

	dev_priv->mchbar_need_disable = true;

	/* Space is allocated or reserved, so enable it. */
	if (IS_I915G(dev) || IS_I915GM(dev)) {
		pci_write_config_dword(dev_priv->bridge_dev, DEVEN_REG,
				       temp | DEVEN_MCHBAR_EN);
	} else {
		pci_read_config_dword(dev_priv->bridge_dev, mchbar_reg, &temp);
		pci_write_config_dword(dev_priv->bridge_dev, mchbar_reg, temp | 1);
	}
#endif
}















#define LFB_SIZE 0xC00000

static int i915_load_gem_init(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	unsigned long prealloc_size, gtt_size, mappable_size;
	int ret;

	prealloc_size = dev_priv->mm.gtt->stolen_size;
	gtt_size = dev_priv->mm.gtt->gtt_total_entries << PAGE_SHIFT;
	mappable_size = dev_priv->mm.gtt->gtt_mappable_entries << PAGE_SHIFT;

    dbgprintf("%s prealloc: %x gtt: %x mappable: %x\n",__FUNCTION__,
             prealloc_size, gtt_size, mappable_size);

	/* Basic memrange allocator for stolen space */
	drm_mm_init(&dev_priv->mm.stolen, 0, prealloc_size);

	/* Let GEM Manage all of the aperture.
	 *
	 * However, leave one page at the end still bound to the scratch page.
	 * There are a number of places where the hardware apparently
	 * prefetches past the end of the object, and we've seen multiple
	 * hangs with the GPU head pointer stuck in a batchbuffer bound
	 * at the last page of the aperture.  One page should be enough to
	 * keep any prefetching inside of the aperture.
	 */
    i915_gem_do_init(dev, LFB_SIZE, mappable_size, gtt_size - PAGE_SIZE - LFB_SIZE);

    mutex_lock(&dev->struct_mutex);
    ret = i915_gem_init_ringbuffer(dev);
    mutex_unlock(&dev->struct_mutex);
    if (ret)
        return ret;

	/* Try to set up FBC with a reasonable compressed buffer size */
//   if (I915_HAS_FBC(dev) && i915_powersave) {
//       int cfb_size;

		/* Leave 1M for line length buffer & misc. */

		/* Try to get a 32M buffer... */
//       if (prealloc_size > (36*1024*1024))
//           cfb_size = 32*1024*1024;
//       else /* fall back to 7/8 of the stolen space */
//           cfb_size = prealloc_size * 7 / 8;
//       i915_setup_compression(dev, cfb_size);
//   }

	/* Allow hardware batchbuffers unless told otherwise. */
	dev_priv->allow_batchbuffer = 1;
	return 0;
}

static int i915_load_modeset_init(struct drm_device *dev)
{
    struct drm_i915_private *dev_priv = dev->dev_private;
    int ret;

    ret = intel_parse_bios(dev);
    if (ret)
        DRM_INFO("failed to find VBIOS tables\n");

//    intel_register_dsm_handler();

    /* IIR "flip pending" bit means done if this bit is set */
    if (IS_GEN3(dev) && (I915_READ(ECOSKPD) & ECO_FLIP_DONE))
        dev_priv->flip_pending_is_done = true;

    intel_modeset_init(dev);

    ret = i915_load_gem_init(dev);
    if (ret)
        goto cleanup_vga_switcheroo;

    intel_modeset_gem_init(dev);

//    ret = drm_irq_install(dev);
//    if (ret)
//        goto cleanup_gem;

    /* Always safe in the mode setting case. */
    /* FIXME: do pre/post-mode set stuff in core KMS code */
    dev->vblank_disable_allowed = 1;

    ret = intel_fbdev_init(dev);
    if (ret)
        goto cleanup_irq;

//    drm_kms_helper_poll_init(dev);

    /* We're off and running w/KMS */
    dev_priv->mm.suspended = 0;

    return 0;

cleanup_irq:
//    drm_irq_uninstall(dev);
cleanup_gem:
//    mutex_lock(&dev->struct_mutex);
//    i915_gem_cleanup_ringbuffer(dev);
//    mutex_unlock(&dev->struct_mutex);
cleanup_vga_switcheroo:
//    vga_switcheroo_unregister_client(dev->pdev);
cleanup_vga_client:
//    vga_client_register(dev->pdev, NULL, NULL, NULL);
out:
    return ret;
}



static void i915_pineview_get_mem_freq(struct drm_device *dev)
{
    drm_i915_private_t *dev_priv = dev->dev_private;
    u32 tmp;

    tmp = I915_READ(CLKCFG);

    switch (tmp & CLKCFG_FSB_MASK) {
    case CLKCFG_FSB_533:
        dev_priv->fsb_freq = 533; /* 133*4 */
        break;
    case CLKCFG_FSB_800:
        dev_priv->fsb_freq = 800; /* 200*4 */
        break;
    case CLKCFG_FSB_667:
        dev_priv->fsb_freq =  667; /* 167*4 */
        break;
    case CLKCFG_FSB_400:
        dev_priv->fsb_freq = 400; /* 100*4 */
        break;
    }

    switch (tmp & CLKCFG_MEM_MASK) {
    case CLKCFG_MEM_533:
        dev_priv->mem_freq = 533;
        break;
    case CLKCFG_MEM_667:
        dev_priv->mem_freq = 667;
        break;
    case CLKCFG_MEM_800:
        dev_priv->mem_freq = 800;
        break;
    }

    /* detect pineview DDR3 setting */
    tmp = I915_READ(CSHRDDR3CTL);
    dev_priv->is_ddr3 = (tmp & CSHRDDR3CTL_DDR3) ? 1 : 0;
}

static void i915_ironlake_get_mem_freq(struct drm_device *dev)
{
    drm_i915_private_t *dev_priv = dev->dev_private;
    u16 ddrpll, csipll;

    ddrpll = I915_READ16(DDRMPLL1);
    csipll = I915_READ16(CSIPLL0);

    switch (ddrpll & 0xff) {
    case 0xc:
        dev_priv->mem_freq = 800;
        break;
    case 0x10:
        dev_priv->mem_freq = 1066;
        break;
    case 0x14:
        dev_priv->mem_freq = 1333;
        break;
    case 0x18:
        dev_priv->mem_freq = 1600;
        break;
    default:
        DRM_DEBUG_DRIVER("unknown memory frequency 0x%02x\n",
                 ddrpll & 0xff);
        dev_priv->mem_freq = 0;
        break;
    }

    dev_priv->r_t = dev_priv->mem_freq;

    switch (csipll & 0x3ff) {
    case 0x00c:
        dev_priv->fsb_freq = 3200;
        break;
    case 0x00e:
        dev_priv->fsb_freq = 3733;
        break;
    case 0x010:
        dev_priv->fsb_freq = 4266;
        break;
    case 0x012:
        dev_priv->fsb_freq = 4800;
        break;
    case 0x014:
        dev_priv->fsb_freq = 5333;
        break;
    case 0x016:
        dev_priv->fsb_freq = 5866;
        break;
    case 0x018:
        dev_priv->fsb_freq = 6400;
        break;
    default:
        DRM_DEBUG_DRIVER("unknown fsb frequency 0x%04x\n",
                 csipll & 0x3ff);
        dev_priv->fsb_freq = 0;
        break;
    }

    if (dev_priv->fsb_freq == 3200) {
        dev_priv->c_m = 0;
    } else if (dev_priv->fsb_freq > 3200 && dev_priv->fsb_freq <= 4800) {
        dev_priv->c_m = 1;
    } else {
        dev_priv->c_m = 2;
    }
}

static int i915_get_bridge_dev(struct drm_device *dev)
{
    struct drm_i915_private *dev_priv = dev->dev_private;

    dev_priv->bridge_dev = pci_get_bus_and_slot(0, PCI_DEVFN(0,0));
    if (!dev_priv->bridge_dev) {
        DRM_ERROR("bridge device not found\n");
        return -1;
    }
    return 0;
}


/* Global for IPS driver to get at the current i915 device */
static struct drm_i915_private *i915_mch_dev;
/*
 * Lock protecting IPS related data structures
 *   - i915_mch_dev
 *   - dev_priv->max_delay
 *   - dev_priv->min_delay
 *   - dev_priv->fmax
 *   - dev_priv->gpu_busy
 */
static DEFINE_SPINLOCK(mchdev_lock);


/**
 * i915_driver_load - setup chip and create an initial config
 * @dev: DRM device
 * @flags: startup flags
 *
 * The driver load routine has to do several things:
 *   - drive output discovery via intel_modeset_init()
 *   - initialize the memory manager
 *   - allocate initial config memory
 *   - setup the DRM framebuffer with the allocated memory
 */
int i915_driver_load(struct drm_device *dev, unsigned long flags)
{
    struct drm_i915_private *dev_priv;
    int ret = 0, mmio_bar;
    uint32_t agp_size;

    ENTER();

    dev_priv = kzalloc(sizeof(drm_i915_private_t), GFP_KERNEL);
    if (dev_priv == NULL)
        return -ENOMEM;

    dev->dev_private = (void *)dev_priv;
    dev_priv->dev = dev;
    dev_priv->info = (struct intel_device_info *) flags;

    if (i915_get_bridge_dev(dev)) {
        ret = -EIO;
        goto free_priv;
    }

    /* overlay on gen2 is broken and can't address above 1G */
//    if (IS_GEN2(dev))
//        dma_set_coherent_mask(&dev->pdev->dev, DMA_BIT_MASK(30));

    /* 965GM sometimes incorrectly writes to hardware status page (HWS)
     * using 32bit addressing, overwriting memory if HWS is located
     * above 4GB.
     *
     * The documentation also mentions an issue with undefined
     * behaviour if any general state is accessed within a page above 4GB,
     * which also needs to be handled carefully.
     */
//    if (IS_BROADWATER(dev) || IS_CRESTLINE(dev))
//        dma_set_coherent_mask(&dev->pdev->dev, DMA_BIT_MASK(32));

    mmio_bar = IS_GEN2(dev) ? 1 : 0;
    dev_priv->regs = pci_iomap(dev->pdev, mmio_bar, 0);
    if (!dev_priv->regs) {
        DRM_ERROR("failed to map registers\n");
        ret = -EIO;
        goto put_bridge;
    }

    dev_priv->mm.gtt = intel_gtt_get();
    if (!dev_priv->mm.gtt) {
        DRM_ERROR("Failed to initialize GTT\n");
        ret = -ENODEV;
        goto out_rmmap;
    }

//    agp_size = dev_priv->mm.gtt->gtt_mappable_entries << PAGE_SHIFT;

/*   agp_bridge->gart_bus_addr = intel_private.gma_bus_addr;   */

//    dev_priv->mm.gtt_mapping =
//        io_mapping_create_wc(dev->agp->base, agp_size);
//    if (dev_priv->mm.gtt_mapping == NULL) {
//        ret = -EIO;
//        goto out_rmmap;
//    }

    /* Set up a WC MTRR for non-PAT systems.  This is more common than
     * one would think, because the kernel disables PAT on first
     * generation Core chips because WC PAT gets overridden by a UC
     * MTRR if present.  Even if a UC MTRR isn't present.
     */
//    dev_priv->mm.gtt_mtrr = mtrr_add(dev->agp->base,
//                     agp_size,
//                     MTRR_TYPE_WRCOMB, 1);
//    if (dev_priv->mm.gtt_mtrr < 0) {
//        DRM_INFO("MTRR allocation failed.  Graphics "
//             "performance may suffer.\n");
//    }

    /* The i915 workqueue is primarily used for batched retirement of
     * requests (and thus managing bo) once the task has been completed
     * by the GPU. i915_gem_retire_requests() is called directly when we
     * need high-priority retirement, such as waiting for an explicit
     * bo.
     *
     * It is also used for periodic low-priority events, such as
     * idle-timers and recording error state.
     *
     * All tasks on the workqueue are expected to acquire the dev mutex
     * so there is no point in running more than one instance of the
     * workqueue at any time: max_active = 1 and NON_REENTRANT.
     */

//    dev_priv->wq = alloc_workqueue("i915",
//                       WQ_UNBOUND | WQ_NON_REENTRANT,
//                       1);
//    if (dev_priv->wq == NULL) {
//        DRM_ERROR("Failed to create our workqueue.\n");
//        ret = -ENOMEM;
//        goto out_mtrrfree;
//    }

    /* enable GEM by default */
    dev_priv->has_gem = 1;


//    intel_irq_init(dev);

    /* Try to make sure MCHBAR is enabled before poking at it */
	intel_setup_mchbar(dev);
    intel_setup_gmbus(dev);
    intel_opregion_setup(dev);

    /* Make sure the bios did its job and set up vital registers */
    intel_setup_bios(dev);

    i915_gem_load(dev);

    /* Init HWS */
    if (!I915_NEED_GFX_HWS(dev)) {
        ret = i915_init_phys_hws(dev);
        if (ret)
            goto out_gem_unload;
    }

    if (IS_PINEVIEW(dev))
        i915_pineview_get_mem_freq(dev);
    else if (IS_GEN5(dev))
        i915_ironlake_get_mem_freq(dev);

    /* On the 945G/GM, the chipset reports the MSI capability on the
     * integrated graphics even though the support isn't actually there
     * according to the published specs.  It doesn't appear to function
     * correctly in testing on 945G.
     * This may be a side effect of MSI having been made available for PEG
     * and the registers being closely associated.
     *
     * According to chipset errata, on the 965GM, MSI interrupts may
     * be lost or delayed, but we use them anyways to avoid
     * stuck interrupts on some machines.
     */
//    if (!IS_I945G(dev) && !IS_I945GM(dev))
//        pci_enable_msi(dev->pdev);

    spin_lock_init(&dev_priv->irq_lock);
    spin_lock_init(&dev_priv->error_lock);
    spin_lock_init(&dev_priv->rps_lock);

    if (IS_MOBILE(dev) || !IS_GEN2(dev))
        dev_priv->num_pipe = 2;
    else
        dev_priv->num_pipe = 1;

//    ret = drm_vblank_init(dev, dev_priv->num_pipe);
//    if (ret)
//        goto out_gem_unload;

    /* Start out suspended */
    dev_priv->mm.suspended = 1;

    intel_detect_pch(dev);

    ret = i915_load_modeset_init(dev);
    if (ret < 0) {
        DRM_ERROR("failed to init modeset\n");
            goto out_gem_unload;
    }

    /* Must be done after probing outputs */
//    intel_opregion_init(dev);
//    acpi_video_register();

//    setup_timer(&dev_priv->hangcheck_timer, i915_hangcheck_elapsed,
//            (unsigned long) dev);

    spin_lock(&mchdev_lock);
    i915_mch_dev = dev_priv;
    dev_priv->mchdev_lock = &mchdev_lock;
    spin_unlock(&mchdev_lock);

//    ips_ping_for_i915_load();

    LEAVE();

    return 0;

out_gem_unload:
//    if (dev_priv->mm.inactive_shrinker.shrink)
//        unregister_shrinker(&dev_priv->mm.inactive_shrinker);

//    if (dev->pdev->msi_enabled)
//        pci_disable_msi(dev->pdev);

//    intel_teardown_gmbus(dev);
//    intel_teardown_mchbar(dev);
//    destroy_workqueue(dev_priv->wq);
out_mtrrfree:
//    if (dev_priv->mm.gtt_mtrr >= 0) {
//        mtrr_del(dev_priv->mm.gtt_mtrr, dev->agp->base,
//             dev->agp->agp_info.aper_size * 1024 * 1024);
//        dev_priv->mm.gtt_mtrr = -1;
//    }
//    io_mapping_free(dev_priv->mm.gtt_mapping);

out_rmmap:
    pci_iounmap(dev->pdev, dev_priv->regs);
put_bridge:
//    pci_dev_put(dev_priv->bridge_dev);
free_priv:
    kfree(dev_priv);
    return ret;
}

