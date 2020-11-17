#include <libc/asm.h>
MK_C_SYM(_alloca)
	sub	%eax, %esp
	mov	(%esp,%eax),%eax
	mov	%eax, (%esp)
	ret
