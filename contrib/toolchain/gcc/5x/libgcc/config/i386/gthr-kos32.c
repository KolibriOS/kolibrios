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

#include "gthr-kos32.h"

#define FUTEX_INIT      0
#define FUTEX_DESTROY   1
#define FUTEX_WAIT      2
#define FUTEX_WAKE      3

unsigned int tls_alloc(void);
int tls_free(unsigned int key);
void *tls_get(unsigned int key);
void *tls_set(unsigned int key, void *val);

#define exchange_acquire(ptr, new) \
  __atomic_exchange_4((ptr), (new), __ATOMIC_ACQUIRE)

#define exchange_release(ptr, new) \
  __atomic_exchange_4((ptr), (new), __ATOMIC_RELEASE)


static inline void yield(void)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(68), "b"(1));
};



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
                yield();
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
    void *ptr;
    ptr = tls_get(key);
    return ptr;
}

int __gthr_kos32_setspecific (__gthread_key_t key, const void *ptr)
{
    tls_set(key, CONST_CAST2(void *, const void *, ptr));
    return 0;
}

void __gthr_kos32_mutex_init_function (__gthread_mutex_t *mutex)
{
    int handle;

    mutex->lock = 0;

    __asm__ volatile(
    "int $0x40\t"
    :"=a"(handle)
    :"a"(77),"b"(FUTEX_INIT),"c"(mutex));
    mutex->handle = handle;
}

void __gthr_kos32_mutex_destroy (__gthread_mutex_t *mutex)
{
    int retval;

    __asm__ volatile(
    "int $0x40\t"
    :"=a"(retval)
    :"a"(77),"b"(FUTEX_DESTROY),"c"(mutex->handle));
}

int __gthr_kos32_mutex_lock (__gthread_mutex_t *mutex)
{
    int tmp;

    if( __sync_fetch_and_add(&mutex->lock, 1) == 0)
        return 0;

    while (exchange_acquire (&mutex->lock, 2) != 0)
    {
        __asm__ volatile(
        "int $0x40\t\n"
        :"=a"(tmp)
        :"a"(77),"b"(FUTEX_WAIT),
        "c"(mutex->handle),"d"(2),"S"(0));
   }
   return 0;
}

int __gthr_kos32_mutex_trylock (__gthread_mutex_t *mutex)
{
    int zero = 0;

    return __atomic_compare_exchange_4(&mutex->lock, &zero, 1,0,__ATOMIC_ACQUIRE,__ATOMIC_RELAXED);
}

int __gthr_kos32_mutex_unlock (__gthread_mutex_t *mutex)
{
    int prev;

    prev = exchange_release (&mutex->lock, 0);

    if (prev != 1)
    {
        __asm__ volatile(
        "int $0x40\t"
        :"=a"(prev)
        :"a"(77),"b"(FUTEX_WAKE),
        "c"(mutex->handle),"d"(1));
    };
    return 0;
}

void __gthr_kos32_recursive_mutex_init_function (__gthread_recursive_mutex_t *mutex)
{
//  mutex->counter = -1;
  mutex->depth = 0;
  mutex->owner = 0;
//  mutex->sema = CreateSemaphoreW (NULL, 0, 65535, NULL);
}

#if 0
int
__gthr_win32_recursive_mutex_lock (__gthread_recursive_mutex_t *mutex)
{
  DWORD me = GetCurrentThreadId();
  if (InterlockedIncrement (&mutex->counter) == 0)
    {
      mutex->depth = 1;
      mutex->owner = me;
    }
  else if (mutex->owner == me)
    {
      InterlockedDecrement (&mutex->counter);
      ++(mutex->depth);
    }
  else if (WaitForSingleObject (mutex->sema, INFINITE) == WAIT_OBJECT_0)
    {
      mutex->depth = 1;
      mutex->owner = me;
    }
  else
    {
      /* WaitForSingleObject returns WAIT_FAILED, and we can only do
         some best-effort cleanup here.  */
      InterlockedDecrement (&mutex->counter);
      return 1;
    }
  return 0;
}

int
__gthr_win32_recursive_mutex_trylock (__gthread_recursive_mutex_t *mutex)
{
  DWORD me = GetCurrentThreadId();
  if (__GTHR_W32_InterlockedCompareExchange (&mutex->counter, 0, -1) < 0)
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

int
__gthr_win32_recursive_mutex_unlock (__gthread_recursive_mutex_t *mutex)
{
  --(mutex->depth);
  if (mutex->depth == 0)
    {
      mutex->owner = 0;

      if (InterlockedDecrement (&mutex->counter) >= 0)
	return ReleaseSemaphore (mutex->sema, 1, NULL) ? 0 : 1;
    }

  return 0;
}

int
__gthr_win32_recursive_mutex_destroy (__gthread_recursive_mutex_t *mutex)
{
  CloseHandle ((HANDLE) mutex->sema);
  return 0;
}

#endif
