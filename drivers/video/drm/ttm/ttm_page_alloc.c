/*
 * Copyright (c) Red Hat Inc.

 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors: Dave Airlie <airlied@redhat.com>
 *          Jerome Glisse <jglisse@redhat.com>
 *          Pauli Nieminen <suokkos@gmail.com>
 */

/* simple list based uncached page pool
 * - Pool collects resently freed pages for reuse
 * - Use page->lru to keep a free list
 * - doesn't track currently in use pages
 */

#define pr_fmt(fmt) "[TTM] " fmt

#include <linux/list.h>
#include <linux/spinlock.h>
//#include <linux/highmem.h>
//#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/seq_file.h> /* for seq_printf */
#include <linux/slab.h>
#include <linux/dma-mapping.h>

#include <linux/atomic.h>

#include <drm/ttm/ttm_bo_driver.h>
#include <drm/ttm/ttm_page_alloc.h>

#ifdef TTM_HAS_AGP
#include <asm/agp.h>
#endif

#define NUM_PAGES_TO_ALLOC		(PAGE_SIZE/sizeof(struct page *))
#define SMALL_ALLOCATION		16
#define FREE_ALL_PAGES			(~0U)
/* times are in msecs */
#define PAGE_FREE_INTERVAL		1000

/**
 * struct ttm_page_pool - Pool to reuse recently allocated uc/wc pages.
 *
 * @lock: Protects the shared pool from concurrnet access. Must be used with
 * irqsave/irqrestore variants because pool allocator maybe called from
 * delayed work.
 * @fill_lock: Prevent concurrent calls to fill.
 * @list: Pool of free uc/wc pages for fast reuse.
 * @gfp_flags: Flags to pass for alloc_page.
 * @npages: Number of pages in pool.
 */
struct ttm_page_pool {
	spinlock_t		lock;
	bool			fill_lock;
	struct list_head	list;
	gfp_t			gfp_flags;
	unsigned		npages;
	char			*name;
	unsigned long		nfrees;
	unsigned long		nrefills;
};

/**
 * Limits for the pool. They are handled without locks because only place where
 * they may change is in sysfs store. They won't have immediate effect anyway
 * so forcing serialization to access them is pointless.
 */

struct ttm_pool_opts {
	unsigned	alloc_size;
	unsigned	max_size;
	unsigned	small;
};

#define NUM_POOLS 4

/**
 * struct ttm_pool_manager - Holds memory pools for fst allocation
 *
 * Manager is read only object for pool code so it doesn't need locking.
 *
 * @free_interval: minimum number of jiffies between freeing pages from pool.
 * @page_alloc_inited: reference counting for pool allocation.
 * @work: Work that is used to shrink the pool. Work is only run when there is
 * some pages to free.
 * @small_allocation: Limit in number of pages what is small allocation.
 *
 * @pools: All pool objects in use.
 **/
struct ttm_pool_manager {
	struct kobject		kobj;
	struct ttm_pool_opts	options;

	union {
		struct ttm_page_pool	pools[NUM_POOLS];
		struct {
			struct ttm_page_pool	wc_pool;
			struct ttm_page_pool	uc_pool;
			struct ttm_page_pool	wc_pool_dma32;
			struct ttm_page_pool	uc_pool_dma32;
		} ;
	};
};

static void ttm_pool_kobj_release(struct kobject *kobj)
{
	struct ttm_pool_manager *m =
		container_of(kobj, struct ttm_pool_manager, kobj);
	kfree(m);
}

static struct ttm_pool_manager *_manager;


/**
 * Select the right pool or requested caching state and ttm flags. */
static struct ttm_page_pool *ttm_get_pool(int flags,
		enum ttm_caching_state cstate)
{
	int pool_index;

	if (cstate == tt_cached)
		return NULL;

	if (cstate == tt_wc)
		pool_index = 0x0;
	else
		pool_index = 0x1;

	if (flags & TTM_PAGE_FLAG_DMA32)
		pool_index |= 0x2;

	return &_manager->pools[pool_index];
}

/* set memory back to wb and free the pages. */
static void ttm_pages_put(struct page *pages[], unsigned npages)
{
	unsigned i;
	for (i = 0; i < npages; ++i)
		__free_page(pages[i]);
}

static void ttm_pool_update_free_locked(struct ttm_page_pool *pool,
		unsigned freed_pages)
{
	pool->npages -= freed_pages;
	pool->nfrees += freed_pages;
}




/* Put all pages in pages list to correct pool to wait for reuse */
static void ttm_put_pages(struct page **pages, unsigned npages, int flags,
			  enum ttm_caching_state cstate)
{
	unsigned long irq_flags;
	struct ttm_page_pool *pool = ttm_get_pool(flags, cstate);
	unsigned i;

	if (1) {
		/* No pool for this memory type so free the pages */
		for (i = 0; i < npages; i++) {
			if (pages[i]) {
				__free_page(pages[i]);
				pages[i] = NULL;
			}
		}
		return;
	}

}

/*
 * On success pages list will hold count number of correctly
 * cached pages.
 */
static int ttm_get_pages(struct page **pages, unsigned npages, int flags,
			 enum ttm_caching_state cstate)
{
	struct ttm_page_pool *pool = ttm_get_pool(flags, cstate);
	struct list_head plist;
	struct page *p = NULL;
	gfp_t gfp_flags = 0;
	unsigned count;
	int r;

	
	/* No pool for cached pages */
	if (1) {

		for (r = 0; r < npages; ++r) {
			p = alloc_page(gfp_flags);
			if (!p) {

				return -ENOMEM;
			}

			pages[r] = p;
		}
		return 0;
	}



	return 0;
}

static void ttm_page_pool_init_locked(struct ttm_page_pool *pool, gfp_t flags,
		char *name)
{
	spin_lock_init(&pool->lock);
	pool->fill_lock = false;
	INIT_LIST_HEAD(&pool->list);
	pool->npages = pool->nfrees = 0;
	pool->gfp_flags = flags;
	pool->name = name;
}

int ttm_page_alloc_init(struct ttm_mem_global *glob, unsigned max_pages)
{
	int ret;

	WARN_ON(_manager);

	_manager = kzalloc(sizeof(*_manager), GFP_KERNEL);

	_manager->options.max_size = max_pages;
	_manager->options.small = SMALL_ALLOCATION;
	_manager->options.alloc_size = NUM_PAGES_TO_ALLOC;

	return 0;
}

void ttm_page_alloc_fini(void)
{
	int i;

	_manager = NULL;
}

int ttm_pool_populate(struct ttm_tt *ttm)
{
	struct ttm_mem_global *mem_glob = ttm->glob->mem_glob;
	unsigned i;
	int ret;

	if (ttm->state != tt_unpopulated)
		return 0;

	for (i = 0; i < ttm->num_pages; ++i) {
		ret = ttm_get_pages(&ttm->pages[i], 1,
				    ttm->page_flags,
				    ttm->caching_state);
		if (ret != 0) {
			ttm_pool_unpopulate(ttm);
			return -ENOMEM;
		}
	}
	ttm->state = tt_unbound;
	return 0;
}
EXPORT_SYMBOL(ttm_pool_populate);

void ttm_pool_unpopulate(struct ttm_tt *ttm)
{
	unsigned i;

	for (i = 0; i < ttm->num_pages; ++i) {
		if (ttm->pages[i]) {
			ttm_put_pages(&ttm->pages[i], 1,
				      ttm->page_flags,
				      ttm->caching_state);
		}
	}
	ttm->state = tt_unpopulated;
}
EXPORT_SYMBOL(ttm_pool_unpopulate);

