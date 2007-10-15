
#include <stdio.h>
#include <string.h>
#include <kolibrisys.h>

char* dllname="/sys/lib/console.obj";
int   console_init_status;

char* imports[] = {"START","version","con_init","con_write_asciiz","con_printf","con_exit",NULL};
char* caption = "Console test - colors";

dword* dll_ver;
void stdcall (* con_init)(dword wnd_width, dword wnd_height, dword scr_width, dword scr_height, const char* title);
void stdcall (* con_write_asciiz)(const char* string);
void cdecl  (* con_printf)(const char* format,...);
void stdcall (* con_exit)(dword bCloseWindow);

struct import{
        char *name;
        void *data;
};

void printf_link(struct import *exp, char** imports){

        dll_ver = (dword*)
                _ksys_cofflib_getproc(exp, imports[1]);
        con_init = (void stdcall (*)(dword , dword, dword, dword, const char*))
                _ksys_cofflib_getproc(exp, imports[2]);
        con_printf = (void cdecl (*)(const char*,...))
                _ksys_cofflib_getproc(exp, imports[4]);
        con_exit = (void stdcall (*)(dword))
                _ksys_cofflib_getproc(exp, imports[5]);
}

int init_console(void)
{
  struct import * hDll;

        if((hDll = (struct import *)_ksys_cofflib_load(dllname)) == 0){
                debug_out_str("can't load lib\n");
                return 1;
        }
        printf_link(hDll, imports);
        debug_out_str("dll loaded\n");

        con_init(-1, -1, -1, -1, caption);
        return(0);
}

int printf(const char *format,...)
{
   int          i;
   int          printed_simbols;
   va_list      arg;
   char         simbol[]={"%s"};
   char         *s;

   va_start(arg,format);

   if (console_init_status==0)
    {
      i=init_console();
      console_init_status=1;
    }

   if (i==0)
   {
     s=malloc(4096);
     printed_simbols=format_print(s,4096,format,arg);
     con_printf(simbol,s);
     free(s);
   }
   return(printed_simbols);
}

