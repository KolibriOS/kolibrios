/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global sqrt;

sqrt:
	fldl	4(%esp)
	fsqrt
	ret
