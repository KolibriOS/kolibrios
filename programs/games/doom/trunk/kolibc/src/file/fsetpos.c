
#include "kolibri.h"
#include "kolibc.h"

int fsetpos(FILE* f,const fpos_t * pos)
{ int err;
  size_t bytes;
  
  bytes = *pos;
  
  bytes= *pos & -8192;
  err=read_file(f->filename,f->buffer,bytes,
                             8192,&bytes);
  if(bytes == -1)
    return EOF;                           
    
  f->filepos= *pos & -8192;
  f->strpos = *pos & 8191;
  f->remain = 8192-f->strpos;
  f->stream = f->buffer+f->strpos;
  return 0;
}

