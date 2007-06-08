
// Console dynamic link library. Sample by Ghost

#include "stdio.h" 
#include "string.h" 
#include "mesys.h" 

char* dllname="/sys/console.obj";
int i;

char* imports[] = {"START","version","con_init","con_write_asciiz","con_printf","con_exit",NULL};
char* caption = "Console test - colors";

dword (* dll_start)(dword res);
dword* dll_ver;
void stdcall (* con_init)(dword wnd_width, dword wnd_height, dword scr_width, dword scr_height, const char* title);
void stdcall (* con_write_asciiz)(dword flags, const char* string);
void cdecl (* con_printf)(dword flags, const char* format, ...);
void stdcall (* con_exit)(dword bCloseWindow);

struct import{
	char *name;
	void *data;
};

// в библиотеке есть аналогичная функция _msys_cofflib_getproc, но пока она работает не корректно
void* __msys_cofflib_getproc(struct import *lib, char *name){
	int i;
	for(i = 0; lib[i].name && strcmp(name, lib[i].name); i++);
	if(lib[i].name)return lib[i].data;
	else return NULL;
}

void link(struct import *exp, char** imports){
	dll_start = (dword (*)(dword))
		__msys_cofflib_getproc(exp, imports[0]);
	dll_ver = (dword*)
		__msys_cofflib_getproc(exp, imports[1]);
	con_init = (void stdcall (*)(dword , dword, dword, dword, const char*))
		__msys_cofflib_getproc(exp, imports[2]);
	con_write_asciiz = (void stdcall (*)(dword, const char*))
		__msys_cofflib_getproc(exp, imports[3]);
	con_printf = (void cdecl (*)(dword, const char*, ...))
		__msys_cofflib_getproc(exp, imports[4]);
	con_exit = (void stdcall (*)(dword))
		__msys_cofflib_getproc(exp, imports[5]);
}

int main(int argc, char **argv){
	struct import * hDll;

	if((hDll = (struct import *)_msys_cofflib_load(dllname)) == 0){
		debug_out_str("can't load lib\n\r");
		return 1;
	}
	link(hDll, imports);
        debug_out_str("dll loaded\n\r");

	if(dll_start(1) == 0){
		debug_out_str("dll_start failed\n\r");
		return 1;
        }

	con_init(-1, -1, -1, -1, caption);
	for(i = 0; i < 256; i++){
		con_printf(7, "Color 0x%02X: ", i);
		con_write_asciiz(i, "Text sample.");

		con_printf(i, "  printf %s test %d\n", "small", i);
        }

	con_exit(0);
	debug_out_str("all right's ;)\n\r");
}