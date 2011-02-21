#include<libc/asm.h>
MK_C_SYM(rint)
	fldl	4(%esp)
	frndint
	ret
