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

#include <drmP.h>
#include <drm.h>
#include "i915_drm.h"
#include "i915_drv.h"
#include "intel_drv.h"

#include <linux/kernel.h>
#include "../bitmap.h"
#include "sna.h"
//#include "sna_reg.h"
//#include <time.h>
//#include <errno.h>

#define NO_CACHE 1

#define list_is_empty list_empty
#define list_init     INIT_LIST_HEAD

extern struct drm_device *main_device;

static struct kgem_bo *
search_linear_cache(struct kgem *kgem, unsigned int num_pages,
                    unsigned flags);

#define INT16_MAX              (32767)

#define PAGE_ALIGN(x) ALIGN(x, PAGE_SIZE)
#define NUM_PAGES(x) (((x) + PAGE_SIZE-1) / PAGE_SIZE)

#define MAX_GTT_VMA_CACHE 512
#define MAX_CPU_VMA_CACHE INT16_MAX
#define MAP_PRESERVE_TIME 10

#define CPU_MAP(ptr) ((void*)((uintptr_t)(ptr) & ~1))
#define MAKE_CPU_MAP(ptr) ((void*)((uintptr_t)(ptr) | 1))

struct kgem_partial_bo {
	struct kgem_bo base;
	void *mem;
	uint32_t used;
	uint32_t need_io : 1;
	uint32_t write : 2;
	uint32_t mmapped : 1;
};

static struct kgem_bo *__kgem_freed_bo;
static struct drm_i915_gem_exec_object2 _kgem_dummy_exec;

static inline int bytes(struct kgem_bo *bo)
{
	return kgem_bo_size(bo);
}

#define bucket(B) (B)->size.pages.bucket
#define num_pages(B) (B)->size.pages.count

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

	if (sna->render.solid_cache.dirty)
		sna_render_flush_solid(sna);
}

static int __gem_write(int fd, uint32_t handle,
               int offset, int length,
               const void *src)
{
    DBG(("%s(handle=%x, offset=%d, len=%d)\n", __FUNCTION__,
         handle, offset, length));

    write_gem_object(handle, offset, length, src);
    return 0;
}


static int gem_write(int fd, uint32_t handle,
             int offset, int length,
             const void *src)
{
    u32 _offset;
    u32 _size;
    u8  *data_ptr;

    DBG(("%s(handle=%x, offset=%d, len=%d)\n", __FUNCTION__,
         handle, offset, length));

    /* align the transfer to cachelines; fortuitously this is safe! */
    if ((offset | length) & 63) {
        _offset = offset & ~63;
        _size = ALIGN(offset+length, 64) - _offset;
        data_ptr = (u8*)src + _offset - offset;
    } else {
        _offset = offset;
        _size = length;
        data_ptr = (u8*)src;
    }

    write_gem_object(handle, _offset, _size, data_ptr);
    return 0;
}

static void kgem_bo_retire(struct kgem *kgem, struct kgem_bo *bo)
{
    DBG(("%s: handle=%x, domain=%d\n",
	     __FUNCTION__, bo->handle, bo->domain));
	assert(!kgem_busy(kgem, bo->handle));

	if (bo->domain == DOMAIN_GPU)
		kgem_retire(kgem);

	if (bo->exec == NULL) {
        DBG(("%s: retiring bo handle=%x (needed flush? %d), rq? %d\n",
		     __FUNCTION__, bo->handle, bo->needs_flush, bo->rq != NULL));
		bo->rq = NULL;
		list_del(&bo->request);
		bo->needs_flush = bo->flush;
	}
}

Bool kgem_bo_write(struct kgem *kgem, struct kgem_bo *bo,
           const void *data, int length)
{
    assert(bo->refcnt);
    assert(!bo->purged);
    assert(!kgem_busy(kgem, bo->handle));

    assert(length <= bytes(bo));
    if (gem_write(kgem->fd, bo->handle, 0, length, data))
        return FALSE;

    DBG(("%s: flush=%d, domain=%d\n", __FUNCTION__, bo->flush, bo->domain));
    kgem_bo_retire(kgem, bo);
    bo->domain = DOMAIN_NONE;
    return TRUE;
}

static uint32_t gem_create(int fd, int num_pages)
{
    struct drm_i915_gem_object *obj;
    int       ret;

    /* Allocate the new object */
    obj = i915_gem_alloc_object(main_device,
                                PAGE_SIZE * num_pages);
    if (obj == NULL)
        goto err1;

    ret = i915_gem_object_pin(obj, 4096, true);
    if (ret)
        goto err2;

    return (uint32_t)obj;

err2:
    drm_gem_object_unreference(&obj->base);
err1:
    return 0;
}

static bool
kgem_bo_set_purgeable(struct kgem *kgem, struct kgem_bo *bo)
{
    return true;
}


static bool
kgem_bo_clear_purgeable(struct kgem *kgem, struct kgem_bo *bo)
{
    return true;
}

static void gem_close(int fd, uint32_t handle)
{
    destroy_gem_object(handle);
}


/*
constant inline static unsigned long __fls(unsigned long word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    return word;
}
*/

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

static struct kgem_request _kgem_static_request;

static struct kgem_request *__kgem_request_alloc(void)
{
    struct kgem_request *rq;

    rq = malloc(sizeof(*rq));
    if (rq == NULL)
        rq = &_kgem_static_request;

    list_init(&rq->buffers);

    return rq;
}

static struct list_head *inactive(struct kgem *kgem, int num_pages)
{
    return &kgem->inactive[cache_bucket(num_pages)];
}

static struct list_head *active(struct kgem *kgem, int num_pages, int tiling)
{
    return &kgem->active[cache_bucket(num_pages)][tiling];
}



