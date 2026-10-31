/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global pow;

frac:
	fld	%st(0)
	frndint
	fxch	%st(1)
	fsub	%st(1),%st
	ret

Lpow2:
	call    frac
	f2xm1
	fld1
	faddp
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

