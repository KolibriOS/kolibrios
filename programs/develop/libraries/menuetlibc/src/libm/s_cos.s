#include<libc/asm.h>
MK_C_SYM(cos)
	fldl	4(%esp)
	fcos
	fnstsw	%ax
	andw	$0x400,%ax
	jnz	1f
	ret	
1:	fldpi
	fadd	%st(0)
	fxch	%st(1)
2:	fprem1
	fnstsw	%ax
	andw	$0x400,%ax
	jnz	2b
	fstp	%st(1)
	fcos
	ret
