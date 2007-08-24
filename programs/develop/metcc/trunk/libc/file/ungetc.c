#include "stdio.h"
int ungetc(int c,FILE* file)
{
	dword res;

	if (c==EOF)
		return EOF;
	if (file->filepos<=0 || file->filepos>file->filesize)
		return EOF;
	file->filepos--;
	res=_ksys_readfile(file->filename,file->filepos,1,file->buffer);
	if (res==0)
	{
		return(c);
	}
	else return(EOF);
}
