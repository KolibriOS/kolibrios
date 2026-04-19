#ifndef _LIMITS_H_
#define _LIMITS_H_

#define CHAR_BIT   8
#define CHAR_MAX   127
#define CHAR_MIN   (-128)
#define INT_MAX    2147483647
#define INT_MIN    (-2147483647 - 1)
#define LONG_MAX   2147483647L
#define LONG_MIN   (-2147483647L - 1L)
#define MB_LEN_MAX 5
#define SCHAR_MAX  127
#define SCHAR_MIN  (-128)
#define SHRT_MAX   32767
#define SHRT_MIN   (-32768)
#define UCHAR_MAX  255
#define UINT_MAX   4294967295U
#define ULONG_MAX  4294967295UL
#define USHRT_MAX  65535
#define WCHAR_MIN  0
#define WCHAR_MAX  127
#define WINT_MIN   0
#define WINT_MAX   32767
#define SSIZE_MAX  2147483647

#define LLONG_MIN  (-9223372036854775807LL - 1LL)
#define LLONG_MAX  9223372036854775807LL
#define ULLONG_MAX 18446744073709551615ULL

/* gnuc ones */
#define LONG_LONG_MIN  LLONG_MIN
#define LONG_LONG_MAX  LLONG_MAX
#define ULONG_LONG_MAX ULLONG_MAX

#ifndef ARG_MAX
#define ARG_MAX 4096
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef STDIO_MAX_MEM
#define STDIO_MAX_MEM 4096
#endif

#endif /* _LIMITS_H_ */
