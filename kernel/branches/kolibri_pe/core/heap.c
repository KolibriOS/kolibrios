
#define ALLOC_FAST

#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>

#define PG_DEMAND   0x400

#define HF_WIDTH    16
#define HF_SIZE     (1 << HF_WIDTH)

#define BUDDY_SYSTEM_INNER_BLOCK  0xff

static zone_t z_heap;

static link_t  shared_mmap;


#define heap_index( frame ) \
	  (index_t)( (frame) - z_heap.frames)

#define heap_index_abs( frame ) \
	  (index_t)( (frame) - z_heap.frames)


static __inline void frame_initialize(frame_t *frame)
{
	frame->refcount = 1;
	frame->buddy_order = 0;
}

#define buddy_get_order( block) \
    ((frame_t*)(block))->buddy_order


#define buddy_set_order( block, order) \
     ((frame_t*)(block))->buddy_order = (order)

#define buddy_mark_busy( block ) \
    ((frame_t*)(block))->refcount = 1


static __inline link_t * buddy_bisect(link_t *block)
{
    frame_t *frame_l, *frame_r;

    frame_l = (frame_t*)block;
	frame_r = (frame_l + (1 << (frame_l->buddy_order - 1)));

	return &frame_r->buddy_link;
}

static __inline link_t *buddy_coalesce(link_t *block_1, link_t *block_2)
{
	frame_t *frame1, *frame2;

    frame1 = (frame_t*)block_1;
    frame2 = (frame_t*)block_2;

	return frame1 < frame2 ? block_1 : block_2;
}


#define IS_BUDDY_LEFT_BLOCK_ABS(frame)  \
  (((heap_index_abs((frame)) >> (frame)->buddy_order) & 0x1) == 0)

#define IS_BUDDY_RIGHT_BLOCK_ABS(frame) \
	(((heap_index_abs((frame)) >> (frame)->buddy_order) & 0x1) == 1)


static link_t *find_buddy(link_t *block)
{
	frame_t *frame;
	index_t index;
    u32_t is_left, is_right;

    frame = (frame_t*)block;
 //   ASSERT(IS_BUDDY_ORDER_OK(frame_index_abs(zone, frame),frame->buddy_order));

	is_left = IS_BUDDY_LEFT_BLOCK_ABS( frame);
	is_right = IS_BUDDY_RIGHT_BLOCK_ABS( frame);

 //   ASSERT(is_left ^ is_right);

    if (is_left) {
        index = (heap_index(frame)) + (1 << frame->buddy_order);
    }
    else {    /* if (is_right) */
        index = (heap_index(frame)) - (1 << frame->buddy_order);
    };


	if ( index < z_heap.count)
	{
		if (z_heap.frames[index].buddy_order == frame->buddy_order &&
		    z_heap.frames[index].refcount == 0) {
			return &z_heap.frames[index].buddy_link;
		}
	}

	return NULL;
}


static void buddy_system_free(link_t *block)
{
    link_t *buddy, *hlp;
    u32_t i;

    /*
	 * Determine block's order.
	 */
    i = buddy_get_order(block);

    ASSERT(i <= z_heap.max_order);

    if (i != z_heap.max_order)
    {
		/*
		 * See if there is any buddy in the list of order i.
		 */
        buddy = find_buddy( block );
		if (buddy)
		{

            ASSERT(buddy_get_order(buddy) == i);
			/*
			 * Remove buddy from the list of order i.
			 */
			list_remove(buddy);

			/*
			 * Invalidate order of both block and buddy.
			 */
            buddy_set_order(block, BUDDY_SYSTEM_INNER_BLOCK);
            buddy_set_order(buddy, BUDDY_SYSTEM_INNER_BLOCK);

			/*
			 * Coalesce block and buddy into one block.
			 */
            hlp = buddy_coalesce( block, buddy );

			/*
			 * Set order of the coalesced block to i + 1.
			 */
            buddy_set_order(hlp, i + 1);

			/*
			 * Recursively add the coalesced block to the list of order i + 1.
			 */
            buddy_system_free( hlp );
			return;
		}
	}
	/*
	 * Insert block into the list of order i.
	 */
    list_append(block, &z_heap.order[i]);
}


