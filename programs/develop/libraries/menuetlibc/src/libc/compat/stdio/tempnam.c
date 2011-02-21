/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>

char * tempnam(const char *_dir, const char *_prefix)
{
  return tmpnam(0);
}
