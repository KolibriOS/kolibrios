/*
 * Copyright 2009 Jerome Glisse.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 */
/*
 * Authors:
 *    Jerome Glisse <glisse@freedesktop.org>
 *    Thomas Hellstrom <thomas-at-tungstengraphics-dot-com>
 *    Dave Airlie
 */
#include <ttm/ttm_bo_api.h>
#include <ttm/ttm_bo_driver.h>
#include <ttm/ttm_placement.h>
#include <ttm/ttm_module.h>
#include <ttm/ttm_page_alloc.h>
#include <drm/drmP.h>
#include <drm/radeon_drm.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include "radeon_reg.h"
#include "radeon.h"

#define DRM_FILE_PAGE_OFFSET (0x100000000ULL >> PAGE_SHIFT)

static int radeon_ttm_debugfs_init(struct radeon_device *rdev);

static struct radeon_device *radeon_get_rdev(struct ttm_bo_device *bdev)
{
	struct radeon_mman *mman;
	struct radeon_device *rdev;

	mman = container_of(bdev, struct radeon_mman, bdev);
	rdev = container_of(mman, struct radeon_device, mman);
	return rdev;
}


/*
 * Global memory.
 */
static int radeon_ttm_mem_global_init(struct drm_global_reference *ref)
{
	return ttm_mem_global_init(ref->object);
}

static void radeon_ttm_mem_global_release(struct drm_global_reference *ref)
{
	ttm_mem_global_release(ref->object);
}

static int radeon_ttm_global_init(struct radeon_device *rdev)
{
	struct drm_global_reference *global_ref;
	int r;

    ENTER();

	rdev->mman.mem_global_referenced = false;
	global_ref = &rdev->mman.mem_global_ref;
	global_ref->global_type = DRM_GLOBAL_TTM_MEM;
	global_ref->size = sizeof(struct ttm_mem_global);
	global_ref->init = &radeon_ttm_mem_global_init;
	global_ref->release = &radeon_ttm_mem_global_release;
	r = drm_global_item_ref(global_ref);
	if (r != 0) {
		DRM_ERROR("Failed setting up TTM memory accounting "
			  "subsystem.\n");
		return r;
	}

	rdev->mman.bo_global_ref.mem_glob =
		rdev->mman.mem_global_ref.object;
	global_ref = &rdev->mman.bo_global_ref.ref;
	global_ref->global_type = DRM_GLOBAL_TTM_BO;
	global_ref->size = sizeof(struct ttm_bo_global);
	global_ref->init = &ttm_bo_global_init;
	global_ref->release = &ttm_bo_global_release;
	r = drm_global_item_ref(global_ref);
	if (r != 0) {
		DRM_ERROR("Failed setting up TTM BO subsystem.\n");
		drm_global_item_unref(&rdev->mman.mem_global_ref);
		return r;
	}

	rdev->mman.mem_global_referenced = true;

    LEAVE();

	return 0;
}



static int radeon_invalidate_caches(struct ttm_bo_device *bdev, uint32_t flags)
{
	return 0;
}

static int radeon_init_mem_type(struct ttm_bo_device *bdev, uint32_t type,
				struct ttm_mem_type_manager *man)
{
	struct radeon_device *rdev;

    ENTER();

	rdev = radeon_get_rdev(bdev);