static link_t* buddy_system_alloc( u32_t i)
{
	link_t *res, *hlp;

    ASSERT(i <= z_heap.max_order);

	/*
	 * If the list of order i is not empty,
	 * the request can be immediatelly satisfied.
	 */
	if (!list_empty(&z_heap.order[i])) {
		res = z_heap.order[i].next;
		list_remove(res);
		buddy_mark_busy(res);
		return res;
	}
	/*
	 * If order i is already the maximal order,
	 * the request cannot be satisfied.
	 */
	if (i == z_heap.max_order)
		return NULL;

	/*
	 * Try to recursively satisfy the request from higher order lists.
	 */
	hlp = buddy_system_alloc( i + 1 );

	/*
	 * The request could not be satisfied
	 * from higher order lists.
	 */
	if (!hlp)
		return NULL;

	res = hlp;

	/*
	 * Bisect the block and set order of both of its parts to i.
	 */
	hlp = buddy_bisect( res );

	buddy_set_order(res, i);
	buddy_set_order(hlp, i);

	/*
	 * Return the other half to buddy system. Mark the first part
	 * full, so that it won't coalesce again.
	 */
	buddy_mark_busy(res);
	buddy_system_free( hlp );

	return res;
}


int __fastcall init_heap(addr_t start, size_t size)
{
	count_t i;
    count_t count;

    count = size >> HF_WIDTH;

    ASSERT( start != 0);
    ASSERT( count != 0);

    spinlock_initialize(&z_heap.lock);

    z_heap.base = start >> HF_WIDTH;
	z_heap.count = count;
	z_heap.free_count = count;
	z_heap.busy_count = 0;

    z_heap.max_order = fnzb(count);

    DBG("create heap zone: base %x count %x\n", start, count);

    ASSERT(z_heap.max_order < BUDDY_SYSTEM_INNER_BLOCK);

    for (i = 0; i <= z_heap.max_order; i++)
        list_initialize(&z_heap.order[i]);


    DBG("count %d frame_t %d page_size %d\n",
               count, sizeof(frame_t), PAGE_SIZE);

    z_heap.frames = (frame_t *)PA2KA(frame_alloc( (count*sizeof(frame_t)) >> PAGE_WIDTH ));


    if( z_heap.frames == 0 )
        return 0;


	for (i = 0; i < count; i++) {
		z_heap.frames[i].buddy_order=0;
		z_heap.frames[i].parent = NULL;
		z_heap.frames[i].refcount=1;
	}

    for (i = 0; i < count; i++)
    {
        z_heap.frames[i].refcount = 0;
        buddy_system_free(&z_heap.frames[i].buddy_link);
    }

    list_initialize(&shared_mmap);

	return 1;
}

