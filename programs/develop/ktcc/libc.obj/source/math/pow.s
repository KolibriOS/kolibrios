/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global pow;

frac:
	fld	%st(0)
	frndint
	fxch	%st(1)
	fsub	%st(1),%st
	ret

Lpow2:
	fxam
	fnstsw	%ax
	andb	$0x45,%ah
	cmpb	$0x05,%ah
	jne	pow.L1
	fldz
	jmp	pow.L2
pow.L1:
	call    frac
pow.L2:
	f2xm1
	fld1
	faddp
	fscale
	fstp	%st(1)
	ret

pow:
	movl	16(%esp),%eax
	andl	$0x7fffffff,%eax
	movl	%eax,%ecx
	orl	12(%esp),%ecx
	jz	one
	cmpl	$0,4(%esp)
	jne	xnot1
	cmpl	$0x3ff00000,8(%esp)
	je	one
	cmpl	$0xbff00000,8(%esp)
	jne	xnot1
	cmpl	$0x7ff00000,%eax
	jne	xnot1
	cmpl	$0,12(%esp)
	je	one
xnot1:
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
	jp	error
	fstp	%st(0)
	pushl	$0x7ff00000
	pushl	$0
	fldl	(%esp)
	addl	$8,%esp
	ret
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
	subl	$4,%esp
	fistl	(%esp)
	fxch    %st(1)
	fyl2x
	call	Lpow2
	popl	%ecx
	testl	$1,%ecx
	jz	yeven
	fchs
yeven:
	ret
one:
	fld1
	ret

