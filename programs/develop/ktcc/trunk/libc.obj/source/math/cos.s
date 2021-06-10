cos.L0:
	.quad	0xffffffffffffffff

.global cos;

cos:
	fldl	4(%esp)
	fcos
	fstsw
	sahf
	jnp	cos.L1
	fstp	%st(0)
	fldl	cos.L0

cos.L1:
	ret

