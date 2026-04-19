.global round;

round:
	fldl	4(%esp)
	fistp	4(%esp)
	fild	4(%esp)
	ret
