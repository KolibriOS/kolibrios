/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>
#include<libc/asm.h>
	
	.text

MK_C_SYM(fpreset)

	fninit
	ret

