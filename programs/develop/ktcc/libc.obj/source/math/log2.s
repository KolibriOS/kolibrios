.global log2;

log2:
	fld1
	fldl	4(%esp)
	fyl2x
	ret
