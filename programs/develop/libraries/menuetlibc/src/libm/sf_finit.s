#include<libc/asm.h>
MK_C_SYM(finitef)
	movl	4(%esp),%eax
	andl	$0x7ff00000, %eax
	cmpl	$0x7ff00000, %eax
	setne	%al
	andl	$0x000000ff, %eax
	ret
