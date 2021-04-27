#include <stdio.h>
#include "conio.h"

int getchar(void) {
	int c = __con_getch();
	if (c == 0) {
		c = EOF;
	}
	return c;
}
