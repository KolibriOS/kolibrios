/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>

int
fprintf(register FILE *iop, const char *fmt, ...)
{
  int len;
  char localbuf[BUFSIZ];
  va_list va;
  va_start(va, fmt);
  if (iop->_flag & _IONBF)
  {
    iop->_flag &= ~_IONBF;
    iop->_ptr = iop->_base = localbuf;
    iop->_bufsiz = BUFSIZ;
    len = _doprnt(fmt, va, iop);
    fflush(iop);
    iop->_flag |= _IONBF;
    iop->_base = NULL;
    iop->_bufsiz = NULL;
    iop->_cnt = 0;
  }
  else
    len = _doprnt(fmt, va, iop);
  va_end(va);
  return ferror(iop) ? EOF : len;
}
