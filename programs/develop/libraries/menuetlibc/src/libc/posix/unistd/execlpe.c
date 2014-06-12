/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <process.h>
#include <libc/dosexec.h>

int execlpe(const char *path, const char *argv0, ... /*, const char **envp */)
{
  scan_ptr();
  return spawnvpe(P_OVERLAY, path, (char * const *)&argv0, (char * const *)ptr);
}
