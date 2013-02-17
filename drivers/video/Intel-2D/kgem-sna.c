/*
 * Copyright (c) 2011 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sna.h"
#include "sna_reg.h"

#define DBG_NO_HW 0
#define DBG_NO_TILING 1
#define DBG_NO_CACHE 0
#define DBG_NO_CACHE_LEVEL 0
#define DBG_NO_CPU 0
#define DBG_NO_USERPTR 0
#define DBG_NO_LLC 0
#define DBG_NO_SEMAPHORES 0
#define DBG_NO_MADV 0
#define DBG_NO_UPLOAD_CACHE 0
#define DBG_NO_UPLOAD_ACTIVE 0
#define DBG_NO_MAP_UPLOAD 0
#define DBG_NO_RELAXED_FENCING 0
#define DBG_NO_SECURE_BATCHES 0
#define DBG_NO_PINNED_BATCHES 0
#define DBG_NO_FAST_RELOC 0
#define DBG_NO_HANDLE_LUT 0
#define DBG_DUMP 0

#define MAX_GTT_VMA_CACHE 512
#define MAX_CPU_VMA_CACHE INT16_MAX
#define MAP_PRESERVE_TIME 10

#define MAP(ptr) ((void*)((uintptr_t)(ptr) & ~3))
#define MAKE_CPU_MAP(ptr) ((void*)((uintptr_t)(ptr) | 1))
#define MAKE_USER_MAP(ptr) ((void*)((uintptr_t)(ptr) | 3))
#define IS_USER_MAP(ptr) ((uintptr_t)(ptr) & 2)
#define __MAP_TYPE(ptr) ((uintptr_t)(ptr) & 3)

#define MAKE_REQUEST(rq, ring) ((struct kgem_request *)((uintptr_t)(rq) | (ring)))

#define LOCAL_I915_PARAM_HAS_BLT		        11
#define LOCAL_I915_PARAM_HAS_RELAXED_FENCING	12
#define LOCAL_I915_PARAM_HAS_RELAXED_DELTA	    15
#define LOCAL_I915_PARAM_HAS_SEMAPHORES		    20
#define LOCAL_I915_PARAM_HAS_SECURE_BATCHES	    23
#define LOCAL_I915_PARAM_HAS_PINNED_BATCHES	    24
#define LOCAL_I915_PARAM_HAS_NO_RELOC		    25
#define LOCAL_I915_PARAM_HAS_HANDLE_LUT		    26



static int gem_param(struct kgem *kgem, int name)
{
    ioctl_t  io;

    drm_i915_getparam_t gp;
    int v = -1; /* No param uses the sign bit, reserve it for errors */

    VG_CLEAR(gp);
    gp.param = name;
    gp.value = &v;

    io.handle   = kgem->fd;
    io.io_code  = SRV_GET_PARAM;
    io.input    = &gp;
    io.inp_size = sizeof(gp);
    io.output   = NULL;
    io.out_size = 0;

    if (call_service(&io)!=0)
        return -1;

    VG(VALGRIND_MAKE_MEM_DEFINED(&v, sizeof(v)));
    return v;
}

static bool test_has_no_reloc(struct kgem *kgem)
{
	if (DBG_NO_FAST_RELOC)
		return false;

	return gem_param(kgem, LOCAL_I915_PARAM_HAS_NO_RELOC) > 0;
}

static bool test_has_handle_lut(struct kgem *kgem)
{
	if (DBG_NO_HANDLE_LUT)
		return false;

	return gem_param(kgem, LOCAL_I915_PARAM_HAS_HANDLE_LUT) > 0;
}

static bool test_has_semaphores_enabled(struct kgem *kgem)
{
	FILE *file;
	bool detected = false;
	int ret;

	if (DBG_NO_SEMAPHORES)
		return false;

	ret = gem_param(kgem, LOCAL_I915_PARAM_HAS_SEMAPHORES);
	if (ret != -1)
		return ret > 0;

	return detected;
}


