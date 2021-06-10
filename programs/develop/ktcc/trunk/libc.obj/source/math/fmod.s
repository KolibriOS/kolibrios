/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.text

.global fmod;

fmod:
	fldl	4(%esp)
	fldl	12(%esp)
	ftst
	fnstsw	%ax
	fxch	%st(1)
	sahf
	jnz	next
	fstpl	%st(0)
	jmp	out
next:
	fprem
	fnstsw	%ax
	sahf
	jpe	next
	fstpl	%st(1)
out:
	ret


