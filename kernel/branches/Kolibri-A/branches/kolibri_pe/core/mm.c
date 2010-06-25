
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>

extern u32_t pg_balloc;

extern u32_t mem_amount;

void __fastcall *balloc(u32_t size);

zone_t z_core;


static void buddy_system_create(zone_t *z);
static void  __fastcall buddy_system_free(zone_t *z, link_t *block);
static void zone_mark_unavailable(zone_t *zone, index_t frame_idx);

static addr_t __fastcall zone_alloc(zone_t *zone, u32_t order);
void __fastcall zone_free(zone_t *zone, pfn_t frame_idx);

size_t buddy_conf_size(int max_order);

static inline void frame_initialize(frame_t *frame);


static void zone_create(zone_t *z, pfn_t start, count_t count);
static void zone_reserve(zone_t *z, pfn_t base, count_t count);
static void zone_release(zone_t *z, pfn_t base, count_t count);


void init_mm()
{
   int i;

   u32_t   base;
   u32_t   size;
   count_t pages;
   size_t  conf_size;
   size_t  core_size;

   pages = mem_amount >> FRAME_WIDTH;
   DBG("last page = %x total pages =  %x\n",mem_amount, pages);

   conf_size = pages*sizeof(frame_t);
   DBG("conf_size = %x  free mem start =%x\n",conf_size, pg_balloc);

   zone_create(&z_core, 0, pages);

   zone_release(&z_core, 0, pages);
   zone_reserve(&z_core, 0, pg_balloc >> FRAME_WIDTH);


#if 0
   core_size = (pg_free+conf_size+1024*1024*5)&(-1024*1024*4);
//   printf("core size = %x core heap = %x\n",core_size,core_size-conf_size-pg_free);

   u32_t p0, p1;
   u32_t b0, b1;

   p0 = core_size>>12;
   p1 = (last_page-core_size)>>12;

   b0 = p0*sizeof(frame_t);
   b1 = p1*sizeof(frame_t);

//   printf("buddy_0: %x pages  conf_size %x\n", p0, b0);
//   printf("buddy_1: %x pages  conf_size %x\n", p1, b1);

   zone_create(&z_core, 0, p0);
   zone_create(&z_user, p0, p1);

//   printf("free mem start = %x\n",pg_balloc);

   for(i = 0; i < mem_counter; i++)
   {
     u32_t page;
     if( mem_table[i].type != 1)
       continue;
     page = (mem_table[i].base+mem_table[i].size)&(~4095);
     if(page > last_page)
        last_page = page;

     zone_release(&z_core,mem_table[i].base>>12, mem_table[i].size>>12);
     zone_release(&z_user,mem_table[i].base>>12, mem_table[i].size>>12);
   };
   zone_reserve(&z_core, 0x100000>>12,(pg_balloc-OS_BASE-0x100000)>>12);
#endif
};

static void zone_create(zone_t *z, pfn_t start, count_t count)
{
	unsigned int i;
//  int znum;

	/* Theoretically we could have here 0, practically make sure
	 * nobody tries to do that. If some platform requires, remove
	 * the assert
	 */
//  ASSERT(confframe);
	/* If conframe is supposed to be inside our zone, then make sure
	 * it does not span kernel & init
	 */

//  printf("create zone: base %x count %x\n", start, count);

  spinlock_initialize(&z->lock);
	z->base = start;
	z->count = count;
	z->free_count = count;
	z->busy_count = 0;

  z->max_order = fnzb(count);

  ASSERT(z->max_order < BUDDY_SYSTEM_INNER_BLOCK);

  for (i = 0; i <= z->max_order; i++)
    list_initialize(&z->order[i]);

  z->frames = (frame_t *)balloc(count*sizeof(frame_t));

	for (i = 0; i < count; i++) {
		frame_initialize(&z->frames[i]);
	}
}

static void zone_reserve(zone_t *z, pfn_t base, count_t count)
{
  int i;
  pfn_t top = base+count;

  if( (base+count < z->base)||(base > z->base+z->count))
    return;

  if(base < z->base)
    base = z->base;

  if(top > z->base+z->count)
     top = z->base+z->count;

  DBG("zone reserve base %x top %x\n", base, top);

  for (i = base; i < top; i++)
    zone_mark_unavailable(z, i - z->base);

};

