// -------------------------------------------------------------
// KWINE is a fork of program PELoad written by 0CodErr
// author of fork - rgimad
//-------------------------------------------------------------
#include "stddef.h"
#include <stdarg.h>
#include "msvcrt.dll.h"

#include "string.h"
#include "conio.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "string.c"
#include "conio.c"
#include "stdio.c"
#include "stdlib.c"
#include "time.c"

// note: by default all function in c are cdecl :D

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
const char sz_putchar[] = "putchar";

//string
const char sz_strlen[] = "strlen";
const char sz_strcmp[] = "strcmp";
const char sz_strcat[] = "strcat";
const char sz_strchr[] = "strchr";
const char sz_strrchr[] = "strrchr";
const char sz_strcpy[] = "strcpy";
const char sz_strncpy[] = "strncpy";
const char sz_memset[] = "memset";
const char sz_memcpy[] = "memcpy";
const char sz_memcmp[] = "memcmp";

// stdlib
const char sz_srand[] = "srand";
const char sz_rand[] = "rand";
const char sz_malloc[] = "malloc";
const char sz_free[] = "free";
const char sz_realloc[] = "realloc";
//const char sz_[] = "";

// time
const char sz_time[] = "time";
const char sz_mktime[] = "mktime";
const char sz_localtime[] = "localtime";
const char sz_difftime[] = "difftime";

 
//uint32_t EXPORTS[] __asm__("EXPORTS") =
export_t EXPORTS[] = 
{
	{sz__getch, (void*)_getch},
	{sz__kbhit, (void*)_kbhit},

	{sz_printf, (void*)printf},
	{sz_puts, (void*)puts},
	{sz_gets, (void*)gets},
	{sz_putchar, (void*)putchar},

	{sz_strlen, (void*)strlen},
	{sz_strcmp, (void*)strcmp},
	{sz_strcat, (void*)strcat},
	{sz_strchr, (void*)strchr},
	{sz_strrchr, (void*)strrchr},
	{sz_strcpy, (void*)strcpy},
	{sz_strncpy, (void*)strncpy},
	{sz_memset, (void*)memset},
	{sz_memcpy, (void*)memcpy},
	{sz_memcmp, (void*)memcmp},

	{sz_srand, (void*)srand},
	{sz_rand, (void*)rand},
	{sz_malloc, (void*)malloc},
	{sz_free, (void*)free},
	{sz_realloc, (void*)realloc},

	{sz_time, (void*)time},
	{sz_mktime, (void*)mktime},
	{sz_localtime, (void*)localtime},
	{sz_difftime, (void*)difftime},


	{NULL, NULL},
};


int lib_init()
{
	con_init_console_dll();
}
