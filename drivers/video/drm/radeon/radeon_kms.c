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
#include "radeon.h"
#include <drm/radeon_drm.h>
#include "radeon_asic.h"

#include <linux/vga_switcheroo.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>

#include "radeon_kfd.h"

#if defined(CONFIG_VGA_SWITCHEROO)
bool radeon_has_atpx(void);
#else
static inline bool radeon_has_atpx(void) { return false; }
#endif


/*
 * VBlank related functions.
 */
/**
 * radeon_get_vblank_counter_kms - get frame count
 *
 * @dev: drm dev pointer
 * @pipe: crtc to get the frame count from
 *
 * Gets the frame count on the requested crtc (all asics).
 * Returns frame count on success, -EINVAL on failure.
 */
u32 radeon_get_vblank_counter_kms(struct drm_device *dev, unsigned int pipe)
{
	int vpos, hpos, stat;
	u32 count;
	struct radeon_device *rdev = dev->dev_private;

	if (pipe >= rdev->num_crtc) {
		DRM_ERROR("Invalid crtc %u\n", pipe);
		return -EINVAL;
	}

	/* The hw increments its frame counter at start of vsync, not at start
	 * of vblank, as is required by DRM core vblank counter handling.
	 * Cook the hw count here to make it appear to the caller as if it
	 * incremented at start of vblank. We measure distance to start of
	 * vblank in vpos. vpos therefore will be >= 0 between start of vblank
	 * and start of vsync, so vpos >= 0 means to bump the hw frame counter
	 * result by 1 to give the proper appearance to caller.
	 */
	if (rdev->mode_info.crtcs[pipe]) {
		/* Repeat readout if needed to provide stable result if
		 * we cross start of vsync during the queries.
		 */
		do {
			count = radeon_get_vblank_counter(rdev, pipe);
			/* Ask radeon_get_crtc_scanoutpos to return vpos as
			 * distance to start of vblank, instead of regular
			 * vertical scanout pos.
			 */
			stat = radeon_get_crtc_scanoutpos(
				dev, pipe, GET_DISTANCE_TO_VBLANKSTART,
				&vpos, &hpos, NULL, NULL,
				&rdev->mode_info.crtcs[pipe]->base.hwmode);
		} while (count != radeon_get_vblank_counter(rdev, pipe));

		if (((stat & (DRM_SCANOUTPOS_VALID | DRM_SCANOUTPOS_ACCURATE)) !=
		    (DRM_SCANOUTPOS_VALID | DRM_SCANOUTPOS_ACCURATE))) {
			DRM_DEBUG_VBL("Query failed! stat %d\n", stat);
		}
		else {
			DRM_DEBUG_VBL("crtc %u: dist from vblank start %d\n",
				      pipe, vpos);

			/* Bump counter if we are at >= leading edge of vblank,
			 * but before vsync where vpos would turn negative and
			 * the hw counter really increments.
			 */
			if (vpos >= 0)
				count++;
		}
	}
	else {
	    /* Fallback to use value as is. */
	    count = radeon_get_vblank_counter(rdev, pipe);
	    DRM_DEBUG_VBL("NULL mode info! Returned count may be wrong.\n");
	}

	return count;
}

/**
 * radeon_enable_vblank_kms - enable vblank interrupt
 *
 * @dev: drm dev pointer
 * @crtc: crtc to enable vblank interrupt for
 *
 * Enable the interrupt on the requested crtc (all asics).
 * Returns 0 on success, -EINVAL on failure.
 */
int radeon_enable_vblank_kms(struct drm_device *dev, int crtc)
{
	struct radeon_device *rdev = dev->dev_private;
	unsigned long irqflags;
	int r;

	if (crtc < 0 || crtc >= rdev->num_crtc) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return -EINVAL;
	}

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	rdev->irq.crtc_vblank_int[crtc] = true;
	r = radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
	return r;
}

/**
 * radeon_disable_vblank_kms - disable vblank interrupt
 *
 * @dev: drm dev pointer
 * @crtc: crtc to disable vblank interrupt for
 *
 * Disable the interrupt on the requested crtc (all asics).
 */
void radeon_disable_vblank_kms(struct drm_device *dev, int crtc)
{
	struct radeon_device *rdev = dev->dev_private;
	unsigned long irqflags;

	if (crtc < 0 || crtc >= rdev->num_crtc) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return;
	}

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	rdev->irq.crtc_vblank_int[crtc] = false;
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
}

/**
 * radeon_get_vblank_timestamp_kms - get vblank timestamp
 *
 * @dev: drm dev pointer
 * @crtc: crtc to get the timestamp for
 * @max_error: max error
 * @vblank_time: time value
 * @flags: flags passed to the driver
 *
 * Gets the timestamp on the requested crtc based on the
 * scanout position.  (all asics).
 * Returns postive status flags on success, negative error on failure.
 */
int radeon_get_vblank_timestamp_kms(struct drm_device *dev, int crtc,
				    int *max_error,
				    struct timeval *vblank_time,
				    unsigned flags)
{
	struct drm_crtc *drmcrtc;
	struct radeon_device *rdev = dev->dev_private;

	if (crtc < 0 || crtc >= dev->num_crtcs) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return -EINVAL;
	}

	/* Get associated drm_crtc: */
	drmcrtc = &rdev->mode_info.crtcs[crtc]->base;
	if (!drmcrtc)
		return -EINVAL;

	/* Helper routine in DRM core does all the work: */
	return drm_calc_vbltimestamp_from_scanoutpos(dev, crtc, max_error,
						     vblank_time, flags,
						     &drmcrtc->hwmode);
}


