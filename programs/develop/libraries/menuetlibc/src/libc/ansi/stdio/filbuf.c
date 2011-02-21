/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <libc/file.h>
#include <libc/stdiohk.h>

int _filbuf(FILE *f)
{
  int size;
  char c;

  if (f->_flag & _IORW)
    f->_flag |= _IOREAD;

  if ((f->_flag&_IOREAD) == 0)
    return EOF;
  if (f->_flag&(_IOSTRG|_IOEOF))
    return EOF;
 tryagain:
  if (f->_base==NULL) {
    if (f->_flag&_IONBF) {
      f->_base = &c;
      goto tryagain;
    }
    size = 512;
    if ((f->_base = malloc(size)) == NULL) {
      f->_flag |= _IONBF;
      goto tryagain;
    }
    f->_flag |= _IOMYBUF;
    f->_bufsiz = size;
  }
  if (f == stdin) {
    if (stdout->_flag&_IOLBF)
      fflush(stdout);
    if (stderr->_flag&_IOLBF)
      fflush(stderr);
  }
  if(f->std_ops && STM_OP(f,read))
  {
   f->_cnt=STM_OP(f,read)(f,f->_base,f->_flag & _IONBF ? 1 : f->_bufsiz);
  } else {
   f->_cnt = read(fileno(f), f->_base,f->_flag & _IONBF ? 1 : f->_bufsiz);
  }
  f->_ptr = f->_base;
  if (f->_flag & _IONBF && f->_base == &c)
    f->_base = NULL;
  if (--f->_cnt < 0) {
    if (f->_cnt == -1) {
      f->_flag |= _IOEOF;
      if (f->_flag & _IORW)
	f->_flag &= ~_IOREAD;
    } else
      f->_flag |= _IOERR;
    f->_cnt = 0;
    return EOF;
  }
  return *f->_ptr++ & 0377;
}
