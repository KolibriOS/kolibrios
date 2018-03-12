#include <stdio.h>
int fsetpos(FILE* file,const fpos_t * pos)
{
    if(!file || !pos)
    {
        errno = E_INVALIDPTR;
        return errno;
    }

	if (*pos>=0)
	{
  		file->filepos=*pos;
		file->ungetc_buf = EOF;
		return 0;
	}
	else
		return EOF;
}
