#include <stdint.h>

#define 	INT8_MAX   0x7f
#define 	INT8_MIN   (-INT8_MAX - 1)
#define 	UINT8_MAX   (INT8_MAX * 2 + 1)
#define 	INT16_MAX   0x7fff
#define 	INT16_MIN   (-INT16_MAX - 1)
#define 	UINT16_MAX   (__CONCAT(INT16_MAX, U) * 2U + 1U)
#define 	INT32_MAX   0x7fffffffL
#define 	INT32_MIN   (-INT32_MAX - 1L)
#define 	UINT32_MAX   (__CONCAT(INT32_MAX, U) * 2UL + 1UL)
#define 	INT64_MAX   0x7fffffffffffffffLL
#define 	INT64_MIN   (-INT64_MAX - 1LL)
#define 	UINT64_MAX   (__CONCAT(INT64_MAX, U) * 2ULL + 1ULL) 
