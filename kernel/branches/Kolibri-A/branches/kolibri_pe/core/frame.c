
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>

extern u32_t pg_balloc;
extern u32_t mem_amount;

void __fastcall *balloc(size_t size);

static zone_t z_core;

#include "buddy.inc"

typedef struct
{
    link_t    link;
    SPINLOCK_DECLARE(lock);
    u32_t     state;
    void     *parent;
    count_t   avail;
    addr_t    base;
    index_t   next;
    int       list[512];
}pslab_t;

typedef struct
{
    SPINLOCK_DECLARE(lock);

    count_t  partial_count;

    link_t   full_slabs;           /**< List of full slabs */
    link_t   partial_slabs;        /**< List of partial slabs */
}pcache_t;

static pcache_t page_cache;

static pslab_t *create_page_slab();



void init_mm()
{
    int i;

    u32_t     base;
    u32_t     size;
    count_t   pages;
    size_t    conf_size;
    size_t    core_size;
    pslab_t  *slab;

    pages = mem_amount >> PAGE_WIDTH;
    DBG("last page = %x total pages =  %x\n",mem_amount, pages);

    conf_size = pages*sizeof(frame_t);
    DBG("conf_size = %x  free mem start =%x\n",conf_size, pg_balloc);

    zone_create(&z_core, 0,  pages);
    zone_release(&z_core, 0, pages);
    zone_reserve(&z_core, 0, pg_balloc >> PAGE_WIDTH);

    list_initialize(&page_cache.full_slabs);
    list_initialize(&page_cache.partial_slabs);

    slab = create_page_slab();

    ASSERT(slab);

    slab->parent = &page_cache;
    page_cache.partial_count++;
    list_prepend(&slab->link, &page_cache.partial_slabs);
};

/** Return wasted space in slab */
static unsigned int badness(index_t order, size_t size)
{
	unsigned int objects;
	unsigned int ssize;

    ssize = PAGE_SIZE << order;
    objects = (PAGE_SIZE << order) / size;
	return ssize - objects * size;
}

#define SLAB_MAX_BADNESS(order)   (((size_t) PAGE_SIZE << (order)) >> 2)


static pslab_t *create_page_slab()
{
    pslab_t  *slab;
    link_t   *tmp;

    spinlock_lock(&z_core.lock);

    tmp = buddy_alloc(9);

    if( tmp != 0 )
    {
        frame_t     *frame;
        int          i;
        addr_t       v;

	/* Update zone information. */
        z_core.free_count -= 512;
        z_core.busy_count += 512;

        spinlock_unlock(&z_core.lock);

	/* Frame will be actually a first frame of the block. */
        frame = (frame_t*)tmp;

        frame->parent = 0;

        v = (z_core.base + (index_t)(frame - z_core.frames)) << PAGE_WIDTH;

        slab = (pslab_t*)PA2KA(v);

        for(i = 1; i < 512; i++)
                frame[i].parent = slab;

        slab->base = v + PAGE_SIZE;

        slab->avail = 511;
        slab->next  = 0;

        for(i = 0; i < 511; i++)
            slab->list[i] = i + 1;

    }
    else
    {
        spinlock_unlock(&z_core.lock);
        slab = NULL;
    };

    DBG("create page slab at %x\n", slab);

	return slab;
}

static void destroy_page_slab(pslab_t *slab)
{
    u32_t     order;
    count_t   idx;
    frame_t  *frame;


    idx =  (KA2PA(slab) >> PAGE_WIDTH)-z_core.base;

    frame = &z_core.frames[idx];

	/* remember frame order */
	order = frame->buddy_order;

    ASSERT(frame->refcount);

    if (!--frame->refcount)
    {
        spinlock_lock(&z_core.lock);

        buddy_system_free(&frame->buddy_link);

    /* Update zone information. */
        z_core.free_count += (1 << order);
        z_core.busy_count -= (1 << order);

        spinlock_unlock(&z_core.lock);
    }
}

#if 0
fslab_t *create_slab(index_t order, size_t size)
{
    fslab_t *slab;

    slab = (fslab_t*)PA2KA(frame_alloc(0));

    if( slab )
    {
        link_t      *tmp;

        spinlock_lock(&z_core.lock);

        tmp = buddy_alloc(order);
        ASSERT(tmp);

        if( tmp )
        {
            frame_t     *frame;
            count_t      objects;
            count_t      i;
            addr_t       v;

	/* Update zone information. */
            z_core.free_count -= (1 << order);
            z_core.busy_count += (1 << order);

            spinlock_unlock(&z_heap.lock);

	/* Frame will be actually a first frame of the block. */
            frame = (frame_t*)tmp;

            for(i = 0; i < (1U<<order); i++)
                frame[i].parent = slab;

	/* get frame address */
            v = z_core.base + (index_t)(frame - z_core.frames);

            slab->base = (v << PAGE_WIDTH);

            slab->avail = (PAGE_SIZE << order) / size;
            slab->next  = 0;

            objects = (PAGE_SIZE << order) / size;

            for(i = 0; i < objects; i++)
                slab->list[i] = i + 1;
        }
        else
        {
            spinlock_unlock(&z_core.lock);
            frame_free(KA2PA(slab));
            slab = NULL;
        };
    };

	return slab;
}

