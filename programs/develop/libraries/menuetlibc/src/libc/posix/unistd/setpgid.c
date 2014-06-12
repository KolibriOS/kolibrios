/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>

/* ARGSUSED */
int
setpgid(pid_t _pid, pid_t _pgid)
{
  if (_pgid != getpid())
  {
    errno = EPERM;
    return -1;
  }
  return 0;
}
