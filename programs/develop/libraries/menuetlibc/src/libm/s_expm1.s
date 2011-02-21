#include<libc/asm.h>

MK_C_SYM(expm1)
	fldl	4(%esp)
	fldl2e
	fmulp				
	fstl	%st(1)
	frndint				
	fstl	%st(2)
	fsubrp				
	f2xm1				 
	fld1
	faddp				
	fscale				
	fld1
	fsubrp				
	ret
