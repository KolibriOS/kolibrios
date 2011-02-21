#include<libc/asm.h>

MK_C_SYM(__ieee754_remainderf)
	flds	8(%esp)
	flds	4(%esp)
1:	fprem1
	fstsw	%ax
	sahf
	jp	1b
	fstpl	%st(1)
	ret
