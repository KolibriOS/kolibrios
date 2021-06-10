
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

int vfprintf(FILE * file, const char *format, va_list arg)
{
    static char	*buf=NULL;
    int		printed=0, rc = 0;
    
    if(!file){
        errno = EBADF;
        return errno;
    }
    if(!format){
        errno = EINVAL;
        return errno;
    } 

    buf = malloc(STDIO_MAX_MEM);
    
    if(!buf){
        errno = ENOMEM;
        return errno;
    }
    
    printed = vsnprintf(buf, STDIO_MAX_MEM, format,arg);
  	rc = fwrite(buf, sizeof(char), printed, file);
    free(buf);
    return rc;
}
