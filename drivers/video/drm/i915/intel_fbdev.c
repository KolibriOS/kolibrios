/*
 * Copyright © 2007 David Airlie
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *     David Airlie
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
//#include <linux/mm.h>
//#include <linux/tty.h>
#include <linux/sysrq.h>
//#include <linux/delay.h>
#include <linux/fb.h>
//#include <linux/init.h>
//#include <linux/vga_switcheroo.h>

#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_fb_helper.h>
#include "intel_drv.h"
#include <drm/i915_drm.h>
#include "i915_drv.h"


struct fb_info *framebuffer_alloc(size_t size, struct device *dev)
{
#define BYTES_PER_LONG (BITS_PER_LONG/8)
#define PADDING (BYTES_PER_LONG - (sizeof(struct fb_info) % BYTES_PER_LONG))
    int fb_info_size = sizeof(struct fb_info);
    struct fb_info *info;
    char *p;

    if (size)
        fb_info_size += PADDING;

    p = kzalloc(fb_info_size + size, GFP_KERNEL);

    if (!p)
        return NULL;

    info = (struct fb_info *) p;

    if (size)
        info->par = p + fb_info_size;

    return info;
#undef PADDING
#undef BYTES_PER_LONG
}


static struct fb_ops intelfb_ops = {
	.owner = THIS_MODULE,
	.fb_check_var = drm_fb_helper_check_var,
	.fb_set_par = drm_fb_helper_set_par,
//   .fb_fillrect = cfb_fillrect,
//   .fb_copyarea = cfb_copyarea,
//   .fb_imageblit = cfb_imageblit,
//   .fb_pan_display = drm_fb_helper_pan_display,
	.fb_blank = drm_fb_helper_blank,
//   .fb_setcmap = drm_fb_helper_setcmap,
//   .fb_debug_enter = drm_fb_helper_debug_enter,
//   .fb_debug_leave = drm_fb_helper_debug_leave,
};

static int intelfb_alloc(struct drm_fb_helper *helper,
			  struct drm_fb_helper_surface_size *sizes)
{
	struct intel_fbdev *ifbdev =
		container_of(helper, struct intel_fbdev, helper);
	struct drm_device *dev = helper->dev;
	struct drm_mode_fb_cmd2 mode_cmd = {};
	struct drm_i915_gem_object *obj;
	int size, ret;

	/* we don't do packed 24bpp */
	if (sizes->surface_bpp == 24)
		sizes->surface_bpp = 32;

	mode_cmd.width = sizes->surface_width;
	mode_cmd.height = sizes->surface_height;

	mode_cmd.pitches[0] = ALIGN(mode_cmd.width * ((sizes->surface_bpp + 7) /
                              8), 512);
	mode_cmd.pixel_format = drm_mode_legacy_fb_format(sizes->surface_bpp,
							  sizes->surface_depth);

	size = mode_cmd.pitches[0] * mode_cmd.height;
	size = ALIGN(size, PAGE_SIZE);
	obj = main_fb_obj;
	if (!obj) {
		DRM_ERROR("failed to allocate framebuffer\n");
		ret = -ENOMEM;
		goto out;
	}
    obj->has_global_gtt_mapping = 0;
    obj->stride = mode_cmd.pitches[0];

	/* Flush everything out, we'll be doing GTT only from now on */
	ret = intel_pin_and_fence_fb_obj(dev, obj, NULL);
	if (ret) {
		DRM_ERROR("failed to pin fb: %d\n", ret);
		goto out_unref;
	}

	ret = intel_framebuffer_init(dev, &ifbdev->ifb, &mode_cmd, obj);
	if (ret)
		goto out_unpin;

	return 0;

out_unpin:
	i915_gem_object_unpin(obj);
out_unref:
	drm_gem_object_unreference(&obj->base);
out:
	return ret;
}

static int intelfb_create(struct drm_fb_helper *helper,
			  struct drm_fb_helper_surface_size *sizes)
{
	struct intel_fbdev *ifbdev =
		container_of(helper, struct intel_fbdev, helper);
	struct intel_framebuffer *intel_fb = &ifbdev->ifb;
	struct drm_device *dev = helper->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct fb_info *info;
	struct drm_framebuffer *fb;
	struct drm_i915_gem_object *obj;
	int size, ret;

	mutex_lock(&dev->struct_mutex);

