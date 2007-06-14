symbol_dump:

	push	edi
	mov	edx,[memory_end]
      dump_symbols:
	cmp	edx,[labels_list]
	jbe	symbols_dumped
	sub	edx,LABEL_STRUCTURE_SIZE
	cmp	dword [edx+24],0
	je	dump_symbols	; do not dump anonymous symbols
	test	byte [edx+8],1
	jz	dump_symbols	; do not dump symbols that didn't get defined
	mov	ax,[current_pass]
	cmp	ax,[edx+16]
	jne	dump_symbols
	test	byte [edx+8],4 or 2
	jnz	dump_symbols	; do not dump assembly-time variables
				; do not dump variables defined with '='
	cmp	word [edx+12], 0
	jnz	dump_symbols	; do not dump register-based variables

	mov	al, '0'
	stosb
	mov	al, 'x'
	stosb
	mov	eax, [edx+4]
	mov	ecx, 8
@@:
	rol	eax, 4
	test	al, 0xF
	loopz	@b
	jz	.nohigh
	inc	ecx
@@:
	push	eax
	and	al, 0xF
	cmp	al, 10
	sbb	al, 69h
	das
	stosb
	pop	eax
	rol	eax, 4
	loop	@b
	mov	eax, [edx]
	mov	ecx, 8
	jmp	.low
.nohigh:
	mov	eax, [edx]
	mov	ecx, 8
@@:
	rol	eax, 4
	test	al, 0xF
	loopz	@b
	inc	ecx
.low:
	push	eax
	and	al, 0xF
	cmp	al, 10
	sbb	al, 69h
	das
	stosb
	pop	eax
	rol	eax, 4
	loop	.low

	mov	al, ' '
	stosb

	mov	esi,[edx+24]
	movzx	ecx,byte [esi-1]
	rep	movsb

	mov	ax,0A0Dh
	stosw

	jmp	dump_symbols

      symbols_dumped:
	mov	edx,dbgfilename
        push    esi edi
	mov     esi, outfile
	mov     edi, edx
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
	lea	ecx, [edi-1]
@@:
	dec	edi
	cmp	edi, edx
	jb	@f
	cmp	byte [edi], '/'
	jz	@f
	cmp	byte [edi], '.'
	jnz	@b
	mov	ecx, edi
@@:
	mov	dword [ecx], '.dbg'
	mov     byte [ecx+4], 0
	pop	edi esi
	call	create
	mov	edx,[esp]
	mov	ecx,edi
	sub	ecx,edx
	call	write
	call	close
	pop	edi

	ret
