#include<libc/asm.h>
MK_C_SYM(significandf)
	flds	4(%esp)
	fxtract
	fstpl	%st(1)
	ret
