#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void * memcpy(void * _dest, const void *_src, size_t _n)
{
int d0, d1, d2;
 __asm__ __volatile__(
        "jcxz 1f\n\t"
	"rep ; movsl\n\t"
	"1:\t"
	"testb $2,%b4\n\t"
	"je 1f\n\t"
	"movsw\n"
	"1:\ttestb $1,%b4\n\t"
	"je 2f\n\t"
	"movsb\n"
	"2:"
	: "=&c" (d0), "=&D" (d1), "=&S" (d2)
	:"0" (_n/4), "q" (_n),"1" ((long)_dest),"2" ((long)_src)
	: "memory");
 return (_dest);
}
