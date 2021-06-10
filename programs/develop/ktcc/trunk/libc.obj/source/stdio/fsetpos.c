#include <stdio.h>

int fsetpos(FILE *stream, const fpos_t *pos) {
	stream->position = *pos;
	stream->eof = 0;
	return 0;
}
