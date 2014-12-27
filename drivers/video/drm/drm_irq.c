/*
 * drm_irq.c IRQ and vblank support
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
//#include "drm_trace.h"
#include "drm_internal.h"

//#include <linux/interrupt.h>   /* For task queue support */
#include <linux/slab.h>

#include <linux/vgaarb.h>
#include <linux/export.h>

/* Access macro for slots in vblank timestamp ringbuffer. */
#define vblanktimestamp(dev, crtc, count) \
	((dev)->vblank[crtc].time[(count) % DRM_VBLANKTIME_RBSIZE])

/* Retry timestamp calculation up to 3 times to satisfy
 * drm_timestamp_precision before giving up.
 */
#define DRM_TIMESTAMP_MAXRETRIES 3

/* Threshold in nanoseconds for detection of redundant
 * vblank irq in drm_handle_vblank(). 1 msec should be ok.
 */
#define DRM_REDUNDANT_VBLIRQ_THRESH_NS 1000000

static bool
drm_get_last_vbltimestamp(struct drm_device *dev, int crtc,
			  struct timeval *tvblank, unsigned flags);

static unsigned int drm_timestamp_precision = 20;  /* Default to 20 usecs. */

/*
 * Clear vblank timestamp buffer for a crtc.
 */


#if 0
/**
 * drm_vblank_init - initialize vblank support
 * @dev: drm_device
 * @num_crtcs: number of crtcs supported by @dev
 *
 * This function initializes vblank support for @num_crtcs display pipelines.
 *
 * Returns:
 * Zero on success or a negative error code on failure.
 */
int drm_vblank_init(struct drm_device *dev, int num_crtcs)
{
	int i, ret = -ENOMEM;

	spin_lock_init(&dev->vbl_lock);
	spin_lock_init(&dev->vblank_time_lock);

	dev->num_crtcs = num_crtcs;

	dev->vblank = kcalloc(num_crtcs, sizeof(*dev->vblank), GFP_KERNEL);
	if (!dev->vblank)
		goto err;

	for (i = 0; i < num_crtcs; i++) {
		struct drm_vblank_crtc *vblank = &dev->vblank[i];

		vblank->dev = dev;
		vblank->crtc = i;
		init_waitqueue_head(&vblank->queue);
		setup_timer(&vblank->disable_timer, vblank_disable_fn,
			    (unsigned long)vblank);
	}

	DRM_INFO("Supports vblank timestamp caching Rev 2 (21.10.2013).\n");

	/* Driver specific high-precision vblank timestamping supported? */
	if (dev->driver->get_vblank_timestamp)
		DRM_INFO("Driver supports precise vblank timestamp query.\n");
	else
		DRM_INFO("No driver support for vblank timestamp query.\n");

	dev->vblank_disable_allowed = false;

	return 0;

err:
	dev->num_crtcs = 0;
	return ret;
}
EXPORT_SYMBOL(drm_vblank_init);

#endif

irqreturn_t device_irq_handler(struct drm_device *dev)
{

//    printf("video irq\n");

//    printf("device %p driver %p handler %p\n", dev, dev->driver, dev->driver->irq_handler) ;

    return dev->driver->irq_handler(0, dev);
}

/**
 * drm_irq_install - install IRQ handler
 * @dev: DRM device
 * @irq: IRQ number to install the handler for
 *
 * Initializes the IRQ related data. Installs the handler, calling the driver
 * irq_preinstall() and irq_postinstall() functions before and after the
 * installation.
 *
 * This is the simplified helper interface provided for drivers with no special
 * needs. Drivers which need to install interrupt handlers for multiple
 * interrupts must instead set drm_device->irq_enabled to signal the DRM core
 * that vblank interrupts are available.
 *
 * Returns:
 * Zero on success or a negative error code on failure.
 */
