#include "kolibc.h"

extern int fill_buff(FILE* f);

int fgetc(FILE* f)
{ char retval;

  if(!((f->mode & FILE_OPEN_READ)|(f->mode & FILE_OPEN_PLUS)))
    return EOF;

  if(f->remain ==0)
  { f->filepos+=4096;
    if(!fill_buff(f))
      return EOF;
  };
      
  retval= *(f->stream);          
  f->strpos++;
  f->stream++;
  f->remain--;  
  return (int)retval; 
}