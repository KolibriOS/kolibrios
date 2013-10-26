/**
 * \file drm_irq.c
 * IRQ support
 *
 * \author Rickard E. (Rik) Faith <faith@valinux.com>
 * \author Gareth Hughes <gareth@valinux.com>
 */

/*
 * Created: Fri Mar 19 14:30:16 1999 by faith@valinux.com
 *
 * Copyright 1999, 2000 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <drm/drmP.h>
#include <asm/div64.h>
//#include "drm_trace.h"

//#include <linux/interrupt.h>   /* For task queue support */
#include <linux/slab.h>

//#include <linux/vgaarb.h>
#include <linux/export.h>

/* Access macro for slots in vblank timestamp ringbuffer. */
#define vblanktimestamp(dev, crtc, count) ( \
	(dev)->_vblank_time[(crtc) * DRM_VBLANKTIME_RBSIZE + \
	((count) % DRM_VBLANKTIME_RBSIZE)])

/* Retry timestamp calculation up to 3 times to satisfy
 * drm_timestamp_precision before giving up.
 */
#define DRM_TIMESTAMP_MAXRETRIES 3

/* Threshold in nanoseconds for detection of redundant
 * vblank irq in drm_handle_vblank(). 1 msec should be ok.
 */
#define DRM_REDUNDANT_VBLIRQ_THRESH_NS 1000000


irqreturn_t device_irq_handler(struct drm_device *dev)
{

//    printf("video irq\n");

//    printf("device %p driver %p handler %p\n", dev, dev->driver, dev->driver->irq_handler) ;

    return dev->driver->irq_handler(0, dev);
}

/**
 * Install IRQ handler.
 *
 * \param dev DRM device.
 *
 * Initializes the IRQ related data. Installs the handler, calling the driver
 * \c irq_preinstall() and \c irq_postinstall() functions
 * before and after the installation.
 */
int drm_irq_install(struct drm_device *dev)
{
	int ret;
    unsigned long sh_flags = 0;
	char *irqname;


	if (drm_dev_to_irq(dev) == 0)
		return -EINVAL;

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

    DRM_DEBUG("irq=%d\n", drm_dev_to_irq(dev));

    /* Before installing handler */
    if (dev->driver->irq_preinstall)
            dev->driver->irq_preinstall(dev);

    ret = !AttachIntHandler(drm_dev_to_irq(dev), device_irq_handler, (u32)dev);

    /* After installing handler */
    if (dev->driver->irq_postinstall)
            ret = dev->driver->irq_postinstall(dev);

    if (ret < 0) {
		dev->irq_enabled = 0;
            DRM_ERROR(__FUNCTION__);
    }

    u16_t cmd = PciRead16(dev->pdev->busnr, dev->pdev->devfn, 4);
    cmd&= ~(1<<10);
    PciWrite16(dev->pdev->busnr, dev->pdev->devfn, 4, cmd);

    return ret;
}
EXPORT_SYMBOL(drm_irq_install);




u64 div64_u64(u64 dividend, u64 divisor)
{
        u32 high, d;

        high = divisor >> 32;
        if (high) {
                unsigned int shift = fls(high);

                d = divisor >> shift;
                dividend >>= shift;
        } else
                d = divisor;

        return div_u64(dividend, d);
}

/**
 * drm_calc_timestamping_constants - Calculate and
 * store various constants which are later needed by
 * vblank and swap-completion timestamping, e.g, by
 * drm_calc_vbltimestamp_from_scanoutpos().
 * They are derived from crtc's true scanout timing,
 * so they take things like panel scaling or other
 * adjustments into account.
 *
 * @crtc drm_crtc whose timestamp constants should be updated.
 *
 */
void drm_calc_timestamping_constants(struct drm_crtc *crtc)
{
	s64 linedur_ns = 0, pixeldur_ns = 0, framedur_ns = 0;
	u64 dotclock;

	/* Dot clock in Hz: */
	dotclock = (u64) crtc->hwmode.clock * 1000;

	/* Fields of interlaced scanout modes are only halve a frame duration.
	 * Double the dotclock to get halve the frame-/line-/pixelduration.
	 */
	if (crtc->hwmode.flags & DRM_MODE_FLAG_INTERLACE)
		dotclock *= 2;

	/* Valid dotclock? */
	if (dotclock > 0) {
		int frame_size;
		/* Convert scanline length in pixels and video dot clock to
		 * line duration, frame duration and pixel duration in
		 * nanoseconds:
		 */
		pixeldur_ns = (s64) div64_u64(1000000000, dotclock);
		linedur_ns  = (s64) div64_u64(((u64) crtc->hwmode.crtc_htotal *
					      1000000000), dotclock);
		frame_size = crtc->hwmode.crtc_htotal *
				crtc->hwmode.crtc_vtotal;
		framedur_ns = (s64) div64_u64((u64) frame_size * 1000000000,
					      dotclock);
	} else
		DRM_ERROR("crtc %d: Can't calculate constants, dotclock = 0!\n",
			  crtc->base.id);

	crtc->pixeldur_ns = pixeldur_ns;
	crtc->linedur_ns  = linedur_ns;
	crtc->framedur_ns = framedur_ns;

	DRM_DEBUG("crtc %d: hwmode: htotal %d, vtotal %d, vdisplay %d\n",
		  crtc->base.id, crtc->hwmode.crtc_htotal,
		  crtc->hwmode.crtc_vtotal, crtc->hwmode.crtc_vdisplay);
	DRM_DEBUG("crtc %d: clock %d kHz framedur %d linedur %d, pixeldur %d\n",
		  crtc->base.id, (int) dotclock/1000, (int) framedur_ns,
		  (int) linedur_ns, (int) pixeldur_ns);
}


/**
 * drm_vblank_pre_modeset - account for vblanks across mode sets
 * @dev: DRM device
 * @crtc: CRTC in question
 *
 * Account for vblank events across mode setting events, which will likely
 * reset the hardware frame counter.
 */
void drm_vblank_pre_modeset(struct drm_device *dev, int crtc)
{
#if 0
    /* vblank is not initialized (IRQ not installed ?) */
    if (!dev->num_crtcs)
        return;
    /*
     * To avoid all the problems that might happen if interrupts
     * were enabled/disabled around or between these calls, we just
     * have the kernel take a reference on the CRTC (just once though
     * to avoid corrupting the count if multiple, mismatch calls occur),
     * so that interrupts remain enabled in the interim.
     */
    if (!dev->vblank_inmodeset[crtc]) {
        dev->vblank_inmodeset[crtc] = 0x1;
        if (drm_vblank_get(dev, crtc) == 0)
            dev->vblank_inmodeset[crtc] |= 0x2;
    }
#endif
}
EXPORT_SYMBOL(drm_vblank_pre_modeset);

void drm_vblank_post_modeset(struct drm_device *dev, int crtc)
{
#if 0
    unsigned long irqflags;

	/* vblank is not initialized (IRQ not installed ?), or has been freed */
	if (!dev->num_crtcs)
		return;

    if (dev->vblank_inmodeset[crtc]) {
        spin_lock_irqsave(&dev->vbl_lock, irqflags);
        dev->vblank_disable_allowed = 1;
        spin_unlock_irqrestore(&dev->vbl_lock, irqflags);

        if (dev->vblank_inmodeset[crtc] & 0x2)
            drm_vblank_put(dev, crtc);

        dev->vblank_inmodeset[crtc] = 0;
    }
#endif
}
EXPORT_SYMBOL(drm_vblank_post_modeset);
