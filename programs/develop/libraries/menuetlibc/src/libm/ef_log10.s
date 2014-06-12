#include<libc/asm.h>
MK_C_SYM(__ieee754_log10f)
.globl __ieee754_log10f
	fldlg2
	flds	4(%esp)
	fyl2x
	ret
