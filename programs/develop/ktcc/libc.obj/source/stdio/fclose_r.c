#include "_stdio.h"

int fclose_r(FILE* stream)
{
	if (stream->name) {
		free(stream->name);
	}

	return 0;
}