/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>

int
setgid(gid_t _gid)
{
  if (_gid != getgid())
  {
    errno = EPERM;
    return -1;
  }
  return 0;
}
