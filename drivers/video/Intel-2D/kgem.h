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

#ifndef KGEM_H
#define KGEM_H

#define HAS_DEBUG_FULL 0

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

#include <i915_drm.h>

#include "compiler.h"
#include "intel_list.h"

static inline void delay(uint32_t time)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(5), "b"(time)
    :"memory");
};

#undef  DBG

#if HAS_DEBUG_FULL
#define DBG(x) printf x
#else
#define DBG(x)
#endif

struct kgem_bo {
	struct kgem_request *rq;
#define RQ(rq) ((struct kgem_request *)((uintptr_t)(rq) & ~3))
#define RQ_RING(rq) ((uintptr_t)(rq) & 3)
#define RQ_IS_BLT(rq) (RQ_RING(rq) == KGEM_BLT)
	struct drm_i915_gem_exec_object2 *exec;

	struct kgem_bo *proxy;

	struct list list;
	struct list request;
	struct list vma;

    void     *map;
#define IS_CPU_MAP(ptr) ((uintptr_t)(ptr) & 1)
#define IS_GTT_MAP(ptr) (ptr && ((uintptr_t)(ptr) & 1) == 0)

	struct kgem_bo_binding {
		struct kgem_bo_binding *next;
		uint32_t format;
		uint16_t offset;
	} binding;

	uint32_t unique_id;
	uint32_t refcnt;
	uint32_t handle;
	uint32_t target_handle;
	uint32_t presumed_offset;
	uint32_t delta;
	union {
		struct {
			uint32_t count:27;
#define PAGE_SIZE 4096
            uint32_t bucket:5;
#define NUM_CACHE_BUCKETS 16
#define MAX_CACHE_SIZE (1 << (NUM_CACHE_BUCKETS+12))
		} pages;
		uint32_t bytes;
	} size;
    uint32_t pitch  : 18; /* max 128k */
	uint32_t tiling : 2;
	uint32_t reusable : 1;
    uint32_t dirty  : 1;
	uint32_t domain : 2;
	uint32_t needs_flush : 1;
	uint32_t snoop : 1;
    uint32_t io     : 1;
    uint32_t flush  : 1;
	uint32_t scanout : 1;
	uint32_t purged : 1;
};
#define DOMAIN_NONE 0
#define DOMAIN_CPU 1
#define DOMAIN_GTT 2
#define DOMAIN_GPU 3

struct kgem_request {
	struct list list;
	struct kgem_bo *bo;
	struct list buffers;
	int ring;
};

enum {
	MAP_GTT = 0,
	MAP_CPU,
	NUM_MAP_TYPES,
};

struct kgem {
	int fd;
	int wedged;
	unsigned gen;

	uint32_t unique_id;

	enum kgem_mode {
		/* order matches I915_EXEC_RING ordering */
		KGEM_NONE = 0,
		KGEM_RENDER,
		KGEM_BSD,
		KGEM_BLT,
	} mode, ring;

	struct list flushing;
	struct list large;
	struct list large_inactive;
	struct list active[NUM_CACHE_BUCKETS][3];
	struct list inactive[NUM_CACHE_BUCKETS];
	struct list pinned_batches[2];
	struct list snoop;
	struct list scanout;
	struct list batch_buffers, active_buffers;

	struct list requests[2];
	struct kgem_request *next_request;
	struct kgem_request static_request;

	struct {
		struct list inactive[NUM_CACHE_BUCKETS];
		int16_t count;
	} vma[NUM_MAP_TYPES];

	uint32_t batch_flags;
	uint32_t batch_flags_base;
#define I915_EXEC_SECURE (1<<9)
#define LOCAL_EXEC_OBJECT_WRITE (1<<2)

	uint16_t nbatch;
	uint16_t surface;
	uint16_t nexec;
	uint16_t nreloc;
	uint16_t nreloc__self;
	uint16_t nfence;
	uint16_t batch_size;
	uint16_t min_alignment;

	uint32_t flush:1;
	uint32_t need_expire:1;
	uint32_t need_purge:1;
	uint32_t need_retire:1;
	uint32_t need_throttle:1;
	uint32_t scanout_busy:1;
	uint32_t busy:1;

	uint32_t has_userptr :1;
	uint32_t has_blt :1;
	uint32_t has_relaxed_fencing :1;
	uint32_t has_relaxed_delta :1;
	uint32_t has_semaphores :1;
	uint32_t has_secure_batches :1;
	uint32_t has_pinned_batches :1;
	uint32_t has_cacheing :1;
	uint32_t has_llc :1;
	uint32_t has_no_reloc :1;
	uint32_t has_handle_lut :1;

