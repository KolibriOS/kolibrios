/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>

int
pipe(int _fildes[2])
{
  errno = EACCES;
  return -1;
}
