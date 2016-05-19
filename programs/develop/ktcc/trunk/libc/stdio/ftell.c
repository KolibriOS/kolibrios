#include <stdio.h>
long ftell(FILE* file)
{
    if(!file)
    {
        errno = E_INVALIDPTR;
        return -1L;
    }

	return file->filepos;
}
