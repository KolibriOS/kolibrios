/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

.global sin;

sin.L0:
	.quad	0xffffffffffffffff

sin:
	fldl	4(%esp)
	fsin
	fstsw
	sahf
	jnp	sin.L1
	fstp	%st(0)
	fldl	sin.L0
sin.L1:
	ret	

