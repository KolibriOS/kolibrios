#include "_stdio.h"
#include <stdio.h>
#include <stdlib.h>

int fclose(FILE* stream)
{
	int err = fclose_r(stream);

	if (err) {
		return err;
	}

	free(stream);
	stream = NULL;
	return 0;
}
