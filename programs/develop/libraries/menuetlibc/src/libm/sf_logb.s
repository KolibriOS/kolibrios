#include<libc/asm.h>
MK_C_SYM(logbf)
	flds	4(%esp)
	fxtract
	fstpl	%st
	ret
