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


unsigned int cpu_cache_size();

static struct kgem_bo *
search_linear_cache(struct kgem *kgem, unsigned int num_pages, unsigned flags);

static struct kgem_bo *
search_snoop_cache(struct kgem *kgem, unsigned int num_pages, unsigned flags);

#define DBG_NO_HW 0
#define DBG_NO_TILING 1
#define DBG_NO_CACHE 0
#define DBG_NO_CACHE_LEVEL 0
#define DBG_NO_CPU 0
#define DBG_NO_USERPTR 0
#define DBG_NO_LLC 0
#define DBG_NO_SEMAPHORES 0
#define DBG_NO_MADV 1
#define DBG_NO_UPLOAD_CACHE 0
#define DBG_NO_UPLOAD_ACTIVE 0
#define DBG_NO_MAP_UPLOAD 0
#define DBG_NO_RELAXED_FENCING 0
#define DBG_NO_SECURE_BATCHES 0
#define DBG_NO_PINNED_BATCHES 0
#define DBG_NO_FAST_RELOC 0
#define DBG_NO_HANDLE_LUT 0
#define DBG_DUMP 0

#ifndef DEBUG_SYNC
#define DEBUG_SYNC 0
#endif

#define SHOW_BATCH 0

#if 0
#define ASSERT_IDLE(kgem__, handle__) assert(!__kgem_busy(kgem__, handle__))
#define ASSERT_MAYBE_IDLE(kgem__, handle__, expect__) assert(!(expect__) || !__kgem_busy(kgem__, handle__))
#else
#define ASSERT_IDLE(kgem__, handle__)
#define ASSERT_MAYBE_IDLE(kgem__, handle__, expect__)
#endif

/* Worst case seems to be 965gm where we cannot write within a cacheline that
 * is being simultaneously being read by the GPU, or within the sampler
 * prefetch. In general, the chipsets seem to have a requirement that sampler
 * offsets be aligned to a cacheline (64 bytes).
 */
#define UPLOAD_ALIGNMENT 128

#define PAGE_ALIGN(x) ALIGN(x, PAGE_SIZE)
#define NUM_PAGES(x) (((x) + PAGE_SIZE-1) / PAGE_SIZE)

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

#define LOCAL_I915_EXEC_IS_PINNED		(1<<10)
#define LOCAL_I915_EXEC_NO_RELOC		(1<<11)
#define LOCAL_I915_EXEC_HANDLE_LUT		(1<<12)
#define UNCACHED	0
#define SNOOPED		1

struct local_i915_gem_cacheing {
	uint32_t handle;
	uint32_t cacheing;
};
static struct kgem_bo *__kgem_freed_bo;
static struct kgem_request *__kgem_freed_request;

#define bucket(B) (B)->size.pages.bucket
#define num_pages(B) (B)->size.pages.count

#ifdef DEBUG_MEMORY
static void debug_alloc(struct kgem *kgem, size_t size)
{
	kgem->debug_memory.bo_allocs++;
	kgem->debug_memory.bo_bytes += size;
}
static void debug_alloc__bo(struct kgem *kgem, struct kgem_bo *bo)
{
	debug_alloc(kgem, bytes(bo));
}
#else
#define debug_alloc(k, b)
#define debug_alloc__bo(k, b)
#endif

static bool gem_set_tiling(int fd, uint32_t handle, int tiling, int stride)
{
	struct drm_i915_gem_set_tiling set_tiling;
	int ret;

	if (DBG_NO_TILING)
		return false;
/*
	VG_CLEAR(set_tiling);
	do {
		set_tiling.handle = handle;
		set_tiling.tiling_mode = tiling;
		set_tiling.stride = stride;

		ret = ioctl(fd, DRM_IOCTL_I915_GEM_SET_TILING, &set_tiling);
	} while (ret == -1 && (errno == EINTR || errno == EAGAIN));
*/	
	return ret == 0;
}

static bool gem_set_cacheing(int fd, uint32_t handle, int cacheing)
{
	struct local_i915_gem_cacheing arg;
    ioctl_t  io;

	VG_CLEAR(arg);
	arg.handle = handle;
	arg.cacheing = cacheing;
	
    io.handle   = fd;
    io.io_code  = SRV_I915_GEM_SET_CACHEING;
    io.input    = &arg;
    io.inp_size = sizeof(arg);
    io.output   = NULL;
    io.out_size = 0;

	return call_service(&io) == 0;
	
}

