/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

.global pow2;

pow2:
	fldl	4(%esp)
	subl	$4,%esp
	fstcw	(%esp)
	movw	(%esp),%ax
	andw	$0xf3ff,%ax
	orw	$0x0400,%ax
	movw	%ax,2(%esp)
	fldcw	2(%esp)
	fld	%st(0)
	frndint
	fldcw	(%esp)
	addl	$4,%esp
	fxch	%st(1)
	fsub	%st(1),%st
	f2xm1
	fld1
	faddp
	fscale
	fstp	%st(1)
	ret
