/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
	.data
LCW1:
	.word	0
LCW2:
	.word	0
LC0:
	.double	0d1.0e+00

	.text

MK_C_SYM(__pow10)
	fldl	4(%esp)
	fldl2t
	fmulp
	fstcw	LCW1
	fstcw	LCW2
	fwait
	andw	$0xf3ff,LCW2
	orw	$0x0400,LCW2
	fldcw	LCW2
	fldl	%st(0)
	frndint
	fldcw	LCW1
	fxch	%st(1)
	fsub	%st(1),%st
	f2xm1
	faddl	LC0
	fscale
	fstp	%st(1)
	ret
