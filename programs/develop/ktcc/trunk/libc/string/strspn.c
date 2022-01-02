#include <string.h>

size_t strspn(const char* string,const char* strCharSet)
{
	int i;
	const char* temp;
	i=0;
	while (*string!='\0')
	{
		temp=strCharSet;
		while (*temp!='\0')
		{
			if (*temp==*string)
				break;
            temp++;
		}
		if (*temp=='\0')
			break;
		string++;
		i++;
	}
	return i;
}
