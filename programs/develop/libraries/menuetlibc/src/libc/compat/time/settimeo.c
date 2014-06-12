/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <time.h>
#include <assert.h>
#include <errno.h>

int settimeofday(struct timeval *_tp, ...)
{
 return -EPERM;
}
