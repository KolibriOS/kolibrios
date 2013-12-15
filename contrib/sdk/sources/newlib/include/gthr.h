/* Threads compatibility routines for libgcc2.  */
/* Compile this one with gcc.  */
/* Copyright (C) 1997, 1998, 2004, 2008, 2009 Free Software Foundation, Inc.

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

#ifndef GCC_GTHR_H
#define GCC_GTHR_H

typedef unsigned int __gthread_key_t;

typedef struct {
  volatile int done;
  int started;
} __gthread_once_t;

typedef struct {
  volatile int counter;
} __gthread_mutex_t;


void *tls_alloc(void);

static int __gthread_mutex_lock (__gthread_mutex_t *mutex)
{
    __mutex_lock(&mutex->counter);
    return 0;
};

static int __gthread_mutex_unlock (__gthread_mutex_t *mutex)
{
    mutex->counter = 0;
    return 0;
};

static inline int __gthread_key_create (__gthread_key_t *__key,
              void (*__dtor) (void *) __attribute__((unused)))
{
    int __status = 0;
    void *__tls_index = tls_alloc();
    if (__tls_index != NULL)
    {
        *__key = (unsigned int)__tls_index;

#ifdef MINGW32_SUPPORTS_MT_EH               /* FIXME */
      /* Mingw runtime will run the dtors in reverse order for each thread
         when the thread exits.  */
//      __status = __mingwthr_key_dtor (*__key, __dtor);
#endif

    }
    else
        __status = (int) ENOMEM;
    return __status;
}


static inline void *
__gthread_getspecific (__gthread_key_t __key)
{
    void *val;
    __asm__ __volatile__(
    "movl %%fs:(%1), %0"
    :"=r"(val)
    :"r"(__key));

  return val;
};

static inline int
__gthread_setspecific (__gthread_key_t __key, const void *__ptr)
{
    if(!(__key & 3))
    {
        __asm__ __volatile__(
        "movl %0, %%fs:(%1)"
        ::"r"(__ptr),"r"(__key));
        return 0;
    }
    else return EINVAL;
}


#endif
