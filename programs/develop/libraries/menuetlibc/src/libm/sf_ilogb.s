#include<libc/asm.h>
MK_C_SYM(ilogbf)
	pushl	%esp
	movl	%esp,%ebp
	subl	$4,%esp

	flds	8(%ebp)
	fxtract
	fstpl	%st

	fistpl	-4(%ebp)
	movl	-4(%ebp),%eax

	leave
	ret
