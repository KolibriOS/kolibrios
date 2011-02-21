#include<libc/asm.h>

MK_C_SYM(__ieee754_atan2)
	fldl	 4(%esp)
	fldl	12(%esp)
	fpatan
	ret
