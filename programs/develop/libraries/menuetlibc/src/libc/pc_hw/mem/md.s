/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_ESI
#define USE_EDI
#include <libc/asmdefs.h>
#include <libc/asmdefs.h>
#include<libc/asm.h>

MK_C_SYM(__movedata)	/* src_sel, src_ofs, dest_sel, dest_ofs, len */
	ENTER

	pushw	%ds
	pushw	%es

	movw	ARG1,%ds
	movw	ARG3,%es

	movl	ARG2,%esi
	movl	ARG4,%edi
	movl	ARG5,%ecx

	call	C_SYM(__dj_movedata)

	popw	%es
	popw	%ds

	LEAVE

