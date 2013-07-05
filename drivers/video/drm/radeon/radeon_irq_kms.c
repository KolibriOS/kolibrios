/*
 * Copyright 2008 Advanced Micro Devices, Inc.
 * Copyright 2008 Red Hat Inc.
 * Copyright 2009 Jerome Glisse.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Dave Airlie
 *          Alex Deucher
 *          Jerome Glisse
 */
#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/radeon_drm.h>
#include "radeon_reg.h"
#include "radeon.h"
#include "atom.h"

#define RADEON_WAIT_IDLE_TIMEOUT 200


extern int irq_override;


/**
 * radeon_driver_irq_handler_kms - irq handler for KMS
 *
 * @DRM_IRQ_ARGS: args
 *
 * This is the irq handler for the radeon KMS driver (all asics).
 * radeon_irq_process is a macro that points to the per-asic
 * irq handler callback.
 */
irqreturn_t radeon_driver_irq_handler_kms(DRM_IRQ_ARGS)
{
	struct drm_device *dev = (struct drm_device *) arg;
	struct radeon_device *rdev = dev->dev_private;

    return radeon_irq_process(rdev);
}

/**
 * radeon_driver_irq_preinstall_kms - drm irq preinstall callback
 *
 * @dev: drm dev pointer
 *
 * Gets the hw ready to enable irqs (all asics).
 * This function disables all interrupt sources on the GPU.
 */
void radeon_driver_irq_preinstall_kms(struct drm_device *dev)
{
	struct radeon_device *rdev = dev->dev_private;
	unsigned long irqflags;
    unsigned i;

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
    /* Disable *all* interrupts */
	for (i = 0; i < RADEON_NUM_RINGS; i++)
		atomic_set(&rdev->irq.ring_int[i], 0);
	for (i = 0; i < RADEON_MAX_HPD_PINS; i++)
		rdev->irq.hpd[i] = false;
	for (i = 0; i < RADEON_MAX_CRTCS; i++) {
        rdev->irq.crtc_vblank_int[i] = false;
		atomic_set(&rdev->irq.pflip[i], 0);
		rdev->irq.afmt[i] = false;
    }
    radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
    /* Clear bits */
    radeon_irq_process(rdev);
}

/**
 * radeon_driver_irq_postinstall_kms - drm irq preinstall callback
 *
 * @dev: drm dev pointer
 *
 * Handles stuff to be done after enabling irqs (all asics).
 * Returns 0 on success.
 */
int radeon_driver_irq_postinstall_kms(struct drm_device *dev)
{
	dev->max_vblank_count = 0x001fffff;
	return 0;
}

/**
 * radeon_driver_irq_uninstall_kms - drm irq uninstall callback
 *
 * @dev: drm dev pointer
 *
 * This function disables all interrupt sources on the GPU (all asics).
 */
void radeon_driver_irq_uninstall_kms(struct drm_device *dev)
{
	struct radeon_device *rdev = dev->dev_private;
	unsigned long irqflags;
	unsigned i;

	if (rdev == NULL) {
		return;
	}
	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	/* Disable *all* interrupts */
	for (i = 0; i < RADEON_NUM_RINGS; i++)
		atomic_set(&rdev->irq.ring_int[i], 0);
	for (i = 0; i < RADEON_MAX_HPD_PINS; i++)
		rdev->irq.hpd[i] = false;
	for (i = 0; i < RADEON_MAX_CRTCS; i++) {
		rdev->irq.crtc_vblank_int[i] = false;
		atomic_set(&rdev->irq.pflip[i], 0);
		rdev->irq.afmt[i] = false;
	}
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
}



/**
 * radeon_irq_kms_init - init driver interrupt info
 *
 * @rdev: radeon device pointer
 *
 * Sets up the work irq handlers, vblank init, MSIs, etc. (all asics).
 * Returns 0 for success, error for failure.
 */
int radeon_irq_kms_init(struct radeon_device *rdev)
{
    int irq_line;
	int r = 0;

    ENTER();


	spin_lock_init(&rdev->irq.lock);
	/* enable msi */
	rdev->msi_enabled = 0;

    rdev->irq.installed = true;
	r = drm_irq_install(rdev->ddev);
    if (r) {
       rdev->irq.installed = false;
       FAIL();
       return r;
   }
	DRM_INFO("radeon: irq initialized.\n");
	return 0;
}

/**
 * radeon_irq_kms_fini - tear down driver interrupt info
 *
 * @rdev: radeon device pointer
 *
 * Tears down the work irq handlers, vblank handlers, MSIs, etc. (all asics).
 */
