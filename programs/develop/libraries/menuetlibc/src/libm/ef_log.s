#include<libc/asm.h>

MK_C_SYM(__ieee754_logf)
	fldln2
	flds	4(%esp)
	fyl2x
	ret
