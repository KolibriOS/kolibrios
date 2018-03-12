#include <stdio.h>
int fgetc(FILE* file)
{
	int c = 0, rc;
	
	rc = fread(&c, 1, 1, file);

	if (rc < 1) return EOF;

	return c;
}
