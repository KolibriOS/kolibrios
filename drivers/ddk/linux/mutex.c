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

static inline void mutex_set_owner(struct mutex *lock)
{
}

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

static inline int __ww_mutex_lock_check_stamp(struct mutex *lock, struct ww_acquire_ctx *ctx)
{
        struct ww_mutex *ww = container_of(lock, struct ww_mutex, base);
        struct ww_acquire_ctx *hold_ctx = READ_ONCE(ww->ctx);

        if (!hold_ctx)
                return 0;

        if (unlikely(ctx == hold_ctx))
                return -EALREADY;

        if (ctx->stamp - hold_ctx->stamp <= LONG_MAX &&
            (ctx->stamp != hold_ctx->stamp || ctx > hold_ctx)) {
                return -EDEADLK;
        }

        return 0;
}


static __always_inline void ww_mutex_lock_acquired(struct ww_mutex *ww,
                           struct ww_acquire_ctx *ww_ctx)
{
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

static inline int __mutex_fastpath_lock_retval(atomic_t *count)
{
    if (unlikely(atomic_dec_return(count) < 0))
        return -1;
    else
        return 0;
}

static __always_inline void
ww_mutex_set_context_fastpath(struct ww_mutex *lock,
                               struct ww_acquire_ctx *ctx)
{
    u32 flags;
    struct mutex_waiter *cur;

    ww_mutex_lock_acquired(lock, ctx);

    lock->ctx = ctx;

    /*
     * The lock->ctx update should be visible on all cores before
     * the atomic read is done, otherwise contended waiters might be
     * missed. The contended waiters will either see ww_ctx == NULL
     * and keep spinning, or it will acquire wait_lock, add itself
     * to waiter list and sleep.
     */
    smp_mb(); /* ^^^ */

    /*
     * Check if lock is contended, if not there is nobody to wake up
     */
    if (likely(atomic_read(&lock->base.count) == 0))
            return;

    /*
     * Uh oh, we raced in fastpath, wake up everyone in this case,
     * so they can see the new lock->ctx.
     */
    flags = safe_cli();
    list_for_each_entry(cur, &lock->base.wait_list, list) {
        ((struct kos_appdata*)cur->task)->state = KOS_SLOT_STATE_RUNNING;
    }
    safe_sti(flags);
}

static __always_inline void
ww_mutex_set_context_slowpath(struct ww_mutex *lock,
                              struct ww_acquire_ctx *ctx)
{
    struct mutex_waiter *cur;

    ww_mutex_lock_acquired(lock, ctx);
    lock->ctx = ctx;

    /*
     * Give any possible sleeping processes the chance to wake up,
     * so they can recheck if they have to back off.
     */
    list_for_each_entry(cur, &lock->base.wait_list, list) {
        ((struct kos_appdata*)cur->task)->state = KOS_SLOT_STATE_RUNNING;
    }
}

int __ww_mutex_lock_slowpath(struct ww_mutex *ww, struct ww_acquire_ctx *ctx)
{
    struct mutex *lock;
    struct mutex_waiter waiter;
    struct kos_appdata *appdata;
    u32 eflags;
    int ret = 0;

    lock = &ww->base;
    appdata = GetCurrSlot();
    waiter.task = appdata;

    eflags = safe_cli();

    list_add_tail(&waiter.list, &lock->wait_list);

    for(;;)
    {
        if( atomic_xchg(&lock->count, -1) == 1)
            break;

        if (ctx->acquired > 0) {
            ret = __ww_mutex_lock_check_stamp(lock, ctx);
            if (ret)
                goto err;
        };
        appdata->state = KOS_SLOT_STATE_SUSPENDED;
        change_task();
    };

    if (likely(list_empty(&lock->wait_list)))
        atomic_set(&lock->count, 0);

    ww_mutex_set_context_slowpath(ww, ctx);

err:
    list_del(&waiter.list);
    safe_sti(eflags);

    return ret;
}


int __ww_mutex_lock(struct ww_mutex *lock, struct ww_acquire_ctx *ctx)
{
    int ret;

    ret = __mutex_fastpath_lock_retval(&lock->base.count);

    if (likely(!ret)) {
            ww_mutex_set_context_fastpath(lock, ctx);
            mutex_set_owner(&lock->base);
    } else
            ret = __ww_mutex_lock_slowpath(lock, ctx);
    return ret;
}


int __ww_mutex_lock_interruptible(struct ww_mutex *lock, struct ww_acquire_ctx *ctx)
{
    int ret;

    ret = __mutex_fastpath_lock_retval(&lock->base.count);

    if (likely(!ret)) {
            ww_mutex_set_context_fastpath(lock, ctx);
            mutex_set_owner(&lock->base);
    } else
            ret = __ww_mutex_lock_slowpath(lock, ctx);
    return ret;
}
