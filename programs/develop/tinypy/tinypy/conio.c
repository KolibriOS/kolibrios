// Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv3

#include <string.h>

char con_enabled=0;
const char title[]="KTinyPy";

#pragma pack(push,1)
typedef struct{
char    *name;
void    *data;
} kol_struct_import;
#pragma pack(pop)

void (* _stdcall con_init)(unsigned w_w, unsigned w_h, unsigned s_w, unsigned s_h, const char* t);
int (* _cdecl con_printf)(const char* format,...);
void (* _stdcall con_exit)(char bCloseWindow);
void (* __stdcall con_gets)(char* str, int n);
void (* _stdcall con_set_title)(const char* title);

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

void kol_exit()
{
  asm ("int $0x40"::"a"(-1));
}

void console_load()
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
                  kol_cofflib_procload (imp, "con_exit")) ||
      !(con_set_title = ( _stdcall void (*)(const char*))
                  kol_cofflib_procload (imp, "con_set_title")))
  {
    kol_exit();
  }
}

void console_init(){
     con_init(-1, -1, -1, -1, title);
     con_enabled=1;
}
