#include <stdio.h>
int fflush(FILE* file)
{
	if ((file->mode & 3)==FILE_OPEN_READ)
  		return 0;
  	return(EOF);
}