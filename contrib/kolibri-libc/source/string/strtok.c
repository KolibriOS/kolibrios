/* strtok( char *, const char * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <string.h>

char* strtok(char* s, const char* delim)
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

