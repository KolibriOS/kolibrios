/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

.global pow2;

pow2:
	fldl	4(%esp)
	fld	%st(0)
	frndint
	fxch	%st(1)
	fsub	%st(1),%st
	f2xm1
	fld1
	faddp
	fscale
	fstp	%st(1)
	ret
