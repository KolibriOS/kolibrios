#include <sys/ksys.h>
#include <conio.h>
#include "stdio.h"

static char* __con_caption = "Console application";
static char* __con_dllname = "/sys/lib/console.obj";

int __con_is_load = 0;

void  __stdcall (*__con_init_hidden)(int wnd_width, unsigned wnd_height, int scr_width, int scr_height, const char* title);
void  __stdcall (*con_exit)(int);
void  __stdcall (*con_set_title)(const char* title);
void  __stdcall (*con_write_asciiz)(const char* str);
void  __stdcall (*con_write_string)(const char* str, dword length);
int   __cdecl   (*con_printf)(const char* format, ...);
dword __stdcall (*con_get_flags)(void);
dword __stdcall (*con_set_flags)(dword new_flags);
int   __stdcall (*con_get_font_height)(void);
int   __stdcall (*con_get_cursor_height)(void);
int   __stdcall (*con_set_cursor_height)(int new_height);
int   __stdcall (*con_getch)(void);
word  __stdcall (*con_getch2)(void);
int   __stdcall (*con_kbhit)(void);
char* __stdcall (*con_gets)(char* str, int n);
char* __stdcall (*con_gets2)(con_gets2_callback callback, char* str, int n);
void  __stdcall (*con_cls)();
void  __stdcall (*con_get_cursor_pos)(int* px, int* py);
void  __stdcall (*con_set_cursor_pos)(int x, int y);

static void __con_lib_link(ksys_dll_t *exp)
{
    __con_init_hidden = _ksys_dlsym(exp, "con_init");
    con_exit          = _ksys_dlsym(exp, "con_exit");
    con_set_title     = _ksys_dlsym(exp, "con_set_title");
    con_write_asciiz  = _ksys_dlsym(exp, "con_write_asciiz");
    con_write_string  = _ksys_dlsym(exp, "con_write_string");
    con_printf        = _ksys_dlsym(exp, "con_printf");
    con_get_flags     = _ksys_dlsym(exp, "con_get_flags");
    con_set_flags     = _ksys_dlsym(exp, "con_set_flags");
    con_get_font_height = _ksys_dlsym(exp, "con_get_font_height");
    con_get_cursor_height = _ksys_dlsym(exp, "con_get_cursor_height");
    con_set_cursor_height = _ksys_dlsym(exp, "con_set_cursor_height");
    con_getch           = _ksys_dlsym(exp, "con_getch");
    con_getch2          = _ksys_dlsym(exp, "con_getch2");
    con_kbhit           = _ksys_dlsym(exp, "con_kbhit");
    con_gets            = _ksys_dlsym(exp, "con_gets");
    con_gets2           = _ksys_dlsym(exp, "con_gets2");
    con_cls             = _ksys_dlsym(exp, "con_cls");
    con_get_cursor_pos  = _ksys_dlsym(exp, "con_get_cursor_pos");
    con_set_cursor_pos  = _ksys_dlsym(exp, "con_set_cursor_pos");
}

int con_init_opt(dword wnd_width, dword wnd_height, dword scr_width, dword scr_height, const char* title)
{   
    if(!__con_is_load){
        ksys_dll_t *__con_lib;
        __con_lib = _ksys_dlopen(__con_dllname);
        if(__con_lib==NULL){
            _ksys_debug_puts("Error! Can't load console.obj lib\n");
            return 1;
        }
        __con_lib_link(__con_lib);
        __con_init_hidden(wnd_width, wnd_height, scr_width, scr_height, title); 
        __con_is_load= 1;
        return 0;
    }
    return 1;
}


int con_init(void)
{
    return con_init_opt(-1, -1, -1, -1, __con_caption); 
}

