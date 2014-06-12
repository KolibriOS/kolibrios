#include<libc/asm.h>

MK_C_SYM(__ieee754_asin)
	fldl	4(%esp)			
	fst	%st(1)
	fmul	%st(0)			
	fld1
	fsubp				
	fsqrt				
	fpatan
	ret
