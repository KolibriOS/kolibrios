#include<libc/asm.h>
MK_C_SYM(setjmp)
	pushl	%ebp
	movl	%esp,%ebp

	pushl	%edi
	movl	8(%ebp),%edi

	movl	%eax, (%edi)
	movl	%ebx,4(%edi)
	movl	%ecx,8(%edi)
	movl	%edx,12(%edi)
	movl	%esi,16(%edi)

	movl	-4(%ebp),%eax
	movl	%eax,20(%edi)

	movl	(%ebp),%eax
	movl	%eax,24(%edi)

	movl	%esp,%eax
	addl	$12,%eax
	movl	%eax,28(%edi)
	
	movl	4(%ebp),%eax
	movl	%eax,32(%edi)

	pushfl
	popl	36(%edi)

	movw	%cs, 40(%edi)
	movw	%ds, 42(%edi)
	movw	%es, 44(%edi)
	movw	%fs, 46(%edi)
	movw	%gs, 48(%edi)
	movw	%ss, 50(%edi)
	
	movl	C_SYM(__djgpp_exception_state_ptr), %eax
	movl	%eax, 60(%edi)

	popl	%edi
	xorl	%eax,%eax
	popl	%ebp
	ret
