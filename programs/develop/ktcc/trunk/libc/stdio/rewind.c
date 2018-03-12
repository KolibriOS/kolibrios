#include <stdio.h>
void rewind(FILE* file)
{
    if(!file)
    {
        errno = E_INVALIDPTR;
        return;
    }

	file->ungetc_buf = EOF;
	file->filepos=0;
}
