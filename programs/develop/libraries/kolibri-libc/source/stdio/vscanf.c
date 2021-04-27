#include <string.h>

char *__scanf_buffer=NULL;
typedef int (*virtual_getc)(void *sp, const void *obj);
typedef void (*virtual_ungetc)(void *sp, int c, const void *obj);

char *__scanf_buffer;

extern int _format_scan(const void *src, const char *fmt, va_list argp, virtual_getc vgetc, virtual_ungetc vungetc);

static int __virtual_getc_con(void *sp, const void *obj)
// get next chat from string obj, save point is ptr to string char ptr
{
    int ch;
    const char**spc= (const char**)sp;
    if (!spc) return EOF;  // error
    if (!*spc) *spc = __scanf_buffer;    // first call, init savepoint

    while (!**spc)  // need to read more
    {
        if(!gets(__scanf_buffer)) return EOF;
        *spc = __scanf_buffer;
        strcat(__scanf_buffer,"\n");    // imitate delimiter
    }
    if (**spc == 26 || **spc == 3)  // ^C ^Z end of scan, clear buffer
    {
        *spc = __scanf_buffer;
        *__scanf_buffer = 0;
        return EOF;  // ^C ^Z
    }

    ch = **spc; (*spc)++ ;
    return ch;
}

static void __virtual_ungetc_con(void *sp, int c, const void *obj)
// if can, one step back savepoint in s
{
    const char**spc= (const char**)sp;

    if (spc && *spc > __scanf_buffer) (*spc)--;
//printf("Ungetc '%c'[%d];", c, c);
}


int vscanf(const char * format, va_list arg)
{
    return _format_scan(NULL, format, arg, &__virtual_getc_con, &__virtual_ungetc_con);
}
