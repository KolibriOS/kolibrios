#include "format_scan.h"

static int __virtual_getc_str(void *sp, const void *obj)
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

void __virtual_ungetc_str(void *sp, int c, const void *obj)
// if can, one step back savepoint in s
{
    const char *s = (const char *)obj;
    const char**spc= (const char**)sp;

    if (s && spc && *spc > s) (*spc)--;
}

int vsscanf(const char * s, const char * format, va_list arg)
{
    return _format_scan(s, format, arg, &__virtual_getc_str, &__virtual_ungetc_str);
};