	uint32_t can_blt_cpu :1;

	uint16_t fence_max;
	uint16_t half_cpu_cache_pages;
	uint32_t aperture_total, aperture_high, aperture_low, aperture_mappable;
	uint32_t aperture, aperture_fenced;
	uint32_t max_upload_tile_size, max_copy_tile_size;
	uint32_t max_gpu_size, max_cpu_size;
	uint32_t large_object_size, max_object_size;
	uint32_t buffer_size;

	void (*context_switch)(struct kgem *kgem, int new_mode);
    void (*retire)(struct kgem *kgem);
	void (*expire)(struct kgem *kgem);

	uint32_t batch[64*1024-8];
	struct drm_i915_gem_exec_object2 exec[256];
	struct drm_i915_gem_relocation_entry reloc[4096];
	uint16_t reloc__self[256];

#ifdef DEBUG_MEMORY
	struct {
		int bo_allocs;
		size_t bo_bytes;
	} debug_memory;
#endif
};

#define KGEM_BATCH_RESERVED 1
#define KGEM_RELOC_RESERVED 4
#define KGEM_EXEC_RESERVED 1

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

#define KGEM_BATCH_SIZE(K) ((K)->batch_size-KGEM_BATCH_RESERVED)
#define KGEM_EXEC_SIZE(K) (int)(ARRAY_SIZE((K)->exec)-KGEM_EXEC_RESERVED)
#define KGEM_RELOC_SIZE(K) (int)(ARRAY_SIZE((K)->reloc)-KGEM_RELOC_RESERVED)

void kgem_init(struct kgem *kgem, int fd, struct pci_device *dev, unsigned gen);
void kgem_reset(struct kgem *kgem);

struct kgem_bo *kgem_create_map(struct kgem *kgem,
				void *ptr, uint32_t size,
				bool read_only);

struct kgem_bo *kgem_create_for_name(struct kgem *kgem, uint32_t name);

struct kgem_bo *kgem_create_linear(struct kgem *kgem, int size, unsigned flags);
struct kgem_bo *kgem_create_proxy(struct kgem *kgem,
				  struct kgem_bo *target,
				  int offset, int length);


int kgem_choose_tiling(struct kgem *kgem,
		       int tiling, int width, int height, int bpp);
unsigned kgem_can_create_2d(struct kgem *kgem, int width, int height, int depth);
#define KGEM_CAN_CREATE_GPU     0x1
#define KGEM_CAN_CREATE_CPU     0x2
#define KGEM_CAN_CREATE_LARGE	0x4
#define KGEM_CAN_CREATE_GTT	0x8

struct kgem_bo *
kgem_replace_bo(struct kgem *kgem,
		struct kgem_bo *src,
		uint32_t width,
		uint32_t height,
		uint32_t pitch,
		uint32_t bpp);
enum {
	CREATE_EXACT = 0x1,
	CREATE_INACTIVE = 0x2,
	CREATE_CPU_MAP = 0x4,
	CREATE_GTT_MAP = 0x8,
	CREATE_SCANOUT = 0x10,
	CREATE_PRIME = 0x20,
	CREATE_TEMPORARY = 0x40,
	CREATE_CACHED = 0x80,
	CREATE_NO_RETIRE = 0x100,
	CREATE_NO_THROTTLE = 0x200,
};
struct kgem_bo *kgem_create_2d(struct kgem *kgem,
			       int width,
			       int height,
			       int bpp,
			       int tiling,
			       uint32_t flags);
struct kgem_bo *kgem_create_cpu_2d(struct kgem *kgem,
				   int width,
				   int height,
				   int bpp,
				   uint32_t flags);

uint32_t kgem_bo_get_binding(struct kgem_bo *bo, uint32_t format);
void kgem_bo_set_binding(struct kgem_bo *bo, uint32_t format, uint16_t offset);
int kgem_bo_get_swizzling(struct kgem *kgem, struct kgem_bo *bo);

bool kgem_retire(struct kgem *kgem);

bool __kgem_ring_is_idle(struct kgem *kgem, int ring);
static inline bool kgem_ring_is_idle(struct kgem *kgem, int ring)
{
	ring = ring == KGEM_BLT;

	if (list_is_empty(&kgem->requests[ring]))
		return true;

	return __kgem_ring_is_idle(kgem, ring);
}

static inline bool kgem_is_idle(struct kgem *kgem)
{
	if (!kgem->need_retire)
		return true;

	return kgem_ring_is_idle(kgem, kgem->ring);
}

