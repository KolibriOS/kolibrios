/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <libc/file.h>

FILE *
tmpfile(void)
{
  FILE *f;
  char *temp_name;
  char *n_t_r;

  temp_name = tmpnam(0);
  if (temp_name == 0)
    return 0;

  n_t_r = (char *)malloc(strlen(temp_name)+1);
  if (!n_t_r)
    return 0;

  f = fopen(temp_name, (_fmode & O_TEXT) ? "wt+" : "wb+");
  if (f)
  {
    f->_flag |= _IORMONCL;
    f->_name_to_remove = n_t_r;
    strcpy(f->_name_to_remove, temp_name);
  }
  return f;
}
