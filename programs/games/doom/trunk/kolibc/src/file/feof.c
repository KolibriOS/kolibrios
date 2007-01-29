#include "kolibc.h"
int feof(FILE* file)
{
  return (file->filepos-file->strpos)>=file->filesize;
}