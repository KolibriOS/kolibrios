/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
# This routine moves %ecx bytes from %ds:%esi to %es:%edi.  It clobbers
# %eax, %ecx, %esi, %edi, and eflags.  The memory ranges must not overlap,
# unless %esi >= %edi.
#include <libc/asmdefs.h>
#include<libc/asm.h>

	.file "djmd.s"
	.text
.balign 16,,7

MK_C_SYM(__dj_movedata)
	cmpl	$15,%ecx
	jle	small_move
	jmp	mod_4_check
	
	# Transfer bytes until either %esi or %edi is aligned % 4
align_mod_4:	
	movsb
	decl	%ecx
mod_4_check:
	testl	$3,%esi
	jz big_move
	testl	$3,%edi
	jnz	align_mod_4

big_move:
	movb	%cl,%al	 # We will store leftover count in %al
	shrl	$2,%ecx
	andb	$3,%al
	rep
	movsl

	# %ecx known to be zero here, so insert the leftover count in %al
	movb	%al,%cl
small_move:
	rep
	movsb
	ret
