#include<libc/asm.h>
MK_C_SYM(log1pf)
	fldln2
	flds 4(%esp)
	fld1
	faddp
	fyl2x
	ret
