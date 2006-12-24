#include "stdio.h"
#include "string.h"
int fclose(FILE* file)
{
	int res;
	res=_msys_write_file(file->filename, 0, file->filesize, file->buffer);
	free(file->buffer);
	free(file->filename);
	free(file);
	return res;
}