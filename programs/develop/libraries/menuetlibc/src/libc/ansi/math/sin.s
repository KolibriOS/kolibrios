/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
L0:
	.quad	0xffffffffffffffff

MK_C_SYM(sin)
	fldl	4(%esp)
	fsin
	fstsw
	sahf
	jnp	L1
	fstp	%st(0)
	fldl	L0
L1:
	ret	

