#ifndef	_ENDIAN_H
#define	_ENDIAN_H	1

#include <features.h>

#define	__LITTLE_ENDIAN	1234
#define	__BIG_ENDIAN	4321
#define	__PDP_ENDIAN	3412

#include <bits/endian.h>

#ifndef __FLOAT_WORD_ORDER
#define __FLOAT_WORD_ORDER __BYTE_ORDER
#endif

#ifdef	__USE_BSD
#define LITTLE_ENDIAN	__LITTLE_ENDIAN
#define BIG_ENDIAN	__BIG_ENDIAN
#define PDP_ENDIAN	__PDP_ENDIAN
#define BYTE_ORDER	__BYTE_ORDER
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define __LONG_LONG_PAIR(HI, LO) LO, HI
#elif __BYTE_ORDER == __BIG_ENDIAN
#define __LONG_LONG_PAIR(HI, LO) HI, LO
#endif

#endif