void kgem_init(struct kgem *kgem, int gen)
{
    struct drm_i915_gem_get_aperture aperture;
    struct drm_i915_gem_object     *obj;

    size_t totalram;
    unsigned int i, j;
    int ret;

    memset(kgem, 0, sizeof(*kgem));

    kgem->gen = gen;
    kgem->wedged  = 0;
//    kgem->wedged |= DBG_NO_HW;

    obj = i915_gem_alloc_object(main_device, 4096*4);
    if (obj == NULL)
        goto err2;

    ret = i915_gem_object_pin(obj, 4096, true);
    if (ret)
        goto err3;

    kgem->batch_ptr = drm_intel_bo_map(obj, true);
    kgem->batch = kgem->batch_ptr;
    kgem->batch_idx = 0;
    kgem->batch_obj = obj;

    kgem->max_batch_size = 1024; //ARRAY_SIZE(kgem->batch);

    kgem->half_cpu_cache_pages = (2048*1024) >> 13;

    list_init(&kgem->partial);
    list_init(&kgem->requests);
    list_init(&kgem->flushing);
    list_init(&kgem->large);
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

    kgem->next_request = __kgem_request_alloc();

//#if defined(USE_VMAP) && defined(I915_PARAM_HAS_VMAP)
//    if (!DBG_NO_VMAP)
//        kgem->has_vmap = gem_param(kgem, I915_PARAM_HAS_VMAP) > 0;
//#endif
//    DBG(("%s: using vmap=%d\n", __FUNCTION__, kgem->has_vmap));

    if (gen < 40) {
//        if (!DBG_NO_RELAXED_FENCING) {
//            kgem->has_relaxed_fencing =
//                gem_param(kgem, I915_PARAM_HAS_RELAXED_FENCING) > 0;
//        }
    } else
        kgem->has_relaxed_fencing = 1;
    DBG(("%s: has relaxed fencing? %d\n", __FUNCTION__,
         kgem->has_relaxed_fencing));

    kgem->has_llc = (gen >= 60)?true:false;
    kgem->has_cpu_bo = kgem->has_llc;
    DBG(("%s: cpu bo enabled %d: llc? %d\n", __FUNCTION__,
         kgem->has_cpu_bo, kgem->has_llc));

    kgem->has_semaphores = false;
//    if (gen >= 60 && semaphores_enabled())
//        kgem->has_semaphores = true;
//    DBG(("%s: semaphores enabled? %d\n", __FUNCTION__,
//         kgem->has_semaphores));

    VG_CLEAR(aperture);
    aperture.aper_size = 64*1024*1024;
    i915_gem_get_aperture_ioctl(main_device, &aperture, NULL);
    kgem->aperture_total = aperture.aper_size;
    kgem->aperture_high = aperture.aper_size * 3/4;
    kgem->aperture_low = aperture.aper_size * 1/3;
    DBG(("%s: aperture low=%d [%d], high=%d [%d]\n", __FUNCTION__,
         kgem->aperture_low, kgem->aperture_low / (1024*1024),
         kgem->aperture_high, kgem->aperture_high / (1024*1024)));

    kgem->aperture_mappable = aperture.aper_size;
    DBG(("%s: aperture mappable=%d [%d MiB]\n", __FUNCTION__,
         kgem->aperture_mappable, kgem->aperture_mappable / (1024*1024)));

    kgem->partial_buffer_size = 64 * 1024;
    while (kgem->partial_buffer_size < kgem->aperture_mappable >> 10)
        kgem->partial_buffer_size *= 2;
    DBG(("%s: partial buffer size=%d [%d KiB]\n", __FUNCTION__,
         kgem->partial_buffer_size, kgem->partial_buffer_size / 1024));

    kgem->min_alignment = 4;
    if (gen < 60)
        /* XXX workaround an issue where we appear to fail to
         * disable dual-stream mode */
        kgem->min_alignment = 64;

    kgem->max_object_size = 2 * kgem->aperture_total / 3;
    kgem->max_cpu_size = kgem->max_object_size;
    kgem->max_gpu_size = kgem->max_object_size;
    if (!kgem->has_llc)
        kgem->max_gpu_size = MAX_CACHE_SIZE;
    if (gen < 40) {
        /* If we have to use fences for blitting, we have to make
         * sure we can fit them into the aperture.
         */
        kgem->max_gpu_size = kgem->aperture_mappable / 2;
        if (kgem->max_gpu_size > kgem->aperture_low)
            kgem->max_gpu_size = kgem->aperture_low;
    }
    if (kgem->max_gpu_size > kgem->max_cpu_size)
        kgem->max_gpu_size = kgem->max_cpu_size;

    kgem->max_upload_tile_size = kgem->aperture_mappable / 2;
    if (kgem->max_upload_tile_size > kgem->max_gpu_size / 2)
        kgem->max_upload_tile_size = kgem->max_gpu_size / 2;

    kgem->max_copy_tile_size = (MAX_CACHE_SIZE + 1)/2;
    if (kgem->max_copy_tile_size > kgem->max_gpu_size / 2)
        kgem->max_copy_tile_size = kgem->max_gpu_size / 2;

    totalram = 1024*1024; //total_ram_size();
    if (totalram == 0) {
        DBG(("%s: total ram size unknown, assuming maximum of total aperture\n",
             __FUNCTION__));
        totalram = kgem->aperture_total;
    }
    if (kgem->max_object_size > totalram / 2)
        kgem->max_object_size = totalram / 2;
    if (kgem->max_cpu_size > totalram / 2)
        kgem->max_cpu_size = totalram / 2;
    if (kgem->max_gpu_size > totalram / 4)
        kgem->max_gpu_size = totalram / 4;

    kgem->large_object_size = MAX_CACHE_SIZE;
    if (kgem->large_object_size > kgem->max_gpu_size)
        kgem->large_object_size = kgem->max_gpu_size;

    DBG(("%s: large object thresold=%d\n",
         __FUNCTION__, kgem->large_object_size));
    DBG(("%s: max object size (gpu=%d, cpu=%d, tile upload=%d, copy=%d)\n",
         __FUNCTION__,
         kgem->max_gpu_size, kgem->max_cpu_size,
         kgem->max_upload_tile_size, kgem->max_copy_tile_size));

    /* Convert the aperture thresholds to pages */
    kgem->aperture_low /= PAGE_SIZE;
    kgem->aperture_high /= PAGE_SIZE;

//    kgem->fence_max = gem_param(kgem, I915_PARAM_NUM_FENCES_AVAIL) - 2;
//    if ((int)kgem->fence_max < 0)
        kgem->fence_max = 5; /* minimum safe value for all hw */
    DBG(("%s: max fences=%d\n", __FUNCTION__, kgem->fence_max));
err3:
err2:
    return;
}

