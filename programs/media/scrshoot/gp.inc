macro use_gp
{
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

	cmp	csi_bitspp,word 24
	je	.next_str_24

.next_str_32:
	push	edx ebx
	xor	ebx,ebx
@@:
	;push    eax
	mov	eax,[gs:edx]
	mov	[edi+ecx],ax
	shr	eax,8
	mov	[edi+ecx+2],ah
	;pop     eax
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
	;push    eax
	mov	eax,[gs:edx]
	mov	[edi+ecx],ax
	shr	eax,8
	mov	[edi+ecx+2],ah
	;pop     eax
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
}