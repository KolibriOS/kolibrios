/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>

pid_t
setsid(void)
{
  return getpid();
}