static struct drm_i915_gem_exec_object2 *
kgem_add_handle(struct kgem *kgem, struct kgem_bo *bo)
{
	struct drm_i915_gem_exec_object2 *exec;

	DBG(("%s: handle=%d, index=%d\n",
	     __FUNCTION__, bo->handle, kgem->nexec));

	assert(kgem->nexec < ARRAY_SIZE(kgem->exec));
	exec = memset(&kgem->exec[kgem->nexec++], 0, sizeof(*exec));
	exec->handle = bo->handle;
	exec->offset = bo->presumed_offset;

	kgem->aperture += num_pages(bo);

	return exec;
}

void _kgem_add_bo(struct kgem *kgem, struct kgem_bo *bo)
{
	bo->exec = kgem_add_handle(kgem, bo);
	bo->rq = kgem->next_request;

	list_move(&bo->request, &kgem->next_request->buffers);

	/* XXX is it worth working around gcc here? */
	kgem->flush |= bo->flush;
	kgem->sync |= bo->sync;
	kgem->scanout |= bo->scanout;
}

static uint32_t kgem_end_batch(struct kgem *kgem)
{
//	kgem->context_switch(kgem, KGEM_NONE);

	kgem->batch[kgem->nbatch++] = MI_BATCH_BUFFER_END;
	if (kgem->nbatch & 1)
		kgem->batch[kgem->nbatch++] = MI_NOOP;

	return kgem->nbatch;
}

static void kgem_fixup_self_relocs(struct kgem *kgem, struct kgem_bo *bo)
{
	int n;

    for (n = 0; n < kgem->nreloc; n++)
    {
        if (kgem->reloc[n].target_handle == 0)
        {
			kgem->reloc[n].target_handle = bo->handle;
			kgem->reloc[n].presumed_offset = bo->presumed_offset;
			kgem->batch[kgem->reloc[n].offset/sizeof(kgem->batch[0])] =
				kgem->reloc[n].delta + bo->presumed_offset;

            dbgprintf("fixup reloc %d pos %d handle %d delta %x \n",
                       n, kgem->reloc[n].offset/sizeof(kgem->batch[0]),
                       bo->handle, kgem->reloc[n].delta);
		}
	}
}

static void kgem_bo_binding_free(struct kgem *kgem, struct kgem_bo *bo)
{
	struct kgem_bo_binding *b;

	b = bo->binding.next;
	while (b) {
		struct kgem_bo_binding *next = b->next;
		free (b);
		b = next;
	}
}

static void kgem_bo_release_map(struct kgem *kgem, struct kgem_bo *bo)
{
	int type = IS_CPU_MAP(bo->map);

	DBG(("%s: releasing %s vma for handle=%d, count=%d\n",
	     __FUNCTION__, type ? "CPU" : "GTT",
	     bo->handle, kgem->vma[type].count));

	VG(if (type) VALGRIND_FREELIKE_BLOCK(CPU_MAP(bo->map), 0));
//	munmap(CPU_MAP(bo->map), bytes(bo));
	bo->map = NULL;

	if (!list_is_empty(&bo->vma)) {
		list_del(&bo->vma);
		kgem->vma[type].count--;
	}
}

static void kgem_bo_free(struct kgem *kgem, struct kgem_bo *bo)
{
    DBG(("%s: handle=%d\n", __FUNCTION__, bo->handle));
    assert(bo->refcnt == 0);
    assert(bo->exec == NULL);

    kgem_bo_binding_free(kgem, bo);

    if (bo->map)
        kgem_bo_release_map(kgem, bo);
    assert(list_is_empty(&bo->vma));

    list_del(&bo->list);
    list_del(&bo->request);
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
    assert(!kgem_busy(kgem, bo->handle));
    assert(!bo->proxy);
    assert(!bo->io);
    assert(!bo->needs_flush);
    assert(bo->rq == NULL);
    assert(bo->domain != DOMAIN_GPU);

    if (bucket(bo) >= NUM_CACHE_BUCKETS) {
        kgem_bo_free(kgem, bo);
        return;
    }

    list_move(&bo->list, &kgem->inactive[bucket(bo)]);
    if (bo->map) {
        int type = IS_CPU_MAP(bo->map);
        if (bucket(bo) >= NUM_CACHE_BUCKETS ||
            (!type && !kgem_bo_is_mappable(kgem, bo))) {
            list_del(&bo->vma);
//            munmap(CPU_MAP(bo->map), bytes(bo));
            bo->map = NULL;
        }
        if (bo->map) {
            list_move(&bo->vma, &kgem->vma[type].inactive[bucket(bo)]);
            kgem->vma[type].count++;
        }
    }

    kgem->need_expire = true;
}