static bool __kgem_throttle_retire(struct kgem *kgem, unsigned flags)
{
	if (flags & CREATE_NO_RETIRE) {
		DBG(("%s: not retiring per-request\n", __FUNCTION__));
		return false;
	}

	if (!kgem->need_retire) {
		DBG(("%s: nothing to retire\n", __FUNCTION__));
		return false;
	}

//	if (kgem_retire(kgem))
//		return true;

	if (flags & CREATE_NO_THROTTLE || !kgem->need_throttle) {
		DBG(("%s: not throttling\n", __FUNCTION__));
		return false;
	}

//	kgem_throttle(kgem);
//	return kgem_retire(kgem);
		return false;

}

static int gem_write(int fd, uint32_t handle,
		     int offset, int length,
		     const void *src)
{
	struct drm_i915_gem_pwrite pwrite;

	DBG(("%s(handle=%d, offset=%d, len=%d)\n", __FUNCTION__,
	     handle, offset, length));

	VG_CLEAR(pwrite);
	pwrite.handle = handle;
	/* align the transfer to cachelines; fortuitously this is safe! */
	if ((offset | length) & 63) {
		pwrite.offset = offset & ~63;
		pwrite.size = ALIGN(offset+length, 64) - pwrite.offset;
		pwrite.data_ptr = (uintptr_t)src + pwrite.offset - offset;
	} else {
		pwrite.offset = offset;
		pwrite.size = length;
		pwrite.data_ptr = (uintptr_t)src;
	}
//	return drmIoctl(fd, DRM_IOCTL_I915_GEM_PWRITE, &pwrite);
    return -1;
}


bool kgem_bo_write(struct kgem *kgem, struct kgem_bo *bo,
		   const void *data, int length)
{
	assert(bo->refcnt);
	assert(!bo->purged);
	assert(bo->proxy == NULL);
	ASSERT_IDLE(kgem, bo->handle);

	assert(length <= bytes(bo));
	if (gem_write(kgem->fd, bo->handle, 0, length, data))
		return false;

	DBG(("%s: flush=%d, domain=%d\n", __FUNCTION__, bo->flush, bo->domain));
	if (bo->exec == NULL) {
//		kgem_bo_retire(kgem, bo);
		bo->domain = DOMAIN_NONE;
	}
	return true;
}

static uint32_t gem_create(int fd, int num_pages)
{
	struct drm_i915_gem_create create;
    ioctl_t  io;

	VG_CLEAR(create);
	create.handle = 0;
	create.size = PAGE_SIZE * num_pages;
	
    io.handle   = fd;
    io.io_code  = SRV_I915_GEM_CREATE;
    io.input    = &create;
    io.inp_size = sizeof(create);
    io.output   = NULL;
    io.out_size = 0;

    if (call_service(&io)!=0)
        return 0;

	return create.handle;
}

static bool
kgem_bo_set_purgeable(struct kgem *kgem, struct kgem_bo *bo)
{
#if DBG_NO_MADV
	return true;
#else
	struct drm_i915_gem_madvise madv;

	assert(bo->exec == NULL);
	assert(!bo->purged);

	VG_CLEAR(madv);
	madv.handle = bo->handle;
	madv.madv = I915_MADV_DONTNEED;
	if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_MADVISE, &madv) == 0) {
		bo->purged = 1;
		kgem->need_purge |= !madv.retained && bo->domain == DOMAIN_GPU;
		return madv.retained;
	}

	return true;
#endif
}

static bool
kgem_bo_is_retained(struct kgem *kgem, struct kgem_bo *bo)
{
#if DBG_NO_MADV
	return true;
#else
	struct drm_i915_gem_madvise madv;

	if (!bo->purged)
		return true;

	VG_CLEAR(madv);
	madv.handle = bo->handle;
	madv.madv = I915_MADV_DONTNEED;
	if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_MADVISE, &madv) == 0)
		return madv.retained;

	return false;
#endif
}

