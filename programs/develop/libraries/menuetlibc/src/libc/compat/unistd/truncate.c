/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <fcntl.h>
#include <io.h>
#include <assert.h>

int truncate(const char *fn, off_t where)
{
  int fd = open(fn, O_WRONLY);
  if (fd < 0)
    return -1;
  if (lseek(fd, where, 0) < 0)
  {
    close(fd);
    return -1;
  }
  if (_write(fd, 0, 0) < 0)
  {
    close(fd);
    return -1;
  }
  return close(fd);
}
