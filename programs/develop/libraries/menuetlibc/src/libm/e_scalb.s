#include<libc/asm.h>
MK_C_SYM(__ieee754_scalb)
	fldl	12(%esp)
	fldl	4(%esp)
	fscale
	ret
