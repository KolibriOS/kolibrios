#ifndef __LIBC_ASM_H
#define __LIBC_ASM_H

#include __DEV_CONFIG_H

#ifndef NEEDS_UNDERSCORES

#define C_SYM(x)	x
#else

#define C_SYM(x)	_##x

#endif

#define MK_C_SYM(x)	C_SYM(x) : ; \
			.globl C_SYM(x)

#endif
