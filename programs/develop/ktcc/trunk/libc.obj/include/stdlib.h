#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <stddef.h>

#define RAND_MAX        65535
#ifndef NULL
# define NULL ((void*)0)
#endif
 
#define min(a, b) ((a)<(b) ? (a) : (b))
#define max(a, b) ((a)>(b) ? (a) : (b))

extern int  _FUNC(atoi)(const char *s);
extern long _FUNC(atol)(const char *);
extern long long _FUNC(atoll)(const char *);
extern void _FUNC(itoa)(int n, char* s);

extern int _FUNC(abs)(int);
extern long _FUNC(labs)(long);
extern long long _FUNC(llabs)(long long);

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
typedef struct { long long quot, rem; } lldiv_t;

extern div_t _FUNC(div)(int, int);
extern ldiv_t _FUNC(ldiv)(long, long);
extern lldiv_t _FUNC(lldiv)(long long, long long);

extern void* _FUNC(malloc)(size_t size);
extern void* _FUNC(calloc)(size_t num, size_t size);
extern void* _FUNC(realloc)(void *ptr, size_t newsize);
extern void  _FUNC(free)(void *ptr);

extern long int _FUNC(strtol)(const char* str, char** endptr, int base);

extern void  _FUNC(exit)(int status);

extern void _FUNC(srand)(unsigned s);
extern int  _FUNC(rand)(void);

extern void _FUNC(__assert_fail)(const char *expr, const char *file, int line, const char *func);
extern void _FUNC(qsort)(void *base0, size_t n, size_t size, int (*compar)(const void *, const void *));

extern double _FUNC(strtod)(const char *s, char **sret);
extern double _FUNC(atof)(const char *ascii);

#endif
