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

#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#endif

#ifdef HAVE_STRUCT_SYSINFO_TOTALRAM
#include <sys/sysinfo.h>
#endif

#include "sna_cpuid.h"

static struct kgem_bo *
search_linear_cache(struct kgem *kgem, unsigned int num_pages, unsigned flags);

static struct kgem_bo *
search_snoop_cache(struct kgem *kgem, unsigned int num_pages, unsigned flags);

#define DBG_NO_HW 0
#define DBG_NO_TILING 0
#define DBG_NO_CACHE 0
#define DBG_NO_CACHE_LEVEL 0
#define DBG_NO_CPU 0
#define DBG_NO_CREATE2 1
#define DBG_NO_USERPTR 1
#define DBG_NO_UNSYNCHRONIZED_USERPTR 0
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
#define DBG_NO_WT 0
#define DBG_DUMP 0

#define FORCE_MMAP_SYNC 0 /* ((1 << DOMAIN_CPU) | (1 << DOMAIN_GTT)) */

#ifndef DEBUG_SYNC
#define DEBUG_SYNC 0
#endif


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

#define MAKE_USER_MAP(ptr) ((void*)((uintptr_t)(ptr) | 1))
#define IS_USER_MAP(ptr) ((uintptr_t)(ptr) & 1)

#define MAKE_REQUEST(rq, ring) ((struct kgem_request *)((uintptr_t)(rq) | (ring)))

#define LOCAL_I915_PARAM_HAS_BLT		        11
#define LOCAL_I915_PARAM_HAS_RELAXED_FENCING	12
#define LOCAL_I915_PARAM_HAS_RELAXED_DELTA	    15
#define LOCAL_I915_PARAM_HAS_SEMAPHORES		    20
#define LOCAL_I915_PARAM_HAS_SECURE_BATCHES	    23
#define LOCAL_I915_PARAM_HAS_PINNED_BATCHES	    24
#define LOCAL_I915_PARAM_HAS_NO_RELOC		    25
#define LOCAL_I915_PARAM_HAS_HANDLE_LUT		    26
#define LOCAL_I915_PARAM_HAS_WT			27

#define LOCAL_I915_EXEC_IS_PINNED		(1<<10)
#define LOCAL_I915_EXEC_NO_RELOC		(1<<11)
#define LOCAL_I915_EXEC_HANDLE_LUT		(1<<12)
struct local_i915_gem_userptr {
	uint64_t user_ptr;
	uint64_t user_size;
	uint32_t flags;
#define I915_USERPTR_READ_ONLY (1<<0)
#define I915_USERPTR_UNSYNCHRONIZED (1<<31)
	uint32_t handle;
};

#define UNCACHED	0
#define SNOOPED		1
#define DISPLAY		2

struct local_i915_gem_caching {
	uint32_t handle;
	uint32_t caching;
};

#define LOCAL_IOCTL_I915_GEM_SET_CACHING SRV_I915_GEM_SET_CACHING

struct local_fbinfo {
	int width;
	int height;
	int pitch;
	int tiling;
};

struct kgem_buffer {
	struct kgem_bo base;
	void *mem;
	uint32_t used;
	uint32_t need_io : 1;
	uint32_t write : 2;
	uint32_t mmapped : 2;
};
enum {
	MMAPPED_NONE,
	MMAPPED_GTT,
	MMAPPED_CPU
};

static struct kgem_bo *__kgem_freed_bo;
static struct kgem_request *__kgem_freed_request;
static struct drm_i915_gem_exec_object2 _kgem_dummy_exec;

static inline int bytes(struct kgem_bo *bo)
{
	return __kgem_bo_size(bo);
}

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

#ifndef NDEBUG
static void assert_tiling(struct kgem *kgem, struct kgem_bo *bo)
{
	struct drm_i915_gem_get_tiling tiling;

	assert(bo);

	VG_CLEAR(tiling);
	tiling.handle = bo->handle;
	tiling.tiling_mode = -1;
	(void)drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_GET_TILING, &tiling);
	assert(tiling.tiling_mode == bo->tiling);
}
#else
#define assert_tiling(kgem, bo)
#endif

static void kgem_sna_reset(struct kgem *kgem)
{
	struct sna *sna = container_of(kgem, struct sna, kgem);

	sna->render.reset(sna);
	sna->blt_state.fill_bo = 0;
}

static void kgem_sna_flush(struct kgem *kgem)
{
	struct sna *sna = container_of(kgem, struct sna, kgem);

	sna->render.flush(sna);

//	if (sna->render.solid_cache.dirty)
//		sna_render_flush_solid(sna);
}

static bool gem_set_tiling(int fd, uint32_t handle, int tiling, int stride)
{
	struct drm_i915_gem_set_tiling set_tiling;
	int ret;

	if (DBG_NO_TILING)
		return false;

	VG_CLEAR(set_tiling);
	do {
		set_tiling.handle = handle;
		set_tiling.tiling_mode = tiling;
		set_tiling.stride = stride;

		ret = drmIoctl(fd, DRM_IOCTL_I915_GEM_SET_TILING, &set_tiling);
	} while (ret != 0);
	return ret == 0;
}

static bool gem_set_caching(int fd, uint32_t handle, int caching)
{
	struct local_i915_gem_caching arg;

	VG_CLEAR(arg);
	arg.handle = handle;
	arg.caching = caching;
	return drmIoctl(fd, LOCAL_IOCTL_I915_GEM_SET_CACHING, &arg) == 0;
}

static uint32_t gem_userptr(int fd, void *ptr, int size, int read_only)
{
    return 0;
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

	if (kgem_retire(kgem))
		return true;

	if (flags & CREATE_NO_THROTTLE || !kgem->need_throttle) {
		DBG(("%s: not throttling\n", __FUNCTION__));
		return false;
	}

	kgem_throttle(kgem);
	return kgem_retire(kgem);
}

static void *__kgem_bo_map__gtt(struct kgem *kgem, struct kgem_bo *bo)
{
	struct drm_i915_gem_mmap_gtt mmap_arg;
	void *ptr;

	DBG(("%s(handle=%d, size=%d)\n", __FUNCTION__,
	     bo->handle, bytes(bo)));
	assert(bo->proxy == NULL);
	assert(!bo->snoop);
	assert(num_pages(bo) <= kgem->aperture_mappable / 4);

retry_gtt:
	VG_CLEAR(mmap_arg);
	mmap_arg.handle = bo->handle;
	if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_MMAP_GTT, &mmap_arg)) {
		int err = 0;

		(void)__kgem_throttle_retire(kgem, 0);
		if (kgem_expire_cache(kgem))
			goto retry_gtt;

		if (kgem_cleanup_cache(kgem))
			goto retry_gtt;

		ErrorF("%s: failed to retrieve GTT offset for handle=%d: %d\n",
		       __FUNCTION__, bo->handle, err);
		return NULL;
	}

retry_mmap:
	ptr = (void*)(int)mmap_arg.offset;
	if (ptr == NULL) {
		ErrorF("%s: failed to mmap handle=%d, %d bytes, into GTT domain\n",
		       __FUNCTION__, bo->handle, bytes(bo));
		ptr = NULL;
	}

	return ptr;
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
	pwrite.offset = offset;
	pwrite.size = length;
	pwrite.data_ptr = (uintptr_t)src;
	return drmIoctl(fd, DRM_IOCTL_I915_GEM_PWRITE, &pwrite);
}

static int gem_write__cachealigned(int fd, uint32_t handle,
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
	return drmIoctl(fd, DRM_IOCTL_I915_GEM_PWRITE, &pwrite);
}


bool __kgem_busy(struct kgem *kgem, int handle)
{
	struct drm_i915_gem_busy busy;

	VG_CLEAR(busy);
	busy.handle = handle;
	busy.busy = !kgem->wedged;
	(void)drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_BUSY, &busy);
	DBG(("%s: handle=%d, busy=%d, wedged=%d\n",
	     __FUNCTION__, handle, busy.busy, kgem->wedged));

	return busy.busy;
}

static void kgem_bo_retire(struct kgem *kgem, struct kgem_bo *bo)
{
	DBG(("%s: retiring bo handle=%d (needed flush? %d), rq? %d [busy?=%d]\n",
	     __FUNCTION__, bo->handle, bo->needs_flush, bo->rq != NULL,
	     __kgem_busy(kgem, bo->handle)));
	assert(bo->exec == NULL);
	assert(list_is_empty(&bo->vma));

	if (bo->rq) {
		if (!__kgem_busy(kgem, bo->handle)) {
			__kgem_bo_clear_busy(bo);
			kgem_retire(kgem);
		}
	} else {
		assert(!bo->needs_flush);
		ASSERT_IDLE(kgem, bo->handle);
	}
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
		kgem_bo_retire(kgem, bo);
		bo->domain = DOMAIN_NONE;
	}
	bo->gtt_dirty = true;
	return true;
}

static uint32_t gem_create(int fd, int num_pages)
{
	struct drm_i915_gem_create create;

	VG_CLEAR(create);
	create.handle = 0;
	create.size = PAGE_SIZE * num_pages;
	(void)drmIoctl(fd, DRM_IOCTL_I915_GEM_CREATE, &create);

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

	VG_CLEAR(close);
	close.handle = handle;
	(void)drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &close);
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

static unsigned
cpu_cache_size__cpuid4(void)
{
	/* Deterministic Cache Parameters (Function 04h)":
	 *    When EAX is initialized to a value of 4, the CPUID instruction
	 *    returns deterministic cache information in the EAX, EBX, ECX
	 *    and EDX registers.  This function requires ECX be initialized
	 *    with an index which indicates which cache to return information
	 *    about. The OS is expected to call this function (CPUID.4) with
	 *    ECX = 0, 1, 2, until EAX[4:0] == 0, indicating no more caches.
	 *    The order in which the caches are returned is not specified
	 *    and may change at Intel's discretion.
	 *
	 * Calculating the Cache Size in bytes:
	 *          = (Ways +1) * (Partitions +1) * (Line Size +1) * (Sets +1)
	 */

	 unsigned int eax, ebx, ecx, edx;
	 unsigned int llc_size = 0;
	 int cnt = 0;

	 if (__get_cpuid_max(BASIC_CPUID, NULL) < 4)
		 return 0;

	 do {
		 unsigned associativity, line_partitions, line_size, sets;

		 __cpuid_count(4, cnt++, eax, ebx, ecx, edx);

		 if ((eax & 0x1f) == 0)
			 break;

		 associativity = ((ebx >> 22) & 0x3ff) + 1;
		 line_partitions = ((ebx >> 12) & 0x3ff) + 1;
		 line_size = (ebx & 0xfff) + 1;
		 sets = ecx + 1;

		 llc_size = associativity * line_partitions * line_size * sets;
	 } while (1);

	 return llc_size;
}

static int gem_param(struct kgem *kgem, int name)
{
    drm_i915_getparam_t gp;
    int v = -1; /* No param uses the sign bit, reserve it for errors */

    VG_CLEAR(gp);
    gp.param = name;
    gp.value = &v;
	if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GETPARAM, &gp))
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

static bool test_has_wt(struct kgem *kgem)
{
	if (DBG_NO_WT)
		return false;

	return gem_param(kgem, LOCAL_I915_PARAM_HAS_WT) > 0;
}

static bool test_has_semaphores_enabled(struct kgem *kgem)
{
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
	if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_THROTTLE, NULL) == 0)
		return false;

	return errno == EIO;
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

	if (kgem->gen == 060 && dev && dev->revision < 8) {
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

static bool test_has_caching(struct kgem *kgem)
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

	ret = gem_set_caching(kgem->fd, handle, UNCACHED);
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

	if (posix_memalign(&ptr, PAGE_SIZE, PAGE_SIZE))
		return false;

	handle = gem_userptr(kgem->fd, ptr, PAGE_SIZE, false);
	gem_close(kgem->fd, handle);
	free(ptr);

	return handle != 0;
#else
	return false;
#endif
}

