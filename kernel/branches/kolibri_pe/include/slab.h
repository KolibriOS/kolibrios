
typedef struct {
	link_t link;
  count_t busy;        /**< Count of full slots in magazine */
  count_t size;        /**< Number of slots in magazine */
  void *objs[];        /**< Slots in magazine */
} slab_magazine_t;

typedef struct {
	slab_magazine_t *current;
	slab_magazine_t *last;
	SPINLOCK_DECLARE(lock);
} slab_mag_cache_t;

typedef struct {

	link_t link;

	/* Configuration */
	/** Size of slab position - align_up(sizeof(obj)) */
	size_t size;

//  int (*constructor)(void *obj, int kmflag);
//  int (*destructor)(void *obj);

	/** Flags changing behaviour of cache */
	int flags;

	/* Computed values */
  u32_t order;                /**< Order of frames to be allocated */
  unsigned int objects;        /**< Number of objects that fit in */

	/* Statistics */
	atomic_t allocated_slabs;
	atomic_t allocated_objs;
	atomic_t cached_objs;
	/** How many magazines in magazines list */
	atomic_t magazine_counter;

	/* Slabs */
  link_t full_slabs;           /**< List of full slabs */
  link_t partial_slabs;        /**< List of partial slabs */
	SPINLOCK_DECLARE(slablock);
	/* Magazines  */
  link_t magazines;            /**< List o full magazines */
	SPINLOCK_DECLARE(maglock);

	/** CPU cache */
	slab_mag_cache_t *mag_cache;
} slab_cache_t;

typedef struct {
  link_t link;                 /**< List of full/partial slabs. */
  slab_cache_t *cache;         /**< Pointer to parent cache. */
  count_t available;           /**< Count of available items in this slab. */
  void *start;                 /**< Start address of first item. */
  void *nextavail;             /**< The index of next available item. */
} slab_t;

#define SLAB_INSIDE_SIZE   (4096 >> 3)

/** Maximum wasted space we allow for cache */
#define SLAB_MAX_BADNESS(cache)   (((size_t) PAGE_SIZE << (cache)->order) >> 2)

 /** Do not use per-cpu cache */
#define SLAB_CACHE_NOMAGAZINE 0x1
/** Have control structure inside SLAB */
#define SLAB_CACHE_SLINSIDE   0x2
/** We add magazine cache later, if we have this flag */
#define SLAB_CACHE_MAGDEFERRED (0x4 | SLAB_CACHE_NOMAGAZINE)


slab_cache_t * slab_cache_create(
				 size_t size,
				 size_t align,
				 int (*constructor)(void *obj, int kmflag),
				 int (*destructor)(void *obj),
         int flags);

void* __fastcall slab_alloc(slab_cache_t *cache, int flags);
