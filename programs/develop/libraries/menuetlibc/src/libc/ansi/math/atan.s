/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
MK_C_SYM(atan)
	fldl	4(%esp)
	fld1
	fpatan
	ret

