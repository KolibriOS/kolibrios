
#include <atomic.h>

#ifdef USE_SMP

typedef struct
{
	atomic_t val;
} spinlock_t;

/*
 * SPINLOCK_DECLARE is to be used for dynamically allocated spinlocks,
 * where the lock gets initialized in run time.
 */

#define SPINLOCK_DECLARE(slname)  spinlock_t slname

/*
 * SPINLOCK_INITIALIZE is to be used for statically allocated spinlocks.
 * It declares and initializes the lock.
 */

#define SPINLOCK_INITIALIZE(slname)   \
	spinlock_t slname = { 		\
		.val = { 0 }		\
	}

extern void spinlock_initialize(spinlock_t *sl);
extern int spinlock_trylock(spinlock_t *sl);

#define spinlock_lock(x) atomic_lock_arch(&(x)->val)

/** Unlock spinlock
 *
 * Unlock spinlock.
 *
 * @param sl Pointer to spinlock_t structure.
 */
static inline void spinlock_unlock(spinlock_t *sl)
{
  ASSERT(atomic_get(&sl->val) != 0);

	/*
	 * Prevent critical section code from bleeding out this way down.
	 */
 // CS_LEAVE_BARRIER();

	atomic_set(&sl->val, 0);
//  preemption_enable();
}

#else

/* On UP systems, spinlocks are effectively left out. */
#define SPINLOCK_DECLARE(name)
#define SPINLOCK_EXTERN(name)
#define SPINLOCK_INITIALIZE(name)

#define spinlock_initialize(x)
#define spinlock_lock(x)
#define spinlock_trylock(x)
#define spinlock_unlock(x)

#endif

