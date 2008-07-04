#include "common.h"

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

int
memcmp(const void *s1, const void *s2, size_t n)
{
  if (n != 0)
  {
    const unsigned char *p1 = s1, *p2 = s2;

    do {
      if (*p1++ != *p2++)
	return (*--p1 - *--p2);
    } while (--n != 0);
  }
  return 0;
}

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

char * strcpy(char *to, const char *from)
{
int d0, d1, d2;
__asm__ __volatile__(
	"1:\tlodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b"
	: "=&S" (d0), "=&D" (d1), "=&a" (d2)
	:"0" (from),"1" (to) : "memory");
 return to;
}

int strcmp(const char *s1, const char *s2)
{
int d0, d1;
register int __res;
__asm__ __volatile__(
	"1:\tlodsb\n\t"
	"scasb\n\t"
	"jne 2f\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"xorl %%eax,%%eax\n\t"
	"jmp 3f\n"
	"2:\tsbbl %%eax,%%eax\n\t"
	"orb $1,%%al\n"
	"3:"
	:"=a" (__res), "=&S" (d0), "=&D" (d1)
		     :"1" (s1),"2" (s2));
return __res;
}

size_t strlen(const char *str)
{
int d0;
register int __res;
__asm__ __volatile__(
	"repne\n\t"
	"scasb\n\t"
	"notl %0\n\t"
	"decl %0"
	:"=c" (__res), "=&D" (d0) :"1" (str),"a" (0), "0" (0xffffffff));
return __res;
}

char * strdup(const char *_s)
{
  char *rv;
  if (_s == 0)
    return 0;
  rv = (char *)kmalloc(strlen(_s) + 1);
  if (rv == 0)
    return 0;
  strcpy(rv, _s);
  return rv;
}

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
