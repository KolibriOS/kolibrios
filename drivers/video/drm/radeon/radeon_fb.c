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
    /*
     *  Modularization
     */

#include <linux/module.h>
#include <linux/fb.h>

#include "drmP.h"
#include "drm.h"
#include "drm_crtc.h"
#include "drm_crtc_helper.h"
#include "radeon_drm.h"
#include "radeon.h"

#include "drm_fb_helper.h"

#include <drm_mm.h>
#include "radeon_object.h"


struct fb_info *framebuffer_alloc(size_t size, void *dev);

struct radeon_fb_device {
    struct drm_fb_helper        helper;
	struct radeon_framebuffer	*rfb;
	struct radeon_device		*rdev;
};

static struct fb_ops radeonfb_ops = {
//   .owner = THIS_MODULE,
	.fb_check_var = drm_fb_helper_check_var,
	.fb_set_par = drm_fb_helper_set_par,
	.fb_setcolreg = drm_fb_helper_setcolreg,
//	.fb_fillrect = cfb_fillrect,
//	.fb_copyarea = cfb_copyarea,
//	.fb_imageblit = cfb_imageblit,
//	.fb_pan_display = drm_fb_helper_pan_display,
	.fb_blank = drm_fb_helper_blank,
	.fb_setcmap = drm_fb_helper_setcmap,
};

/**
 * Currently it is assumed that the old framebuffer is reused.
 *
 * LOCKING
 * caller should hold the mode config lock.
 *
 */
int radeonfb_resize(struct drm_device *dev, struct drm_crtc *crtc)
{
	struct fb_info *info;
	struct drm_framebuffer *fb;
	struct drm_display_mode *mode = crtc->desired_mode;

	fb = crtc->fb;
	if (fb == NULL) {
		return 1;
	}
	info = fb->fbdev;
	if (info == NULL) {
		return 1;
	}
	if (mode == NULL) {
		return 1;
	}
	info->var.xres = mode->hdisplay;
	info->var.right_margin = mode->hsync_start - mode->hdisplay;
	info->var.hsync_len = mode->hsync_end - mode->hsync_start;
	info->var.left_margin = mode->htotal - mode->hsync_end;
	info->var.yres = mode->vdisplay;
	info->var.lower_margin = mode->vsync_start - mode->vdisplay;
	info->var.vsync_len = mode->vsync_end - mode->vsync_start;
	info->var.upper_margin = mode->vtotal - mode->vsync_end;
	info->var.pixclock = 10000000 / mode->htotal * 1000 / mode->vtotal * 100;
	/* avoid overflow */
	info->var.pixclock = info->var.pixclock * 1000 / mode->vrefresh;

	return 0;
}
EXPORT_SYMBOL(radeonfb_resize);

int radeon_align_pitch(struct radeon_device *rdev, int width, int bpp, bool tiled)
{
	int aligned = width;
	int align_large = (ASIC_IS_AVIVO(rdev)) || tiled;
	int pitch_mask = 0;

	switch (bpp / 8) {
	case 1:
		pitch_mask = align_large ? 255 : 127;
		break;
	case 2:
		pitch_mask = align_large ? 127 : 31;
		break;
	case 3:
	case 4:
		pitch_mask = align_large ? 63 : 15;
		break;
	}

	aligned += pitch_mask;
	aligned &= ~pitch_mask;
	return aligned;
}

static struct drm_fb_helper_funcs radeon_fb_helper_funcs = {
	.gamma_set = radeon_crtc_fb_gamma_set,
	.gamma_get = radeon_crtc_fb_gamma_get,
};

int radeonfb_create(struct drm_device *dev,
		    uint32_t fb_width, uint32_t fb_height,
		    uint32_t surface_width, uint32_t surface_height,
		    uint32_t surface_depth, uint32_t surface_bpp,
		    struct drm_framebuffer **fb_p)
{
	struct radeon_device *rdev = dev->dev_private;
	struct fb_info *info;
	struct radeon_fb_device *rfbdev;
	struct drm_framebuffer *fb = NULL;
	struct radeon_framebuffer *rfb;
	struct drm_mode_fb_cmd mode_cmd;
	struct drm_gem_object *gobj = NULL;
	struct radeon_object *robj = NULL;
    void   *device = NULL; //&rdev->pdev->dev;
	int size, aligned_size, ret;
	u64 fb_gpuaddr;
	void *fbptr = NULL;
	unsigned long tmp;
	bool fb_tiled = false; /* useful for testing */
	u32 tiling_flags = 0;
	int crtc_count;