static void zone_release(zone_t *z, pfn_t base, count_t count)
{
    int i;
    pfn_t top = base+count;

    if( (base+count < z->base)||(base > z->base+z->count))
        return;

    if(base < z->base)
        base = z->base;

    if(top > z->base+z->count)
        top = z->base+z->count;

    DBG("zone release base %x top %x\n", base, top);

    for (i = base; i < top; i++) {
        z->frames[i-z->base].refcount = 0;
        buddy_system_free(z, &z->frames[i-z->base].buddy_link);
    }
};


static inline index_t frame_index(zone_t *zone, frame_t *frame)
{
	return (index_t) (frame - zone->frames);
}

static inline index_t frame_index_abs(zone_t *zone, frame_t *frame)
{
  return (index_t) (frame - zone->frames);
}

static inline int frame_index_valid(zone_t *zone, index_t index)
{
	return (index < zone->count);
}

/** Compute pfn_t from frame_t pointer & zone pointer */
static inline index_t make_frame_index(zone_t *zone, frame_t *frame)
{
	return (frame - zone->frames);
}

static inline void frame_initialize(frame_t *frame)
{
	frame->refcount = 1;
	frame->buddy_order = 0;
}


static link_t *buddy_find_block(zone_t *zone, link_t *child,
    u32_t order)
{
	frame_t *frame;
	index_t index;

  frame = (frame_t*)child;

	index = frame_index(zone, frame);
	do {
		if (zone->frames[index].buddy_order != order) {
			return &zone->frames[index].buddy_link;
		}
	} while(index-- > 0);
	return NULL;
}

static inline link_t * buddy_bisect(zone_t *z, link_t *block) {
	frame_t *frame_l, *frame_r;

  frame_l = (frame_t*)block;
	frame_r = (frame_l + (1 << (frame_l->buddy_order - 1)));

	return &frame_r->buddy_link;
}

static inline u32_t buddy_get_order(zone_t *z, link_t *block) {
  frame_t *frame = (frame_t*)block;
	return frame->buddy_order;
}

static inline void buddy_set_order(zone_t *z, link_t *block,
    u32_t order) {
  frame_t *frame = (frame_t*)block;
	frame->buddy_order = order;
}

static link_t *buddy_coalesce(zone_t *z, link_t *block_1,
    link_t *block_2)
{
	frame_t *frame1, *frame2;

  frame1 = (frame_t*)block_1;
  frame2 = (frame_t*)block_2;

	return frame1 < frame2 ? block_1 : block_2;
}

static inline void buddy_mark_busy(zone_t *z, link_t * block) {
    frame_t * frame = (frame_t*)block;
	frame->refcount = 1;
}

static inline void buddy_mark_available(zone_t *z, link_t *block) {
    frame_t *frame = (frame_t*)block;
	frame->refcount = 0;
}

#define IS_BUDDY_ORDER_OK(index, order)   \
    ((~(((u32_t) -1) << (order)) & (index)) == 0)
#define IS_BUDDY_LEFT_BLOCK_ABS(zone, frame)  \
  (((frame_index_abs((zone), (frame)) >> (frame)->buddy_order) & 0x1) == 0)

#define IS_BUDDY_RIGHT_BLOCK_ABS(zone, frame) \
	(((frame_index_abs((zone), (frame)) >> (frame)->buddy_order) & 0x1) == 1)

static link_t *find_buddy(zone_t *zone, link_t *block)
{
	frame_t *frame;
	index_t index;
    u32_t is_left, is_right;

    frame = (frame_t*)block;
    ASSERT(IS_BUDDY_ORDER_OK(frame_index_abs(zone, frame),frame->buddy_order));

	is_left = IS_BUDDY_LEFT_BLOCK_ABS(zone, frame);
	is_right = IS_BUDDY_RIGHT_BLOCK_ABS(zone, frame);

    ASSERT(is_left ^ is_right);
	if (is_left) {
		index = (frame_index(zone, frame)) + (1 << frame->buddy_order);
	} else { 	/* if (is_right) */
		index = (frame_index(zone, frame)) - (1 << frame->buddy_order);
	}

	if (frame_index_valid(zone, index)) {
		if (zone->frames[index].buddy_order == frame->buddy_order &&
		    zone->frames[index].refcount == 0) {
			return &zone->frames[index].buddy_link;
		}
	}

	return NULL;
}

