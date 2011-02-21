/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
MK_C_SYM(log2)
	fld1
	fldl	4(%esp)
	fyl2x
	ret
