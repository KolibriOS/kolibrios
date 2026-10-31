/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global sin;

sin:
	fldl	4(%esp)
	fsin
	fstsw
	sahf
	jnp	sin.L1
	fstp	%st(0)
	pushl	$-1
	pushl	$-1
	fldl	(%esp)
	addl	$8,%esp
sin.L1:
	ret	

