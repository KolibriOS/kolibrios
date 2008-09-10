
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>


extern zone_t z_core;


static LIST_INITIALIZE(slab_cache_list);

static slab_cache_t *slab_cache;

static slab_cache_t slab_cache_cache;

static slab_t *slab_create();

static slab_cache_t * slab_cache_alloc();

void slab_free(slab_cache_t *cache, void *obj);


/**
 * Allocate frames for slab space and initialize
 *
 */
static slab_t * slab_space_alloc(slab_cache_t *cache, int flags)
{
  void *data;
  slab_t *slab;
  size_t fsize;
  unsigned int i;
  u32_t p;

  data = (void*)PA2KA(core_alloc(cache->order));
  if (!data) {
    return NULL;
  }
  slab = (slab_t*)slab_create();
  if (!slab) {
    core_free(KA2PA(data));
      return NULL;
  }

  /* Fill in slab structures */
  for (i = 0; i < ((u32_t) 1 << cache->order); i++)
    frame_set_parent(ADDR2PFN(KA2PA(data)) + i, slab);

  slab->start = data;
  slab->available = cache->objects;
  slab->nextavail = (void*)data;
  slab->cache = cache;

  for (i = 0, p = (u32_t)slab->start; i < cache->objects; i++)
  {
    *(addr_t *)p = p+cache->size;
    p = p+cache->size;
  };
  atomic_inc(&cache->allocated_slabs);
  return slab;
}

/**
 * Take new object from slab or create new if needed
 *
 * @return Object address or null
 */
static void * slab_obj_create(slab_cache_t *cache, int flags)
{
  slab_t *slab;
  void *obj;

  spinlock_lock(&cache->slablock);

  if (list_empty(&cache->partial_slabs)) {
    /* Allow recursion and reclaiming
     * - this should work, as the slab control structures
     *   are small and do not need to allocate with anything
     *   other than frame_alloc when they are allocating,
     *   that's why we should get recursion at most 1-level deep
     */
    slab = slab_space_alloc(cache, flags);
    if (!slab)
    {
      spinlock_unlock(&cache->slablock);
      return NULL;
    }
  } else {
    slab = list_get_instance(cache->partial_slabs.next, slab_t, link);
    list_remove(&slab->link);
  }

  obj = slab->nextavail;
  slab->nextavail = *(void**)obj;
  slab->available--;

  if (!slab->available)
    list_prepend(&slab->link, &cache->full_slabs);
  else
    list_prepend(&slab->link, &cache->partial_slabs);

  spinlock_unlock(&cache->slablock);

//  if (cache->constructor && cache->constructor(obj, flags)) {
    /* Bad, bad, construction failed */
//    slab_obj_destroy(cache, obj, slab);
//    return NULL;
//  }
  return obj;
}


/** Map object to slab structure */
static slab_t * obj2slab(void *obj)
{
  return (slab_t *) frame_get_parent(ADDR2PFN(KA2PA(obj)));
}


/** Allocate new object from cache - if no flags given, always returns
    memory */
void* __fastcall slab_alloc(slab_cache_t *cache, int flags)
{
   eflags_t efl;
   void *result = NULL;

  /* Disable interrupts to avoid deadlocks with interrupt handlers */
   efl = safe_cli();

 // if (!(cache->flags & SLAB_CACHE_NOMAGAZINE)) {
 //   result = magazine_obj_get(cache);
 // }
//  if (!result)
    result = slab_obj_create(cache, flags);

   safe_sti(efl);

//  if (result)
//    atomic_inc(&cache->allocated_objs);

  return result;
}



/**************************************/
/* Slab cache functions */

/** Return number of objects that fit in certain cache size */
static unsigned int comp_objects(slab_cache_t *cache)
{
  if (cache->flags & SLAB_CACHE_SLINSIDE)
    return ((PAGE_SIZE << cache->order) - sizeof(slab_t)) / cache->size;
  else
    return (PAGE_SIZE << cache->order) / cache->size;
}

/** Return wasted space in slab */
static unsigned int badness(slab_cache_t *cache)
{
  unsigned int objects;
  unsigned int ssize;
  size_t val;

  objects = comp_objects(cache);
  ssize = PAGE_SIZE << cache->order;
  if (cache->flags & SLAB_CACHE_SLINSIDE)
    ssize -= sizeof(slab_t);
  val = ssize - objects * cache->size;
  return val;

}