	switch (type) {
	case TTM_PL_SYSTEM:
		/* System memory */
		man->flags = TTM_MEMTYPE_FLAG_MAPPABLE;
		man->available_caching = TTM_PL_MASK_CACHING;
		man->default_caching = TTM_PL_FLAG_CACHED;
		break;
	case TTM_PL_TT:
		man->func = &ttm_bo_manager_func;
		man->gpu_offset = rdev->mc.gtt_start;
		man->available_caching = TTM_PL_MASK_CACHING;
		man->default_caching = TTM_PL_FLAG_CACHED;
		man->flags = TTM_MEMTYPE_FLAG_MAPPABLE | TTM_MEMTYPE_FLAG_CMA;
#if __OS_HAS_AGP
		if (rdev->flags & RADEON_IS_AGP) {
			if (!(drm_core_has_AGP(rdev->ddev) && rdev->ddev->agp)) {
				DRM_ERROR("AGP is not enabled for memory type %u\n",
					  (unsigned)type);
				return -EINVAL;
			}
			if (!rdev->ddev->agp->cant_use_aperture)
				man->flags = TTM_MEMTYPE_FLAG_MAPPABLE;
			man->available_caching = TTM_PL_FLAG_UNCACHED |
						 TTM_PL_FLAG_WC;
			man->default_caching = TTM_PL_FLAG_WC;
		}
#endif
		break;
	case TTM_PL_VRAM:
		/* "On-card" video ram */
		man->func = &ttm_bo_manager_func;
		man->gpu_offset = rdev->mc.vram_start;
		man->flags = TTM_MEMTYPE_FLAG_FIXED |
			     TTM_MEMTYPE_FLAG_MAPPABLE;
		man->available_caching = TTM_PL_FLAG_UNCACHED | TTM_PL_FLAG_WC;
		man->default_caching = TTM_PL_FLAG_WC;
		break;
	default:
		DRM_ERROR("Unsupported memory type %u\n", (unsigned)type);
		return -EINVAL;
	}

    LEAVE();

	return 0;
}

static void radeon_evict_flags(struct ttm_buffer_object *bo,
				struct ttm_placement *placement)
{
	struct radeon_bo *rbo;
	static u32 placements = TTM_PL_MASK_CACHING | TTM_PL_FLAG_SYSTEM;

	if (!radeon_ttm_bo_is_radeon_bo(bo)) {
		placement->fpfn = 0;
		placement->lpfn = 0;
		placement->placement = &placements;
		placement->busy_placement = &placements;
		placement->num_placement = 1;
		placement->num_busy_placement = 1;
		return;
	}
	rbo = container_of(bo, struct radeon_bo, tbo);
	switch (bo->mem.mem_type) {
	case TTM_PL_VRAM:
		if (rbo->rdev->ring[RADEON_RING_TYPE_GFX_INDEX].ready == false)
			radeon_ttm_placement_from_domain(rbo, RADEON_GEM_DOMAIN_CPU);
		else
			radeon_ttm_placement_from_domain(rbo, RADEON_GEM_DOMAIN_GTT);
		break;
	case TTM_PL_TT:
	default:
		radeon_ttm_placement_from_domain(rbo, RADEON_GEM_DOMAIN_CPU);
	}
	*placement = rbo->placement;
}

static int radeon_verify_access(struct ttm_buffer_object *bo, struct file *filp)
{
	return 0;
}

static void radeon_move_null(struct ttm_buffer_object *bo,
			     struct ttm_mem_reg *new_mem)
{
	struct ttm_mem_reg *old_mem = &bo->mem;

	BUG_ON(old_mem->mm_node != NULL);
	*old_mem = *new_mem;
	new_mem->mm_node = NULL;
}

static void radeon_ttm_io_mem_free(struct ttm_bo_device *bdev, struct ttm_mem_reg *mem)
{
}

static int radeon_sync_obj_wait(void *sync_obj, bool lazy, bool interruptible)
{
	return radeon_fence_wait((struct radeon_fence *)sync_obj, interruptible);
}

static int radeon_sync_obj_flush(void *sync_obj)
{
	return 0;
}

static void radeon_sync_obj_unref(void **sync_obj)
{
	radeon_fence_unref((struct radeon_fence **)sync_obj);
}

static void *radeon_sync_obj_ref(void *sync_obj)
{
	return radeon_fence_ref((struct radeon_fence *)sync_obj);
}

static bool radeon_sync_obj_signaled(void *sync_obj)
{
	return radeon_fence_signaled((struct radeon_fence *)sync_obj);
}