static bool test_has_relaxed_fencing(struct kgem *kgem)
{
	if (kgem->gen < 040) {
		if (DBG_NO_RELAXED_FENCING)
			return false;

		return gem_param(kgem, LOCAL_I915_PARAM_HAS_RELAXED_FENCING) > 0;
	} else
		return true;
}

static bool test_has_llc(struct kgem *kgem)
{
	int has_llc = -1;

	if (DBG_NO_LLC)
		return false;

#if defined(I915_PARAM_HAS_LLC) /* Expected in libdrm-2.4.31 */
	has_llc = gem_param(kgem, I915_PARAM_HAS_LLC);
#endif
	if (has_llc == -1) {
		DBG(("%s: no kernel/drm support for HAS_LLC, assuming support for LLC based on GPU generation\n", __FUNCTION__));
		has_llc = kgem->gen >= 060;
	}

	return has_llc;
}

static bool test_has_cacheing(struct kgem *kgem)
{
	uint32_t handle;
	bool ret = false;

	if (DBG_NO_CACHE_LEVEL)
		return false;

	/* Incoherent blt and sampler hangs the GPU */
	if (kgem->gen == 040)
		return false;

//	handle = gem_create(kgem->fd, 1);
//	if (handle == 0)
//		return false;

//	ret = gem_set_cacheing(kgem->fd, handle, UNCACHED);
//	gem_close(kgem->fd, handle);
	return ret;
}

static bool test_has_userptr(struct kgem *kgem)
{
#if defined(USE_USERPTR)
	uint32_t handle;
	void *ptr;

	if (DBG_NO_USERPTR)
		return false;

	/* Incoherent blt and sampler hangs the GPU */
	if (kgem->gen == 040)
		return false;

	ptr = malloc(PAGE_SIZE);
	handle = gem_userptr(kgem->fd, ptr, PAGE_SIZE, false);
	gem_close(kgem->fd, handle);
	free(ptr);

	return handle != 0;
#else
	return false;
#endif
}

static bool test_has_secure_batches(struct kgem *kgem)
{
	if (DBG_NO_SECURE_BATCHES)
		return false;

	return gem_param(kgem, LOCAL_I915_PARAM_HAS_SECURE_BATCHES) > 0;
}

static bool test_has_pinned_batches(struct kgem *kgem)
{
	if (DBG_NO_PINNED_BATCHES)
		return false;

	return gem_param(kgem, LOCAL_I915_PARAM_HAS_PINNED_BATCHES) > 0;
}



