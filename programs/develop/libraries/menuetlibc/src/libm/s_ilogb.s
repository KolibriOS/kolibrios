#include<libc/asm.h>
MK_C_SYM(ilogb)
	pushl	%esp
	movl	%esp,%ebp
	subl	$4,%esp

	fldl	8(%ebp)
	fxtract
	fstpl	%st

	fistpl	-4(%ebp)
	movl	-4(%ebp),%eax

	leave
	ret
