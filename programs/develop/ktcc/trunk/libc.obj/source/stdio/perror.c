#include <stdio.h>

void perror(const char *s) {
	debug_printf("%s: It's some error, maybe...\n", s);
}
