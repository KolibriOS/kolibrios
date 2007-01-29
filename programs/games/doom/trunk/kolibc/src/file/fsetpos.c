#include "kolibc.h"
#include "kolibri.h"

int fsetpos(FILE* f,const fpos_t * pos)
{ int err;
  size_t bytes;
  
  if (*pos>=0)
  {
    bytes= *pos & -4096;
    err=read_file(f->filename,f->buffer,bytes,
                             4096,&bytes);
    if (err) return EOF; 
    if(!bytes) return EOF;    
    f->filepos= *pos & -4096;
    f->strpos = *pos & 4095;
    f->remain = 4096-f->strpos;
    f->stream = f->buffer+f->strpos;
    return 0;
  }
  else
    return EOF;
}