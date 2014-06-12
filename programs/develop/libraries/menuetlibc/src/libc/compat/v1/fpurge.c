/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>

int
fpurge(FILE *f)
{
  char *base;

  if ((f->_flag&(_IONBF|_IOWRT))==_IOWRT
      && (base = f->_base) != NULL
      && (f->_ptr - base) > 0)
  {
    f->_ptr = base;
    f->_cnt = (f->_flag&(_IOLBF|_IONBF)) ? 0 : f->_bufsiz;
  }
  return 0;
}
