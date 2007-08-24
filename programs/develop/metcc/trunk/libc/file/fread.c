#include <stdio.h>
#include <mesys.h>
int fread(void *buffer,int size,int count,FILE* file)
{
	dword res;
	dword fullsize;

	if ((file->mode & 3!=FILE_OPEN_READ) && (file->mode & FILE_OPEN_PLUS==0))	return 0;

	fullsize=count*size;
	if ((fullsize+file->filepos)>(file->filesize))
	{
		fullsize=file->filesize-file->filepos;
		if (fullsize<=0) return(0);
	}

	res=_ksys_readfile(file->filename,file->filepos,fullsize,buffer);
	if (res==0)
	{	
		file->filepos=file->filepos+fullsize;
		fullsize=fullsize/size;
		return(fullsize);
	}
	else	return 0;
}