#include <stdio.h>
#include <string.h>

void clearerr ( FILE * stream )
{
    errno = 0;
}

int ferror ( FILE * stream )
{
    return errno;
}

void perror ( const char * str )
{
    char *msg = strerror(errno);

    if (str)
        fprintf(stderr, "%s:%s\n", str, msg);
    else
        fprintf(stderr, "%s\n", msg);
}
