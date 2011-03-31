#include "kolibri.h"

#define __stdcall __attribute__((stdcall))
#define _cdecl __attribute__((cdecl))
#define _stdcall __attribute__((stdcall))

void (* _stdcall con_init)(unsigned w_w, unsigned w_h, unsigned s_w, unsigned s_h, const char* t);
int (* _cdecl con_printf)(const char* format,...);
void (* _stdcall con_exit)(char bCloseWindow);
void (* __stdcall con_gets)(char* str, int n);

void CONSOLE_INIT(const char title[])
{
  kol_struct_import *imp;

  if (!(imp = kol_cofflib_load("/sys/lib/console.obj")) ||
      !(con_init = ( _stdcall  void (*)(unsigned, unsigned, unsigned, unsigned, const char*))
                  kol_cofflib_procload (imp, "con_init")) ||
      !(con_printf = ( _cdecl int (*)(const char*,...))
                  kol_cofflib_procload (imp, "con_printf"))||
      !(con_gets = ( __stdcall void (*)(char*, int))
                  kol_cofflib_procload (imp, "con_gets"))||
      !(con_exit = ( _stdcall void (*)(char))
                  kol_cofflib_procload (imp, "con_exit")))
  {
    kol_exit();
  }
  con_init(-1, -1, -1, -1, title);
}
kol_struct_import* kol_cofflib_load(char *name)
{
  kol_struct_import* result;
  asm ("int $0x40":"=a"(result):"a"(68), "b"(19), "c"(name));
  return result;
}

void* kol_cofflib_procload (kol_struct_import *imp, char *name)
{
  int i;
  for (i=0;;i++)
  {
    if ( NULL == ((imp+i) -> name))
      break;
    else
      if ( 0 == strcmp(name, (imp+i)->name) )
        return (imp+i)->data;
  }
  return NULL;
}

void kol_board_puts(char *s)
{
  unsigned i;
  i = 0;
  while (*(s+i))
  {
    asm ("int $0x40"::"a"(63), "b"(1), "c"(*(s+i)));
    i++;
  }
}

void kol_exit()
{
  asm ("int $0x40"::"a"(-1));
}