int drm_irq_install(struct drm_device *dev, int irq)
{
	int ret;
    unsigned long sh_flags = 0;

	if (!drm_core_check_feature(dev, DRIVER_HAVE_IRQ))
		return -EINVAL;

	if (irq == 0)
		return -EINVAL;

    /* Driver must have been initialized */
	if (!dev->dev_private)
            return -EINVAL;

	if (dev->irq_enabled)
            return -EBUSY;
	dev->irq_enabled = true;

	DRM_DEBUG("irq=%d\n", irq);

    /* Before installing handler */
    if (dev->driver->irq_preinstall)
            dev->driver->irq_preinstall(dev);

    ret = !AttachIntHandler(irq, device_irq_handler, (u32)dev);

    /* After installing handler */
    if (dev->driver->irq_postinstall)
            ret = dev->driver->irq_postinstall(dev);

    if (ret < 0) {
		dev->irq_enabled = false;
        DRM_ERROR(__FUNCTION__);
	} else {
		dev->irq = irq;
    }

    u16 cmd = PciRead16(dev->pdev->busnr, dev->pdev->devfn, 4);
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
 * drm_calc_timestamping_constants - calculate vblank timestamp constants
 * @crtc: drm_crtc whose timestamp constants should be updated.
 * @mode: display mode containing the scanout timings
 *
 * Calculate and store various constants which are later
 * needed by vblank and swap-completion timestamping, e.g,
 * by drm_calc_vbltimestamp_from_scanoutpos(). They are
 * derived from CRTC's true scanout timing, so they take
 * things like panel scaling or other adjustments into account.
 */
void drm_calc_timestamping_constants(struct drm_crtc *crtc,
				     const struct drm_display_mode *mode)
{
	int linedur_ns = 0, pixeldur_ns = 0, framedur_ns = 0;
	int dotclock = mode->crtc_clock;

	/* Valid dotclock? */
	if (dotclock > 0) {
		int frame_size = mode->crtc_htotal * mode->crtc_vtotal;

		/*
		 * Convert scanline length in pixels and video
		 * dot clock to line duration, frame duration
		 * and pixel duration in nanoseconds:
	 */
		pixeldur_ns = 1000000 / dotclock;
		linedur_ns  = div_u64((u64) mode->crtc_htotal * 1000000, dotclock);
		framedur_ns = div_u64((u64) frame_size * 1000000, dotclock);

		/*
		 * Fields of interlaced scanout modes are only half a frame duration.
		 */
		if (mode->flags & DRM_MODE_FLAG_INTERLACE)
			framedur_ns /= 2;
	} else
		DRM_ERROR("crtc %d: Can't calculate constants, dotclock = 0!\n",
			  crtc->base.id);

	crtc->pixeldur_ns = pixeldur_ns;
	crtc->linedur_ns  = linedur_ns;
	crtc->framedur_ns = framedur_ns;

	DRM_DEBUG("crtc %d: hwmode: htotal %d, vtotal %d, vdisplay %d\n",
		  crtc->base.id, mode->crtc_htotal,
		  mode->crtc_vtotal, mode->crtc_vdisplay);
	DRM_DEBUG("crtc %d: clock %d kHz framedur %d linedur %d, pixeldur %d\n",
		  crtc->base.id, dotclock, framedur_ns,
		  linedur_ns, pixeldur_ns);
}
EXPORT_SYMBOL(drm_calc_timestamping_constants);

/**
 * drm_calc_vbltimestamp_from_scanoutpos - precise vblank timestamp helper
 * @dev: DRM device
 * @crtc: Which CRTC's vblank timestamp to retrieve
 * @max_error: Desired maximum allowable error in timestamps (nanosecs)
 *             On return contains true maximum error of timestamp
 * @vblank_time: Pointer to struct timeval which should receive the timestamp
 * @flags: Flags to pass to driver:
 *         0 = Default,
 *         DRM_CALLED_FROM_VBLIRQ = If function is called from vbl IRQ handler
 * @refcrtc: CRTC which defines scanout timing
 * @mode: mode which defines the scanout timings
 *
 * Implements calculation of exact vblank timestamps from given drm_display_mode
 * timings and current video scanout position of a CRTC. This can be called from
 * within get_vblank_timestamp() implementation of a kms driver to implement the
 * actual timestamping.
 *
 * Should return timestamps conforming to the OML_sync_control OpenML
 * extension specification. The timestamp corresponds to the end of
 * the vblank interval, aka start of scanout of topmost-leftmost display
 * pixel in the following video frame.
 *
 * Requires support for optional dev->driver->get_scanout_position()
 * in kms driver, plus a bit of setup code to provide a drm_display_mode
 * that corresponds to the true scanout timing.
 *
 * The current implementation only handles standard video modes. It
 * returns as no operation if a doublescan or interlaced video mode is
 * active. Higher level code is expected to handle this.
 *
 * Returns:
 * Negative value on error, failure or if not supported in current
 * video mode:
 *
 * -EINVAL   - Invalid CRTC.
 * -EAGAIN   - Temporary unavailable, e.g., called before initial modeset.
 * -ENOTSUPP - Function not supported in current display mode.
 * -EIO      - Failed, e.g., due to failed scanout position query.
 *
 * Returns or'ed positive status flags on success:
 *
 * DRM_VBLANKTIME_SCANOUTPOS_METHOD - Signal this method used for timestamping.
 * DRM_VBLANKTIME_INVBL - Timestamp taken while scanout was in vblank interval.
 *
 */
int drm_calc_vbltimestamp_from_scanoutpos(struct drm_device *dev, int crtc,
					  int *max_error,
					  struct timeval *vblank_time,
					  unsigned flags,
					  const struct drm_crtc *refcrtc,
					  const struct drm_display_mode *mode)
{
	struct timeval tv_etime;
	int vbl_status;
	int vpos, hpos, i;
	int framedur_ns, linedur_ns, pixeldur_ns, delta_ns, duration_ns;
	bool invbl;

	if (crtc < 0 || crtc >= dev->num_crtcs) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return -EINVAL;
	}

	/* Scanout position query not supported? Should not happen. */
	if (!dev->driver->get_scanout_position) {
		DRM_ERROR("Called from driver w/o get_scanout_position()!?\n");
		return -EIO;
	}

	/* Durations of frames, lines, pixels in nanoseconds. */
	framedur_ns = refcrtc->framedur_ns;
	linedur_ns  = refcrtc->linedur_ns;
	pixeldur_ns = refcrtc->pixeldur_ns;

	/* If mode timing undefined, just return as no-op:
	 * Happens during initial modesetting of a crtc.
	 */
	if (framedur_ns == 0) {
		DRM_DEBUG("crtc %d: Noop due to uninitialized mode.\n", crtc);
		return -EAGAIN;
	}

	return -EIO;
}
EXPORT_SYMBOL(drm_calc_vbltimestamp_from_scanoutpos);