static bool test_has_create2(struct kgem *kgem)
{
#if defined(USE_CREATE2)
	struct local_i915_gem_create2 args;

	if (DBG_NO_CREATE2)
		return false;

	memset(&args, 0, sizeof(args));
	args.size = PAGE_SIZE;
	args.caching = DISPLAY;
	if (drmIoctl(kgem->fd, LOCAL_IOCTL_I915_GEM_CREATE2, &args) == 0)
		gem_close(kgem->fd, args.handle);

	return args.handle != 0;
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
	int count[2] = { 4, 4 };
	int size[2] = { 1, 2 };
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
			if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_PIN, &pin)) {
				gem_close(kgem->fd, pin.handle);
				free(bo);
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

	kgem->has_wt = test_has_wt(kgem);
	DBG(("%s: has write-through caching for scanouts? %d\n", __FUNCTION__,
	     kgem->has_wt));

	kgem->has_caching = test_has_caching(kgem);
    DBG(("%s: has set-cache-level? %d\n", __FUNCTION__,
	     kgem->has_caching));

    kgem->has_userptr = test_has_userptr(kgem);
    DBG(("%s: has userptr? %d\n", __FUNCTION__,
         kgem->has_userptr));

	kgem->has_create2 = test_has_create2(kgem);
	kgem->has_create2 = 0;
	DBG(("%s: has create2? %d\n", __FUNCTION__,
	     kgem->has_create2));

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

	kgem->can_render_y = gen != 021 && (gen >> 3) != 4;
	DBG(("%s: can render to Y-tiled surfaces? %d\n", __FUNCTION__,
	     kgem->can_render_y));

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

	kgem->min_alignment = 16;
    if (gen < 040)
        kgem->min_alignment = 64;

    kgem->half_cpu_cache_pages = cpu_cache_size() >> 13;
	DBG(("%s: last-level cache size: %d bytes, threshold in pages: %d\n",
	     __FUNCTION__, cpu_cache_size(), kgem->half_cpu_cache_pages));

    kgem->next_request = __kgem_request_alloc(kgem);

    DBG(("%s: cpu bo enabled %d: llc? %d, set-cache-level? %d, userptr? %d\n", __FUNCTION__,
	     !DBG_NO_CPU && (kgem->has_llc | kgem->has_userptr | kgem->has_caching),
	     kgem->has_llc, kgem->has_caching, kgem->has_userptr));

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

	kgem->aperture_mappable = 256 * 1024 * 1024;
	if (dev != NULL)
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
	kgem->buffer_size = 1 << __fls(kgem->buffer_size);
    DBG(("%s: buffer size=%d [%d KiB]\n", __FUNCTION__,
         kgem->buffer_size, kgem->buffer_size / 1024));
	assert(kgem->buffer_size);

    kgem->max_object_size = 3 * (kgem->aperture_high >> 12) << 10;
    kgem->max_gpu_size = kgem->max_object_size;
	if (!kgem->has_llc && kgem->max_gpu_size > MAX_CACHE_SIZE)
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

	if (kgem->aperture_high > totalram / 2) {
		kgem->aperture_high = totalram / 2;
		kgem->aperture_low = kgem->aperture_high / 4;
		DBG(("%s: reduced aperture watermaks to fit into ram; low=%d [%d], high=%d [%d]\n", __FUNCTION__,
		     kgem->aperture_low, kgem->aperture_low / (1024*1024),
		     kgem->aperture_high, kgem->aperture_high / (1024*1024)));
	}

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
	if (kgem->max_upload_tile_size > kgem->aperture_high/2)
		kgem->max_upload_tile_size = kgem->aperture_high/2;
	if (kgem->max_upload_tile_size > kgem->aperture_low)
		kgem->max_upload_tile_size = kgem->aperture_low;
	if (kgem->max_upload_tile_size < 16*PAGE_SIZE)
		kgem->max_upload_tile_size = 16*PAGE_SIZE;

    kgem->large_object_size = MAX_CACHE_SIZE;
	if (kgem->large_object_size > half_gpu_max)
		kgem->large_object_size = half_gpu_max;
	if (kgem->max_copy_tile_size > kgem->aperture_high/2)
		kgem->max_copy_tile_size = kgem->aperture_high/2;
	if (kgem->max_copy_tile_size > kgem->aperture_low)
		kgem->max_copy_tile_size = kgem->aperture_low;
	if (kgem->max_copy_tile_size < 16*PAGE_SIZE)
		kgem->max_copy_tile_size = 16*PAGE_SIZE;

	if (kgem->has_llc | kgem->has_caching | kgem->has_userptr) {
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
	kgem->aperture_mappable /= PAGE_SIZE;
    kgem->aperture_low /= PAGE_SIZE;
    kgem->aperture_high /= PAGE_SIZE;
	kgem->aperture_total /= PAGE_SIZE;

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

/* XXX hopefully a good approximation */
uint32_t kgem_get_unique_id(struct kgem *kgem)
{
	uint32_t id;
	id = ++kgem->unique_id;
	if (id == 0)
		id = ++kgem->unique_id;
	return id;
}

inline static uint32_t kgem_pitch_alignment(struct kgem *kgem, unsigned flags)
{
	if (flags & CREATE_PRIME)
		return 256;
	if (flags & CREATE_SCANOUT)
		return 64;
	return kgem->min_alignment;
}

void kgem_get_tile_size(struct kgem *kgem, int tiling, int pitch,
			int *tile_width, int *tile_height, int *tile_size)
{
	if (kgem->gen <= 030) {
		if (tiling) {
			if (kgem->gen < 030) {
				*tile_width = 128;
				*tile_height = 16;
				*tile_size = 2048;
			} else {
				*tile_width = 512;
				*tile_height = 8;
				*tile_size = 4096;
			}
		} else {
			*tile_width = 1;
			*tile_height = 1;
			*tile_size = 1;
		}
	} else switch (tiling) {
	default:
	case I915_TILING_NONE:
		*tile_width = 1;
		*tile_height = 1;
		*tile_size = 1;
		break;
	case I915_TILING_X:
		*tile_width = 512;
		*tile_height = 8;
		*tile_size = 4096;
		break;
	case I915_TILING_Y:
		*tile_width = 128;
		*tile_height = 32;
		*tile_size = 4096;
		break;
	}

	/* Force offset alignment to tile-row */
	if (tiling && kgem->gen < 033)
		*tile_width = pitch;
}

uint32_t kgem_surface_size(struct kgem *kgem,
				  bool relaxed_fencing,
				  unsigned flags,
				  uint32_t width,
				  uint32_t height,
				  uint32_t bpp,
				  uint32_t tiling,
				  uint32_t *pitch)
{
	uint32_t tile_width, tile_height;
	uint32_t size;

	assert(width <= MAXSHORT);
	assert(height <= MAXSHORT);
	assert(bpp >= 8);

	if (kgem->gen <= 030) {
		if (tiling) {
			if (kgem->gen < 030) {
				tile_width = 128;
				tile_height = 32;
			} else {
				tile_width = 512;
				tile_height = 16;
			}
		} else {
			tile_width = 2 * bpp >> 3;
			tile_width = ALIGN(tile_width,
					   kgem_pitch_alignment(kgem, flags));
			tile_height = 2;
		}
	} else switch (tiling) {
	default:
	case I915_TILING_NONE:
		tile_width = 2 * bpp >> 3;
		tile_width = ALIGN(tile_width,
				   kgem_pitch_alignment(kgem, flags));
		tile_height = 2;
		break;

		/* XXX align to an even tile row */
	case I915_TILING_X:
		tile_width = 512;
		tile_height = 16;
		break;
	case I915_TILING_Y:
		tile_width = 128;
		tile_height = 64;
		break;
	}

	*pitch = ALIGN(width * bpp / 8, tile_width);
	height = ALIGN(height, tile_height);
	if (kgem->gen >= 040)
		return PAGE_ALIGN(*pitch * height);

	/* If it is too wide for the blitter, don't even bother.  */
	if (tiling != I915_TILING_NONE) {
		if (*pitch > 8192)
			return 0;

		for (size = tile_width; size < *pitch; size <<= 1)
			;
		*pitch = size;
	} else {
		if (*pitch >= 32768)
			return 0;
	}

	size = *pitch * height;
	if (relaxed_fencing || tiling == I915_TILING_NONE)
		return PAGE_ALIGN(size);

	/*  We need to allocate a pot fence region for a tiled buffer. */
	if (kgem->gen < 030)
		tile_width = 512 * 1024;
	else
		tile_width = 1024 * 1024;
	while (tile_width < size)
		tile_width *= 2;
	return tile_width;
}

static uint32_t kgem_aligned_height(struct kgem *kgem,
				    uint32_t height, uint32_t tiling)
{
	uint32_t tile_height;

	if (kgem->gen <= 030) {
		tile_height = tiling ? kgem->gen < 030 ? 32 : 16 : 1;
	} else switch (tiling) {
		/* XXX align to an even tile row */
	default:
	case I915_TILING_NONE:
		tile_height = 1;
		break;
	case I915_TILING_X:
		tile_height = 16;
		break;
	case I915_TILING_Y:
		tile_height = 64;
		break;
	}

	return ALIGN(height, tile_height);
}

static struct drm_i915_gem_exec_object2 *
kgem_add_handle(struct kgem *kgem, struct kgem_bo *bo)
{
	struct drm_i915_gem_exec_object2 *exec;

	DBG(("%s: handle=%d, index=%d\n",
	     __FUNCTION__, bo->handle, kgem->nexec));

	assert(kgem->nexec < ARRAY_SIZE(kgem->exec));
	bo->target_handle = kgem->has_handle_lut ? kgem->nexec : bo->handle;
	exec = memset(&kgem->exec[kgem->nexec++], 0, sizeof(*exec));
	exec->handle = bo->handle;
	exec->offset = bo->presumed_offset;

	kgem->aperture += num_pages(bo);

	return exec;
}

static void kgem_add_bo(struct kgem *kgem, struct kgem_bo *bo)
{
	assert(bo->refcnt);
	assert(bo->proxy == NULL);

	bo->exec = kgem_add_handle(kgem, bo);
	bo->rq = MAKE_REQUEST(kgem->next_request, kgem->ring);

	list_move_tail(&bo->request, &kgem->next_request->buffers);
	if (bo->io && !list_is_empty(&bo->list))
		list_move(&bo->list, &kgem->batch_buffers);

	/* XXX is it worth working around gcc here? */
	kgem->flush |= bo->flush;
}

static uint32_t kgem_end_batch(struct kgem *kgem)
{
	kgem->batch[kgem->nbatch++] = MI_BATCH_BUFFER_END;
	if (kgem->nbatch & 1)
		kgem->batch[kgem->nbatch++] = MI_NOOP;

	return kgem->nbatch;
}

static void kgem_fixup_self_relocs(struct kgem *kgem, struct kgem_bo *bo)
{
	int n;

	assert(kgem->nreloc__self <= 256);
	if (kgem->nreloc__self == 0)
		return;

	for (n = 0; n < kgem->nreloc__self; n++) {
		int i = kgem->reloc__self[n];
		assert(kgem->reloc[i].target_handle == ~0U);
		kgem->reloc[i].target_handle = bo->target_handle;
		kgem->reloc[i].presumed_offset = bo->presumed_offset;
		kgem->batch[kgem->reloc[i].offset/sizeof(kgem->batch[0])] =
			kgem->reloc[i].delta + bo->presumed_offset;
	}

	if (n == 256) {
		for (n = kgem->reloc__self[255]; n < kgem->nreloc; n++) {
			if (kgem->reloc[n].target_handle == ~0U) {
				kgem->reloc[n].target_handle = bo->target_handle;
				kgem->reloc[n].presumed_offset = bo->presumed_offset;
				kgem->batch[kgem->reloc[n].offset/sizeof(kgem->batch[0])] =
					kgem->reloc[n].delta + bo->presumed_offset;
			}
		}

	}

}

static void kgem_bo_binding_free(struct kgem *kgem, struct kgem_bo *bo)
{
	struct kgem_bo_binding *b;

	b = bo->binding.next;
	while (b) {
		struct kgem_bo_binding *next = b->next;
		free(b);
		b = next;
	}
}

static void kgem_bo_free(struct kgem *kgem, struct kgem_bo *bo)
{
	DBG(("%s: handle=%d\n", __FUNCTION__, bo->handle));
	assert(bo->refcnt == 0);
	assert(bo->proxy == NULL);
	assert(bo->exec == NULL);
	assert(!bo->snoop || bo->rq == NULL);

#ifdef DEBUG_MEMORY
	kgem->debug_memory.bo_allocs--;
	kgem->debug_memory.bo_bytes -= bytes(bo);
#endif

	kgem_bo_binding_free(kgem, bo);

	if (IS_USER_MAP(bo->map__cpu)) {
		assert(bo->rq == NULL);
		assert(!__kgem_busy(kgem, bo->handle));
		assert(MAP(bo->map__cpu) != bo || bo->io || bo->flush);
		if (!(bo->io || bo->flush)) {
			DBG(("%s: freeing snooped base\n", __FUNCTION__));
			assert(bo != MAP(bo->map__cpu));
			free(MAP(bo->map__cpu));
		}
		bo->map__cpu = NULL;
	}

	DBG(("%s: releasing %p:%p vma for handle=%d, count=%d\n",
	     __FUNCTION__, bo->map__gtt, bo->map__cpu,
	     bo->handle, list_is_empty(&bo->vma) ? 0 : kgem->vma[bo->map__gtt == NULL].count));

	if (!list_is_empty(&bo->vma)) {
		_list_del(&bo->vma);
		kgem->vma[bo->map__gtt == NULL].count--;
	}

//   if (bo->map__gtt)
//       munmap(MAP(bo->map__gtt), bytes(bo));
//   if (bo->map__cpu)
//       munmap(MAP(bo->map__cpu), bytes(bo));

	_list_del(&bo->list);
	_list_del(&bo->request);
	gem_close(kgem->fd, bo->handle);

	if (!bo->io) {
		*(struct kgem_bo **)bo = __kgem_freed_bo;
		__kgem_freed_bo = bo;
	} else
		free(bo);
}

inline static void kgem_bo_move_to_inactive(struct kgem *kgem,
					    struct kgem_bo *bo)
{
	DBG(("%s: moving handle=%d to inactive\n", __FUNCTION__, bo->handle));

	assert(bo->refcnt == 0);
	assert(bo->reusable);
	assert(bo->rq == NULL);
	assert(bo->exec == NULL);
	assert(bo->domain != DOMAIN_GPU);
	assert(!bo->proxy);
	assert(!bo->io);
	assert(!bo->scanout);
	assert(!bo->snoop);
	assert(!bo->flush);
	assert(!bo->needs_flush);
	assert(list_is_empty(&bo->vma));
	assert_tiling(kgem, bo);
	ASSERT_IDLE(kgem, bo->handle);

	kgem->need_expire = true;

	if (bucket(bo) >= NUM_CACHE_BUCKETS) {
		if (bo->map__gtt) {
//           munmap(MAP(bo->map__gtt), bytes(bo));
			bo->map__gtt = NULL;
	}

		list_move(&bo->list, &kgem->large_inactive);
	} else {
	assert(bo->flush == false);
	list_move(&bo->list, &kgem->inactive[bucket(bo)]);
		if (bo->map__gtt) {
			if (!kgem_bo_can_map(kgem, bo)) {
//				munmap(MAP(bo->map__gtt), bytes(bo));
				bo->map__gtt = NULL;
			}
			if (bo->map__gtt) {
				list_add(&bo->vma, &kgem->vma[0].inactive[bucket(bo)]);
				kgem->vma[0].count++;
			}
		}
		if (bo->map__cpu && !bo->map__gtt) {
			list_add(&bo->vma, &kgem->vma[1].inactive[bucket(bo)]);
			kgem->vma[1].count++;
		}
	}
}

static struct kgem_bo *kgem_bo_replace_io(struct kgem_bo *bo)
{
	struct kgem_bo *base;

	if (!bo->io)
		return bo;

	assert(!bo->snoop);
	if (__kgem_freed_bo) {
		base = __kgem_freed_bo;
		__kgem_freed_bo = *(struct kgem_bo **)base;
	} else
	base = malloc(sizeof(*base));
	if (base) {
		DBG(("%s: transferring io handle=%d to bo\n",
		     __FUNCTION__, bo->handle));
		/* transfer the handle to a minimum bo */
		memcpy(base, bo, sizeof(*base));
		base->io = false;
		list_init(&base->list);
		list_replace(&bo->request, &base->request);
		list_replace(&bo->vma, &base->vma);
		free(bo);
		bo = base;
	} else
		bo->reusable = false;

	return bo;
}

inline static void kgem_bo_remove_from_inactive(struct kgem *kgem,
						struct kgem_bo *bo)
{
	DBG(("%s: removing handle=%d from inactive\n", __FUNCTION__, bo->handle));

	list_del(&bo->list);
	assert(bo->rq == NULL);
	assert(bo->exec == NULL);
	if (!list_is_empty(&bo->vma)) {
		assert(bo->map__gtt || bo->map__cpu);
		list_del(&bo->vma);
		kgem->vma[bo->map__gtt == NULL].count--;
	}
}

inline static void kgem_bo_remove_from_active(struct kgem *kgem,
					      struct kgem_bo *bo)
{
	DBG(("%s: removing handle=%d from active\n", __FUNCTION__, bo->handle));

	list_del(&bo->list);
	assert(bo->rq != NULL);
	if (RQ(bo->rq) == (void *)kgem) {
		assert(bo->exec == NULL);
		list_del(&bo->request);
	}
	assert(list_is_empty(&bo->vma));
}

static void _kgem_bo_delete_buffer(struct kgem *kgem, struct kgem_bo *bo)
{
	struct kgem_buffer *io = (struct kgem_buffer *)bo->proxy;

	DBG(("%s: size=%d, offset=%d, parent used=%d\n",
	     __FUNCTION__, bo->size.bytes, bo->delta, io->used));

	if (ALIGN(bo->delta + bo->size.bytes, UPLOAD_ALIGNMENT) == io->used)
		io->used = bo->delta;
}

static void kgem_bo_move_to_scanout(struct kgem *kgem, struct kgem_bo *bo)
{
	assert(bo->refcnt == 0);
	assert(bo->scanout);
	assert(bo->delta);
	assert(!bo->flush);
	assert(!bo->snoop);
	assert(!bo->io);

	if (bo->purged) {
		DBG(("%s: discarding purged scanout - external name?\n",
		     __FUNCTION__));
		kgem_bo_free(kgem, bo);
		return;
	}

	DBG(("%s: moving %d [fb %d] to scanout cache, active? %d\n",
	     __FUNCTION__, bo->handle, bo->delta, bo->rq != NULL));
	if (bo->rq)
		list_move_tail(&bo->list, &kgem->scanout);
	else
	list_move(&bo->list, &kgem->scanout);
}

static void kgem_bo_move_to_snoop(struct kgem *kgem, struct kgem_bo *bo)
{
	assert(bo->reusable);
	assert(!bo->flush);
	assert(!bo->needs_flush);
	assert(bo->refcnt == 0);
	assert(bo->exec == NULL);

	if (num_pages(bo) > kgem->max_cpu_size >> 13) {
		DBG(("%s handle=%d discarding large CPU buffer (%d >%d pages)\n",
		     __FUNCTION__, bo->handle, num_pages(bo), kgem->max_cpu_size >> 13));
		kgem_bo_free(kgem, bo);
		return;
	}

	assert(bo->tiling == I915_TILING_NONE);
	assert(bo->rq == NULL);

	DBG(("%s: moving %d to snoop cachee\n", __FUNCTION__, bo->handle));
	list_add(&bo->list, &kgem->snoop);
}

static struct kgem_bo *
search_snoop_cache(struct kgem *kgem, unsigned int num_pages, unsigned flags)
{
	struct kgem_bo *bo, *first = NULL;

	DBG(("%s: num_pages=%d, flags=%x\n", __FUNCTION__, num_pages, flags));

	if ((kgem->has_caching | kgem->has_userptr) == 0)
		return NULL;

	if (list_is_empty(&kgem->snoop)) {
		DBG(("%s: inactive and cache empty\n", __FUNCTION__));
		if (!__kgem_throttle_retire(kgem, flags)) {
			DBG(("%s: nothing retired\n", __FUNCTION__));
			return NULL;
		}
	}

	list_for_each_entry(bo, &kgem->snoop, list) {
		assert(bo->refcnt == 0);
		assert(bo->snoop);
		assert(!bo->scanout);
		assert(!bo->purged);
		assert(bo->proxy == NULL);
		assert(bo->tiling == I915_TILING_NONE);
		assert(bo->rq == NULL);
		assert(bo->exec == NULL);

		if (num_pages > num_pages(bo))
			continue;

		if (num_pages(bo) > 2*num_pages) {
			if (first == NULL)
				first = bo;
			continue;
		}

		list_del(&bo->list);
		bo->pitch = 0;
		bo->delta = 0;

		DBG(("  %s: found handle=%d (num_pages=%d) in snoop cache\n",
		     __FUNCTION__, bo->handle, num_pages(bo)));
		return bo;
	}

	if (first) {
		list_del(&first->list);
		first->pitch = 0;
		first->delta = 0;

		DBG(("  %s: found handle=%d (num_pages=%d) in snoop cache\n",
		     __FUNCTION__, first->handle, num_pages(first)));
		return first;
	}

	return NULL;
}

void kgem_bo_undo(struct kgem *kgem, struct kgem_bo *bo)
{
	if (kgem->nexec != 1 || bo->exec == NULL)
		return;

	assert(bo);
	DBG(("%s: only handle in batch, discarding last operations for handle=%d\n",
	     __FUNCTION__, bo->handle));

	assert(bo->exec == &kgem->exec[0]);
	assert(kgem->exec[0].handle == bo->handle);
	assert(RQ(bo->rq) == kgem->next_request);

	bo->refcnt++;
	kgem_reset(kgem);
	bo->refcnt--;

	assert(kgem->nreloc == 0);
	assert(kgem->nexec == 0);
	assert(bo->exec == NULL);
}

static void __kgem_bo_destroy(struct kgem *kgem, struct kgem_bo *bo)
{
	DBG(("%s: handle=%d\n", __FUNCTION__, bo->handle));

	assert(list_is_empty(&bo->list));
	assert(bo->refcnt == 0);
	assert(!bo->purged || !bo->reusable);
	assert(bo->proxy == NULL);
	assert_tiling(kgem, bo);

	bo->binding.offset = 0;

	if (DBG_NO_CACHE)
		goto destroy;

	if (bo->snoop && !bo->flush) {
		DBG(("%s: handle=%d is snooped\n", __FUNCTION__, bo->handle));
		assert(bo->reusable);
		assert(list_is_empty(&bo->list));
		if (bo->exec == NULL && bo->rq && !__kgem_busy(kgem, bo->handle))
			__kgem_bo_clear_busy(bo);
		if (bo->rq == NULL)
			kgem_bo_move_to_snoop(kgem, bo);
		return;
	}
	if (!IS_USER_MAP(bo->map__cpu))
		bo->flush = false;

	if (bo->scanout) {
		kgem_bo_move_to_scanout(kgem, bo);
		return;
	}

	if (bo->io)
		bo = kgem_bo_replace_io(bo);
	if (!bo->reusable) {
		DBG(("%s: handle=%d, not reusable\n",
		     __FUNCTION__, bo->handle));
		goto destroy;
	}

	assert(list_is_empty(&bo->vma));
	assert(list_is_empty(&bo->list));
	assert(bo->flush == false);
	assert(bo->snoop == false);
	assert(bo->io == false);
	assert(bo->scanout == false);

	kgem_bo_undo(kgem, bo);
	assert(bo->refcnt == 0);

	if (bo->rq && bo->exec == NULL && !__kgem_busy(kgem, bo->handle))
		__kgem_bo_clear_busy(bo);

	if (bo->rq) {
		struct list *cache;

		DBG(("%s: handle=%d -> active\n", __FUNCTION__, bo->handle));
		if (bucket(bo) < NUM_CACHE_BUCKETS)
			cache = &kgem->active[bucket(bo)][bo->tiling];
		else
			cache = &kgem->large;
		list_add(&bo->list, cache);
		return;
	}

	assert(bo->exec == NULL);
	assert(list_is_empty(&bo->request));

	if (bo->map__cpu == NULL || bucket(bo) >= NUM_CACHE_BUCKETS) {
		if (!kgem_bo_set_purgeable(kgem, bo))
			goto destroy;

		if (!kgem->has_llc && bo->domain == DOMAIN_CPU)
			goto destroy;

		DBG(("%s: handle=%d, purged\n",
		     __FUNCTION__, bo->handle));
	}

	kgem_bo_move_to_inactive(kgem, bo);
	return;

destroy:
	if (!bo->exec)
		kgem_bo_free(kgem, bo);
}

static void kgem_bo_unref(struct kgem *kgem, struct kgem_bo *bo)
{
	assert(bo->refcnt);
	if (--bo->refcnt == 0)
		__kgem_bo_destroy(kgem, bo);
}

static void kgem_buffer_release(struct kgem *kgem, struct kgem_buffer *bo)
{
	assert(bo->base.io);
	while (!list_is_empty(&bo->base.vma)) {
		struct kgem_bo *cached;

		cached = list_first_entry(&bo->base.vma, struct kgem_bo, vma);
		assert(cached->proxy == &bo->base);
		assert(cached != &bo->base);
		list_del(&cached->vma);

		assert(*(struct kgem_bo **)cached->map__gtt == cached);
		*(struct kgem_bo **)cached->map__gtt = NULL;
		cached->map__gtt = NULL;

		kgem_bo_destroy(kgem, cached);
	}
}

static bool kgem_retire__buffers(struct kgem *kgem)
{
	bool retired = false;

	while (!list_is_empty(&kgem->active_buffers)) {
		struct kgem_buffer *bo =
			list_last_entry(&kgem->active_buffers,
					struct kgem_buffer,
					base.list);

		DBG(("%s: handle=%d, busy? %d [%d]\n",
		     __FUNCTION__, bo->base.handle, bo->base.rq != NULL, bo->base.exec != NULL));

		assert(bo->base.exec == NULL || RQ(bo->base.rq) == kgem->next_request);
		if (bo->base.rq)
			break;

		DBG(("%s: releasing upload cache for handle=%d? %d\n",
		     __FUNCTION__, bo->base.handle, !list_is_empty(&bo->base.vma)));
		list_del(&bo->base.list);
		kgem_buffer_release(kgem, bo);
		kgem_bo_unref(kgem, &bo->base);
		retired = true;
	}

	return retired;
}

static bool kgem_retire__flushing(struct kgem *kgem)
{
	struct kgem_bo *bo, *next;
	bool retired = false;

	list_for_each_entry_safe(bo, next, &kgem->flushing, request) {
		assert(RQ(bo->rq) == (void *)kgem);
		assert(bo->exec == NULL);

		if (__kgem_busy(kgem, bo->handle))
			break;

		__kgem_bo_clear_busy(bo);

		if (bo->refcnt)
			continue;

		if (bo->snoop) {
			kgem_bo_move_to_snoop(kgem, bo);
		} else if (bo->scanout) {
			kgem_bo_move_to_scanout(kgem, bo);
		} else if ((bo = kgem_bo_replace_io(bo))->reusable &&
			   kgem_bo_set_purgeable(kgem, bo)) {
			kgem_bo_move_to_inactive(kgem, bo);
			retired = true;
		} else
			kgem_bo_free(kgem, bo);
	}
#if HAS_DEBUG_FULL
	{
		int count = 0;
		list_for_each_entry(bo, &kgem->flushing, request)
			count++;
		ErrorF("%s: %d bo on flushing list\n", __FUNCTION__, count);
	}
#endif

	kgem->need_retire |= !list_is_empty(&kgem->flushing);

	return retired;
}


static bool __kgem_retire_rq(struct kgem *kgem, struct kgem_request *rq)
{
	bool retired = false;

	DBG(("%s: request %d complete\n",
	     __FUNCTION__, rq->bo->handle));

	while (!list_is_empty(&rq->buffers)) {
		struct kgem_bo *bo;

		bo = list_first_entry(&rq->buffers,
				      struct kgem_bo,
				      request);

		assert(RQ(bo->rq) == rq);
		assert(bo->exec == NULL);
		assert(bo->domain == DOMAIN_GPU || bo->domain == DOMAIN_NONE);

		list_del(&bo->request);

		if (bo->needs_flush)
			bo->needs_flush = __kgem_busy(kgem, bo->handle);
		if (bo->needs_flush) {
			DBG(("%s: moving %d to flushing\n",
			     __FUNCTION__, bo->handle));
			list_add(&bo->request, &kgem->flushing);
			bo->rq = MAKE_REQUEST(kgem, RQ_RING(bo->rq));
			kgem->need_retire = true;
			continue;
		}

		bo->domain = DOMAIN_NONE;
		bo->rq = NULL;
		if (bo->refcnt)
			continue;

		if (bo->snoop) {
			kgem_bo_move_to_snoop(kgem, bo);
		} else if (bo->scanout) {
			kgem_bo_move_to_scanout(kgem, bo);
		} else if ((bo = kgem_bo_replace_io(bo))->reusable &&
			   kgem_bo_set_purgeable(kgem, bo)) {
			kgem_bo_move_to_inactive(kgem, bo);
			retired = true;
		} else {
			DBG(("%s: closing %d\n",
			     __FUNCTION__, bo->handle));
			kgem_bo_free(kgem, bo);
		}
	}

	assert(rq->bo->rq == NULL);
	assert(rq->bo->exec == NULL);
	assert(list_is_empty(&rq->bo->request));

	if (--rq->bo->refcnt == 0) {
		if (kgem_bo_set_purgeable(kgem, rq->bo)) {
			kgem_bo_move_to_inactive(kgem, rq->bo);
			retired = true;
		} else {
			DBG(("%s: closing %d\n",
			     __FUNCTION__, rq->bo->handle));
			kgem_bo_free(kgem, rq->bo);
		}
	}

	__kgem_request_free(rq);
	return retired;
}

static bool kgem_retire__requests_ring(struct kgem *kgem, int ring)
{
	bool retired = false;

	while (!list_is_empty(&kgem->requests[ring])) {
		struct kgem_request *rq;

		rq = list_first_entry(&kgem->requests[ring],
				      struct kgem_request,
				      list);
		if (__kgem_busy(kgem, rq->bo->handle))
			break;

		retired |= __kgem_retire_rq(kgem, rq);
	}

#if HAS_DEBUG_FULL
	{
		struct kgem_bo *bo;
		int count = 0;

		list_for_each_entry(bo, &kgem->requests[ring], request)
			count++;

		bo = NULL;
		if (!list_is_empty(&kgem->requests[ring]))
			bo = list_first_entry(&kgem->requests[ring],
					      struct kgem_request,
					      list)->bo;

		ErrorF("%s: ring=%d, %d outstanding requests, oldest=%d\n",
		       __FUNCTION__, ring, count, bo ? bo->handle : 0);
	}
#endif

	return retired;
}

static bool kgem_retire__requests(struct kgem *kgem)
{
	bool retired = false;
	int n;

	for (n = 0; n < ARRAY_SIZE(kgem->requests); n++) {
		retired |= kgem_retire__requests_ring(kgem, n);
		kgem->need_retire |= !list_is_empty(&kgem->requests[n]);
	}

	return retired;
}

bool kgem_retire(struct kgem *kgem)
{
	bool retired = false;

	DBG(("%s, need_retire?=%d\n", __FUNCTION__, kgem->need_retire));

	kgem->need_retire = false;

	retired |= kgem_retire__flushing(kgem);
	retired |= kgem_retire__requests(kgem);
	retired |= kgem_retire__buffers(kgem);

	DBG(("%s -- retired=%d, need_retire=%d\n",
	     __FUNCTION__, retired, kgem->need_retire));

	kgem->retire(kgem);

	return retired;
}

bool __kgem_ring_is_idle(struct kgem *kgem, int ring)
{
	struct kgem_request *rq;

	assert(ring < ARRAY_SIZE(kgem->requests));
	assert(!list_is_empty(&kgem->requests[ring]));

	rq = list_last_entry(&kgem->requests[ring],
			     struct kgem_request, list);
	if (__kgem_busy(kgem, rq->bo->handle)) {
		DBG(("%s: last requests handle=%d still busy\n",
		     __FUNCTION__, rq->bo->handle));
		return false;
	}

	DBG(("%s: ring=%d idle (handle=%d)\n",
	     __FUNCTION__, ring, rq->bo->handle));

	kgem_retire__requests_ring(kgem, ring);
	kgem_retire__buffers(kgem);

	assert(list_is_empty(&kgem->requests[ring]));
	return true;
}

#ifndef NDEBUG
static void kgem_commit__check_buffers(struct kgem *kgem)
{
	struct kgem_buffer *bo;

	list_for_each_entry(bo, &kgem->active_buffers, base.list)
		assert(bo->base.exec == NULL);
}
#else
#define kgem_commit__check_buffers(kgem)
#endif

static void kgem_commit(struct kgem *kgem)
{
	struct kgem_request *rq = kgem->next_request;
	struct kgem_bo *bo, *next;

	list_for_each_entry_safe(bo, next, &rq->buffers, request) {
		assert(next->request.prev == &bo->request);

		DBG(("%s: release handle=%d (proxy? %d), dirty? %d flush? %d, snoop? %d -> offset=%x\n",
		     __FUNCTION__, bo->handle, bo->proxy != NULL,
		     bo->gpu_dirty, bo->needs_flush, bo->snoop,
		     (unsigned)bo->exec->offset));

		assert(bo->exec);
		assert(bo->proxy == NULL || bo->exec == &_kgem_dummy_exec);
		assert(RQ(bo->rq) == rq || (RQ(bo->proxy->rq) == rq));

		bo->presumed_offset = bo->exec->offset;
		bo->exec = NULL;
		bo->target_handle = -1;

		if (!bo->refcnt && !bo->reusable) {
			assert(!bo->snoop);
			assert(!bo->proxy);
			kgem_bo_free(kgem, bo);
			continue;
		}

		bo->binding.offset = 0;
		bo->domain = DOMAIN_GPU;
		bo->gpu_dirty = false;

		if (bo->proxy) {
			/* proxies are not used for domain tracking */
			__kgem_bo_clear_busy(bo);
		}

		kgem->scanout_busy |= bo->scanout;
	}

	if (rq == &kgem->static_request) {
		struct drm_i915_gem_set_domain set_domain;

		DBG(("%s: syncing due to allocation failure\n", __FUNCTION__));

		VG_CLEAR(set_domain);
		set_domain.handle = rq->bo->handle;
		set_domain.read_domains = I915_GEM_DOMAIN_GTT;
		set_domain.write_domain = I915_GEM_DOMAIN_GTT;
		if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &set_domain)) {
			DBG(("%s: sync: GPU hang detected\n", __FUNCTION__));
			kgem_throttle(kgem);
		}

		kgem_retire(kgem);
		assert(list_is_empty(&rq->buffers));

		assert(rq->bo->map__gtt == NULL);
		assert(rq->bo->map__cpu == NULL);
		gem_close(kgem->fd, rq->bo->handle);
		kgem_cleanup_cache(kgem);
	} else {
		list_add_tail(&rq->list, &kgem->requests[rq->ring]);
		kgem->need_throttle = kgem->need_retire = 1;
	}

	kgem->next_request = NULL;

	kgem_commit__check_buffers(kgem);
}

