/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
#include<libc/asm.h>
NaN:
	.long	0xFFC00000

ninf:
	.long	0xFF800000

pinf:
	.long	0x7F800000

.text
MK_C_SYM(log1p)					/* ln(1.+x) */
					/* log1p(x) */
	movl	8(%esp), %eax
	movl	%eax, %edx

	cmpl	$0xBFF00000,%eax	/* x <= -1 ? */
	jae	nonpos

	andl	$0x7FF00000,%eax	
	cmpl	$0x7FF00000,%eax	
	je	abarg			/* x == +inf or +NaN */

	movl	%edx, %eax
	andl	$0x7FFFFFFF,%eax
	cmpl	$0x3FD2BEC3,%eax	/* 1 - sqrt(0.5) */
	fldln2				/* ln(2) */
	jbe	1f
	fld1
	faddl	4(%esp)
	fyl2x				/* logi(x) */
	ret
1:					/* log1pi(x) */
	fldl	4(%esp)
	fyl2xp1
	ret

nonpos:
	cmpl	$0xBFF00000,%eax	
	ja	badarg			/* x == -1 ? */
	movl	4(%esp), %eax
	testl %eax, %eax
	jz	negone

badarg:
	movl	$1, C_SYM(errno)
	flds	NaN
	ret

negone:
	movl	$2, C_SYM(errno)
	flds	ninf			/* arg == -1; load -inf. */
	ret

abarg:
	movl	%edx, %eax
	testl	$0x000FFFFF, %eax
	jnz	badarg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	badarg

	flds	pinf			/* arg = +inf */
	ret
