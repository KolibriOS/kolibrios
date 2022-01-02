#include <stdio.h>

int fputs ( const char * str, FILE * file )
{
	int rc;

    if(!file || !str)
    {
        errno = E_INVALIDPTR;
        return EOF;
    }

	if ((file->mode & 3)==FILE_OPEN_READ)
    {
        errno = E_ACCESS;
        return EOF;
    }

    while(*str)
    {
        rc = fputc(*str, file);
        if (rc < 0) return rc;
        str++;
    }

    return 0;
}