static void kgem_close_list(struct kgem *kgem, struct list *head)
{
	while (!list_is_empty(head))
		kgem_bo_free(kgem, list_first_entry(head, struct kgem_bo, list));
}

static void kgem_close_inactive(struct kgem *kgem)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(kgem->inactive); i++)
		kgem_close_list(kgem, &kgem->inactive[i]);
}

static void kgem_finish_buffers(struct kgem *kgem)
{
	struct kgem_buffer *bo, *next;

	list_for_each_entry_safe(bo, next, &kgem->batch_buffers, base.list) {
		DBG(("%s: buffer handle=%d, used=%d, exec?=%d, write=%d, mmapped=%s, refcnt=%d\n",
		     __FUNCTION__, bo->base.handle, bo->used, bo->base.exec!=NULL,
		     bo->write, bo->mmapped == MMAPPED_CPU ? "cpu" : bo->mmapped == MMAPPED_GTT ? "gtt" : "no",
		     bo->base.refcnt));

		assert(next->base.list.prev == &bo->base.list);
		assert(bo->base.io);
		assert(bo->base.refcnt >= 1);

		if (bo->base.refcnt > 1 && !bo->base.exec) {
			DBG(("%s: skipping unattached handle=%d, used=%d, refcnt=%d\n",
			     __FUNCTION__, bo->base.handle, bo->used, bo->base.refcnt));
			continue;
		}

		if (!bo->write) {
			assert(bo->base.exec || bo->base.refcnt > 1);
			goto decouple;
		}

		if (bo->mmapped) {
			uint32_t used;

			assert(!bo->need_io);

			used = ALIGN(bo->used, PAGE_SIZE);
			if (!DBG_NO_UPLOAD_ACTIVE &&
			    used + PAGE_SIZE <= bytes(&bo->base) &&
			    (kgem->has_llc || bo->mmapped == MMAPPED_GTT || bo->base.snoop)) {
				DBG(("%s: retaining upload buffer (%d/%d): used=%d, refcnt=%d\n",
				     __FUNCTION__, bo->used, bytes(&bo->base), used, bo->base.refcnt));
				bo->used = used;
				list_move(&bo->base.list,
					  &kgem->active_buffers);
				kgem->need_retire = true;
				continue;
			}
			DBG(("%s: discarding mmapped buffer, used=%d, map type=%d\n",
			     __FUNCTION__, bo->used, bo->mmapped));
			goto decouple;
		}

		if (!bo->used || !bo->base.exec) {
			/* Unless we replace the handle in the execbuffer,
			 * then this bo will become active. So decouple it
			 * from the buffer list and track it in the normal
			 * manner.
			 */
			goto decouple;
		}

		assert(bo->need_io);
		assert(bo->base.rq == MAKE_REQUEST(kgem->next_request, kgem->ring));
		assert(bo->base.domain != DOMAIN_GPU);

		if (bo->base.refcnt == 1 &&
		    bo->base.size.pages.count > 1 &&
		    bo->used < bytes(&bo->base) / 2) {
			struct kgem_bo *shrink;
			unsigned alloc = NUM_PAGES(bo->used);

			shrink = search_snoop_cache(kgem, alloc,
						    CREATE_INACTIVE | CREATE_NO_RETIRE);
			if (shrink) {
				void *map;
				int n;

				DBG(("%s: used=%d, shrinking %d to %d, handle %d to %d\n",
				     __FUNCTION__,
				     bo->used, bytes(&bo->base), bytes(shrink),
				     bo->base.handle, shrink->handle));

				assert(bo->used <= bytes(shrink));
				map = kgem_bo_map__cpu(kgem, shrink);
				if (map) {
					kgem_bo_sync__cpu(kgem, shrink);
					memcpy(map, bo->mem, bo->used);

					shrink->target_handle =
						kgem->has_handle_lut ? bo->base.target_handle : shrink->handle;
					for (n = 0; n < kgem->nreloc; n++) {
						if (kgem->reloc[n].target_handle == bo->base.target_handle) {
							kgem->reloc[n].target_handle = shrink->target_handle;
							kgem->reloc[n].presumed_offset = shrink->presumed_offset;
							kgem->batch[kgem->reloc[n].offset/sizeof(kgem->batch[0])] =
								kgem->reloc[n].delta + shrink->presumed_offset;
						}
					}

					bo->base.exec->handle = shrink->handle;
					bo->base.exec->offset = shrink->presumed_offset;
					shrink->exec = bo->base.exec;
					shrink->rq = bo->base.rq;
					list_replace(&bo->base.request,
						     &shrink->request);
					list_init(&bo->base.request);
					shrink->needs_flush = bo->base.gpu_dirty;

					bo->base.exec = NULL;
					bo->base.rq = NULL;
					bo->base.gpu_dirty = false;
					bo->base.needs_flush = false;
					bo->used = 0;

					goto decouple;
				}

				__kgem_bo_destroy(kgem, shrink);
			}

			shrink = search_linear_cache(kgem, alloc,
						     CREATE_INACTIVE | CREATE_NO_RETIRE);
			if (shrink) {
				int n;

				DBG(("%s: used=%d, shrinking %d to %d, handle %d to %d\n",
				     __FUNCTION__,
				     bo->used, bytes(&bo->base), bytes(shrink),
				     bo->base.handle, shrink->handle));

				assert(bo->used <= bytes(shrink));
				if (gem_write__cachealigned(kgem->fd, shrink->handle,
					      0, bo->used, bo->mem) == 0) {
					shrink->target_handle =
						kgem->has_handle_lut ? bo->base.target_handle : shrink->handle;
					for (n = 0; n < kgem->nreloc; n++) {
						if (kgem->reloc[n].target_handle == bo->base.target_handle) {
							kgem->reloc[n].target_handle = shrink->target_handle;
							kgem->reloc[n].presumed_offset = shrink->presumed_offset;
							kgem->batch[kgem->reloc[n].offset/sizeof(kgem->batch[0])] =
								kgem->reloc[n].delta + shrink->presumed_offset;
						}
					}

					bo->base.exec->handle = shrink->handle;
					bo->base.exec->offset = shrink->presumed_offset;
					shrink->exec = bo->base.exec;
					shrink->rq = bo->base.rq;
					list_replace(&bo->base.request,
						     &shrink->request);
					list_init(&bo->base.request);
					shrink->needs_flush = bo->base.gpu_dirty;

					bo->base.exec = NULL;
					bo->base.rq = NULL;
					bo->base.gpu_dirty = false;
					bo->base.needs_flush = false;
					bo->used = 0;

					goto decouple;
				}

				__kgem_bo_destroy(kgem, shrink);
			}
		}

		DBG(("%s: handle=%d, uploading %d/%d\n",
		     __FUNCTION__, bo->base.handle, bo->used, bytes(&bo->base)));
		ASSERT_IDLE(kgem, bo->base.handle);
		assert(bo->used <= bytes(&bo->base));
		gem_write__cachealigned(kgem->fd, bo->base.handle,
			  0, bo->used, bo->mem);
		bo->need_io = 0;

decouple:
		DBG(("%s: releasing handle=%d\n",
		     __FUNCTION__, bo->base.handle));
		list_del(&bo->base.list);
		kgem_bo_unref(kgem, &bo->base);
	}
}

static void kgem_cleanup(struct kgem *kgem)
{
	int n;

	for (n = 0; n < ARRAY_SIZE(kgem->requests); n++) {
		while (!list_is_empty(&kgem->requests[n])) {
			struct kgem_request *rq;

			rq = list_first_entry(&kgem->requests[n],
					      struct kgem_request,
					      list);
			while (!list_is_empty(&rq->buffers)) {
				struct kgem_bo *bo;

				bo = list_first_entry(&rq->buffers,
						      struct kgem_bo,
						      request);

				bo->exec = NULL;
				bo->gpu_dirty = false;
				__kgem_bo_clear_busy(bo);
				if (bo->refcnt == 0)
					kgem_bo_free(kgem, bo);
			}

			__kgem_request_free(rq);
		}
	}

	kgem_close_inactive(kgem);
}

static int kgem_batch_write(struct kgem *kgem, uint32_t handle, uint32_t size)
{
	int ret;

	ASSERT_IDLE(kgem, handle);

retry:
	/* If there is no surface data, just upload the batch */
	if (kgem->surface == kgem->batch_size) {
		if (gem_write__cachealigned(kgem->fd, handle,
				 0, sizeof(uint32_t)*kgem->nbatch,
					    kgem->batch) == 0)
			return 0;

		goto expire;
	}

	/* Are the batch pages conjoint with the surface pages? */
	if (kgem->surface < kgem->nbatch + PAGE_SIZE/sizeof(uint32_t)) {
		assert(size == PAGE_ALIGN(kgem->batch_size*sizeof(uint32_t)));
		if (gem_write__cachealigned(kgem->fd, handle,
				 0, kgem->batch_size*sizeof(uint32_t),
					    kgem->batch) == 0)
			return 0;

		goto expire;
	}

	/* Disjoint surface/batch, upload separately */
	if (gem_write__cachealigned(kgem->fd, handle,
			0, sizeof(uint32_t)*kgem->nbatch,
				    kgem->batch))
		goto expire;

	ret = PAGE_ALIGN(sizeof(uint32_t) * kgem->batch_size);
	ret -= sizeof(uint32_t) * kgem->surface;
	assert(size-ret >= kgem->nbatch*sizeof(uint32_t));
	if (gem_write(kgem->fd, handle,
			size - ret, (kgem->batch_size - kgem->surface)*sizeof(uint32_t),
		      kgem->batch + kgem->surface))
		goto expire;

	return 0;

expire:
	ret = errno;
	assert(ret != EINVAL);

	(void)__kgem_throttle_retire(kgem, 0);
	if (kgem_expire_cache(kgem))
		goto retry;

	if (kgem_cleanup_cache(kgem))
		goto retry;

	ErrorF("%s: failed to write batch (handle=%d): %d\n",
	       __FUNCTION__, handle, ret);
	return ret;
}

void kgem_reset(struct kgem *kgem)
{
	if (kgem->next_request) {
		struct kgem_request *rq = kgem->next_request;

		while (!list_is_empty(&rq->buffers)) {
			struct kgem_bo *bo =
				list_first_entry(&rq->buffers,
						 struct kgem_bo,
						 request);
			list_del(&bo->request);

			assert(RQ(bo->rq) == rq);

			bo->binding.offset = 0;
			bo->exec = NULL;
			bo->target_handle = -1;
			bo->gpu_dirty = false;

			if (bo->needs_flush && __kgem_busy(kgem, bo->handle)) {
				assert(bo->domain == DOMAIN_GPU || bo->domain == DOMAIN_NONE);
				list_add(&bo->request, &kgem->flushing);
				bo->rq = (void *)kgem;
				kgem->need_retire = true;
			} else
				__kgem_bo_clear_busy(bo);

			if (bo->refcnt || bo->rq)
				continue;

			if (bo->snoop) {
				kgem_bo_move_to_snoop(kgem, bo);
			} else if (bo->scanout) {
				kgem_bo_move_to_scanout(kgem, bo);
			} else if ((bo = kgem_bo_replace_io(bo))->reusable &&
				   kgem_bo_set_purgeable(kgem, bo)) {
				kgem_bo_move_to_inactive(kgem, bo);
			} else {
				DBG(("%s: closing %d\n",
				     __FUNCTION__, bo->handle));
				kgem_bo_free(kgem, bo);
			}
		}

		if (rq != &kgem->static_request) {
			list_init(&rq->list);
			__kgem_request_free(rq);
		}
	}

	kgem->nfence = 0;
	kgem->nexec = 0;
	kgem->nreloc = 0;
	kgem->nreloc__self = 0;
	kgem->aperture = 0;
	kgem->aperture_fenced = 0;
	kgem->aperture_max_fence = 0;
	kgem->nbatch = 0;
	kgem->surface = kgem->batch_size;
	kgem->mode = KGEM_NONE;
	kgem->flush = 0;
	kgem->batch_flags = kgem->batch_flags_base;

	kgem->next_request = __kgem_request_alloc(kgem);

	kgem_sna_reset(kgem);
}

