#include <stdio.h>
#include <string.h>


extern struct{int argc; char** argv;} __ARGS;

char* getfullpath(const char* relpath){
    byte prev_is_slash=0;
    int len=0, depth=0, i;
    char* buff;
    buff = malloc(strlen(__ARGS.argv[0]) + strlen(relpath));
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
    return buff;
}


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
	res->mode=imode;
	res->filename=getfullpath(filename);
//check if file exists
        res=_msys_read_file(res->filename, 0, 0, 0, &res->filesize);
        if (res==5)
        {
          if ((imode & 3) == FILE_OPEN_READ)
          {
            free(res->filename);
            free(res);
            return 0;
          }
          res=_msys_create_file(res->filename);
          if (res!=0)
          {
            free(res->filename);
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
                    free(res->filename);
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
        	res=_msys_read_file(res->filename,0,res->filesize,res->buffer,0);
	        if (res!=0)
        	{
		        free(res->buffer);
        		free(res);
	        }
	        if (imode & 3==FILE_OPEN_APPEND)
	        	res->filepos=res->filesize;
        }
	return res;
}
