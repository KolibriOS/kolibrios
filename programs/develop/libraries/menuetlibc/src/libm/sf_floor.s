#include<libc/asm.h>
MK_C_SYM(floorf)
	pushl	%ebp
	movl	%esp,%ebp
	subl	$8,%esp

	fstcw	-12(%ebp)		
	movw	-12(%ebp),%dx
	orw	$0x0400,%dx		
	andw	$0xf7ff,%dx
	movw	%dx,-16(%ebp)
	fldcw	-16(%ebp)		

	flds	8(%ebp);		
	frndint

	fldcw	-12(%ebp)		

	leave
	ret
