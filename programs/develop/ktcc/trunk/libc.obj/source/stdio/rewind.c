#include <stdio.h>

void rewind(FILE *stream) {
	stream->position = 0;
}
