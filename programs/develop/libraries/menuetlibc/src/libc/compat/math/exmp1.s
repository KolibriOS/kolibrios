/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
pinf:
	.long	0x7F800000

NaN:
	.long	0xFFC00000


MK_C_SYM(expm1)
	movl	8(%esp), %eax		/* Test for special cases. */
	andl	$0x7FFFFFFF, %eax
	cmpl	$0x40862E42, %eax
	jge	bigarg			/* normal args: */
					/* 0 < |x| <= log(DBL_MAX) */
argok:					/* N.B. */
					/* log(DBL_MAX) = 0x40862E42FEFA39EF */
	fldl	4(%esp)
	fldl2e				/* log2(e)  x			   */
	fmulp				/* xs				   */
	fld	%st			/* xs	    xs			   */
	frndint				/* nint(xs) xs			   */
	fxch	%st(1)			/* xs	    nint		   */
	fsub	%st(1),%st		/* fract    nint		   */
	f2xm1				/* exps-1   nint		   */
	fxch	%st(1)			/* nint	    exps-1		   */
	fld1				/* 1	    nint    exps-1	   */
	fscale				/* scale    nint    exps-1	   */
	fld1				/* 1	    scale   nint    exps-1 */
	/* Should be fsubp %st,%st(1) (gas bug) */
	.byte	0xDE, 0xE9		/* scale-1  nint    exps-1	   */
	fxch	%st(2)			/* exps-1   nint    scale-1	   */
	fscale				/* expm	    nint    scale-1	   */
	fstp	%st(1)			/* exp	    scale-1		   */
	faddp				/* exp-1			   */
	ret

bigarg:
	je	edge
	andl	$0x7FF00000, %eax	/* |x| > log(DBL_MAX) */
	cmpl	$0x7FF00000, %eax
	je	abarg

posneg:
	testl	$0x80000000, 8(%esp)
	jnz	argok			/* Large negative -- OK */
	movl	$2, C_SYM(errno)	/* |x| is really big, but finite */
	jmp	argok

edge:					/* |x| is nearly log(DBL_MAX) */
	cmpl	$0xFEFA39EF, 4(%esp)
	jbe	argok
	jmp	posneg

abarg:					/* x = +/-inf, or +NaN */
	testl	$0x000FFFFF, 8(%esp)
	jnz	badarg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	badarg

infarg:					/* |x| = inf */
	testl	$0x80000000, 8(%esp)
	jz	posinf

neginf:
	fld1
	fchs
	ret

posinf:
	movl	$2, C_SYM(errno)
	flds	pinf
	ret

badarg:					/* arg is NaN */
	movl	$1, C_SYM(errno)
	flds	NaN
	ret
