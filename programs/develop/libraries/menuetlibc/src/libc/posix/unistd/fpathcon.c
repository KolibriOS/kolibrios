/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <unistd.h>
#include <limits.h>

/* ARGSUSED */
long fpathconf(int fildes, int name)
{
  return pathconf("/", name);
}
