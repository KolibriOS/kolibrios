#include "kolibc.h"
#include "kolibri.h"

int write_buffer(FILE *f);
int fill_buff(FILE* f);

int fputc(int c,FILE* f)
{
    if(!((f->mode & FILE_OPEN_WRITE)|(f->mode & FILE_OPEN_PLUS)))
		return EOF;

    if(!f->remain)
    { if (!write_buffer(f))
        return EOF;
      f->filepos+=4096;        
      fill_buff(f);
    };  
        
    *f->stream = (char)c;
    f->stream++;
    f->remain--;
    f->strpos++;
    if((f->filepos+f->strpos) > f->filesize)
      f->filesize=f->filepos+f->strpos;

	return c;
};

int write_buffer(FILE *f)
{ size_t bytes;
  int err;

  bytes= f->filepos+4096 > f->filesize ? f->strpos:4096;   
  err=write_file(f->filename,f->buffer,f->filepos,bytes,&bytes);
  if(err)
    return 0;
  return 1;  
};
