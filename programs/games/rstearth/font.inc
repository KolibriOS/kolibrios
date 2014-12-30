;---------------------------------------------------------------------
draw_font:
; in:
; ebx - X*65536 + Y
; edx - text ASCII
;  
	pusha
	xor	ebp,ebp
	mov	esi,edx
	mov	edx,ebx
	cld
;--------------------------------------
@@:
	lodsb
	test	al,al
	jz	.exit

	movzx	ebx,al
	imul	ebx, FONT_SIZE_X * FONT_SIZE_Y /8
	mov	eax,[font_icons]
	add	ebx,[eax+28]
	add	ebx,eax
	mov	ecx,FONT_SIZE_X shl 16 + FONT_SIZE_Y
	mov	edi,[eax+20]	; palette
	add	edi,eax

	push	esi
	mov	esi,1
	mcall	65
	pop	esi

	add	edx,FONT_REAL_SIZE_X shl 16	; font size x
	jmp	@b
;--------------------------------------
.exit:
	popa
	ret
;---------------------------------------------------------------------