/*
 * TTM backend functions.
 */
struct radeon_ttm_tt {
	struct ttm_dma_tt		ttm;
	struct radeon_device		*rdev;
	u64				offset;
};

static int radeon_ttm_backend_bind(struct ttm_tt *ttm,
				   struct ttm_mem_reg *bo_mem)
{
	struct radeon_ttm_tt *gtt = (void*)ttm;
	int r;

	gtt->offset = (unsigned long)(bo_mem->start << PAGE_SHIFT);
	if (!ttm->num_pages) {
		WARN(1, "nothing to bind %lu pages for mreg %p back %p!\n",
		     ttm->num_pages, bo_mem, ttm);
	}
	r = radeon_gart_bind(gtt->rdev, gtt->offset,
			     ttm->num_pages, ttm->pages, gtt->ttm.dma_address);
	if (r) {
		DRM_ERROR("failed to bind %lu pages at 0x%08X\n",
			  ttm->num_pages, (unsigned)gtt->offset);
		return r;
	}
	return 0;
}

static int radeon_ttm_backend_unbind(struct ttm_tt *ttm)
{
	struct radeon_ttm_tt *gtt = (void *)ttm;

	radeon_gart_unbind(gtt->rdev, gtt->offset, ttm->num_pages);
	return 0;
}

static void radeon_ttm_backend_destroy(struct ttm_tt *ttm)
{
	struct radeon_ttm_tt *gtt = (void *)ttm;

	ttm_dma_tt_fini(&gtt->ttm);
	kfree(gtt);
}

static struct ttm_backend_func radeon_backend_func = {
	.bind = &radeon_ttm_backend_bind,
	.unbind = &radeon_ttm_backend_unbind,
	.destroy = &radeon_ttm_backend_destroy,
};

static struct ttm_tt *radeon_ttm_tt_create(struct ttm_bo_device *bdev,
				    unsigned long size, uint32_t page_flags,
				    struct page *dummy_read_page)
{
	struct radeon_device *rdev;
	struct radeon_ttm_tt *gtt;

	rdev = radeon_get_rdev(bdev);
#if __OS_HAS_AGP
	if (rdev->flags & RADEON_IS_AGP) {
		return ttm_agp_tt_create(bdev, rdev->ddev->agp->bridge,
					 size, page_flags, dummy_read_page);
	}
#endif

	gtt = kzalloc(sizeof(struct radeon_ttm_tt), GFP_KERNEL);
	if (gtt == NULL) {
		return NULL;
	}
	gtt->ttm.ttm.func = &radeon_backend_func;
	gtt->rdev = rdev;
	if (ttm_dma_tt_init(&gtt->ttm, bdev, size, page_flags, dummy_read_page)) {
		kfree(gtt);
		return NULL;
	}
	return &gtt->ttm.ttm;
}

static struct ttm_bo_driver radeon_bo_driver = {
	.ttm_tt_create = &radeon_ttm_tt_create,
//	.ttm_tt_populate = &radeon_ttm_tt_populate,
//	.ttm_tt_unpopulate = &radeon_ttm_tt_unpopulate,
//	.invalidate_caches = &radeon_invalidate_caches,
	.init_mem_type = &radeon_init_mem_type,
//	.evict_flags = &radeon_evict_flags,
//	.move = &radeon_bo_move,
//	.verify_access = &radeon_verify_access,
//	.sync_obj_signaled = &radeon_sync_obj_signaled,
//	.sync_obj_wait = &radeon_sync_obj_wait,
//	.sync_obj_flush = &radeon_sync_obj_flush,
//	.sync_obj_unref = &radeon_sync_obj_unref,
//	.sync_obj_ref = &radeon_sync_obj_ref,
//	.move_notify = &radeon_bo_move_notify,
//	.fault_reserve_notify = &radeon_bo_fault_reserve_notify,
//	.io_mem_reserve = &radeon_ttm_io_mem_reserve,
//	.io_mem_free = &radeon_ttm_io_mem_free,
};

