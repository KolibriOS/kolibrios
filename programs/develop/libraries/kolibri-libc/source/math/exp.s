
	.data
exp.LCW1:
	.word	0
exp.LCW2:
	.word	0
exp.LC0:
	.double	0d1.0e+00

	.text

.global exp;

exp:
	fldl	4(%esp)
	fldl2e
	fmulp
	fstcw	exp.LCW1
	fstcw	exp.LCW2
	fwait
	andw	$0xf3ff, exp.LCW2
	orw	$0x0400, exp.LCW2
	fldcw	exp.LCW2
	fldl	%st(0)
	frndint
	fldcw	exp.LCW1
	fxch	%st(1)
	fsub	%st(1),%st
	f2xm1
	faddl	exp.LC0
	fscale
	fstp	%st(1)
	ret