static link_t* __fastcall buddy_system_alloc_block(zone_t *z, link_t *block)
{
	link_t *left,*right, *tmp;
    u32_t order;

    left = buddy_find_block(z, block, BUDDY_SYSTEM_INNER_BLOCK);
    ASSERT(left);
	list_remove(left);
	while (1) {
        if (! buddy_get_order(z,left)) {
            buddy_mark_busy(z, left);
			return left;
		}

        order = buddy_get_order(z, left);

        right = buddy_bisect(z, left);
        buddy_set_order(z, left, order-1);
        buddy_set_order(z, right, order-1);

        tmp = buddy_find_block(z, block, BUDDY_SYSTEM_INNER_BLOCK);

        if (tmp == right) {
            right = left;
            left = tmp;
        }
        ASSERT(tmp == left);
        buddy_mark_busy(z, left);
        buddy_system_free(z, right);
        buddy_mark_available(z, left);
	}
}

static void __fastcall buddy_system_free(zone_t *z, link_t *block)
{
    link_t *buddy, *hlp;
    u8_t i;

    /*
	 * Determine block's order.
	 */
    i = buddy_get_order(z, block);

    ASSERT(i <= z->max_order);

    if (i != z->max_order) {
		/*
		 * See if there is any buddy in the list of order i.
		 */
        buddy = find_buddy(z, block);
		if (buddy) {

            ASSERT(buddy_get_order(z, buddy) == i);
			/*
			 * Remove buddy from the list of order i.
			 */
			list_remove(buddy);

			/*
			 * Invalidate order of both block and buddy.
			 */
            buddy_set_order(z, block, BUDDY_SYSTEM_INNER_BLOCK);
            buddy_set_order(z, buddy, BUDDY_SYSTEM_INNER_BLOCK);

			/*
			 * Coalesce block and buddy into one block.
			 */
            hlp = buddy_coalesce(z, block, buddy);

			/*
			 * Set order of the coalesced block to i + 1.
			 */
            buddy_set_order(z, hlp, i + 1);

			/*
			 * Recursively add the coalesced block to the list of order i + 1.
			 */
            buddy_system_free(z, hlp);
			return;
		}
	}
	/*
	 * Insert block into the list of order i.
	 */
    list_append(block, &z->order[i]);
}

static inline frame_t * zone_get_frame(zone_t *zone, index_t frame_idx)
{
    ASSERT(frame_idx < zone->count);
	return &zone->frames[frame_idx];
}

static void zone_mark_unavailable(zone_t *zone, index_t frame_idx)
{
	frame_t *frame;
	link_t *link;

	frame = zone_get_frame(zone, frame_idx);
	if (frame->refcount)
		return;
    link = buddy_system_alloc_block(zone, &frame->buddy_link);
    ASSERT(link);
	zone->free_count--;
}

static link_t* __fastcall buddy_system_alloc(zone_t *z, u32_t i)
{
	link_t *res, *hlp;

    ASSERT(i <= z->max_order);

	/*
	 * If the list of order i is not empty,
	 * the request can be immediatelly satisfied.
	 */
	if (!list_empty(&z->order[i])) {
		res = z->order[i].next;
		list_remove(res);
		buddy_mark_busy(z, res);
		return res;
	}
	/*
	 * If order i is already the maximal order,
	 * the request cannot be satisfied.
	 */
	if (i == z->max_order)
		return NULL;

	/*
	 * Try to recursively satisfy the request from higher order lists.
	 */
	hlp = buddy_system_alloc(z, i + 1);

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
	hlp = buddy_bisect(z, res);
	buddy_set_order(z, res, i);
	buddy_set_order(z, hlp, i);

	/*
	 * Return the other half to buddy system. Mark the first part
	 * full, so that it won't coalesce again.
	 */
	buddy_mark_busy(z, res);
	buddy_system_free(z, hlp);

	return res;

}


