/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
MK_C_SYM(ffs)
	bsfl	4(%esp), %eax
	jnz	.Lzero
	movl	$0,%eax
.Lzero:
	ret
