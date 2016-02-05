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

#include <va/va_drmcommon.h>

#include "intel_batchbuffer.h"
#include "intel_memman.h"
#include "intel_driver.h"
uint32_t g_intel_debug_option_flags = 0;

#ifdef I915_PARAM_HAS_BSD2
#define LOCAL_I915_PARAM_HAS_BSD2 I915_PARAM_HAS_BSD2
#endif

#ifndef LOCAL_I915_PARAM_HAS_BSD2
#define LOCAL_I915_PARAM_HAS_BSD2	30
#endif

static Bool
intel_driver_get_param(struct intel_driver_data *intel, int param, int *value)
{
   struct drm_i915_getparam gp;

   gp.param = param;
   gp.value = value;

   return drmIoctl(intel->fd, DRM_IOCTL_I915_GETPARAM, &gp) == 0;

//   return drmCommandWriteRead(intel->fd, DRM_I915_GETPARAM, &gp, sizeof(gp)) == 0;
}

static void intel_driver_get_revid(struct intel_driver_data *intel, int *value)
{
#if 0
#define PCI_REVID	8
	FILE *fp;
	char config_data[16];
	
	fp = fopen("/sys/devices/pci0000:00/0000:00:02.0/config", "r");

        if (fp) {
            if (fread(config_data, 1, 16, fp))
                *value = config_data[PCI_REVID];
            else
                *value = 2; /* assume it is at least  B-steping */
            fclose(fp);
        } else {
            *value = 2; /* assume it is at least  B-steping */
        }
#endif

    *value = 2;
	return;
}

extern const struct intel_device_info *i965_get_device_info(int devid);

bool 
intel_driver_init(VADriverContextP ctx)
{
    struct intel_driver_data *intel = intel_driver_data(ctx);
    struct drm_state * const drm_state = (struct drm_state *)ctx->drm_state;
    int has_exec2 = 0, has_bsd = 0, has_blt = 0, has_vebox = 0;
    int ret_value = 0;

    g_intel_debug_option_flags = 0;
//    if ((env_str = getenv("VA_INTEL_DEBUG")))
//        g_intel_debug_option_flags = atoi(env_str);

//    if (g_intel_debug_option_flags)
//        fprintf(stderr, "g_intel_debug_option_flags:%x\n", g_intel_debug_option_flags);

    assert(drm_state);

    intel->fd = drm_state->fd;
    intel->dri2Enabled = 1;

    intel->locked = 0;
//    pthread_mutex_init(&intel->ctxmutex, NULL);

    intel_memman_init(intel);
    intel->device_id = drm_intel_bufmgr_gem_get_devid(intel->bufmgr);
    intel->device_info = i965_get_device_info(intel->device_id);

    if (!intel->device_info)
        return false;

    if (intel_driver_get_param(intel, I915_PARAM_HAS_EXECBUF2, &has_exec2))
        intel->has_exec2 = has_exec2;
    if (intel_driver_get_param(intel, I915_PARAM_HAS_BSD, &has_bsd))
        intel->has_bsd = has_bsd;
    if (intel_driver_get_param(intel, I915_PARAM_HAS_BLT, &has_blt))
        intel->has_blt = has_blt;
    if (intel_driver_get_param(intel, I915_PARAM_HAS_VEBOX, &has_vebox))
        intel->has_vebox = !!has_vebox;

    intel->has_bsd2 = 0;
    if (intel_driver_get_param(intel, LOCAL_I915_PARAM_HAS_BSD2, &ret_value))
        intel->has_bsd2 = !!ret_value;

    intel_driver_get_revid(intel, &intel->revision);
    return true;
}

void 
intel_driver_terminate(VADriverContextP ctx)
{
    struct intel_driver_data *intel = intel_driver_data(ctx);

    intel_memman_terminate(intel);
//    pthread_mutex_destroy(&intel->ctxmutex);
}
