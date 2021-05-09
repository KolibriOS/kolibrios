#ifndef _STDINT_H_
#define _STDINT_H_

#include <stddef.h>

#define INT8_MIN        (-128)
#define INT8_MAX         (127)
#define UINT8_MAX        (255)

#define INT16_MIN       (-32768)
#define INT16_MAX        (32767)
#define UINT16_MAX       (65535)

#define INT32_MIN        (-2147483647L-1)
#define INT32_MAX        (2147483647L)
#define UINT32_MAX       (4294967295UL)

#if __have_long64
#define INT64_MIN       (-9223372036854775807L-1L)
#define INT64_MAX        (9223372036854775807L)
#define UINT64_MAX      (18446744073709551615U)
#elif __have_longlong64
#define INT64_MIN       (-9223372036854775807LL-1LL)
#define INT64_MAX        (9223372036854775807LL)
#define UINT64_MAX      (18446744073709551615ULL)
#else
#define INT64_MAX       0x7fffffffffffffffLL
#define INT64_MIN       (-INT64_MAX - 1LL)
#define UINT64_MAX      (__CONCAT(INT64_MAX, U) * 2ULL + 1ULL) 
#endif

#endif /* _STDINT_H_*/
