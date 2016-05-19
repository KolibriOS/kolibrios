#include <stdio.h>
void rewind(FILE* file)
{
    if(!file)
    {
        errno = E_INVALIDPTR;
        return;
    }

	file->filepos=0;
}
