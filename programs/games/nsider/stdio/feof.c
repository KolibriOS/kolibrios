#include <stdio.h>
int feof(FILE* file)
{
  return file->filepos>=file->filesize;
}