void radeon_irq_kms_fini(struct radeon_device *rdev)
{
//	drm_vblank_cleanup(rdev->ddev);
	if (rdev->irq.installed) {
//		drm_irq_uninstall(rdev->ddev);
		rdev->irq.installed = false;
//       if (rdev->msi_enabled)
//			pci_disable_msi(rdev->pdev);
	}
//	flush_work(&rdev->hotplug_work);
}

/**
 * radeon_irq_kms_sw_irq_get - enable software interrupt
 *
 * @rdev: radeon device pointer
 * @ring: ring whose interrupt you want to enable
 *
 * Enables the software interrupt for a specific ring (all asics).
 * The software interrupt is generally used to signal a fence on
 * a particular ring.
 */
void radeon_irq_kms_sw_irq_get(struct radeon_device *rdev, int ring)
{
	unsigned long irqflags;

	if (!rdev->ddev->irq_enabled)
		return;

	if (atomic_inc_return(&rdev->irq.ring_int[ring]) == 1) {
		spin_lock_irqsave(&rdev->irq.lock, irqflags);
		radeon_irq_set(rdev);
		spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
	}
}

/**
 * radeon_irq_kms_sw_irq_put - disable software interrupt
 *
 * @rdev: radeon device pointer
 * @ring: ring whose interrupt you want to disable
 *
 * Disables the software interrupt for a specific ring (all asics).
 * The software interrupt is generally used to signal a fence on
 * a particular ring.
 */
void radeon_irq_kms_sw_irq_put(struct radeon_device *rdev, int ring)
{
	unsigned long irqflags;

	if (!rdev->ddev->irq_enabled)
		return;

	if (atomic_dec_and_test(&rdev->irq.ring_int[ring])) {
		spin_lock_irqsave(&rdev->irq.lock, irqflags);
		radeon_irq_set(rdev);
		spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
	}
}

/**
 * radeon_irq_kms_enable_hpd - enable hotplug detect interrupt
 *
 * @rdev: radeon device pointer
 * @hpd_mask: mask of hpd pins you want to enable.
 *
 * Enables the hotplug detect interrupt for a specific hpd pin (all asics).
 */
void radeon_irq_kms_enable_hpd(struct radeon_device *rdev, unsigned hpd_mask)
{
	unsigned long irqflags;
	int i;

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	for (i = 0; i < RADEON_MAX_HPD_PINS; ++i)
		rdev->irq.hpd[i] |= !!(hpd_mask & (1 << i));
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
}

/**
 * radeon_irq_kms_disable_hpd - disable hotplug detect interrupt
 *
 * @rdev: radeon device pointer
 * @hpd_mask: mask of hpd pins you want to disable.
 *
 * Disables the hotplug detect interrupt for a specific hpd pin (all asics).
 */
void radeon_irq_kms_disable_hpd(struct radeon_device *rdev, unsigned hpd_mask)
{
	unsigned long irqflags;
	int i;

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	for (i = 0; i < RADEON_MAX_HPD_PINS; ++i)
		rdev->irq.hpd[i] &= !(hpd_mask & (1 << i));
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
}


static struct drm_driver drm_driver = {
    .irq_preinstall = radeon_driver_irq_preinstall_kms,
    .irq_postinstall = radeon_driver_irq_postinstall_kms,
    .irq_handler = radeon_driver_irq_handler_kms
};

static struct drm_driver *driver = &drm_driver;

int drm_irq_install(struct drm_device *dev)
{
    unsigned long sh_flags = 0;
    int irq_line;
    int ret = 0;

    char *irqname;

    mutex_lock(&dev->struct_mutex);

    /* Driver must have been initialized */
    if (!dev->dev_private) {
            mutex_unlock(&dev->struct_mutex);
            return -EINVAL;
    }

    if (dev->irq_enabled) {
            mutex_unlock(&dev->struct_mutex);
            return -EBUSY;
    }
    dev->irq_enabled = 1;
    mutex_unlock(&dev->struct_mutex);

    irq_line   = drm_dev_to_irq(dev);

    DRM_DEBUG("irq=%d\n", drm_dev_to_irq(dev));

    /* Before installing handler */
    if (driver->irq_preinstall)
            driver->irq_preinstall(dev);

    ret = AttachIntHandler(irq_line, driver->irq_handler, (u32)dev);

    /* After installing handler */
    if (driver->irq_postinstall)
            ret = driver->irq_postinstall(dev);

    if (ret < 0) {
            DRM_ERROR(__FUNCTION__);
    }

    u16_t cmd = PciRead16(dev->pdev->busnr, dev->pdev->devfn, 4);
    cmd&= ~(1<<10);
    PciWrite16(dev->pdev->busnr, dev->pdev->devfn, 4, cmd);

    return ret;
}
