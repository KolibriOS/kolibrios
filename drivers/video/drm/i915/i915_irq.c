/* i915_irq.c -- IRQ support for the I915 -*- linux-c -*-
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

#include <linux/irqreturn.h>
//#include <linux/slab.h>
#include "drmP.h"
#include "drm.h"
#include "i915_drm.h"
#include "i915_drv.h"
#include "i915_trace.h"
#include "intel_drv.h"

#define DRM_WAKEUP( queue ) wake_up( queue )
#define DRM_INIT_WAITQUEUE( queue ) init_waitqueue_head( queue )

#define MAX_NOPID ((u32)~0)

/**
 * Interrupts that are always left unmasked.
 *
 * Since pipe events are edge-triggered from the PIPESTAT register to IIR,
 * we leave them always unmasked in IMR and then control enabling them through
 * PIPESTAT alone.
 */
#define I915_INTERRUPT_ENABLE_FIX			\
	(I915_ASLE_INTERRUPT |				\
	 I915_DISPLAY_PIPE_A_EVENT_INTERRUPT |		\
	 I915_DISPLAY_PIPE_B_EVENT_INTERRUPT |		\
	 I915_DISPLAY_PLANE_A_FLIP_PENDING_INTERRUPT |	\
	 I915_DISPLAY_PLANE_B_FLIP_PENDING_INTERRUPT |	\
	 I915_RENDER_COMMAND_PARSER_ERROR_INTERRUPT)

/** Interrupts that we mask and unmask at runtime. */
#define I915_INTERRUPT_ENABLE_VAR (I915_USER_INTERRUPT | I915_BSD_USER_INTERRUPT)

#define I915_PIPE_VBLANK_STATUS	(PIPE_START_VBLANK_INTERRUPT_STATUS |\
				 PIPE_VBLANK_INTERRUPT_STATUS)

#define I915_PIPE_VBLANK_ENABLE	(PIPE_START_VBLANK_INTERRUPT_ENABLE |\
				 PIPE_VBLANK_INTERRUPT_ENABLE)

#define DRM_I915_VBLANK_PIPE_ALL	(DRM_I915_VBLANK_PIPE_A | \
					 DRM_I915_VBLANK_PIPE_B)

/* For display hotplug interrupt */
static void
ironlake_enable_display_irq(drm_i915_private_t *dev_priv, u32 mask)
{
    if ((dev_priv->irq_mask & mask) != 0) {
        dev_priv->irq_mask &= ~mask;
        I915_WRITE(DEIMR, dev_priv->irq_mask);
        POSTING_READ(DEIMR);
    }
}

static inline void
ironlake_disable_display_irq(drm_i915_private_t *dev_priv, u32 mask)
{
    if ((dev_priv->irq_mask & mask) != mask) {
        dev_priv->irq_mask |= mask;
        I915_WRITE(DEIMR, dev_priv->irq_mask);
        POSTING_READ(DEIMR);
    }
}
static void notify_ring(struct drm_device *dev,
			struct intel_ring_buffer *ring)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	u32 seqno;

	if (ring->obj == NULL)
		return;

	seqno = ring->get_seqno(ring);
	trace_i915_gem_request_complete(ring, seqno);

	ring->irq_seqno = seqno;
	wake_up_all(&ring->irq_queue);
//   if (i915_enable_hangcheck) {
//       dev_priv->hangcheck_count = 0;
//       mod_timer(&dev_priv->hangcheck_timer,
//             jiffies +
//             msecs_to_jiffies(DRM_I915_HANGCHECK_PERIOD));
//   }
}



