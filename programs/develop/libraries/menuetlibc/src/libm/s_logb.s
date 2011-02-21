#include<libc/asm.h>
MK_C_SYM(logb)
	fldl	4(%esp)
	fxtract
	fstpl	%st
	ret
