#include <stdio.h>

int fgetpos(FILE *restrict stream, fpos_t *restrict pos) {
	*pos = stream->position;
	return 0;
}