static int ironlake_irq_handler(struct drm_device *dev)
{
    drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
    int ret = IRQ_NONE;
    u32 de_iir, gt_iir, de_ier, pch_iir, pm_iir;
    u32 hotplug_mask;
    struct drm_i915_master_private *master_priv;
    u32 bsd_usr_interrupt = GT_BSD_USER_INTERRUPT;

    atomic_inc(&dev_priv->irq_received);

    if (IS_GEN6(dev))
        bsd_usr_interrupt = GT_GEN6_BSD_USER_INTERRUPT;

    /* disable master interrupt before clearing iir  */
    de_ier = I915_READ(DEIER);
    I915_WRITE(DEIER, de_ier & ~DE_MASTER_IRQ_CONTROL);
    POSTING_READ(DEIER);

    de_iir = I915_READ(DEIIR);
    gt_iir = I915_READ(GTIIR);
    pch_iir = I915_READ(SDEIIR);
    pm_iir = I915_READ(GEN6_PMIIR);

    if (de_iir == 0 && gt_iir == 0 && pch_iir == 0 &&
        (!IS_GEN6(dev) || pm_iir == 0))
        goto done;

    if (HAS_PCH_CPT(dev))
        hotplug_mask = SDE_HOTPLUG_MASK_CPT;
    else
        hotplug_mask = SDE_HOTPLUG_MASK;

    ret = IRQ_HANDLED;


    if (gt_iir & (GT_USER_INTERRUPT | GT_PIPE_NOTIFY))
        notify_ring(dev, &dev_priv->ring[RCS]);
    if (gt_iir & bsd_usr_interrupt)
        notify_ring(dev, &dev_priv->ring[VCS]);
    if (gt_iir & GT_BLT_USER_INTERRUPT)
        notify_ring(dev, &dev_priv->ring[BCS]);

//    if (de_iir & DE_GSE)
//        intel_opregion_gse_intr(dev);

//    if (de_iir & DE_PLANEA_FLIP_DONE) {
//        intel_prepare_page_flip(dev, 0);
//        intel_finish_page_flip_plane(dev, 0);
//    }

//    if (de_iir & DE_PLANEB_FLIP_DONE) {
//        intel_prepare_page_flip(dev, 1);
//        intel_finish_page_flip_plane(dev, 1);
//    }

//    if (de_iir & DE_PIPEA_VBLANK)
//        drm_handle_vblank(dev, 0);

//    if (de_iir & DE_PIPEB_VBLANK)
//        drm_handle_vblank(dev, 1);

    /* check event from PCH */
//    if (de_iir & DE_PCH_EVENT) {
//        if (pch_iir & hotplug_mask)
//            queue_work(dev_priv->wq, &dev_priv->hotplug_work);
//        pch_irq_handler(dev);
//    }

//    if (de_iir & DE_PCU_EVENT) {
//        I915_WRITE16(MEMINTRSTS, I915_READ(MEMINTRSTS));
//        i915_handle_rps_change(dev);
//    }

    if (IS_GEN6(dev) && pm_iir & GEN6_PM_DEFERRED_EVENTS) {
        /*
         * IIR bits should never already be set because IMR should
         * prevent an interrupt from being shown in IIR. The warning
         * displays a case where we've unsafely cleared
         * dev_priv->pm_iir. Although missing an interrupt of the same
         * type is not a problem, it displays a problem in the logic.
         *
         * The mask bit in IMR is cleared by rps_work.
         */
        unsigned long flags;
        spin_lock_irqsave(&dev_priv->rps_lock, flags);
        WARN(dev_priv->pm_iir & pm_iir, "Missed a PM interrupt\n");
        dev_priv->pm_iir |= pm_iir;
        I915_WRITE(GEN6_PMIMR, dev_priv->pm_iir);
        POSTING_READ(GEN6_PMIMR);
        spin_unlock_irqrestore(&dev_priv->rps_lock, flags);
//        queue_work(dev_priv->wq, &dev_priv->rps_work);
    }

    /* should clear PCH hotplug event before clear CPU irq */
    I915_WRITE(SDEIIR, pch_iir);
    I915_WRITE(GTIIR, gt_iir);
    I915_WRITE(DEIIR, de_iir);
    I915_WRITE(GEN6_PMIIR, pm_iir);

done:
    I915_WRITE(DEIER, de_ier);
    POSTING_READ(DEIER);

    return ret;
}









