#include <stdio.h>
int fsetpos(FILE* file,const fpos_t * pos)
{
	if (*pos>=0)
	{
  		file->filepos=*pos;
		return 0;
	}
	else
		return EOF;
}