	if (!intel_fb->obj) {
		DRM_DEBUG_KMS("no BIOS fb, allocating a new one\n");
		ret = intelfb_alloc(helper, sizes);
		if (ret)
			goto out_unlock;
	} else {
		DRM_DEBUG_KMS("re-using BIOS fb\n");
		sizes->fb_width = intel_fb->base.width;
		sizes->fb_height = intel_fb->base.height;
	}

	obj = intel_fb->obj;
	size = obj->base.size;

	info = framebuffer_alloc(0, &dev->pdev->dev);
	if (!info) {
		ret = -ENOMEM;
		goto out_unpin;
	}

	info->par = helper;

	fb = &ifbdev->ifb.base;

	ifbdev->helper.fb = fb;
	ifbdev->helper.fbdev = info;

    strcpy(info->fix.id, "inteldrmfb");

    info->flags = FBINFO_DEFAULT | FBINFO_CAN_FORCE_OUTPUT;
    info->fbops = &intelfb_ops;

    /* setup aperture base/size for vesafb takeover */
	info->apertures = alloc_apertures(1);
	if (!info->apertures) {
		ret = -ENOMEM;
		goto out_unpin;
	}
	info->apertures->ranges[0].base = dev->mode_config.fb_base;
	info->apertures->ranges[0].size = dev_priv->gtt.mappable_end;


	info->screen_base = (void*) 0xFE000000;
	info->screen_size = size;

	/* This driver doesn't need a VT switch to restore the mode on resume */
	info->skip_vt_switch = true;

	drm_fb_helper_fill_fix(info, fb->pitches[0], fb->depth);
	drm_fb_helper_fill_var(info, &ifbdev->helper, sizes->fb_width, sizes->fb_height);

	/* Use default scratch pixmap (info->pixmap.flags = FB_PIXMAP_SYSTEM) */

	DRM_DEBUG_KMS("allocated %dx%d fb: 0x%08lx, bo %p\n",
		      fb->width, fb->height,
		      i915_gem_obj_ggtt_offset(obj), obj);

	mutex_unlock(&dev->struct_mutex);
	return 0;

out_unpin:
	i915_gem_object_unpin(obj);
	drm_gem_object_unreference(&obj->base);
out_unlock:
	mutex_unlock(&dev->struct_mutex);
	return ret;
}

/** Sets the color ramps on behalf of RandR */
static void intel_crtc_fb_gamma_set(struct drm_crtc *crtc, u16 red, u16 green,
				    u16 blue, int regno)
{
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);

	intel_crtc->lut_r[regno] = red >> 8;
	intel_crtc->lut_g[regno] = green >> 8;
	intel_crtc->lut_b[regno] = blue >> 8;
}

static void intel_crtc_fb_gamma_get(struct drm_crtc *crtc, u16 *red, u16 *green,
				    u16 *blue, int regno)
{
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);

	*red = intel_crtc->lut_r[regno] << 8;
	*green = intel_crtc->lut_g[regno] << 8;
	*blue = intel_crtc->lut_b[regno] << 8;
}

static struct drm_fb_helper_funcs intel_fb_helper_funcs = {
	.gamma_set = intel_crtc_fb_gamma_set,
	.gamma_get = intel_crtc_fb_gamma_get,
	.fb_probe = intelfb_create,
};


int intel_fbdev_init(struct drm_device *dev)
{
	struct intel_fbdev *ifbdev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	int ret;

	ifbdev = kzalloc(sizeof(*ifbdev), GFP_KERNEL);
	if (!ifbdev)
		return -ENOMEM;

	dev_priv->fbdev = ifbdev;
	ifbdev->helper.funcs = &intel_fb_helper_funcs;

	ret = drm_fb_helper_init(dev, &ifbdev->helper,
				 INTEL_INFO(dev)->num_pipes,
				 4);
	if (ret) {
		kfree(ifbdev);
		return ret;
	}

	drm_fb_helper_single_add_all_connectors(&ifbdev->helper);

    return 0;
}

void intel_fbdev_initial_config(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;

	/* Due to peculiar init order wrt to hpd handling this is separate. */
	drm_fb_helper_initial_config(&dev_priv->fbdev->helper, 32);
}


void intel_fbdev_output_poll_changed(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	drm_fb_helper_hotplug_event(&dev_priv->fbdev->helper);
}
