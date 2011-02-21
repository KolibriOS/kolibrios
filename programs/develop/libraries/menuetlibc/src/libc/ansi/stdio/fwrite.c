/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libc/file.h>

size_t
fwrite(const void *vptr, size_t size, size_t count, FILE *f)
{
  const char *ptr = (const char *)vptr;
  register int s;

  s = size * count;

//  __libclog_printf("fwrite(%x,%u,%u,%u)\n",vptr,size,count,f->_file);

  if (f->_flag & _IOLBF)
    while (s > 0) {
      if (--f->_cnt > -f->_bufsiz && *(const char *)ptr != '\n')
	*f->_ptr++ = *(const char *)ptr++;
      else if (_flsbuf(*(const char *)ptr++, f) == EOF)
	break;
      s--;
    }
  else while (s > 0) {
    if (f->_cnt < s) {
      if (f->_cnt > 0) {
	memcpy(f->_ptr, ptr, f->_cnt);
	ptr += f->_cnt;
	f->_ptr += f->_cnt;
	s -= f->_cnt;
      }
      if (_flsbuf(*(const unsigned char *)ptr++, f) == EOF)
	break;
      s--;
    }
    if (f->_cnt >= s) {
      memcpy(f->_ptr, ptr, s);
      f->_ptr += s;
      f->_cnt -= s;
      return count;
    }
  }
  return size != 0 ? count - ((s + size - 1) / size) : 0;
}
