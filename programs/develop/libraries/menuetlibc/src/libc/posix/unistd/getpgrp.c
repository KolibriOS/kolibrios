/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>

pid_t
getpgrp(void)
{
  return getpid();
}