/** Initialize allocated memory as a slab cache */
static void
_slab_cache_create(slab_cache_t *cache,
       size_t size,
       size_t align,
       int (*constructor)(void *obj, int kmflag),
       int (*destructor)(void *obj),
       int flags)
{
  int pages;
 // ipl_t ipl;

//  memsetb((uintptr_t)cache, sizeof(*cache), 0);
//  cache->name = name;

//if (align < sizeof(unative_t))
//    align = sizeof(unative_t);
//  size = ALIGN_UP(size, align);

  cache->size = size;

//  cache->constructor = constructor;
//  cache->destructor = destructor;
  cache->flags = flags;

  list_initialize(&cache->full_slabs);
  list_initialize(&cache->partial_slabs);
  list_initialize(&cache->magazines);
//  spinlock_initialize(&cache->slablock, "slab_lock");
//  spinlock_initialize(&cache->maglock, "slab_maglock");
//  if (! (cache->flags & SLAB_CACHE_NOMAGAZINE))
//    make_magcache(cache);

  /* Compute slab sizes, object counts in slabs etc. */

  /* Minimum slab order */
  pages = SIZE2FRAMES(cache->size);
  /* We need the 2^order >= pages */
  if (pages == 1)
    cache->order = 0;
  else
    cache->order = fnzb(pages-1)+1;

  while (badness(cache) > SLAB_MAX_BADNESS(cache)) {
    cache->order += 1;
  }
  cache->objects = comp_objects(cache);

  /* Add cache to cache list */
//  ipl = interrupts_disable();
//  spinlock_lock(&slab_cache_lock);

  list_append(&cache->link, &slab_cache_list);

//  spinlock_unlock(&slab_cache_lock);
//  interrupts_restore(ipl);
}

/** Create slab cache  */
slab_cache_t * slab_cache_create(
				 size_t size,
				 size_t align,
				 int (*constructor)(void *obj, int kmflag),
				 int (*destructor)(void *obj),
				 int flags)
{
	slab_cache_t *cache;

	cache = (slab_cache_t*)slab_cache_alloc();
  _slab_cache_create(cache, size, align, constructor, destructor, flags);
	return cache;
}

/**
 * Deallocate space associated with slab
 *
 * @return number of freed frames
 */
static count_t slab_space_free(slab_cache_t *cache, slab_t *slab)
{
	frame_free(KA2PA(slab->start));
	if (! (cache->flags & SLAB_CACHE_SLINSIDE))
    slab_free(slab_cache, slab);

//	atomic_dec(&cache->allocated_slabs);

	return 1 << cache->order;
}

/**
 * Return object to slab and call a destructor
 *
 * @param slab If the caller knows directly slab of the object, otherwise NULL
 *
 * @return Number of freed pages
 */
static count_t slab_obj_destroy(slab_cache_t *cache, void *obj,
				slab_t *slab)
{
	int freed = 0;

	if (!slab)
		slab = obj2slab(obj);

//	ASSERT(slab->cache == cache);

//  if (cache->destructor)
//    freed = cache->destructor(obj);

//	spinlock_lock(&cache->slablock);
//	ASSERT(slab->available < cache->objects);

	*(void**)obj = slab->nextavail;
	slab->nextavail = obj;
	slab->available++;

	/* Move it to correct list */
	if (slab->available == cache->objects) {
		/* Free associated memory */
		list_remove(&slab->link);
//		spinlock_unlock(&cache->slablock);

		return freed + slab_space_free(cache, slab);

	} else if (slab->available == 1) {
		/* It was in full, move to partial */
		list_remove(&slab->link);
		list_prepend(&slab->link, &cache->partial_slabs);
	}
//	spinlock_unlock(&cache->slablock);
	return freed;
}



/** Return object to cache, use slab if known  */
static void _slab_free(slab_cache_t *cache, void *obj, slab_t *slab)
{
//	ipl_t ipl;

//	ipl = interrupts_disable();

//	if ((cache->flags & SLAB_CACHE_NOMAGAZINE) \
	    || magazine_obj_put(cache, obj)) {

		slab_obj_destroy(cache, obj, slab);

//	}
//	interrupts_restore(ipl);
//	atomic_dec(&cache->allocated_objs);
}

