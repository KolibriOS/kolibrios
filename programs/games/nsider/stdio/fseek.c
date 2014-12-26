#include <stdio.h>
int fseek(FILE* file,long offset,int origin)
{
	if (origin==SEEK_CUR)
		offset+=file->filepos;
	else if (origin==SEEK_END)
		offset+=file->filesize;
	else if (origin!=SEEK_SET)
		return EOF;
	return fsetpos(file,offset);		
}