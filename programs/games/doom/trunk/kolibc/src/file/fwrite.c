#include "kolibc.h"

int write_buffer(FILE *f);
int fill_buff(FILE* f);

int fwrite(const void* src,size_t size,size_t count,FILE* f)
{
  size_t req;
  size_t cnt;

//append mode unsupported
	
  if(!((f->mode & FILE_OPEN_WRITE)|(f->mode & FILE_OPEN_PLUS)))
    return EOF;
	
  req=count*size;
  count=0;
  while(req)
  {
    if(f->remain)
    { cnt= req > f->remain ? f->remain : req;
	  memcpy(f->stream,src,cnt);
	  f->stream+=cnt;
	  f->strpos+=cnt;
	  f->remain-=cnt;
	  count+=cnt;
	  req-=cnt;
	}
	else
	{
	  if(!write_buffer(f))
	    break;
	  f->filepos+=4096;
	  fill_buff(f);  
	};
  };
  return count/size;
};