#include "stdio.h"
int fread(void* buffer,int size,int count,FILE* file)
{
	if ((file->mode & 3!=FILE_OPEN_READ) && (file->mode & FILE_OPEN_PLUS==0))
		return 0;
	count=count*size;
	if (count+file->filepos>file->filesize)
		count=file->filesize-file->filepos;
	memcpy(buffer,file->buffer+file->filepos,count);
	file->filepos+=count;
	return count/size;
}