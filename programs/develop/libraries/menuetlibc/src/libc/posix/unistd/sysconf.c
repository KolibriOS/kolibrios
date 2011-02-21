/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>

long
sysconf(int name)
{
  switch (name)
  {
  case _SC_ARG_MAX:	return ARG_MAX;
  case _SC_CHILD_MAX:	return CHILD_MAX;
  case _SC_CLK_TCK:	return CLOCKS_PER_SEC;
  case _SC_NGROUPS_MAX:	return NGROUPS_MAX;
  case _SC_OPEN_MAX:	return 255;
  case _SC_JOB_CONTROL:	return -1;
  case _SC_SAVED_IDS:	return -1;
  case _SC_STREAM_MAX:	return _POSIX_STREAM_MAX;
  case _SC_TZNAME_MAX:	return TZNAME_MAX;
  case _SC_VERSION:	return _POSIX_VERSION;

  default:
    errno = EINVAL;
    return -1;
  }
}
