#include <conio.h>

char * gets ( char * str )
{
	con_init_console_dll();

	return con_gets(str, 80); // small, to reduce overflow risk
}