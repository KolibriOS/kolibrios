#include "stdio.h"
int fwrite(const void* buffer,int size,int count,FILE* file)
{
	void* p;
	if ((file->mode & 3==FILE_OPEN_READ) && (file->mode & FILE_OPEN_PLUS==0))
		return 0;
	if (file->mode & 3==FILE_OPEN_APPEND)
		file->filepos=file->filesize;
        count=count*size;		
	if (file->buffersize<file->filepos+count)
	{
		p=realloc(file->buffer,(file->filepos+count)+(file->filepos+count)<<1);
		if (p==0)
			return 0;
		file->buffer=p;
		file->buffersize=(file->filepos+count)+(file->filepos+count)<<1;
	}
	if (file->filesize<file->filepos+count)
		file->filesize=file->filepos+count;
	memcpy(file->buffer+file->filepos,buffer,count);
	file->filepos+=count;
	return count;
}