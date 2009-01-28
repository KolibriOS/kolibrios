
#include "string.h"

void*  memset(void *mem, int c, unsigned size)
{
unsigned i;

for ( i = 0; i < size; i++ )
	 *((char *)mem+i) = (char) c;

return NULL;	
}


void* memcpy(void *dst, const void *src, unsigned size)
{

unsigned i;

for ( i = 0; i < size; i++)
	*((char *)dst+i) = *((char *)src+i);

return NULL;
}


void strcat(char strDest[], char strSource[])
{

int i, j;
 
i = j = 0;
while (strDest[i] != '\0')
	i++;

while ((strDest[i++] = strSource[j++]) != '\0')
             ;
}


int strcmp(const char* string1, const char* string2)
{

while (1)
{
if (*string1<*string2)
	return -1;
if (*string1>*string2)
	return 1;

if (*string1=='\0')
	return 0;

string1++;
string2++;
}

}


void strcpy(char strDest[], const char strSource[])
{
unsigned i;

i = 0;
while ((strDest[i] = strSource[i]) != '\0')
	i++;

}


char* strncpy(char *strDest, const char *strSource, unsigned n)
{
unsigned i;

if (! n )
	return strDest;

i = 0;
while ((strDest[i] = strSource[i]) != '\0')
	if ( (n-1) == i )
		break;
	else
		i++;

return strDest;
}


int strlen(const char* string)
{
int i;

i=0;
while (*string++) i++;
return i;
}



char* strchr(const char* string, int c)
{
	while (*string)
	{
		if (*string==c)
			return (char*)string;
		string++;
	}	
	return (char*)0;
}

