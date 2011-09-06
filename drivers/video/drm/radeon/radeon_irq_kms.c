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
#include "drmP.h"
#include "drm_crtc_helper.h"
#include "radeon_drm.h"
#include "radeon_reg.h"
#include "radeon.h"
#include "atom.h"

struct radeon_device *main_device;

extern int irq_override;

void irq_handler_kms()
{
//    dbgprintf("%s\n",__FUNCTION__);
    radeon_irq_process(main_device);
}


static void radeon_irq_preinstall(struct radeon_device *rdev)
{
    unsigned i;

    /* Disable *all* interrupts */
    rdev->irq.sw_int = false;
    rdev->irq.gui_idle = false;
    for (i = 0; i < rdev->num_crtc; i++)
        rdev->irq.crtc_vblank_int[i] = false;
    for (i = 0; i < 6; i++) {
        rdev->irq.hpd[i] = false;
        rdev->irq.pflip[i] = false;
    }
    radeon_irq_set(rdev);
    /* Clear bits */
    radeon_irq_process(rdev);
}

int radeon_driver_irq_postinstall(struct radeon_device *rdev)
{
//    struct radeon_device *rdev = dev->dev_private;

//    dev->max_vblank_count = 0x001fffff;
    rdev->irq.sw_int = true;
    radeon_irq_set(rdev);
    return 0;
}

int radeon_irq_kms_init(struct radeon_device *rdev)
{
	int i;
    int irq_line;
	int r = 0;

    ENTER();

//   INIT_WORK(&rdev->hotplug_work, radeon_hotplug_work_func);

	spin_lock_init(&rdev->irq.sw_lock);
	for (i = 0; i < rdev->num_crtc; i++)
		spin_lock_init(&rdev->irq.pflip_lock[i]);
//   r = drm_vblank_init(rdev->ddev, rdev->num_crtc);
//   if (r) {
//       return r;
//   }

	rdev->msi_enabled = 0;
    rdev->irq.installed = true;
    main_device = rdev;

    radeon_irq_preinstall(rdev);

    if (irq_override)
        irq_line = irq_override;
    else
        irq_line = rdev->pdev->irq;

    dbgprintf("%s install irq %d\n", __FUNCTION__, irq_line);

    AttachIntHandler(irq_line, irq_handler_kms, 2);

//   r = drm_irq_install(rdev->ddev);

    r = radeon_driver_irq_postinstall(rdev);
    if (r) {
       rdev->irq.installed = false;
        LEAVE();
       return r;
   }

	DRM_INFO("radeon: irq initialized.\n");
	return 0;
}


void radeon_irq_kms_sw_irq_get(struct radeon_device *rdev)
{
	unsigned long irqflags;

	spin_lock_irqsave(&rdev->irq.sw_lock, irqflags);
	if (rdev->ddev->irq_enabled && (++rdev->irq.sw_refcount == 1)) {
		rdev->irq.sw_int = true;
		radeon_irq_set(rdev);
	}
	spin_unlock_irqrestore(&rdev->irq.sw_lock, irqflags);
}

void radeon_irq_kms_sw_irq_put(struct radeon_device *rdev)
{
	unsigned long irqflags;

	spin_lock_irqsave(&rdev->irq.sw_lock, irqflags);
	BUG_ON(rdev->ddev->irq_enabled && rdev->irq.sw_refcount <= 0);
	if (rdev->ddev->irq_enabled && (--rdev->irq.sw_refcount == 0)) {
		rdev->irq.sw_int = false;
		radeon_irq_set(rdev);
	}
	spin_unlock_irqrestore(&rdev->irq.sw_lock, irqflags);
}


