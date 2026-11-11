/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global tan;

tan:
	fldl	4(%esp)
	fptan
	fstsw
	fstp	%st(0)
	sahf
	jnp	tan.L1
	fstp	%st(0) /*- if exception, there is nothing on the stack */
	pushl	$-1
	pushl	$-1
	fldl	(%esp)
	addl	$8,%esp
tan.L1:
	ret
