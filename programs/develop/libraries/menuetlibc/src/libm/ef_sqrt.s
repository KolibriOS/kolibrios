#include<libc/asm.h>
MK_C_SYM(__ieee754_sqrtf)
	flds	4(%esp)
	fsqrt
	ret
