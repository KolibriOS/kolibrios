;------------------------------------------------------------------------------
draw_device_button:
	call	device_detect_f70
	mov	edi,[retrieved_devices_table_counter]
	mov	edx,120
	mov	bx,[left_panel_x.start_x]
	mov	cx,[left_panel_y.start_y]
	shl	ebx,16
;	mov	bx,50	; button size X 
	sub	cx,30
	shl	ecx,16
	mov	cx,12
	mov	esi,0xaa00
	mov	ebp,retrieved_devices_table
;--------------------------------------
.loop:
	push	esi
	mov	esi,ebp
	cld
@@:
	lodsb
	test	al,al
	jnz	@b
	
	sub	esi,ebp
	lea	esi,[esi*3]
	shl	esi,1
	mov	bx,si
	pop	esi
	
	mcall	8

	pusha
	mov	eax,ebx
	inc	eax
	shl	eax,16
	add	ebx,eax
	mov	bx,2
	inc	ecx
	mcall	13,,,0xcccccc
	popa

	pusha
	mov	edx,ebp
	inc	edx
	shr	ecx,16
	mov	bx,cx
	add	ebx,4 shl 16 +3
	mcall	4,,0x90000000
	sub	ebx,1 shl 16 + 1
	mcall	,,0x90ffffff
	popa

	mov	eax,ebx
	shl	eax,16
	add	ebx,eax
	add	ebx,3 shl 16

	inc	edx
	add	ebp,10
	dec	edi
	jnz	.loop
	
	xor	eax,eax
	mov	ax,[right_panel_x.start_x]
	add	ax,[right_panel_x.size_x]
	ror	ebx,16
	sub	ax,bx
	rol	ebx,16
	inc	ax
	mov	bx,ax
	
	add	ecx,1
	add	ebx,1
	mcall	13,,,0xcccccc
	ret
;------------------------------------------------------------------------------
