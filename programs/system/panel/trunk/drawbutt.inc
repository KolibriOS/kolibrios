;------------------------------------------------------------------------------	
align 4
draw_appl_button:
; in:
; ebx = [x]*65536 + [size x]
; ecx = [y]*65536 + [size y]
; edx = internal color 0xRRGGBB
; esi = external color 0xRRGGBB
; edi = 0 up or 1 down
COLOR_CHANGE_MAGNITUDE_0 = 0x080808
COLOR_CHANGE_MAGNITUDE = 0x101010
COLOR_CHANGE_MAGNITUDE_1 = 0x202020
COLOR_CHANGE_MAGNITUDE_2 = 0x404040
	pusha
	add	ebx,2 shl 16
	sub	bx,4
	add	ecx,2 shl 16
	sub	cx,4
	mcall	13
;------------------------------------------------------------------------------	
;	mov	edx,esi
	test	edi,edi
	jnz	@f

	mov	eax,COLOR_CHANGE_MAGNITUDE
	call	add_color_change_magnitude
	jmp	.1
;--------------------------------------
align 4
@@:
	mov	eax,COLOR_CHANGE_MAGNITUDE
	call	subtract_color_change_magnitude
;--------------------------------------
align 4
.1:
	call	draw_four_extreme_lines
;------------------------------------------------------------------------------	
	mov	edx,esi
	mov	eax,COLOR_CHANGE_MAGNITUDE_2
	call	subtract_color_change_magnitude
	sub	ecx,1 shl 16
	add	ecx,2
	sub	ebx,1 shl 16
	add	ebx,2
	call	draw_four_extreme_lines
;------------------------------------------------------------------------------
	popa
	ret
;------------------------------------------------------------------------------
align 4
draw_four_extreme_lines:
; top left point
	push	ebx ecx
	shr	ebx,16
	shr	ecx,16
	mcall	1
	pop	ecx ebx
; bottom right point
	push	ebx ecx
	movzx	eax,bx
	shr	ebx,16
	add	ebx,eax
	dec	ebx
	movzx	eax,cx
	shr	ecx,16
	add	ecx,eax
	dec	ecx
	mcall	1
	pop	ecx ebx
;--------------------------------------
; bottom left point
	push	ebx ecx
	shr	ebx,16
	movzx	eax,cx
	shr	ecx,16
	add	ecx,eax
	dec	ecx
	mcall	1
	pop	ecx ebx
;--------------------------------------
; top right point
	push	ebx ecx
	movzx	eax,bx
	shr	ebx,16
	add	ebx,eax
	dec	ebx
	shr	ecx,16
	mcall	1
	pop	ecx ebx
;--------------------------------------	
; top line
	push	ecx
	sub	ecx,1 shl 16
	mov	cx,1
	mcall	13
	pop	ecx
; bottom line 
	push	ecx
	mov	ax,cx
	shl	eax,16
	add	ecx,eax
	mov	cx,1
	mcall	13
	pop	ecx
; left line
	push	ebx
	sub	ebx,1 shl 16
	mov	bx,1
	mcall	13
	pop	ebx
; right line
	push	ebx
	mov	ax,bx
	shl	eax,16
	add	ebx,eax
	mov	bx,1
	mcall	13
	pop	ebx
	
	ret
;------------------------------------------------------------------------------
align 4
subtract_color_change_magnitude:
; in:
; edx = original color
; eax = magnitude
; out:
; edx = processed color
	sub	dl,al
	jae	@f
	xor	dl,dl
@@:
	ror	edx,8
	ror	eax,8
	sub	dl,al
	jae	@f
	xor	dl,dl
@@:
	ror	edx,8
	ror	eax,8
	sub	dl,al
	jae	@f
	xor	dl,dl
@@:
	rol	edx,16
	ret
;------------------------------------------------------------------------------
align 4
add_color_change_magnitude:
; in:
; edx = original color
; eax = magnitude
; out:
; edx = processed color
	add	dl,al
	jae	@f
	mov	dl,0xff
@@:
	ror	edx,8
	ror	eax,8
	add	dl,al
	jae	@f
	mov	dl,0xff
@@:
	ror	edx,8
	ror	eax,8
	add	dl,al
	jae	@f
	mov	dl,0xff
@@:
	rol	edx,16
	ret
;------------------------------------------------------------------------------
calculate_button_y_coordinate_and_size:
	mov	ecx,[button_top_offset]
	shl	ecx,16
	add	ecx,[height]
	sub	ecx,[button_top_offset]
	sub	ecx,[button_bottom_offset]
	ret