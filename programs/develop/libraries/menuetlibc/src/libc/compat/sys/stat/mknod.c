/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

/* ARGSUSED */
int
mknod(const char *path, mode_t mode, dev_t dev)
{
  errno = EACCES;
  return -1;
}
