#include <string.h>
#include <libc/unconst.h>

char * strchr(const char *s, int c)
{
 int d0;
 register char * __res;
 __asm__ __volatile__(
	"movb %%al,%%ah\n"
	"1:\tlodsb\n\t"
	"cmpb %%ah,%%al\n\t"
	"je 2f\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"movl $1,%1\n"
	"2:\tmovl %1,%0\n\t"
	"decl %0"
	:"=a" (__res), "=&S" (d0) : "1" (s),"0" (c));
 return __res;
}
