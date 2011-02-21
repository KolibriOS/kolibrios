/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/unconst.h>
#include <process.h>

extern char **environ;

int spawnlp(int mode, const char *path, const char *argv0, ...)
{
  return spawnvpe(mode, path, unconst(&argv0,char * const *),
                              (char * const *)environ);
}
