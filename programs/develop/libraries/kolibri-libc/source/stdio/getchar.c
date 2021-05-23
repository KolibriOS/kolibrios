#include <stdio.h>
#include "conio.h"

int getchar(void) {
	__con_init();
	char c = 0;
	__con_gets(&c, 2);
	if (c == 0) {
		c = EOF;
	}
	return c;
}
