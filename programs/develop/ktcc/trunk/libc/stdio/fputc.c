#include <stdio.h>
int fputc(int c,FILE* file)
{
	dword res;
    if(!file)
    {
        errno = E_INVALIDPTR;
        return EOF;
    }

	if ((file->mode & 3)==FILE_OPEN_READ)
    {
        errno = E_ACCESS;
        return EOF;
    }

	file->buffer[0]=c;
	if ((file->mode & 3)==FILE_OPEN_APPEND)
	{
		file->filepos=file->filesize;
		file->filesize++;
		res=_ksys_appendtofile(file->filename,file->filepos,1,file->buffer);
		if (res!=0)
        {
            errno = -res;
            return EOF;
        }
		file->filepos++;
		return c;
	}
	if ((file->mode & 3)==FILE_OPEN_WRITE)
	{
		if (file->filepos==0)
		{	//file not created
			res=_ksys_rewritefile(file->filename,1,file->buffer);
            if (res!=0)
            {
                errno = -res;
                return EOF;
            }
			file->filepos++;
			return c;
		}
		else
		{	//file created and need append one byte
			res=_ksys_appendtofile(file->filename,file->filepos,1,file->buffer);
            if (res!=0)
            {
                errno = -res;
                return EOF;
            }
			file->filepos++;
			return c;
		}
	}
}
