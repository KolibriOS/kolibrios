/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <unistd.h>
#include <limits.h>

long
pathconf(const char *path, int name)
{
  switch (name)
  {
  case _PC_LINK_MAX:		return LINK_MAX;
  case _PC_MAX_CANON:		return MAX_CANON;
  case _PC_MAX_INPUT:		return MAX_INPUT;
  case _PC_NAME_MAX:		return NAME_MAX;
  case _PC_PATH_MAX:		return PATH_MAX;
  case _PC_PIPE_BUF:		return PIPE_BUF;
  case _PC_CHOWN_RESTRICTED:	return _POSIX_CHOWN_RESTRICTED;
  case _PC_NO_TRUNC:		return _POSIX_NO_TRUNC;
  case _PC_VDISABLE:		return _POSIX_VDISABLE;

  default:
    errno = EINVAL;
    return -1;
  }
}
