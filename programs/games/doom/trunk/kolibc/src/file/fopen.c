#include "kolibc.h"
#include "kolibri.h"

extern struct{int argc; char** argv;} __ARGS;

char* getfullpath(const char* relpath){
    byte prev_is_slash=0;
    int len=0, depth=0, i;
    char* buff;
    buff = (char*)malloc(strlen(relpath));
    strcpy(buff, relpath);
/**********    
    buff = (char*)malloc(strlen(__ARGS.argv[0]) + strlen(relpath));
    
    if(*relpath == '/') buff[0] = '\0';
    else {
        len = strrchr(__ARGS.argv[0], '/') - __ARGS.argv[0] + 1;
        strncpy(buff, __ARGS.argv[0], len);
        prev_is_slash = 1;
    	buff[len] = 0;
        for(i=0; buff[i]; i++)
            if(buff[i] == '/' && i < len-1) depth++;
    }
    for (; *relpath; relpath++){
        switch (*relpath){
            case '/':
                prev_is_slash = 1;
                buff[len++] = '/';
                break;
            case '.':
                if(*(relpath+1) == '.' && *(relpath+2) == '/' && prev_is_slash){
                    if(!depth) return 0;
                    buff[len-1] = 0;
                    len = strrchr(buff, '/') + 1 - buff;
                    buff[len] = 0;
                    depth--;
                    relpath += 2;
                } else {
                    depth++;
                    buff[len++] = '.';
                }
                break;
            default:
                if(prev_is_slash){ 
                    depth++;
                    prev_is_slash = 0;
                }
                buff[len++] = *relpath;
                break;
            }
    }
    buff[len]= '\0';
*************/    
    return buff;
   
}

FILE* fopen(const char* filename, const char *mode)
{
    FILEINFO fileinfo;
	FILE* res;
    char *path;
	int err;
	int imode;
    
	imode=0;
	if (*mode=='r')
	{
		imode=FILE_OPEN_READ;	  
		mode++;
	}else if (*mode=='w')
	{
		imode=FILE_OPEN_WRITE;
		mode++;
	}else if (*mode=='a')
	{
		imode=FILE_OPEN_APPEND;
		mode++;
	}else
		return 0;
	if (*mode=='t')
	{
		imode|=FILE_OPEN_TEXT;
		mode++;
	}else if (*mode=='b')
		mode++;
	if (*mode=='+')
	{
		imode|=FILE_OPEN_PLUS;
		mode++;
	}
	if (*mode!=0)
		return 0;
	
	path= getfullpath(filename);
    err=get_fileinfo(path, &fileinfo);
    if(err)
    {
      if ((imode & 7)== 0)
      { free(path);
        return 0;
      };
  //    err=_msys_create_file(path);
      if (err)
      {  free(path);
         return 0;
      }
      fileinfo.size=0;
    };
		
	res=(FILE*)malloc(sizeof(FILE));
	if(!res)
	{ free(path);
	  return 0;
	};
	
    res->buffer=malloc(4096);
	res->stream=res->buffer;
	res->strpos=0;
	res->remain=4096;
	res->buffersize=4096;
    res->filesize=fileinfo.size;
    res->filename=path;
    res->mode=imode;
	
    if (imode & FILE_OPEN_APPEND)
    { size_t bytes;
      res->strpos= res->filesize & 4095;
      if (res->strpos)    //not align
      {
	    res->filepos=res->filesize & -4096; // align buffer
        res->remain=4096-res->strpos;	   
        err=read_file(res->filename,res->buffer,
                      res->filesize,res->remain,&bytes);
        res->stream=res->buffer+res->strpos;
	  };
	}  
    else
    { size_t bytes;
    
      err=read_file(res->filename,res->buffer,
                    0,4096,&bytes);
	  res->filepos=0;
    };
	return res;
}
