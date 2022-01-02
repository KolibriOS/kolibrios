#include <stdio.h>
int fputc(int c,FILE* file)
{
	dword res;
	
	res = fwrite(&c, 1, 1, file);
	if (res < 1) return EOF;
	
	return c;
}