int radeon_ttm_init(struct radeon_device *rdev)
{
	int r;

    ENTER();

	r = radeon_ttm_global_init(rdev);
	if (r) {
		return r;
	}
	/* No others user of address space so set it to 0 */
	r = ttm_bo_device_init(&rdev->mman.bdev,
			       rdev->mman.bo_global_ref.ref.object,
			       &radeon_bo_driver, DRM_FILE_PAGE_OFFSET,
			       rdev->need_dma32);
	if (r) {
		DRM_ERROR("failed initializing buffer object driver(%d).\n", r);
		return r;
	}
	rdev->mman.initialized = true;
	r = ttm_bo_init_mm(&rdev->mman.bdev, TTM_PL_VRAM,
				rdev->mc.real_vram_size >> PAGE_SHIFT);
	if (r) {
		DRM_ERROR("Failed initializing VRAM heap.\n");
		return r;
	}

//   r = radeon_bo_create(rdev, 256 * 1024, PAGE_SIZE, true,
//               RADEON_GEM_DOMAIN_VRAM,
//                NULL, &rdev->stollen_vga_memory);
//   if (r) {
//       return r;
//   }
//   r = radeon_bo_reserve(rdev->stollen_vga_memory, false);
//   if (r)
//       return r;
//   r = radeon_bo_pin(rdev->stollen_vga_memory, RADEON_GEM_DOMAIN_VRAM, NULL);
//   radeon_bo_unreserve(rdev->stollen_vga_memory);
//   if (r) {
//       radeon_bo_unref(&rdev->stollen_vga_memory);
//       return r;
//   }

	DRM_INFO("radeon: %uM of VRAM memory ready\n",
		 (unsigned)rdev->mc.real_vram_size / (1024 * 1024));
	r = ttm_bo_init_mm(&rdev->mman.bdev, TTM_PL_TT,
				rdev->mc.gtt_size >> PAGE_SHIFT);
	if (r) {
		DRM_ERROR("Failed initializing GTT heap.\n");
		return r;
	}
	DRM_INFO("radeon: %uM of GTT memory ready.\n",
		 (unsigned)(rdev->mc.gtt_size / (1024 * 1024)));
		rdev->mman.bdev.dev_mapping = rdev->ddev->dev_mapping;

    LEAVE();

    return 0;
}


/* this should only be called at bootup or when userspace
 * isn't running */
void radeon_ttm_set_active_vram_size(struct radeon_device *rdev, u64 size)
{
	struct ttm_mem_type_manager *man;

	if (!rdev->mman.initialized)
		return;

	man = &rdev->mman.bdev.man[TTM_PL_VRAM];
	/* this just adjusts TTM size idea, which sets lpfn to the correct value */
	man->size = size >> PAGE_SHIFT;
}

static struct vm_operations_struct radeon_ttm_vm_ops;
static const struct vm_operations_struct *ttm_vm_ops = NULL;

#if 0

radeon_bo_init
{
    <6>[drm] Detected VRAM RAM=1024M, BAR=256M
    <6>[drm] RAM width 128bits DDR

    radeon_ttm_init
    {
        radeon_ttm_global_init
        {
            radeon_ttm_mem_global_init

            ttm_bo_global_init
        }

        ttm_bo_device_init
        {
            ttm_bo_init_mm
            {
                radeon_init_mem_type
            };
        }

        ttm_bo_init_mm
        {
            radeon_init_mem_type

            ttm_bo_man_init
        }

        <6>[drm] radeon: 1024M of VRAM memory ready

        ttm_bo_init_mm
        {
            radeon_init_mem_type

            ttm_bo_man_init
        }

        <6>[drm] radeon: 512M of GTT memory ready.
    }
};

#endif




