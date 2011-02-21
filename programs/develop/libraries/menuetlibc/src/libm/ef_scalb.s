#include<libc/asm.h>
MK_C_SYM(__ieee754_scalbf)
	flds	8(%esp)
	flds	4(%esp)
	fscale
	ret
