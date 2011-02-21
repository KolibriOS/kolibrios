/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <process.h>

extern char **environ;

int spawnv(int mode, const char *path, char *const argv[])
{
  return spawnve(mode, path, (char * const *)argv, environ);
}
