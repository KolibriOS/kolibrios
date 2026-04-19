/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

	.data
pow10.LCW1:
	.word	0
pow10.LCW2:
	.word	0
pow10.LC0:
	.double	0d1.0e+00

	.text

.global pow10;

pow10:
	fldl	4(%esp)
	fldl2t
	fmulp
	fstcw	pow10.LCW1
	fstcw	pow10.LCW2
	fwait
	andw	$0xf3ff,pow10.LCW2
	orw	$0x0400,pow10.LCW2
	fldcw	pow10.LCW2
	fldl	%st(0)
	frndint
	fldcw	pow10.LCW1
	fxch	%st(1)
	fsub	%st(1),%st
	f2xm1
	faddl	pow10.LC0
	fscale
	fstp	%st(1)
	ret
