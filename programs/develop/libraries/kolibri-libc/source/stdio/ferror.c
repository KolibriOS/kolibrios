#include <stdio.h>

int ferror(FILE *stream) {
	return stream->error;
}
