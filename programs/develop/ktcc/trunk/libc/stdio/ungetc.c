#include <stdio.h>
// non standard realization - support for virtually change ONLY ONE char



int ungetc(int c,FILE* file)
{
	dword res;

    if(!file)
    {
        errno = E_INVALIDPTR;
        return EOF;
    }

	if ((file->mode & 3) != FILE_OPEN_READ && (file->mode & FILE_OPEN_PLUS) == 0)
    {
        errno = E_ACCESS;
        return EOF;
    }

	if (file->filepos > file->filesize || file->filepos == 0 || c == EOF || file->ungetc_buf != EOF)
	{
	    errno = E_EOF;
		return EOF;
	}
	
	file->ungetc_buf = c;
	file->filepos--;

	return c;
}
