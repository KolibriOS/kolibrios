#include<libc/asm.h>
MK_C_SYM(sinf)
	flds	4(%esp)
	fsin
	ret
