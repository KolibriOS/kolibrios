/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <process.h>
#include <libc/dosexec.h>

extern char * const *environ;

int execlp(const char *path, const char *argv0, ...)
{
  return spawnvpe(P_OVERLAY, path, (char * const *)&argv0, environ);
}
