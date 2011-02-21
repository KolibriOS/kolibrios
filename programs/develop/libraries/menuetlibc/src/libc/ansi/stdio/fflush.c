/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <libc/file.h>

int fflush(FILE *f)
{
  char *base;
  int n, rn;
  if(!f) return 0;
  if(f->std_ops && STM_OP(f,flush))
   return STM_OP(f,flush)(f);
  if ((f->_flag&(_IONBF|_IOWRT))==_IOWRT
      && (base = f->_base) != NULL
      && (rn = n = f->_ptr - base) > 0)
  {
    f->_ptr = base;
    f->_cnt = (f->_flag&(_IOLBF|_IONBF)) ? 0 : f->_bufsiz;
    do {
      n = write(fileno(f), base, rn);
      if (n <= 0) {
	f->_flag |= _IOERR;
	return EOF;
      }
      rn -= n;
      base += n;
    } while (rn > 0);
    _dosemu_flush(fileno(f));
  }
  if (f->_flag & _IORW)
  {
    f->_cnt = 0;
    f->_flag &= ~(_IOWRT|_IOREAD);
    f->_ptr = f->_base;
  }
  return 0;
}
