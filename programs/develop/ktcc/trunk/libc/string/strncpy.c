#include <string.h>

char* strncpy(char* strDest,const char* strSource,size_t count)
{
	char* res;
	res=strDest;
	while (count>0)
	{
		*strDest=*strSource;
		if (*strSource!='\0')
			strSource++;
		strDest++;
		count--;
	}
	return res;
}
