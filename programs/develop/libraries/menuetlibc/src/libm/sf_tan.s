#include<libc/asm.h>
MK_C_SYM(tanf)
	flds	4(%esp)
	fptan
	fstp	%st(0)
	ret
