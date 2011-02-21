/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <crt0.h>
#include <glob.h>

char ** __crt0_glob_function(char *arg)
{
  char **rv;
  glob_t g;
  if (glob(arg, GLOB_NOCHECK, 0, &g) !=  0)
    return 0;
  rv = g.gl_pathv;
  return rv;
}
