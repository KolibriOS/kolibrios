
#include <types.h>
#include <spinlock.h>
#include <core.h>

#ifdef USE_SMP

void spinlock_initialize(spinlock_t *sl)
{
	atomic_set(&sl->val, 0);
}

#endif
