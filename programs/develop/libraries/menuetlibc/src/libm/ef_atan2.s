#include<libc/asm.h>
MK_C_SYM(__ieee754_atan2f)
	flds	4(%esp)
	flds	8(%esp)
	fpatan
	ret
