#include <stdio.h>
#include <mesys.h>

int fwrite(void *buffer,int size,int count,FILE* file)
{
	dword res;
	dword fullsize;

	if ((file->mode & 3==FILE_OPEN_READ) && (file->mode & FILE_OPEN_PLUS==0))	return 0;

	if (file->mode & 3==FILE_OPEN_APPEND)
		file->filepos=file->filesize;
        fullsize=count*size;		
	
	if ((file->filesize)<(file->filepos+fullsize))	file->filesize=file->filepos+fullsize;

	if (file->mode & 3==FILE_OPEN_APPEND)
	{
		file->filepos==file->filesize;
		res=_ksys_appendtofile(file->filename,file->filepos,fullsize,buffer);
		if (res==0)
		{
			file->filepos+=fullsize;
			fullsize=fullsize/size;
			return(fullsize);
		}
		else return(0);
	
	}

	if (file->mode & 3==FILE_OPEN_WRITE)
	{
		if (file->filepos==0)
		{	//file mot craeted yet
			res=_ksys_rewritefile(file->filename,fullsize,buffer);
			if (res==0)
			{
				file->filepos+=fullsize;
				fullsize=fullsize/count;
				return(fullsize);
			}
			else return(0);
		}
		else
		{	
			res=_ksys_appendtofile(file->filename,file->filepos,fullsize,buffer);	
			if (res==0)
			{
				file->filepos+=fullsize;
				fullsize=fullsize/count;
				return(fullsize);
			}
			else return(0);
		}
	}
	else return(0);
}