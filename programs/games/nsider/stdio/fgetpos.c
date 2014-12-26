#include <stdio.h>
int fgetpos(FILE* file,fpos_t* pos)
{
  *pos=file->filepos;
  return 0;
}