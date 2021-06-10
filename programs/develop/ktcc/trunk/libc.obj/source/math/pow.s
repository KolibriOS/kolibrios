/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
	.data
yint: 
	.word   0,0
pow.LCW1:
	.word	0
pow.LCW2:
	.word	0

	.text
pow.LC0:
	.double	0d1.0e+00

.global pow;

frac:
	fstcw	pow.LCW1
	fstcw	pow.LCW2
	fwait
	andw	$0xf3ff,pow.LCW2
	orw	$0x0400,pow.LCW2
	fldcw	pow.LCW2
	fldl	%st(0)
	frndint
	fldcw	pow.LCW1
	fxch	%st(1)
	fsub	%st(1),%st
	ret

Lpow2:
	call    frac
	f2xm1
	faddl	pow.LC0
	fscale
	fstp	%st(1)
	ret

pow:
	fldl	12(%esp)
	fldl	4(%esp)
	ftst	
	fnstsw	%ax
	sahf
	jbe	xltez
	fyl2x
	jmp	Lpow2
xltez:
	jb	xltz
	fstp	%st(0)
	ftst
	fnstsw	%ax
	sahf
	ja	ygtz
	jb	error
	fstp	%st(0) 
	fld1
	fchs
error:
	fsqrt
	ret
ygtz:
	fstp	%st(0)
	fldz
	ret
xltz:
	fabs
	fxch    %st(1)
	call	frac
	ftst
	fnstsw	%ax
	fstp	%st(0)
	sahf
	je	yisint
	fstp	%st(0)
	fchs
	jmp	error
yisint:
	fistl	yint
	fxch    %st(1)
	fyl2x
	call	Lpow2
	andl	$1,yint
	jz	yeven
	fchs
yeven:
	ret

