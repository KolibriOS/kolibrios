#include<libc/asm.h>

MK_C_SYM(__ieee754_acos)
	fldl	4(%esp)			
	fst	%st(1)
	fmul	%st(0)			
	fld1				
	fsubp				
	fsqrt				
	fxch	%st(1)
	fpatan
	ret
