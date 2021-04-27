/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

.global floor;

floor:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$8,%esp        

	fstcw	-4(%ebp)
	fwait
	movw	-4(%ebp),%ax
	andw	$0xf3ff,%ax
	orw	$0x0400,%ax
	movw	%ax,-2(%ebp)
	fldcw	-2(%ebp)

	fldl	8(%ebp)
	frndint

	fldcw	-4(%ebp)

	movl	%ebp,%esp
	popl	%ebp
	ret

