#include<libc/asm.h>
MK_C_SYM(__ieee754_log10)
	fldlg2
	fldl	4(%esp)
	fyl2x
	ret