/**
 * drm_vblank_get - get a reference count on vblank events
 * @dev: DRM device
 * @crtc: which CRTC to own
 *
 * Acquire a reference count on vblank events to avoid having them disabled
 * while in use.
 *
 * This is the legacy version of drm_crtc_vblank_get().
 *
 * Returns:
 * Zero on success, nonzero on failure.
 */
int drm_vblank_get(struct drm_device *dev, int crtc)
{
	struct drm_vblank_crtc *vblank = &dev->vblank[crtc];
	unsigned long irqflags;
	int ret = 0;
#if 0

	if (WARN_ON(crtc >= dev->num_crtcs))
		return -EINVAL;

	spin_lock_irqsave(&dev->vbl_lock, irqflags);
	/* Going from 0->1 means we have to enable interrupts again */
	if (atomic_add_return(1, &vblank->refcount) == 1) {
		ret = drm_vblank_enable(dev, crtc);
	} else {
		if (!vblank->enabled) {
			atomic_dec(&vblank->refcount);
			ret = -EINVAL;
		}
	}
	spin_unlock_irqrestore(&dev->vbl_lock, irqflags);
#endif
	return ret;
}
EXPORT_SYMBOL(drm_vblank_get);

/**
 * drm_crtc_vblank_get - get a reference count on vblank events
 * @crtc: which CRTC to own
 *
 * Acquire a reference count on vblank events to avoid having them disabled
 * while in use.
 *
 * This is the native kms version of drm_vblank_off().
 *
 * Returns:
 * Zero on success, nonzero on failure.
 */
int drm_crtc_vblank_get(struct drm_crtc *crtc)
{
	return drm_vblank_get(crtc->dev, drm_crtc_index(crtc));
}
EXPORT_SYMBOL(drm_crtc_vblank_get);

/**
 * drm_vblank_put - give up ownership of vblank events
 * @dev: DRM device
 * @crtc: which counter to give up
 *
 * Release ownership of a given vblank counter, turning off interrupts
 * if possible. Disable interrupts after drm_vblank_offdelay milliseconds.
 *
 * This is the legacy version of drm_crtc_vblank_put().
 */
