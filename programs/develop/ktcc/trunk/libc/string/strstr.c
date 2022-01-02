#include <string.h>

char* strstr(const char* s, const char* find)
{
	int len;
	len=strlen(find);
	while (1)
	{
		if (strncmp(s,find,len)==0) return (char*)s;
		if (*s=='\0')
			return (char*) 0;
		s++;
	}
}
