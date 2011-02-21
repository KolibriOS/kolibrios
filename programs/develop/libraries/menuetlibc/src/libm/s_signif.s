#include<libc/asm.h>
MK_C_SYM(significand)
	fldl	4(%esp)
	fxtract
	fstpl	%st(1)
	ret