static bool
kgem_bo_clear_purgeable(struct kgem *kgem, struct kgem_bo *bo)
{
#if DBG_NO_MADV
	return true;
#else
	struct drm_i915_gem_madvise madv;

	assert(bo->purged);

	VG_CLEAR(madv);
	madv.handle = bo->handle;
	madv.madv = I915_MADV_WILLNEED;
	if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_MADVISE, &madv) == 0) {
		bo->purged = !madv.retained;
		kgem->need_purge |= !madv.retained && bo->domain == DOMAIN_GPU;
		return madv.retained;
	}

	return false;
#endif
}

static void gem_close(int fd, uint32_t handle)
{
	struct drm_gem_close close;
    ioctl_t  io;

	VG_CLEAR(close);
	close.handle = handle;

    io.handle   = fd;
    io.io_code  = SRV_DRM_GEM_CLOSE;
    io.input    = &close;
    io.inp_size = sizeof(close);
    io.output   = NULL;
    io.out_size = 0;

    call_service(&io);
}

constant inline static unsigned long __fls(unsigned long word)
{
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86__) || defined(__x86_64__))
	asm("bsr %1,%0"
	    : "=r" (word)
	    : "rm" (word));
	return word;
#else
	unsigned int v = 0;

	while (word >>= 1)
		v++;

	return v;
#endif
}

constant inline static int cache_bucket(int num_pages)
{
	return __fls(num_pages);
}

static struct kgem_bo *__kgem_bo_init(struct kgem_bo *bo,
				      int handle, int num_pages)
{
	assert(num_pages);
	memset(bo, 0, sizeof(*bo));

	bo->refcnt = 1;
	bo->handle = handle;
	bo->target_handle = -1;
	num_pages(bo) = num_pages;
	bucket(bo) = cache_bucket(num_pages);
	bo->reusable = true;
	bo->domain = DOMAIN_CPU;
	list_init(&bo->request);
	list_init(&bo->list);
	list_init(&bo->vma);

	return bo;
}

static struct kgem_bo *__kgem_bo_alloc(int handle, int num_pages)
{
	struct kgem_bo *bo;

	if (__kgem_freed_bo) {
		bo = __kgem_freed_bo;
		__kgem_freed_bo = *(struct kgem_bo **)bo;
	} else {
		bo = malloc(sizeof(*bo));
		if (bo == NULL)
			return NULL;
	}

	return __kgem_bo_init(bo, handle, num_pages);
}

static struct kgem_request *__kgem_request_alloc(struct kgem *kgem)
{
	struct kgem_request *rq;

	rq = __kgem_freed_request;
	if (rq) {
		__kgem_freed_request = *(struct kgem_request **)rq;
	} else {
		rq = malloc(sizeof(*rq));
		if (rq == NULL)
			rq = &kgem->static_request;
	}

	list_init(&rq->buffers);
	rq->bo = NULL;
	rq->ring = 0;

	return rq;
}

static void __kgem_request_free(struct kgem_request *rq)
{
	_list_del(&rq->list);
	*(struct kgem_request **)rq = __kgem_freed_request;
	__kgem_freed_request = rq;
}

static struct list *inactive(struct kgem *kgem, int num_pages)
{
	assert(num_pages < MAX_CACHE_SIZE / PAGE_SIZE);
	assert(cache_bucket(num_pages) < NUM_CACHE_BUCKETS);
	return &kgem->inactive[cache_bucket(num_pages)];
}

static struct list *active(struct kgem *kgem, int num_pages, int tiling)
{
	assert(num_pages < MAX_CACHE_SIZE / PAGE_SIZE);
	assert(cache_bucket(num_pages) < NUM_CACHE_BUCKETS);
	return &kgem->active[cache_bucket(num_pages)][tiling];
}

static size_t
agp_aperture_size(struct pci_device *dev, unsigned gen)
{
	/* XXX assume that only future chipsets are unknown and follow
	 * the post gen2 PCI layout.
	 */
//	return dev->regions[gen < 030 ? 0 : 2].size;

    return 0;
}

static size_t
total_ram_size(void)
{
    uint32_t  data[9];
    size_t    size = 0;
    
    asm volatile("int $0x40"
        : "=a" (size)
        : "a" (18),"b"(20), "c" (data)
        : "memory");
    
    return size != -1 ? size : 0;
}


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

