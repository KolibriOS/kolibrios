#include <stdio.h>
int feof(FILE* file)
{
    if(!file)
    {
        errno = E_INVALIDPTR;
        return EOF;
    }

    return file->filepos>=file->filesize;
}