static int compact_batch_surface(struct kgem *kgem)
{
	int size, shrink, n;

	if (!kgem->has_relaxed_delta)
		return kgem->batch_size;

	/* See if we can pack the contents into one or two pages */
	n = ALIGN(kgem->batch_size, 1024);
	size = n - kgem->surface + kgem->nbatch;
	size = ALIGN(size, 1024);

	shrink = n - size;
	if (shrink) {
		DBG(("shrinking from %d to %d\n", kgem->batch_size, size));

		shrink *= sizeof(uint32_t);
		for (n = 0; n < kgem->nreloc; n++) {
			if (kgem->reloc[n].read_domains == I915_GEM_DOMAIN_INSTRUCTION &&
			    kgem->reloc[n].target_handle == ~0U)
				kgem->reloc[n].delta -= shrink;

			if (kgem->reloc[n].offset >= sizeof(uint32_t)*kgem->nbatch)
				kgem->reloc[n].offset -= shrink;
		}
	}

	return size * sizeof(uint32_t);
}

static struct kgem_bo *
kgem_create_batch(struct kgem *kgem, int size)
{
	struct drm_i915_gem_set_domain set_domain;
	struct kgem_bo *bo;

	if (size <= 4096) {
		bo = list_first_entry(&kgem->pinned_batches[0],
				      struct kgem_bo,
				      list);
		if (!bo->rq) {
out_4096:
			list_move_tail(&bo->list, &kgem->pinned_batches[0]);
			return kgem_bo_reference(bo);
		}

		if (!__kgem_busy(kgem, bo->handle)) {
			assert(RQ(bo->rq)->bo == bo);
			__kgem_retire_rq(kgem, RQ(bo->rq));
			goto out_4096;
		}
	}

	if (size <= 16384) {
		bo = list_first_entry(&kgem->pinned_batches[1],
				      struct kgem_bo,
				      list);
		if (!bo->rq) {
out_16384:
			list_move_tail(&bo->list, &kgem->pinned_batches[1]);
			return kgem_bo_reference(bo);
		}

		if (!__kgem_busy(kgem, bo->handle)) {
			assert(RQ(bo->rq)->bo == bo);
			__kgem_retire_rq(kgem, RQ(bo->rq));
			goto out_16384;
		}
	}

	if (kgem->gen == 020 && !kgem->has_pinned_batches) {
		assert(size <= 16384);

		bo = list_first_entry(&kgem->pinned_batches[size > 4096],
				      struct kgem_bo,
				      list);
		list_move_tail(&bo->list, &kgem->pinned_batches[size > 4096]);

		DBG(("%s: syncing due to busy batches\n", __FUNCTION__));

		VG_CLEAR(set_domain);
		set_domain.handle = bo->handle;
		set_domain.read_domains = I915_GEM_DOMAIN_GTT;
		set_domain.write_domain = I915_GEM_DOMAIN_GTT;
		if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &set_domain)) {
			DBG(("%s: sync: GPU hang detected\n", __FUNCTION__));
			kgem_throttle(kgem);
			return NULL;
		}

		kgem_retire(kgem);
		assert(bo->rq == NULL);
		return kgem_bo_reference(bo);
	}

	return kgem_create_linear(kgem, size, CREATE_NO_THROTTLE);
}

void _kgem_submit(struct kgem *kgem)
{
	struct kgem_request *rq;
	uint32_t batch_end;
	int size;

	assert(!DBG_NO_HW);
	assert(!kgem->wedged);

	assert(kgem->nbatch);
	assert(kgem->nbatch <= KGEM_BATCH_SIZE(kgem));
	assert(kgem->nbatch <= kgem->surface);

	batch_end = kgem_end_batch(kgem);
	kgem_sna_flush(kgem);

	DBG(("batch[%d/%d, flags=%x]: %d %d %d %d, nreloc=%d, nexec=%d, nfence=%d, aperture=%d [fenced=%d]\n",
	     kgem->mode, kgem->ring, kgem->batch_flags,
	     batch_end, kgem->nbatch, kgem->surface, kgem->batch_size,
	     kgem->nreloc, kgem->nexec, kgem->nfence, kgem->aperture, kgem->aperture_fenced));

	assert(kgem->nbatch <= kgem->batch_size);
	assert(kgem->nbatch <= kgem->surface);
	assert(kgem->nreloc <= ARRAY_SIZE(kgem->reloc));
	assert(kgem->nexec < ARRAY_SIZE(kgem->exec));
	assert(kgem->nfence <= kgem->fence_max);

	kgem_finish_buffers(kgem);

#if SHOW_BATCH
	__kgem_batch_debug(kgem, batch_end);
#endif

	rq = kgem->next_request;
	if (kgem->surface != kgem->batch_size)
		size = compact_batch_surface(kgem);
	else
		size = kgem->nbatch * sizeof(kgem->batch[0]);
	rq->bo = kgem_create_batch(kgem, size);
	if (rq->bo) {
		uint32_t handle = rq->bo->handle;
		int i;

		assert(!rq->bo->needs_flush);

		i = kgem->nexec++;
		kgem->exec[i].handle = handle;
		kgem->exec[i].relocation_count = kgem->nreloc;
		kgem->exec[i].relocs_ptr = (uintptr_t)kgem->reloc;
		kgem->exec[i].alignment = 0;
		kgem->exec[i].offset = rq->bo->presumed_offset;
		kgem->exec[i].flags = 0;
		kgem->exec[i].rsvd1 = 0;
		kgem->exec[i].rsvd2 = 0;

		rq->bo->target_handle = kgem->has_handle_lut ? i : handle;
		rq->bo->exec = &kgem->exec[i];
		rq->bo->rq = MAKE_REQUEST(rq, kgem->ring); /* useful sanity check */
		list_add(&rq->bo->request, &rq->buffers);
		rq->ring = kgem->ring == KGEM_BLT;

		kgem_fixup_self_relocs(kgem, rq->bo);

		if (kgem_batch_write(kgem, handle, size) == 0) {
			struct drm_i915_gem_execbuffer2 execbuf;
			int ret, retry = 3;

			memset(&execbuf, 0, sizeof(execbuf));
			execbuf.buffers_ptr = (uintptr_t)kgem->exec;
			execbuf.buffer_count = kgem->nexec;
			execbuf.batch_len = batch_end*sizeof(uint32_t);
			execbuf.flags = kgem->ring | kgem->batch_flags;

   		    if (DEBUG_DUMP)
            {
                int fd = open("/tmp1/1/batchbuffer.bin", O_CREAT|O_WRONLY);
				if (fd != -1) {
					ret = write(fd, kgem->batch, batch_end*sizeof(uint32_t));
					fd = close(fd);
				}
                else printf("SNA: failed to write batchbuffer\n");
                asm volatile("int3");
			}

			ret = drmIoctl(kgem->fd,
				       DRM_IOCTL_I915_GEM_EXECBUFFER2,
				       &execbuf);
			while (ret == -1 && errno == EBUSY && retry--) {
				__kgem_throttle(kgem);
				ret = drmIoctl(kgem->fd,
					       DRM_IOCTL_I915_GEM_EXECBUFFER2,
					       &execbuf);
			}
			if (DEBUG_SYNC && ret == 0) {
				struct drm_i915_gem_set_domain set_domain;

				VG_CLEAR(set_domain);
				set_domain.handle = handle;
				set_domain.read_domains = I915_GEM_DOMAIN_GTT;
				set_domain.write_domain = I915_GEM_DOMAIN_GTT;

				ret = drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &set_domain);
			}
			if (ret == -1) {
				DBG(("%s: GPU hang detected [%d]\n",
				     __FUNCTION__, errno));
				kgem_throttle(kgem);
				kgem->wedged = true;

#if 0
				ret = errno;
				ErrorF("batch[%d/%d]: %d %d %d, nreloc=%d, nexec=%d, nfence=%d, aperture=%d, fenced=%d, high=%d,%d: errno=%d\n",
				       kgem->mode, kgem->ring, batch_end, kgem->nbatch, kgem->surface,
				       kgem->nreloc, kgem->nexec, kgem->nfence, kgem->aperture, kgem->aperture_fenced, kgem->aperture_high, kgem->aperture_total, errno);

				for (i = 0; i < kgem->nexec; i++) {
					struct kgem_bo *bo, *found = NULL;

					list_for_each_entry(bo, &kgem->next_request->buffers, request) {
						if (bo->handle == kgem->exec[i].handle) {
							found = bo;
							break;
						}
					}
					ErrorF("exec[%d] = handle:%d, presumed offset: %x, size: %d, tiling %d, fenced %d, snooped %d, deleted %d\n",
					       i,
					       kgem->exec[i].handle,
					       (int)kgem->exec[i].offset,
					       found ? kgem_bo_size(found) : -1,
					       found ? found->tiling : -1,
					       (int)(kgem->exec[i].flags & EXEC_OBJECT_NEEDS_FENCE),
					       found ? found->snoop : -1,
					       found ? found->purged : -1);
				}
				for (i = 0; i < kgem->nreloc; i++) {
					ErrorF("reloc[%d] = pos:%d, target:%d, delta:%d, read:%x, write:%x, offset:%x\n",
					       i,
					       (int)kgem->reloc[i].offset,
					       kgem->reloc[i].target_handle,
					       kgem->reloc[i].delta,
					       kgem->reloc[i].read_domains,
					       kgem->reloc[i].write_domain,
					       (int)kgem->reloc[i].presumed_offset);
				}

				if (DEBUG_SYNC) {
					int fd = open("/tmp/batchbuffer", O_WRONLY | O_CREAT | O_APPEND, 0666);
					if (fd != -1) {
						write(fd, kgem->batch, batch_end*sizeof(uint32_t));
						close(fd);
					}

					FatalError("SNA: failed to submit batchbuffer, errno=%d\n", ret);
				}
#endif
			}
		}

		kgem_commit(kgem);
	}
	if (kgem->wedged)
		kgem_cleanup(kgem);

	kgem_reset(kgem);

	assert(kgem->next_request != NULL);
}

void kgem_throttle(struct kgem *kgem)
{
	kgem->need_throttle = 0;
	if (kgem->wedged)
		return;

	kgem->wedged = __kgem_throttle(kgem);
	if (kgem->wedged) {
		printf("Detected a hung GPU, disabling acceleration.\n");
		printf("When reporting this, please include i915_error_state from debugfs and the full dmesg.\n");
	}
}

static void kgem_purge_cache(struct kgem *kgem)
{
	struct kgem_bo *bo, *next;
	int i;

	for (i = 0; i < ARRAY_SIZE(kgem->inactive); i++) {
		list_for_each_entry_safe(bo, next, &kgem->inactive[i], list) {
			if (!kgem_bo_is_retained(kgem, bo)) {
				DBG(("%s: purging %d\n",
				     __FUNCTION__, bo->handle));
				kgem_bo_free(kgem, bo);
			}
		}
	}

	kgem->need_purge = false;
}


void kgem_clean_large_cache(struct kgem *kgem)
{
	while (!list_is_empty(&kgem->large_inactive)) {
		kgem_bo_free(kgem,
			     list_first_entry(&kgem->large_inactive,
					      struct kgem_bo, list));

	}
}

bool kgem_expire_cache(struct kgem *kgem)
{
	time_t now, expire;
	struct kgem_bo *bo;
	unsigned int size = 0, count = 0;
	bool idle;
	unsigned int i;

	time(&now);

	while (__kgem_freed_bo) {
		bo = __kgem_freed_bo;
		__kgem_freed_bo = *(struct kgem_bo **)bo;
		free(bo);
	}

	while (__kgem_freed_request) {
		struct kgem_request *rq = __kgem_freed_request;
		__kgem_freed_request = *(struct kgem_request **)rq;
		free(rq);
	}

	kgem_clean_large_cache(kgem);

	expire = 0;
	list_for_each_entry(bo, &kgem->snoop, list) {
		if (bo->delta) {
			expire = now - MAX_INACTIVE_TIME/2;
			break;
		}

		bo->delta = now;
	}
	if (expire) {
		while (!list_is_empty(&kgem->snoop)) {
			bo = list_last_entry(&kgem->snoop, struct kgem_bo, list);

			if (bo->delta > expire)
				break;

			kgem_bo_free(kgem, bo);
		}
	}
#ifdef DEBUG_MEMORY
	{
		long snoop_size = 0;
		int snoop_count = 0;
		list_for_each_entry(bo, &kgem->snoop, list)
			snoop_count++, snoop_size += bytes(bo);
		ErrorF("%s: still allocated %d bo, %ld bytes, in snoop cache\n",
		       __FUNCTION__, snoop_count, snoop_size);
	}
#endif

	kgem_retire(kgem);
	if (kgem->wedged)
		kgem_cleanup(kgem);

	kgem->expire(kgem);

	if (kgem->need_purge)
		kgem_purge_cache(kgem);

	expire = 0;

	idle = !kgem->need_retire;
	for (i = 0; i < ARRAY_SIZE(kgem->inactive); i++) {
		idle &= list_is_empty(&kgem->inactive[i]);
		list_for_each_entry(bo, &kgem->inactive[i], list) {
			if (bo->delta) {
				expire = now - MAX_INACTIVE_TIME;
				break;
			}

			bo->delta = now;
		}
	}
	if (idle) {
		DBG(("%s: idle\n", __FUNCTION__));
		kgem->need_expire = false;
		return false;
	}
	if (expire == 0)
		return true;

	idle = !kgem->need_retire;
	for (i = 0; i < ARRAY_SIZE(kgem->inactive); i++) {
		struct list preserve;

		list_init(&preserve);
		while (!list_is_empty(&kgem->inactive[i])) {
			bo = list_last_entry(&kgem->inactive[i],
					     struct kgem_bo, list);

			if (bo->delta > expire) {
				idle = false;
				break;
			}

			if (bo->map__cpu && bo->delta + MAP_PRESERVE_TIME > expire) {
				idle = false;
				list_move_tail(&bo->list, &preserve);
			} else {
				count++;
				size += bytes(bo);
				kgem_bo_free(kgem, bo);
				DBG(("%s: expiring %d\n",
				     __FUNCTION__, bo->handle));
			}
		}
		if (!list_is_empty(&preserve)) {
			preserve.prev->next = kgem->inactive[i].next;
			kgem->inactive[i].next->prev = preserve.prev;
			kgem->inactive[i].next = preserve.next;
			preserve.next->prev = &kgem->inactive[i];
		}
	}

#ifdef DEBUG_MEMORY
	{
		long inactive_size = 0;
		int inactive_count = 0;
		for (i = 0; i < ARRAY_SIZE(kgem->inactive); i++)
			list_for_each_entry(bo, &kgem->inactive[i], list)
				inactive_count++, inactive_size += bytes(bo);
		ErrorF("%s: still allocated %d bo, %ld bytes, in inactive cache\n",
		       __FUNCTION__, inactive_count, inactive_size);
	}
#endif

	DBG(("%s: expired %d objects, %d bytes, idle? %d\n",
	     __FUNCTION__, count, size, idle));

	kgem->need_expire = !idle;
	return !idle;
	(void)count;
	(void)size;
}

bool kgem_cleanup_cache(struct kgem *kgem)
{
	unsigned int i;
	int n;

	/* sync to the most recent request */
	for (n = 0; n < ARRAY_SIZE(kgem->requests); n++) {
		if (!list_is_empty(&kgem->requests[n])) {
			struct kgem_request *rq;
			struct drm_i915_gem_set_domain set_domain;

			rq = list_first_entry(&kgem->requests[n],
					      struct kgem_request,
					      list);

			DBG(("%s: sync on cleanup\n", __FUNCTION__));

			VG_CLEAR(set_domain);
			set_domain.handle = rq->bo->handle;
			set_domain.read_domains = I915_GEM_DOMAIN_GTT;
			set_domain.write_domain = I915_GEM_DOMAIN_GTT;
			(void)drmIoctl(kgem->fd,
				       DRM_IOCTL_I915_GEM_SET_DOMAIN,
				       &set_domain);
		}
	}

	kgem_retire(kgem);
	kgem_cleanup(kgem);

	if (!kgem->need_expire)
		return false;

	for (i = 0; i < ARRAY_SIZE(kgem->inactive); i++) {
		while (!list_is_empty(&kgem->inactive[i]))
			kgem_bo_free(kgem,
				     list_last_entry(&kgem->inactive[i],
						     struct kgem_bo, list));
	}

	kgem_clean_large_cache(kgem);

	while (!list_is_empty(&kgem->snoop))
		kgem_bo_free(kgem,
			     list_last_entry(&kgem->snoop,
					     struct kgem_bo, list));

	while (__kgem_freed_bo) {
		struct kgem_bo *bo = __kgem_freed_bo;
		__kgem_freed_bo = *(struct kgem_bo **)bo;
		free(bo);
	}

	kgem->need_purge = false;
	kgem->need_expire = false;
	return true;
}

