/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <stddef.h>
#include <sys/resource.h>

extern struct rlimit __libc_limits[];

int
setrlimit (int rltype, const struct rlimit *rlimitp)
{
  /* check argument range */
  if (rlimitp->rlim_cur > rlimitp->rlim_max || rlimitp == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  switch (rltype)
    {
    case RLIMIT_CPU:
    case RLIMIT_FSIZE:
    case RLIMIT_DATA:
    case RLIMIT_STACK:
    case RLIMIT_CORE:
    case RLIMIT_RSS:
    case RLIMIT_MEMLOCK:
    case RLIMIT_NPROC:
    case RLIMIT_NOFILE:
      /* not supported */
      errno = EPERM;
      return -1;
    default:
      errno = EINVAL;
      return -1;
    }

  return 0;
}
