/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global pow10;

pow10:
	fldl	4(%esp)
	fldl2t
	fmulp
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
