.global log10;

log10:
	fld1
	fldl	4(%esp)
	fyl2x
	fldl2t
	fxch
	fdivp
	ret
