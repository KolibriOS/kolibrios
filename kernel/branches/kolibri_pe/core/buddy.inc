
#define BUDDY_SYSTEM_INNER_BLOCK  0xff

#define frame_index( frame ) \
      (index_t)( (frame) - z_core.frames)

#define frame_initialize( frame ) \
    (frame)->refcount = 1;          \
    (frame)->buddy_order = 0

#define buddy_get_order( block) \
    ((frame_t*)(block))->buddy_order

#define buddy_set_order( block, order) \
     ((frame_t*)(block))->buddy_order = (order)

#define buddy_mark_busy( block ) \
    ((frame_t*)(block))->refcount = 1

#define IS_BUDDY_LEFT_BLOCK(frame)  \
  (((frame_index((frame)) >> (frame)->buddy_order) & 0x1) == 0)

#define IS_BUDDY_RIGHT_BLOCK(frame) \
    (((frame_index((frame)) >> (frame)->buddy_order) & 0x1) == 1)

#define buddy_mark_available( block ) \
    ((frame_t*)(block))->refcount = 0


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

static link_t *find_buddy(link_t *block)
{
    frame_t  *frame;
    index_t   index;
    u32_t     is_left, is_right;

    frame = (frame_t*)block;
 //   ASSERT(IS_BUDDY_ORDER_OK(frame_index_abs(zone, frame),frame->buddy_order));

    is_left = IS_BUDDY_LEFT_BLOCK( frame);
    is_right = IS_BUDDY_RIGHT_BLOCK( frame);

 //   ASSERT(is_left ^ is_right);
    if (is_left) {
        index = (frame_index(frame)) + (1 << frame->buddy_order);
    } else {  /* if (is_right) */
        index = (frame_index(frame)) - (1 << frame->buddy_order);
    }

    if ( index < z_core.count)
    {
        if (z_core.frames[index].buddy_order == frame->buddy_order &&
            z_core.frames[index].refcount == 0) {
            return &z_core.frames[index].buddy_link;
      }
    }

    return NULL;
}


static link_t *buddy_find_block(link_t *child, u32_t order)
{
    frame_t *frame;
    index_t index;

    frame = (frame_t*)child;

    index = frame_index(frame);
    do {
        if (z_core.frames[index].buddy_order != order)
            return &z_core.frames[index].buddy_link;

    } while(index-- > 0);
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

  //  ASSERT(i <= z->max_order);

    if (i != z_core.max_order)
    {
		/*
		 * See if there is any buddy in the list of order i.
		 */
        buddy = find_buddy( block );
        if (buddy)
		{

        //    ASSERT(buddy_get_order(z, buddy) == i);
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
    list_append(block, &z_core.order[i]);
}


static link_t* buddy_alloc( u32_t i)
{
	link_t *res, *hlp;

    ASSERT(i <= z_core.max_order);

	/*
	 * If the list of order i is not empty,
	 * the request can be immediatelly satisfied.
	 */
    if (!list_empty(&z_core.order[i])) {
        res = z_core.order[i].next;
		list_remove(res);
		buddy_mark_busy(res);
		return res;
	}
	/*
	 * If order i is already the maximal order,
	 * the request cannot be satisfied.
	 */
    if (i == z_core.max_order)
		return NULL;

	/*
	 * Try to recursively satisfy the request from higher order lists.
	 */
    hlp = buddy_alloc( i + 1 );

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

static link_t* buddy_alloc_block(link_t *block)
{
	link_t *left,*right, *tmp;
    u32_t order;

    left = buddy_find_block(block, BUDDY_SYSTEM_INNER_BLOCK);
    ASSERT(left);
	list_remove(left);
    while (1)
   {
        if ( !buddy_get_order(left))
        {
            buddy_mark_busy(left);
			return left;
		}

        order = buddy_get_order(left);

        right = buddy_bisect(left);
        buddy_set_order(left, order-1);
        buddy_set_order(right, order-1);

        tmp = buddy_find_block( block, BUDDY_SYSTEM_INNER_BLOCK);

        if (tmp == right) {
            right = left;
            left = tmp;
        }
        ASSERT(tmp == left);
        buddy_mark_busy(left);
        buddy_system_free(right);
        buddy_mark_available(left);
	}
}

static void zone_create(zone_t *z, pfn_t start, count_t count)
{
    unsigned int i;

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

    for (i = 0; i < count; i++)
		frame_initialize(&z->frames[i]);

/*
    for (i = 0; i < count; i++) {
        z_core.frames[i].buddy_order=0;
        z_core.frames[i].parent = NULL;
        z_core.frames[i].refcount=1;
	}

    for (i = 0; i < count; i++)
    {
        z_core.frames[i].refcount = 0;
        buddy_system_free(&z_core.frames[i].buddy_link);
    }
*/

    DBG("create zone: base %x count %x order %d\n",
         start, count, z->max_order);

}

static void zone_mark_unavailable(zone_t *zone, index_t frame_idx)
{
	frame_t *frame;
	link_t *link;

    ASSERT(frame_idx < zone->count);

    frame = &zone->frames[frame_idx];

	if (frame->refcount)
		return;
    link = buddy_alloc_block( &frame->buddy_link);
    ASSERT(link);
	zone->free_count--;
}

static void zone_reserve(zone_t *z, pfn_t base, count_t count)
{
    int i;
    pfn_t top = base + count;

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
        buddy_system_free(&z->frames[i-z->base].buddy_link);
    }
};

static inline frame_t * zone_get_frame(zone_t *zone, index_t frame_idx)
{
    ASSERT(frame_idx < zone->count);
	return &zone->frames[frame_idx];
}

void __fastcall frame_set_parent(pfn_t pfn, void *data)
{
  spinlock_lock(&z_core.lock);
  zone_get_frame(&z_core, pfn-z_core.base)->parent = data;
  spinlock_unlock(&z_core.lock);
}

void* __fastcall frame_get_parent(pfn_t pfn)
{
	void *res;

    spinlock_lock(&z_core.lock);
    res = zone_get_frame(&z_core, pfn)->parent;
    spinlock_unlock(&z_core.lock);

	return res;
}

