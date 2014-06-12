/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>
#include<libc/asm.h>

MK_C_SYM(htons)

	movl	4(%esp), %eax
	xchgb	%ah, %al
	ret