    mode_cmd.width  = surface_width;
	mode_cmd.height = surface_height;

	/* avivo can't scanout real 24bpp */
	if ((surface_bpp == 24) && ASIC_IS_AVIVO(rdev))
		surface_bpp = 32;

	mode_cmd.bpp = 32;
	/* need to align pitch with crtc limits */
	mode_cmd.pitch = radeon_align_pitch(rdev, mode_cmd.width, mode_cmd.bpp, fb_tiled) * ((mode_cmd.bpp + 1) / 8);
	mode_cmd.depth = surface_depth;

	size = mode_cmd.pitch * mode_cmd.height;
	aligned_size = ALIGN(size, PAGE_SIZE);

    ret = radeon_gem_fb_object_create(rdev, aligned_size, 0,
			RADEON_GEM_DOMAIN_VRAM,
            false, 0,
			false, &gobj);
	if (ret) {
		printk(KERN_ERR "failed to allocate framebuffer (%d %d)\n",
		       surface_width, surface_height);
		ret = -ENOMEM;
		goto out;
	}
	robj = gobj->driver_private;

	mutex_lock(&rdev->ddev->struct_mutex);
	fb = radeon_framebuffer_create(rdev->ddev, &mode_cmd, gobj);
	if (fb == NULL) {
		DRM_ERROR("failed to allocate fb.\n");
		ret = -ENOMEM;
		goto out_unref;
	}
	ret = radeon_object_pin(robj, RADEON_GEM_DOMAIN_VRAM, &fb_gpuaddr);
	if (ret) {
		printk(KERN_ERR "failed to pin framebuffer\n");
		ret = -ENOMEM;
		goto out_unref;
	}

    list_add(&fb->filp_head, &rdev->ddev->mode_config.fb_kernel_list);

	*fb_p = fb;
	rfb = to_radeon_framebuffer(fb);
	rdev->fbdev_rfb = rfb;
	rdev->fbdev_robj = robj;

	info = framebuffer_alloc(sizeof(struct radeon_fb_device), device);
	if (info == NULL) {
		ret = -ENOMEM;
		goto out_unref;
	}

	rdev->fbdev_info = info;
	rfbdev = info->par;
	rfbdev->helper.funcs = &radeon_fb_helper_funcs;
	rfbdev->helper.dev = dev;
	if (rdev->flags & RADEON_SINGLE_CRTC)
		crtc_count = 1;
	else
		crtc_count = 2;
	ret = drm_fb_helper_init_crtc_count(&rfbdev->helper, crtc_count,
					    RADEONFB_CONN_LIMIT);
	if (ret)
		goto out_unref;

//   ret = radeon_object_kmap(robj, &fbptr);
//   if (ret) {
//       goto out_unref;
//   }


    fbptr = (void*)0xFE000000; // LFB_BASE

	strcpy(info->fix.id, "radeondrmfb");

	drm_fb_helper_fill_fix(info, fb->pitch, fb->depth);

	info->flags = FBINFO_DEFAULT;
	info->fbops = &radeonfb_ops;

	tmp = fb_gpuaddr - rdev->mc.vram_location;
	info->fix.smem_start = rdev->mc.aper_base + tmp;
	info->fix.smem_len = size;
	info->screen_base = fbptr;
	info->screen_size = size;

	drm_fb_helper_fill_var(info, fb, fb_width, fb_height);

	/* setup aperture base/size for vesafb takeover */
	info->aperture_base = rdev->ddev->mode_config.fb_base;
	info->aperture_size = rdev->mc.real_vram_size;

	info->fix.mmio_start = 0;
	info->fix.mmio_len = 0;
//   info->pixmap.size = 64*1024;
//   info->pixmap.buf_align = 8;
//   info->pixmap.access_align = 32;
//   info->pixmap.flags = FB_PIXMAP_SYSTEM;
//   info->pixmap.scan_align = 1;
	if (info->screen_base == NULL) {
		ret = -ENOSPC;
		goto out_unref;
	}
	DRM_INFO("fb mappable at 0x%lX\n",  info->fix.smem_start);
	DRM_INFO("vram apper at 0x%lX\n",  (unsigned long)rdev->mc.aper_base);
	DRM_INFO("size %lu\n", (unsigned long)size);
	DRM_INFO("fb depth is %d\n", fb->depth);
	DRM_INFO("   pitch is %d\n", fb->pitch);

