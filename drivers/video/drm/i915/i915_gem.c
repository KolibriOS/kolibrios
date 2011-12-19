/*
 * Copyright © 2008 Intel Corporation
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#include "drmP.h"
#include "drm.h"
//#include "i915_drm.h"
#include "i915_drv.h"
//#include "i915_trace.h"
#include "intel_drv.h"
//#include <linux/shmem_fs.h>
//#include <linux/slab.h>
//#include <linux/swap.h>
#include <linux/pci.h>

#define I915_EXEC_CONSTANTS_MASK        (3<<6)
#define I915_EXEC_CONSTANTS_REL_GENERAL (0<<6) /* default */
#define I915_EXEC_CONSTANTS_ABSOLUTE    (1<<6)
#define I915_EXEC_CONSTANTS_REL_SURFACE (2<<6) /* gen4/5 only */


/**
 * i915_gem_clear_fence_reg - clear out fence register info
 * @obj: object to clear
 *
 * Zeroes out the fence register itself and clears out the associated
 * data structures in dev_priv and obj.
 */
static void
i915_gem_clear_fence_reg(struct drm_device *dev,
             struct drm_i915_fence_reg *reg)
{
    drm_i915_private_t *dev_priv = dev->dev_private;
    uint32_t fence_reg = reg - dev_priv->fence_regs;

    switch (INTEL_INFO(dev)->gen) {
    case 7:
    case 6:
        I915_WRITE64(FENCE_REG_SANDYBRIDGE_0 + fence_reg*8, 0);
        break;
    case 5:
    case 4:
        I915_WRITE64(FENCE_REG_965_0 + fence_reg*8, 0);
        break;
    case 3:
        if (fence_reg >= 8)
            fence_reg = FENCE_REG_945_8 + (fence_reg - 8) * 4;
        else
    case 2:
            fence_reg = FENCE_REG_830_0 + fence_reg * 4;

        I915_WRITE(fence_reg, 0);
        break;
    }

    list_del_init(&reg->lru_list);
    reg->obj = NULL;
    reg->setup_seqno = 0;
}


static void
init_ring_lists(struct intel_ring_buffer *ring)
{
    INIT_LIST_HEAD(&ring->active_list);
    INIT_LIST_HEAD(&ring->request_list);
    INIT_LIST_HEAD(&ring->gpu_write_list);
}


void
i915_gem_load(struct drm_device *dev)
{
    int i;
    drm_i915_private_t *dev_priv = dev->dev_private;

    INIT_LIST_HEAD(&dev_priv->mm.active_list);
    INIT_LIST_HEAD(&dev_priv->mm.flushing_list);
    INIT_LIST_HEAD(&dev_priv->mm.inactive_list);
    INIT_LIST_HEAD(&dev_priv->mm.pinned_list);
    INIT_LIST_HEAD(&dev_priv->mm.fence_list);
    INIT_LIST_HEAD(&dev_priv->mm.deferred_free_list);
    INIT_LIST_HEAD(&dev_priv->mm.gtt_list);
    for (i = 0; i < I915_NUM_RINGS; i++)
        init_ring_lists(&dev_priv->ring[i]);
    for (i = 0; i < 16; i++)
        INIT_LIST_HEAD(&dev_priv->fence_regs[i].lru_list);
//    INIT_DELAYED_WORK(&dev_priv->mm.retire_work,
//              i915_gem_retire_work_handler);
//    init_completion(&dev_priv->error_completion);

    /* On GEN3 we really need to make sure the ARB C3 LP bit is set */
    if (IS_GEN3(dev)) {
        u32 tmp = I915_READ(MI_ARB_STATE);
        if (!(tmp & MI_ARB_C3_LP_WRITE_ENABLE)) {
            /* arb state is a masked write, so set bit + bit in mask */
            tmp = MI_ARB_C3_LP_WRITE_ENABLE | (MI_ARB_C3_LP_WRITE_ENABLE << MI_ARB_MASK_SHIFT);
            I915_WRITE(MI_ARB_STATE, tmp);
        }
    }

    dev_priv->relative_constants_mode = I915_EXEC_CONSTANTS_REL_GENERAL;

    if (INTEL_INFO(dev)->gen >= 4 || IS_I945G(dev) || IS_I945GM(dev) || IS_G33(dev))
        dev_priv->num_fence_regs = 16;
    else
        dev_priv->num_fence_regs = 8;

    /* Initialize fence registers to zero */
    for (i = 0; i < dev_priv->num_fence_regs; i++) {
        i915_gem_clear_fence_reg(dev, &dev_priv->fence_regs[i]);
    }

    i915_gem_detect_bit_6_swizzle(dev);
//    init_waitqueue_head(&dev_priv->pending_flip_queue);

    dev_priv->mm.interruptible = true;

//    dev_priv->mm.inactive_shrinker.shrink = i915_gem_inactive_shrink;
//    dev_priv->mm.inactive_shrinker.seeks = DEFAULT_SEEKS;
//    register_shrinker(&dev_priv->mm.inactive_shrinker);
}