addr_t  __fastcall mem_alloc(size_t size, u32_t flags)
{
    eflags_t  efl;
    addr_t    heap = 0;

    count_t   order;
    frame_t  *frame;
    index_t   v;
    int i;
    mmap_t   *map;
    count_t   pages;

 //   __asm__ __volatile__ ("xchgw %bx, %bx");

    size = (size + 4095) & ~4095;

    pages = size >> PAGE_WIDTH;

//    map = (mmap_t*)malloc( sizeof(mmap_t) +
//                           sizeof(addr_t) * pages);

    map = (mmap_t*)PA2KA(frame_alloc( (sizeof(mmap_t) +
                           sizeof(addr_t) * pages) >> PAGE_WIDTH));

    if ( map )
    {
    map->size = size;

        order = size >> HF_WIDTH;

        if( order )
            order = fnzb(order - 1) + 1;

        efl = safe_cli();

        spinlock_lock(&z_heap.lock);

        frame = (frame_t*)buddy_system_alloc(order);

        ASSERT( frame );

        if( frame )
        {
            addr_t  mem;

            z_heap.free_count -= (1 << order);
            z_heap.busy_count += (1 << order);

/* get frame address */

            v = z_heap.base + (index_t)(frame - z_heap.frames);

            heap = v << HF_WIDTH;

            map->base = heap;

            for(i = 0; i < (1 << order); i++)
                frame[i].parent = map;

            spinlock_unlock(&z_heap.lock);

            safe_sti(efl);


            addr_t  *pte = &((addr_t*)page_tabs)[heap >> PAGE_WIDTH];
            addr_t  *mpte = &map->pte[0];

            mem = heap;

            if( flags & PG_MAP )
            {
                addr_t  page_frame;

#ifdef  ALLOC_FAST

                while( pages )
                {
                    u32_t   order;

                    asm volatile ("bsrl %1, %0":"=&r"(order):"r"(pages):"cc");
                    asm volatile ("btrl %1, %0" :"=&r"(pages):"r"(order):"cc");

                    page_frame = frame_alloc(1 << order) | (flags & 0xFFF);    /* FIXME check */

                    for(i = 0; i < 1 << order; i++)
                    {
                        *pte++  = 0;
                        *mpte++ = page_frame;

                        asm volatile ( "invlpg (%0)" ::"r" (mem) );
                        mem+=  4096;
                        page_frame+= 4096;
                    };
                }
#else

                page_frame = PG_DEMAND | (flags & 0xFFF);

                while(pages--)
                {
                    *pte++  = 0;
                    *mpte++ = page_frame;
                    asm volatile ( "invlpg (%0)" ::"r" (mem) );
                    mem+=  4096;
                };
#endif
            }
            else
            {
                while(pages--)
                {
                    *pte++  = 0;
                    *mpte++ = 0;

                    asm volatile ( "invlpg (%0)" ::"r" (mem) );
                    mem+=  4096;
                };
            }

            DBG("%s %x size %d order %d\n", __FUNCTION__, heap, size, order);

            return heap;
        }

        spinlock_unlock(&z_heap.lock);

        safe_sti(efl);

        frame_free( KA2PA(map) );
    };

    return 0;
}

void __fastcall mem_free(addr_t addr)
{
    eflags_t     efl;
    frame_t     *frame;
    count_t      idx;

    idx = (addr >> HF_WIDTH);

    if( (idx < z_heap.base) ||
        (idx >= z_heap.base+z_heap.count)) {
        DBG("invalid address %x\n", addr);
        return;
    }

    efl = safe_cli();

    frame = &z_heap.frames[idx-z_heap.base];

    u32_t order = frame->buddy_order;

    DBG("%s %x order %d\n", __FUNCTION__, addr, order);

    ASSERT(frame->refcount);

    spinlock_lock(&z_heap.lock);

    if (!--frame->refcount)
    {
        mmap_t  *map;
        count_t  i;

        map = frame->parent;

        for(i = 0; i < (1 << order); i++)
                frame[i].parent = NULL;

        buddy_system_free(&frame->buddy_link);

		/* Update zone information. */
        z_heap.free_count += (1 << order);
        z_heap.busy_count -= (1 << order);

        spinlock_unlock(&z_heap.lock);
        safe_sti(efl);

        for( i = 0; i < (map->size >> PAGE_WIDTH); )
        {
           i+= frame_free(map->pte[i]);
        }

        frame_free( KA2PA(map) );
    }
    else
    {
        spinlock_unlock(&z_heap.lock);
        safe_sti(efl);
    };
};

void __fastcall heap_fault(addr_t faddr, u32_t code)
{
    index_t   idx;
    frame_t  *frame;
    mmap_t   *map;

    idx = faddr >> HF_WIDTH;

    frame = &z_heap.frames[idx-z_heap.base];

    map = frame->parent;

    ASSERT( faddr >= map->base);

    if( faddr < map->base + map->size)
    {
        addr_t page;

        idx = (faddr - map->base) >> PAGE_WIDTH;

        page = map->pte[idx];

        if( page != 0)
        {
            if( page & PG_DEMAND)
            {
                page &= ~PG_DEMAND;
                page = alloc_page() | (page & 0xFFF);

                map->pte[idx] = page;
            };
            ((addr_t*)page_tabs)[faddr >> PAGE_WIDTH] = page;
        };
    };
};

//#include "mmap.inc"

