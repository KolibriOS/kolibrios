#include "kolibc.h"
#include "kolibri.h"

int fill_buff(FILE* f);

int fread(void* dst,size_t size,size_t count,FILE* f)
{ size_t req;
  size_t cnt;
  char *p = dst;
  if(!((f->mode & FILE_OPEN_READ)|(f->mode & FILE_OPEN_PLUS)))
		return 0;
  req=count*size;
  if (req+f->filepos+f->strpos > f->filesize)
		req=f->filesize-f->filepos-f->strpos;
  count=0;		
  while(req)
  {
    if (f->remain)
    { cnt= req > f->remain ? f->remain : req;
	  memcpy(p,f->stream,cnt);
	  p+=cnt;
	  f->stream+=cnt;
	  f->strpos+=cnt;
	  f->remain-=cnt;
	  count+=cnt;
	  req-=cnt;
	}
	else
	{
	  f->filepos+=4096;
	  if(!fill_buff(f))  //eof or error
	    break;
	}  
  };
  return count/size;
}

int fill_buff(FILE* f)
{ int err;
  size_t bytes;
  
  err=read_file(f->filename,f->buffer,f->filepos,
                             4096,&bytes);
  if (err) return 0; 
  if(!bytes) return 0;
  f->stream = f->buffer;
  f->remain = 4096;
  f->strpos=0;
  return bytes; 
};