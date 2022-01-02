#include <string.h>

char* strcpy(char* strDest,const char* strSource)
{
	char* res;
	res=strDest;
	while((*strDest++ = *strSource++)) ;
	return res;
}
