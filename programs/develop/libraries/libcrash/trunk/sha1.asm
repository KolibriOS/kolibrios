proc crash.sha1.f
	push	ebx ecx edx
	xor	ecx, edx
	and	ebx, ecx
	xor	ebx, edx
	mov	esi, ebx
	pop	edx ecx ebx
	ret
endp

proc crash.sha1.g
	push	ebx ecx edx
	xor	ebx, ecx
	xor	ebx, edx
	mov	esi, ebx
	pop	edx ecx ebx
	ret
endp

proc crash.sha1.h
	push	ebx ecx edx
	mov	esi, ebx
	and	ebx, ecx
	and	ecx, edx
	and	esi, edx
	or	ebx, ecx
	or	esi, ebx
	pop	edx ecx ebx
	ret
endp


proc crash.sha1 _sha1, _data, _len, _callback, _msglen
locals
	final	rd 1
	temp	rd 1
	counter	rd 1
	summand	rd 1
	shafunc	rd 1
	w	rd 80
endl
	mov	[final], 0
  .first:
	mov	eax, [_msglen]
	mov	ecx, [_len]
	add	[eax], ecx
	mov	esi, [_data]
	test	ecx, ecx
	jz	.callback
  .begin:
	sub	[_len], 64
	jnc	@f
	add	[_len], 64
	jmp	.endofblock
    @@:
	lea	edi, [w]
	xor	ecx, ecx
    @@:
	mov	eax, [esi]
	add	esi, 4
	bswap	eax
	mov	[edi], eax
	add	edi, 4
	add	ecx, 1
	cmp	ecx, 16
	jne	@b
    @@:
	mov	eax, [w + (ecx -  3)*4]
	xor	eax, [w + (ecx -  8)*4]
	xor	eax, [w + (ecx - 14)*4]
	xor	eax, [w + (ecx - 16)*4]
	rol	eax, 1
	mov	[w + ecx*4], eax
	add	ecx, 1
	cmp	ecx, 80
	jne	@b

	mov	edi, [_sha1]
	mov	eax, [edi + 0x00]
	mov	ebx, [edi + 0x04]
	mov	ecx, [edi + 0x08]
	mov	edx, [edi + 0x0c]
	mov	edi, [edi + 0x10]

	push	esi

	mov	[counter], 0
	mov	[summand], 0x5a827999
	mov	[shafunc], crash.sha1.f
    @@:
	mov	esi, eax
	rol	esi, 5
	mov	[temp], esi
	call	[shafunc]

	add	esi, edi
	add	[temp], esi
	mov	esi, [counter]
	mov	esi, [w + esi*4]
	add	esi, [summand]
	add	[temp], esi

	mov	edi, edx
	mov	edx, ecx
	mov	ecx, ebx
	rol	ecx, 30
	mov	ebx, eax
	mov	eax, [temp]

	add	[counter], 1
	cmp	[counter], 20
	jne	@b

	mov	[summand], 0x6ed9eba1
	mov	[shafunc], crash.sha1.g
    @@:
	mov	esi, eax
	rol	esi, 5
	mov	[temp], esi
	call	dword[shafunc]

	add	esi, edi
	add	[temp], esi
	mov	esi, [counter]
	mov	esi, [w + esi*4]
	add	esi, [summand]
	add	[temp], esi

	mov	edi, edx
	mov	edx, ecx
	mov	ecx, ebx
	rol	ecx, 30
	mov	ebx, eax
	mov	eax, [temp]

	add	[counter], 1
	cmp	[counter], 40
	jne	@b

	mov	[summand], 0x8f1bbcdc
	mov	[shafunc], crash.sha1.h
    @@:
	mov	esi, eax
	rol	esi, 5
	mov	[temp], esi
	call	dword[shafunc]

	add	esi, edi
	add	[temp], esi
	mov	esi, [counter]
	mov	esi, [w + esi*4]
	add	esi, [summand]
	add	[temp], esi

	mov	edi, edx
	mov	edx, ecx
	mov	ecx, ebx
	rol	ecx, 30
	mov	ebx, eax
	mov	eax, [temp]

	add	[counter], 1
	cmp	[counter], 60
	jne	@b

	mov	[summand], 0xca62c1d6
	mov	[shafunc], crash.sha1.g
    @@:
	mov	esi, eax
	rol	esi, 5
	mov	[temp], esi
	call	dword[shafunc]

	add	esi, edi
	add	[temp], esi
	mov	esi, [counter]
	mov	esi, [w + esi*4]
	add	esi, [summand]
	add	[temp], esi

	mov	edi, edx
	mov	edx, ecx
	mov	ecx, ebx
	rol	ecx, 30
	mov	ebx, eax
	mov	eax, [temp]

	add	[counter], 1
	cmp	[counter], 80
	jne	@b

	pop	esi

	mov	[temp], edi
	mov	edi, [_sha1]
	add	[edi + 0x00], eax
	add	[edi + 0x04], ebx
	add	[edi + 0x08], ecx
	add	[edi + 0x0c], edx
	mov	eax, [temp]
	add	[edi + 0x10], eax
	jmp	.begin
  .endofblock:
	cmp	[final], 1
	je	.quit

  .callback:
	mov	eax, [_callback]
	test	eax, eax
	jz	@f
	call	eax
	test	eax, eax
	jz	@f
	mov	[_len], eax
	jmp	.first
    @@:

	mov	edi, [_data]
	mov	ecx, [_len]
	rep	movsb
	mov	eax, [_msglen]
	mov	eax, [eax]
	and	eax, 63
	mov	ecx, 56
	sub	ecx, eax
	ja	@f
	add	ecx, 64
    @@:
	add	[_len], ecx
	mov	byte[edi], 0x80
	add	edi, 1
	sub	ecx, 1
	mov	al, 0
	rep	stosb
	mov	eax, [_msglen]
	mov	eax, [eax]
	mov	edx, 8
	mul	edx
	bswap	eax
	bswap	edx
	mov	dword[edi], edx
	mov	dword[edi + 4], eax
	add	[_len], 8
	mov	[final], 1
	jmp	.first
  .quit:
	mov	esi, [_sha1]
	mov	edi, esi
	mov	ecx, 5
    @@:
	lodsd
	bswap	eax
	stosd
	sub	ecx, 1
	jnz	@b
	ret
endp