static __fastcall pfn_t zone_frame_alloc(zone_t *zone, u32_t order)
{
	pfn_t v;
	link_t *tmp;
	frame_t *frame;


	/* Allocate frames from zone buddy system */
	tmp = buddy_system_alloc(zone, order);

    ASSERT(tmp);

	/* Update zone information. */
	zone->free_count -= (1 << order);
	zone->busy_count += (1 << order);

	/* Frame will be actually a first frame of the block. */
	frame = (frame_t*)tmp;

	/* get frame address */
	v = make_frame_index(zone, frame);

	return v;
}


/** Set parent of frame */
void __fastcall frame_set_parent(pfn_t pfn, void *data)
{
/*  zone_t *zone = find_zone_and_lock(pfn, &hint);
  ASSERT(zone);
 */

  spinlock_lock(&z_core.lock);
    zone_get_frame(&z_core, pfn-z_core.base)->parent = data;
  spinlock_unlock(&z_core.lock);
}

void* __fastcall frame_get_parent(pfn_t pfn)
{
//	zone_t *zone = find_zone_and_lock(pfn, &hint);
	void *res;

  spinlock_lock(&z_core.lock);
    res = zone_get_frame(&z_core, pfn)->parent;
  spinlock_unlock(&z_core.lock);

	return res;
}

static inline int to_order(count_t arg)
{
  int n;
  asm volatile (
                "xorl %eax, %eax \n\t"
                "bsr %edx, %eax \n\t"
                "incl %eax"
                :"=a" (n)
                :"d"(arg)
                );
	return n;
}


addr_t __fastcall zone_alloc(zone_t *zone, u32_t order)
{
   eflags_t efl;
   pfn_t v;

   efl = safe_cli();
     spinlock_lock(&zone->lock);
       v = zone_frame_alloc(zone, order);
       v += zone->base;
     spinlock_unlock(&zone->lock);
   safe_sti(efl);

   return (v << FRAME_WIDTH);
}

addr_t  __fastcall core_alloc(u32_t order)
{
   eflags_t efl;
   pfn_t v;

   efl = safe_cli();
     spinlock_lock(&z_core.lock);
       v = zone_frame_alloc(&z_core, order);
     spinlock_unlock(&z_core.lock);
   safe_sti(efl);

   DBG("core alloc at: 0x%x, size 0x%x   remain  %d\n", v << FRAME_WIDTH,
        ((1<<order)<<FRAME_WIDTH), z_core.free_count);

   return (v << FRAME_WIDTH);
};

void __fastcall core_free(addr_t frame)
{
   eflags_t efl;

   DBG("core free  0x%x", frame);

   efl = safe_cli();
     spinlock_lock(&z_core.lock);
 //      zone_free(&z_core, frame>>12);
     spinlock_unlock(&z_core.lock);
   safe_sti(efl);

   DBG("  remain %d\n", z_core.free_count);

}

addr_t alloc_page()                                //obsolete
{
   eflags_t efl;
   u32_t    edx;
   pfn_t v;

   edx = save_edx();
   efl = safe_cli();
     spinlock_lock(&z_core.lock);
       v = zone_frame_alloc(&z_core, 0);
     spinlock_unlock(&z_core.lock);
   safe_sti(efl);

   DBG("alloc_page: %x   remain  %d\n", v << FRAME_WIDTH, z_core.free_count);

   restore_edx(edx);
   return (v << FRAME_WIDTH);
};

void __fastcall zone_free(zone_t *zone, pfn_t frame_idx)
{
	frame_t *frame;
    u32_t order;

	frame = &zone->frames[frame_idx];

	/* remember frame order */
	order = frame->buddy_order;

    ASSERT(frame->refcount);

    if (!--frame->refcount)
    {
		buddy_system_free(zone, &frame->buddy_link);

		/* Update zone information. */
		zone->free_count += (1 << order);
		zone->busy_count -= (1 << order);
	}
}

void frame_free(addr_t frame)           //export
{
   eflags_t efl;
   zone_t *zone;

   efl = safe_cli();
     spinlock_lock(&z_core.lock);
       zone_free(&z_core, frame>>12);
     spinlock_unlock(&z_core.lock);
  safe_sti(efl);
}

count_t get_free_mem()
{
   return z_core.free_count;
}

