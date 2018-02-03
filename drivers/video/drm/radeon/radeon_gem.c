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
#include <drm/radeon_drm.h>
#include "radeon.h"

void radeon_gem_object_free(struct drm_gem_object *gobj)
{
	struct radeon_bo *robj = gem_to_radeon_bo(gobj);

	if (robj) {
		radeon_bo_unref(&robj);
	}
}

int radeon_gem_object_create(struct radeon_device *rdev, unsigned long size,
				int alignment, int initial_domain,
				u32 flags, bool kernel,
				struct drm_gem_object **obj)
{
	struct radeon_bo *robj;
	unsigned long max_size;
	int r;

	*obj = NULL;
	/* At least align on page size */
	if (alignment < PAGE_SIZE) {
		alignment = PAGE_SIZE;
	}

	/* Maximum bo size is the unpinned gtt size since we use the gtt to
	 * handle vram to system pool migrations.
	 */
	max_size = rdev->mc.gtt_size - rdev->gart_pin_size;
	if (size > max_size) {
		DRM_DEBUG("Allocation size %ldMb bigger than %ldMb limit\n",
			  size >> 20, max_size >> 20);
		return -ENOMEM;
	}

retry:
	r = radeon_bo_create(rdev, size, alignment, kernel, initial_domain,
			     flags, NULL, NULL, &robj);
	if (r) {
		if (r != -ERESTARTSYS) {
			if (initial_domain == RADEON_GEM_DOMAIN_VRAM) {
				initial_domain |= RADEON_GEM_DOMAIN_GTT;
				goto retry;
			}
			DRM_ERROR("Failed to allocate GEM object (%ld, %d, %u, %d)\n",
				  size, initial_domain, alignment, r);
		}
		return r;
	}
	*obj = &robj->gem_base;

	mutex_lock(&rdev->gem.mutex);
	list_add_tail(&robj->list, &rdev->gem.objects);
	mutex_unlock(&rdev->gem.mutex);

	return 0;
}

static int radeon_gem_set_domain(struct drm_gem_object *gobj,
			  uint32_t rdomain, uint32_t wdomain)
{
	struct radeon_bo *robj;
	uint32_t domain;
	long r;

	/* FIXME: reeimplement */
	robj = gem_to_radeon_bo(gobj);
	/* work out where to validate the buffer to */
	domain = wdomain;
	if (!domain) {
		domain = rdomain;
	}
	if (!domain) {
		/* Do nothings */
		printk(KERN_WARNING "Set domain without domain !\n");
		return 0;
	}
	if (domain == RADEON_GEM_DOMAIN_CPU) {
		/* Asking for cpu access wait for object idle */
//		r = radeon_bo_wait(robj, NULL, false);
//		if (r) {
//			printk(KERN_ERR "Failed to wait for object !\n");
//			return r;
//		}
	}
	return 0;
}

int radeon_gem_init(struct radeon_device *rdev)
{
	INIT_LIST_HEAD(&rdev->gem.objects);
	return 0;
}

void radeon_gem_fini(struct radeon_device *rdev)
{
 //  radeon_object_force_delete(rdev);
}

#if 0
/*
 * GEM ioctls.
 */
int radeon_gem_info_ioctl(struct drm_device *dev, void *data,
			  struct drm_file *filp)
{
	struct radeon_device *rdev = dev->dev_private;
	struct drm_radeon_gem_info *args = data;
	struct ttm_mem_type_manager *man;

	man = &rdev->mman.bdev.man[TTM_PL_VRAM];

	args->vram_size = rdev->mc.real_vram_size;
	args->vram_visible = (u64)man->size << PAGE_SHIFT;
	args->vram_visible -= rdev->vram_pin_size;
	args->gart_size = rdev->mc.gtt_size;
	args->gart_size -= rdev->gart_pin_size;

	return 0;
}

int radeon_gem_pread_ioctl(struct drm_device *dev, void *data,
			   struct drm_file *filp)
{
	/* TODO: implement */
	DRM_ERROR("unimplemented %s\n", __func__);
	return -ENOSYS;
}

int radeon_gem_pwrite_ioctl(struct drm_device *dev, void *data,
			    struct drm_file *filp)
{
	/* TODO: implement */
	DRM_ERROR("unimplemented %s\n", __func__);
	return -ENOSYS;
}

int radeon_gem_create_ioctl(struct drm_device *dev, void *data,
			    struct drm_file *filp)
{
	struct radeon_device *rdev = dev->dev_private;
	struct drm_radeon_gem_create *args = data;
	struct drm_gem_object *gobj;
	uint32_t handle;
	int r;

