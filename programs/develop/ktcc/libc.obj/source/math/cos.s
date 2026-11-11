.global cos;

cos:
	fldl	4(%esp)
	fcos
	fstsw
	sahf
	jnp	cos.L1
	fstp	%st(0)
	pushl	$-1
	pushl	$-1
	fldl	(%esp)
	addl	$8,%esp

cos.L1:
	ret