void _kgem_submit(struct kgem *kgem);
static inline void kgem_submit(struct kgem *kgem)
{
	if (kgem->nbatch)
		_kgem_submit(kgem);
}

static inline bool kgem_flush(struct kgem *kgem, bool flush)
{
	if (kgem->nreloc == 0)
		return false;

	return (kgem->flush ^ flush) && kgem_ring_is_idle(kgem, kgem->ring);
}

static inline void kgem_bo_submit(struct kgem *kgem, struct kgem_bo *bo)
{
	if (bo->exec)
		_kgem_submit(kgem);
}

void __kgem_flush(struct kgem *kgem, struct kgem_bo *bo);
static inline void kgem_bo_flush(struct kgem *kgem, struct kgem_bo *bo)
{
	kgem_bo_submit(kgem, bo);

	if (!bo->needs_flush)
		return;

	/* If the kernel fails to emit the flush, then it will be forced when
	 * we assume direct access. And as the useual failure is EIO, we do
	 * not actualy care.
	 */
	__kgem_flush(kgem, bo);
}

static inline struct kgem_bo *kgem_bo_reference(struct kgem_bo *bo)
{
	assert(bo->refcnt);
	bo->refcnt++;
	return bo;
}

void _kgem_bo_destroy(struct kgem *kgem, struct kgem_bo *bo);
static inline void kgem_bo_destroy(struct kgem *kgem, struct kgem_bo *bo)
{
	assert(bo->refcnt);
	if (--bo->refcnt == 0)
		_kgem_bo_destroy(kgem, bo);
}

void kgem_clear_dirty(struct kgem *kgem);

static inline void kgem_set_mode(struct kgem *kgem,
				 enum kgem_mode mode,
				 struct kgem_bo *bo)
{
	assert(!kgem->wedged);

#if DEBUG_FLUSH_BATCH
	kgem_submit(kgem);
#endif

	if (kgem->mode == mode)
		return;

//   kgem->context_switch(kgem, mode);
	kgem->mode = mode;
}

static inline void _kgem_set_mode(struct kgem *kgem, enum kgem_mode mode)
{
	assert(kgem->mode == KGEM_NONE);
	assert(kgem->nbatch == 0);
	assert(!kgem->wedged);
//   kgem->context_switch(kgem, mode);
	kgem->mode = mode;
}

static inline bool kgem_check_batch(struct kgem *kgem, int num_dwords)
{
	assert(num_dwords > 0);
	assert(kgem->nbatch < kgem->surface);
	assert(kgem->surface <= kgem->batch_size);
	return likely(kgem->nbatch + num_dwords + KGEM_BATCH_RESERVED <= kgem->surface);
}

static inline bool kgem_check_reloc(struct kgem *kgem, int n)
{
	assert(kgem->nreloc <= KGEM_RELOC_SIZE(kgem));
	return likely(kgem->nreloc + n <= KGEM_RELOC_SIZE(kgem));
}

static inline bool kgem_check_exec(struct kgem *kgem, int n)
{
	assert(kgem->nexec <= KGEM_EXEC_SIZE(kgem));
	return likely(kgem->nexec + n <= KGEM_EXEC_SIZE(kgem));
}

static inline bool kgem_check_reloc_and_exec(struct kgem *kgem, int n)
{
	return kgem_check_reloc(kgem, n) && kgem_check_exec(kgem, n);
}

static inline bool kgem_check_batch_with_surfaces(struct kgem *kgem,
						  int num_dwords,
						  int num_surfaces)
{
	return (int)(kgem->nbatch + num_dwords + KGEM_BATCH_RESERVED) <= (int)(kgem->surface - num_surfaces*8) &&
		kgem_check_reloc(kgem, num_surfaces) &&
		kgem_check_exec(kgem, num_surfaces);
}

static inline uint32_t *kgem_get_batch(struct kgem *kgem)
{

	return kgem->batch + kgem->nbatch;
}

bool kgem_check_bo(struct kgem *kgem, ...) __attribute__((sentinel(0)));
bool kgem_check_bo_fenced(struct kgem *kgem, struct kgem_bo *bo);
bool kgem_check_many_bo_fenced(struct kgem *kgem, ...) __attribute__((sentinel(0)));

#define KGEM_RELOC_FENCED 0x8000
uint32_t kgem_add_reloc(struct kgem *kgem,
			uint32_t pos,
			struct kgem_bo *bo,
			uint32_t read_write_domains,
			uint32_t delta);

