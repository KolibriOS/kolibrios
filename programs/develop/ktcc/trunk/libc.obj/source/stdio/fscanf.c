#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

int virtual_getc_file(void *sp, const void *obj)
// get next chat from file obj, save point is ptr to string char ptr
{
    FILE *f = (FILE *)obj;
    int     ch = fgetc(f);

//printf("getc '%c'[%d];", ch, ch);

    return ch;
}

void virtual_ungetc_file(void *sp, int c, const void *obj)
// if can, one step back savepoint in s
{
    FILE *f = (FILE *)obj;

    if (f) ungetc(c, f);
}


int vfscanf ( FILE * stream, const char * format, va_list arg )
{
    return format_scan(stream, format, arg, &virtual_getc_file, &virtual_ungetc_file);
};

int fscanf ( FILE * stream, const char * format, ...)
{
   va_list      arg;
   int  n;
   va_start(arg, format);

   n = vfscanf(stream, format, arg);

   va_end(arg);
   return n;
}


