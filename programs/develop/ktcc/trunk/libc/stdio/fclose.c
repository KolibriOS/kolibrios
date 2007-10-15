#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void fclose(FILE* file)
{
	free(file->buffer);
	free(file);
}