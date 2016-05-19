#include <string.h>

char* strpbrk(const char* string, const char* strCharSet)
{
	const char* temp;
	while (*string!='\0')
	{
		temp=strCharSet;
		while (*temp!='\0')
		{
			if (*string==*temp)
				return (char*)string;
			temp++;
		}
		string++;
	}
	return (char*)0;
}