void kgem_init(struct kgem *kgem, int fd, struct pci_device *dev, unsigned gen)
{
    struct drm_i915_gem_get_aperture aperture;
    size_t totalram;
    unsigned half_gpu_max;
    unsigned int i, j;

    DBG(("%s: fd=%d, gen=%d\n", __FUNCTION__, fd, gen));

    memset(kgem, 0, sizeof(*kgem));

    kgem->fd = fd;
    kgem->gen = gen;

    list_init(&kgem->requests[0]);
    list_init(&kgem->requests[1]);
    list_init(&kgem->batch_buffers);
    list_init(&kgem->active_buffers);
    list_init(&kgem->flushing);
    list_init(&kgem->large);
    list_init(&kgem->large_inactive);
    list_init(&kgem->snoop);
    list_init(&kgem->scanout);
    for (i = 0; i < ARRAY_SIZE(kgem->pinned_batches); i++)
        list_init(&kgem->pinned_batches[i]);
    for (i = 0; i < ARRAY_SIZE(kgem->inactive); i++)
        list_init(&kgem->inactive[i]);
    for (i = 0; i < ARRAY_SIZE(kgem->active); i++) {
        for (j = 0; j < ARRAY_SIZE(kgem->active[i]); j++)
            list_init(&kgem->active[i][j]);
    }
    for (i = 0; i < ARRAY_SIZE(kgem->vma); i++) {
        for (j = 0; j < ARRAY_SIZE(kgem->vma[i].inactive); j++)
            list_init(&kgem->vma[i].inactive[j]);
    }

    kgem->vma[MAP_GTT].count = -MAX_GTT_VMA_CACHE;
    kgem->vma[MAP_CPU].count = -MAX_CPU_VMA_CACHE;

    kgem->has_blt = gem_param(kgem, LOCAL_I915_PARAM_HAS_BLT) > 0;
    DBG(("%s: has BLT ring? %d\n", __FUNCTION__,
         kgem->has_blt));

    kgem->has_relaxed_delta =
        gem_param(kgem, LOCAL_I915_PARAM_HAS_RELAXED_DELTA) > 0;
    DBG(("%s: has relaxed delta? %d\n", __FUNCTION__,
         kgem->has_relaxed_delta));


    kgem->has_relaxed_fencing = test_has_relaxed_fencing(kgem);
    DBG(("%s: has relaxed fencing? %d\n", __FUNCTION__,
         kgem->has_relaxed_fencing));

    kgem->has_llc = test_has_llc(kgem);
    DBG(("%s: has shared last-level-cache? %d\n", __FUNCTION__,
         kgem->has_llc));

    kgem->has_cacheing = test_has_cacheing(kgem);
    DBG(("%s: has set-cache-level? %d\n", __FUNCTION__,
         kgem->has_cacheing));

    kgem->has_userptr = test_has_userptr(kgem);
    DBG(("%s: has userptr? %d\n", __FUNCTION__,
         kgem->has_userptr));

    kgem->has_no_reloc = test_has_no_reloc(kgem);
    DBG(("%s: has no-reloc? %d\n", __FUNCTION__,
         kgem->has_no_reloc));

    kgem->has_handle_lut = test_has_handle_lut(kgem);
    DBG(("%s: has handle-lut? %d\n", __FUNCTION__,
         kgem->has_handle_lut));

    kgem->has_semaphores = false;
    if (kgem->has_blt && test_has_semaphores_enabled(kgem))
        kgem->has_semaphores = true;
    DBG(("%s: semaphores enabled? %d\n", __FUNCTION__,
         kgem->has_semaphores));

    kgem->can_blt_cpu = gen >= 030;
    DBG(("%s: can blt to cpu? %d\n", __FUNCTION__,
         kgem->can_blt_cpu));

    kgem->has_secure_batches = test_has_secure_batches(kgem);
    DBG(("%s: can use privileged batchbuffers? %d\n", __FUNCTION__,
         kgem->has_secure_batches));

    kgem->has_pinned_batches = test_has_pinned_batches(kgem);
    DBG(("%s: can use pinned batchbuffers (to avoid CS w/a)? %d\n", __FUNCTION__,
         kgem->has_pinned_batches));

#if 0

    if (!is_hw_supported(kgem, dev)) {
        xf86DrvMsg(kgem_get_screen_index(kgem), X_WARNING,
               "Detected unsupported/dysfunctional hardware, disabling acceleration.\n");
        kgem->wedged = 1;
    } else if (__kgem_throttle(kgem)) {
        xf86DrvMsg(kgem_get_screen_index(kgem), X_WARNING,
               "Detected a hung GPU, disabling acceleration.\n");
        kgem->wedged = 1;
    }

    kgem->batch_size = ARRAY_SIZE(kgem->batch);
    if (gen == 020 && !kgem->has_pinned_batches)
        /* Limited to what we can pin */
        kgem->batch_size = 4*1024;
    if (gen == 022)
        /* 865g cannot handle a batch spanning multiple pages */
        kgem->batch_size = PAGE_SIZE / sizeof(uint32_t);
    if ((gen >> 3) == 7)
        kgem->batch_size = 16*1024;
    if (!kgem->has_relaxed_delta && kgem->batch_size > 4*1024)
        kgem->batch_size = 4*1024;

    if (!kgem_init_pinned_batches(kgem) && gen == 020) {
        xf86DrvMsg(kgem_get_screen_index(kgem), X_WARNING,
               "Unable to reserve memory for GPU, disabling acceleration.\n");
        kgem->wedged = 1;
    }

    DBG(("%s: maximum batch size? %d\n", __FUNCTION__,
         kgem->batch_size));

    kgem->min_alignment = 4;
    if (gen < 040)
        kgem->min_alignment = 64;

    kgem->half_cpu_cache_pages = cpu_cache_size() >> 13;
    DBG(("%s: half cpu cache %d pages\n", __FUNCTION__,
         kgem->half_cpu_cache_pages));

    kgem->next_request = __kgem_request_alloc(kgem);

    DBG(("%s: cpu bo enabled %d: llc? %d, set-cache-level? %d, userptr? %d\n", __FUNCTION__,
         !DBG_NO_CPU && (kgem->has_llc | kgem->has_userptr | kgem->has_cacheing),
         kgem->has_llc, kgem->has_cacheing, kgem->has_userptr));

    VG_CLEAR(aperture);
    aperture.aper_size = 0;
    (void)drmIoctl(fd, DRM_IOCTL_I915_GEM_GET_APERTURE, &aperture);
    if (aperture.aper_size == 0)
        aperture.aper_size = 64*1024*1024;

    DBG(("%s: aperture size %lld, available now %lld\n",
         __FUNCTION__,
         (long long)aperture.aper_size,
         (long long)aperture.aper_available_size));

    kgem->aperture_total = aperture.aper_size;
    kgem->aperture_high = aperture.aper_size * 3/4;
    kgem->aperture_low = aperture.aper_size * 1/3;
    if (gen < 033) {
        /* Severe alignment penalties */
        kgem->aperture_high /= 2;
        kgem->aperture_low /= 2;
    }
    DBG(("%s: aperture low=%d [%d], high=%d [%d]\n", __FUNCTION__,
         kgem->aperture_low, kgem->aperture_low / (1024*1024),
         kgem->aperture_high, kgem->aperture_high / (1024*1024)));

    kgem->aperture_mappable = agp_aperture_size(dev, gen);
    if (kgem->aperture_mappable == 0 ||
        kgem->aperture_mappable > aperture.aper_size)
        kgem->aperture_mappable = aperture.aper_size;
    DBG(("%s: aperture mappable=%d [%d MiB]\n", __FUNCTION__,
         kgem->aperture_mappable, kgem->aperture_mappable / (1024*1024)));

    kgem->buffer_size = 64 * 1024;
    while (kgem->buffer_size < kgem->aperture_mappable >> 10)
        kgem->buffer_size *= 2;
    if (kgem->buffer_size >> 12 > kgem->half_cpu_cache_pages)
        kgem->buffer_size = kgem->half_cpu_cache_pages << 12;
    DBG(("%s: buffer size=%d [%d KiB]\n", __FUNCTION__,
         kgem->buffer_size, kgem->buffer_size / 1024));

    kgem->max_object_size = 3 * (kgem->aperture_high >> 12) << 10;
    kgem->max_gpu_size = kgem->max_object_size;
    if (!kgem->has_llc)
        kgem->max_gpu_size = MAX_CACHE_SIZE;

    totalram = total_ram_size();
    if (totalram == 0) {
        DBG(("%s: total ram size unknown, assuming maximum of total aperture\n",
             __FUNCTION__));
        totalram = kgem->aperture_total;
    }
    DBG(("%s: total ram=%ld\n", __FUNCTION__, (long)totalram));
    if (kgem->max_object_size > totalram / 2)
        kgem->max_object_size = totalram / 2;
    if (kgem->max_gpu_size > totalram / 4)
        kgem->max_gpu_size = totalram / 4;

    kgem->max_cpu_size = kgem->max_object_size;

    half_gpu_max = kgem->max_gpu_size / 2;
    kgem->max_copy_tile_size = (MAX_CACHE_SIZE + 1)/2;
    if (kgem->max_copy_tile_size > half_gpu_max)
        kgem->max_copy_tile_size = half_gpu_max;

    if (kgem->has_llc)
        kgem->max_upload_tile_size = kgem->max_copy_tile_size;
    else
        kgem->max_upload_tile_size = kgem->aperture_mappable / 4;
    if (kgem->max_upload_tile_size > half_gpu_max)
        kgem->max_upload_tile_size = half_gpu_max;

    kgem->large_object_size = MAX_CACHE_SIZE;
    if (kgem->large_object_size > kgem->max_gpu_size)
        kgem->large_object_size = kgem->max_gpu_size;

    if (kgem->has_llc | kgem->has_cacheing | kgem->has_userptr) {
        if (kgem->large_object_size > kgem->max_cpu_size)
            kgem->large_object_size = kgem->max_cpu_size;
    } else
        kgem->max_cpu_size = 0;
    if (DBG_NO_CPU)
        kgem->max_cpu_size = 0;

    DBG(("%s: maximum object size=%d\n",
         __FUNCTION__, kgem->max_object_size));
    DBG(("%s: large object thresold=%d\n",
         __FUNCTION__, kgem->large_object_size));
    DBG(("%s: max object sizes (gpu=%d, cpu=%d, tile upload=%d, copy=%d)\n",
         __FUNCTION__,
         kgem->max_gpu_size, kgem->max_cpu_size,
         kgem->max_upload_tile_size, kgem->max_copy_tile_size));

    /* Convert the aperture thresholds to pages */
    kgem->aperture_low /= PAGE_SIZE;
    kgem->aperture_high /= PAGE_SIZE;

    kgem->fence_max = gem_param(kgem, I915_PARAM_NUM_FENCES_AVAIL) - 2;
    if ((int)kgem->fence_max < 0)
        kgem->fence_max = 5; /* minimum safe value for all hw */
    DBG(("%s: max fences=%d\n", __FUNCTION__, kgem->fence_max));

    kgem->batch_flags_base = 0;
    if (kgem->has_no_reloc)
        kgem->batch_flags_base |= LOCAL_I915_EXEC_NO_RELOC;
    if (kgem->has_handle_lut)
        kgem->batch_flags_base |= LOCAL_I915_EXEC_HANDLE_LUT;
    if (kgem->has_pinned_batches)
        kgem->batch_flags_base |= LOCAL_I915_EXEC_IS_PINNED;

#endif

}



