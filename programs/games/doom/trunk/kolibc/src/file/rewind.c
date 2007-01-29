#include "kolibc.h"

int fill_buff(FILE* f);

void rewind(FILE* file)
{
	file->filepos=0;
	fill_buff(file);
}