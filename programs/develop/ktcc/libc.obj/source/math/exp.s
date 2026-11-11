.global exp;

exp:
	fldl	4(%esp)
	fldl2e
	fmulp
	fxam
	fnstsw	%ax
	andb	$0x45,%ah
	cmpb	$0x05,%ah
	jne	exp.L1
	fldz
	jmp	exp.L2
exp.L1:
	fld	%st(0)
	frndint
	fxch	%st(1)
	fsub	%st(1),%st
exp.L2:
	f2xm1
	fld1
	faddp
	fscale
	fstp	%st(1)
	ret
