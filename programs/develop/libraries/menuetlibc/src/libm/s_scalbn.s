#include<libc/asm.h>
MK_C_SYM(scalbn)
	fildl	12(%esp)
	fldl	4(%esp)
	fscale
	ret
