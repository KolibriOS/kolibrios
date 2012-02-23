.pgm:
	stdcall	img.create, [width], [height], Image.bpp8
	test	eax, eax
	jz	.quit
	mov	[retvalue], eax
	mov	ebx, eax

	mov	edi, [ebx+Image.Palette]
	mov	eax, 0xff000000
    @@:
	stosd
	add	eax, 0x00010101
	jnc	@b

	mov	edi, [ebx+Image.Data]
	mov	ecx, [ebx+Image.Width]
	imul	ecx, [ebx+Image.Height]

	cmp	[data_type], PNM_ASCII
	je	.pgm.ascii

  .pgm.raw:
	cmp	[maxval], 0xff
	jne	.pgm.raw.scale
	rep	movsb
	jmp	.quit

  .pgm.raw.scale:
	mov	edx, [maxval]
	mov	eax, 0
    @@:
	lodsb
	mov	ebx, eax
	shl	eax, 8
	sub	eax, ebx
	div	dl
	stosb
	dec	ecx
	jnz	@b
	jmp	.quit

  .pgm.ascii:
	xor	eax, eax
	cmp	[maxval], 0xff
	jne	.pgm.ascii.scale
  .pgm.ascii.next_char:
	lodsb
	cmp	al, ' '
	jna	.pgm.ascii.next_char
	call	pnm._.get_number
	mov	eax, ebx
	stosb
	dec	ecx
	jnz	.pgm.ascii.next_char
	jmp	.quit

  .pgm.ascii.scale:
	mov	edx, [maxval]
  .pgm.ascii.scale.next_char:
	lodsb
	cmp	al, ' '
	jna	.pgm.ascii.scale.next_char
	call	pnm._.get_number
	mov	eax, ebx
	shl	eax, 8
	sub	eax, ebx
	div	dl
	stosb
	dec	ecx
	jnz	.pgm.ascii.scale.next_char
	jmp	.quit

