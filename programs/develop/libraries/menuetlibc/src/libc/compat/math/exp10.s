/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
	.file "exp10.s"
MK_C_SYM(exp10)
	jmp C_SYM(__pow10)

