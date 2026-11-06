/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

.global pow2;

pow2:
	fldl	4(%esp)
	fxam
	fnstsw	%ax
	andb	$0x45,%ah
	cmpb	$0x05,%ah
	jne	pow2.L1
	fldz
	jmp	pow2.L2
pow2.L1:
	fld	%st(0)
	frndint
	fxch	%st(1)
	fsub	%st(1),%st
pow2.L2:
	f2xm1
	fld1
	faddp
	fscale
	fstp	%st(1)
	ret
