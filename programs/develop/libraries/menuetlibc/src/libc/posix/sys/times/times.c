/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <sys/times.h>
#include <time.h>

clock_t
times(struct tms *buffer)
{
  if (buffer == 0)
  {
    errno = EINVAL;
    return (clock_t)(-1);
  }
  buffer->tms_utime = clock();
  buffer->tms_stime = 0;
  buffer->tms_cutime = 0;
  buffer->tms_cstime = 0;
  return buffer->tms_utime;
}
