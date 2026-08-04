#ifndef __LIBC_MUTEX_H
#define __LIBC_MUTEX_H

#include <sys/ksys.h>

#define MUTEX_FREE   1
#define MUTEX_LOCKED 0

typedef struct {
    volatile int lock_state;
    int id;
} __libc_mutex_t;

#define AUTO_INIT_MUTEX(name)                               \
    __libc_mutex_t name;                                    \
    static const __libc_mutex_t* const ptr_##name           \
        __attribute__((section(".mutex_init_array"), used)) \
        = &name

extern const __libc_mutex_t* const __start_mutex_init_array[];
extern const __libc_mutex_t* const __stop_mutex_init_array[];

inline int atomic_cas(volatile int* ptr, int old_val, int new_val)
{
    int actual;
    asm_inline(
        "lock; cmpxchgl %2, %1" : "=a"(actual), "+m"(*ptr) : "r"(new_val), "0"(old_val) : "memory");
    return actual;
}

inline static int __libc_mutex_init(__libc_mutex_t* m)
{

    m->lock_state = MUTEX_FREE;
    m->id = _ksys_futex_create((int*)&m->lock_state);
    return m->id > 0 ? 0 : -1;
}

inline static void __libc_mutex_lock(__libc_mutex_t* m)
{

    while (1) {
        uint32_t prev = atomic_cas(&m->lock_state, MUTEX_FREE, MUTEX_LOCKED);

        if (prev == MUTEX_FREE) {
            return;
        }

        _ksys_futex_wait(m->id, MUTEX_LOCKED, 0);
    }
}

inline static int __libc_mutex_destroy(__libc_mutex_t* m)
{
    return _ksys_futex_destroy(m->id);
}

inline static void __libc_mutex_unlock(__libc_mutex_t* m)
{

    atomic_cas(&m->lock_state, MUTEX_LOCKED, MUTEX_FREE);

    _ksys_futex_wake(m->id, 1);
}

#endif
