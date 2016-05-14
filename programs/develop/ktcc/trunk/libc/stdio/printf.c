#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

int printf(const char *format, ...)
{
   int          i = 0;
   int          printed_simbols = 0;
   va_list      arg;
   char         *s;

   va_start(arg,format);

   i=con_init_console_dll();

   if (i==0)
   {
     s=malloc(4096);
     printed_simbols=format_print(s,4096,format,arg);
     con_write_string(s, printed_simbols);
     free(s);
   }
   return(printed_simbols);
}

