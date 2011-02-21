# 1 "longjmp.s"
#include<libc/asm.h>
MK_C_SYM(longjmp)
	movl	4(%esp),%edi	
	movl	8(%esp),%eax	
	movl	%eax,0(%edi)

	movw	46(%edi),%fs
	movw	48(%edi),%gs
	movl	4(%edi),%ebx
	movl	8(%edi),%ecx
	movl	12(%edi),%edx
	movl	24(%edi),%ebp

	movw	50(%edi),%es
	movl	28(%edi),%esi
	subl	$28,%esi

	movl	60(%edi),%eax
	es
	movl	%eax,(%esi)	

	movzwl	42(%edi),%eax
	es
	movl	%eax,4(%esi)	 

	movl	20(%edi),%eax
	es
	movl	%eax,8(%esi)	 

	movl	16(%edi),%eax
	es
	movl	%eax,12(%esi)	 

	movl	32(%edi),%eax
	es
	movl	%eax,16(%esi)	 

	movl	40(%edi),%eax
	es
	movl	%eax,20(%esi)	 

	movl	36(%edi),%eax
	es
	movl	%eax,24(%esi)	 

	movl	0(%edi),%eax
	movw	44(%edi),%es

	movw	50(%edi),%ss
	movl	%esi,%esp

	popl	C_SYM(__djgpp_exception_state_ptr)
	popl	%ds
	popl	%edi
	popl	%esi

	iret			 

MK_C_SYM(__djgpp_exception_state_ptr)
 .word 0
 .word 0
 .word 0
 .word 0
