#ifndef __LIBC_ASM_H
#define __LIBC_ASM_H

#define C_SYM(x) x
// #define C_SYM(x)	_##x

#define MK_C_SYM(x)	.global C_SYM(x); C_SYM(x):

#endif
