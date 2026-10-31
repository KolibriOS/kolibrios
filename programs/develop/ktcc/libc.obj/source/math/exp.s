.global exp;

exp:
	fldl	4(%esp)
	fldl2e
	fmulp
	fld	%st(0)
	frndint
	fxch	%st(1)
	fsub	%st(1),%st
	f2xm1
	fld1
	faddp
	fscale
	fstp	%st(1)
	ret
