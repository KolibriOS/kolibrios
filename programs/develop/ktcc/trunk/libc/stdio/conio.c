#include <conio.h>
#include <kolibrisys.h>

char* con_caption = "Console app";
extern int __argc;
extern char** __argv;
extern char* __path;
dword	*con_dll_ver;
int   __console_initdll_status;

char* con_dllname="/sys/lib/console.obj";

struct import{
        char *name;
        void *data;
};

void stdcall (*con_init)(dword wnd_width, dword wnd_height,
	dword scr_width, dword scr_height, const char* title);
void stdcall (*con_exit)(int bCloseWindow);
void stdcall (*con_set_title)(const char* title);
void stdcall (*con_write_asciiz)(const char* str);
void stdcall (*con_write_string)(const char* str, dword length);
int cdecl (*con_printf)(const char* format, ...);
dword stdcall (*con_get_flags)(void);
dword stdcall (*con_set_flags)(dword new_flags);
int stdcall (*con_get_font_height)(void);
int stdcall (*con_get_cursor_height)(void);
int stdcall (*con_set_cursor_height)(int new_height);
int stdcall (*con_getch)(void);
word stdcall (*con_getch2)(void);
int stdcall (*con_kbhit)(void);
char* stdcall (*con_gets)(char* str, int n);
char* stdcall (*con_gets2)(con_gets2_callback callback, char* str, int n);
void stdcall (*con_cls)();
void stdcall (*con_get_cursor_pos)(int* px, int* py);
void stdcall (*con_set_cursor_pos)(int x, int y);


// don't change order in this! linked by index
char* con_imports[] = {
	"START", "version", "con_init", "con_write_asciiz", "con_write_string",
	"con_printf", "con_exit", "con_get_flags", "con_set_flags", "con_kbhit",
	"con_getch", "con_getch2", "con_gets", "con_gets2", "con_get_font_height",
	"con_get_cursor_height", "con_set_cursor_height",  "con_cls", 
	"con_get_cursor_pos", "con_set_cursor_pos", "con_set_title",
	(char*)0
};

void con_lib_link(struct import *exp, char** imports){

        con_dll_ver 		= _ksys_cofflib_getproc(exp, imports[1]);
        con_init 			= _ksys_cofflib_getproc(exp, imports[2]);
        con_write_asciiz	= _ksys_cofflib_getproc(exp, imports[3]);
        con_write_string	= _ksys_cofflib_getproc(exp, imports[4]);
        con_printf 			= _ksys_cofflib_getproc(exp, imports[5]);
        con_exit 			= _ksys_cofflib_getproc(exp, imports[6]);
        con_get_flags 		= _ksys_cofflib_getproc(exp, imports[7]);
        con_set_flags 		= _ksys_cofflib_getproc(exp, imports[8]);
        con_kbhit	 		= _ksys_cofflib_getproc(exp, imports[9]);
        con_getch			= _ksys_cofflib_getproc(exp, imports[10]);
        con_getch2	 		= _ksys_cofflib_getproc(exp, imports[11]);
        con_gets	 		= _ksys_cofflib_getproc(exp, imports[12]);
        con_gets2	 		= _ksys_cofflib_getproc(exp, imports[13]);
        con_get_font_height	= _ksys_cofflib_getproc(exp, imports[14]);
        con_get_cursor_height=_ksys_cofflib_getproc(exp, imports[15]);
        con_set_cursor_height=_ksys_cofflib_getproc(exp, imports[16]);
        con_cls		 		= _ksys_cofflib_getproc(exp, imports[17]);
        con_get_cursor_pos	= _ksys_cofflib_getproc(exp, imports[18]);
		con_set_cursor_pos	= _ksys_cofflib_getproc(exp, imports[19]);
		con_set_title		= _ksys_cofflib_getproc(exp, imports[20]);
}


int con_init_console_dll(void)
{
    return con_init_console_dll_param(-1, -1, -1, -1, con_caption); 
}


int con_init_console_dll_param(dword wnd_width, dword wnd_height,
	dword scr_width, dword scr_height, const char* title)
/*	work as con_init_console_dll, but call con_init with params
*/
{
	struct import * hDll;

  	if (__console_initdll_status == 1) return 0;
    
	if((hDll = (struct import *)_ksys_cofflib_load(con_dllname)) == 0){
                debug_out_str("can't load lib\n");
                return 1;
    	}
    	con_lib_link(hDll, con_imports);

    	con_init(wnd_width, wnd_height, scr_width, scr_height, title); 

    	__console_initdll_status = 1;

    	return 0;
}