inline static void kgem_bo_remove_from_inactive(struct kgem *kgem,
                        struct kgem_bo *bo)
{
    list_del(&bo->list);
    assert(bo->rq == NULL);
    if (bo->map) {
        assert(!list_is_empty(&bo->vma));
        list_del(&bo->vma);
        kgem->vma[IS_CPU_MAP(bo->map)].count--;
    }
}

inline static void kgem_bo_remove_from_active(struct kgem *kgem,
                          struct kgem_bo *bo)
{
    list_del(&bo->list);
    if (bo->rq == &_kgem_static_request)
        list_del(&bo->request);
    assert(list_is_empty(&bo->vma));
}

static void __kgem_bo_destroy(struct kgem *kgem, struct kgem_bo *bo)
{
    DBG(("%s: handle=%d\n", __FUNCTION__, bo->handle));

    assert(list_is_empty(&bo->list));
    assert(bo->refcnt == 0);

    bo->binding.offset = 0;

    if (NO_CACHE)
        goto destroy;

    if (bo->io) {
        struct kgem_bo *base;

        base = malloc(sizeof(*base));
        if (base) {
            DBG(("%s: transferring io handle=%d to bo\n",
                 __FUNCTION__, bo->handle));
            /* transfer the handle to a minimum bo */
            memcpy(base, bo, sizeof (*base));
            base->reusable = true;
            base->io = false;
            list_init(&base->list);
            list_replace(&bo->request, &base->request);
            list_replace(&bo->vma, &base->vma);
            free(bo);
            bo = base;
        }
    }

    if (!bo->reusable) {
        DBG(("%s: handle=%d, not reusable\n",
             __FUNCTION__, bo->handle));
        goto destroy;
    }

    if (!kgem->has_llc && IS_CPU_MAP(bo->map) && bo->domain != DOMAIN_CPU)
        kgem_bo_release_map(kgem, bo);

    assert(list_is_empty(&bo->vma));
    assert(list_is_empty(&bo->list));
    assert(bo->vmap == false && bo->sync == false);
    assert(bo->io == false);

    bo->scanout = bo->flush = false;
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
/*
    if (bo->needs_flush) {
        if ((bo->needs_flush = kgem_busy(kgem, bo->handle))) {
            struct list *cache;

            DBG(("%s: handle=%d -> flushing\n",
                 __FUNCTION__, bo->handle));

            list_add(&bo->request, &kgem->flushing);
            if (bucket(bo) < NUM_CACHE_BUCKETS)
                cache = &kgem->active[bucket(bo)][bo->tiling];
            else
                cache = &kgem->large;
            list_add(&bo->list, cache);
            bo->rq = &_kgem_static_request;
            return;
        }

        bo->domain = DOMAIN_NONE;
    }
*/
    if (!IS_CPU_MAP(bo->map)) {
        if (!kgem_bo_set_purgeable(kgem, bo))
            goto destroy;

        if (!kgem->has_llc && bo->domain == DOMAIN_CPU)
            goto destroy;

        DBG(("%s: handle=%d, purged\n",
             __FUNCTION__, bo->handle));
    }

    DBG(("%s: handle=%d -> inactive\n", __FUNCTION__, bo->handle));
    kgem_bo_move_to_inactive(kgem, bo);
    return;

destroy:
    if (!bo->exec)
        kgem_bo_free(kgem, bo);
}





bool kgem_retire(struct kgem *kgem)
{
    struct kgem_bo *bo, *next;
    bool retired = false;

    DBG(("%s\n", __FUNCTION__));

    list_for_each_entry_safe(bo, next, &kgem->flushing, request) {
        assert(bo->refcnt == 0);
        assert(bo->rq == &_kgem_static_request);
        assert(bo->exec == NULL);

//        if (kgem_busy(kgem, bo->handle))
//            break;

        DBG(("%s: moving %d from flush to inactive\n",
             __FUNCTION__, bo->handle));
        if (kgem_bo_set_purgeable(kgem, bo)) {
            bo->needs_flush = false;
            bo->domain = DOMAIN_NONE;
            bo->rq = NULL;
            list_del(&bo->request);
            kgem_bo_move_to_inactive(kgem, bo);
        } else
            kgem_bo_free(kgem, bo);

        retired = true;
    }

    while (!list_is_empty(&kgem->requests)) {
        struct kgem_request *rq;

        rq = list_first_entry(&kgem->requests,
                      struct kgem_request,
                      list);
//        if (kgem_busy(kgem, rq->bo->handle))
//            break;

        DBG(("%s: request %d complete\n",
             __FUNCTION__, rq->bo->handle));

        while (!list_is_empty(&rq->buffers)) {
            bo = list_first_entry(&rq->buffers,
                          struct kgem_bo,
                          request);

            assert(bo->rq == rq);
            assert(bo->exec == NULL);
            assert(bo->domain == DOMAIN_GPU);

            list_del(&bo->request);
            bo->rq = NULL;

//            if (bo->needs_flush)
//                bo->needs_flush = kgem_busy(kgem, bo->handle);
            if (!bo->needs_flush)
                bo->domain = DOMAIN_NONE;

            if (bo->refcnt)
                continue;

            if (!bo->reusable) {
                DBG(("%s: closing %d\n",
                     __FUNCTION__, bo->handle));
                kgem_bo_free(kgem, bo);
                continue;
            }

            if (bo->needs_flush) {
                DBG(("%s: moving %d to flushing\n",
                     __FUNCTION__, bo->handle));
                list_add(&bo->request, &kgem->flushing);
                bo->rq = &_kgem_static_request;
            } else if (kgem_bo_set_purgeable(kgem, bo)) {
                DBG(("%s: moving %d to inactive\n",
                     __FUNCTION__, bo->handle));
                kgem_bo_move_to_inactive(kgem, bo);
                retired = true;
            } else {
                DBG(("%s: closing %d\n",
                     __FUNCTION__, bo->handle));
                kgem_bo_free(kgem, bo);
            }
        }

        rq->bo->refcnt--;
        assert(rq->bo->refcnt == 0);
        assert(rq->bo->rq == NULL);
        assert(list_is_empty(&rq->bo->request));
        if (kgem_bo_set_purgeable(kgem, rq->bo)) {
            kgem_bo_move_to_inactive(kgem, rq->bo);
            retired = true;
        } else {
            DBG(("%s: closing %d\n",
                 __FUNCTION__, rq->bo->handle));
            kgem_bo_free(kgem, rq->bo);
        }

        list_del(&rq->list);
        free(rq);
    }

    kgem->need_retire = !list_is_empty(&kgem->requests);
    DBG(("%s -- need_retire=%d\n", __FUNCTION__, kgem->need_retire));

    kgem->retire(kgem);

    return retired;
}









































