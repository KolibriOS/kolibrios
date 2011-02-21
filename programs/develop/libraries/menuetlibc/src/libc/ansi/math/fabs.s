/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
MK_C_SYM(fabs)
	fldl	4(%esp)
	fabs
	ret
