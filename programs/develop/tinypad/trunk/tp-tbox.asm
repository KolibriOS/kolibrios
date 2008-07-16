diff16 'tp-tbox.asm',0,$

virtual at ebp
  tbox TBOX
end virtual

tb.pos.x	db ?
tb.sel.x	db ?
tb.sel.selected db ?

proc textbox.get_width
	push	ebx edx
	movzx	eax,[tbox.width]
	add	eax,-6
	xor	edx,edx
	mov	ebx,6
	div	ebx
	movzx	ebx,[tbox.length]
	cmp	eax,ebx
	jle	@f
	mov	eax,ebx
    @@: pop	edx ebx
	ret
endp

proc textbox.delete_selection
	cmp	[tb.sel.selected],0
	je	.exit.2
	pushad
	movzx	ecx,[tbox.length]
	sub	cl,[tb.sel.x]
	lea	eax,[tbox.text]
	movzx	esi,[tb.pos.x]
	movzx	edi,[tb.sel.x]
	add	esi,eax
	add	edi,eax
	mov	eax,esi
	sub	eax,edi
	sub	[tbox.length],al
	cld
	rep	movsb

  .exit:
	mov	al,[tb.sel.x]
	mov	[tbox.pos.x],al
	mov	[tbox.sel.x],al
	popad
	clc
	ret

  .exit.2:
	stc
	ret
endp

proc textbox.draw ; TBOX* ebp
	call	textbox.get_width
	movzx	ebx,[tbox.pos.x]
	sub	bl,[tbox.ofs.x]
	sub	ebx,eax
	jle	@f
	mov	bl,[tbox.pos.x]
	sub	bl,al
	mov	[tbox.ofs.x],bl
    @@:
	mov	[tb.sel.selected],0
	mov	al,[tbox.pos.x]
	mov	ah,[tbox.sel.x]
	cmp	al,ah
	je	@f
	inc	[tb.sel.selected]
	ja	@f
	xchg	al,ah
    @@: mov	[tb.pos.x],al
	mov	[tb.sel.x],ah

	mcall	13,dword[tbox.width],dword[tbox.height],[color_tbl.back]
	mov	edx,[cl_3d_inset]
	call	draw_framerect

	call	textbox.get_width
	mov	esi,eax
	mov	edi,eax

	cmp	ebp,[focused_tb]
	je	@f
	mov	ebx,dword[tbox.x-2]
	mov	bx,[tbox.y]
	movzx	eax,[tbox.height]
	shr	eax,1
	add	eax,4*65536-4
	add	ebx,eax
	lea	edx,[tbox.text]
	mcall	4,,[color_tbl.text]
	ret

    @@: movzx	eax,[tb.pos.x]
	cmp	al,[tb.sel.x]
	je	@f
	sub	al,[tbox.ofs.x]
	jz	@f
	movzx	ebx,[tb.sel.x]
	sub	bl,[tbox.ofs.x]
	jge	.lp1
	xor	ebx,ebx
  .lp1: push	eax ebx
	sub	eax,ebx
	mov	ecx,edi
	sub	ecx,ebx
	cmp	eax,ecx
	jbe	.lp2
	mov	eax,ecx
  .lp2: imul	eax,6
	imul	ebx,6
	movzx	ecx,[tbox.x]
	add	ecx,3
	add	ebx,ecx
	shl	ebx,16
	add	ebx,eax
	movzx	ecx,[tbox.height]
	shr	ecx,1
	add	cx,[tbox.y]
	shl	ecx,16
	add	ecx,-5*65536+10
	mcall	13,,,[color_tbl.back.sel]

	mov	esi,[esp]
	lea	edx,[tbox.text]
	movzx	eax,[tbox.ofs.x]
	add	edx,eax
	mov	ebx,dword[tbox.x-2]
	mov	bx,[tbox.y]
	movzx	eax,[tbox.height]
	shr	eax,1
	add	eax,4*65536-4
	add	ebx,eax
	mov	eax,4
	or	esi,esi
	jz	.lp3
	mcall	,,[color_tbl.text]
  .lp3: sub	edi,esi
	jnz	.lp4
	add	esp,8
	jmp	.exit
  .lp4:
	add	edx,esi
	imul	esi,6*65536
	add	ebx,esi
	pop	ecx esi
	sub	esi,ecx
	cmp	esi,edi
	jbe	.lp5
	mov	esi,edi
  .lp5:
	mcall	,,[color_tbl.text.sel]
	sub	edi,esi
	jz	.exit
	add	edx,esi
	imul	esi,6*65536
	add	ebx,esi
	lea	ecx,[tbox.text]
	mcall	,,[color_tbl.text],,edi
	jmp	.exit

    @@: lea	edx,[tbox.text]
	movzx	eax,[tbox.ofs.x]
	add	edx,eax
	mov	ebx,dword[tbox.x-2]
	mov	bx,[tbox.y]
	movzx	eax,[tbox.height]
	shr	eax,1
	add	eax,4*65536-4
	add	ebx,eax
	movzx	eax,[tbox.ofs.x]
	call	textbox.get_width
	mov	esi,eax
	mcall	4,,[color_tbl.text]

  .exit:
	movzx	ebx,[tbox.pos.x]
	movzx	eax,[tbox.ofs.x]
	sub	ebx,eax
	imul	ebx,6
	movzx	eax,[tbox.x]
	add	eax,3
	add	ebx,eax
	push	bx
	shl	ebx,16
	pop	bx
	movzx	ecx,[tbox.height]
	shr	ecx,1
	add	cx,[tbox.y]
	push	cx
	shl	ecx,16
	pop	cx
	add	ecx,-5*65536+4
	mcall	38,,,0x01000000
	add	ebx,0x00010001
	mcall
	ret
