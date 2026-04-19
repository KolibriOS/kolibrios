#include <stdio.h>

FILE *tmpfile(void) {
	char name[FILENAME_MAX + 1];

	if (!tmpnam(name)) {
		return NULL;
	}

	return fopen(name, "wb+");
}