	down_read(&rdev->exclusive_lock);
	/* create a gem object to contain this object in */
	args->size = roundup(args->size, PAGE_SIZE);
	r = radeon_gem_object_create(rdev, args->size, args->alignment,
				     args->initial_domain, args->flags,
				     false, &gobj);
	if (r) {
		up_read(&rdev->exclusive_lock);
		r = radeon_gem_handle_lockup(rdev, r);
		return r;
	}
	r = drm_gem_handle_create(filp, gobj, &handle);
	/* drop reference from allocate - handle holds it now */
	drm_gem_object_unreference_unlocked(gobj);
	if (r) {
		up_read(&rdev->exclusive_lock);
		r = radeon_gem_handle_lockup(rdev, r);
		return r;
	}
	args->handle = handle;
	up_read(&rdev->exclusive_lock);
	return 0;
}

int radeon_gem_set_domain_ioctl(struct drm_device *dev, void *data,
				struct drm_file *filp)
{
	/* transition the BO to a domain -
	 * just validate the BO into a certain domain */
	struct radeon_device *rdev = dev->dev_private;
	struct drm_radeon_gem_set_domain *args = data;
	struct drm_gem_object *gobj;
	struct radeon_bo *robj;
	int r;

	/* for now if someone requests domain CPU -
	 * just make sure the buffer is finished with */
	down_read(&rdev->exclusive_lock);

	/* just do a BO wait for now */
	gobj = drm_gem_object_lookup(dev, filp, args->handle);
	if (gobj == NULL) {
		up_read(&rdev->exclusive_lock);
		return -ENOENT;
	}
	robj = gem_to_radeon_bo(gobj);

	r = radeon_gem_set_domain(gobj, args->read_domains, args->write_domain);

	drm_gem_object_unreference_unlocked(gobj);
	up_read(&rdev->exclusive_lock);
	r = radeon_gem_handle_lockup(robj->rdev, r);
	return r;
}

int radeon_mode_dumb_mmap(struct drm_file *filp,
			  struct drm_device *dev,
			  uint32_t handle, uint64_t *offset_p)
{
	struct drm_gem_object *gobj;
	struct radeon_bo *robj;

	gobj = drm_gem_object_lookup(dev, filp, handle);
	if (gobj == NULL) {
		return -ENOENT;
	}
	robj = gem_to_radeon_bo(gobj);
	*offset_p = radeon_bo_mmap_offset(robj);
	drm_gem_object_unreference_unlocked(gobj);
	return 0;
}

int radeon_gem_mmap_ioctl(struct drm_device *dev, void *data,
			  struct drm_file *filp)
{
	struct drm_radeon_gem_mmap *args = data;

	return radeon_mode_dumb_mmap(filp, dev, args->handle, &args->addr_ptr);
}

int radeon_gem_busy_ioctl(struct drm_device *dev, void *data,
			  struct drm_file *filp)
{
	struct drm_radeon_gem_busy *args = data;
	struct drm_gem_object *gobj;
	struct radeon_bo *robj;
	int r;
	uint32_t cur_placement = 0;

	gobj = drm_gem_object_lookup(dev, filp, args->handle);
	if (gobj == NULL) {
		return -ENOENT;
	}
	robj = gem_to_radeon_bo(gobj);
	r = radeon_bo_wait(robj, &cur_placement, true);
	args->domain = radeon_mem_type_to_domain(cur_placement);
	drm_gem_object_unreference_unlocked(gobj);
	return r;
}

int radeon_gem_wait_idle_ioctl(struct drm_device *dev, void *data,
			      struct drm_file *filp)
{
	struct radeon_device *rdev = dev->dev_private;
	struct drm_radeon_gem_wait_idle *args = data;
	struct drm_gem_object *gobj;
	struct radeon_bo *robj;
	int r = 0;
	uint32_t cur_placement = 0;
	long ret;

	gobj = drm_gem_object_lookup(dev, filp, args->handle);
	if (gobj == NULL) {
		return -ENOENT;
	}
	robj = gem_to_radeon_bo(gobj);
	r = radeon_bo_wait(robj, &cur_placement, false);
	/* Flush HDP cache via MMIO if necessary */
	if (rdev->asic->mmio_hdp_flush &&
	    radeon_mem_type_to_domain(cur_placement) == RADEON_GEM_DOMAIN_VRAM)
		robj->rdev->asic->mmio_hdp_flush(rdev);
	drm_gem_object_unreference_unlocked(gobj);
	r = radeon_gem_handle_lockup(rdev, r);
	return r;
}

int radeon_gem_set_tiling_ioctl(struct drm_device *dev, void *data,
				struct drm_file *filp)
{
	struct drm_radeon_gem_set_tiling *args = data;
	struct drm_gem_object *gobj;
	struct radeon_bo *robj;
	int r = 0;

	DRM_DEBUG("%d \n", args->handle);
	gobj = drm_gem_object_lookup(dev, filp, args->handle);
	if (gobj == NULL)
		return -ENOENT;
	robj = gem_to_radeon_bo(gobj);
	r = radeon_bo_set_tiling_flags(robj, args->tiling_flags, args->pitch);
	drm_gem_object_unreference_unlocked(gobj);
	return r;
}

#endif