    dbgprintf("fb = %x\n", fb);

	fb->fbdev = info;
	rfbdev->rfb = rfb;
	rfbdev->rdev = rdev;

	mutex_unlock(&rdev->ddev->struct_mutex);
	return 0;

out_unref:
	if (robj) {
//       radeon_object_kunmap(robj);
	}
	if (fb && ret) {
		list_del(&fb->filp_head);
 //      drm_gem_object_unreference(gobj);
//       drm_framebuffer_cleanup(fb);
		kfree(fb);
	}
//   drm_gem_object_unreference(gobj);
   mutex_unlock(&rdev->ddev->struct_mutex);
out:
	return ret;
}

int radeonfb_probe(struct drm_device *dev)
{
	return drm_fb_helper_single_fb_probe(dev, 32, &radeonfb_create);
}

int radeonfb_remove(struct drm_device *dev, struct drm_framebuffer *fb)
{
	struct fb_info *info;
	struct radeon_framebuffer *rfb = to_radeon_framebuffer(fb);
	struct radeon_object *robj;

	if (!fb) {
		return -EINVAL;
	}
	info = fb->fbdev;
	if (info) {
		struct radeon_fb_device *rfbdev = info->par;
		robj = rfb->obj->driver_private;
//       unregister_framebuffer(info);
//       radeon_object_kunmap(robj);
//       radeon_object_unpin(robj);
//       framebuffer_release(info);
	}

	printk(KERN_INFO "unregistered panic notifier\n");

	return 0;
}
EXPORT_SYMBOL(radeonfb_remove);


/**
 * Allocate a GEM object of the specified size with shmfs backing store
 */
struct drm_gem_object *
drm_gem_object_alloc(struct drm_device *dev, size_t size)
{
    struct drm_gem_object *obj;

    BUG_ON((size & (PAGE_SIZE - 1)) != 0);

    obj = kzalloc(sizeof(*obj), GFP_KERNEL);

    obj->dev = dev;
//    obj->filp = shmem_file_setup("drm mm object", size, VM_NORESERVE);
//    if (IS_ERR(obj->filp)) {
//        kfree(obj);
//        return NULL;
//    }

//    kref_init(&obj->refcount);
//    kref_init(&obj->handlecount);
    obj->size = size;

//    if (dev->driver->gem_init_object != NULL &&
//        dev->driver->gem_init_object(obj) != 0) {
//        fput(obj->filp);
//        kfree(obj);
//        return NULL;
//    }
//    atomic_inc(&dev->object_count);
//    atomic_add(obj->size, &dev->object_memory);
    return obj;
}


int radeon_gem_fb_object_create(struct radeon_device *rdev, int size,
                 int alignment, int initial_domain,
                 bool discardable, bool kernel,
                 bool interruptible,
                 struct drm_gem_object **obj)
{
    struct drm_gem_object *gobj;
    struct radeon_object *robj;

    *obj = NULL;
    gobj = drm_gem_object_alloc(rdev->ddev, size);
    if (!gobj) {
        return -ENOMEM;
    }
    /* At least align on page size */
    if (alignment < PAGE_SIZE) {
        alignment = PAGE_SIZE;
    }

    robj = kzalloc(sizeof(struct radeon_object), GFP_KERNEL);
    if (!robj) {
        DRM_ERROR("Failed to allocate GEM object (%d, %d, %u)\n",
              size, initial_domain, alignment);
//       mutex_lock(&rdev->ddev->struct_mutex);
//       drm_gem_object_unreference(gobj);
//       mutex_unlock(&rdev->ddev->struct_mutex);
        return -ENOMEM;;
    }
    robj->rdev = rdev;
    robj->gobj = gobj;
    INIT_LIST_HEAD(&robj->list);

    robj->flags = TTM_PL_FLAG_VRAM;

    struct drm_mm_node *vm_node;

    vm_node = kzalloc(sizeof(*vm_node),0);

    vm_node->free = 0;
    vm_node->size = 0xC00000 >> 12;
    vm_node->start = 0;
    vm_node->mm = NULL;

    robj->mm_node = vm_node;

    robj->vm_addr = ((uint32_t)robj->mm_node->start);

    gobj->driver_private = robj;
    *obj = gobj;
    return 0;
}


struct fb_info *framebuffer_alloc(size_t size, void *dev)
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




