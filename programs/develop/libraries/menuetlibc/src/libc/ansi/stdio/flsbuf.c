/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <libc/file.h>

int
_flsbuf(int c, FILE *f)
{
  char *base;
  int n, rn;
  char c1;
  int size;

  if (f->_flag & _IORW)
  {
    f->_flag |= _IOWRT;
    f->_flag &= ~(_IOEOF|_IOREAD);
  }

  if ((f->_flag&_IOWRT)==0)
    return EOF;

 tryagain:
  if (f->_flag&_IOLBF)
  {
    base = f->_base;
    *f->_ptr++ = c;
    if ((rn = f->_ptr - base) >= f->_bufsiz || c == '\n')
    {
      f->_ptr = base;
      f->_cnt = 0;
    }
    else
    {
      /* we got here because _cnt is wrong, so fix it */
      f->_cnt = -rn;
      rn = n = 0;
    }
  }
  else
    if (f->_flag&_IONBF)
    {
      c1 = c;
      rn = 1;
      base = &c1;
      f->_cnt = 0;
    }
    else
    {
      if ((base=f->_base)==NULL)
      {
	size = 512;
	if ((f->_base=base=malloc(size)) == NULL)
	{
	  f->_flag |= _IONBF;
	  goto tryagain;
	}
	f->_flag |= _IOMYBUF;
	f->_bufsiz = size;
	if (f==stdout)
	{
	  f->_flag |= _IOLBF;
	  f->_ptr = base;
	  goto tryagain;
	}
	rn = n = 0;
      }
      else
	rn = f->_ptr - base;
      f->_ptr = base;
      f->_cnt = f->_bufsiz;
    }
  while (rn > 0)
  {
   if(f->std_ops && STM_OP(f,write))
   {
    n=STM_OP(f,write)(f,base,rn);
   } else {
    n = write(fileno(f), base, rn);
   }
   if (n <= 0)
   {
    f->_flag |= _IOERR;
    return EOF;
   }
   rn -= n;
  base += n;
 }
  if ((f->_flag&(_IOLBF|_IONBF)) == 0)
  {
    f->_cnt--;
    *f->_ptr++ = c;
  }
  return c;
}
