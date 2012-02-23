.pbm:
	stdcall	img.create, [width], [height], Image.bpp1
	test	eax, eax
	jz	.quit
	mov	[retvalue], eax
	mov	ebx, eax

	mov	edi, [ebx+Image.Palette]
	mov	[edi], dword 0x00ffffff
	mov	[edi + 4], dword 0x00000000

	cmp	[data_type], PNM_ASCII
	je	.pbm.ascii

  .pbm.raw:
	mov	ecx, [ebx+Image.Width]
	add	ecx, 7
	shr	ecx, 3
	imul	ecx, [ebx+Image.Height]
	mov	edi, [ebx+Image.Data]
	rep	movsb
	jmp	.quit

  .pbm.ascii:
	mov	edi, [ebx+Image.Data]
  .pbm.next_line:
	mov	edx, [width]
	mov	ecx, 7
	xor	eax, eax
  .pbm.next_char:
	lodsb
	cmp	al, ' '
	jna	.pbm.next_char
  .pbm.get_number:
	cmp	al, '1'
	sete	bl
	shl	bl, cl
	or	ah, bl
	dec	ecx
	jns	@f
	shr	eax, 8
	stosb
	mov	ecx, 7
    @@:
	dec	edx
	jnz	.pbm.next_char
	test	byte[width], 0x07
	jz	@f
	shr	eax, 8
	stosb
    @@:
	dec	[height]
	jnz	.pbm.next_line
	jmp	.quit
