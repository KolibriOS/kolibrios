/*
 * kernel/locking/mutex.c
 *
 * Mutexes: blocking mutual exclusion locks
 *
 * Started by Ingo Molnar:
 *
 *  Copyright (C) 2004, 2005, 2006 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 *
 * Many thanks to Arjan van de Ven, Thomas Gleixner, Steven Rostedt and
 * David Howells for suggestions and improvements.
 *
 *  - Adaptive spinning for mutexes by Peter Zijlstra. (Ported to mainline
 *    from the -rt tree, where it was originally implemented for rtmutexes
 *    by Steven Rostedt, based on work by Gregory Haskins, Peter Morreale
 *    and Sven Dietrich.
 *
 * Also see Documentation/mutex-design.txt.
 */
#include <linux/lockdep.h>
#include <linux/mutex.h>
#include <linux/ww_mutex.h>
#include <linux/sched.h>
#include <linux/export.h>
#include <linux/spinlock.h>
#include <syscall.h>
/*
 * A negative mutex count indicates that waiters are sleeping waiting for the
 * mutex.
 */
#define MUTEX_SHOW_NO_WAITER(mutex) (atomic_read(&(mutex)->count) >= 0)

void
__mutex_init(struct mutex *lock, const char *name, struct lock_class_key *key)
{
    atomic_set(&lock->count, 1);
//    spin_lock_init(&lock->wait_lock);
    INIT_LIST_HEAD(&lock->wait_list);
//    mutex_clear_owner(lock);
#ifdef CONFIG_MUTEX_SPIN_ON_OWNER
    lock->osq = NULL;
#endif

}

static __always_inline void ww_mutex_lock_acquired(struct ww_mutex *ww,
                           struct ww_acquire_ctx *ww_ctx)
{
#ifdef CONFIG_DEBUG_MUTEXES
    /*
     * If this WARN_ON triggers, you used ww_mutex_lock to acquire,
     * but released with a normal mutex_unlock in this call.
     *
     * This should never happen, always use ww_mutex_unlock.
     */
    DEBUG_LOCKS_WARN_ON(ww->ctx);

    /*
     * Not quite done after calling ww_acquire_done() ?
     */
    DEBUG_LOCKS_WARN_ON(ww_ctx->done_acquire);

    if (ww_ctx->contending_lock) {
        /*
         * After -EDEADLK you tried to
         * acquire a different ww_mutex? Bad!
         */
        DEBUG_LOCKS_WARN_ON(ww_ctx->contending_lock != ww);

        /*
         * You called ww_mutex_lock after receiving -EDEADLK,
         * but 'forgot' to unlock everything else first?
         */
        DEBUG_LOCKS_WARN_ON(ww_ctx->acquired > 0);
        ww_ctx->contending_lock = NULL;
    }

    /*
     * Naughty, using a different class will lead to undefined behavior!
     */
    DEBUG_LOCKS_WARN_ON(ww_ctx->ww_class != ww->ww_class);
#endif
    ww_ctx->acquired++;
}

void ww_mutex_unlock(struct ww_mutex *lock)
{
    /*
     * The unlocking fastpath is the 0->1 transition from 'locked'
     * into 'unlocked' state:
     */
    if (lock->ctx) {
        if (lock->ctx->acquired > 0)
            lock->ctx->acquired--;
        lock->ctx = NULL;
    }
    MutexUnlock(&lock->base);
}

int __ww_mutex_lock(struct ww_mutex *lock, struct ww_acquire_ctx *ctx)
{
    MutexLock(&lock->base);
    ww_mutex_lock_acquired(lock, ctx);
    lock->ctx = ctx;

    return 0;
}


int __ww_mutex_lock_interruptible(struct ww_mutex *lock, struct ww_acquire_ctx *ctx)
{
    MutexLock(&lock->base);
    ww_mutex_lock_acquired(lock, ctx);
    lock->ctx = ctx;

    return 0;
}
