.ppm:
	stdcall	img.create, [width], [height], Image.bpp24
	test	eax, eax
	jz	.quit
	mov	[retvalue], eax
	mov	ebx, eax

	mov	edi, [ebx + Image.Data]
	mov	ecx, [ebx + Image.Width]
	imul	ecx, [ebx + Image.Height]

	cmp	[data_type], PNM_ASCII
	je	.ppm.ascii

  .ppm.raw:
	cmp	[maxval], 0xff
	jne	.ppm.raw.scale
    @@:
	lodsw
	xchg	al, ah
	movsb
	stosw
	dec	ecx
	jnz	@b
	jmp	.quit
  .ppm.raw.scale:
	mov	edx, [maxval]
	xor	eax, eax
    @@:
	lodsb
	mov	ebx, eax
	shl	eax, 8
	sub	eax, ebx
	div	dl
	stosb
	lodsb
	mov	ebx, eax
	shl	eax, 8
	sub	eax, ebx
	div	dl
	stosb
	lodsb
	mov	ebx, eax
	shl	eax, 8
	sub	eax, ebx
	div	dl
	stosb
	dec	ecx
	jnz	@b
	jmp	.quit

  .ppm.ascii:
	xor	eax, eax
	cmp	[maxval], 0xff
	jne	.ppm.ascii.scale
  .ppm.ascii.next_char:
    @@:
	lodsb
	cmp	al, ' '
	jna	@b
	call	pnm._.get_number
	mov	[edi + 2], bl
    @@:
	lodsb
	cmp	al, ' '
	jna	@b
	call	pnm._.get_number
	mov	[edi + 1], bl
    @@:
	lodsb
	cmp	al, ' '
	jna	@b
	call	pnm._.get_number
	mov	[edi + 0], bl
	add	edi, 3
	dec	ecx
	jnz	.ppm.ascii.next_char
	jmp	.quit

  .ppm.ascii.scale:
	mov	edx, [maxval]
  .ppm.ascii.scale.next_char:
    @@:
	lodsb
	cmp	al, ' '
	jna	@b
	call	pnm._.get_number
	mov	eax, ebx
	shl	eax, 8
	sub	eax, ebx
	div	dl
	mov	[edi + 2], al
    @@:
	lodsb
	cmp	al, ' '
	jna	@b
	call	pnm._.get_number
	mov	eax, ebx
	shl	eax, 8
	sub	eax, ebx
	div	dl
	mov	[edi + 1], al
    @@:
	lodsb
	cmp	al, ' '
	jna	@b
	call	pnm._.get_number
	mov	eax, ebx
	shl	eax, 8
	sub	eax, ebx
	div	dl
	mov	[edi + 0], al
	add	edi, 3
	dec	ecx
	jnz	.ppm.ascii.next_char
	jmp	.quit

