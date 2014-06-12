/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <time.h>

time_t time(time_t *t)
{
  struct timeval tt;

  if (gettimeofday(&tt, 0) < 0)
    return(-1);
  if (t)
    *t = tt.tv_sec;
  return tt.tv_sec;
}
