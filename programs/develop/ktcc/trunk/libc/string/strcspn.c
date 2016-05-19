#include <string.h>

size_t strcspn(const char* string, const char* strCharSet)
{
	const char* temp;
	int i;
	i=0;
	while(*string)
	{
		temp=strCharSet;
		while (*temp!='\0')
		{
			if (*string==*temp)
				return i;
			temp++;
		}
		i++;string++;
	}
	return i;
}
