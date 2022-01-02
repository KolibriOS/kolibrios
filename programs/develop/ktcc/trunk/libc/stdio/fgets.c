#include <stdio.h>

char * fgets ( char * str, int num, FILE * file )
// need to ignore \r\n in text mode
{
	int rd = 0;
	char c;

    if(!file || !str)
    {
        errno = E_INVALIDPTR;
        return NULL;
    }


	while (rd < num - 1)
	{
		c = fgetc(file);
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
