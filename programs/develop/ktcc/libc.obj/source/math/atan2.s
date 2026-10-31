/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

.text

.global atan2;

atan2:
	fldl	4(%esp)
	fldl	12(%esp)
	ftst
	fnstsw	%ax
	sahf
	jne	doit
	fxch	%st(1)
	ftst
	fnstsw	%ax
	sahf
	je	isanan
	fxch	%st(1)
doit:
	fpatan
	ret
isanan:
	movl	$1, __errno
	fstp	%st(0)
	fstp	%st(0)
	pushl	$0x7fffffff
	pushl	$-1
	fldl	(%esp)
	addl	$8,%esp
	ret
