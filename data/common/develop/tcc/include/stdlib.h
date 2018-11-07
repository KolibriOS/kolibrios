#ifndef stdlib_h
#define stdlib_h
#include "kolibrisys.h"

#define	RAND_MAX	65535
#ifndef NULL
# define NULL ((void*)0)
#endif

#define abs(i) (((i)<0)?(-(i)):(i))
#define labs(li) abs(li)

#define min(a, b) ((a)<(b) ? (a) : (b))
#define max(a, b) ((a)>(b) ? (a) : (b))


extern int atoib(char *s,int b);
extern int atoi(char *s);
extern char *itoab(unsigned int n,char* s,int  b);
extern char *__itoa(int n,char* s);

// function using KOS syscalls
extern void* stdcall sysmalloc(dword size);
extern void  stdcall sysfree(void *pointer);
extern void* stdcall sysrealloc(void* pointer,dword size);
extern void* syscalloc (size_t num, size_t size);

// suballocator functions
extern void* wtmalloc(size_t size);
extern void  wtfree(void *pointer);
extern void* wtrealloc(void* pointer, size_t size);
extern void* wtcalloc (size_t num, size_t size);
extern int   wtmalloc_freelist_check();
extern int   wtmalloc_poiner_check(void *ptr);
extern void  wtmalloc_freelist_print();

#ifdef USESYSALLOC
#define malloc(x) sysmalloc(x)
#define free(x)   sysfree(x)
#define realloc(x,y) sysrealloc(x,y)
#define calloc(x,y) syscalloc(x,y)
#else
#define malloc(x) wtmalloc(x)
#define free(x)   wtfree(x)
#define realloc(x,y) wtrealloc(x,y)
#define calloc(x,y) wtcalloc(x,y)
#endif


extern int rand (void);
extern void srand (unsigned int seed);

double strtod (const char* str, char** endptr);
long double strtold (const char* str, char** endptr);
float strtof (const char* str, char** endptr);
long int strtol (const char* str, char** endptr, int base);
#define strtoul(s, ep, b) ((unsigned long int)strtol(s, ep, b))


void exit (int status); /* close console if was initialized, also stay window [finished] when status is error < 0 */ 
#define abort() exit(-1)

typedef struct {
  int quot;
  int rem;
} div_t;

typedef div_t ldiv_t;

div_t div (int numer, int denom);
#define ldiv(a, b) div(a, b)
#define atol(a) atoi(a)
#define atof(a) strtod(a, NULL)



#endif
