/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>

int __file_exists(const char *fn)
{
 FILE * f;
 f=fopen(fn,"r");
 if(!f) return 0;
 fclose(f);
  return 1;
}
