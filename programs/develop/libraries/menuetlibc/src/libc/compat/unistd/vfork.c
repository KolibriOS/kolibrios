/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <unistd.h>

pid_t
vfork(void)
{
  errno = ENOMEM; /* The only other choice is EAGAIN, and we don't want that */
  return -1;
}
