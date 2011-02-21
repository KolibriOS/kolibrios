/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/resource.h>

int getrusage(int _who, struct rusage *_rusage)
{
 clock_t q;
 if (_rusage == 0)
 {
  errno = EFAULT;
  return -1;
 }
 if (_who != RUSAGE_SELF && _who != RUSAGE_CHILDREN)
 {
  errno = EINVAL;
  return -1;
 }
 memset(_rusage, 0, sizeof(struct rusage));
 q = clock();
 _rusage->ru_utime.tv_sec = q / CLOCKS_PER_SEC;
 _rusage->ru_utime.tv_usec = q % CLOCKS_PER_SEC * 1000000 / CLOCKS_PER_SEC;
 return 0;
}
