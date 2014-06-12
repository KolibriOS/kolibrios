#include<libc/asm.h>
MK_C_SYM(cosf)
 flds	4(%esp)
	fcos
	ret	
