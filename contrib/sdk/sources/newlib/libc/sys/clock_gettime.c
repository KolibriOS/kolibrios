/* Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <kos32sys.h>

/* Get current value of CLOCK and store it in TP.  */
int clock_gettime (clockid_t clock_id, struct timespec *tp)
{
    uint64_t tsc;

    tsc = get_ns_count();

    tp->tv_sec = tsc / 1000000000;
    tp->tv_nsec = tsc % 1000000000;

    return 0;
}
