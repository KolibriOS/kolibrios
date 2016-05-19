#include <stdio.h>
#include <kolibrisys.h>

int fread(void *buffer,int size,int count,FILE* file)
{
	dword res;
	dword fullsize;

    if(!file || !buffer)
    {
        errno = E_INVALIDPTR;
        return 0;
    }

	if ((file->mode &3)!=FILE_OPEN_READ && (file->mode & FILE_OPEN_PLUS==0))
    {
        errno = E_ACCESS;
        return 0;
    }

	fullsize=count*size;
	if ((fullsize+file->filepos)>=(file->filesize))
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
	else
    {
        errno = -res;
        return 0;
    }
}
