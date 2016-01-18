#ifndef __LBSYNC_H__
#define __LBSYNC_H__

#define FUTEX_INIT      0
#define FUTEX_DESTROY   1
#define FUTEX_WAIT      2
#define FUTEX_WAKE      3

#define exchange_acquire(ptr, new) \
  __atomic_exchange_4((ptr), (new), __ATOMIC_ACQUIRE)

#define exchange_release(ptr, new) \
  __atomic_exchange_4((ptr), (new), __ATOMIC_RELEASE)


typedef struct
{
    volatile int lock;
    int handle;
}mutex_t;

static inline int mutex_init(mutex_t *mutex)
{
    int handle;

    mutex->lock = 0;

    asm volatile(
    "int $0x40\t"
    :"=a"(handle)
    :"a"(77),"b"(FUTEX_INIT),"c"(mutex));
    mutex->handle = handle;

    return handle;
};

static inline int mutex_destroy(mutex_t *mutex)
{
    int retval;

    asm volatile(
    "int $0x40\t"
    :"=a"(retval)
    :"a"(77),"b"(FUTEX_DESTROY),"c"(mutex->handle));

    return retval;
};

static inline void mutex_lock(mutex_t *mutex)
{
    int tmp;

    if( __sync_fetch_and_add(&mutex->lock, 1) == 0)
        return;

    while (exchange_acquire (&mutex->lock, 2) != 0)
    {
        asm volatile(
        "int $0x40\t"
        :"=a"(tmp)
        :"a"(77),"b"(FUTEX_WAIT),
        "c"(mutex->handle),"d"(2),"S"(0));
   }
};

static inline void mutex_lock_timeout(mutex_t *mutex, int timeout)
{
    int tmp;

    if( __sync_fetch_and_add(&mutex->lock, 1) == 0)
        return;

    while (exchange_acquire (&mutex->lock, 2) != 0)
    {
        asm volatile(
        "int $0x40\t"
        :"=a"(tmp)
        :"a"(77),"b"(FUTEX_WAIT),
        "c"(mutex->handle),"d"(2),"S"(timeout));
   }
};

static inline int mutex_trylock (mutex_t *mutex)
{
    int zero = 0;

    return __atomic_compare_exchange_4(&mutex->lock, &zero, 1,0,__ATOMIC_ACQUIRE,__ATOMIC_RELAXED);
};

static inline void mutex_unlock(mutex_t *mutex)
{
    int prev;

    prev = exchange_release (&mutex->lock, 0);

    if (prev != 1)
    {
        asm volatile(
        "int $0x40\t"
        :"=a"(prev)
        :"a"(77),"b"(FUTEX_WAKE),
        "c"(mutex->handle),"d"(1));
    };
};

#endif
