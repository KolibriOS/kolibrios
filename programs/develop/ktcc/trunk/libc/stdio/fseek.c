#include <stdio.h>
int fseek(FILE* file,long offset,int origin)
{
    fpos_t pos;
    if(!file)
    {
        errno = E_INVALIDPTR;
        return errno;
    }

	if (origin==SEEK_CUR)
		offset+=file->filepos;
	else if (origin==SEEK_END)
		offset+=file->filesize;
	else if (origin!=SEEK_SET)
		return EOF;
	pos = offset;
	return fsetpos(file, &pos);
}
