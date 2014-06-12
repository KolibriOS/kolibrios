/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>

int
setuid(uid_t uid)
{
  if (uid == getuid())
    return 0;
  errno = EPERM;
  return -1;
}
