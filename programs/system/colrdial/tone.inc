;-----------------------------------------------------------------------------
align 4
create_tone:
;-------------------------------------------
	mov	ebx,[tone_SIZE_Y]
	xor	eax,eax
	dec	al
	sub	al,[tone_color.red]
	shl	eax,24
	xor	edx,edx
	div	ebx
	mov	[offset_y.red],eax

	xor	eax,eax
	dec	al
	sub	al,[tone_color.green]
	shl	eax,24
	xor	edx,edx
	div	ebx
	mov	[offset_y.green],eax

	xor	eax,eax
	dec	al
	sub	al,[tone_color.blue]
	shl	eax,24
	xor	edx,edx
	div	ebx
	mov	[offset_y.blue],eax
;-------------------------------------------
	mov	edi,[tone_area]
	xor	ecx,ecx
;-------------------------------------------
align 4
@@:
	mov	eax,[tone_color]
; blue
	mov	edx,[offset_y.blue]
	call	next_color_Y
	ror	eax,8
; green
	mov	edx,[offset_y.green]
	call	next_color_Y
	ror	eax,8
; red
	mov	edx,[offset_y.red]
	call	next_color_Y
	rol	eax,16

	call	tone_put_line
	add	ecx,1
	cmp	ecx,[tone_SIZE_X]
	jne	@b

	ret
;-----------------------------------------------------------------------------
tone_put_line:
	push	ecx

	mov	ecx,eax
	mov	ebx,[tone_SIZE_X]
	shl	eax,24
	xor	edx,edx
	div	ebx
	mov	[offset_x.blue],eax

	mov	eax,ecx
	shr	eax,8
	shl	eax,24
	xor	edx,edx
	div	ebx
	mov	[offset_x.green],eax

	mov	eax,ecx
	shr	eax,16
	shl	eax,24
	xor	edx,edx
	div	ebx
	mov	[offset_x.red],eax
	mov	eax,ecx

	push	ebx esi
	mov	ecx,[tone_SIZE_X]
	xor	edx,edx
	xor	ebx,ebx
	xor	esi,esi
@@:
	mov	eax,edx
	shr	eax,24
	add	edx,[offset_x.blue]
	mov	[edi],al
	mov	eax,ebx
	shr	eax,24
	add	ebx,[offset_x.green]
	mov	[edi+1],al
	mov	eax,esi
	shr	eax,24
	add	esi,[offset_x.red]
	mov	[edi+2],al
	add	edi,3
	dec	ecx
	jnz	@b
	pop	esi ebx
	pop	ecx
	ret
;-----------------------------------------------------------------------------
align 4
next_color_Y:
	imul	edx,ecx
	rol	edx,8
	add	al,dl
	ret
;-----------------------------------------------------------------------------