/* drm_dma.h hooks
*/
static void ironlake_irq_preinstall(struct drm_device *dev)
{
    drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;

    atomic_set(&dev_priv->irq_received, 0);

//    INIT_WORK(&dev_priv->hotplug_work, i915_hotplug_work_func);
//    INIT_WORK(&dev_priv->error_work, i915_error_work_func);
//    if (IS_GEN6(dev) || IS_IVYBRIDGE(dev))
//        INIT_WORK(&dev_priv->rps_work, gen6_pm_rps_work);

    I915_WRITE(HWSTAM, 0xeffe);

    if (IS_GEN6(dev)) {
        /* Workaround stalls observed on Sandy Bridge GPUs by
         * making the blitter command streamer generate a
         * write to the Hardware Status Page for
         * MI_USER_INTERRUPT.  This appears to serialize the
         * previous seqno write out before the interrupt
         * happens.
         */
        I915_WRITE(GEN6_BLITTER_HWSTAM, ~GEN6_BLITTER_USER_INTERRUPT);
        I915_WRITE(GEN6_BSD_HWSTAM, ~GEN6_BSD_USER_INTERRUPT);
    }

    /* XXX hotplug from PCH */

    I915_WRITE(DEIMR, 0xffffffff);
    I915_WRITE(DEIER, 0x0);
    POSTING_READ(DEIER);

    /* and GT */
    I915_WRITE(GTIMR, 0xffffffff);
    I915_WRITE(GTIER, 0x0);
    POSTING_READ(GTIER);

    /* south display irq */
    I915_WRITE(SDEIMR, 0xffffffff);
    I915_WRITE(SDEIER, 0x0);
    POSTING_READ(SDEIER);
}

/*
 * Enable digital hotplug on the PCH, and configure the DP short pulse
 * duration to 2ms (which is the minimum in the Display Port spec)
 *
 * This register is the same on all known PCH chips.
 */

static void ironlake_enable_pch_hotplug(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
	u32	hotplug;

	hotplug = I915_READ(PCH_PORT_HOTPLUG);
	hotplug &= ~(PORTD_PULSE_DURATION_MASK|PORTC_PULSE_DURATION_MASK|PORTB_PULSE_DURATION_MASK);
	hotplug |= PORTD_HOTPLUG_ENABLE | PORTD_PULSE_DURATION_2ms;
	hotplug |= PORTC_HOTPLUG_ENABLE | PORTC_PULSE_DURATION_2ms;
	hotplug |= PORTB_HOTPLUG_ENABLE | PORTB_PULSE_DURATION_2ms;
	I915_WRITE(PCH_PORT_HOTPLUG, hotplug);
}

static int ironlake_irq_postinstall(struct drm_device *dev)
{
    drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
    /* enable kind of interrupts always enabled */
    u32 display_mask = DE_MASTER_IRQ_CONTROL | DE_GSE | DE_PCH_EVENT |
               DE_PLANEA_FLIP_DONE | DE_PLANEB_FLIP_DONE;
    u32 render_irqs;
    u32 hotplug_mask;

    DRM_INIT_WAITQUEUE(&dev_priv->ring[RCS].irq_queue);
    if (HAS_BSD(dev))
        DRM_INIT_WAITQUEUE(&dev_priv->ring[VCS].irq_queue);
    if (HAS_BLT(dev))
        DRM_INIT_WAITQUEUE(&dev_priv->ring[BCS].irq_queue);

    dev_priv->vblank_pipe = DRM_I915_VBLANK_PIPE_A | DRM_I915_VBLANK_PIPE_B;
    dev_priv->irq_mask = ~display_mask;

    /* should always can generate irq */
    I915_WRITE(DEIIR, I915_READ(DEIIR));
    I915_WRITE(DEIMR, dev_priv->irq_mask);
    I915_WRITE(DEIER, display_mask | DE_PIPEA_VBLANK | DE_PIPEB_VBLANK);
    POSTING_READ(DEIER);

	dev_priv->gt_irq_mask = ~0;

    I915_WRITE(GTIIR, I915_READ(GTIIR));
    I915_WRITE(GTIMR, dev_priv->gt_irq_mask);

    if (IS_GEN6(dev))
        render_irqs =
            GT_USER_INTERRUPT |
            GT_GEN6_BSD_USER_INTERRUPT |
            GT_BLT_USER_INTERRUPT;
    else
        render_irqs =
            GT_USER_INTERRUPT |
            GT_PIPE_NOTIFY |
            GT_BSD_USER_INTERRUPT;
    I915_WRITE(GTIER, render_irqs);
    POSTING_READ(GTIER);

    if (HAS_PCH_CPT(dev)) {
        hotplug_mask = (SDE_CRT_HOTPLUG_CPT |
                SDE_PORTB_HOTPLUG_CPT |
                SDE_PORTC_HOTPLUG_CPT |
                SDE_PORTD_HOTPLUG_CPT);
    } else {
        hotplug_mask = (SDE_CRT_HOTPLUG |
                SDE_PORTB_HOTPLUG |
                SDE_PORTC_HOTPLUG |
                SDE_PORTD_HOTPLUG |
                SDE_AUX_MASK);
    }

    dev_priv->pch_irq_mask = ~hotplug_mask;

    I915_WRITE(SDEIIR, I915_READ(SDEIIR));
    I915_WRITE(SDEIMR, dev_priv->pch_irq_mask);
    I915_WRITE(SDEIER, hotplug_mask);
    POSTING_READ(SDEIER);

    ironlake_enable_pch_hotplug(dev);

    if (IS_IRONLAKE_M(dev)) {
        /* Clear & enable PCU event interrupts */
        I915_WRITE(DEIIR, DE_PCU_EVENT);
        I915_WRITE(DEIER, I915_READ(DEIER) | DE_PCU_EVENT);
        ironlake_enable_display_irq(dev_priv, DE_PCU_EVENT);
    }

    return 0;
}


