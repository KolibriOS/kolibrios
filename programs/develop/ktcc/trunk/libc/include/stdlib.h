#ifndef stdlib_h
#define stdlib_h
#include "kolibrisys.h"

#define	RAND_MAX	65535
#ifndef NULL
# define NULL ((void*)0)
#endif

#define abs(i) (((i)<0)?(-(i)):(i))
#define labs(li) abs(li)

extern int atoib(char *s,int b);
extern int atoi(char *s);
extern char *itoab(int n,char* s,int  b);
extern char *itoa(int n,char* s);

extern void* stdcall malloc(dword size);
extern void  stdcall free(void *pointer);
extern void* stdcall realloc(void* pointer,dword size);

extern int rand (void);
extern void srand (unsigned int seed);

double strtod (const char* str, char** endptr);
long double strtold (const char* str, char** endptr);
float strtof (const char* str, char** endptr);

void* calloc (size_t num, size_t size);

#define exit(a) _ksys_exit()
#define abort() exit(-1)

typedef struct {
  int quot;
  int rem;
} div_t;

typedef div_t ldiv_t;

div_t div (int numer, int denom);
#define ldiv(a, b) div(a, b)
#define atol(a) atoi(a)

#endif