void drm_vblank_put(struct drm_device *dev, int crtc)
{
#if 0
	struct drm_vblank_crtc *vblank = &dev->vblank[crtc];

	if (WARN_ON(atomic_read(&vblank->refcount) == 0))
		return;

	if (WARN_ON(crtc >= dev->num_crtcs))
		return;

	/* Last user schedules interrupt disable */
	if (atomic_dec_and_test(&vblank->refcount)) {
		if (drm_vblank_offdelay == 0)
			return;
		else if (dev->vblank_disable_immediate || drm_vblank_offdelay < 0)
			vblank_disable_fn((unsigned long)vblank);
		else
			mod_timer(&vblank->disable_timer,
				  jiffies + ((drm_vblank_offdelay * HZ)/1000));
	}
#endif
}
EXPORT_SYMBOL(drm_vblank_put);

/**
 * drm_crtc_vblank_put - give up ownership of vblank events
 * @crtc: which counter to give up
 *
 * Release ownership of a given vblank counter, turning off interrupts
 * if possible. Disable interrupts after drm_vblank_offdelay milliseconds.
 *
 * This is the native kms version of drm_vblank_put().
 */
void drm_crtc_vblank_put(struct drm_crtc *crtc)
{
	drm_vblank_put(crtc->dev, drm_crtc_index(crtc));
}
EXPORT_SYMBOL(drm_crtc_vblank_put);

/**
 * drm_wait_one_vblank - wait for one vblank
 * @dev: DRM device
 * @crtc: crtc index
 *
 * This waits for one vblank to pass on @crtc, using the irq driver interfaces.
 * It is a failure to call this when the vblank irq for @crtc is disabled, e.g.
 * due to lack of driver support or because the crtc is off.
 */
void drm_wait_one_vblank(struct drm_device *dev, int crtc)
{
#if 0
	int ret;
	u32 last;

	ret = drm_vblank_get(dev, crtc);
	if (WARN(ret, "vblank not available on crtc %i, ret=%i\n", crtc, ret))
		return;

	last = drm_vblank_count(dev, crtc);

	ret = wait_event_timeout(dev->vblank[crtc].queue,
				 last != drm_vblank_count(dev, crtc),
				 msecs_to_jiffies(100));

	WARN(ret == 0, "vblank wait timed out on crtc %i\n", crtc);

	drm_vblank_put(dev, crtc);
#endif
}
EXPORT_SYMBOL(drm_wait_one_vblank);

/**
 * drm_crtc_wait_one_vblank - wait for one vblank
 * @crtc: DRM crtc
 *
 * This waits for one vblank to pass on @crtc, using the irq driver interfaces.
 * It is a failure to call this when the vblank irq for @crtc is disabled, e.g.
 * due to lack of driver support or because the crtc is off.
 */
void drm_crtc_wait_one_vblank(struct drm_crtc *crtc)
{
	drm_wait_one_vblank(crtc->dev, drm_crtc_index(crtc));
}
EXPORT_SYMBOL(drm_crtc_wait_one_vblank);

/**
 * drm_vblank_off - disable vblank events on a CRTC
 * @dev: DRM device
 * @crtc: CRTC in question
 *
 * Drivers can use this function to shut down the vblank interrupt handling when
 * disabling a crtc. This function ensures that the latest vblank frame count is
 * stored so that drm_vblank_on() can restore it again.
 *
 * Drivers must use this function when the hardware vblank counter can get
 * reset, e.g. when suspending.
 *
 * This is the legacy version of drm_crtc_vblank_off().
 */
void drm_vblank_off(struct drm_device *dev, int crtc)
{
	struct drm_vblank_crtc *vblank = &dev->vblank[crtc];
	struct drm_pending_vblank_event *e, *t;
	struct timeval now;
	unsigned long irqflags;
	unsigned int seq;


}
EXPORT_SYMBOL(drm_vblank_off);

/**
 * drm_crtc_vblank_off - disable vblank events on a CRTC
 * @crtc: CRTC in question
 *
 * Drivers can use this function to shut down the vblank interrupt handling when
 * disabling a crtc. This function ensures that the latest vblank frame count is
 * stored so that drm_vblank_on can restore it again.
 *
 * Drivers must use this function when the hardware vblank counter can get
 * reset, e.g. when suspending.
 *
 * This is the native kms version of drm_vblank_off().
 */
