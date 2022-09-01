/* Implementation of Kos32-specific threads compatibility routines for
   libgcc2.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdint.h>
#include <sys/ksys.h>
#include "gthr-kos32.h"

#define FUTEX_INIT      0
#define FUTEX_DESTROY   1
#define FUTEX_WAIT      2
#define FUTEX_WAKE      3

#define exchange_acquire(ptr, new) \
  __atomic_exchange_4((ptr), (new), __ATOMIC_ACQUIRE)

#define exchange_release(ptr, new) \
  __atomic_exchange_4((ptr), (new), __ATOMIC_RELEASE)

#define TLS_KEY_PID         0
#define TLS_KEY_TID         4
#define TLS_KEY_LOW_STACK   8
#define TLS_KEY_HIGH_STACK 12
#define TLS_KEY_LIBC       16

unsigned int tls_alloc(void);
int tls_free(unsigned int key);

static inline int tls_set(unsigned int key, void *val)
{
    int ret = -1;
    if(key < 4096)
    {
        __asm__ __volatile__(
        "movl %0, %%fs:(%1)"
        ::"r"(val),"r"(key));
        ret = 0;
    }
    return ret;
};

static inline void *tls_get(unsigned int key)
{
    void *val = (void*)-1;
    if(key < 4096)
    {
        __asm__ __volatile__(
        "movl %%fs:(%1), %0"
        :"=r"(val)
        :"r"(key));
    };
    return val;
}


int __gthr_kos32_once (__gthread_once_t *once, void (*func) (void))
{
    if (once == NULL || func == NULL)
        return EINVAL;

    if (! once->done)
    {
        if (__sync_add_and_fetch(&(once->started), 1) == 0)
        {
            (*func) ();
            once->done = 1;
        }
        else
        {
            while (! once->done)
                _ksys_thread_yield();
        }
    }
    return 0;
};

int __gthr_kos32_key_create (__gthread_key_t *key,
			 void (*dtor) (void *) __attribute__((unused)))
{
    int status = 0;
    unsigned int tls_index = tls_alloc();
    if (tls_index != 0xFFFFFFFF)
    {
        *key = tls_index;
#ifdef MINGW32_SUPPORTS_MT_EH
      /* Mingw runtime will run the dtors in reverse order for each thread
         when the thread exits.  */
        status = __mingwthr_key_dtor (*key, dtor);
#endif
    }
    else
        status = -1;
    return status;
}

int __gthr_kos32_key_delete (__gthread_key_t key)
{
    return tls_free(key);
}

void* __gthr_kos32_getspecific (__gthread_key_t key)
{
    return tls_get(key);
}

int __gthr_kos32_setspecific (__gthread_key_t key, const void *ptr)
{
    return tls_set(key, CONST_CAST2(void *, const void *, ptr));
}

void __gthr_kos32_mutex_init_function (__gthread_mutex_t *mutex)
{
    mutex->lock = 0;
    mutex->handle  = _ksys_futex_create(mutex);
}

void __gthr_kos32_mutex_destroy (__gthread_mutex_t *mutex)
{
    int retval;
    _ksys_futex_destroy(mutex->handle);
}

int __gthr_kos32_mutex_lock (__gthread_mutex_t *mutex)
{
    int tmp;

    if( __sync_fetch_and_add(&mutex->lock, 1) == 0)
        return 0;

    while (exchange_acquire (&mutex->lock, 2) != 0)
    {
        _ksys_futex_wait(mutex->handle, 2 , 0);
    }
    return 0;
}

int __gthr_kos32_mutex_trylock (__gthread_mutex_t *mutex)
{
    int zero = 0;

    return !__atomic_compare_exchange_4(&mutex->lock, &zero, 1,0,__ATOMIC_ACQUIRE,__ATOMIC_RELAXED);
}

int __gthr_kos32_mutex_unlock (__gthread_mutex_t *mutex)
{
    int prev;

    prev = exchange_release (&mutex->lock, 0);

    if (prev != 1)
    {
        _ksys_futex_wake(mutex->handle, 1);
    };
    return 0;
}

void __gthr_kos32_recursive_mutex_init_function (__gthread_recursive_mutex_t *mutex)
{
    int handle;
    mutex->lock = 0;

    mutex->handle = _ksys_futex_create(mutex);

    mutex->depth = 0;
    mutex->owner = 0;
}

int __gthr_kos32_recursive_mutex_lock (__gthread_recursive_mutex_t *mutex)
{
    int tmp;

    unsigned long me = (unsigned long)tls_get(TLS_KEY_LOW_STACK);

    if( __sync_fetch_and_add(&mutex->lock, 1) == 0)
    {
        mutex->depth = 1;
        mutex->owner = me;
        return 0;
    }
    else if (mutex->owner == me)
    {
        __sync_fetch_and_sub(&mutex->lock, 1);
        ++(mutex->depth);
    }
    else while (exchange_acquire (&mutex->lock, 2) != 0)
    {
        _ksys_futex_wait(mutex->handle, 2, 0);
        mutex->depth = 1;
        mutex->owner = me;
    };

    return 0;
}

int __gthr_kos32_recursive_mutex_trylock (__gthread_recursive_mutex_t *mutex)
{
    unsigned long me = (unsigned long)tls_get(TLS_KEY_LOW_STACK);
    int zero = 0;

    if(__atomic_compare_exchange_4(&mutex->lock, &zero, 1,0,__ATOMIC_ACQUIRE,__ATOMIC_RELAXED))
    {
        mutex->depth = 1;
        mutex->owner = me;
    }
    else if (mutex->owner == me)
        ++(mutex->depth);
    else
        return 1;

    return 0;
}

int __gthr_kos32_recursive_mutex_unlock (__gthread_recursive_mutex_t *mutex)
{
    --(mutex->depth);

    if (mutex->depth == 0)
    {
        int prev;

        prev = exchange_release (&mutex->lock, 0);

        if (prev != 1)
        {
            _ksys_futex_wake(mutex->handle, 1);
        };
        mutex->owner = 0;
    };

    return 0;
}

int __gthr_kos32_recursive_mutex_destroy (__gthread_recursive_mutex_t *mutex)
{
    int retval;
    _ksys_futex_destroy(mutex->handle);
    return 0;
}

