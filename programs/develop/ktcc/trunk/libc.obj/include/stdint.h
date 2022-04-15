#ifndef _STDINT_H_
#define _STDINT_H_

#include <stddef.h>

#define INT8_MIN  (-128)
#define INT8_MAX  (127)
#define UINT8_MAX (255)

#define INT16_MIN  (-32768)
#define INT16_MAX  (32767)
#define UINT16_MAX (65535)

#define INT32_MIN  (-2147483647L - 1)
#define INT32_MAX  (2147483647L)
#define UINT32_MAX (4294967295UL)

#define INT64_MAX  0x7fffffffffffffffLL
#define INT64_MIN  (-INT64_MAX - 1LL)
#define UINT64_MAX (__CONCAT(INT64_MAX, U) * 2ULL + 1ULL)

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

#define SIZE_MAX UINT32_MAX

#endif /* _STDINT_H_*/
