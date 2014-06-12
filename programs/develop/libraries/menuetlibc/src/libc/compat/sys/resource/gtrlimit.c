/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/resource.h>
#include <libc/bss.h>

extern unsigned int _stklen;
extern struct rlimit __libc_limits[];

static int rlimit_count = -1;
struct rlimit __libc_limits[RLIM_NLIMITS];

static void
__limits_initialize (void)
{
  int i;

  /* set hard limit */
  __libc_limits[RLIMIT_CPU].rlim_max = RLIM_INFINITY;
  __libc_limits[RLIMIT_FSIZE].rlim_max = RLIM_INFINITY;
  __libc_limits[RLIMIT_DATA].rlim_max = RLIM_INFINITY;
  __libc_limits[RLIMIT_STACK].rlim_max = (long) _stklen;
  __libc_limits[RLIMIT_CORE].rlim_max = RLIM_INFINITY;
  __libc_limits[RLIMIT_RSS].rlim_max = RLIM_INFINITY;
  __libc_limits[RLIMIT_MEMLOCK].rlim_max = RLIM_INFINITY;
  __libc_limits[RLIMIT_NPROC].rlim_max = RLIM_INFINITY;
  __libc_limits[RLIMIT_NOFILE].rlim_max = sysconf (_SC_OPEN_MAX);

  /* copy all hard limit to soft limit */
  for (i = 0; i < RLIM_NLIMITS; i++)
    __libc_limits[i].rlim_cur = __libc_limits[i].rlim_max;
}

int
getrlimit (int rltype, struct rlimit *rlimitp)
{
  /* check argument range */
  if (rltype < 0 || rltype >= RLIM_NLIMITS || rlimitp == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  /* initialize */
  if (rlimit_count != __bss_count)
    {
      rlimit_count = __bss_count;
      __limits_initialize ();
    }

  /* copy limit value */
  *rlimitp = __libc_limits[rltype];

  return 0;
}
