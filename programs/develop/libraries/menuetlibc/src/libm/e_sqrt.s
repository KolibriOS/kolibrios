#include<libc/asm.h>

MK_C_SYM(__ieee754_sqrt)
	fldl	4(%esp)
	fsqrt
	ret