static struct kgem_bo *
search_linear_cache(struct kgem *kgem, unsigned int num_pages, unsigned flags)
{
	struct kgem_bo *bo, *first = NULL;
	bool use_active = (flags & CREATE_INACTIVE) == 0;
	struct list *cache;

	DBG(("%s: num_pages=%d, flags=%x, use_active? %d, use_large=%d [max=%d]\n",
	     __FUNCTION__, num_pages, flags, use_active,
	     num_pages >= MAX_CACHE_SIZE / PAGE_SIZE,
	     MAX_CACHE_SIZE / PAGE_SIZE));

	assert(num_pages);

	if (num_pages >= MAX_CACHE_SIZE / PAGE_SIZE) {
		DBG(("%s: searching large buffers\n", __FUNCTION__));
retry_large:
		cache = use_active ? &kgem->large : &kgem->large_inactive;
		list_for_each_entry_safe(bo, first, cache, list) {
			assert(bo->refcnt == 0);
			assert(bo->reusable);
			assert(!bo->scanout);

			if (num_pages > num_pages(bo))
				goto discard;

			if (bo->tiling != I915_TILING_NONE) {
				if (use_active)
					goto discard;

				if (!gem_set_tiling(kgem->fd, bo->handle,
						    I915_TILING_NONE, 0))
					goto discard;

				bo->tiling = I915_TILING_NONE;
				bo->pitch = 0;
			}

			if (bo->purged && !kgem_bo_clear_purgeable(kgem, bo))
				goto discard;

			list_del(&bo->list);
			if (RQ(bo->rq) == (void *)kgem) {
				assert(bo->exec == NULL);
				list_del(&bo->request);
			}

			bo->delta = 0;
			assert_tiling(kgem, bo);
			return bo;

discard:
			if (!use_active)
				kgem_bo_free(kgem, bo);
		}

		if (use_active) {
			use_active = false;
			goto retry_large;
		}

		if (__kgem_throttle_retire(kgem, flags))
			goto retry_large;

		return NULL;
	}

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
			assert(for_cpu ? bo->map__cpu : bo->map__gtt);
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
			assert(list_is_empty(&bo->vma));
			assert(list_is_empty(&bo->list));

			bo->tiling = I915_TILING_NONE;
			bo->pitch = 0;
			bo->delta = 0;
			DBG(("  %s: found handle=%d (num_pages=%d) in linear vma cache\n",
			     __FUNCTION__, bo->handle, num_pages(bo)));
			assert(use_active || bo->domain != DOMAIN_GPU);
			assert(!bo->needs_flush);
			assert_tiling(kgem, bo);
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

		if (bo->map__gtt || bo->map__cpu) {
			if (flags & (CREATE_CPU_MAP | CREATE_GTT_MAP)) {
				int for_cpu = !!(flags & CREATE_CPU_MAP);
				if (for_cpu ? bo->map__cpu : bo->map__gtt){
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
			if (flags & CREATE_GTT_MAP && !kgem_bo_can_map(kgem, bo))
				continue;

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
		assert(list_is_empty(&bo->vma));
		assert(use_active || bo->domain != DOMAIN_GPU);
		assert(!bo->needs_flush || use_active);
		assert_tiling(kgem, bo);
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
		assert(list_is_empty(&first->vma));
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
	assert(size);

	if (flags & CREATE_GTT_MAP && kgem->has_llc) {
		flags &= ~CREATE_GTT_MAP;
		flags |= CREATE_CPU_MAP;
	}

	size = NUM_PAGES(size);
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

inline int kgem_bo_fenced_size(struct kgem *kgem, struct kgem_bo *bo)
{
	unsigned int size;

	assert(bo->tiling);
	assert_tiling(kgem, bo);
	assert(kgem->gen < 040);

	if (kgem->gen < 030)
		size = 512 * 1024 / PAGE_SIZE;
	else
		size = 1024 * 1024 / PAGE_SIZE;
	while (size < num_pages(bo))
		size <<= 1;

	return size;
}

struct kgem_bo *kgem_create_2d(struct kgem *kgem,
			       int width,
			       int height,
			       int bpp,
			       int tiling,
			       uint32_t flags)
{
	struct list *cache;
	struct kgem_bo *bo;
	uint32_t pitch, tiled_height, size;
	uint32_t handle;
	int i, bucket, retry;
	bool exact = flags & (CREATE_EXACT | CREATE_SCANOUT);

	if (tiling < 0)
		exact = true, tiling = -tiling;

	DBG(("%s(%dx%d, bpp=%d, tiling=%d, exact=%d, inactive=%d, cpu-mapping=%d, gtt-mapping=%d, scanout?=%d, prime?=%d, temp?=%d)\n", __FUNCTION__,
	     width, height, bpp, tiling, exact,
	     !!(flags & CREATE_INACTIVE),
	     !!(flags & CREATE_CPU_MAP),
	     !!(flags & CREATE_GTT_MAP),
	     !!(flags & CREATE_SCANOUT),
	     !!(flags & CREATE_PRIME),
	     !!(flags & CREATE_TEMPORARY)));

	size = kgem_surface_size(kgem, kgem->has_relaxed_fencing, flags,
				 width, height, bpp, tiling, &pitch);
	assert(size && size <= kgem->max_object_size);
	size /= PAGE_SIZE;
	bucket = cache_bucket(size);

	if (bucket >= NUM_CACHE_BUCKETS) {
		DBG(("%s: large bo num pages=%d, bucket=%d\n",
		     __FUNCTION__, size, bucket));

		if (flags & CREATE_INACTIVE)
			goto large_inactive;

		tiled_height = kgem_aligned_height(kgem, height, tiling);

		list_for_each_entry(bo, &kgem->large, list) {
			assert(!bo->purged);
			assert(!bo->scanout);
			assert(bo->refcnt == 0);
			assert(bo->reusable);
			assert_tiling(kgem, bo);

			if (kgem->gen < 040) {
				if (bo->pitch < pitch) {
					DBG(("tiled and pitch too small: tiling=%d, (want %d), pitch=%d, need %d\n",
					     bo->tiling, tiling,
					     bo->pitch, pitch));
					continue;
				}

				if (bo->pitch * tiled_height > bytes(bo))
					continue;
			} else {
				if (num_pages(bo) < size)
					continue;

				if (bo->pitch != pitch || bo->tiling != tiling) {
					if (!gem_set_tiling(kgem->fd, bo->handle,
							    tiling, pitch))
						continue;

					bo->pitch = pitch;
					bo->tiling = tiling;
				}
			}

			kgem_bo_remove_from_active(kgem, bo);

			bo->unique_id = kgem_get_unique_id(kgem);
			bo->delta = 0;
			DBG(("  1:from active: pitch=%d, tiling=%d, handle=%d, id=%d\n",
			     bo->pitch, bo->tiling, bo->handle, bo->unique_id));
			assert(bo->pitch*kgem_aligned_height(kgem, height, bo->tiling) <= kgem_bo_size(bo));
			assert_tiling(kgem, bo);
			bo->refcnt = 1;
			return bo;
		}

large_inactive:
		__kgem_throttle_retire(kgem, flags);
		list_for_each_entry(bo, &kgem->large_inactive, list) {
			assert(bo->refcnt == 0);
			assert(bo->reusable);
			assert(!bo->scanout);
			assert_tiling(kgem, bo);

			if (size > num_pages(bo))
				continue;

			if (bo->tiling != tiling ||
			    (tiling != I915_TILING_NONE && bo->pitch != pitch)) {
				if (!gem_set_tiling(kgem->fd, bo->handle,
						    tiling, pitch))
					continue;

				bo->tiling = tiling;
				bo->pitch = pitch;
			}

			if (bo->purged && !kgem_bo_clear_purgeable(kgem, bo)) {
				kgem_bo_free(kgem, bo);
				break;
			}

			list_del(&bo->list);

			assert(bo->domain != DOMAIN_GPU);
			bo->unique_id = kgem_get_unique_id(kgem);
			bo->pitch = pitch;
			bo->delta = 0;
			DBG(("  1:from large inactive: pitch=%d, tiling=%d, handle=%d, id=%d\n",
			     bo->pitch, bo->tiling, bo->handle, bo->unique_id));
			assert(bo->pitch*kgem_aligned_height(kgem, height, bo->tiling) <= kgem_bo_size(bo));
			assert_tiling(kgem, bo);
			bo->refcnt = 1;
			return bo;
		}

		goto create;
	}

	if (flags & (CREATE_CPU_MAP | CREATE_GTT_MAP)) {
		int for_cpu = !!(flags & CREATE_CPU_MAP);
		if (kgem->has_llc && tiling == I915_TILING_NONE)
			for_cpu = 1;
		/* We presume that we will need to upload to this bo,
		 * and so would prefer to have an active VMA.
		 */
		cache = &kgem->vma[for_cpu].inactive[bucket];
		do {
			list_for_each_entry(bo, cache, vma) {
				assert(bucket(bo) == bucket);
				assert(bo->refcnt == 0);
				assert(!bo->scanout);
				assert(for_cpu ? bo->map__cpu : bo->map__gtt);
				assert(bo->rq == NULL);
				assert(bo->exec == NULL);
				assert(list_is_empty(&bo->request));
				assert(bo->flush == false);
				assert_tiling(kgem, bo);

				if (size > num_pages(bo)) {
					DBG(("inactive too small: %d < %d\n",
					     num_pages(bo), size));
					continue;
				}

				if (bo->tiling != tiling ||
				    (tiling != I915_TILING_NONE && bo->pitch != pitch)) {
					DBG(("inactive vma with wrong tiling: %d < %d\n",
					     bo->tiling, tiling));
					continue;
				}

				if (bo->purged && !kgem_bo_clear_purgeable(kgem, bo)) {
					kgem_bo_free(kgem, bo);
					break;
				}

				assert(bo->tiling == tiling);
				bo->pitch = pitch;
				bo->delta = 0;
				bo->unique_id = kgem_get_unique_id(kgem);
				bo->domain = DOMAIN_NONE;

				kgem_bo_remove_from_inactive(kgem, bo);
				assert(list_is_empty(&bo->list));
				assert(list_is_empty(&bo->vma));

				DBG(("  from inactive vma: pitch=%d, tiling=%d: handle=%d, id=%d\n",
				     bo->pitch, bo->tiling, bo->handle, bo->unique_id));
				assert(bo->reusable);
				assert(bo->domain != DOMAIN_GPU);
				ASSERT_IDLE(kgem, bo->handle);
				assert(bo->pitch*kgem_aligned_height(kgem, height, bo->tiling) <= kgem_bo_size(bo));
				assert_tiling(kgem, bo);
				bo->refcnt = 1;
				return bo;
			}
		} while (!list_is_empty(cache) &&
			 __kgem_throttle_retire(kgem, flags));

		if (flags & CREATE_CPU_MAP && !kgem->has_llc) {
			if (list_is_empty(&kgem->active[bucket][tiling]) &&
			    list_is_empty(&kgem->inactive[bucket]))
				flags &= ~CREATE_CACHED;

			goto create;
	}
	}

	if (flags & CREATE_INACTIVE)
		goto skip_active_search;

	/* Best active match */
	retry = NUM_CACHE_BUCKETS - bucket;
	if (retry > 3 && (flags & CREATE_TEMPORARY) == 0)
		retry = 3;
search_again:
	assert(bucket < NUM_CACHE_BUCKETS);
	cache = &kgem->active[bucket][tiling];
	if (tiling) {
		tiled_height = kgem_aligned_height(kgem, height, tiling);
		list_for_each_entry(bo, cache, list) {
			assert(!bo->purged);
			assert(bo->refcnt == 0);
			assert(bucket(bo) == bucket);
			assert(bo->reusable);
			assert(bo->tiling == tiling);
			assert(bo->flush == false);
			assert(!bo->scanout);
			assert_tiling(kgem, bo);

			if (kgem->gen < 040) {
				if (bo->pitch < pitch) {
					DBG(("tiled and pitch too small: tiling=%d, (want %d), pitch=%d, need %d\n",
					     bo->tiling, tiling,
					     bo->pitch, pitch));
					continue;
				}

				if (bo->pitch * tiled_height > bytes(bo))
					continue;
			} else {
				if (num_pages(bo) < size)
					continue;

				if (bo->pitch != pitch) {
					if (!gem_set_tiling(kgem->fd,
							    bo->handle,
							    tiling, pitch))
						continue;

					bo->pitch = pitch;
				}
			}

			kgem_bo_remove_from_active(kgem, bo);

			bo->unique_id = kgem_get_unique_id(kgem);
			bo->delta = 0;
			DBG(("  1:from active: pitch=%d, tiling=%d, handle=%d, id=%d\n",
			     bo->pitch, bo->tiling, bo->handle, bo->unique_id));
			assert(bo->pitch*kgem_aligned_height(kgem, height, bo->tiling) <= kgem_bo_size(bo));
			assert_tiling(kgem, bo);
			bo->refcnt = 1;
			return bo;
		}
	} else {
		list_for_each_entry(bo, cache, list) {
			assert(bucket(bo) == bucket);
			assert(!bo->purged);
			assert(bo->refcnt == 0);
			assert(bo->reusable);
			assert(!bo->scanout);
			assert(bo->tiling == tiling);
			assert(bo->flush == false);
			assert_tiling(kgem, bo);

			if (num_pages(bo) < size)
				continue;

			kgem_bo_remove_from_active(kgem, bo);

			bo->pitch = pitch;
			bo->unique_id = kgem_get_unique_id(kgem);
			bo->delta = 0;
			DBG(("  1:from active: pitch=%d, tiling=%d, handle=%d, id=%d\n",
			     bo->pitch, bo->tiling, bo->handle, bo->unique_id));
			assert(bo->pitch*kgem_aligned_height(kgem, height, bo->tiling) <= kgem_bo_size(bo));
			assert_tiling(kgem, bo);
			bo->refcnt = 1;
			return bo;
		}
	}

	if (--retry && exact) {
		if (kgem->gen >= 040) {
			for (i = I915_TILING_NONE; i <= I915_TILING_Y; i++) {
				if (i == tiling)
					continue;

				cache = &kgem->active[bucket][i];
				list_for_each_entry(bo, cache, list) {
					assert(!bo->purged);
					assert(bo->refcnt == 0);
					assert(bo->reusable);
					assert(!bo->scanout);
					assert(bo->flush == false);
					assert_tiling(kgem, bo);

					if (num_pages(bo) < size)
						continue;

					if (!gem_set_tiling(kgem->fd,
							    bo->handle,
							    tiling, pitch))
						continue;

					kgem_bo_remove_from_active(kgem, bo);

					bo->unique_id = kgem_get_unique_id(kgem);
					bo->pitch = pitch;
					bo->tiling = tiling;
					bo->delta = 0;
					DBG(("  1:from active: pitch=%d, tiling=%d, handle=%d, id=%d\n",
					     bo->pitch, bo->tiling, bo->handle, bo->unique_id));
					assert(bo->pitch*kgem_aligned_height(kgem, height, bo->tiling) <= kgem_bo_size(bo));
					assert_tiling(kgem, bo);
					bo->refcnt = 1;
					return bo;
				}
			}
		}

		bucket++;
		goto search_again;
	}

	if (!exact) { /* allow an active near-miss? */
		i = tiling;
		while (--i >= 0) {
			tiled_height = kgem_surface_size(kgem, kgem->has_relaxed_fencing, flags,
							 width, height, bpp, tiling, &pitch);
			cache = active(kgem, tiled_height / PAGE_SIZE, i);
			tiled_height = kgem_aligned_height(kgem, height, i);
			list_for_each_entry(bo, cache, list) {
				assert(!bo->purged);
				assert(bo->refcnt == 0);
				assert(bo->reusable);
				assert(!bo->scanout);
				assert(bo->flush == false);
				assert_tiling(kgem, bo);

				if (bo->tiling) {
					if (bo->pitch < pitch) {
						DBG(("tiled and pitch too small: tiling=%d, (want %d), pitch=%d, need %d\n",
						     bo->tiling, tiling,
						     bo->pitch, pitch));
						continue;
					}
				} else
					bo->pitch = pitch;

				if (bo->pitch * tiled_height > bytes(bo))
					continue;

				kgem_bo_remove_from_active(kgem, bo);

				bo->unique_id = kgem_get_unique_id(kgem);
				bo->delta = 0;
				DBG(("  1:from active: pitch=%d, tiling=%d, handle=%d, id=%d\n",
				     bo->pitch, bo->tiling, bo->handle, bo->unique_id));
				assert(bo->pitch*kgem_aligned_height(kgem, height, bo->tiling) <= kgem_bo_size(bo));
				assert_tiling(kgem, bo);
				bo->refcnt = 1;
				return bo;
			}
		}
	}

skip_active_search:
	bucket = cache_bucket(size);
	retry = NUM_CACHE_BUCKETS - bucket;
	if (retry > 3)
		retry = 3;
search_inactive:
	/* Now just look for a close match and prefer any currently active */
	assert(bucket < NUM_CACHE_BUCKETS);
	cache = &kgem->inactive[bucket];
	list_for_each_entry(bo, cache, list) {
		assert(bucket(bo) == bucket);
		assert(bo->reusable);
		assert(!bo->scanout);
		assert(bo->flush == false);
		assert_tiling(kgem, bo);

		if (size > num_pages(bo)) {
			DBG(("inactive too small: %d < %d\n",
			     num_pages(bo), size));
			continue;
		}

		if (bo->tiling != tiling ||
		    (tiling != I915_TILING_NONE && bo->pitch != pitch)) {
			if (!gem_set_tiling(kgem->fd, bo->handle,
					    tiling, pitch))
				continue;
		}

		if (bo->purged && !kgem_bo_clear_purgeable(kgem, bo)) {
			kgem_bo_free(kgem, bo);
			break;
		}

		kgem_bo_remove_from_inactive(kgem, bo);
		assert(list_is_empty(&bo->list));
		assert(list_is_empty(&bo->vma));

		bo->pitch = pitch;
		bo->tiling = tiling;

		bo->delta = 0;
		bo->unique_id = kgem_get_unique_id(kgem);
		assert(bo->pitch);
		DBG(("  from inactive: pitch=%d, tiling=%d: handle=%d, id=%d\n",
		     bo->pitch, bo->tiling, bo->handle, bo->unique_id));
		assert(bo->refcnt == 0);
		assert(bo->reusable);
		assert((flags & CREATE_INACTIVE) == 0 || bo->domain != DOMAIN_GPU);
		ASSERT_MAYBE_IDLE(kgem, bo->handle, flags & CREATE_INACTIVE);
		assert(bo->pitch*kgem_aligned_height(kgem, height, bo->tiling) <= kgem_bo_size(bo));
		assert_tiling(kgem, bo);
		bo->refcnt = 1;
		return bo;
	}

	if (flags & CREATE_INACTIVE &&
	    !list_is_empty(&kgem->active[bucket][tiling]) &&
	    __kgem_throttle_retire(kgem, flags)) {
		flags &= ~CREATE_INACTIVE;
		goto search_inactive;
	}

	if (--retry) {
		bucket++;
		flags &= ~CREATE_INACTIVE;
		goto search_inactive;
	}

create:
	if (flags & CREATE_CACHED)
		return NULL;

	if (bucket >= NUM_CACHE_BUCKETS)
		size = ALIGN(size, 1024);
	handle = gem_create(kgem->fd, size);
	if (handle == 0)
		return NULL;

	bo = __kgem_bo_alloc(handle, size);
	if (!bo) {
		gem_close(kgem->fd, handle);
		return NULL;
	}

	bo->unique_id = kgem_get_unique_id(kgem);
	if (tiling == I915_TILING_NONE ||
	    gem_set_tiling(kgem->fd, handle, tiling, pitch)) {
		bo->tiling = tiling;
		bo->pitch = pitch;
	} else {
		if (flags & CREATE_EXACT) {
			if (bo->pitch != pitch || bo->tiling != tiling) {
				kgem_bo_free(kgem, bo);
				return NULL;
			}
		}
	}

	assert(bytes(bo) >= bo->pitch * kgem_aligned_height(kgem, height, bo->tiling));
	assert_tiling(kgem, bo);

	debug_alloc__bo(kgem, bo);

	DBG(("  new pitch=%d, tiling=%d, handle=%d, id=%d, num_pages=%d [%d], bucket=%d\n",
	     bo->pitch, bo->tiling, bo->handle, bo->unique_id,
	     size, num_pages(bo), bucket(bo)));
	return bo;
}

#if 0
struct kgem_bo *kgem_create_cpu_2d(struct kgem *kgem,
				   int width,
				   int height,
				   int bpp,
				   uint32_t flags)
{
	struct kgem_bo *bo;
	int stride, size;

	if (DBG_NO_CPU)
		return NULL;

	DBG(("%s(%dx%d, bpp=%d)\n", __FUNCTION__, width, height, bpp));

	if (kgem->has_llc) {
		bo = kgem_create_2d(kgem, width, height, bpp,
				    I915_TILING_NONE, flags);
		if (bo == NULL)
			return bo;

		assert(bo->tiling == I915_TILING_NONE);
		assert_tiling(kgem, bo);

		if (kgem_bo_map__cpu(kgem, bo) == NULL) {
			kgem_bo_destroy(kgem, bo);
			return NULL;
		}

		return bo;
	}

	assert(width > 0 && height > 0);
	stride = ALIGN(width, 2) * bpp >> 3;
	stride = ALIGN(stride, 4);
	size = stride * ALIGN(height, 2);
	assert(size >= PAGE_SIZE);

	DBG(("%s: %dx%d, %d bpp, stride=%d\n",
	     __FUNCTION__, width, height, bpp, stride));

	bo = search_snoop_cache(kgem, NUM_PAGES(size), 0);
	if (bo) {
		assert(bo->tiling == I915_TILING_NONE);
		assert_tiling(kgem, bo);
		assert(bo->snoop);
		bo->refcnt = 1;
		bo->pitch = stride;
		bo->unique_id = kgem_get_unique_id(kgem);
		return bo;
	}

	if (kgem->has_caching) {
		bo = kgem_create_linear(kgem, size, flags);
		if (bo == NULL)
			return NULL;

		assert(bo->tiling == I915_TILING_NONE);
		assert_tiling(kgem, bo);

		if (!gem_set_caching(kgem->fd, bo->handle, SNOOPED)) {
			kgem_bo_destroy(kgem, bo);
			return NULL;
		}
		bo->snoop = true;

		if (kgem_bo_map__cpu(kgem, bo) == NULL) {
			kgem_bo_destroy(kgem, bo);
			return NULL;
		}

		bo->pitch = stride;
		bo->unique_id = kgem_get_unique_id(kgem);
		return bo;
	}

	if (kgem->has_userptr) {
		void *ptr;

		/* XXX */
		//if (posix_memalign(&ptr, 64, ALIGN(size, 64)))
		if (posix_memalign(&ptr, PAGE_SIZE, ALIGN(size, PAGE_SIZE)))
			return NULL;

		bo = kgem_create_map(kgem, ptr, size, false);
		if (bo == NULL) {
			free(ptr);
			return NULL;
		}

		bo->pitch = stride;
		bo->unique_id = kgem_get_unique_id(kgem);
		return bo;
	}

		return NULL;
}
#endif

void _kgem_bo_destroy(struct kgem *kgem, struct kgem_bo *bo)
{
	DBG(("%s: handle=%d, proxy? %d\n",
	     __FUNCTION__, bo->handle, bo->proxy != NULL));

	if (bo->proxy) {
		assert(!bo->reusable);
		kgem_bo_binding_free(kgem, bo);

		assert(list_is_empty(&bo->list));
		_list_del(&bo->vma);
		_list_del(&bo->request);

		if (bo->io && bo->domain == DOMAIN_CPU)
			_kgem_bo_delete_buffer(kgem, bo);

		kgem_bo_unref(kgem, bo->proxy);

		*(struct kgem_bo **)bo = __kgem_freed_bo;
		__kgem_freed_bo = bo;
	} else
	__kgem_bo_destroy(kgem, bo);
}

static void __kgem_flush(struct kgem *kgem, struct kgem_bo *bo)
{
	assert(bo->rq);
	assert(bo->exec == NULL);
	assert(bo->needs_flush);

	/* The kernel will emit a flush *and* update its own flushing lists. */
	if (!__kgem_busy(kgem, bo->handle))
		__kgem_bo_clear_busy(bo);

	DBG(("%s: handle=%d, busy?=%d\n",
	     __FUNCTION__, bo->handle, bo->rq != NULL));
}

void kgem_scanout_flush(struct kgem *kgem, struct kgem_bo *bo)
{
	kgem_bo_submit(kgem, bo);
	if (!bo->needs_flush)
		return;

	/* If the kernel fails to emit the flush, then it will be forced when
	 * we assume direct access. And as the usual failure is EIO, we do
	 * not actually care.
	 */
	assert(bo->exec == NULL);
	if (bo->rq)
		__kgem_flush(kgem, bo);

	/* Whatever actually happens, we can regard the GTT write domain
	 * as being flushed.
	 */
	bo->gtt_dirty = false;
	bo->needs_flush = false;
	bo->domain = DOMAIN_NONE;
}

inline static bool needs_semaphore(struct kgem *kgem, struct kgem_bo *bo)
{
	return kgem->nreloc && bo->rq && RQ_RING(bo->rq) != kgem->ring;
}

static bool aperture_check(struct kgem *kgem, unsigned num_pages)
{
	if (kgem->aperture) {
		struct drm_i915_gem_get_aperture aperture;

		VG_CLEAR(aperture);
		aperture.aper_available_size = kgem->aperture_high;
		aperture.aper_available_size *= PAGE_SIZE;
		(void)drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_GET_APERTURE, &aperture);

		DBG(("%s: aperture required %ld bytes, available %ld bytes\n",
		     __FUNCTION__,
		     (long)num_pages * PAGE_SIZE,
		     (long)aperture.aper_available_size));

		/* Leave some space in case of alignment issues */
		aperture.aper_available_size -= 1024 * 1024;
		aperture.aper_available_size -= kgem->aperture_mappable * PAGE_SIZE / 2;
		if (kgem->gen < 033)
			aperture.aper_available_size -= kgem->aperture_max_fence * PAGE_SIZE;
		if (!kgem->has_llc)
			aperture.aper_available_size -= 2 * kgem->nexec * PAGE_SIZE;

		DBG(("%s: num_pages=%d, estimated max usable=%ld\n",
		     __FUNCTION__, num_pages, (long)(aperture.aper_available_size/PAGE_SIZE)));

		if (num_pages <= aperture.aper_available_size / PAGE_SIZE)
			return true;
	}

	return false;
}

static inline bool kgem_flush(struct kgem *kgem, bool flush)
{
	if (unlikely(kgem->wedged))
		return false;

	if (kgem->nreloc == 0)
		return true;

	if (container_of(kgem, struct sna, kgem)->flags & SNA_POWERSAVE)
		return true;

	if (kgem->flush == flush && kgem->aperture < kgem->aperture_low)
		return true;

	DBG(("%s: opportunistic flushing? flush=%d,%d, aperture=%d/%d, idle?=%d\n",
	     __FUNCTION__, kgem->flush, flush, kgem->aperture, kgem->aperture_low, kgem_ring_is_idle(kgem, kgem->ring)));
	return !kgem_ring_is_idle(kgem, kgem->ring);
}

bool kgem_check_bo(struct kgem *kgem, ...)
{
	va_list ap;
	struct kgem_bo *bo;
	int num_exec = 0;
	int num_pages = 0;
	bool flush = false;
	bool busy = true;

	va_start(ap, kgem);
	while ((bo = va_arg(ap, struct kgem_bo *))) {
		while (bo->proxy)
			bo = bo->proxy;
		if (bo->exec)
			continue;

		if (needs_semaphore(kgem, bo)) {
			DBG(("%s: flushing for required semaphore\n", __FUNCTION__));
			return false;
		}

		num_pages += num_pages(bo);
		num_exec++;

		flush |= bo->flush;
		busy &= bo->rq != NULL;
	}
	va_end(ap);

	DBG(("%s: num_pages=+%d, num_exec=+%d\n",
	     __FUNCTION__, num_pages, num_exec));

	if (!num_pages)
		return true;

	if (kgem->nexec + num_exec >= KGEM_EXEC_SIZE(kgem)) {
		DBG(("%s: out of exec slots (%d + %d / %d)\n", __FUNCTION__,
		     kgem->nexec, num_exec, KGEM_EXEC_SIZE(kgem)));
		return false;
	}

	if (num_pages + kgem->aperture > kgem->aperture_high) {
		DBG(("%s: final aperture usage (%d) is greater than high water mark (%d)\n",
		     __FUNCTION__, num_pages + kgem->aperture, kgem->aperture_high));
		if (!aperture_check(kgem, num_pages + kgem->aperture))
		return false;
	}

	if (busy)
		return true;

	return kgem_flush(kgem, flush);
}

#if 0
bool kgem_check_bo_fenced(struct kgem *kgem, struct kgem_bo *bo)
{
	assert(bo->refcnt);
	while (bo->proxy)
		bo = bo->proxy;
	assert(bo->refcnt);

	if (bo->exec) {
		if (kgem->gen < 040 &&
		    bo->tiling != I915_TILING_NONE &&
		    (bo->exec->flags & EXEC_OBJECT_NEEDS_FENCE) == 0) {
			uint32_t size;

			assert(bo->tiling == I915_TILING_X);

			if (kgem->nfence >= kgem->fence_max)
				return false;

			if (kgem->aperture_fenced) {
				size = 3*kgem->aperture_fenced;
				if (kgem->aperture_total == kgem->aperture_mappable)
					size += kgem->aperture;
				if (size > kgem->aperture_mappable &&
				    kgem_ring_is_idle(kgem, kgem->ring)) {
					DBG(("%s: opportunistic fence flush\n", __FUNCTION__));
					return false;
				}
			}

			size = kgem_bo_fenced_size(kgem, bo);
			if (size > kgem->aperture_max_fence)
				kgem->aperture_max_fence = size;
			size += kgem->aperture_fenced;
			if (kgem->gen < 033)
				size += kgem->aperture_max_fence;
			if (kgem->aperture_total == kgem->aperture_mappable)
				size += kgem->aperture;
			if (size > kgem->aperture_mappable) {
				DBG(("%s: estimated fence space required [%d] exceed aperture [%d]\n",
				     __FUNCTION__, size, kgem->aperture_mappable));
				return false;
			}
		}

		return true;
	}

	if (kgem->nexec >= KGEM_EXEC_SIZE(kgem) - 1)
		return false;

	if (needs_semaphore(kgem, bo)) {
		DBG(("%s: flushing for required semaphore\n", __FUNCTION__));
		return false;
	}

	assert_tiling(kgem, bo);
	if (kgem->gen < 040 && bo->tiling != I915_TILING_NONE) {
		uint32_t size;

		assert(bo->tiling == I915_TILING_X);

		if (kgem->nfence >= kgem->fence_max)
			return false;

		if (kgem->aperture_fenced) {
			size = 3*kgem->aperture_fenced;
			if (kgem->aperture_total == kgem->aperture_mappable)
				size += kgem->aperture;
			if (size > kgem->aperture_mappable &&
			    kgem_ring_is_idle(kgem, kgem->ring)) {
				DBG(("%s: opportunistic fence flush\n", __FUNCTION__));
				return false;
			}
		}

		size = kgem_bo_fenced_size(kgem, bo);
		if (size > kgem->aperture_max_fence)
			kgem->aperture_max_fence = size;
		size += kgem->aperture_fenced;
		if (kgem->gen < 033)
			size += kgem->aperture_max_fence;
		if (kgem->aperture_total == kgem->aperture_mappable)
			size += kgem->aperture;
		if (size > kgem->aperture_mappable) {
			DBG(("%s: estimated fence space required [%d] exceed aperture [%d]\n",
			     __FUNCTION__, size, kgem->aperture_mappable));
			return false;
		}
	}

	if (kgem->aperture + kgem->aperture_fenced + num_pages(bo) > kgem->aperture_high) {
		DBG(("%s: final aperture usage (%d) is greater than high water mark (%d)\n",
		     __FUNCTION__, num_pages(bo) + kgem->aperture, kgem->aperture_high));
		if (!aperture_check(kgem, num_pages(bo) + kgem->aperture + kgem->aperture_fenced))
			return false;
	}

	if (bo->rq)
		return true;

	return kgem_flush(kgem, bo->flush);
}
#endif

















uint32_t kgem_add_reloc(struct kgem *kgem,
			uint32_t pos,
			struct kgem_bo *bo,
			uint32_t read_write_domain,
			uint32_t delta)
{
	int index;

	DBG(("%s: handle=%d, pos=%d, delta=%d, domains=%08x\n",
	     __FUNCTION__, bo ? bo->handle : 0, pos, delta, read_write_domain));

	assert(kgem->gen < 0100);
	assert((read_write_domain & 0x7fff) == 0 || bo != NULL);

	index = kgem->nreloc++;
	assert(index < ARRAY_SIZE(kgem->reloc));
	kgem->reloc[index].offset = pos * sizeof(kgem->batch[0]);
	if (bo) {
		assert(kgem->mode != KGEM_NONE);
		assert(bo->refcnt);
		while (bo->proxy) {
			DBG(("%s: adding proxy [delta=%d] for handle=%d\n",
			     __FUNCTION__, bo->delta, bo->handle));
			delta += bo->delta;
			assert(bo->handle == bo->proxy->handle);
			/* need to release the cache upon batch submit */
			if (bo->exec == NULL) {
				list_move_tail(&bo->request,
					       &kgem->next_request->buffers);
				bo->rq = MAKE_REQUEST(kgem->next_request,
						      kgem->ring);
				bo->exec = &_kgem_dummy_exec;
				bo->domain = DOMAIN_GPU;
		}

			if (read_write_domain & 0x7fff && !bo->gpu_dirty)
				__kgem_bo_mark_dirty(bo);

			bo = bo->proxy;
			assert(bo->refcnt);
		}
		assert(bo->refcnt);

		if (bo->exec == NULL)
			kgem_add_bo(kgem, bo);
		assert(bo->rq == MAKE_REQUEST(kgem->next_request, kgem->ring));
		assert(RQ_RING(bo->rq) == kgem->ring);

		if (kgem->gen < 040 && read_write_domain & KGEM_RELOC_FENCED) {
			if (bo->tiling &&
			    (bo->exec->flags & EXEC_OBJECT_NEEDS_FENCE) == 0) {
				assert(bo->tiling == I915_TILING_X);
				assert(kgem->nfence < kgem->fence_max);
				kgem->aperture_fenced +=
					kgem_bo_fenced_size(kgem, bo);
				kgem->nfence++;
			}
			bo->exec->flags |= EXEC_OBJECT_NEEDS_FENCE;
		}

		kgem->reloc[index].delta = delta;
		kgem->reloc[index].target_handle = bo->target_handle;
		kgem->reloc[index].presumed_offset = bo->presumed_offset;

		if (read_write_domain & 0x7fff && !bo->gpu_dirty) {
			assert(!bo->snoop || kgem->can_blt_cpu);
			__kgem_bo_mark_dirty(bo);
		}

		delta += bo->presumed_offset;
	} else {
		kgem->reloc[index].delta = delta;
		kgem->reloc[index].target_handle = ~0U;
		kgem->reloc[index].presumed_offset = 0;
		if (kgem->nreloc__self < 256)
			kgem->reloc__self[kgem->nreloc__self++] = index;
		}
	kgem->reloc[index].read_domains = read_write_domain >> 16;
	kgem->reloc[index].write_domain = read_write_domain & 0x7fff;

	return delta;
}

uint64_t kgem_add_reloc64(struct kgem *kgem,
			  uint32_t pos,
			  struct kgem_bo *bo,
			  uint32_t read_write_domain,
			  uint64_t delta)
{
	int index;

	DBG(("%s: handle=%d, pos=%d, delta=%ld, domains=%08x\n",
	     __FUNCTION__, bo ? bo->handle : 0, pos, (long)delta, read_write_domain));

	assert(kgem->gen >= 0100);
	assert((read_write_domain & 0x7fff) == 0 || bo != NULL);

	index = kgem->nreloc++;
	assert(index < ARRAY_SIZE(kgem->reloc));
	kgem->reloc[index].offset = pos * sizeof(kgem->batch[0]);
	if (bo) {
		assert(kgem->mode != KGEM_NONE);
		assert(bo->refcnt);
		while (bo->proxy) {
			DBG(("%s: adding proxy [delta=%ld] for handle=%d\n",
			     __FUNCTION__, (long)bo->delta, bo->handle));
			delta += bo->delta;
			assert(bo->handle == bo->proxy->handle);
			/* need to release the cache upon batch submit */
			if (bo->exec == NULL) {
				list_move_tail(&bo->request,
					       &kgem->next_request->buffers);
				bo->rq = MAKE_REQUEST(kgem->next_request,
						      kgem->ring);
				bo->exec = &_kgem_dummy_exec;
				bo->domain = DOMAIN_GPU;
			}

			if (read_write_domain & 0x7fff && !bo->gpu_dirty)
				__kgem_bo_mark_dirty(bo);

			bo = bo->proxy;
			assert(bo->refcnt);
		}
		assert(bo->refcnt);

		if (bo->exec == NULL)
			kgem_add_bo(kgem, bo);
		assert(bo->rq == MAKE_REQUEST(kgem->next_request, kgem->ring));
		assert(RQ_RING(bo->rq) == kgem->ring);

		kgem->reloc[index].delta = delta;
		kgem->reloc[index].target_handle = bo->target_handle;
		kgem->reloc[index].presumed_offset = bo->presumed_offset;

		if (read_write_domain & 0x7fff && !bo->gpu_dirty) {
			assert(!bo->snoop || kgem->can_blt_cpu);
			__kgem_bo_mark_dirty(bo);
		}

		delta += bo->presumed_offset;
	} else {
		kgem->reloc[index].delta = delta;
		kgem->reloc[index].target_handle = ~0U;
		kgem->reloc[index].presumed_offset = 0;
		if (kgem->nreloc__self < 256)
			kgem->reloc__self[kgem->nreloc__self++] = index;
	}
	kgem->reloc[index].read_domains = read_write_domain >> 16;
	kgem->reloc[index].write_domain = read_write_domain & 0x7fff;

	return delta;
}

static void kgem_trim_vma_cache(struct kgem *kgem, int type, int bucket)
{
	int i, j;

	DBG(("%s: type=%d, count=%d (bucket: %d)\n",
	     __FUNCTION__, type, kgem->vma[type].count, bucket));
	if (kgem->vma[type].count <= 0)
	       return;

	if (kgem->need_purge)
		kgem_purge_cache(kgem);

	/* vma are limited on a per-process basis to around 64k.
	 * This includes all malloc arenas as well as other file
	 * mappings. In order to be fair and not hog the cache,
	 * and more importantly not to exhaust that limit and to
	 * start failing mappings, we keep our own number of open
	 * vma to within a conservative value.
	 */
	i = 0;
	while (kgem->vma[type].count > 0) {
		struct kgem_bo *bo = NULL;
		void **ptr;

		for (j = 0;
		     bo == NULL && j < ARRAY_SIZE(kgem->vma[type].inactive);
		     j++) {
			struct list *head = &kgem->vma[type].inactive[i++%ARRAY_SIZE(kgem->vma[type].inactive)];
			if (!list_is_empty(head))
				bo = list_last_entry(head, struct kgem_bo, vma);
	}
		if (bo == NULL)
			break;

		DBG(("%s: discarding inactive %s vma cache for %d\n",
		     __FUNCTION__, type ? "CPU" : "GTT", bo->handle));

		ptr = type ? &bo->map__cpu : &bo->map__gtt;
			assert(bo->rq == NULL);

		VG(if (type) VALGRIND_MAKE_MEM_NOACCESS(MAP(*ptr), bytes(bo)));
//		munmap(MAP(*ptr), bytes(bo));
		*ptr = NULL;
		list_del(&bo->vma);
		kgem->vma[type].count--;

		if (!bo->purged && !kgem_bo_set_purgeable(kgem, bo)) {
			DBG(("%s: freeing unpurgeable old mapping\n",
			     __FUNCTION__));
				kgem_bo_free(kgem, bo);
			}
	}
}

void *kgem_bo_map__async(struct kgem *kgem, struct kgem_bo *bo)
{
	void *ptr;

	DBG(("%s: handle=%d, offset=%ld, tiling=%d, map=%p:%p, domain=%d\n", __FUNCTION__,
	     bo->handle, (long)bo->presumed_offset, bo->tiling, bo->map__gtt, bo->map__cpu, bo->domain));

	assert(bo->proxy == NULL);
	assert(list_is_empty(&bo->list));
	assert_tiling(kgem, bo);

	if (bo->tiling == I915_TILING_NONE && !bo->scanout && kgem->has_llc) {
		DBG(("%s: converting request for GTT map into CPU map\n",
		     __FUNCTION__));
		return kgem_bo_map__cpu(kgem, bo);
	}

	ptr = MAP(bo->map__gtt);
	if (ptr == NULL) {
		assert(num_pages(bo) <= kgem->aperture_mappable / 2);

		kgem_trim_vma_cache(kgem, MAP_GTT, bucket(bo));

		ptr = __kgem_bo_map__gtt(kgem, bo);
		if (ptr == NULL)
			return NULL;

		/* Cache this mapping to avoid the overhead of an
		 * excruciatingly slow GTT pagefault. This is more an
		 * issue with compositing managers which need to frequently
		 * flush CPU damage to their GPU bo.
		 */
		bo->map__gtt = ptr;
		DBG(("%s: caching GTT vma for %d\n", __FUNCTION__, bo->handle));
	}

	return ptr;
}

void *kgem_bo_map(struct kgem *kgem, struct kgem_bo *bo)
{
	void *ptr;

	DBG(("%s: handle=%d, offset=%ld, tiling=%d, map=%p:%p, domain=%d\n", __FUNCTION__,
	     bo->handle, (long)bo->presumed_offset, bo->tiling, bo->map__gtt, bo->map__cpu, bo->domain));

	assert(bo->proxy == NULL);
	assert(list_is_empty(&bo->list));
	assert(bo->exec == NULL);
	assert_tiling(kgem, bo);

	if (bo->tiling == I915_TILING_NONE && !bo->scanout &&
	    (kgem->has_llc || bo->domain == DOMAIN_CPU)) {
		DBG(("%s: converting request for GTT map into CPU map\n",
		     __FUNCTION__));
		ptr = kgem_bo_map__cpu(kgem, bo);
		if (ptr)
			kgem_bo_sync__cpu(kgem, bo);
		return ptr;
	}

	ptr = MAP(bo->map__gtt);
	if (ptr == NULL) {
		assert(num_pages(bo) <= kgem->aperture_mappable / 2);
		assert(kgem->gen != 021 || bo->tiling != I915_TILING_Y);

		kgem_trim_vma_cache(kgem, MAP_GTT, bucket(bo));

		ptr = __kgem_bo_map__gtt(kgem, bo);
		if (ptr == NULL)
			return NULL;

		/* Cache this mapping to avoid the overhead of an
		 * excruciatingly slow GTT pagefault. This is more an
		 * issue with compositing managers which need to frequently
		 * flush CPU damage to their GPU bo.
		 */
		bo->map__gtt = ptr;
		DBG(("%s: caching GTT vma for %d\n", __FUNCTION__, bo->handle));
		}

	if (bo->domain != DOMAIN_GTT || FORCE_MMAP_SYNC & (1 << DOMAIN_GTT)) {
		struct drm_i915_gem_set_domain set_domain;

		DBG(("%s: sync: needs_flush? %d, domain? %d, busy? %d\n", __FUNCTION__,
		     bo->needs_flush, bo->domain, __kgem_busy(kgem, bo->handle)));

		/* XXX use PROT_READ to avoid the write flush? */

		VG_CLEAR(set_domain);
		set_domain.handle = bo->handle;
		set_domain.read_domains = I915_GEM_DOMAIN_GTT;
		set_domain.write_domain = I915_GEM_DOMAIN_GTT;
		if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &set_domain) == 0) {
			kgem_bo_retire(kgem, bo);
			bo->domain = DOMAIN_GTT;
			bo->gtt_dirty = true;
		}
		}

	return ptr;
}

void *kgem_bo_map__gtt(struct kgem *kgem, struct kgem_bo *bo)
{
	void *ptr;

	DBG(("%s: handle=%d, offset=%ld, tiling=%d, map=%p:%p, domain=%d\n", __FUNCTION__,
	     bo->handle, (long)bo->presumed_offset, bo->tiling, bo->map__gtt, bo->map__cpu, bo->domain));

	assert(bo->exec == NULL);
	assert(list_is_empty(&bo->list));
	assert_tiling(kgem, bo);

	ptr = MAP(bo->map__gtt);
	if (ptr == NULL) {
		assert(num_pages(bo) <= kgem->aperture_mappable / 4);

		kgem_trim_vma_cache(kgem, MAP_GTT, bucket(bo));

		ptr = __kgem_bo_map__gtt(kgem, bo);
		if (ptr == NULL)
			return NULL;

		/* Cache this mapping to avoid the overhead of an
		 * excruciatingly slow GTT pagefault. This is more an
		 * issue with compositing managers which need to frequently
		 * flush CPU damage to their GPU bo.
		 */
		bo->map__gtt = ptr;
		DBG(("%s: caching GTT vma for %d\n", __FUNCTION__, bo->handle));
	}

	return ptr;
}

void *kgem_bo_map__debug(struct kgem *kgem, struct kgem_bo *bo)
{
	return kgem_bo_map__async(kgem, bo);
}

void *kgem_bo_map__cpu(struct kgem *kgem, struct kgem_bo *bo)
{
	struct drm_i915_gem_mmap mmap_arg;

	DBG(("%s(handle=%d, size=%d, map=%p:%p)\n",
	     __FUNCTION__, bo->handle, bytes(bo), bo->map__gtt, bo->map__cpu));
	assert(!bo->purged);
	assert(list_is_empty(&bo->list));
	assert(bo->proxy == NULL);

	if (bo->map__cpu)
		return MAP(bo->map__cpu);

	kgem_trim_vma_cache(kgem, MAP_CPU, bucket(bo));

retry:
	VG_CLEAR(mmap_arg);
	mmap_arg.handle = bo->handle;
	mmap_arg.offset = 0;
	mmap_arg.size = bytes(bo);
	if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_MMAP, &mmap_arg)) {
		int err = 0;


		if (__kgem_throttle_retire(kgem, 0))
			goto retry;

		if (kgem_cleanup_cache(kgem))
			goto retry;

		ErrorF("%s: failed to mmap handle=%d, %d bytes, into CPU domain: %d\n",
		       __FUNCTION__, bo->handle, bytes(bo), err);
		return NULL;
	}

	VG(VALGRIND_MAKE_MEM_DEFINED(mmap_arg.addr_ptr, bytes(bo)));

	DBG(("%s: caching CPU vma for %d\n", __FUNCTION__, bo->handle));
	return bo->map__cpu = (void *)(uintptr_t)mmap_arg.addr_ptr;
}


