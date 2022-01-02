#include <string.h>

char* strtok(char* s,const char* delim)
// non reentrant
{
    static char* savep;
    char* res;

    if(s)
        savep = NULL;
    else
        s = savep;

    if (*s == '\0')
        return NULL;
	s += strspn(s, delim);
	if (*s == '\0')
		return NULL;
	res = s;
	s += strcspn(s, delim);
	savep = s + 1;
	*s = '\0';
	return res;
}

