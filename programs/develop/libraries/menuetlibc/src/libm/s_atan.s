#include<libc/asm.h>
MK_C_SYM(atan)
	fldl	4(%esp)
	fld1
	fpatan
	ret