static int kgem_batch_write(struct kgem *kgem, uint32_t handle, uint32_t size)
{
	int ret;

	assert(!kgem_busy(kgem, handle));

	/* If there is no surface data, just upload the batch */
	if (kgem->surface == kgem->max_batch_size)
		return gem_write(kgem->fd, handle,
				 0, sizeof(uint32_t)*kgem->nbatch,
				 kgem->batch);

	/* Are the batch pages conjoint with the surface pages? */
	if (kgem->surface < kgem->nbatch + PAGE_SIZE/4) {
		assert(size == sizeof(kgem->batch));
		return gem_write(kgem->fd, handle,
				 0, sizeof(kgem->batch),
				 kgem->batch);
	}

	/* Disjoint surface/batch, upload separately */
	ret = gem_write(kgem->fd, handle,
			0, sizeof(uint32_t)*kgem->nbatch,
			kgem->batch);
	if (ret)
		return ret;

   assert(kgem->nbatch*sizeof(uint32_t) <=
	       sizeof(uint32_t)*kgem->surface - (sizeof(kgem->batch)-size));
   return __gem_write(kgem->fd, handle,
			sizeof(uint32_t)*kgem->surface - (sizeof(kgem->batch)-size),
			sizeof(kgem->batch) - sizeof(uint32_t)*kgem->surface,
           kgem->batch + kgem->surface);
}

void kgem_reset(struct kgem *kgem)
{
//    ENTER();

    kgem->nfence = 0;
    kgem->nexec = 0;
    kgem->nreloc = 0;
    kgem->aperture = 0;
    kgem->aperture_fenced = 0;
    kgem->nbatch = 0;
    kgem->surface = kgem->max_batch_size;
    kgem->mode = KGEM_NONE;
    kgem->flush = 0;
    kgem->scanout = 0;

    kgem->batch = kgem->batch_ptr+1024*kgem->batch_idx;

    kgem->next_request = __kgem_request_alloc();

    kgem_sna_reset(kgem);
//    dbgprintf("surface %x\n", kgem->surface);
//    LEAVE();
}

static int compact_batch_surface(struct kgem *kgem)
{
	int size, shrink, n;

	/* See if we can pack the contents into one or two pages */
	size = kgem->max_batch_size - kgem->surface + kgem->nbatch;
	if (size > 2048)
		return sizeof(kgem->batch);
	else if (size > 1024)
		size = 8192, shrink = 2*4096;
	else
		size = 4096, shrink = 3*4096;


	for (n = 0; n < kgem->nreloc; n++) {
		if (kgem->reloc[n].read_domains == I915_GEM_DOMAIN_INSTRUCTION &&
		    kgem->reloc[n].target_handle == 0)
			kgem->reloc[n].delta -= shrink;

		if (kgem->reloc[n].offset >= size)
			kgem->reloc[n].offset -= shrink;
	}

	return size;
}

void execute_buffer (struct drm_i915_gem_object *buffer, uint32_t offset,
                     int size);

void _kgem_submit(struct kgem *kgem)
{
	struct kgem_request *rq;
	uint32_t batch_end;
	int size;

	assert(!DBG_NO_HW);

	assert(kgem->nbatch);
	assert(kgem->nbatch <= KGEM_BATCH_SIZE(kgem));
	assert(kgem->nbatch <= kgem->surface);

	batch_end = kgem_end_batch(kgem);
	kgem_sna_flush(kgem);

	DBG(("batch[%d/%d]: %d %d %d, nreloc=%d, nexec=%d, nfence=%d, aperture=%d\n",
	     kgem->mode, kgem->ring, batch_end, kgem->nbatch, kgem->surface,
	     kgem->nreloc, kgem->nexec, kgem->nfence, kgem->aperture));

	assert(kgem->nbatch <= kgem->max_batch_size);
	assert(kgem->nbatch <= kgem->surface);
	assert(kgem->nreloc <= ARRAY_SIZE(kgem->reloc));
	assert(kgem->nexec < ARRAY_SIZE(kgem->exec));
	assert(kgem->nfence <= kgem->fence_max);

//   kgem_finish_partials(kgem);

	rq = kgem->next_request;
//   if (kgem->surface != kgem->max_batch_size)
//       size = compact_batch_surface(kgem);
//   else
		size = kgem->nbatch * sizeof(kgem->batch[0]);
#if 0
    {
        int i;

        dbgprintf("\nDump batch\n\n");

        for(i=0; i < kgem->nbatch; i++)
        {
            dbgprintf("\t0x%08x,\t/* %d */\n",
                      kgem->batch[i], i);
        }
        dbgprintf("\ndone\n");
    };
#endif

    execute_buffer(kgem->batch_obj, kgem->batch_idx*4096, sizeof(uint32_t)*kgem->nbatch);

//   if (kgem->wedged)
//       kgem_cleanup(kgem);

    kgem->batch_idx++;
    kgem->batch_idx&= 3;

	kgem->flush_now = kgem->scanout;
	kgem_reset(kgem);

	assert(kgem->next_request != NULL);
}

