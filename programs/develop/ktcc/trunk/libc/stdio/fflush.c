#include <stdio.h>
int fflush(FILE* file)
// file can be zero, as flush all
{
	if (file && (file->mode & 3)==FILE_OPEN_READ)
  		return 0;

  	return(0);  // always good, as no write buffering
}
