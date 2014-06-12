/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <process.h>
#include <errno.h>
#include <libc/unconst.h>
#include <libc/dosexec.h>

int spawnvpe(int mode, const char *path, char *const argv[], char *const envp[])
{
  char rpath[300];
  union { char * const *cpcp; char **cpp; } u;
  u.cpcp = envp;

  if (!__dosexec_find_on_path(path, u.cpp, rpath))
  {
    errno = ENOENT;
    return -1;
  }
  else
    return spawnve(mode, rpath, argv, envp);
}
