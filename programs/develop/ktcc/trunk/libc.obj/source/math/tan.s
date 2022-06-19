/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global tan;

tan.L0:
	.quad	0xffffffffffffffff

tan:
	fldl	4(%esp)
	fptan
	fstsw
	fstp	%st(0)
	sahf
	jnp	tan.L1
	fstp	%st(0) /*- if exception, there is nothing on the stack */
	fldl	tan.L0
tan.L1:
	ret
