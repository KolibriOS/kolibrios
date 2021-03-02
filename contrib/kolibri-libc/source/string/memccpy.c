#include <string.h>

void *memccpy(void *restrict dest, const void *restrict src, int c, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	c = (unsigned char)c;
	for (; n && (*d=*s)!=c; n--, s++, d++);

tail:
	if (n) return d+1;
	return 0;
}
