#include <stdio.h>
#include "../sys/_conio.h"

int getchar(void) {

	char c = 0;
	console_gets(&c, 2);
	if (c == 0) {
		c = EOF;
	}
	return c;
}
