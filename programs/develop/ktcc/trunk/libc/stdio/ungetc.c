#include <stdio.h>
// non standard realization - no support for virtually change char
int ungetc(int c,FILE* file)
{
	dword res;

    if(!file)
    {
        errno = E_INVALIDPTR;
        return EOF;
    }

	if ((file->mode & 3!=FILE_OPEN_READ) && (file->mode & FILE_OPEN_PLUS==0))
    {
        errno = E_ACCESS;
        return EOF;
    }

	if (file->filepos>file->filesize || file->filepos==0)
	{
	    errno = E_EOF;
		return EOF;
	}
	file->filepos--;

	return c;
}
