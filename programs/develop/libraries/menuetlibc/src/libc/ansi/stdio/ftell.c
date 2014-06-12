/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <unistd.h>
#include <libc/file.h>
#include <fcntl.h>
#include <libc/dosio.h>

long
ftell(FILE *f)
{
  long tres;
  int adjust=0;
  int idx;

  if (f->_cnt < 0)
    f->_cnt = 0;
  if (f->_flag&_IOREAD)
  {
    /* When reading files, the file position known by `lseek' is
     at the end of the buffered portion of the file.  So `adjust'
     is negative (current buf position is BEFORE the one returned
     by `lseek') and, for TEXT files, it gets decremented (larger
     in absolute value) for every NL from current pos to the end
     of the buffer, to account for stripped CR characters.  */
    adjust = - f->_cnt;

    if (__file_handle_modes[f->_file] & O_TEXT) /* if a text file */
    {
      if (f->_cnt)
      {
	char *cp;

        /* For every char in buf AFTER current pos... */
	for (cp=f->_ptr + f->_cnt - 1; cp >= f->_ptr; cp--)
	  if (*cp == '\n')	/* ...if it's LF... */
	    adjust--;		/* ...there was a CR also */
      }
    }
  }
  else if (f->_flag&(_IOWRT|_IORW))
  {
    /* When writing a file, the current file position known by `lseek'
       is at the beginning of the buffered portion of the file.  We
       have to adjust it by our offset from the beginning of the buffer,
       and account for the CR characters which will be added by `write'.  */
    if (f->_flag&_IOWRT && f->_base && (f->_flag&_IONBF)==0)
    {
      int lastidx = adjust = f->_ptr - f->_base;

      if (__file_handle_modes[f->_file] & O_TEXT)
	for (idx=0; idx < lastidx; idx++)
	  if (f->_base[idx] == '\n')
	    adjust++;
    }
  }
  else
    return -1;
  if(f && f->std_ops && STM_OP(f,seek))
   tres=STM_OP(f,seek)(f,0,1);
  else
   tres = lseek(fileno(f), 0L, 1);
  if (tres<0)
    return tres;
  tres += adjust;
  return tres;
}
