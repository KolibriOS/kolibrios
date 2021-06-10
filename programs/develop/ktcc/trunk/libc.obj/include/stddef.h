#ifndef _STDDEF_H_
#define _STDDEF_H_

typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ssize_t;
typedef __WCHAR_TYPE__ wchar_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __PTRDIFF_TYPE__ intptr_t;
typedef __SIZE_TYPE__ uintptr_t;

#ifndef __int8_t_defined
#define __int8_t_defined
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
typedef int64_t intmax_t;
typedef uint64_t uintmax_t;
#endif

#ifndef NULL
    #define NULL ((void*)0)
#endif

#ifdef _BUILD_LIBC
    #define _FUNC(func) func
#else
    #define _FUNC(func) (*func)
#endif

#define offsetof(type, field) ((size_t)&((type *)0)->field)

#endif /* _STDDEF_H_ */