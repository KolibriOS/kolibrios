#include "stdio.h"
int ungetc(int c,FILE* file)
{
	if (c==EOF)
		return EOF;
	if (file->filepos<=0 || file->filepos>file->filesize)
		return EOF;
	file->filepos--;
	file->buffer[file->filepos]=(char)c;
	return c;
}