/*
struct kgem_bo *kgem_create_map(struct kgem *kgem,
				void *ptr, uint32_t size,
				bool read_only)
{
	struct kgem_bo *bo;
	uintptr_t first_page, last_page;
	uint32_t handle;

	assert(MAP(ptr) == ptr);

	if (!kgem->has_userptr)
		return NULL;

	first_page = (uintptr_t)ptr;
	last_page = first_page + size + PAGE_SIZE - 1;

	first_page &= ~(PAGE_SIZE-1);
	last_page &= ~(PAGE_SIZE-1);
	assert(last_page > first_page);

	handle = gem_userptr(kgem->fd,
			     (void *)first_page, last_page-first_page,
			     read_only);
	if (handle == 0)
		return NULL;

	bo = __kgem_bo_alloc(handle, (last_page - first_page) / PAGE_SIZE);
	if (bo == NULL) {
		gem_close(kgem->fd, handle);
		return NULL;
	}

	bo->snoop = !kgem->has_llc;
	debug_alloc__bo(kgem, bo);

	if (first_page != (uintptr_t)ptr) {
		struct kgem_bo *proxy;

		proxy = kgem_create_proxy(kgem, bo,
					  (uintptr_t)ptr - first_page, size);
		kgem_bo_destroy(kgem, bo);
		if (proxy == NULL)
		return NULL;

		bo = proxy;
	}

	bo->map__cpu = MAKE_USER_MAP(ptr);

	DBG(("%s(ptr=%p, size=%d, pages=%d, read_only=%d) => handle=%d (proxy? %d)\n",
	     __FUNCTION__, ptr, size, NUM_PAGES(size), read_only, handle, bo->proxy != NULL));
	return bo;
}
*/