static struct kgem_bo *
search_linear_cache(struct kgem *kgem, unsigned int num_pages, unsigned flags)
{
    struct kgem_bo *bo, *first = NULL;
    bool use_active = (flags & CREATE_INACTIVE) == 0;
    struct list_head *cache;

    if (num_pages >= MAX_CACHE_SIZE / PAGE_SIZE)
        return NULL;

    if (!use_active &&
        list_is_empty(inactive(kgem, num_pages)) &&
        !list_is_empty(active(kgem, num_pages, I915_TILING_NONE)) &&
        !kgem_retire(kgem))
        return NULL;

    if (!use_active && flags & (CREATE_CPU_MAP | CREATE_GTT_MAP)) {
        int for_cpu = !!(flags & CREATE_CPU_MAP);
        cache = &kgem->vma[for_cpu].inactive[cache_bucket(num_pages)];
        list_for_each_entry(bo, cache, vma) {
            assert(IS_CPU_MAP(bo->map) == for_cpu);
            assert(bucket(bo) == cache_bucket(num_pages));

            if (num_pages > num_pages(bo)) {
                DBG(("inactive too small: %d < %d\n",
                     num_pages(bo), num_pages));
                continue;
            }

            if (bo->purged && !kgem_bo_clear_purgeable(kgem, bo)) {
                kgem_bo_free(kgem, bo);
                break;
            }

//            if (I915_TILING_NONE != bo->tiling &&
//                gem_set_tiling(kgem->fd, bo->handle,
//                       I915_TILING_NONE, 0) != I915_TILING_NONE)
//                continue;

            kgem_bo_remove_from_inactive(kgem, bo);

            bo->tiling = I915_TILING_NONE;
            bo->pitch = 0;
            bo->delta = 0;
            DBG(("  %s: found handle=%d (num_pages=%d) in linear vma cache\n",
                 __FUNCTION__, bo->handle, num_pages(bo)));
            assert(use_active || bo->domain != DOMAIN_GPU);
            assert(!bo->needs_flush);
            //assert(!kgem_busy(kgem, bo->handle));
            return bo;
        }
    }

    cache = use_active ? active(kgem, num_pages, I915_TILING_NONE) : inactive(kgem, num_pages);
    list_for_each_entry(bo, cache, list) {
        assert(bo->refcnt == 0);
        assert(bo->reusable);
        assert(!!bo->rq == !!use_active);

        if (num_pages > num_pages(bo))
            continue;

        if (use_active && bo->tiling != I915_TILING_NONE)
            continue;

        if (bo->purged && !kgem_bo_clear_purgeable(kgem, bo)) {
            kgem_bo_free(kgem, bo);
            break;
        }
/*
        if (I915_TILING_NONE != bo->tiling) {
            if (use_active)
                continue;

            if (gem_set_tiling(kgem->fd, bo->handle,
                       I915_TILING_NONE, 0) != I915_TILING_NONE)
                continue;

            bo->tiling = I915_TILING_NONE;
        }
*/
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
        assert(use_active || bo->domain != DOMAIN_GPU);
        assert(!bo->needs_flush || use_active);
        //assert(use_active || !kgem_busy(kgem, bo->handle));
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
        DBG(("  %s: found handle=%d (num_pages=%d) in linear %s cache\n",
             __FUNCTION__, first->handle, num_pages(first),
             use_active ? "active" : "inactive"));
        assert(use_active || first->domain != DOMAIN_GPU);
        assert(!first->needs_flush || use_active);
        //assert(use_active || !kgem_busy(kgem, first->handle));
        return first;
    }

    return NULL;
}






struct kgem_bo *kgem_create_linear(struct kgem *kgem, int size)
{
    struct kgem_bo *bo;
    uint32_t handle;

    DBG(("%s(%d)\n", __FUNCTION__, size));

    size = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    bo = search_linear_cache(kgem, size, CREATE_INACTIVE);
    if (bo)
        return kgem_bo_reference(bo);

    handle = gem_create(kgem->fd, size);
    if (handle == 0)
        return NULL;

    DBG(("%s: new handle=%x\n", __FUNCTION__, handle));
    bo = __kgem_bo_alloc(handle, size);
    if (bo == NULL) {
        gem_close(kgem->fd, handle);
        return NULL;
    }
    struct drm_i915_gem_object *obj;
    obj = (void*)handle;

    bo->gaddr = obj->gtt_offset;
    return bo;
}






inline int kgem_bo_fenced_size(struct kgem *kgem, struct kgem_bo *bo)
{
	unsigned int size;

	assert(bo->tiling);
	assert(kgem->gen < 40);

	if (kgem->gen < 30)
		size = 512 * 1024;
	else
		size = 1024 * 1024;
	while (size < bytes(bo))
		size *= 2;

	return size;
}
















































