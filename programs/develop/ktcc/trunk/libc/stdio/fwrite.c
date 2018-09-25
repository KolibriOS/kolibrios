#include <stdio.h>
#include <kolibrisys.h>

// dont support return partial writing when fail
// only 32-bit filesize
int fwrite(void *buffer,int size,int count,FILE* file)
{
	dword res;
	dword fullsize;

    if(!file || !buffer)
    {
        errno = E_INVALIDPTR;
        return EOF;
    }


	if ((file->mode & 3)==FILE_OPEN_READ)
    {
        errno = E_ACCESS;
        return 0;
    }

	if ((file->mode &3)==FILE_OPEN_APPEND)
		file->filepos=file->filesize;

    fullsize=count*size;

	if ((file->filesize)<(file->filepos+fullsize))	file->filesize=file->filepos+fullsize;

	/*
	if (file->mode==FILE_OPEN_APPEND)
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
	*/
	file->ungetc_buf = EOF;
	if (file->filepos >= file->buffer_start && file->filepos < file->buffer_end) // drop buffer, if change his data
	{
		file->buffer_start = -1;
		file->buffer_end = -1;
	}
	
	if ((file->mode &3)==FILE_OPEN_WRITE || (file->mode&3)==FILE_OPEN_APPEND) // always true, as read checked previous
	{
		if (file->filepos==0)
		{	//file mot created yet
			res=_ksys_rewritefile(file->filename,fullsize,buffer);
			if (res==0)
			{
				file->filepos+=fullsize;
				return(count);
			} else
            {
                errno = -res;
                return(0);
            }
		}
		else
		{
			res=_ksys_appendtofile(file->filename,file->filepos,fullsize,buffer);
			if (res==0)
			{
				file->filepos+=fullsize;
				return(count);
			} else
            {
                errno = -res;
                return(0);
            }
		}
	}
	else return(0);
}
