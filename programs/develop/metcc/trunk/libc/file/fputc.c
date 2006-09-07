#include "stdio.h"
int fputc(int c,FILE* file)
{
	void* p;
	if ((file->mode & 3)==FILE_OPEN_READ)
		return EOF;
	if ((file->mode & 3)==FILE_OPEN_APPEND)
		file->filepos=file->filesize;
	if (file->filepos==file->filesize)
	{
		file->filesize++;
		if (file->filesize>file->buffersize)
		{
		  p=realloc(file->buffer,file->filesize+file->filesize<<1);
		  if (p==0)
		  	return EOF;
		  file->buffersize=file->filesize+file->filesize<<1;
		  file->buffer=p;
		}
	}
	file->buffer[file->filepos]=(char)c;
	file->filepos++;
	return 0;
}
