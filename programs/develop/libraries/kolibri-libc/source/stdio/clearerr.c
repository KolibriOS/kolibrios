#include <stdio.h>

void clearerr(FILE *stream) {
	stream->error = 0;
	stream->eof = 0;
}
