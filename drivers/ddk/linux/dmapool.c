/*
 * DMA Pool allocator
 *
 * Copyright 2001 David Brownell
 * Copyright 2007 Intel Corporation
 *   Author: Matthew Wilcox <willy@linux.intel.com>
 *
 * This software may be redistributed and/or modified under the terms of
 * the GNU General Public License ("GPL") version 2 as published by the
 * Free Software Foundation.
 *
 * This allocator returns small blocks of a given size which are DMA-able by
 * the given device.  It uses the dma_alloc_coherent page allocator to get
 * new pages, then splits them up into blocks of the required size.
 * Many older drivers still have their own code to do this.
 *
 * The current design of this allocator is fairly simple.  The pool is
 * represented by the 'struct dma_pool' which keeps a doubly-linked list of
 * allocated pages.  Each page in the page_list is split into blocks of at
 * least 'size' bytes.  Free blocks are tracked in an unsorted singly-linked
 * list of free blocks within the page.  Used blocks aren't tracked, but we
 * keep a count of how many are currently allocated from each page.
 */


#include <ddk.h>
#include <linux/mutex.h>
#include <syscall.h>


struct dma_pool {       /* the pool */
    struct list_head page_list;
    struct mutex lock;
    size_t size;
    size_t allocation;
    size_t boundary;
    struct list_head pools;
};

struct dma_page {       /* cacheable header for 'allocation' bytes */
    struct list_head page_list;
    void *vaddr;
    dma_addr_t dma;
    unsigned int in_use;
    unsigned int offset;
};


static DEFINE_MUTEX(pools_lock);


/**
 * dma_pool_create - Creates a pool of consistent memory blocks, for dma.
 * @name: name of pool, for diagnostics
 * @dev: device that will be doing the DMA
 * @size: size of the blocks in this pool.
 * @align: alignment requirement for blocks; must be a power of two
 * @boundary: returned blocks won't cross this power of two boundary
 * Context: !in_interrupt()
 *
 * Returns a dma allocation pool with the requested characteristics, or
 * null if one can't be created.  Given one of these pools, dma_pool_alloc()
 * may be used to allocate memory.  Such memory will all have "consistent"
 * DMA mappings, accessible by the device and its driver without using
 * cache flushing primitives.  The actual size of blocks allocated may be
 * larger than requested because of alignment.
 *
 * If @boundary is nonzero, objects returned from dma_pool_alloc() won't
 * cross that size boundary.  This is useful for devices which have
 * addressing restrictions on individual DMA transfers, such as not crossing
 * boundaries of 4KBytes.
 */
struct dma_pool *dma_pool_create(const char *name, struct device *dev,
                 size_t size, size_t align, size_t boundary)
{
    struct dma_pool *retval;
    size_t allocation;

    if (align == 0) {
        align = 1;
    } else if (align & (align - 1)) {
        return NULL;
    }

    if (size == 0) {
        return NULL;
    } else if (size < 4) {
        size = 4;
    }

    if ((size % align) != 0)
        size = ALIGN(size, align);

    allocation = max_t(size_t, size, PAGE_SIZE);

    allocation = (allocation+0x7FFF) & ~0x7FFF;

    if (!boundary) {
        boundary = allocation;
    } else if ((boundary < size) || (boundary & (boundary - 1))) {
        return NULL;
    }

    retval = kmalloc(sizeof(*retval), GFP_KERNEL);

    if (!retval)
        return retval;

    INIT_LIST_HEAD(&retval->page_list);

//    spin_lock_init(&retval->lock);

    retval->size = size;
    retval->boundary = boundary;
    retval->allocation = allocation;

    INIT_LIST_HEAD(&retval->pools);

    return retval;
}

static void pool_initialise_page(struct dma_pool *pool, struct dma_page *page)
{
    unsigned int offset = 0;
    unsigned int next_boundary = pool->boundary;

    do {
        unsigned int next = offset + pool->size;
        if (unlikely((next + pool->size) >= next_boundary)) {
            next = next_boundary;
            next_boundary += pool->boundary;
        }
        *(int *)(page->vaddr + offset) = next;
        offset = next;
    } while (offset < pool->allocation);
}


static struct dma_page *pool_alloc_page(struct dma_pool *pool)
{
    struct dma_page *page;

    page = malloc(sizeof(*page));
    if (!page)
        return NULL;
    page->vaddr = (void*)KernelAlloc(pool->allocation);

