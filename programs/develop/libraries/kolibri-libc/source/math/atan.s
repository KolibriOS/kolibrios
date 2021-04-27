/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global atan;

atan:
	fldl	4(%esp)
	fld1
	fpatan
	ret

