#include "stdio.h"
#include "string.h"
int fclose(FILE* file)
{
	int res;
	res=_msys_write_file(file->filename,file->filesize,file->buffer);
	free(file->buffer);
	free(file);
	return res;
}