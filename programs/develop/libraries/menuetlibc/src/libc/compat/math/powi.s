/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
	.text
MK_C_SYM(powi)
	/* double powi(double x, int iy) = x^iy */

	fldl	4(%esp)			/* x2p = x; */
	movl	12(%esp), %eax

	testl	%eax, %eax		/* if (iy < 0) { */
	jge	Endif1
	negl	%eax			/*	  iy = -iy; */
	fld1				/*	  x = 1./x; */
	/* Should be fdivrp %st, %st(1) (gas bug) */
	.byte	0xDE, 0xF1
Endif1:					/* } */

	fld1				/* result = 1.; */
	fxch	%st(1)

	jmp	Test
	.balign 16,,7
Loop:
	testb	$1, %al			/*	  if (iy & 1) result *= x2p; */
	je	Endif2
	fmul	%st, %st(1)
Endif2:
	shrl	$1, %eax		/*	  (unsigned) iy >>= 1; */
	fmul	%st(0), %st		/*	  x2p *= x2p; */
Test:
	testl	%eax, %eax		/* } */
	jne	Loop
	fstp	%st(0)
	ret				/* return result; */
