/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>
	
	.text

#include<libc/asm.h>
MK_C_SYM(status87)

	fstsw	%ax
	movzwl	%ax, %eax
	ret

