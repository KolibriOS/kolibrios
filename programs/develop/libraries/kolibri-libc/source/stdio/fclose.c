#include <stdio.h>
#include <stdlib.h>

int fclose(FILE *stream) {
	free(stream);
	return 0;
}