static void destroy_slab(fslab_t *slab)
{
    u32_t order;
    count_t idx;
    frame_t *frame;

    idx =   (slab->base >> PAGE_WIDTH)-z_core.base;
    frame = &z_core.frames[idx];

	/* remember frame order */
	order = frame->buddy_order;

    ASSERT(frame->refcount);

    if (!--frame->refcount)
    {
        spinlock_lock(&z_core.lock);

        buddy_system_free(&frame->buddy_link);

    /* Update zone information. */
        z_core.free_count += (1 << order);
        z_core.busy_count -= (1 << order);

        spinlock_unlock(&z_core.lock);
    }

//    slab_free(fslab, slab);

};
#endif

addr_t alloc_page(void)
{
    eflags_t  efl;
    pslab_t  *slab;
    addr_t    frame;

    efl = safe_cli();

    spinlock_lock(&page_cache.lock);

    if (list_empty(&page_cache.partial_slabs))
    {
        slab = create_page_slab();
        if (!slab)
        {
            spinlock_unlock(&page_cache.lock);
            safe_sti(efl);
            return 0;
        }
        slab->parent = &page_cache;
        slab->state  = 1;
        page_cache.partial_count++;
        list_prepend(&slab->link, &page_cache.partial_slabs);
    }
    else
        slab = (pslab_t*)page_cache.partial_slabs.next;

    frame = slab->base + (slab->next << PAGE_WIDTH);
    slab->next = slab->list[slab->next];

    slab->avail--;
    if( slab->avail == 0 )
    {
        slab->state  = 0;
        list_remove(&slab->link);
        list_prepend(&slab->link, &page_cache.full_slabs);
        page_cache.partial_count--;
        DBG("%s insert empty page slab\n");
    };
    spinlock_unlock(&page_cache.lock);

//    DBG("alloc_page: %x   remain  %d\n", frame, slab->avail);

    safe_sti(efl);

    return frame;
}


addr_t __fastcall frame_alloc(count_t count)
{
    addr_t    frame;

    if ( count > 1)
    {
        eflags_t  efl;
        index_t   order;
        frame_t  *tmp;
        count_t   i;

        order = fnzb(count-1)+1;

        efl = safe_cli();

        spinlock_lock(&z_core.lock);

        tmp = (frame_t*)buddy_alloc( order );

        ASSERT(tmp);

        z_core.free_count -= (1 << order);
        z_core.busy_count += (1 << order);

        for(i = 0; i < (1 << order); i++)
            tmp[i].parent = NULL;

        spinlock_unlock(&z_core.lock);

        safe_sti(efl);

        frame = (z_core.base +
                (index_t)(tmp - z_core.frames)) << PAGE_WIDTH;


        DBG("%s %x order %d remain %d\n", __FUNCTION__,
             frame, order, z_core.free_count);
    }
    else
        frame = alloc_page();

    return frame;
}

size_t __fastcall frame_free(addr_t addr)
{
    eflags_t  efl;
    index_t   idx;
    frame_t  *frame;
    size_t    frame_size;

    idx = addr >> PAGE_WIDTH;

    if( (idx < z_core.base) ||
        (idx >= z_core.base+z_core.count)) {
        DBG("%s: invalid address %x\n", __FUNCTION__, addr);
        return 0;
    }

    efl = safe_cli();

    frame = &z_core.frames[idx-z_core.base];

    if( frame->parent != NULL )
    {
        pslab_t  *slab;

        slab = frame->parent;

        spinlock_lock(&page_cache.lock);

        idx = (addr - slab->base) >> PAGE_WIDTH;

        ASSERT(idx < 512);

        slab->list[idx] = slab->next;
        slab->next = idx;

        slab->avail++;

        if(  (slab->state == 0 ) &&
             (slab->avail >= 4))
        {
            slab->state = 1;
     //       list_remove(&slab->link);
     //       list_prepend(&slab->link, &page_cache.partial_slabs);
     //       page_cache.partial_count++;

            DBG("%s: insert partial page slab\n", __FUNCTION__);
        }
        spinlock_unlock(&page_cache.lock);

        frame_size = 1;
    }
    else
    {
        count_t   order;

        order = frame->buddy_order;

        DBG("%s %x order %d\n", __FUNCTION__, addr, order);

        ASSERT(frame->refcount);

        spinlock_lock(&z_core.lock);

        if (!--frame->refcount)
        {
            buddy_system_free(&frame->buddy_link);

        /* Update zone information. */
            z_core.free_count += (1 << order);
            z_core.busy_count -= (1 << order);
        }
        spinlock_unlock(&z_core.lock);

        frame_size = 1 << order;
    };
    safe_sti(efl);

    return frame_size;
}

count_t get_free_mem()
{
   return z_core.free_count;
}

