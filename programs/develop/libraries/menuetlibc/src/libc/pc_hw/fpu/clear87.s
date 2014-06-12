/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>
#include<libc/asm.h>
	
MK_C_SYM(clear87)

	fstsw	%ax
	movzwl	%ax, %eax
	fclex
	ret
