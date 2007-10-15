#include <stdio.h>
long ftell(FILE* file)
{
	return file->filepos;
}