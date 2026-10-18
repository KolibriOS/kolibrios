#include <stdio.h>
#include <limits.h>
#include <errno.h>

long int ftell(FILE* stream)
{
    fpos_t pos = stream->position;

    if (pos > INT_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
	
    return pos;
}
