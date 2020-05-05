// -------------------------------------------------------------
// KWINE is a fork of program PELoad written by 0CodErr
// author of fork - rgimad
//-------------------------------------------------------------
#include "stddef.h"
#include <stdarg.h>
#include "msvcrt.dll.h"

#include "string.c"
//#include "dlfcn.c"
#include "conio.c"
#include "stdio.c"
#include "stdlib.c"


typedef struct
{
	char *name;
	void *f;
} export_t;

// conio
const char sz__getch[] = "_getch";
const char sz__kbhit[] = "_kbhit";

// stdio
const char sz_printf[] = "printf";
const char sz_puts[] = "puts";
const char sz_gets[] = "gets";

//string
const char sz_strlen[] = "strlen";
const char sz_strcmp[] = "strcmp";
const char sz_strcat[] = "strcat";

// stdlib
const char sz_malloc[] = "malloc";
const char sz_free[] = "free";
const char sz_realloc[] = "realloc";
//const char sz_[] = "";

 
//uint32_t EXPORTS[] __asm__("EXPORTS") =
export_t EXPORTS[] = 
{
	{sz__getch, (void*)_getch},
	{sz__kbhit, (void*)_kbhit},

	{sz_printf, (void*)printf},
	{sz_puts, (void*)puts},
	{sz_gets, (void*)gets},

	{sz_strlen, (void*)strlen},
	{sz_strcmp, (void*)strcmp},
	{sz_strcat, (void*)strcat},

	{sz_malloc, (void*)malloc},
	{sz_free, (void*)free},
	{sz_realloc, (void*)realloc},
	{NULL, NULL},
};


int lib_init()
{
	con_init_console_dll();
}
