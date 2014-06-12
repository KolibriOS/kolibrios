#include<libc/asm.h>
MK_C_SYM(log1p)
	fldln2
	fldl 4(%esp)
	fld1
	faddp
	fyl2x
	ret
