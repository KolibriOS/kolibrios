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

#include "compiler.h"
#include <linux/list.h>
//#include <stdarg.h>

#include <i915_drm.h>


#if DEBUG_KGEM
#define DBG_HDR(x) ErrorF x
#else
#define DBG_HDR(x)
#endif

struct kgem_bo {
	struct kgem_bo *proxy;

    struct list_head list;
    struct list_head request;
    struct list_head vma;

    void     *map;
    uint32_t  gaddr;

#define IS_CPU_MAP(ptr) ((uintptr_t)(ptr) & 1)
#define IS_GTT_MAP(ptr) (ptr && ((uintptr_t)(ptr) & 1) == 0)
	struct kgem_request *rq;
    struct drm_i915_gem_exec_object2 *exec;

	struct kgem_bo_binding {
		struct kgem_bo_binding *next;
		uint32_t format;
		uint16_t offset;
	} binding;

	uint32_t unique_id;
	uint32_t refcnt;
	uint32_t handle;
	uint32_t presumed_offset;
	uint32_t delta;
	union {
		struct {
			uint32_t count:27;
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
    uint32_t vmap   : 1;
    uint32_t io     : 1;
    uint32_t flush  : 1;
	uint32_t scanout : 1;
    uint32_t sync   : 1;
	uint32_t purged : 1;
};
#define DOMAIN_NONE 0
#define DOMAIN_CPU 1
#define DOMAIN_GTT 2
#define DOMAIN_GPU 3

struct kgem_request {
    struct list_head list;
	struct kgem_bo *bo;
    struct list_head buffers;
};

enum {
	MAP_GTT = 0,
	MAP_CPU,
	NUM_MAP_TYPES,
};

struct kgem {
	int fd;
	int wedged;
	int gen;

	uint32_t unique_id;

	enum kgem_mode {
		/* order matches I915_EXEC_RING ordering */
		KGEM_NONE = 0,
		KGEM_RENDER,
		KGEM_BSD,
		KGEM_BLT,
	} mode, ring;

    struct list_head flushing;
    struct list_head large;
    struct list_head active[NUM_CACHE_BUCKETS][3];
    struct list_head inactive[NUM_CACHE_BUCKETS];
    struct list_head partial;
    struct list_head requests;
	struct kgem_request *next_request;

	struct {
        struct list_head inactive[NUM_CACHE_BUCKETS];
		int16_t count;
	} vma[NUM_MAP_TYPES];

	uint16_t nbatch;
	uint16_t surface;
	uint16_t nexec;
	uint16_t nreloc;
	uint16_t nfence;
	uint16_t max_batch_size;

	uint32_t flush:1;
	uint32_t sync:1;
	uint32_t need_expire:1;
	uint32_t need_purge:1;
	uint32_t need_retire:1;
	uint32_t scanout:1;
	uint32_t flush_now:1;
	uint32_t busy:1;

	uint32_t has_vmap :1;
	uint32_t has_relaxed_fencing :1;
	uint32_t has_semaphores :1;
	uint32_t has_llc :1;
	uint32_t has_cpu_bo :1;

	uint16_t fence_max;
	uint16_t half_cpu_cache_pages;
	uint32_t aperture_total, aperture_high, aperture_low, aperture_mappable;
	uint32_t aperture, aperture_fenced;
	uint32_t min_alignment;
	uint32_t max_upload_tile_size, max_copy_tile_size;
	uint32_t max_gpu_size, max_cpu_size;
	uint32_t large_object_size, max_object_size;
	uint32_t partial_buffer_size;

//   void (*context_switch)(struct kgem *kgem, int new_mode);
    void (*retire)(struct kgem *kgem);

    uint32_t *batch;
    uint32_t *batch_ptr;
    int       batch_idx;
    struct drm_i915_gem_object *batch_obj;