void intel_irq_init(struct drm_device *dev)
{
#if 0
	if (IS_IVYBRIDGE(dev)) {
		/* Share pre & uninstall handlers with ILK/SNB */
		dev->driver->irq_handler = ivybridge_irq_handler;
		dev->driver->irq_preinstall = ironlake_irq_preinstall;
		dev->driver->irq_postinstall = ivybridge_irq_postinstall;
		dev->driver->irq_uninstall = ironlake_irq_uninstall;
		dev->driver->enable_vblank = ivybridge_enable_vblank;
		dev->driver->disable_vblank = ivybridge_disable_vblank;
	} else if (HAS_PCH_SPLIT(dev)) {
		dev->driver->irq_handler = ironlake_irq_handler;
		dev->driver->irq_preinstall = ironlake_irq_preinstall;
		dev->driver->irq_postinstall = ironlake_irq_postinstall;
		dev->driver->irq_uninstall = ironlake_irq_uninstall;
		dev->driver->enable_vblank = ironlake_enable_vblank;
		dev->driver->disable_vblank = ironlake_disable_vblank;
	} else {
		dev->driver->irq_preinstall = i915_driver_irq_preinstall;
		dev->driver->irq_postinstall = i915_driver_irq_postinstall;
		dev->driver->irq_uninstall = i915_driver_irq_uninstall;
		dev->driver->irq_handler = i915_driver_irq_handler;
		dev->driver->enable_vblank = i915_enable_vblank;
		dev->driver->disable_vblank = i915_disable_vblank;
	}
#endif
}


static struct drm_device *irq_device;

void irq_handler_kms()
{
//    printf("%s\n",__FUNCTION__);
    ironlake_irq_handler(irq_device);
}

int drm_irq_install(struct drm_device *dev)
{
    int irq_line;
    int ret = 0;

    ENTER();

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

    irq_device = dev;
    irq_line   = drm_dev_to_irq(dev);

    DRM_DEBUG("irq=%d\n", drm_dev_to_irq(dev));

    ironlake_irq_preinstall(dev);

    ret = AttachIntHandler(irq_line, irq_handler_kms, 2);
    if (ret == 0) {
        mutex_lock(&dev->struct_mutex);
        dev->irq_enabled = 0;
        mutex_unlock(&dev->struct_mutex);
        return ret;
    }

    ret = ironlake_irq_postinstall(dev);

//    if (ret < 0) {
//        mutex_lock(&dev->struct_mutex);
//        dev->irq_enabled = 0;
//        mutex_unlock(&dev->struct_mutex);
//        free_irq(drm_dev_to_irq(dev), dev);
//    }

    u16_t cmd = PciRead16(dev->pdev->busnr, dev->pdev->devfn, 4);

    cmd&= ~(1<<10);

    PciWrite16(dev->pdev->busnr, dev->pdev->devfn, 4, cmd);

    dbgprintf("PCI_CMD: %04x\n", cmd);

    DRM_INFO("i915: irq initialized.\n");
    LEAVE();
    return ret;
}