void kgem_bo_sync__cpu(struct kgem *kgem, struct kgem_bo *bo)
{
	DBG(("%s: handle=%d\n", __FUNCTION__, bo->handle));
	assert(!bo->scanout);
	kgem_bo_submit(kgem, bo);

	/* SHM pixmaps use proxies for subpage offsets */
	assert(!bo->purged);
	while (bo->proxy)
		bo = bo->proxy;
	assert(!bo->purged);

	if (bo->domain != DOMAIN_CPU || FORCE_MMAP_SYNC & (1 << DOMAIN_CPU)) {
		struct drm_i915_gem_set_domain set_domain;

		DBG(("%s: SYNC: handle=%d, needs_flush? %d, domain? %d, busy? %d\n",
		     __FUNCTION__, bo->handle,
		     bo->needs_flush, bo->domain,
		     __kgem_busy(kgem, bo->handle)));

		VG_CLEAR(set_domain);
		set_domain.handle = bo->handle;
		set_domain.read_domains = I915_GEM_DOMAIN_CPU;
		set_domain.write_domain = I915_GEM_DOMAIN_CPU;

		if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &set_domain) == 0) {
			kgem_bo_retire(kgem, bo);
			bo->domain = DOMAIN_CPU;
		}
	}
}

void kgem_bo_sync__cpu_full(struct kgem *kgem, struct kgem_bo *bo, bool write)
{
	DBG(("%s: handle=%d\n", __FUNCTION__, bo->handle));
	assert(!bo->scanout || !write);

	if (write || bo->needs_flush)
		kgem_bo_submit(kgem, bo);

	/* SHM pixmaps use proxies for subpage offsets */
	assert(!bo->purged);
	assert(bo->refcnt);
	while (bo->proxy)
		bo = bo->proxy;
	assert(bo->refcnt);
	assert(!bo->purged);

	if (bo->domain != DOMAIN_CPU || FORCE_MMAP_SYNC & (1 << DOMAIN_CPU)) {
		struct drm_i915_gem_set_domain set_domain;

		DBG(("%s: SYNC: handle=%d, needs_flush? %d, domain? %d, busy? %d\n",
		     __FUNCTION__, bo->handle,
		     bo->needs_flush, bo->domain,
		     __kgem_busy(kgem, bo->handle)));

		VG_CLEAR(set_domain);
		set_domain.handle = bo->handle;
		set_domain.read_domains = I915_GEM_DOMAIN_CPU;
		set_domain.write_domain = write ? I915_GEM_DOMAIN_CPU : 0;

		if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &set_domain) == 0) {
			if (bo->exec == NULL)
				kgem_bo_retire(kgem, bo);
			bo->domain = write ? DOMAIN_CPU : DOMAIN_NONE;
		}
	}
}

void kgem_bo_sync__gtt(struct kgem *kgem, struct kgem_bo *bo)
{
	DBG(("%s: handle=%d\n", __FUNCTION__, bo->handle));
	assert(bo->refcnt);
	assert(bo->proxy == NULL);

	kgem_bo_submit(kgem, bo);

	if (bo->domain != DOMAIN_GTT || FORCE_MMAP_SYNC & (1 << DOMAIN_GTT)) {
		struct drm_i915_gem_set_domain set_domain;

		DBG(("%s: SYNC: handle=%d, needs_flush? %d, domain? %d, busy? %d\n",
		     __FUNCTION__, bo->handle,
		     bo->needs_flush, bo->domain,
		     __kgem_busy(kgem, bo->handle)));

		VG_CLEAR(set_domain);
		set_domain.handle = bo->handle;
		set_domain.read_domains = I915_GEM_DOMAIN_GTT;
		set_domain.write_domain = I915_GEM_DOMAIN_GTT;

		if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &set_domain) == 0) {
			kgem_bo_retire(kgem, bo);
			bo->domain = DOMAIN_GTT;
			bo->gtt_dirty = true;
		}
	}
}

void kgem_clear_dirty(struct kgem *kgem)
{
	struct list * const buffers = &kgem->next_request->buffers;
	struct kgem_bo *bo;

	list_for_each_entry(bo, buffers, request) {
		if (!bo->gpu_dirty)
			break;

		bo->gpu_dirty = false;
	}
}

struct kgem_bo *kgem_create_proxy(struct kgem *kgem,
				  struct kgem_bo *target,
				  int offset, int length)
{
	struct kgem_bo *bo;

	DBG(("%s: target handle=%d [proxy? %d], offset=%d, length=%d, io=%d\n",
	     __FUNCTION__, target->handle, target->proxy ? target->proxy->delta : -1,
	     offset, length, target->io));

	bo = __kgem_bo_alloc(target->handle, length);
	if (bo == NULL)
		return NULL;

	bo->unique_id = kgem_get_unique_id(kgem);
	bo->reusable = false;
	bo->size.bytes = length;

	bo->io = target->io && target->proxy == NULL;
	bo->gpu_dirty = target->gpu_dirty;
	bo->tiling = target->tiling;
	bo->pitch = target->pitch;
	bo->flush = target->flush;
	bo->snoop = target->snoop;

	assert(!bo->scanout);
	bo->proxy = kgem_bo_reference(target);
	bo->delta = offset;

	if (target->exec && !bo->io) {
		list_move_tail(&bo->request, &kgem->next_request->buffers);
		bo->exec = &_kgem_dummy_exec;
	}
	bo->rq = target->rq;

	return bo;
}

#if 0
static struct kgem_buffer *
buffer_alloc(void)
{
	struct kgem_buffer *bo;

	bo = malloc(sizeof(*bo));
	if (bo == NULL)
		return NULL;

	bo->mem = NULL;
	bo->need_io = false;
	bo->mmapped = MMAPPED_CPU;

	return bo;
}

static struct kgem_buffer *
buffer_alloc_with_data(int num_pages)
{
	struct kgem_buffer *bo;

	bo = malloc(sizeof(*bo) + 2*UPLOAD_ALIGNMENT + num_pages * PAGE_SIZE);
	if (bo == NULL)
		return NULL;

	bo->mem = (void *)ALIGN((uintptr_t)bo + sizeof(*bo), UPLOAD_ALIGNMENT);
	bo->mmapped = false;
	return bo;
}

static inline bool
use_snoopable_buffer(struct kgem *kgem, uint32_t flags)
{
	if ((flags & KGEM_BUFFER_WRITE) == 0)
		return kgem->gen >= 030;

	return true;
}

static void
init_buffer_from_bo(struct kgem_buffer *bo, struct kgem_bo *old)
{
	DBG(("%s: reusing handle=%d for buffer\n",
	     __FUNCTION__, old->handle));

	assert(old->proxy == NULL);

	memcpy(&bo->base, old, sizeof(*old));
	if (old->rq)
		list_replace(&old->request, &bo->base.request);
	else
		list_init(&bo->base.request);
	list_replace(&old->vma, &bo->base.vma);
	list_init(&bo->base.list);
	free(old);

	assert(bo->base.tiling == I915_TILING_NONE);

	bo->base.refcnt = 1;
}

static struct kgem_buffer *
search_snoopable_buffer(struct kgem *kgem, unsigned alloc)
{
	struct kgem_buffer *bo;
	struct kgem_bo *old;

	old = search_snoop_cache(kgem, alloc, 0);
	if (old) {
		if (!old->io) {
			bo = buffer_alloc();
			if (bo == NULL)
				return NULL;

			init_buffer_from_bo(bo, old);
		} else {
			bo = (struct kgem_buffer *)old;
			bo->base.refcnt = 1;
		}

		DBG(("%s: created CPU handle=%d for buffer, size %d\n",
		     __FUNCTION__, bo->base.handle, num_pages(&bo->base)));

		assert(bo->base.snoop);
		assert(bo->base.tiling == I915_TILING_NONE);
		assert(num_pages(&bo->base) >= alloc);
		assert(bo->mmapped == MMAPPED_CPU);
		assert(bo->need_io == false);

		bo->mem = kgem_bo_map__cpu(kgem, &bo->base);
		if (bo->mem == NULL) {
			bo->base.refcnt = 0;
			kgem_bo_free(kgem, &bo->base);
			bo = NULL;
		}

		return bo;
	}

	return NULL;
}

static struct kgem_buffer *
create_snoopable_buffer(struct kgem *kgem, unsigned alloc)
{
	struct kgem_buffer *bo;
	uint32_t handle;

	if (kgem->has_llc) {
		struct kgem_bo *old;

		bo = buffer_alloc();
		if (bo == NULL)
			return NULL;

		old = search_linear_cache(kgem, alloc,
					 CREATE_INACTIVE | CREATE_CPU_MAP | CREATE_EXACT);
		if (old) {
			init_buffer_from_bo(bo, old);
		} else {
			handle = gem_create(kgem->fd, alloc);
			if (handle == 0) {
				free(bo);
				return NULL;
			}

			debug_alloc(kgem, alloc);
			__kgem_bo_init(&bo->base, handle, alloc);
			DBG(("%s: created CPU (LLC) handle=%d for buffer, size %d\n",
			     __FUNCTION__, bo->base.handle, alloc));
		}

		assert(bo->base.refcnt == 1);
		assert(bo->mmapped == MMAPPED_CPU);
		assert(bo->need_io == false);

		bo->mem = kgem_bo_map__cpu(kgem, &bo->base);
		if (bo->mem != NULL)
			return bo;

		bo->base.refcnt = 0; /* for valgrind */
		kgem_bo_free(kgem, &bo->base);
	}

	if (kgem->has_caching) {
		struct kgem_bo *old;

		bo = buffer_alloc();
		if (bo == NULL)
			return NULL;

		old = search_linear_cache(kgem, alloc,
					 CREATE_INACTIVE | CREATE_CPU_MAP | CREATE_EXACT);
		if (old) {
			init_buffer_from_bo(bo, old);
		} else {
			handle = gem_create(kgem->fd, alloc);
			if (handle == 0) {
				free(bo);
				return NULL;
			}

			debug_alloc(kgem, alloc);
			__kgem_bo_init(&bo->base, handle, alloc);
			DBG(("%s: created CPU handle=%d for buffer, size %d\n",
			     __FUNCTION__, bo->base.handle, alloc));
		}

		assert(bo->base.refcnt == 1);
		assert(bo->mmapped == MMAPPED_CPU);
		assert(bo->need_io == false);

		if (!gem_set_caching(kgem->fd, bo->base.handle, SNOOPED))
			goto free_caching;

		bo->base.snoop = true;

		bo->mem = kgem_bo_map__cpu(kgem, &bo->base);
		if (bo->mem == NULL)
			goto free_caching;

		return bo;

free_caching:
		bo->base.refcnt = 0; /* for valgrind */
		kgem_bo_free(kgem, &bo->base);
	}

	if (kgem->has_userptr) {
		bo = buffer_alloc();
		if (bo == NULL)
			return NULL;

		//if (posix_memalign(&ptr, 64, ALIGN(size, 64)))
		if (posix_memalign(&bo->mem, PAGE_SIZE, alloc * PAGE_SIZE)) {
			free(bo);
			return NULL;
		}

		handle = gem_userptr(kgem->fd, bo->mem, alloc * PAGE_SIZE, false);
		if (handle == 0) {
			free(bo->mem);
			free(bo);
			return NULL;
		}

		debug_alloc(kgem, alloc);
		__kgem_bo_init(&bo->base, handle, alloc);
		DBG(("%s: created snoop handle=%d for buffer\n",
		     __FUNCTION__, bo->base.handle));

		assert(bo->mmapped == MMAPPED_CPU);
		assert(bo->need_io == false);

		bo->base.refcnt = 1;
		bo->base.snoop = true;
		bo->base.map__cpu = MAKE_USER_MAP(bo->mem);

		return bo;
	}

	return NULL;
}

