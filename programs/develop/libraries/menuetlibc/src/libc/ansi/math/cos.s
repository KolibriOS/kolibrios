#include<libc/asm.h>
L0:
	.quad	0xffffffffffffffff

MK_C_SYM(cos)
	fldl	4(%esp)
	fcos
	fstsw
	sahf
	jnp	L1
	fstp	%st(0)
	fldl	L0
L1:
	ret

