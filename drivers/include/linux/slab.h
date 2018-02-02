/*
 * Written by Mark Hemment, 1996 (markhe@nextd.demon.co.uk).
 *
 * (C) SGI 2006, Christoph Lameter
 * 	Cleaned up and restructured to ease the addition of alternative
 * 	implementations of SLAB allocators.
 * (C) Linux Foundation 2008-2013
 *      Unified interface for all slab allocators
 */

#ifndef _LINUX_SLAB_H
#define	_LINUX_SLAB_H

#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/workqueue.h>


/*
 * Flags to pass to kmem_cache_create().
 * The ones marked DEBUG are only valid if CONFIG_DEBUG_SLAB is set.
 */
#define SLAB_CONSISTENCY_CHECKS	0x00000100UL	/* DEBUG: Perform (expensive) checks on alloc/free */
#define SLAB_RED_ZONE		0x00000400UL	/* DEBUG: Red zone objs in a cache */
#define SLAB_POISON		0x00000800UL	/* DEBUG: Poison objects */
#define SLAB_HWCACHE_ALIGN	0x00002000UL	/* Align objs on cache lines */
#define SLAB_CACHE_DMA		0x00004000UL	/* Use GFP_DMA memory */
#define SLAB_STORE_USER		0x00010000UL	/* DEBUG: Store the last owner for bug hunting */
#define SLAB_PANIC		0x00040000UL	/* Panic if kmem_cache_create() fails */
/*
 * SLAB_DESTROY_BY_RCU - **WARNING** READ THIS!
 *
 * This delays freeing the SLAB page by a grace period, it does _NOT_
 * delay object freeing. This means that if you do kmem_cache_free()
 * that memory location is free to be reused at any time. Thus it may
 * be possible to see another object there in the same RCU grace period.
 *
 * This feature only ensures the memory location backing the object
 * stays valid, the trick to using this is relying on an independent
 * object validation pass. Something like:
 *
 *  rcu_read_lock()
 * again:
 *  obj = lockless_lookup(key);
 *  if (obj) {
 *    if (!try_get_ref(obj)) // might fail for free objects
 *      goto again;
 *
 *    if (obj->key != key) { // not the object we expected
 *      put_ref(obj);
 *      goto again;
 *    }
 *  }
 *  rcu_read_unlock();
 *
 * This is useful if we need to approach a kernel structure obliquely,
 * from its address obtained without the usual locking. We can lock
 * the structure to stabilize it and check it's still at the given address,
 * only if we can be sure that the memory has not been meanwhile reused
 * for some other kind of object (which our subsystem's lock might corrupt).
 *
 * rcu_read_lock before reading the address, then rcu_read_unlock after
 * taking the spinlock within the structure expected at that address.
 */
#define SLAB_DESTROY_BY_RCU	0x00080000UL	/* Defer freeing slabs to RCU */
#define SLAB_MEM_SPREAD		0x00100000UL	/* Spread some memory over cpuset */
#define SLAB_TRACE		0x00200000UL	/* Trace allocations and frees */

/* Flag to prevent checks on free */
#ifdef CONFIG_DEBUG_OBJECTS
# define SLAB_DEBUG_OBJECTS	0x00400000UL
#else
# define SLAB_DEBUG_OBJECTS	0x00000000UL
#endif

#define SLAB_NOLEAKTRACE	0x00800000UL	/* Avoid kmemleak tracing */

/* Don't track use of uninitialized memory */
#ifdef CONFIG_KMEMCHECK
# define SLAB_NOTRACK		0x01000000UL
#else
# define SLAB_NOTRACK		0x00000000UL
#endif
#ifdef CONFIG_FAILSLAB
# define SLAB_FAILSLAB		0x02000000UL	/* Fault injection mark */
#else
# define SLAB_FAILSLAB		0x00000000UL
#endif
#if defined(CONFIG_MEMCG) && !defined(CONFIG_SLOB)
# define SLAB_ACCOUNT		0x04000000UL	/* Account to memcg */
#else
# define SLAB_ACCOUNT		0x00000000UL
#endif

#ifdef CONFIG_KASAN
#define SLAB_KASAN		0x08000000UL
#else
#define SLAB_KASAN		0x00000000UL
#endif

/* The following flags affect the page allocator grouping pages by mobility */
#define SLAB_RECLAIM_ACCOUNT	0x00020000UL		/* Objects are reclaimable */
#define SLAB_TEMPORARY		SLAB_RECLAIM_ACCOUNT	/* Objects are short-lived */
/*
 * ZERO_SIZE_PTR will be returned for zero sized kmalloc requests.
 *
 * Dereferencing ZERO_SIZE_PTR will lead to a distinct access fault.
 *
 * ZERO_SIZE_PTR can be passed to kfree though in the same way that NULL can.
 * Both make kfree a no-op.
 */
#define ZERO_SIZE_PTR ((void *)16)

#define ZERO_OR_NULL_PTR(x) ((unsigned long)(x) <= \
				(unsigned long)ZERO_SIZE_PTR)

void __init kmem_cache_init(void);
bool slab_is_available(void);

struct kmem_cache *kmem_cache_create(const char *, size_t, size_t,
			unsigned long,
			void (*)(void *));
void kmem_cache_destroy(struct kmem_cache *);
int kmem_cache_shrink(struct kmem_cache *);
void kmem_cache_free(struct kmem_cache *, void *);

static inline void *krealloc(const void *p, size_t new_size, gfp_t flags)
{
    return __builtin_realloc((void*)p, new_size);
}

static inline void kfree(const void *p)
{
    __builtin_free((void*)p);
}
static __always_inline void *kmalloc(size_t size, gfp_t flags)
{
    void *ret = __builtin_malloc(size);
    memset(ret, 0, size);
    return ret;
}

/**
 * kzalloc - allocate memory. The memory is set to zero.
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kzalloc(size_t size, gfp_t flags)
{
    void *ret = __builtin_malloc(size);
    memset(ret, 0, size);
    return ret;
}

static inline void *kcalloc(size_t n, size_t size, uint32_t flags)
{
    return (void*)kzalloc(n * size, 0);
}

static inline void *kmalloc_array(size_t n, size_t size, gfp_t flags)
{
//    if (size != 0 && n > SIZE_MAX / size)
//        return NULL;
    return (void*)kmalloc(n * size, flags);
}

#endif	/* _LINUX_SLAB_H */
