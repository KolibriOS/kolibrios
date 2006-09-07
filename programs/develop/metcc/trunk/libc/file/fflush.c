#include "stdio.h"
int fflush(FILE* file)
{
	if ((file->mode & 3)==FILE_OPEN_READ)
  		return 0;
  	return _msys_file_write(file->filename,file->filesize,file->buffer) ? EOF : 0;
}