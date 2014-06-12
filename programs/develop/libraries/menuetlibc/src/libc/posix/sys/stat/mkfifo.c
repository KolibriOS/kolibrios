/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <sys/stat.h>
#include <errno.h>

int
mkfifo(const char *path, mode_t mode)
{
  errno = EACCES;
  return -1;
}