void _kgem_bo_destroy(struct kgem *kgem, struct kgem_bo *bo)
{
//    if (bo->proxy) {
//        assert(bo->map == NULL);
//        if (bo->io && bo->exec == NULL)
//            _kgem_bo_delete_partial(kgem, bo);
//        kgem_bo_unref(kgem, bo->proxy);
//        kgem_bo_binding_free(kgem, bo);
//        _list_del(&bo->request);
//        free(bo);
//        return;
//    }

//    if (bo->vmap)
//        kgem_bo_sync__cpu(kgem, bo);

    __kgem_bo_destroy(kgem, bo);
}

void __kgem_flush(struct kgem *kgem, struct kgem_bo *bo)
{
	/* The kernel will emit a flush *and* update its own flushing lists. */
//	kgem_busy(kgem, bo->handle);
}

bool kgem_check_bo(struct kgem *kgem, ...)
{
    va_list ap;
    struct kgem_bo *bo;
    int num_exec = 0;
    int num_pages = 0;

    va_start(ap, kgem);
    while ((bo = va_arg(ap, struct kgem_bo *))) {
        if (bo->exec)
            continue;

        if (bo->proxy) {
            bo = bo->proxy;
            if (bo->exec)
                continue;
        }
        num_pages += num_pages(bo);
        num_exec++;
    }
    va_end(ap);

    if (!num_pages)
        return true;

    if (kgem->aperture > kgem->aperture_low)
        return false;

    if (num_pages + kgem->aperture > kgem->aperture_high)
        return false;

    if (kgem->nexec + num_exec >= KGEM_EXEC_SIZE(kgem))
        return false;

    return true;
}

/*
bool kgem_check_bo_fenced(struct kgem *kgem, ...)
{
	va_list ap;
	struct kgem_bo *bo;
	int num_fence = 0;
	int num_exec = 0;
	int num_pages = 0;
	int fenced_size = 0;

	va_start(ap, kgem);
	while ((bo = va_arg(ap, struct kgem_bo *))) {
		if (bo->proxy)
			bo = bo->proxy;
		if (bo->exec) {
			if (kgem->gen >= 40 || bo->tiling == I915_TILING_NONE)
				continue;

			if ((bo->exec->flags & EXEC_OBJECT_NEEDS_FENCE) == 0) {
				fenced_size += kgem_bo_fenced_size(kgem, bo);
				num_fence++;
			}

			continue;
		}

		num_pages += num_pages(bo);
		num_exec++;
		if (kgem->gen < 40 && bo->tiling) {
			fenced_size += kgem_bo_fenced_size(kgem, bo);
			num_fence++;
		}
	}
	va_end(ap);

	if (fenced_size + kgem->aperture_fenced > kgem->aperture_mappable)
		return false;

	if (kgem->nfence + num_fence > kgem->fence_max)
		return false;

	if (!num_pages)
		return true;

	if (kgem->aperture > kgem->aperture_low)
		return false;

	if (num_pages + kgem->aperture > kgem->aperture_high)
		return false;

	if (kgem->nexec + num_exec >= KGEM_EXEC_SIZE(kgem))
		return false;

	return true;
}
*/
#if 0
uint32_t kgem_add_reloc(struct kgem *kgem,
            uint32_t pos,
            struct kgem_bo *bo,
            uint32_t read_write_domain,
            uint32_t delta)
{
    int index;

	DBG(("%s: handle=%d, pos=%d, delta=%d, domains=%08x\n",
         __FUNCTION__, bo ? bo->handle : 0, pos, delta, read_write_domain));

    assert((read_write_domain & 0x7fff) == 0 || bo != NULL);

    index = kgem->nreloc++;
    assert(index < ARRAY_SIZE(kgem->reloc));
    kgem->reloc[index].offset = pos * sizeof(kgem->batch[0]);
    if (bo) {
        assert(bo->refcnt);
        assert(!bo->purged);

        delta += bo->delta;
        if (bo->proxy) {
            DBG(("%s: adding proxy for handle=%d\n",
                 __FUNCTION__, bo->handle));
            assert(bo->handle == bo->proxy->handle);
            /* need to release the cache upon batch submit */
            list_move(&bo->request, &kgem->next_request->buffers);
            bo->exec = &_kgem_dummy_exec;
            bo = bo->proxy;
        }

        assert(!bo->purged);

//       if (bo->exec == NULL)
//           _kgem_add_bo(kgem, bo);

//        if (kgem->gen < 40 && read_write_domain & KGEM_RELOC_FENCED) {
//            if (bo->tiling &&
//                (bo->exec->flags & EXEC_OBJECT_NEEDS_FENCE) == 0) {
//                assert(kgem->nfence < kgem->fence_max);
//                kgem->aperture_fenced +=
//                    kgem_bo_fenced_size(kgem, bo);
//                kgem->nfence++;
//            }
//            bo->exec->flags |= EXEC_OBJECT_NEEDS_FENCE;
//        }

        kgem->reloc[index].delta = delta;
        kgem->reloc[index].target_handle = bo->handle;
        kgem->reloc[index].presumed_offset = bo->presumed_offset;

        if (read_write_domain & 0x7fff) {
           DBG(("%s: marking handle=%d dirty\n",
                __FUNCTION__, bo->handle));
           bo->needs_flush = bo->dirty = true;
       }

        delta += bo->presumed_offset;
    } else {
        kgem->reloc[index].delta = delta;
        kgem->reloc[index].target_handle = 0;
        kgem->reloc[index].presumed_offset = 0;
    }
    kgem->reloc[index].read_domains = read_write_domain >> 16;
    kgem->reloc[index].write_domain = read_write_domain & 0x7fff;

    return delta;
}
#endif









