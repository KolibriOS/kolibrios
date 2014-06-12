#include <string.h>

char * strcat(char *s, const char *append)
{
 int d0, d1, d2, d3;
 __asm__ __volatile__(
	"repne\n\t"
	"scasb\n\t"
	"decl %1\n"
	"1:\tlodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b"
	: "=&S" (d0), "=&D" (d1), "=&a" (d2), "=&c" (d3)
	: "0" (append),"1"(s),"2"(0),"3" (0xffffffff):"memory");
 return s;
}
