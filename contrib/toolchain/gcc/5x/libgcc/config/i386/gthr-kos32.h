/* Threads compatibility routines for libgcc2 and libobjc.  */
/* Compile this one with gcc.  */

/* Copyright (C) 1999-2015 Free Software Foundation, Inc.
   Contributed by Mumit Khan <khan@xraylith.wisc.edu>.

This file is part of GCC.

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

#ifndef GCC_GTHR_KOS32_H
#define GCC_GTHR_KOS32_H

/* Make sure CONST_CAST2 (origin in system.h) is declared.  */
#ifndef CONST_CAST2
#define CONST_CAST2(TOTYPE,FROMTYPE,X) ((__extension__(union {FROMTYPE _q; TOTYPE _nq;})(X))._nq)
#endif


#define __GTHREADS 1

#include <stddef.h>

#ifndef __UNUSED_PARAM
#define __UNUSED_PARAM(x) x
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long __gthread_key_t;

typedef struct
{
    int done;
    long started;
} __gthread_once_t;

typedef struct
{
    volatile int lock;
    int handle;
} __gthread_mutex_t;

typedef struct
{
    volatile int lock;
    int handle;
    long depth;
    unsigned long owner;
} __gthread_recursive_mutex_t;

#define __GTHREAD_ONCE_INIT {0, -1}
#define __GTHREAD_MUTEX_INIT_FUNCTION __gthread_mutex_init_function

#define __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION \
  __gthread_recursive_mutex_init_function


#if defined (_WIN32) && !defined(__CYGWIN__)
#define MINGW32_SUPPORTS_MT_EH 1
/* Mingw runtime >= v0.3 provides a magic variable that is set to nonzero
   if -mthreads option was specified, or 0 otherwise. This is to get around
   the lack of weak symbols in PE-COFF.  */
extern int _CRT_MT;
extern int __mingwthr_key_dtor (unsigned long, void (*) (void *));
#endif /* _WIN32 && !__CYGWIN__ */


static inline int __gthread_active_p (void)
{
#ifdef MINGW32_SUPPORTS_MT_EH
  return _CRT_MT;
#else
  return 1;
#endif
}

extern int __gthr_kos32_once (__gthread_once_t *, void (*) (void));
extern int __gthr_kos32_key_create (__gthread_key_t *, void (*) (void*));
extern int __gthr_kos32_key_delete (__gthread_key_t);
extern void * __gthr_kos32_getspecific (__gthread_key_t);
extern int __gthr_kos32_setspecific (__gthread_key_t, const void *);
extern void __gthr_kos32_mutex_init_function (__gthread_mutex_t *);
extern int __gthr_kos32_mutex_lock (__gthread_mutex_t *);
extern int __gthr_kos32_mutex_trylock (__gthread_mutex_t *);
extern int __gthr_kos32_mutex_unlock (__gthread_mutex_t *);
extern void __gthr_kos32_recursive_mutex_init_function (__gthread_recursive_mutex_t *);
extern int  __gthr_kos32_recursive_mutex_lock (__gthread_recursive_mutex_t *);
extern int  __gthr_kos32_recursive_mutex_trylock (__gthread_recursive_mutex_t *);
extern int  __gthr_kos32_recursive_mutex_unlock (__gthread_recursive_mutex_t *);
extern void __gthr_kos32_mutex_destroy (__gthread_mutex_t *);
extern int  __gthr_kos32_recursive_mutex_destroy (__gthread_recursive_mutex_t *);

static inline int __gthread_once (__gthread_once_t *__once, void (*__func) (void))
{
    if (__gthread_active_p ())
        return __gthr_kos32_once (__once, __func);
    else
        return -1;
}

static inline int __gthread_key_create (__gthread_key_t *__key, void (*__dtor) (void *))
{
    return __gthr_kos32_key_create (__key, __dtor);
}

static inline int __gthread_key_delete (__gthread_key_t __key)
{
    return __gthr_kos32_key_delete (__key);
}

static inline void* __gthread_getspecific (__gthread_key_t __key)
{
    return __gthr_kos32_getspecific (__key);
}

static inline int __gthread_setspecific (__gthread_key_t __key, const void *__ptr)
{
    return __gthr_kos32_setspecific (__key, __ptr);
}

static inline void __gthread_mutex_init_function (__gthread_mutex_t *__mutex)
{
    __gthr_kos32_mutex_init_function(__mutex);
}

static inline void __gthread_mutex_destroy (__gthread_mutex_t *__mutex)
{
    __gthr_kos32_mutex_destroy (__mutex);
}

static inline int __gthread_mutex_lock (__gthread_mutex_t *__mutex)
{
    if (__gthread_active_p ())
        return __gthr_kos32_mutex_lock (__mutex);
    else
        return 0;
}

static inline int __gthread_mutex_trylock (__gthread_mutex_t *__mutex)
{
    if (__gthread_active_p ())
        return __gthr_kos32_mutex_trylock (__mutex);
    else
        return 0;
}

static inline int __gthread_mutex_unlock (__gthread_mutex_t *__mutex)
{
    if (__gthread_active_p ())
        return __gthr_kos32_mutex_unlock (__mutex);
    else
        return 0;
}

static inline void __gthread_recursive_mutex_init_function (__gthread_recursive_mutex_t *__mutex)
{
    __gthr_kos32_recursive_mutex_init_function (__mutex);
}

static inline int __gthread_recursive_mutex_lock (__gthread_recursive_mutex_t *__mutex)
{
    if (__gthread_active_p ())
        return __gthr_kos32_recursive_mutex_lock (__mutex);
    else
        return 0;
}

static inline int __gthread_recursive_mutex_trylock (__gthread_recursive_mutex_t *__mutex)
{
    if (__gthread_active_p ())
        return __gthr_kos32_recursive_mutex_trylock (__mutex);
    else
        return 0;
}

static inline int __gthread_recursive_mutex_unlock (__gthread_recursive_mutex_t *__mutex)
{
  if (__gthread_active_p ())
        return __gthr_kos32_recursive_mutex_unlock (__mutex);
  else
        return 0;
}

static inline int __gthread_recursive_mutex_destroy (__gthread_recursive_mutex_t *__mutex)
{
    return __gthr_kos32_recursive_mutex_destroy (__mutex);
}


#ifdef __cplusplus
}
#endif

#endif /* ! GCC_GTHR_WIN32_H */
