#include <stdio.h>
void rewind(FILE* file)
{
	file->filepos=0;
}