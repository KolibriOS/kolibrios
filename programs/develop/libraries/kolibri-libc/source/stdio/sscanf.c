#include <stdio.h>
#include <stdarg.h>

int virtual_getc_str(void *sp, const void *obj)
// get next chat from string obj, save point is ptr to string char ptr
{
    int ch;
    const char *s = (const char *)obj;
    const char**spc= (const char**)sp;
    if (!s || !spc) return EOF;  // error

    if (!*spc) *spc = s;    // first call, init savepoint

    if (!**spc) return EOF;  // EOS

    ch = **spc; (*spc)++ ;

    return ch;
}

void virtual_ungetc_str(void *sp, int c, const void *obj)
// if can, one step back savepoint in s
{
    const char *s = (const char *)obj;
    const char**spc= (const char**)sp;

    if (s && spc && *spc > s) (*spc)--;
}

int vsscanf ( const char * s, const char * format, va_list arg )
{
    return format_scan(s, format, arg, &virtual_getc_str, &virtual_ungetc_str);
};

int sscanf ( const char * s, const char * format, ...)
{
   va_list      arg;
   int  n;
   va_start(arg, format);

   n = vsscanf(s, format, arg);

   va_end(arg);
   return n;
}

