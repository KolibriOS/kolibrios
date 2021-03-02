#include <ksys.h>
#include "conio.h"

static char* __con_caption = "Console application";
static char* __con_dllname = "/sys/lib/console.obj";

int __con_is_load = 0;

unsigned *__con_dll_ver;
void stdcall (*__con_init_hidden)(int wnd_width, int wnd_height,int scr_width, int scr_height, const char* title);
void stdcall (*__con_write_asciiz)(const char* str);
void stdcall (*__con_write_string)(const char* str, unsigned length);
int stdcall (*__con_getch)(void);
short stdcall (*__con_getch2)(void);
int stdcall (*__con_kbhit)(void);
char* stdcall (*__con_gets)(char* str, int n);
char* stdcall (*__con_gets2)(__con_gets2_callback callback, char* str, int n);
void stdcall (*__con_exit)(int status);

static void __con_lib_link(ksys_coff_etable_t *exp)
{
    __con_dll_ver       = _ksys_cofflib_getproc(exp, "con_dll_ver");
    __con_init_hidden   = _ksys_cofflib_getproc(exp, "con_init");
    __con_write_asciiz  = _ksys_cofflib_getproc(exp, "con_write_asciiz");
    __con_write_string  = _ksys_cofflib_getproc(exp, "con_write_string");
    __con_getch         = _ksys_cofflib_getproc(exp, "con_getch");
    __con_getch2        = _ksys_cofflib_getproc(exp, "con_getch2");
    __con_kbhit         = _ksys_cofflib_getproc(exp, "con_kbhit");
    __con_gets          = _ksys_cofflib_getproc(exp, "con_gets");
    __con_gets2         = _ksys_cofflib_getproc(exp, "con_gets2");
    __con_exit          = _ksys_cofflib_getproc(exp, "con_exit");
}


int __con_init(void)
{
    return __con_init_opt(-1, -1, -1, -1, __con_caption); 
}


int __con_init_opt(int wnd_width, int wnd_height,int scr_width, int scr_height, const char* title)
{   
    if(!__con_is_load){
        ksys_coff_etable_t *__con_lib;
        __con_lib = _ksys_cofflib_load(__con_dllname);
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
