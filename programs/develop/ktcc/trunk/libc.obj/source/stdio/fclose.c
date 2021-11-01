#include "stddef.h"
#include <stdio.h>
#include <stdlib.h>

int fclose(FILE *stream) {
	free(stream);
	stream = NULL;
	return 0;
}