void kgem_clear_dirty(struct kgem *kgem)
{
	struct list * const buffers = &kgem->next_request->buffers;
	struct kgem_bo *bo;

	list_for_each_entry(bo, buffers, request) {
		if (!bo->dirty)
			break;

		bo->dirty = false;
	}
}



uint32_t kgem_bo_get_binding(struct kgem_bo *bo, uint32_t format)
{
	struct kgem_bo_binding *b;

	for (b = &bo->binding; b && b->offset; b = b->next)
		if (format == b->format)
			return b->offset;

	return 0;
}

void kgem_bo_set_binding(struct kgem_bo *bo, uint32_t format, uint16_t offset)
{
	struct kgem_bo_binding *b;

	for (b = &bo->binding; b; b = b->next) {
		if (b->offset)
			continue;

		b->offset = offset;
		b->format = format;

		if (b->next)
			b->next->offset = 0;

		return;
	}

	b = malloc(sizeof(*b));
	if (b) {
		b->next = bo->binding.next;
		b->format = format;
		b->offset = offset;
		bo->binding.next = b;
	}
}

uint32_t kgem_add_reloc(struct kgem *kgem,
			uint32_t pos,
			struct kgem_bo *bo,
			uint32_t read_write_domain,
			uint32_t delta)
{
    return 0;
}

void kgem_reset(struct kgem *kgem)
{

};

void _kgem_submit(struct kgem *kgem)
{
};

struct kgem_bo *kgem_create_linear(struct kgem *kgem, int size, unsigned flags)
{
	struct kgem_bo *bo = NULL;

	return bo;
};

void _kgem_bo_destroy(struct kgem *kgem, struct kgem_bo *bo)
{


};
