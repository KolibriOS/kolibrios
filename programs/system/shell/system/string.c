
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
	*(char *)(dst+i) = *(char *)(src+i);

return NULL;
}


int memcmp(const void* buf1, const void* buf2, int count)
{
int i;
for (i=0;i<count;i++)
	{
	if (*(unsigned char*)buf1<*(unsigned char*)buf2)
		return -1;
	if (*(unsigned char*)buf1>*(unsigned char*)buf2)			
		return 1;
	}
return 0;
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


char* strrchr(const char* string, int c)
{
	int last_found;
	while (*string)
	{
		if (*string==c)
		{
			last_found = string;
		}
		string++;
	}	
	return (char*)last_found;
}



void _itoa(int i, char *s)
{
int a, b, c, d;
a = (i - i%1000)/1000;
b = (i - i%100)/100 - a*10;
c = (i - i%10)/10 - a*100 - b*10;
d = i%10;
s[0] = a + '0';
s[1] = b + '0';
s[2] = c + '0';
s[3] = d + '0';
s[4] = 0;
}


 /* reverse:  переворачиваем строку s на месте */
 void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }


 /* itoa:  конвертируем n в символы в s */
 void itoa(int n, char s[])
 {
     int i, sign;
 
     if ((sign = n) < 0)
         n = -n;
     i = 0;
     do {
         s[i++] = n % 10 + '0';
     } while ((n /= 10) > 0);
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }



int _atoi ( char *s )
{
int i, n;

n = 0;
for ( i = 0; s[i]!= '\0'; ++i)
	if ((s[i]<'0') || (s[i]>'9'))
		return 0;
	else
		n = 10 * n + s[i] - '0';

return n;
}