struct kgem_bo *kgem_create_buffer(struct kgem *kgem,
				   uint32_t size, uint32_t flags,
				   void **ret)
{
	struct kgem_buffer *bo;
	unsigned offset, alloc;
	struct kgem_bo *old;

	DBG(("%s: size=%d, flags=%x [write?=%d, inplace?=%d, last?=%d]\n",
	     __FUNCTION__, size, flags,
	     !!(flags & KGEM_BUFFER_WRITE),
	     !!(flags & KGEM_BUFFER_INPLACE),
	     !!(flags & KGEM_BUFFER_LAST)));
	assert(size);
	/* we should never be asked to create anything TOO large */
	assert(size <= kgem->max_object_size);

#if !DBG_NO_UPLOAD_CACHE
	list_for_each_entry(bo, &kgem->batch_buffers, base.list) {
		assert(bo->base.io);
		assert(bo->base.refcnt >= 1);

		/* We can reuse any write buffer which we can fit */
		if (flags == KGEM_BUFFER_LAST &&
		    bo->write == KGEM_BUFFER_WRITE &&
		    bo->base.refcnt == 1 &&
		    bo->mmapped == MMAPPED_NONE &&
		    size <= bytes(&bo->base)) {
			DBG(("%s: reusing write buffer for read of %d bytes? used=%d, total=%d\n",
			     __FUNCTION__, size, bo->used, bytes(&bo->base)));
			gem_write__cachealigned(kgem->fd, bo->base.handle,
				  0, bo->used, bo->mem);
			kgem_buffer_release(kgem, bo);
			bo->need_io = 0;
			bo->write = 0;
			offset = 0;
			bo->used = size;
			goto done;
		}

		if (flags & KGEM_BUFFER_WRITE) {
			if ((bo->write & KGEM_BUFFER_WRITE) == 0 ||
			    (((bo->write & ~flags) & KGEM_BUFFER_INPLACE) &&
			     !bo->base.snoop)) {
				DBG(("%s: skip write %x buffer, need %x\n",
				     __FUNCTION__, bo->write, flags));
				continue;
			}
			assert(bo->mmapped || bo->need_io);
		} else {
			if (bo->write & KGEM_BUFFER_WRITE) {
				DBG(("%s: skip write %x buffer, need %x\n",
				     __FUNCTION__, bo->write, flags));
				continue;
			}
		}

		if (bo->used + size <= bytes(&bo->base)) {
			DBG(("%s: reusing buffer? used=%d + size=%d, total=%d\n",
			     __FUNCTION__, bo->used, size, bytes(&bo->base)));
			offset = bo->used;
			bo->used += size;
			goto done;
		}
	}

	if (flags & KGEM_BUFFER_WRITE) {
		list_for_each_entry(bo, &kgem->active_buffers, base.list) {
			assert(bo->base.io);
			assert(bo->base.refcnt >= 1);
			assert(bo->base.exec == NULL);
			assert(bo->mmapped);
			assert(bo->mmapped == MMAPPED_GTT || kgem->has_llc || bo->base.snoop);

			if ((bo->write & ~flags) & KGEM_BUFFER_INPLACE && !bo->base.snoop) {
				DBG(("%s: skip write %x buffer, need %x\n",
				     __FUNCTION__, bo->write, flags));
				continue;
			}

			if (bo->used + size <= bytes(&bo->base)) {
				DBG(("%s: reusing buffer? used=%d + size=%d, total=%d\n",
				     __FUNCTION__, bo->used, size, bytes(&bo->base)));
				offset = bo->used;
				bo->used += size;
				list_move(&bo->base.list, &kgem->batch_buffers);
				goto done;
			}

			if (size <= bytes(&bo->base) &&
			    (bo->base.rq == NULL ||
			     !__kgem_busy(kgem, bo->base.handle))) {
				DBG(("%s: reusing whole buffer? size=%d, total=%d\n",
				     __FUNCTION__, size, bytes(&bo->base)));
				__kgem_bo_clear_busy(&bo->base);
				kgem_buffer_release(kgem, bo);

				switch (bo->mmapped) {
				case MMAPPED_CPU:
					kgem_bo_sync__cpu(kgem, &bo->base);
					break;
				case MMAPPED_GTT:
					kgem_bo_sync__gtt(kgem, &bo->base);
					break;
				}

				offset = 0;
				bo->used = size;
				list_move(&bo->base.list, &kgem->batch_buffers);
				goto done;
			}
		}
	}
#endif

#if !DBG_NO_MAP_UPLOAD
	/* Be a little more generous and hope to hold fewer mmappings */
	alloc = ALIGN(2*size, kgem->buffer_size);
	if (alloc > MAX_CACHE_SIZE)
		alloc = ALIGN(size, kgem->buffer_size);
	if (alloc > MAX_CACHE_SIZE)
		alloc = PAGE_ALIGN(size);
	assert(alloc);

	alloc /= PAGE_SIZE;
	if (alloc > kgem->aperture_mappable / 4)
		flags &= ~KGEM_BUFFER_INPLACE;

	if (kgem->has_llc &&
	    (flags & KGEM_BUFFER_WRITE_INPLACE) != KGEM_BUFFER_WRITE_INPLACE) {
		bo = buffer_alloc();
		if (bo == NULL)
			goto skip_llc;

		old = NULL;
		if ((flags & KGEM_BUFFER_WRITE) == 0)
			old = search_linear_cache(kgem, alloc, CREATE_CPU_MAP);
		if (old == NULL)
			old = search_linear_cache(kgem, alloc, CREATE_INACTIVE | CREATE_CPU_MAP);
		if (old == NULL)
			old = search_linear_cache(kgem, NUM_PAGES(size), CREATE_INACTIVE | CREATE_CPU_MAP);
		if (old) {
			DBG(("%s: found LLC handle=%d for buffer\n",
			     __FUNCTION__, old->handle));

			init_buffer_from_bo(bo, old);
		} else {
			uint32_t handle = gem_create(kgem->fd, alloc);
			if (handle == 0) {
				free(bo);
				goto skip_llc;
			}
			__kgem_bo_init(&bo->base, handle, alloc);
			DBG(("%s: created LLC handle=%d for buffer\n",
			     __FUNCTION__, bo->base.handle));

			debug_alloc(kgem, alloc);
		}

		assert(bo->mmapped);
		assert(!bo->need_io);

		bo->mem = kgem_bo_map__cpu(kgem, &bo->base);
		if (bo->mem) {
			if (flags & KGEM_BUFFER_WRITE)
				kgem_bo_sync__cpu(kgem, &bo->base);
			flags &= ~KGEM_BUFFER_INPLACE;
			goto init;
		} else {
			bo->base.refcnt = 0; /* for valgrind */
			kgem_bo_free(kgem, &bo->base);
		}
	}
skip_llc:

	if ((flags & KGEM_BUFFER_WRITE_INPLACE) == KGEM_BUFFER_WRITE_INPLACE) {
		/* The issue with using a GTT upload buffer is that we may
		 * cause eviction-stalls in order to free up some GTT space.
		 * An is-mappable? ioctl could help us detect when we are
		 * about to block, or some per-page magic in the kernel.
		 *
		 * XXX This is especially noticeable on memory constrained
		 * devices like gen2 or with relatively slow gpu like i3.
		 */
		DBG(("%s: searching for an inactive GTT map for upload\n",
		     __FUNCTION__));
		old = search_linear_cache(kgem, alloc,
					  CREATE_EXACT | CREATE_INACTIVE | CREATE_GTT_MAP);
#if HAVE_I915_GEM_BUFFER_INFO
		if (old) {
			struct drm_i915_gem_buffer_info info;

			/* An example of such a non-blocking ioctl might work */

			VG_CLEAR(info);
			info.handle = handle;
			if (drmIoctl(kgem->fd,
				     DRM_IOCTL_I915_GEM_BUFFER_INFO,
				     &fino) == 0) {
				old->presumed_offset = info.addr;
				if ((info.flags & I915_GEM_MAPPABLE) == 0) {
					kgem_bo_move_to_inactive(kgem, old);
					old = NULL;
				}
			}
		}
#endif
		if (old == NULL)
			old = search_linear_cache(kgem, NUM_PAGES(size),
						  CREATE_EXACT | CREATE_INACTIVE | CREATE_GTT_MAP);
		if (old == NULL) {
			old = search_linear_cache(kgem, alloc, CREATE_INACTIVE);
			if (old && !kgem_bo_can_map(kgem, old)) {
				_kgem_bo_destroy(kgem, old);
				old = NULL;
			}
		}
		if (old) {
			DBG(("%s: reusing handle=%d for buffer\n",
			     __FUNCTION__, old->handle));
			assert(kgem_bo_can_map(kgem, old));
			assert(!old->snoop);
			assert(old->rq == NULL);

			bo = buffer_alloc();
			if (bo == NULL)
				return NULL;

			init_buffer_from_bo(bo, old);
			assert(num_pages(&bo->base) >= NUM_PAGES(size));

			assert(bo->mmapped);
			assert(bo->base.refcnt == 1);

			bo->mem = kgem_bo_map(kgem, &bo->base);
			if (bo->mem) {
				if (bo->mem == MAP(bo->base.map__cpu))
					flags &= ~KGEM_BUFFER_INPLACE;
				else
					bo->mmapped = MMAPPED_GTT;
				goto init;
			} else {
				bo->base.refcnt = 0;
				kgem_bo_free(kgem, &bo->base);
			}
		}
	}
#else
	flags &= ~KGEM_BUFFER_INPLACE;
#endif
	/* Be more parsimonious with pwrite/pread/cacheable buffers */
	if ((flags & KGEM_BUFFER_INPLACE) == 0)
		alloc = NUM_PAGES(size);

	if (use_snoopable_buffer(kgem, flags)) {
		bo = search_snoopable_buffer(kgem, alloc);
		if (bo) {
			if (flags & KGEM_BUFFER_WRITE)
				kgem_bo_sync__cpu(kgem, &bo->base);
			flags &= ~KGEM_BUFFER_INPLACE;
			goto init;
		}

		if ((flags & KGEM_BUFFER_INPLACE) == 0) {
			bo = create_snoopable_buffer(kgem, alloc);
			if (bo)
				goto init;
		}
	}

	flags &= ~KGEM_BUFFER_INPLACE;

	old = NULL;
	if ((flags & KGEM_BUFFER_WRITE) == 0)
		old = search_linear_cache(kgem, alloc, 0);
	if (old == NULL)
		old = search_linear_cache(kgem, alloc, CREATE_INACTIVE);
	if (old) {
		DBG(("%s: reusing ordinary handle %d for io\n",
		     __FUNCTION__, old->handle));
		bo = buffer_alloc_with_data(num_pages(old));
		if (bo == NULL)
			return NULL;

		init_buffer_from_bo(bo, old);
		bo->need_io = flags & KGEM_BUFFER_WRITE;
	} else {
		unsigned hint;

		if (use_snoopable_buffer(kgem, flags)) {
			bo = create_snoopable_buffer(kgem, alloc);
			if (bo)
				goto init;
		}

		bo = buffer_alloc();
		if (bo == NULL)
			return NULL;

		hint = CREATE_INACTIVE;
		if (flags & KGEM_BUFFER_WRITE)
			hint |= CREATE_CPU_MAP;
		old = search_linear_cache(kgem, alloc, hint);
		if (old) {
			DBG(("%s: reusing handle=%d for buffer\n",
			     __FUNCTION__, old->handle));

			init_buffer_from_bo(bo, old);
		} else {
			uint32_t handle = gem_create(kgem->fd, alloc);
			if (handle == 0) {
				free(bo);
				return NULL;
			}

			DBG(("%s: created handle=%d for buffer\n",
			     __FUNCTION__, handle));

			__kgem_bo_init(&bo->base, handle, alloc);
			debug_alloc(kgem, alloc * PAGE_SIZE);
		}

		assert(bo->mmapped);
		assert(!bo->need_io);
		assert(bo->base.refcnt == 1);

		if (flags & KGEM_BUFFER_WRITE) {
			bo->mem = kgem_bo_map__cpu(kgem, &bo->base);
			if (bo->mem != NULL) {
				kgem_bo_sync__cpu(kgem, &bo->base);
				goto init;
			}
		}

		DBG(("%s: failing back to new pwrite buffer\n", __FUNCTION__));
		old = &bo->base;
		bo = buffer_alloc_with_data(num_pages(old));
		if (bo == NULL) {
			old->refcnt= 0;
			kgem_bo_free(kgem, old);
			return NULL;
		}

		init_buffer_from_bo(bo, old);

		assert(bo->mem);
		assert(!bo->mmapped);
		assert(bo->base.refcnt == 1);

		bo->need_io = flags & KGEM_BUFFER_WRITE;
	}
init:
	bo->base.io = true;
	assert(bo->base.refcnt == 1);
	assert(num_pages(&bo->base) >= NUM_PAGES(size));
	assert(!bo->need_io || !bo->base.needs_flush);
	assert(!bo->need_io || bo->base.domain != DOMAIN_GPU);
	assert(bo->mem);
	assert(bo->mmapped != MMAPPED_GTT || MAP(bo->base.map__gtt) == bo->mem);
	assert(bo->mmapped != MMAPPED_CPU || MAP(bo->base.map__cpu) == bo->mem);

	bo->used = size;
	bo->write = flags & KGEM_BUFFER_WRITE_INPLACE;
	offset = 0;

	assert(list_is_empty(&bo->base.list));
	list_add(&bo->base.list, &kgem->batch_buffers);

	DBG(("%s(pages=%d [%d]) new handle=%d, used=%d, write=%d\n",
	     __FUNCTION__, num_pages(&bo->base), alloc, bo->base.handle, bo->used, bo->write));

done:
	bo->used = ALIGN(bo->used, UPLOAD_ALIGNMENT);
	assert(bo->used && bo->used <= bytes(&bo->base));
	assert(bo->mem);
	*ret = (char *)bo->mem + offset;
	return kgem_create_proxy(kgem, &bo->base, offset, size);
}

bool kgem_buffer_is_inplace(struct kgem_bo *_bo)
{
	struct kgem_buffer *bo = (struct kgem_buffer *)_bo->proxy;
	return bo->write & KGEM_BUFFER_WRITE_INPLACE;
}

struct kgem_bo *kgem_create_buffer_2d(struct kgem *kgem,
				      int width, int height, int bpp,
				      uint32_t flags,
				      void **ret)
{
	struct kgem_bo *bo;
	int stride;

	assert(width > 0 && height > 0);
	assert(ret != NULL);
	stride = ALIGN(width, 2) * bpp >> 3;
	stride = ALIGN(stride, 4);

	DBG(("%s: %dx%d, %d bpp, stride=%d\n",
	     __FUNCTION__, width, height, bpp, stride));

	bo = kgem_create_buffer(kgem, stride * ALIGN(height, 2), flags, ret);
	if (bo == NULL) {
		DBG(("%s: allocation failure for upload buffer\n",
		     __FUNCTION__));
		return NULL;
	}
	assert(*ret != NULL);
	assert(bo->proxy != NULL);

	if (height & 1) {
		struct kgem_buffer *io = (struct kgem_buffer *)bo->proxy;
		int min;

		assert(io->used);

		/* Having padded this surface to ensure that accesses to
		 * the last pair of rows is valid, remove the padding so
		 * that it can be allocated to other pixmaps.
		 */
		min = bo->delta + height * stride;
		min = ALIGN(min, UPLOAD_ALIGNMENT);
		if (io->used != min) {
			DBG(("%s: trimming buffer from %d to %d\n",
			     __FUNCTION__, io->used, min));
			io->used = min;
		}
		bo->size.bytes -= stride;
	}

	bo->map__cpu = *ret;
	bo->pitch = stride;
	bo->unique_id = kgem_get_unique_id(kgem);
	return bo;
}

struct kgem_bo *kgem_upload_source_image(struct kgem *kgem,
					 const void *data,
					 const BoxRec *box,
					 int stride, int bpp)
{
	int width  = box->x2 - box->x1;
	int height = box->y2 - box->y1;
	struct kgem_bo *bo;
	void *dst;

	if (!kgem_can_create_2d(kgem, width, height, bpp))
		return NULL;

	DBG(("%s : (%d, %d), (%d, %d), stride=%d, bpp=%d\n",
	     __FUNCTION__, box->x1, box->y1, box->x2, box->y2, stride, bpp));

	assert(data);
	assert(width > 0);
	assert(height > 0);
	assert(stride);
	assert(bpp);

	bo = kgem_create_buffer_2d(kgem,
				   width, height, bpp,
				   KGEM_BUFFER_WRITE_INPLACE, &dst);
	if (bo)
		memcpy_blt(data, dst, bpp,
			   stride, bo->pitch,
			   box->x1, box->y1,
			   0, 0,
			   width, height);

	return bo;
}

void kgem_proxy_bo_attach(struct kgem_bo *bo,
			  struct kgem_bo **ptr)
{
	DBG(("%s: handle=%d\n", __FUNCTION__, bo->handle));
	assert(bo->map__gtt == NULL);
	assert(bo->proxy);
	list_add(&bo->vma, &bo->proxy->vma);
	bo->map__gtt = ptr;
	*ptr = kgem_bo_reference(bo);
}

void kgem_buffer_read_sync(struct kgem *kgem, struct kgem_bo *_bo)
{
	struct kgem_buffer *bo;
	uint32_t offset = _bo->delta, length = _bo->size.bytes;

	/* We expect the caller to have already submitted the batch */
	assert(_bo->io);
	assert(_bo->exec == NULL);
	assert(_bo->rq == NULL);
	assert(_bo->proxy);

	_bo = _bo->proxy;
	assert(_bo->proxy == NULL);
	assert(_bo->exec == NULL);

	bo = (struct kgem_buffer *)_bo;

	DBG(("%s(offset=%d, length=%d, snooped=%d)\n", __FUNCTION__,
	     offset, length, bo->base.snoop));

	if (bo->mmapped) {
		struct drm_i915_gem_set_domain set_domain;

		DBG(("%s: sync: needs_flush? %d, domain? %d, busy? %d\n",
		     __FUNCTION__,
		     bo->base.needs_flush,
		     bo->base.domain,
		     __kgem_busy(kgem, bo->base.handle)));

		assert(bo->mmapped == MMAPPED_GTT || bo->base.snoop || kgem->has_llc);

		VG_CLEAR(set_domain);
		set_domain.handle = bo->base.handle;
		set_domain.write_domain = 0;
		set_domain.read_domains =
			bo->mmapped == MMAPPED_CPU ? I915_GEM_DOMAIN_CPU : I915_GEM_DOMAIN_GTT;

		if (drmIoctl(kgem->fd,
			     DRM_IOCTL_I915_GEM_SET_DOMAIN, &set_domain))
			return;
	} else {
		if (gem_read(kgem->fd,
			     bo->base.handle, (char *)bo->mem+offset,
			     offset, length))
			return;
	}
	kgem_bo_retire(kgem, &bo->base);
	bo->base.domain = DOMAIN_NONE;
}
#endif

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

int kgem_init_fb(struct kgem *kgem, struct sna_fb *fb)
{
    struct kgem_bo *bo;
	struct drm_gem_open open_arg;
	struct drm_i915_gem_get_tiling get_tiling;

    size_t size;
    int ret;

	ret = drmIoctl(kgem->fd, SRV_FBINFO, fb);
	if( ret != 0 )
	    return 0;

	open_arg.name = fb->name;
	ret = drmIoctl(kgem->fd, DRM_IOCTL_GEM_OPEN, &open_arg);
	if (ret != 0) {
		printf("Couldn't reference %s handle 0x%08x\n",
		    fb->name, fb->name);
		return NULL;
	}
	size = open_arg.size / PAGE_SIZE;

  	bo = __kgem_bo_alloc(open_arg.handle, size);
	if (!bo) {
		return 0;
	}

	get_tiling.handle = bo->handle;
	ret = drmIoctl(kgem->fd,DRM_IOCTL_I915_GEM_GET_TILING,&get_tiling);
	if (ret != 0) {
        printf("%s: couldn't get tiling for handle %d\n", __FUNCTION__, bo->handle);
//		drm_intel_gem_bo_unreference(&bo_gem->bo);
		return 0;
	}

	bo->domain    = DOMAIN_GTT;
	bo->unique_id = kgem_get_unique_id(kgem);
	bo->pitch     = fb->pitch;
    bo->tiling    = get_tiling.tiling_mode;
    bo->scanout   = 1;
	fb->fb_bo     = bo;

    printf("fb handle %d w: %d h: %d pitch %d tilng %d bo %p\n",
            bo->handle, fb->width, fb->height, fb->pitch, fb->tiling, fb->fb_bo);

    return 1;
};


int kgem_update_fb(struct kgem *kgem, struct sna_fb *fb)
{
    struct kgem_bo *bo;
    size_t size;
    int ret;

    bo = fb->fb_bo;

	ret = drmIoctl(kgem->fd, SRV_FBINFO, fb);
	if( ret != 0 )
	    return 0;

	fb->fb_bo = bo;

    size = fb->pitch * fb->height / PAGE_SIZE;

    if((size != bo->size.pages.count) ||
       (fb->pitch != bo->pitch))
    {
        bo->size.pages.count = size;
	    bo->pitch     = fb->pitch;

    printf("fb width %d height %d pitch %d bo %p\n",
            fb->width, fb->height, fb->pitch, fb->fb_bo);

        return 1;
    }

    return 0;
};

void sna_bo_destroy(struct kgem *kgem, struct kgem_bo *bo)
{
    kgem_bo_destroy(kgem, bo);
    kgem_bo_free(kgem, bo);
}


void kgem_close_batches(struct kgem *kgem)
{
    int n;
	for (n = 0; n < ARRAY_SIZE(kgem->pinned_batches); n++) {
		while (!list_is_empty(&kgem->pinned_batches[n]))
        {
			struct kgem_bo *bo =
					list_first_entry(&kgem->pinned_batches[n],
							 struct kgem_bo, list);
			list_del(&bo->list);
			kgem_bo_destroy(kgem,bo);
		}
	}
};

struct kgem_bo *kgem_bo_from_handle(struct kgem *kgem, int handle,
                        int pitch, int height)
{
	struct kgem_bo *bo;
    int size;

    size = pitch * height / PAGE_SIZE;

  	bo = __kgem_bo_alloc(handle, size);
    if(bo == NULL)
        return NULL;

	bo->domain    = DOMAIN_GTT;
	bo->unique_id = kgem_get_unique_id(kgem);
	bo->pitch     = pitch;
    bo->tiling    = I915_TILING_X;
    bo->scanout   = 0;

    return bo;
}
