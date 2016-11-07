/*
 * Console example in C--
*/

#define MEMSIZE 4096*10

#include "../lib/obj/console.h"

void main()
{
	load_dll(libConsole, #con_init, 0);
	con_init stdcall (-1, -1, -1, -1, "Hello");
	con_set_flags stdcall (0x1F);
	con_write_string stdcall ("Console test", 12);
	con_exit stdcall (0);
	ExitProcess();
}

