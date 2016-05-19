#include <string.h>

char* strchr(const char* string, int c)
{
	do {
		if (*string == (char)c)
			return (char*)string;
	} while (*string++);

    return NULL;
}
