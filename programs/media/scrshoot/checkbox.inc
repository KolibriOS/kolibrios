macro use_check_box
{
ch_text_margin=3
ch_size=10
ch_left equ [edi]
ch_top equ [edi+2]
ch_color equ [edi+4]
ch_border_color equ [edi+8]
ch_text_color equ [edi+12]
ch_text_ptr equ [edi+16]
ch_flags equ [edi+20]
check_box:
.draw:
pusha
	mov	eax,38
	mov	bx,ch_left
	push	bx
	shl	ebx,16
	pop	bx
	mov	cx,ch_top
	push	cx
	shl	ecx,16
	pop	cx
	push	cx
	add	cx,ch_size
	mov	edx,ch_border_color
	int	0x40
	push	bx
	add	bx,ch_size
	ror	ebx,16
	add	bx,ch_size
	int	0x40
	pop	bx
	pop	cx
	int	0x40
	add	cx,ch_size
	push	cx
	shl	ecx,16
	pop	cx
	int	0x40

	mov	eax,13
	mov	bx,ch_left
	inc	bx
	shl	ebx,16
	mov	bx,ch_size
	dec	bx
	push	bx
	mov	cx,ch_top
	inc	cx
	shl	ecx,16
	pop	cx
	mov	edx,ch_color
	int	0x40

	bt	dword ch_flags,1
	jnc	@f
	call	.draw_ch
@@:

	call	.get_text_n
	mov	bx,ch_left
	add	bx,(ch_size+ch_text_margin)
	shl	ebx,16
	mov	bx,ch_top
	add	bx,(ch_size-9+2)
	mov	ecx,ch_text_color
	mov	eax,4
	int	0x40
popa
ret

.get_text_n:
	push	eax
	xor	esi,esi
	mov	edx,ch_text_ptr
@@:
	mov	ah,[edx+esi]
	inc	esi
	test	ah,ah
	jnz	@b
	dec	esi
	pop	eax
ret

.clear_ch:
	mov	edx,ch_color
	jmp	@f
.draw_ch:
	mov	edx,ch_border_color
@@:
	mov	bx,ch_left
	add	bx,(ch_size/3)
	shl	ebx,16
	mov	bx,(ch_size/2)
	push	bx
	mov	cx,ch_top
	add	cx,(ch_size/3)
	shl	ecx,16
	pop	cx
	mov	eax,13
	int	0x40
ret

.mouse:
pusha
	mov	eax,37
	mov	ebx,2
	int	0x40
	bt	eax,0
	jc	@f
	btr	dword ch_flags,2
	popa
	ret
@@:
	bts	dword ch_flags,2
	jc	.mouse_end
	call	.get_text_n
	imul	esi,6
	add	si,ch_text_margin
	mov	eax,37
	xor	ebx,ebx
	inc	ebx
	int	0x40
	mov	bx,ch_top
	cmp	ax,bx
	jl	.mouse_end
	add	bx,ch_size
	cmp	ax,bx
	jg	.mouse_end
	shr	eax,16
	mov	bx,ch_left
	cmp	ax,bx
	jl	.mouse_end
	add	bx,ch_size
	add	bx,si
	cmp	ax,bx
	jg	.mouse_end

	bts	dword ch_flags,1
	jc	@f
	call	.draw_ch
	jmp	.mouse_end
@@:
	btr	dword ch_flags,1
	call	.clear_ch
.mouse_end:
popa
ret
}

struc check_box left,top,color,border_color,text_color,text,flags
{
.left: dw left	;+0
.top: dw top	;+2
.color: dd color ;+4
.border_color: dd border_color ;+8
.text_color: dd text_color     ;+12
.text: dd text	     ;+16
.flags: dd flags+0   ;+20
}

ch_struc_size=24
macro draw_check_boxes start,end
{
	mov	edi,start
	mov	ecx,((end-start)/ch_struc_size)
@@:
	call	check_box.draw
	add	edi,ch_struc_size
	loop	@b
}

macro mouse_check_boxes start,end
{
	mov	edi,start
	mov	ecx,((end-start)/ch_struc_size)
@@:
	call	check_box.mouse
	add	edi,ch_struc_size
	loop	@b
}

macro check_boxes_set_sys_color start,end,color_table
{
	mov	edi,start
	mov	ecx,((end-start)/ch_struc_size)
	mov	esi,color_table
@@:
	push	dword [esi+32]
	pop	dword ch_text_color
	push	dword [esi+36]
	pop	dword ch_border_color
	add	edi,ch_struc_size
	loop	@b
}