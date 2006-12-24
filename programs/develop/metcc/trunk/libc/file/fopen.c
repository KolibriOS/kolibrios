#include "stdio.h"
#include "string.h"
FILE* fopen(const char* filename, const char *mode)
{
	FILE* res;
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
	res=malloc(sizeof(FILE));
	res->buffer=0;
	res->buffersize=0;
	res->filesize=0;
	res->filepos=0;
	res->filename=0;
	res->mode=imode;
//check if file exists
        res=_msys_read_file(filename, 0, 0, 0, &res->filesize);
        if (res==5)
        {
          if ((imode & 3) == FILE_OPEN_READ)
          {
            free(res);
            return 0;
          }
          res=_msys_create_file(filename);
          if (res!=0)
          {
            free(res);
            return 0;
          }
          res->filesize=0;
        }
        if (imode & 3==FILE_OPEN_WRITE)
        {
                res->buffersize=512;
                res->buffer=malloc(res->buffersize);
                if (res->buffer=0)
                {
                	free(res);
                	return 0;
                }
                res->filesize=0;
        }else
        {
        	res->buffersize=(res->filesize & (~511))+512;
	        res->buffer=malloc(res->buffersize);
        	if (res->buffer==0)
	        {
        		free(res);
		        return 0;
	        }
        	res=_msys_read_file(filename,0,res->filesize,res->buffer,0);
	        if (res!=0)
        	{
		        free(res->buffer);
        		free(res);
	        }
	        if (imode & 3==FILE_OPEN_APPEND)
	        	res->filepos=res->filesize;
        }
        res->filename=strdup(filename);
	return res;
}
