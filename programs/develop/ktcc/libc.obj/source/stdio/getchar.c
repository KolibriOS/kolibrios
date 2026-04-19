#include <stdio.h>
#include <conio.h>

int getchar(void) {
	con_init();
	char c = 0;
	con_gets(&c, 2);
	if (c == 0) {
		c = EOF;
	}
	return c;
}
