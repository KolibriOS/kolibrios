#include<libc/asm.h>

/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
	.data
pinf:
	.long	0xFF800000

NaN:
	.long	0xFFC00000

temp:
	.long	0, 0

onethird:
	.long	1431655765

two54:
	.long	0x5A800000

a0:
	.float	+1.87277957900533E+00

a1:
	.float	-1.87243905326548E+00

a2:
	.float	+1.60286399719912E+00

a3:
	.float	-7.46198924594210E-01

a4:
	.float	+1.42994392730009E-01

b0:
	.float	14.

b1:
	.float	-7.

b2:
	.float	+2.

one9th:
	.tfloat +0.11111111111111111111

	.text
MK_C_SYM(cbrt)

	movl	8(%esp), %eax
	movl	%eax, %ecx		/* Save sign */

	andl	$0x7FFFFFFF, %eax	/* fabs */
	movl	%eax, 8(%esp)

	cmpl	$0x7FF00000, %eax	/* Control flows straight through for */
	jae	abarg			/* normal args: 0 < fabs(x) < +inf */
	testl	$0x7FF00000, %eax
	jz	verysmall

	mull	onethird
	addl	$0x2A9F7893, %edx
	movl	%edx, temp+4		/* First approximation good */
					/* to 5.5 bits */

have55:
	fldl	4(%esp)
	fld1
	fdivp				/* recip */

	fldl	temp			/* Load approximation */
					/* 4rd-order minimax to 24 bits */
	fld	%st(0)			/* x	x   recip	*/
	fmul	%st(1)			/* x^2	x   recip	*/
	fmul	%st(1)			/* x^3	x   recip	*/
	fmul	%st(2)			/* y	x   recip	*/
	fld	%st(0)			/* y	y   x	  recip */
	fmuls	a4			/* P1'	y   x	  recip */
	fadds	a3			/* P1	y   x	  recip */
	fmul	%st(1)			/* P2'	y   x	  recip */
	fadds	a2			/* P2	y   x	  recip */
	fmul	%st(1)			/* P3'	y   x	  recip */
	fadds	a1			/* P3	y   x	  recip */
	fmulp				/* P4'	x   recip	*/
	fadds	a0			/* P4	x   recip	*/
	fmulp				/* x'	recip		*/
					/* 2nd-order Taylor to 64 bits */
	fld	%st(0)			/* x	x   recip */
	fmul	%st(1)			/* x^2	x   recip */
	fmul	%st(1)			/* x^3	x   recip */
	fmul	%st(2)			/* y	x   recip */
	ffree	%st(2)			/* y	x	  */
	fld	%st(0)			/* y	y   x	  */
	fmuls	b2
	fadds	b1
	fmulp				/* ccc	x */
	fadds	b0			/* P(y) x */
	fmulp				/* x''	  */
	fldt	one9th
	fmulp

cleanup:				/* Restore sign */
	testl	%ecx, %ecx
	jns	end
	fchs

end:
	ret

verysmall:				/* Exponent is 0 */
	movl	8(%esp), %eax
	testl	%eax, %eax
	jnz	denormal
	movl	4(%esp), %eax
	testl	%eax, %eax
	jz	special			/* x = 0 */
	
denormal:
	fldl	4(%esp)
	fmuls	two54			/* Multiply by 2^54 to normalize */
	fstpl	temp

	movl	temp+4, %eax
	mull	onethird
	addl	$0x297F7893, %edx	/* Undo 2^54 multiplier */
	movl	%edx, temp+4		/* First approximation to 5.5 bits */
	movl	$0, temp

	jmp	have55

abarg:					/* x = inf, or NaN */
	testl	$0x000FFFFF, %eax
	jnz	badarg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jz	special

badarg:					/* arg is negative or NaN */
	movl	$1, C_SYM(errno)
	flds	NaN
	ret

special:
	fldl	4(%esp)			/* x = 0 or inf: just load x */
	jmp	cleanup
