/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

	.data
pow2.LCW1:
	.word	0
pow2.LCW2:
	.word	0
pow2.LC0:
	.double	0d1.0e+00

	.text

.global pow2;

pow2:
	fldl	4(%esp)
	fstcw	pow2.LCW1
	fstcw	pow2.LCW2
	fwait
	andw	$0xf3ff,pow2.LCW2
	orw	$0x0400,pow2.LCW2
	fldcw	pow2.LCW2
	fldl	%st(0)
	frndint
	fldcw	pow2.LCW1
	fxch	%st(1)
	fsub	%st(1),%st
	f2xm1
	faddl	pow2.LC0
	fscale
	fstp	%st(1)
	ret
