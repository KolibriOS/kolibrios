#include <stdio.h>
int fputc(int c,FILE* file)
{
	dword res;

	if ((file->mode & 3)==FILE_OPEN_READ) return EOF;

	file->buffer[0]=c;
	if ((file->mode & 3)==FILE_OPEN_APPEND)
	{
		file->filepos=file->filesize;
		file->filesize++;
		res=_ksys_appendtofile(file->filename,file->filepos,1,file->buffer);
		if (res!=0) return(res);
		file->filepos++;
		return(0);
	}
	if ((file->mode & 3)==FILE_OPEN_WRITE)
	{
		if (file->filepos==0)
		{	//file not craeted 
			res=_ksys_rewritefile(file->filename,1,file->buffer);
			if (res!=0) return(res);
			file->filepos++;
			return 0;
		}
		else
		{	//file craeted and need append one byte 
			res=_ksys_appendtofile(file->filename,file->filepos,1,file->buffer);
			if (res!=0) return(res);
			file->filepos++;
			return 0;
		}
	}
}
