#include<libc/asm.h>
MK_C_SYM(scalbnf)
	fildl	8(%esp)
	flds	4(%esp)
	fscale
	ret
