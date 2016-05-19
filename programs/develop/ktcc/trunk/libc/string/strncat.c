#include <string.h>

char* strncat(char* strDest,const char* strSource,size_t count)
{
	char* res;
	res=strDest;
	while (*strDest) strDest++;
	while(count-- > 0)
	{
	    if((*strDest++ = *strSource++)) continue;
		return(res);
	}
	*strDest = 0;
	return res;
}
