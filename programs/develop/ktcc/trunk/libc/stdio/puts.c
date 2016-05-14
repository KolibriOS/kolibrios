#include <conio.h>

int puts ( const char * str )
{
	con_init_console_dll();
	
	con_write_asciiz(str);
	con_write_asciiz("\n");

	return 1;
}