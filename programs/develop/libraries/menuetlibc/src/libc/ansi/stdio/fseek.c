/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <unistd.h>
#include <libc/file.h>
#include <fcntl.h>
#include <libc/dosio.h>

int
fseek(FILE *f, long offset, int ptrname)
{
  long p = -1;			/* can't happen? */
  if(f && f->std_ops && STM_OP(f,seek))
   return STM_OP(f,seek)(f,offset,ptrname);
  f->_flag &= ~_IOEOF;
  if (f->_flag & _IOREAD)
  {
    if ((ptrname == SEEK_CUR) && f->_base && !(f->_flag & _IONBF))
    {
      offset += ftell(f);
      ptrname = SEEK_SET;
    }

    if (f->_flag & _IORW)
    {
      f->_ptr = f->_base;
      f->_flag &= ~_IOREAD;
    }
    p = lseek(fileno(f), offset, ptrname);
    f->_cnt = 0;
    f->_ptr = f->_base;
  }
  else if (f->_flag & (_IOWRT|_IORW))
  {
    p = fflush(f);
    return lseek(fileno(f), offset, ptrname) == -1 || p == EOF ?
      -1 : 0;
  }
  return p==-1 ? -1 : 0;
}