/** Return slab object to cache */
void slab_free(slab_cache_t *cache, void *obj)
{
	_slab_free(cache, obj, NULL);
}

static slab_t *slab_create()
{
  slab_t *slab;
  void *obj;
  u32_t p;

//  spinlock_lock(&cache->slablock);

  if (list_empty(&slab_cache->partial_slabs)) {
    /* Allow recursion and reclaiming
     * - this should work, as the slab control structures
     *   are small and do not need to allocate with anything
     *   other than frame_alloc when they are allocating,
     *   that's why we should get recursion at most 1-level deep
     */
//    spinlock_unlock(&cache->slablock);
//    slab = slab_create();

    void *data;
    unsigned int i;

    data = (void*)PA2KA(core_alloc(0));
    if (!data) {
      return NULL;
    }

    slab = (slab_t*)((u32_t)data + PAGE_SIZE - sizeof(slab_t));

    /* Fill in slab structures */
    frame_set_parent(ADDR2PFN(KA2PA(data)), slab);

    slab->start = data;
    slab->available = slab_cache->objects;
    slab->nextavail = (void*)data;
    slab->cache = slab_cache;

    for (i = 0,p = (u32_t)slab->start;i < slab_cache->objects; i++)
    {
      *(int *)p = p+slab_cache->size;
      p = p+slab_cache->size;
    };


    atomic_inc(&slab_cache->allocated_slabs);
//    spinlock_lock(&cache->slablock);
  } else {
    slab = list_get_instance(slab_cache->partial_slabs.next, slab_t, link);
    list_remove(&slab->link);
  }
  obj = slab->nextavail;
  slab->nextavail = *((void**)obj);
  slab->available--;

  if (!slab->available)
    list_prepend(&slab->link, &slab_cache->full_slabs);
  else
    list_prepend(&slab->link, &slab_cache->partial_slabs);

//  spinlock_unlock(&cache->slablock);

  return (slab_t*)obj;
}

static slab_cache_t * slab_cache_alloc()
{
  slab_t *slab;
  void *obj;
  u32_t *p;

  if (list_empty(&slab_cache_cache.partial_slabs)) {
    /* Allow recursion and reclaiming
     * - this should work, as the slab control structures
     *   are small and do not need to allocate with anything
     *   other than frame_alloc when they are allocating,
     *   that's why we should get recursion at most 1-level deep
     */
//    spinlock_unlock(&cache->slablock);
//    slab = slab_create();

    void *data;
    unsigned int i;

    data = (void*)(PA2KA(core_alloc(0)));
    if (!data) {
      return NULL;
    }

    slab = (slab_t*)((u32_t)data + PAGE_SIZE - sizeof(slab_t));

    /* Fill in slab structures */
    frame_set_parent(ADDR2PFN(KA2PA(data)), slab);

    slab->start = data;
    slab->available = slab_cache_cache.objects;
    slab->nextavail = (void*)data;
    slab->cache = &slab_cache_cache;

    for (i = 0,p = (u32_t*)slab->start;i < slab_cache_cache.objects; i++)
    {
      *p = (u32_t)p+slab_cache_cache.size;
      p = (u32_t*)((u32_t)p+slab_cache_cache.size);
    };


    atomic_inc(&slab_cache_cache.allocated_slabs);
//    spinlock_lock(&cache->slablock);
  } else {
    slab = list_get_instance(slab_cache_cache.partial_slabs.next, slab_t, link);
    list_remove(&slab->link);
  }
  obj = slab->nextavail;
  slab->nextavail = *((void**)obj);
  slab->available--;

  if (!slab->available)
    list_prepend(&slab->link, &slab_cache_cache.full_slabs);
  else
    list_prepend(&slab->link, &slab_cache_cache.partial_slabs);

//  spinlock_unlock(&cache->slablock);

  return (slab_cache_t*)obj;
}

void slab_cache_init(void)
{

  _slab_cache_create(&slab_cache_cache, sizeof(slab_cache_t),
                     sizeof(void *), NULL, NULL,
                     SLAB_CACHE_NOMAGAZINE | SLAB_CACHE_SLINSIDE);

	/* Initialize external slab cache */
  slab_cache = slab_cache_create(sizeof(slab_t),
					      0, NULL, NULL,SLAB_CACHE_MAGDEFERRED);
};

