/*
 * Copyright © 2009 Intel Corporation
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
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Xiang Haihao <haihao.xiang@intel.com>
 *    Zou Nan hai <nanhai.zou@intel.com>
 *
 */

#include "sysdeps.h"
#include "intel_batchbuffer.h"
#include "intel_memman.h"
#include "intel_driver.h"

static Bool
intel_driver_get_param(struct intel_driver_data *intel, int param, int *value)
{
   struct drm_i915_getparam gp;

   gp.param = param;
   gp.value = value;

   return (drmIoctl(intel->fd, DRM_IOCTL_I915_GETPARAM, &gp) == 0);

}

static void intel_driver_get_revid(struct intel_driver_data *intel, int *value)
{
    *value = 2; /* assume it is at least  B-steping */

	return;
}

Bool 
intel_driver_init(VADriverContextP ctx)
{
    struct intel_driver_data *intel = intel_driver_data(ctx);
    struct drm_state * const drm_state = (struct drm_state *)ctx->drm_state;
    int has_exec2, has_bsd, has_blt;

    assert(drm_state);
    assert(VA_CHECK_DRM_AUTH_TYPE(ctx, VA_DRM_AUTH_DRI1) ||
           VA_CHECK_DRM_AUTH_TYPE(ctx, VA_DRM_AUTH_DRI2) ||
           VA_CHECK_DRM_AUTH_TYPE(ctx, VA_DRM_AUTH_CUSTOM));

   printf("%s context %p\n", __FUNCTION__, ctx);

    intel->fd = drm_state->fd;
    intel->dri2Enabled = 1;
    
    intel->locked = 0;
//    pthread_mutex_init(&intel->ctxmutex, NULL);

    intel_driver_get_param(intel, I915_PARAM_CHIPSET_ID, &intel->device_id);
    if (intel_driver_get_param(intel, I915_PARAM_HAS_EXECBUF2, &has_exec2))
        intel->has_exec2 = has_exec2;
    if (intel_driver_get_param(intel, I915_PARAM_HAS_BSD, &has_bsd))
        intel->has_bsd = has_bsd;
    if (intel_driver_get_param(intel, I915_PARAM_HAS_BLT, &has_blt))
        intel->has_blt = has_blt;
   
    printf("device_id=%x has_exec2=%d has_bsd=%d has_blt=%d\n",
           intel->device_id, intel->has_exec2, intel->has_bsd, intel->has_blt);
            
    intel_driver_get_revid(intel, &intel->revision);
    intel_memman_init(intel);
    return True;
}

Bool 
intel_driver_terminate(VADriverContextP ctx)
{
    struct intel_driver_data *intel = intel_driver_data(ctx);

    intel_memman_terminate(intel);
//    pthread_mutex_destroy(&intel->ctxmutex);

    return True;
}