void drm_crtc_vblank_off(struct drm_crtc *crtc)
{
	drm_vblank_off(crtc->dev, drm_crtc_index(crtc));
}
EXPORT_SYMBOL(drm_crtc_vblank_off);

/**
 * drm_vblank_on - enable vblank events on a CRTC
 * @dev: DRM device
 * @crtc: CRTC in question
 *
 * This functions restores the vblank interrupt state captured with
 * drm_vblank_off() again. Note that calls to drm_vblank_on() and
 * drm_vblank_off() can be unbalanced and so can also be unconditionally called
 * in driver load code to reflect the current hardware state of the crtc.
 *
 * This is the legacy version of drm_crtc_vblank_on().
 */
void drm_vblank_on(struct drm_device *dev, int crtc)
{
	struct drm_vblank_crtc *vblank = &dev->vblank[crtc];
	unsigned long irqflags;

}
EXPORT_SYMBOL(drm_vblank_on);

/**
 * drm_crtc_vblank_on - enable vblank events on a CRTC
 * @crtc: CRTC in question
 *
 * This functions restores the vblank interrupt state captured with
 * drm_vblank_off() again. Note that calls to drm_vblank_on() and
 * drm_vblank_off() can be unbalanced and so can also be unconditionally called
 * in driver load code to reflect the current hardware state of the crtc.
 *
 * This is the native kms version of drm_vblank_on().
 */
void drm_crtc_vblank_on(struct drm_crtc *crtc)
{
	drm_vblank_on(crtc->dev, drm_crtc_index(crtc));
}
EXPORT_SYMBOL(drm_crtc_vblank_on);

/**
 * drm_vblank_pre_modeset - account for vblanks across mode sets
 * @dev: DRM device
 * @crtc: CRTC in question
 *
 * Account for vblank events across mode setting events, which will likely
 * reset the hardware frame counter.
 *
 * This is done by grabbing a temporary vblank reference to ensure that the
 * vblank interrupt keeps running across the modeset sequence. With this the
 * software-side vblank frame counting will ensure that there are no jumps or
 * discontinuities.
 *
 * Unfortunately this approach is racy and also doesn't work when the vblank
 * interrupt stops running, e.g. across system suspend resume. It is therefore
 * highly recommended that drivers use the newer drm_vblank_off() and
 * drm_vblank_on() instead. drm_vblank_pre_modeset() only works correctly when
 * using "cooked" software vblank frame counters and not relying on any hardware
 * counters.
 *
 * Drivers must call drm_vblank_post_modeset() when re-enabling the same crtc
 * again.
 */
void drm_vblank_pre_modeset(struct drm_device *dev, int crtc)
{
#if 0
    /* vblank is not initialized (IRQ not installed ?) */
    if (!dev->num_crtcs)
        return;

	if (WARN_ON(crtc >= dev->num_crtcs))
		return;

    /*
     * To avoid all the problems that might happen if interrupts
     * were enabled/disabled around or between these calls, we just
     * have the kernel take a reference on the CRTC (just once though
     * to avoid corrupting the count if multiple, mismatch calls occur),
     * so that interrupts remain enabled in the interim.
     */
	if (!vblank->inmodeset) {
		vblank->inmodeset = 0x1;
        if (drm_vblank_get(dev, crtc) == 0)
			vblank->inmodeset |= 0x2;
    }
#endif
}
EXPORT_SYMBOL(drm_vblank_pre_modeset);

/**
 * drm_vblank_post_modeset - undo drm_vblank_pre_modeset changes
 * @dev: DRM device
 * @crtc: CRTC in question
 *
 * This function again drops the temporary vblank reference acquired in
 * drm_vblank_pre_modeset.
 */
void drm_vblank_post_modeset(struct drm_device *dev, int crtc)
{
#if 0
    unsigned long irqflags;

	/* vblank is not initialized (IRQ not installed ?), or has been freed */
	if (!dev->num_crtcs)
		return;

	if (vblank->inmodeset) {
        spin_lock_irqsave(&dev->vbl_lock, irqflags);
		dev->vblank_disable_allowed = true;
        spin_unlock_irqrestore(&dev->vbl_lock, irqflags);

		if (vblank->inmodeset & 0x2)
            drm_vblank_put(dev, crtc);

		vblank->inmodeset = 0;
    }
#endif
}
EXPORT_SYMBOL(drm_vblank_post_modeset);


