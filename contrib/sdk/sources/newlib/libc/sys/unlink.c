/* unlink.c -- remove a file.
 *
 * Copyright (c) 1995 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#include "glue.h"

static int delete_file(const char *path)
{
     int retval;
     __asm__ __volatile__ (
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "movl %1, 1(%%esp) \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $8 \n\t"
     "movl %%esp, %%ebx \n\t"
     "movl $70, %%eax \n\t"
     "int $0x40 \n\t"
     "addl $28, %%esp \n\t"
     :"=a" (retval)
     :"r" (path)
     :"ebx");
  return retval;
};


int
_DEFUN (unlink, (path),
        char * path)
{
    int err;

    err = delete_file(path);

    if (!err)
        return 0;

    if (err == 5)
        errno = ENOENT;
    else
        errno = EIO;

    return (-1);
}

int
_DEFUN(link, (old, new),
       const char *old _AND
       const char *new)
{
  errno = EIO;
  return (-1);
}

clock_t
times (struct tms *tp)
{
  return -1;
}

