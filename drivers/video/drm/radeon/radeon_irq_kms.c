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

struct radeon_device *main_device;

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
void irq_handler_kms()
{
    radeon_irq_process(main_device);
}

/**
 * radeon_driver_irq_preinstall_kms - drm irq preinstall callback
 *
 * @dev: drm dev pointer
 *
 * Gets the hw ready to enable irqs (all asics).
 * This function disables all interrupt sources on the GPU.
 */
void radeon_irq_preinstall_kms(struct radeon_device *rdev)
{
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

int radeon_driver_irq_postinstall_kms(struct radeon_device *rdev)
{
//    struct radeon_device *rdev = dev->dev_private;

//    dev->max_vblank_count = 0x001fffff;

    radeon_irq_set(rdev);
    return 0;
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

//    INIT_WORK(&rdev->hotplug_work, radeon_hotplug_work_func);
//	INIT_WORK(&rdev->audio_work, r600_audio_update_hdmi);

	spin_lock_init(&rdev->irq.lock);
//   r = drm_vblank_init(rdev->ddev, rdev->num_crtc);
//   if (r) {
//       return r;
//   }
	/* enable msi */
	rdev->msi_enabled = 0;

//	if (radeon_msi_ok(rdev)) {
//		int ret = pci_enable_msi(rdev->pdev);
//		if (!ret) {
//			rdev->msi_enabled = 1;
//			dev_info(rdev->dev, "radeon: using MSI.\n");
//		}
//	}
    rdev->irq.installed = true;
    main_device = rdev;

    radeon_irq_preinstall_kms(rdev);

    if (irq_override)
        irq_line = irq_override;
    else
        irq_line = rdev->pdev->irq;

    dbgprintf("%s install irq %d\n", __FUNCTION__, irq_line);

    AttachIntHandler(irq_line, irq_handler_kms, 2);

//   r = drm_irq_install(rdev->ddev);

    r = radeon_driver_irq_postinstall_kms(rdev);
    if (r) {
       rdev->irq.installed = false;
        LEAVE();
       return r;
   }
	DRM_INFO("radeon: irq initialized.\n");
	return 0;
}

/**
 * radeon_irq_kms_fini - tear down driver interrrupt info
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


