#include <stdlib.h>
#include <string.h>

char* strdup(const char* str)
{
	char* res;
	int len;
	len=strlen(str)+1;
	res=malloc(len);
	memcpy(res,str,len);
	return res;
}