void *kgem_bo_map(struct kgem *kgem, struct kgem_bo *bo);
void *kgem_bo_map__async(struct kgem *kgem, struct kgem_bo *bo);
void *kgem_bo_map__gtt(struct kgem *kgem, struct kgem_bo *bo);
void kgem_bo_sync__gtt(struct kgem *kgem, struct kgem_bo *bo);
void *kgem_bo_map__debug(struct kgem *kgem, struct kgem_bo *bo);
void *kgem_bo_map__cpu(struct kgem *kgem, struct kgem_bo *bo);
void kgem_bo_sync__cpu(struct kgem *kgem, struct kgem_bo *bo);
void kgem_bo_sync__cpu_full(struct kgem *kgem, struct kgem_bo *bo, bool write);
void *__kgem_bo_map__cpu(struct kgem *kgem, struct kgem_bo *bo);
void __kgem_bo_unmap__cpu(struct kgem *kgem, struct kgem_bo *bo, void *ptr);
uint32_t kgem_bo_flink(struct kgem *kgem, struct kgem_bo *bo);

bool kgem_bo_write(struct kgem *kgem, struct kgem_bo *bo,
		   const void *data, int length);

int kgem_bo_fenced_size(struct kgem *kgem, struct kgem_bo *bo);
void kgem_get_tile_size(struct kgem *kgem, int tiling,
			int *tile_width, int *tile_height, int *tile_size);

static inline int __kgem_buffer_size(struct kgem_bo *bo)
{
	assert(bo->proxy != NULL);
	return bo->size.bytes;
}

static inline int __kgem_bo_size(struct kgem_bo *bo)
{
	assert(bo->proxy == NULL);
	return PAGE_SIZE * bo->size.pages.count;
}

static inline int kgem_bo_size(struct kgem_bo *bo)
{
	if (bo->proxy)
		return __kgem_buffer_size(bo);
	else
		return __kgem_bo_size(bo);
}

/*
static inline bool kgem_bo_blt_pitch_is_ok(struct kgem *kgem,
					   struct kgem_bo *bo)
{
	int pitch = bo->pitch;
	if (kgem->gen >= 040 && bo->tiling)
		pitch /= 4;
	if (pitch > MAXSHORT) {
		DBG(("%s: can not blt to handle=%d, adjusted pitch=%d\n",
		     __FUNCTION__, bo->handle, pitch));
		return false;
	}

	return true;
}

static inline bool kgem_bo_can_blt(struct kgem *kgem,
				   struct kgem_bo *bo)
{
	if (bo->tiling == I915_TILING_Y) {
		DBG(("%s: can not blt to handle=%d, tiling=Y\n",
		     __FUNCTION__, bo->handle));
		return false;
	}

	return kgem_bo_blt_pitch_is_ok(kgem, bo);
}
*/

static inline bool __kgem_bo_is_mappable(struct kgem *kgem,
				       struct kgem_bo *bo)
{
	if (bo->domain == DOMAIN_GTT)
		return true;

	if (kgem->gen < 040 && bo->tiling &&
	    bo->presumed_offset & (kgem_bo_fenced_size(kgem, bo) - 1))
		return false;

	if (!bo->presumed_offset)
		return kgem_bo_size(bo) <= kgem->aperture_mappable / 4;

	return bo->presumed_offset + kgem_bo_size(bo) <= kgem->aperture_mappable;
}

static inline bool kgem_bo_is_mappable(struct kgem *kgem,
				       struct kgem_bo *bo)
{
	DBG(("%s: domain=%d, offset: %d size: %d\n",
	     __FUNCTION__, bo->domain, bo->presumed_offset, kgem_bo_size(bo)));
	assert(bo->refcnt);
	return __kgem_bo_is_mappable(kgem, bo);
}

static inline bool kgem_bo_mapped(struct kgem *kgem, struct kgem_bo *bo)
{
	DBG(("%s: map=%p, tiling=%d, domain=%d\n",
	     __FUNCTION__, bo->map, bo->tiling, bo->domain));
	assert(bo->refcnt);

	if (bo->map == NULL)
		return bo->tiling == I915_TILING_NONE && bo->domain == DOMAIN_CPU;

	return IS_CPU_MAP(bo->map) == !bo->tiling;
}

static inline bool kgem_bo_can_map(struct kgem *kgem, struct kgem_bo *bo)
{
	if (kgem_bo_mapped(kgem, bo))
		return true;

	if (!bo->tiling && kgem->has_llc)
		return true;

	if (kgem->gen == 021 && bo->tiling == I915_TILING_Y)
		return false;

	return kgem_bo_size(bo) <= kgem->aperture_mappable / 4;
}