static bool test_has_execbuffer2(struct kgem *kgem)
{
	return 1;
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

static bool __kgem_throttle(struct kgem *kgem)
{
//	if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_THROTTLE, NULL) == 0)
		return false;

//	return errno == EIO;
}

static bool is_hw_supported(struct kgem *kgem,
			    struct pci_device *dev)
{
	if (DBG_NO_HW)
		return false;

	if (!test_has_execbuffer2(kgem))
		return false;

	if (kgem->gen == (unsigned)-1) /* unknown chipset, assume future gen */
		return kgem->has_blt;

	/* Although pre-855gm the GMCH is fubar, it works mostly. So
	 * let the user decide through "NoAccel" whether or not to risk
	 * hw acceleration.
	 */

	if (kgem->gen == 060 && dev->revision < 8) {
		/* pre-production SNB with dysfunctional BLT */
		return false;
	}

	if (kgem->gen >= 060) /* Only if the kernel supports the BLT ring */
		return kgem->has_blt;

	return true;
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
	bool ret;

	if (DBG_NO_CACHE_LEVEL)
		return false;

	/* Incoherent blt and sampler hangs the GPU */
	if (kgem->gen == 040)
		return false;

	handle = gem_create(kgem->fd, 1);
	if (handle == 0)
		return false;

	ret = gem_set_cacheing(kgem->fd, handle, UNCACHED);
	gem_close(kgem->fd, handle);
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


static bool kgem_init_pinned_batches(struct kgem *kgem)
{
	ioctl_t  io;

	int count[2] = { 4, 2 };
	int size[2] = { 1, 4 };
	int n, i;

	if (kgem->wedged)
		return true;

	for (n = 0; n < ARRAY_SIZE(count); n++) {
		for (i = 0; i < count[n]; i++) {
			struct drm_i915_gem_pin pin;
			struct kgem_bo *bo;

			VG_CLEAR(pin);

			pin.handle = gem_create(kgem->fd, size[n]);
			if (pin.handle == 0)
				goto err;

			DBG(("%s: new handle=%d, num_pages=%d\n",
			     __FUNCTION__, pin.handle, size[n]));

			bo = __kgem_bo_alloc(pin.handle, size[n]);
			if (bo == NULL) {
				gem_close(kgem->fd, pin.handle);
				goto err;
			}

			pin.alignment = 0;
			
            io.handle   = kgem->fd;
            io.io_code  = SRV_I915_GEM_PIN;
            io.input    = &pin;
            io.inp_size = sizeof(pin);
            io.output   = NULL;
            io.out_size = 0;

            if (call_service(&io)!=0){
				gem_close(kgem->fd, pin.handle);
				goto err;
			}
			bo->presumed_offset = pin.offset;
			debug_alloc__bo(kgem, bo);
			list_add(&bo->list, &kgem->pinned_batches[n]);
		}
	}

	return true;

err:
	for (n = 0; n < ARRAY_SIZE(kgem->pinned_batches); n++) {
		while (!list_is_empty(&kgem->pinned_batches[n])) {
			kgem_bo_destroy(kgem,
					list_first_entry(&kgem->pinned_batches[n],
							 struct kgem_bo, list));
		}
	}

	/* For simplicity populate the lists with a single unpinned bo */
	for (n = 0; n < ARRAY_SIZE(count); n++) {
		struct kgem_bo *bo;
		uint32_t handle;

		handle = gem_create(kgem->fd, size[n]);
		if (handle == 0)
			break;

		bo = __kgem_bo_alloc(handle, size[n]);
		if (bo == NULL) {
			gem_close(kgem->fd, handle);
			break;
		}

		debug_alloc__bo(kgem, bo);
		list_add(&bo->list, &kgem->pinned_batches[n]);
	}
	return false;
}

void kgem_init(struct kgem *kgem, int fd, struct pci_device *dev, unsigned gen)
{
    struct drm_i915_gem_get_aperture aperture;
    size_t totalram;
    unsigned half_gpu_max;
    unsigned int i, j;
    ioctl_t   io;

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

    if (!is_hw_supported(kgem, dev)) {
        printf("Detected unsupported/dysfunctional hardware, disabling acceleration.\n");
        kgem->wedged = 1;
    } else if (__kgem_throttle(kgem)) {
        printf("Detected a hung GPU, disabling acceleration.\n");
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
        printf("Unable to reserve memory for GPU, disabling acceleration.\n");
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
    
    io.handle   = fd;
    io.io_code  = SRV_I915_GEM_GET_APERTURE;
    io.input    = &aperture;
    io.inp_size = sizeof(aperture);
    io.output   = NULL;
    io.out_size = 0;

    (void)call_service(&io);

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
    DBG(("%s: total ram=%u\n", __FUNCTION__, totalram));
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

}


inline static void kgem_bo_remove_from_inactive(struct kgem *kgem,
						struct kgem_bo *bo)
{
	DBG(("%s: removing handle=%d from inactive\n", __FUNCTION__, bo->handle));

	list_del(&bo->list);
	assert(bo->rq == NULL);
	assert(bo->exec == NULL);
	if (bo->map) {
		assert(!list_is_empty(&bo->vma));
		list_del(&bo->vma);
		kgem->vma[IS_CPU_MAP(bo->map)].count--;
	}
}




static struct kgem_bo *
search_linear_cache(struct kgem *kgem, unsigned int num_pages, unsigned flags)
{
	struct kgem_bo *bo, *first = NULL;
	bool use_active = (flags & CREATE_INACTIVE) == 0;
	struct list *cache;

	DBG(("%s: num_pages=%d, flags=%x, use_active? %d\n",
	     __FUNCTION__, num_pages, flags, use_active));

	if (num_pages >= MAX_CACHE_SIZE / PAGE_SIZE)
		return NULL;

	if (!use_active && list_is_empty(inactive(kgem, num_pages))) {
		DBG(("%s: inactive and cache bucket empty\n",
		     __FUNCTION__));

		if (flags & CREATE_NO_RETIRE) {
			DBG(("%s: can not retire\n", __FUNCTION__));
			return NULL;
		}

		if (list_is_empty(active(kgem, num_pages, I915_TILING_NONE))) {
			DBG(("%s: active cache bucket empty\n", __FUNCTION__));
			return NULL;
		}

		if (!__kgem_throttle_retire(kgem, flags)) {
			DBG(("%s: nothing retired\n", __FUNCTION__));
			return NULL;
		}

		if (list_is_empty(inactive(kgem, num_pages))) {
			DBG(("%s: active cache bucket still empty after retire\n",
			     __FUNCTION__));
			return NULL;
		}
	}

	if (!use_active && flags & (CREATE_CPU_MAP | CREATE_GTT_MAP)) {
		int for_cpu = !!(flags & CREATE_CPU_MAP);
		DBG(("%s: searching for inactive %s map\n",
		     __FUNCTION__, for_cpu ? "cpu" : "gtt"));
		cache = &kgem->vma[for_cpu].inactive[cache_bucket(num_pages)];
		list_for_each_entry(bo, cache, vma) {
			assert(IS_CPU_MAP(bo->map) == for_cpu);
			assert(bucket(bo) == cache_bucket(num_pages));
			assert(bo->proxy == NULL);
			assert(bo->rq == NULL);
			assert(bo->exec == NULL);
			assert(!bo->scanout);

			if (num_pages > num_pages(bo)) {
				DBG(("inactive too small: %d < %d\n",
				     num_pages(bo), num_pages));
				continue;
			}

			if (bo->purged && !kgem_bo_clear_purgeable(kgem, bo)) {
				kgem_bo_free(kgem, bo);
				break;
			}

			if (I915_TILING_NONE != bo->tiling &&
			    !gem_set_tiling(kgem->fd, bo->handle,
					    I915_TILING_NONE, 0))
				continue;

			kgem_bo_remove_from_inactive(kgem, bo);

			bo->tiling = I915_TILING_NONE;
			bo->pitch = 0;
			bo->delta = 0;
			DBG(("  %s: found handle=%d (num_pages=%d) in linear vma cache\n",
			     __FUNCTION__, bo->handle, num_pages(bo)));
			assert(use_active || bo->domain != DOMAIN_GPU);
			assert(!bo->needs_flush);
			ASSERT_MAYBE_IDLE(kgem, bo->handle, !use_active);
			return bo;
		}

		if (flags & CREATE_EXACT)
			return NULL;

		if (flags & CREATE_CPU_MAP && !kgem->has_llc)
			return NULL;
	}

	cache = use_active ? active(kgem, num_pages, I915_TILING_NONE) : inactive(kgem, num_pages);
	list_for_each_entry(bo, cache, list) {
		assert(bo->refcnt == 0);
		assert(bo->reusable);
		assert(!!bo->rq == !!use_active);
		assert(bo->proxy == NULL);
		assert(!bo->scanout);

		if (num_pages > num_pages(bo))
			continue;

		if (use_active &&
		    kgem->gen <= 040 &&
		    bo->tiling != I915_TILING_NONE)
			continue;

		if (bo->purged && !kgem_bo_clear_purgeable(kgem, bo)) {
			kgem_bo_free(kgem, bo);
			break;
		}

		if (I915_TILING_NONE != bo->tiling) {
			if (flags & (CREATE_CPU_MAP | CREATE_GTT_MAP))
				continue;

			if (first)
				continue;

			if (!gem_set_tiling(kgem->fd, bo->handle,
					    I915_TILING_NONE, 0))
				continue;

			bo->tiling = I915_TILING_NONE;
			bo->pitch = 0;
		}

		if (bo->map) {
			if (flags & (CREATE_CPU_MAP | CREATE_GTT_MAP)) {
				int for_cpu = !!(flags & CREATE_CPU_MAP);
				if (IS_CPU_MAP(bo->map) != for_cpu) {
					if (first != NULL)
						break;

					first = bo;
					continue;
				}
			} else {
				if (first != NULL)
					break;

				first = bo;
				continue;
			}
		} else {
			if (flags & (CREATE_CPU_MAP | CREATE_GTT_MAP)) {
				if (first != NULL)
					break;

				first = bo;
				continue;
			}
		}

		if (use_active)
			kgem_bo_remove_from_active(kgem, bo);
		else
			kgem_bo_remove_from_inactive(kgem, bo);

		assert(bo->tiling == I915_TILING_NONE);
		bo->pitch = 0;
		bo->delta = 0;
		DBG(("  %s: found handle=%d (num_pages=%d) in linear %s cache\n",
		     __FUNCTION__, bo->handle, num_pages(bo),
		     use_active ? "active" : "inactive"));
		assert(list_is_empty(&bo->list));
		assert(use_active || bo->domain != DOMAIN_GPU);
		assert(!bo->needs_flush || use_active);
		ASSERT_MAYBE_IDLE(kgem, bo->handle, !use_active);
		return bo;
	}

	if (first) {
		assert(first->tiling == I915_TILING_NONE);

		if (use_active)
			kgem_bo_remove_from_active(kgem, first);
		else
			kgem_bo_remove_from_inactive(kgem, first);

		first->pitch = 0;
		first->delta = 0;
		DBG(("  %s: found handle=%d (near-miss) (num_pages=%d) in linear %s cache\n",
		     __FUNCTION__, first->handle, num_pages(first),
		     use_active ? "active" : "inactive"));
		assert(list_is_empty(&first->list));
		assert(use_active || first->domain != DOMAIN_GPU);
		assert(!first->needs_flush || use_active);
		ASSERT_MAYBE_IDLE(kgem, first->handle, !use_active);
		return first;
	}

	return NULL;
}


struct kgem_bo *kgem_create_linear(struct kgem *kgem, int size, unsigned flags)
{
	struct kgem_bo *bo;
	uint32_t handle;

	DBG(("%s(%d)\n", __FUNCTION__, size));

	if (flags & CREATE_GTT_MAP && kgem->has_llc) {
		flags &= ~CREATE_GTT_MAP;
		flags |= CREATE_CPU_MAP;
	}

	size = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	bo = search_linear_cache(kgem, size, CREATE_INACTIVE | flags);
	if (bo) {
		assert(bo->domain != DOMAIN_GPU);
		ASSERT_IDLE(kgem, bo->handle);
		bo->refcnt = 1;
		return bo;
	}

	if (flags & CREATE_CACHED)
		return NULL;

	handle = gem_create(kgem->fd, size);
	if (handle == 0)
		return NULL;

	DBG(("%s: new handle=%d, num_pages=%d\n", __FUNCTION__, handle, size));
	bo = __kgem_bo_alloc(handle, size);
	if (bo == NULL) {
		gem_close(kgem->fd, handle);
		return NULL;
	}

	debug_alloc__bo(kgem, bo);
	return bo;
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


void _kgem_bo_destroy(struct kgem *kgem, struct kgem_bo *bo)
{


};