    dbgprintf("%s 0x%0x ",__FUNCTION__, page->vaddr);

    if (page->vaddr)
    {
        page->dma = GetPgAddr(page->vaddr);

        dbgprintf("dma 0x%0x\n", page->dma);

        pool_initialise_page(pool, page);
        list_add(&page->page_list, &pool->page_list);
        page->in_use = 0;
        page->offset = 0;
    } else {
        free(page);
        page = NULL;
    }
    return page;
}

static inline int is_page_busy(struct dma_page *page)
{
    return page->in_use != 0;
}


static void pool_free_page(struct dma_pool *pool, struct dma_page *page)
{
    dma_addr_t dma = page->dma;

    KernelFree(page->vaddr);
    list_del(&page->page_list);
    free(page);
}


/**
 * dma_pool_destroy - destroys a pool of dma memory blocks.
 * @pool: dma pool that will be destroyed
 * Context: !in_interrupt()
 *
 * Caller guarantees that no more memory from the pool is in use,
 * and that nothing will try to use the pool after this call.
 */
void dma_pool_destroy(struct dma_pool *pool)
{
    mutex_lock(&pools_lock);
    list_del(&pool->pools);
    mutex_unlock(&pools_lock);

    while (!list_empty(&pool->page_list)) {
        struct dma_page *page;
        page = list_entry(pool->page_list.next,
                  struct dma_page, page_list);
        if (is_page_busy(page))
        {
            printk(KERN_ERR "dma_pool_destroy %p busy\n",
                   page->vaddr);
            /* leak the still-in-use consistent memory */
            list_del(&page->page_list);
            kfree(page);
        } else
            pool_free_page(pool, page);
    }

    kfree(pool);
}


/**
 * dma_pool_alloc - get a block of consistent memory
 * @pool: dma pool that will produce the block
 * @mem_flags: GFP_* bitmask
 * @handle: pointer to dma address of block
 *
 * This returns the kernel virtual address of a currently unused block,
 * and reports its dma address through the handle.
 * If such a memory block can't be allocated, %NULL is returned.
 */
void *dma_pool_alloc(struct dma_pool *pool, gfp_t mem_flags,
             dma_addr_t *handle)
{
    u32_t   efl;
    struct  dma_page *page;
    size_t  offset;
    void   *retval;

    efl = safe_cli();
 restart:
    list_for_each_entry(page, &pool->page_list, page_list) {
        if (page->offset < pool->allocation)
            goto ready;
    }
    page = pool_alloc_page(pool);
    if (!page)
    {
        retval = NULL;
        goto done;
    }

 ready:
    page->in_use++;
    offset = page->offset;
    page->offset = *(int *)(page->vaddr + offset);
    retval = offset + page->vaddr;
    *handle = offset + page->dma;
 done:
    safe_sti(efl);
    return retval;
}



static struct dma_page *pool_find_page(struct dma_pool *pool, dma_addr_t dma)
{
    struct dma_page *page;
    u32_t  efl;

    efl = safe_cli();

    list_for_each_entry(page, &pool->page_list, page_list) {
        if (dma < page->dma)
            continue;
        if (dma < (page->dma + pool->allocation))
            goto done;
    }
    page = NULL;
 done:
    safe_sti(efl);

    return page;
}

/**
 * dma_pool_free - put block back into dma pool
 * @pool: the dma pool holding the block
 * @vaddr: virtual address of block
 * @dma: dma address of block
 *
 * Caller promises neither device nor driver will again touch this block
 * unless it is first re-allocated.
 */
void dma_pool_free(struct dma_pool *pool, void *vaddr, dma_addr_t dma)
{
    struct dma_page *page;
    unsigned long flags;
    unsigned int offset;

    u32_t efl;

    page = pool_find_page(pool, dma);
    if (!page) {
        printk(KERN_ERR "dma_pool_free %p/%lx (bad dma)\n",
               vaddr, (unsigned long)dma);
        return;
    }

    offset = vaddr - page->vaddr;

    efl = safe_cli();
    {
        page->in_use--;
        *(int *)vaddr = page->offset;
        page->offset = offset;
    /*
     * Resist a temptation to do
     *    if (!is_page_busy(page)) pool_free_page(pool, page);
     * Better have a few empty pages hang around.
     */
    }safe_sti(efl);
}

