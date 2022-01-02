#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
char    *__scanf_buffer = 0;

int virtual_getc_con(void *sp, const void *obj)
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

//printf("getc '%c'[%d];", ch, ch);
    return ch;
}

void virtual_ungetc_con(void *sp, int c, const void *obj)
// if can, one step back savepoint in s
{
    const char**spc= (const char**)sp;

    if (spc && *spc > __scanf_buffer) (*spc)--;
//printf("Ungetc '%c'[%d];", c, c);
}


int vscanf ( const char * format, va_list arg )
{
    return format_scan(NULL, format, arg, &virtual_getc_con, &virtual_ungetc_con);
};

int scanf ( const char * format, ...)
{
   va_list      arg;
   int  n;
   va_start(arg, format);

   if(__scanf_buffer == NULL) __scanf_buffer = malloc(4096);
   if(__scanf_buffer == NULL) return -3;

   *__scanf_buffer = 0;
   n = vscanf(format, arg);

   va_end(arg);
   return n;
}


