
#ifndef _SYS_LOCK_H_
#define _SYS_LOCK_H_

#include <sys/cdefs.h>
#include <stddef.h>
#include <sys/gthr.h>

typedef __gthread_mutex_t _LOCK_T;

typedef __gthread_recursive_mutex_t _LOCK_RECURSIVE_T;

#define _MUTEX_INITIALIZER { 0, -1 }

#define _MUTEX_RECURSIVE_INITIALIZER { 0,-1,0,0 }

#define __LOCK_INIT(_qualifier, _designator) \
    _qualifier _LOCK_T _designator = _MUTEX_INITIALIZER

#define __LOCK_INIT_RECURSIVE(_qualifier, _designator) \
    _qualifier _LOCK_RECURSIVE_T _designator = _MUTEX_RECURSIVE_INITIALIZER

static inline int __libc_lock_acquire(_LOCK_T *lock)
{
    if(lock->handle == -1)
        __gthread_mutex_init_function(lock);

    return __gthread_mutex_lock(lock);
}

static inline int __libc_lock_acquire_recursive(_LOCK_RECURSIVE_T *lock)
{
    if(lock->handle == -1)
        __gthread_recursive_mutex_init_function(lock);

    return __gthread_recursive_mutex_lock(lock);
}

#define __lock_acquire(_lock) __libc_lock_acquire(&_lock)
#define __lock_release(_lock) __gthread_mutex_unlock(&_lock)

#define __lock_init_recursive(_lock) __gthread_recursive_mutex_init_function(&_lock)
#define __lock_acquire_recursive(_lock) __libc_lock_acquire_recursive(&_lock)
#define __lock_release_recursive(_lock) __gthread_recursive_mutex_unlock(&_lock)
#define __lock_close_recursive(_lock) __gthread_recursive_mutex_destroy(&_lock)

#endif /* _SYS_LOCK_H_ */
