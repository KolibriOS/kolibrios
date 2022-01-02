#include <string.h>
#include <stdlib.h>
//#include <stdint.h>
#include "test.h"

static char buf[512];

static void *(*volatile pmemset)(void *, int, size_t);

static void *aligned(void *p)
{
	return (void*)(((uintptr_t)p + 63) & -64U);
}

#define N 80
static void test_align(int align, int len)
{
	char *s = aligned(buf);
	char *want = aligned(buf + 256);
	char *p;
	int i;

	if (align + len > N)
		abort();
	for (i = 0; i < N; i++)
		s[i] = want[i] = ' ';
	for (i = 0; i < len; i++)
		want[align+i] = '#';
	p = pmemset(s+align, '#', len);
	if (p != s+align)
		t_error("memset(%p,...) returned %p\n", s+align, p);
	for (i = 0; i < N; i++)
		if (s[i] != want[i]) {
			t_error("memset(align %d, '#', %d) failed\n", align, len);
			t_printf("got : %.*s\n", align+len+1, s);
			t_printf("want: %.*s\n", align+len+1, want);
			break;
		}
}

static void test_value(int c)
{
	int i;

	pmemset(buf, c, 10);
	for (i = 0; i < 10; i++)
		if ((unsigned char)buf[i] != (unsigned char)c) {
			t_error("memset(%d) failed: got %d\n", c, buf[i]);
			break;
		}
}

int main(void)
{
	int i,j,k;

	pmemset = memset;

	for (i = 0; i < 16; i++)
		for (j = 0; j < 64; j++)
			test_align(i,j);

	test_value('c');
	test_value(0);
	test_value(-1);
	test_value(-5);
	test_value(0xab);

	printf("%s finished\n", __FILE__);
	return t_status;
}
