#include "string.h"
char* strtok(char* s,const char* delim)
{
	char* res;
	if (*s=='\0')
		return (char*)0;
	s+=strspn(s,delim);
	if (*s=='\0')
		return (char*)0;
	res=s;
	s+=strcspn(s,delim);
	*s=='\0';
	return res;
}
