#include "kolibc.h"

int write_buffer(FILE *f);

int fclose(FILE* file)
{
  int err;
  
//  if((file->mode & FILE_OPEN_WRITE)||
//     (file->mode & FILE_OPEN_APPEND)||
//     (file->mode & FILE_OPEN_PLUS))
//    err=write_buffer(file); 
        
//  UserFree      dlfree(file->buffer);
  dlfree(file->filename);
  dlfree(file);
  return err == 0 ? 0:EOF;
}
