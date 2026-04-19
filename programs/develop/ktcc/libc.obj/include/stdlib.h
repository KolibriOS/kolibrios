#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <stddef.h>

#define RAND_MAX 65535
#ifndef NULL
#define NULL ((void*)0)
#endif

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

static inline div_t div(int num, int den)
{
    return (div_t) { num / den, num % den };
}

static inline ldiv_t ldiv(long num, long den)
{
    return (ldiv_t) { num / den, num % den };
}

static inline lldiv_t lldiv(long long num, long long den)
{
    return (lldiv_t) { num / den, num % den };
}

DLLAPI void* malloc(size_t size);
DLLAPI void* calloc(size_t num, size_t size);
DLLAPI void* realloc(void* ptr, size_t newsize);
DLLAPI void free(void* ptr);

DLLAPI long int strtol(const char* str, char** endptr, int base);

DLLAPI void exit(int status);

DLLAPI void srand(unsigned s);
DLLAPI int rand(void);

DLLAPI void __assert_fail(const char* expr, const char* file, int line, const char* func);
DLLAPI void qsort(void* base0, size_t n, size_t size, int (*compar)(const void*, const void*));

DLLAPI double strtod(const char* s, char** sret);
DLLAPI double atof(const char* ascii);

DLLAPI int atoi(const char* s);
DLLAPI long atol(const char*);
DLLAPI long long atoll(const char*);
DLLAPI void itoa(int n, char* s);

DLLAPI int abs(int);
DLLAPI long labs(long);
DLLAPI long long llabs(long long);

#endif
