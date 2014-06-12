#include<libc/asm.h>
MK_C_SYM(ceil)

	pushl	%ebp
	movl	%esp,%ebp
	subl	$8,%esp

	fstcw	-12(%ebp)		
	movw	-12(%ebp),%dx
	orw	$0x0800,%dx		
	andw	$0xfbff,%dx
	movw	%dx,-16(%ebp)
	fldcw	-16(%ebp)		

	fldl	8(%ebp);		
	frndint

	fldcw	-12(%ebp)		

	leave
	ret
