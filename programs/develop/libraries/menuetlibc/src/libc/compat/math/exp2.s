/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
MK_C_SYM(exp2)
	jmp C_SYM(__pow2)

