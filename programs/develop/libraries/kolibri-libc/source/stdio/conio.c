#include <sys/ksys.h>
#include <conio.h>
#include "stdio.h"

static char* __con_caption = "Console application";
static char* __con_dllname = "/sys/lib/console.obj";

int __con_is_load = 0;

void  stdcall (*__con_init_hidden)(int wnd_width, unsigned wnd_height, int scr_width, int scr_height, const char* title);
void  stdcall (*con_exit)(int);
void  stdcall (*con_set_title)(const char* title);
void  stdcall (*con_write_asciiz)(const char* str);
void  stdcall (*con_write_string)(const char* str, dword length);
int   cdecl   (*con_printf)(const char* format, ...);
dword stdcall (*con_get_flags)(void);
dword stdcall (*con_set_flags)(dword new_flags);
int   stdcall (*con_get_font_height)(void);
int   stdcall (*con_get_cursor_height)(void);
int   stdcall (*con_set_cursor_height)(int new_height);
int   stdcall (*con_getch)(void);
word  stdcall (*con_getch2)(void);
int   stdcall (*con_kbhit)(void);
char* stdcall (*con_gets)(char* str, int n);
char* stdcall (*con_gets2)(con_gets2_callback callback, char* str, int n);
void  stdcall (*con_cls)();
void  stdcall (*con_get_cursor_pos)(int* px, int* py);
void  stdcall (*con_set_cursor_pos)(int x, int y);

static void __con_panic(char* func_name)
{
    debug_printf("In console.obj %s=NULL!\n", func_name);
    _ksys_exit();
}

static void __con_lib_link(ksys_coff_etable_t *exp)
{
    __con_init_hidden = _ksys_get_coff_func(exp, "con_init", __con_panic);
    con_exit          = _ksys_get_coff_func(exp, "con_exit", __con_panic);
    con_set_title     = _ksys_get_coff_func(exp, "con_set_title", __con_panic);
    con_write_asciiz  = _ksys_get_coff_func(exp, "con_write_asciiz", __con_panic);
    con_write_string  = _ksys_get_coff_func(exp, "con_write_string", __con_panic);
    con_printf        = _ksys_get_coff_func(exp, "con_printf", __con_panic);
    con_get_flags     = _ksys_get_coff_func(exp, "con_get_flags", __con_panic);
    con_set_flags     = _ksys_get_coff_func(exp, "con_set_flags", __con_panic);
    con_get_font_height = _ksys_get_coff_func(exp, "con_get_font_height", __con_panic);
    con_get_cursor_height = _ksys_get_coff_func(exp, "con_get_cursor_height", __con_panic);
    con_set_cursor_height = _ksys_get_coff_func(exp, "con_set_cursor_height", __con_panic);
    con_getch           = _ksys_get_coff_func(exp, "con_getch", __con_panic);
    con_getch2          = _ksys_get_coff_func(exp, "con_getch2", __con_panic);
    con_kbhit           = _ksys_get_coff_func(exp, "con_kbhit", __con_panic);
    con_gets            = _ksys_get_coff_func(exp, "con_gets", __con_panic);
    con_gets2           = _ksys_get_coff_func(exp, "con_gets2", __con_panic);
    con_cls             = _ksys_get_coff_func(exp, "con_cls", __con_panic);
    con_get_cursor_pos  = _ksys_get_coff_func(exp, "con_get_cursor_pos", __con_panic);
    con_set_cursor_pos  = _ksys_get_coff_func(exp, "con_set_cursor_pos", __con_panic);
}

int con_init_opt(dword wnd_width, dword wnd_height, dword scr_width, dword scr_height, const char* title)
{   
    if(!__con_is_load){
        ksys_coff_etable_t *__con_lib;
        __con_lib = _ksys_load_coff(__con_dllname);
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

