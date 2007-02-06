
#include "kolibri.h"
#include <string.h>
#include <malloc.h>
#include "kolibc.h"

extern char *__appcwd;
extern int __appcwdlen;

char* getfullpath(char* path)
{
    int prev_is_slash=0;
    int len=0, depth=0, i;
    char* buff;
    char c;
    
    len= __appcwdlen; 
    
    buff = (char*)malloc(len+strlen(path)+1);
    strncpy(buff, __appcwd, __appcwdlen);
    
    if(*path == '/')
    { buff[0] = '\0';
      len=0;
    }   
    else
    {
      prev_is_slash = 1;
      buff[len] = 0;
      for(i=0; buff[i]; i++)
        if(buff[i] == '/' && i < len-1) depth++;
    }
    
    while(c=*path++)
    {
      switch (c)
      {
      
        case '.':
          if((*path == '.')&&
             (*path+1)== '/')
          { if(!depth)
            {  free(buff);
               return 0;
            };
            buff[len-1] = 0;
            len = strrchr(buff, '/') + 1 - buff;
            buff[len] = 0;
            depth--;
            path +=2;
            prev_is_slash = 1;
            continue;
          }
          if(*path == '/')
          {
            path++;
            prev_is_slash = 1;
            continue;
          }
          buff[len++] = c;
          continue;
      
        case '/':
          prev_is_slash = 1;
          buff[len++] = c;
          continue;
          
        default:
          prev_is_slash = 0;
          buff[len++] = c;
          continue;
        };
    };
    buff[len]= '\0';
    return buff;
}

FILE* fopen(const char* filename, const char *mode)
{
    FILEINFO fileinfo;
        FILE* res;
    char *path;
        int err;
        int imode;
    size_t bytes;
    
    imode=0;
    if (*mode=='r')
    {
      imode=FILE_OPEN_READ;     
       mode++;
    }
    else
    if (*mode=='w')
    {
      imode=FILE_OPEN_WRITE;
       mode++;
    }
    else
    if (*mode=='a')
    {
      imode=FILE_OPEN_APPEND;
      mode++;
    }
    else
      return 0;
    if (*mode=='t')
    {
      imode|=FILE_OPEN_TEXT;
      mode++;
    }
    else
    if (*mode=='b')
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

    res->buffer=UserAlloc(8192);
    res->stream=res->buffer;
    res->strpos=0;
    res->remain=8192;
    res->buffersize=8192;
    res->filesize=fileinfo.size;
    res->filename=path;
    res->mode=imode;
        
    if (imode & FILE_OPEN_APPEND)
    { size_t bytes;
      res->strpos= res->filesize & 8191;
      if (res->strpos)    //not align
      {
        res->filepos=res->filesize & -8192; // align buffer
        res->remain=8192-res->strpos;      
        err=read_file(res->filename,res->buffer,
                      res->filesize,res->remain,&bytes);
        res->stream=res->buffer+res->strpos;
      };
    }  
    else
    { size_t bytes;
    
       err=read_file(res->filename,res->buffer,
                    0,8192,&bytes);
       res->filepos=0;
    };
    return res;
}

size_t FileSize(FILE *handle) 
{
  return handle->filesize;
 
}
