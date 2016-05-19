#include <string.h>

int strncmp(const char* string1, const char* string2, size_t count)
{
	while(count>0 && (*string1==*string2))
	{
		if ('\0' == *string1) return 0;
		++string1;
		++string2;
		--count;
	}
	if(count) return (*string1 - *string2);
	return 0;
}
