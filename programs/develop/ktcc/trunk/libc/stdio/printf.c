#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <stdarg.h>

int printf(const char *format, ...)
{
   va_list      arg;
   va_start(arg, format);

   return vprintf(format, arg);
}


int vprintf ( const char * format, va_list arg )
{
   int          i = 0;
   int          printed_simbols = 0;
   char         *s;

   i=con_init_console_dll();

   if (i == 0)
   {
     s = malloc(4096);
     printed_simbols = format_print(s, 4096, format, arg);
     con_write_string(s, printed_simbols);
     free(s);
   }

   return(printed_simbols);
}
