 
format ELF
include '../proc32.inc'
section '.text' executable

public floorf

floorf:

        push	ebp
	mov	ebp,esp
	sub	esp,8

	fstcw	[ebp-12]
	mov	dx,word[ebp-12]
	or	dx,0x0400	
	and	dx,0xf7ff
	mov	word[ebp-16],dx
	fldcw	[ebp-16]	

	fld	dword[ebp+8]
	frndint

	fldcw	[ebp-12]	

	leave

	ret

