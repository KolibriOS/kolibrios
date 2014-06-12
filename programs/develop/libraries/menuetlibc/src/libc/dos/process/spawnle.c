/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/unconst.h>
#include <process.h>
#include <libc/dosexec.h>

int spawnle(int mode, const char *path, const char *argv0, ... /*, const char **envp */)
{
  scan_ptr();
  return spawnve(mode, path, unconst(&argv0,char * const *), 
                             unconst(ptr,char * const *));
}
