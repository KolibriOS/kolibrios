;esi - screen size
;edi - pointer to image
csi_width equ [esi+2]
csi_height equ [esi]
csi_bitspp equ [esi+4]
csi_bytesps equ [esi+6]


copy_screen_to_image:
pusha
	xor	edx,edx
	xor	ecx,ecx
	xor	ebx,ebx

	bt	dword [use_rect.flags],1
	jc	copy_area

	cmp	csi_bitspp,word 24
	je	.next_str_24
	cmp	csi_bitspp,word 16
	je	.next_str_16

.next_str_32:
	push	edx ebx
	xor	ebx,ebx
@@:
	;push	 eax
	mov	eax,[gs:edx]
	mov	[edi+ecx],ax
	shr	eax,8
	mov	[edi+ecx+2],ah
	;pop	 eax
	add	ecx,3
	add	edx,4;eax
	inc	bx
	cmp	bx,csi_width
	jl     @b
	pop	ebx edx
	add	edx,csi_bytesps
	inc	bx
	cmp	bx,csi_height
	jl	.next_str_32
popa
ret

.next_str_24:
	push	edx ebx
	xor	ebx,ebx
@@:
	;push	 eax
	mov	eax,[gs:edx]
	mov	[edi+ecx],ax
	shr	eax,8
	mov	[edi+ecx+2],ah
	;pop	 eax
	add	ecx,3
	add	edx,3;eax
	inc	bx
	cmp	bx,csi_width
	jl     @b
	pop	ebx edx
	add	edx,csi_bytesps
	inc	bx
	cmp	bx,csi_height
	jl	.next_str_24
popa
ret

.next_str_16:
	push	edx ebx
	xor	ebx,ebx
@@:
	;push	 eax
	mov	eax,[gs:edx]
	shl	eax, 3
	ror	eax, 8
	shl	ax, 2
	ror	eax, 8
	shl	al, 3
	mov	[edi+ecx+2],al
	rol	eax, 16
	mov	[edi+ecx],ax
	;pop	 eax
	add	ecx,3
	add	edx,2
	inc	bx
	cmp	bx,csi_width
	jl     @b
	pop	ebx edx
	add	edx,csi_bytesps
	inc	bx
	cmp	bx,csi_height
	jl	.next_str_16
popa
ret

copy_area:
	mov	eax,[scr.bytesps]
	mov	ebx,eax
	movzx	ecx,word [rect.top]
	mul	ecx
	movzx	ecx,word [rect.width]
	shl	ecx,2
	sub	ebx,ecx
	movzx	ecx,word [rect.left]
	shl	ecx,2
	add	eax,ecx
	movzx	ecx,word [rect.height]
	mov	edx,eax
.next_str:
	movzx	esi,word [rect.width]
.next_pixel:
	mov	eax,[gs:edx]
	mov	[edi],ax
	shr	eax,8
	mov	[edi+2],ah
	add	edx,4
	add	edi,3
	dec	esi
	jnz	.next_pixel
	add	edx,ebx
	loop	.next_str
popa
ret