#include "kolibc.h"

long ftell(FILE* file)
{
      return file->filepos+file->strpos;
}

