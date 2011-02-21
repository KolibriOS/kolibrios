/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <errno.h>
#include <io.h>
#include <libc/dosio.h>

int
dup2(int fd, int newfd)
{
  return -1;
}