    struct drm_i915_gem_exec_object2 exec[256];
    struct drm_i915_gem_relocation_entry reloc[384];
};

typedef struct
{
    struct drm_i915_gem_object *batch;
    struct list_head  objects;
    u32    exec_start;
    u32    exec_len;

}batchbuffer_t;

#define KGEM_BATCH_RESERVED 1
#define KGEM_RELOC_RESERVED 4
#define KGEM_EXEC_RESERVED 1

#define KGEM_BATCH_SIZE(K) ((K)->max_batch_size-KGEM_BATCH_RESERVED)
#define KGEM_EXEC_SIZE(K) (int)(ARRAY_SIZE((K)->exec)-KGEM_EXEC_RESERVED)
#define KGEM_RELOC_SIZE(K) (int)(ARRAY_SIZE((K)->reloc)-KGEM_RELOC_RESERVED)

void kgem_init(struct kgem *kgem, int gen);
void kgem_reset(struct kgem *kgem);

struct kgem_bo *kgem_create_map(struct kgem *kgem,
				void *ptr, uint32_t size,
				bool read_only);

struct kgem_bo *kgem_create_for_name(struct kgem *kgem, uint32_t name);

struct kgem_bo *kgem_create_linear(struct kgem *kgem, int size);
struct kgem_bo *kgem_create_proxy(struct kgem_bo *target,
				  int offset, int length);

//struct kgem_bo *kgem_upload_source_image(struct kgem *kgem,
//                    const void *data,
//                    BoxPtr box,
//                    int stride, int bpp);

int kgem_choose_tiling(struct kgem *kgem,
		       int tiling, int width, int height, int bpp);
unsigned kgem_can_create_2d(struct kgem *kgem, int width, int height, int depth);
#define KGEM_CAN_CREATE_GPU     0x1
#define KGEM_CAN_CREATE_CPU     0x2
#define KGEM_CAN_CREATE_LARGE	0x4

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
};
struct kgem_bo *kgem_create_2d(struct kgem *kgem,
			       int width,
			       int height,
			       int bpp,
			       int tiling,
			       uint32_t flags);

uint32_t kgem_bo_get_binding(struct kgem_bo *bo, uint32_t format);
void kgem_bo_set_binding(struct kgem_bo *bo, uint32_t format, uint16_t offset);

bool kgem_retire(struct kgem *kgem);

void _kgem_submit(struct kgem *kgem, batchbuffer_t *exb);
//static inline void kgem_submit(struct kgem *kgem)
//{
//   if (kgem->nbatch)
//       _kgem_submit(kgem);
//}

/*
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

	__kgem_flush(kgem, bo);

	bo->needs_flush = false;
}
*/
static inline struct kgem_bo *kgem_bo_reference(struct kgem_bo *bo)
{
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

static inline void kgem_set_mode(struct kgem *kgem, enum kgem_mode mode)
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
//   kgem->context_switch(kgem, mode);
	kgem->mode = mode;
}

static inline bool kgem_check_batch(struct kgem *kgem, int num_dwords)
{
	return likely(kgem->nbatch + num_dwords + KGEM_BATCH_RESERVED <= kgem->surface);
}

static inline bool kgem_check_reloc(struct kgem *kgem, int n)
{
	return likely(kgem->nreloc + n <= KGEM_RELOC_SIZE(kgem));
}

static inline bool kgem_check_exec(struct kgem *kgem, int n)
{
	return likely(kgem->nexec + n <= KGEM_EXEC_SIZE(kgem));
}

static inline bool kgem_check_batch_with_surfaces(struct kgem *kgem,
						  int num_dwords,
						  int num_surfaces)
{
	return (int)(kgem->nbatch + num_dwords + KGEM_BATCH_RESERVED) <= (int)(kgem->surface - num_surfaces*8) &&
		kgem_check_reloc(kgem, num_surfaces);
}

static inline uint32_t *kgem_get_batch(struct kgem *kgem, int num_dwords)
{
//   if (!kgem_check_batch(kgem, num_dwords))
//       _kgem_submit(kgem);

	return kgem->batch + kgem->nbatch;
}

static inline void kgem_advance_batch(struct kgem *kgem, int num_dwords)
{
	kgem->nbatch += num_dwords;
}

bool kgem_check_bo(struct kgem *kgem, ...) __attribute__((sentinel(0)));
bool kgem_check_bo_fenced(struct kgem *kgem, ...) __attribute__((sentinel(0)));

void _kgem_add_bo(struct kgem *kgem, struct kgem_bo *bo);
static inline void kgem_add_bo(struct kgem *kgem, struct kgem_bo *bo)
{
	if (bo->proxy)
		bo = bo->proxy;

	if (bo->exec == NULL)
		_kgem_add_bo(kgem, bo);
}

#define KGEM_RELOC_FENCED 0x8000
uint32_t kgem_add_reloc(struct kgem *kgem,
			uint32_t pos,
			struct kgem_bo *bo,
			uint32_t read_write_domains,
			uint32_t delta);

void *kgem_bo_map(struct kgem *kgem, struct kgem_bo *bo);
void *kgem_bo_map__debug(struct kgem *kgem, struct kgem_bo *bo);
void *kgem_bo_map__cpu(struct kgem *kgem, struct kgem_bo *bo);
void kgem_bo_sync__cpu(struct kgem *kgem, struct kgem_bo *bo);
uint32_t kgem_bo_flink(struct kgem *kgem, struct kgem_bo *bo);

Bool kgem_bo_write(struct kgem *kgem, struct kgem_bo *bo,
		   const void *data, int length);

int kgem_bo_fenced_size(struct kgem *kgem, struct kgem_bo *bo);
void kgem_get_tile_size(struct kgem *kgem, int tiling,
			int *tile_width, int *tile_height, int *tile_size);

static inline int kgem_bo_size(struct kgem_bo *bo)
{
	assert(!(bo->proxy && bo->io));
	return PAGE_SIZE * bo->size.pages.count;
}

static inline int kgem_buffer_size(struct kgem_bo *bo)
{
	assert(bo->proxy && bo->io);
	return bo->size.bytes;
}

/*
static inline bool kgem_bo_blt_pitch_is_ok(struct kgem *kgem,
					   struct kgem_bo *bo)
{
	int pitch = bo->pitch;
	if (kgem->gen >= 40 && bo->tiling)
		pitch /= 4;
	if (pitch > MAXSHORT) {
		DBG(("%s: can not blt to handle=%d, adjusted pitch=%d\n",
		     __FUNCTION__, pitch));
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
static inline bool kgem_bo_is_mappable(struct kgem *kgem,
				       struct kgem_bo *bo)
{
	DBG_HDR(("%s: domain=%d, offset: %d size: %d\n",
		 __FUNCTION__, bo->domain, bo->presumed_offset, kgem_bo_size(bo)));

	if (bo->domain == DOMAIN_GTT)
		return true;

	if (IS_GTT_MAP(bo->map))
		return true;

	if (kgem->gen < 40 && bo->tiling &&
	    bo->presumed_offset & (kgem_bo_fenced_size(kgem, bo) - 1))
		return false;

	if (!bo->presumed_offset)
		return kgem_bo_size(bo) <= kgem->aperture_mappable / 4;

	return bo->presumed_offset + kgem_bo_size(bo) <= kgem->aperture_mappable;
}

static inline bool kgem_bo_mapped(struct kgem_bo *bo)
{
	DBG_HDR(("%s: map=%p, tiling=%d\n", __FUNCTION__, bo->map, bo->tiling));

	if (bo->map == NULL)
		return false;

	return IS_CPU_MAP(bo->map) == !bo->tiling;
}

static inline bool kgem_bo_is_busy(struct kgem_bo *bo)
{
	DBG_HDR(("%s: domain: %d exec? %d, rq? %d\n",
		 __FUNCTION__, bo->domain, bo->exec != NULL, bo->rq != NULL));
	assert(bo->proxy == NULL);
	return bo->rq;
}
/*
static inline bool kgem_bo_map_will_stall(struct kgem *kgem, struct kgem_bo *bo)
{
	DBG(("%s? handle=%d, domain=%d, offset=%x, size=%x\n",
	     __FUNCTION__, bo->handle,
	     bo->domain, bo->presumed_offset, bo->size));

	if (!kgem_bo_is_mappable(kgem, bo))
		return true;

	if (kgem->wedged)
		return false;

	if (kgem_bo_is_busy(bo))
		return true;

	if (bo->presumed_offset == 0)
		return !list_is_empty(&kgem->requests);

	return false;
}
*/

static inline bool kgem_bo_is_dirty(struct kgem_bo *bo)
{
	if (bo == NULL)
		return FALSE;

	return bo->dirty;
}

static inline void kgem_bo_mark_dirty(struct kgem_bo *bo)
{
	DBG_HDR(("%s: handle=%d\n", __FUNCTION__, bo->handle));
	bo->dirty = true;
}

void kgem_sync(struct kgem *kgem);

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
void kgem_buffer_read_sync(struct kgem *kgem, struct kgem_bo *bo);

void kgem_throttle(struct kgem *kgem);
#define MAX_INACTIVE_TIME 10
bool kgem_expire_cache(struct kgem *kgem);
void kgem_purge_cache(struct kgem *kgem);
void kgem_cleanup_cache(struct kgem *kgem);

#if HAS_EXTRA_DEBUG
void __kgem_batch_debug(struct kgem *kgem, uint32_t nbatch);
#else
static inline void __kgem_batch_debug(struct kgem *kgem, uint32_t nbatch)
{
	(void)kgem;
	(void)nbatch;
}
#endif

#undef DBG_HDR

u32 get_buffer_offset(uint32_t handle);


#endif /* KGEM_H */
