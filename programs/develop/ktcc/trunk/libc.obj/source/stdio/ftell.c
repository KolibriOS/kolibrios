#include <stdio.h>

long int ftell(FILE *stream) {
	return stream->position;
}
