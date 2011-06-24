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

#include "drmP.h"
#include <asm/div64.h>
//#include "drm_trace.h"

//#include <linux/interrupt.h>   /* For task queue support */
#include <linux/slab.h>

//#include <linux/vgaarb.h>

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


static inline u64 div_u64(u64 dividend, u32 divisor)
{
        u32 remainder;
        return div_u64_rem(dividend, divisor, &remainder);
}


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
		/* Convert scanline length in pixels and video dot clock to
		 * line duration, frame duration and pixel duration in
		 * nanoseconds:
		 */
		pixeldur_ns = (s64) div64_u64(1000000000, dotclock);
		linedur_ns  = (s64) div64_u64(((u64) crtc->hwmode.crtc_htotal *
					      1000000000), dotclock);
		framedur_ns = (s64) crtc->hwmode.crtc_vtotal * linedur_ns;
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