endp

proc textbox.key
	mov	ebp,[focused_tb]
	mov	esi,accel_table_textbox
  .acc: cmp	ebx,[esi]
	jne	@f
	call	dword[esi+4]
	jmp	.exit.2
    @@: add	esi,8
	cmp	byte[esi],0
	jne	.acc

	test	byte[shi+2],0x06
	jnz	.exit
	cmp	[tbox.length],255
	je	.exit

	movzx	eax,[chr]
	movzx	eax,[eax+key0]
	or	al,al
	jz	.exit
	mov	al,[eax+key1]
	cmp	[tb_casesen],0
	je	@f
	cmp	al,$60
	jle	@f
	add	al,[add_table-$60+eax]
    @@:
	call	textbox.delete_selection

	mov	ecx,255
	sub	cl,[tbox.pos.x]
	lea	edi,[tbox.text]
	add	edi,255
	lea	esi,[edi-1]
	std
	rep	movsb
	stosb
	cld
	inc	[tbox.length]
	call	key.tb.right

  .exit.2:
	call	textbox.draw
  .exit:
	ret
endp

textbox.mouse:
	ret

proc key.tb.bkspace
	call	textbox.delete_selection
	jnc	@f

	cmp	[tbox.pos.x],0
	je	@f
	call	key.tb.left
	jmp	key.tb.del.direct

    @@: ret
endp

proc key.tb.home
	xor	al,al
	mov	[tbox.pos.x],al
	mov	[tbox.sel.x],al
	mov	[tbox.ofs.x],al
	ret
endp

proc key.tb.left
	mov	al,[tbox.pos.x]
	mov	[tbox.sel.x],al
	dec	al
	js	@f
	mov	[tbox.pos.x],al
	mov	[tbox.sel.x],al
	cmp	[tbox.ofs.x],al
	jl	@f
	sub	[tbox.ofs.x],8
	jge	@f
	mov	[tbox.ofs.x],0
    @@: ret
endp

proc key.tb.right
	call	textbox.get_width
	mov	bl,[tbox.pos.x]
	mov	[tbox.sel.x],bl
	inc	bl
	cmp	bl,[tbox.length]
	ja	@f
	mov	[tbox.pos.x],bl
	mov	[tbox.sel.x],bl
	sub	bl,[tbox.ofs.x]
	cmp	bl,al
	jbe	@f
	inc	[tbox.ofs.x]
    @@: ret
endp

proc key.tb.end
	call	textbox.get_width
	movzx	ebx,[tbox.length]
	mov	[tbox.pos.x],bl
	mov	[tbox.sel.x],bl
	sub	ebx,eax
	jge	@f
	xor	bl,bl
    @@: mov	[tbox.ofs.x],bl
	ret
endp

proc key.tb.del
	call	textbox.delete_selection
	jnc	@f
  .direct:
	movzx	ecx,[tbox.length]
	sub	cl,[tbox.pos.x]
	jz	@f
	lea	eax,[tbox.text]
	movzx	edi,[tbox.pos.x]
	add	edi,eax
	lea	esi,[edi+1]
	dec	[tbox.length]
	cld
	rep	movsb

    @@: ret
endp

proc key.tb.shift_home
	xor	al,al
	mov	[tbox.pos.x],al
	mov	[tbox.ofs.x],al
	ret
endp

proc key.tb.shift_left
	mov	al,[tbox.pos.x]
	dec	al
	js	@f
	mov	[tbox.pos.x],al
	cmp	[tbox.ofs.x],al
	jl	@f
	sub	[tbox.ofs.x],8
	jge	@f
	mov	[tbox.ofs.x],0
    @@: ret
endp

proc key.tb.shift_right
	call	textbox.get_width
	mov	bl,[tbox.pos.x]
	inc	bl
	cmp	bl,[tbox.length]
	ja	@f
	mov	[tbox.pos.x],bl
	sub	bl,[tbox.ofs.x]
	cmp	bl,al
	jbe	@f
	inc	[tbox.ofs.x]
    @@: ret
endp

proc key.tb.shift_end
	call	textbox.get_width
	movzx	ebx,[tbox.length]
	mov	[tbox.pos.x],bl
	sub	ebx,eax
	jge	@f
	xor	bl,bl
    @@: mov	[tbox.ofs.x],bl
	ret
endp
