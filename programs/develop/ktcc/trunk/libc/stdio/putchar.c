#include <conio.h>

int putchar ( int ch )
{
	char s[2];

	con_init_console_dll();
	
	s[0] = (char)ch;
	s[1] = '\0';

	con_write_asciiz(s);
	return ch;
}