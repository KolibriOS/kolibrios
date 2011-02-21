/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* Modified by SET to copy in reverse order */
# This routine moves %ecx bytes from %ds:%esi to %es:%edi.  It clobbers
# %eax, %ecx, %esi, %edi, and eflags. 
#include <libc/asmdefs.h>
#include<libc/asm.h>

	.file "djmdr.s"
	.text
	.balign 16,,7
MK_C_SYM(__dj_movedata_rev)
	std
	# Add the counter to the index
	addl	%ecx,%edi
	addl	%ecx,%esi
	decl	%esi
	decl	%edi

	cmpl	$15,%ecx
	jle	small_move
	jmp	mod_4_check
	
	# Transfer bytes until either %esi or %edi is aligned % 3
align_mod_4:	
	movsb
	decl	%ecx
mod_4_check:
	movl	%esi,%eax
	andl	$3,%eax
	cmpl	$3,%eax
	jz big_move
	movl	%edi,%eax
	andl	$3,%eax
	cmpl	$3,%eax
	jnz	align_mod_4

big_move:
	movb	%cl,%al	 # We will store leftover count in %al
	shrl	$2,%ecx
	andb	$3,%al
	# Now retrocess the index 3 positions
	subl	$3,%edi
	subl	$3,%esi
	rep
	movsl

	# %ecx known to be zero here, so insert the leftover count in %al
	movb	%al,%cl

	# advance the index by 3
	addl	$3,%edi
	addl	$3,%esi

small_move:
	rep
	movsb
	ret
