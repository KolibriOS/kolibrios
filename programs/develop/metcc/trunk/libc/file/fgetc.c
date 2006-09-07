#include "stdio.h"
int fgetc(FILE* file)
{
	if ((file->mode & 3!=FILE_OPEN_READ) && (file->mode & FILE_OPEN_PLUS==0))
		return EOF;
	if (file->filepos>=file->filesize)
  		return EOF;
	else
	{
  		return (int)file->buffer[file->filepos++];
	}
}