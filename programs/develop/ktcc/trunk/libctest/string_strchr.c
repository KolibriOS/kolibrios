#include <string.h>
#include <stdlib.h>
#include "test.h"

#define N(s, c) { \
	char *p = s; \
	char *q = strchr(p, c); \
	if (q) \
		t_error("strchr(%s,%s) returned str+%d, wanted 0\n", #s, #c, q-p); \
}

#define T(s, c, n) { \
	char *p = s; \
	char *q = strchr(p, c); \
	if (q == 0) \
		t_error("strchr(%s,%s) returned 0, wanted str+%d\n", #s, #c, n); \
	else if (q - p != n) \
		t_error("strchr(%s,%s) returned str+%d, wanted str+%d\n", #s, #c, q-p, n); \
}

int main(void)
{
	int i;
	char a[128];
	char s[256];

	for (i = 0; i < 128; i++)
		a[i] = (i+1) & 127;
	for (i = 0; i < 256; i++)
		*((unsigned char*)s+i) = i+1;

	N("", 'a')
	N("a", 'b')
	N("abc abc", 'x')
	N(a, 128)
	N(a, 255)

	T("", 0, 0)
	T("a", 'a', 0)
	T("a", 'a'+256, 0)
	T("a", 0, 1)
	T("ab", 'b', 1)
	T("aab", 'b', 2)
	T("aaab", 'b', 3)
	T("aaaab", 'b', 4)
	T("aaaaab", 'b', 5)
	T("aaaaaab", 'b', 6)
	T("abc abc", 'c', 2)
	T(s, 1, 0)
	T(s, 2, 1)
	T(s, 10, 9)
	T(s, 11, 10)
	T(s, 127, 126)
	T(s, 128, 127)
	T(s, 255, 254)
	T(s, 0, 255)

	printf("%s finished\n", __FILE__);
	return t_status;
}
