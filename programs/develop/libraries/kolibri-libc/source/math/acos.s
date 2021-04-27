.text

acos.LC0:
	.double 0d1.00000000000000000000e+00

.global acos;

acos:
	fldl	4(%esp)
	fld1
	fsubp	%st(0),%st(1)
	fsqrt

	fldl	4(%esp)
	fld1
	faddp	%st(0),%st(1)
	fsqrt

	fpatan

	fld	%st(0)
	faddp
	ret
