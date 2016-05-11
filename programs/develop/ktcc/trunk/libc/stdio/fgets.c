#include <stdio.h>

char * fgets ( char * str, int num, FILE * stream )
{
	int rd = 0;
	char c;
	while (rd < num - 1) 
	{
		c = fgetc(stream);
		if (EOF == c) break;
		if ('\n' == c)
		{
			str[rd++] = c;
			break;
		}
		else
			str[rd++] = c;
	}
	if (0 == rd) return NULL;
	else 
	{	
		str[rd] = '\0';
		return str;
	}
}
