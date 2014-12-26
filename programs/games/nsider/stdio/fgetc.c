#include <stdio.h>
int fgetc(FILE* file)
{
	dword res;

	if ((file->mode & 3!=FILE_OPEN_READ) && (file->mode & FILE_OPEN_PLUS==0)) return EOF;

	if (file->filepos>=file->filesize)
	{
		return EOF;
	}
	else
	{
		res=_ksys_readfile(file->filename,file->filepos,1,file->buffer);
		if (res==0)
		{
			file->filepos++;
  			return (int)file->buffer[0];
		}
		else return(res);
	}
}