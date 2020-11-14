// Console.obj loading for kos32-gcc
// Writed by rgimad and maxcodehack

#include <string.h>
#include <stdlib.h>

#ifndef CONSOLE_OBJ_H
#define CONSOLE_OBJ_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef cdecl
#define cdecl   __attribute__ ((cdecl))
#endif

#ifndef stdcall
#define stdcall __attribute__ ((stdcall))
#endif

typedef unsigned int dword;
typedef unsigned short word;

const char* imports[] = {
        "START", "version", "con_init", "con_write_asciiz", "con_write_string",
        "con_printf", "con_exit", "con_get_flags", "con_set_flags", "con_kbhit",
        "con_getch", "con_getch2", "con_gets", "con_gets2", "con_get_font_height",
        "con_get_cursor_height", "con_set_cursor_height",  "con_cls",
        "con_get_cursor_pos", "con_set_cursor_pos", "con_set_title",
        (char*)0
};

dword *version;

typedef int (stdcall * con_gets2_callback)(int keycode, char** pstr, int* pn, 
	int* ppos);

void stdcall (*con_init)(dword wnd_width, dword wnd_height, dword scr_width, dword scr_height, const char* title) = 0;
void stdcall (*con_exit)(int bCloseWindow) = 0;
void stdcall (*con_set_title)(const char* title) = 0;
void stdcall (*con_write_asciiz)(const char* str) = 0;
void stdcall (*con_write_string)(const char* str, dword length) = 0;
int cdecl (*con_printf)(const char* format, ...) = 0;
dword stdcall (*con_get_flags)(void) = 0;
dword stdcall (*con_set_flags)(dword new_flags) = 0;
int stdcall (*con_get_font_height)(void) = 0;
int stdcall (*con_get_cursor_height)(void) = 0;
int stdcall (*con_set_cursor_height)(int new_height) = 0;
int stdcall (*con_getch)(void) = 0;
word stdcall (*con_getch2)(void) = 0;
int stdcall (*con_kbhit)(void) = 0;
char* stdcall (*con_gets)(char* str, int n) = 0;
char* stdcall (*con_gets2)(con_gets2_callback callback, char* str, int n) = 0;
void stdcall (*con_cls)() = 0;
void stdcall (*con_get_cursor_pos)(int* px, int* py) = 0;
void stdcall (*con_set_cursor_pos)(int x, int y) = 0;

const char lib_path[] = "/sys/lib/console.obj";

void* load_library(const char *name)
{
    void *table;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(table)
    :"a"(68), "b"(19), "c"(name));
    return table;
}

void *load_library_procedure(void *exports, const char *name)
{
	if (exports == NULL) { return 0; }
	while (*(dword*)exports != 0)
	{
		char *str1 = (char*)(*(dword*)exports);
		if (strcmp(str1, name) == 0)
		{
            void *ptr = (void*)*(dword*)(exports + 4);
			return ptr;
		}
		exports += 8;
	}
	return 0;
}

void output_debug_string(const char *s)
{
	unsigned int i = 0;
	while(*(s + i))
	{
		asm volatile ("int $0x40"::"a"(63), "b"(1), "c"(*(s + i)));
		i++;
	}
}

void load_console()
{
	void *lib = load_library(lib_path);

	if (!lib)
	{
		output_debug_string("Console.obj loading error\r\n");
		exit(1);
	}

    dword (*start_lib)(dword) = (dword(*)(dword))load_library_procedure(lib, imports[0]);

    version = (dword*)load_library_procedure(lib, imports[1]);

    con_init = (void stdcall(*)(dword,dword,dword,dword,const char*))load_library_procedure(lib, imports[2]);
    con_write_asciiz = (void stdcall(*)(const char*))load_library_procedure(lib, imports[3]);
    con_write_string = (void stdcall(*)(const char*,dword))load_library_procedure(lib, imports[4]);
    con_printf = (int cdecl(*)(const char*,...))load_library_procedure(lib, imports[5]);
    con_exit = (void stdcall(*)(int))load_library_procedure(lib, imports[6]);
    con_get_flags = (dword stdcall(*)(void))load_library_procedure(lib, imports[7]);
    con_set_flags = (dword stdcall(*)(dword))load_library_procedure(lib, imports[8]);
    con_kbhit = (int stdcall(*)(void))load_library_procedure(lib, imports[9]);
    con_getch = (int stdcall(*)(void))load_library_procedure(lib, imports[10]);
    con_getch2 = (word stdcall(*)(void))load_library_procedure(lib, imports[11]);
    con_gets = (char* stdcall(*)(char*,int))load_library_procedure(lib, imports[12]);
    con_gets2 = (char* stdcall(*)(con_gets2_callback,char*,int))load_library_procedure(lib, imports[13]);
    con_get_font_height	= (int stdcall(*)(void))load_library_procedure(lib, imports[14]);
    con_get_cursor_height = (int stdcall(*)(void))load_library_procedure(lib, imports[15]);
    con_set_cursor_height = (int stdcall(*)(int))load_library_procedure(lib, imports[16]);
    con_cls	= (void stdcall(*)(void))load_library_procedure(lib, imports[17]);
    con_get_cursor_pos = (void stdcall(*)(int*,int*))load_library_procedure(lib, imports[18]);
	con_set_cursor_pos = (void stdcall(*)(int,int))load_library_procedure(lib, imports[19]);
	con_set_title = (void stdcall(*)(const char*))load_library_procedure(lib, imports[20]);

}

#ifdef __cplusplus
}
#endif

#endif
