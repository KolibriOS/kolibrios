#include<libc/asm.h>
MK_C_SYM(atanf)
	flds	4(%esp)
	fld1
	fpatan
	ret
