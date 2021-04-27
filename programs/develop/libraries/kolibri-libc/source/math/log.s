/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global log;

log:
	fldln2
	fldl	4(%esp)
	fyl2x
	ret
