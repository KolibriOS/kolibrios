#include <stdio.h>
#include <stdlib.h>



int fprintf(FILE* file, const char* format, ...)
{
    va_list		arg;
    va_start (arg, format);

    return vfprintf(file, format, arg);

}

int vfprintf ( FILE * file, const char * format, va_list arg )
{
    char		*buf;
    int		printed, rc = 0;

    if(!file || !format)
    {
        errno = E_INVALIDPTR;
        return errno;
    }

    buf=malloc(4096*2); //8kb max
    if(!buf)
    {
        errno = E_NOMEM;
        return errno;
    }

  printed=format_print(buf,8191, format,arg);
  if (file == stderr)
  	debug_out_str(buf);
  else
  	rc = fwrite(buf,printed,1,file);
  free(buf);

  if (rc < 0)
    return rc;
  else
    return(printed);
}
