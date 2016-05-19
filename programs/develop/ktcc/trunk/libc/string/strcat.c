#include <string.h>

char* strcat(char* strDest, const char* strSource)
{
	char* res;
	res=strDest;
	while (*strDest) strDest++;
	while ((*strDest++ = *strSource++)) ;
	return res;
}