void *kgem_bo_map(struct kgem *kgem, struct kgem_bo *bo)
{
    void *ptr;

    DBG(("%s: handle=%d, offset=%d, tiling=%d, map=%p, domain=%d\n", __FUNCTION__,
         bo->handle, bo->presumed_offset, bo->tiling, bo->map, bo->domain));

    assert(!bo->purged);
    assert(bo->exec == NULL);
    assert(list_is_empty(&bo->list));

//    if (bo->tiling == I915_TILING_NONE &&
//        (kgem->has_llc || bo->domain == bo->presumed_offset)) {
        DBG(("%s: converting request for GTT map into CPU map\n",
             __FUNCTION__));
        ptr = kgem_bo_map__cpu(kgem, bo);
//        kgem_bo_sync__cpu(kgem, bo);
        return ptr;
//    }

#if 0

    if (IS_CPU_MAP(bo->map))
        kgem_bo_release_map(kgem, bo);

    ptr = bo->map;
    if (ptr == NULL) {
        assert(bytes(bo) <= kgem->aperture_mappable / 4);

        kgem_trim_vma_cache(kgem, MAP_GTT, bucket(bo));

        ptr = gem_mmap(kgem->fd, bo->handle, bytes(bo),
                   PROT_READ | PROT_WRITE);
        if (ptr == NULL)
            return NULL;

        /* Cache this mapping to avoid the overhead of an
         * excruciatingly slow GTT pagefault. This is more an
         * issue with compositing managers which need to frequently
         * flush CPU damage to their GPU bo.
         */
        bo->map = ptr;
        DBG(("%s: caching GTT vma for %d\n", __FUNCTION__, bo->handle));
    }

    if (bo->domain != DOMAIN_GTT) {
        struct drm_i915_gem_set_domain set_domain;

        DBG(("%s: sync: needs_flush? %d, domain? %d\n", __FUNCTION__,
             bo->needs_flush, bo->domain));

        /* XXX use PROT_READ to avoid the write flush? */

        VG_CLEAR(set_domain);
        set_domain.handle = bo->handle;
        set_domain.read_domains = I915_GEM_DOMAIN_GTT;
        set_domain.write_domain = I915_GEM_DOMAIN_GTT;
        drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &set_domain);

        kgem_bo_retire(kgem, bo);
        bo->domain = DOMAIN_GTT;
    }
#endif

    return ptr;
}

void *kgem_bo_map__cpu(struct kgem *kgem, struct kgem_bo *bo)
{
//    struct drm_i915_gem_mmap mmap_arg;

    DBG(("%s(handle=%d, size=%d)\n", __FUNCTION__, bo->handle, bytes(bo)));
    assert(!bo->purged);
    assert(list_is_empty(&bo->list));

    if (IS_CPU_MAP(bo->map))
        return CPU_MAP(bo->map);

    struct drm_i915_gem_object *obj = (void*)bo->handle;
    u8    *dst;
    int    ret;

    if(obj->pin_count == 0)
    {
        ret = i915_gem_object_pin(obj, 4096, true);
        if (ret)
            return NULL;
    };

    dst = drm_intel_bo_map(obj, true);
    DBG(("%s: caching CPU vma for %d\n", __FUNCTION__, bo->handle));
    bo->map = MAKE_CPU_MAP(dst);
    return (void *)dst;


#if 0
    if (bo->map)
        kgem_bo_release_map(kgem, bo);

    kgem_trim_vma_cache(kgem, MAP_CPU, bucket(bo));

    VG_CLEAR(mmap_arg);
    mmap_arg.handle = bo->handle;
    mmap_arg.offset = 0;
    mmap_arg.size = bytes(bo);
    if (drmIoctl(kgem->fd, DRM_IOCTL_I915_GEM_MMAP, &mmap_arg)) {
        ErrorF("%s: failed to mmap %d, %d bytes, into CPU domain\n",
               __FUNCTION__, bo->handle, bytes(bo));
        return NULL;
    }

    VG(VALGRIND_MALLOCLIKE_BLOCK(mmap_arg.addr_ptr, bytes(bo), 0, 1));
#endif

}



















void kgem_clear_dirty(struct kgem *kgem)
{
    struct kgem_request *rq = kgem->next_request;
    struct kgem_bo *bo;

    list_for_each_entry(bo, &rq->buffers, request)
        bo->dirty = false;
}

struct kgem_bo *kgem_create_proxy(struct kgem_bo *target,
				  int offset, int length)
{
	struct kgem_bo *bo;

	DBG(("%s: target handle=%d, offset=%d, length=%d, io=%d\n",
	     __FUNCTION__, target->handle, offset, length, target->io));

	bo = __kgem_bo_alloc(target->handle, length);
	if (bo == NULL)
		return NULL;

	bo->reusable = false;
	bo->size.bytes = length;

	bo->io = target->io;
	bo->dirty = target->dirty;
	bo->tiling = target->tiling;
	bo->pitch = target->pitch;

	if (target->proxy) {
		offset += target->delta;
		target = target->proxy;
	}
	bo->proxy = kgem_bo_reference(target);
	bo->delta = offset;
    bo->gaddr = offset + target->gaddr;
	return bo;
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


struct kgem_bo *create_bo(bitmap_t *bitmap)
{
  struct kgem_bo *bo;

  bo = __kgem_bo_alloc(bitmap->obj, 1024*768*4/4096);
  bo->gaddr  = bitmap->gaddr;
  bo->pitch  = bitmap->pitch;
  bo->tiling = 0;
  return bo;

};
