/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <process.h>
#include <libc/dosexec.h>

int execle(const char *path, const char *argv0, ... /*, const char **envp */)
{
  scan_ptr();
  return spawnve(P_OVERLAY, path, (char *const *)&argv0, (char *const *)ptr);
}
