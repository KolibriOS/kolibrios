#include <stdio.h>
int fgetpos(FILE* file,fpos_t* pos)
{
    if(!file || !pos)
    {
        errno = E_INVALIDPTR;
        return EOF;
    }

  *pos=file->filepos;
  
  return 0;
}
