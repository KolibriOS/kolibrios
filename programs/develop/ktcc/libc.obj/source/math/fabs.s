/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global fabs;

fabs:
	fldl	4(%esp)
	fabs
	ret