static inline bool kgem_bo_is_snoop(struct kgem_bo *bo)
{
	assert(bo->refcnt);
	while (bo->proxy)
		bo = bo->proxy;
	return bo->snoop;
}

bool __kgem_busy(struct kgem *kgem, int handle);

static inline void kgem_bo_mark_busy(struct kgem_bo *bo, int ring)
{
	bo->rq = (struct kgem_request *)((uintptr_t)bo->rq | ring);
}

inline static void __kgem_bo_clear_busy(struct kgem_bo *bo)
{
	bo->needs_flush = false;
	list_del(&bo->request);
	bo->rq = NULL;
	bo->domain = DOMAIN_NONE;
}

static inline bool kgem_bo_is_busy(struct kgem_bo *bo)
{
	DBG(("%s: handle=%d, domain: %d exec? %d, rq? %d\n", __FUNCTION__,
	     bo->handle, bo->domain, bo->exec != NULL, bo->rq != NULL));
	assert(bo->refcnt);
	return bo->rq;
}

/*

static inline bool __kgem_bo_is_busy(struct kgem *kgem, struct kgem_bo *bo)
{
	DBG(("%s: handle=%d, domain: %d exec? %d, rq? %d\n", __FUNCTION__,
	     bo->handle, bo->domain, bo->exec != NULL, bo->rq != NULL));
	assert(bo->refcnt);

	if (bo->exec)
		return true;

	if (kgem_flush(kgem, bo->flush))
		kgem_submit(kgem);

	if (bo->rq && !__kgem_busy(kgem, bo->handle))
		__kgem_bo_clear_busy(bo);

	return kgem_bo_is_busy(bo);
}

*/

static inline bool kgem_bo_is_dirty(struct kgem_bo *bo)
{
	if (bo == NULL)
		return false;

	assert(bo->refcnt);
	return bo->dirty;
}

static inline void kgem_bo_unclean(struct kgem *kgem, struct kgem_bo *bo)
{
	/* The bo is outside of our control, so presume it is written to */
	bo->needs_flush = true;
	if (bo->rq == NULL)
		bo->rq = (void *)kgem;

	if (bo->domain != DOMAIN_GPU)
		bo->domain = DOMAIN_NONE;
}

static inline void __kgem_bo_mark_dirty(struct kgem_bo *bo)
{
	DBG(("%s: handle=%d (proxy? %d)\n", __FUNCTION__,
	     bo->handle, bo->proxy != NULL));

	bo->exec->flags |= LOCAL_EXEC_OBJECT_WRITE;
	bo->needs_flush = bo->dirty = true;
	list_move(&bo->request, &RQ(bo->rq)->buffers);
}

static inline void kgem_bo_mark_dirty(struct kgem_bo *bo)
{
	assert(bo->refcnt);
	do {
		assert(bo->exec);
		assert(bo->rq);

		if (bo->dirty)
			return;

		__kgem_bo_mark_dirty(bo);
	} while ((bo = bo->proxy));
}

#define KGEM_BUFFER_WRITE	0x1
#define KGEM_BUFFER_INPLACE	0x2
#define KGEM_BUFFER_LAST	0x4

#define KGEM_BUFFER_WRITE_INPLACE (KGEM_BUFFER_WRITE | KGEM_BUFFER_INPLACE)

struct kgem_bo *kgem_create_buffer(struct kgem *kgem,
				   uint32_t size, uint32_t flags,
				   void **ret);
struct kgem_bo *kgem_create_buffer_2d(struct kgem *kgem,
				      int width, int height, int bpp,
				      uint32_t flags,
				      void **ret);
bool kgem_buffer_is_inplace(struct kgem_bo *bo);
void kgem_buffer_read_sync(struct kgem *kgem, struct kgem_bo *bo);

void kgem_throttle(struct kgem *kgem);
#define MAX_INACTIVE_TIME 10
bool kgem_expire_cache(struct kgem *kgem);
void kgem_purge_cache(struct kgem *kgem);
void kgem_cleanup_cache(struct kgem *kgem);

#if HAS_DEBUG_FULL
void __kgem_batch_debug(struct kgem *kgem, uint32_t nbatch);
#else
static inline void __kgem_batch_debug(struct kgem *kgem, uint32_t nbatch)
{
	(void)kgem;
	(void)nbatch;
}
#endif

#endif /* KGEM_H */
