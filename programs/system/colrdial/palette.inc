;-----------------------------------------------------------------------------
align 4
create_palette:
;-------------------------------------------
	mov	eax,[palette_SIZE_Y]
	mov	ebx,6
	xor	edx,edx
	div	ebx
	mov	[part_of_size_y],eax
;-------------------------------------------
	mov	ebx,eax
	mov	eax,255 shl 24
	xor	edx,edx
	div	ebx
	mov	[offset_y],eax
;-------------------------------------------
	mov	edi,[palette_area]
	mov	edx,[offset_y]
;-------------------------------------------
	xor	ecx,ecx
	mov	eax,[tone_color]
;-------------------------------------------
align 4
@@:
	call	palette_put_line
; blue
	call	next_color
	add	ecx,1
	cmp	ecx,[part_of_size_y]
	jne	@b
;-------------------------------------------
	mov	ecx,[part_of_size_y]
;-------------------------------------------
align 4
@@:
	call	palette_put_line
; red
	ror	eax,16
	call	next_color
	rol	eax,16
	sub	ecx,1
	jne	@b
;-------------------------------------------
	xor	ecx,ecx
;-------------------------------------------
align 4
@@:
	call	palette_put_line
; green
	ror	eax,8
	call	next_color
	rol	eax,8
	add	ecx,1
	cmp	ecx,[part_of_size_y]
	jne	@b
;-------------------------------------------
	mov	ecx,[part_of_size_y]
;-------------------------------------------
align 4
@@:
	call	palette_put_line
; blue
	call	next_color
	sub	ecx,1
	jne	@b
;-------------------------------------------
	xor	ecx,ecx
;-------------------------------------------
align 4
@@:
	call	palette_put_line
; red
	ror	eax,16
	call	next_color
	rol	eax,16
	add	ecx,1
	cmp	ecx,[part_of_size_y]
	jne	@b
;-------------------------------------------
	mov	ecx,[part_of_size_y]
;-------------------------------------------
align 4
@@:
	call	palette_put_line
; green
	ror	eax,8
	call	next_color
	rol	eax,8
	sub	ecx,1
	jne	@b
;-------------------------------------------
	mov	edx,[part_of_size_y]
	lea	edx,[edx*3]
	shl	edx,1
	mov	ecx,[palette_SIZE_Y]
	sub	ecx,edx
	test	ecx,ecx
	jz	.end
;-------------------------------------------
align 4
@@:
	call	palette_put_line
	loop	@b
;-------------------------------------------
align 4
.end:
	ret
;---------------------------------------------------------------------
align 4
next_color:
	mov	ebx,ecx
	imul	ebx,edx
	rol	ebx,8
	mov	al,bl
	ret
;---------------------------------------------------------------------
align 4
palette_put_line:
	push	ecx
	mov	ecx,[palette_SIZE_X]
	cld
;-------------------------------------------
align 4
.loop:
	stosd
	dec	edi
	loop	.loop
	pop	ecx
	ret
;---------------------------------------------------------------------