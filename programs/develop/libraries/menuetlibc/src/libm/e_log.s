#include<libc/asm.h>

MK_C_SYM(__ieee754_log)
	fldln2
	fldl	4(%esp)
	fyl2x
	ret
