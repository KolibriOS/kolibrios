macro crash.md4.f b, c, d
{
	push	c
	xor	c, d
	and	b, c
	xor	b, d
	pop	c
}

macro crash.md4.g b, c, d
{
	push	c  d
	mov	edi, b
	and	b, c
	and	c, d
	and	d, edi
	or	b, c
	or	b, d
	pop	d  c
}

macro crash.md4.h b, c, d
{
	xor	b, c
	xor	b, d
}

macro crash.md4.round func, a, b, c, d, index, shift, ac
{
	push	b
	func	b, c, d
	lea	a, [a + b + ac]
	add	a, [esi + index*4]
	rol	a, shift
	pop	b
}


proc crash.md4 _md4, _data, _len, _callback, _msglen
locals
	final	rd 1
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
	mov	edi, [_md4]
	mov	eax, [edi + 0x0]
	mov	ebx, [edi + 0x4]
	mov	ecx, [edi + 0x8]
	mov	edx, [edi + 0xc]

	crash.md4.round		crash.md4.f, eax, ebx, ecx, edx,  0,  3, 0x00000000
	crash.md4.round		crash.md4.f, edx, eax, ebx, ecx,  1,  7, 0x00000000
	crash.md4.round		crash.md4.f, ecx, edx, eax, ebx,  2, 11, 0x00000000
	crash.md4.round		crash.md4.f, ebx, ecx, edx, eax,  3, 19, 0x00000000
	crash.md4.round		crash.md4.f, eax, ebx, ecx, edx,  4,  3, 0x00000000
	crash.md4.round		crash.md4.f, edx, eax, ebx, ecx,  5,  7, 0x00000000
	crash.md4.round		crash.md4.f, ecx, edx, eax, ebx,  6, 11, 0x00000000
	crash.md4.round		crash.md4.f, ebx, ecx, edx, eax,  7, 19, 0x00000000
	crash.md4.round		crash.md4.f, eax, ebx, ecx, edx,  8,  3, 0x00000000
	crash.md4.round		crash.md4.f, edx, eax, ebx, ecx,  9,  7, 0x00000000
	crash.md4.round		crash.md4.f, ecx, edx, eax, ebx, 10, 11, 0x00000000
	crash.md4.round		crash.md4.f, ebx, ecx, edx, eax, 11, 19, 0x00000000
	crash.md4.round		crash.md4.f, eax, ebx, ecx, edx, 12,  3, 0x00000000
	crash.md4.round		crash.md4.f, edx, eax, ebx, ecx, 13,  7, 0x00000000
	crash.md4.round		crash.md4.f, ecx, edx, eax, ebx, 14, 11, 0x00000000
	crash.md4.round		crash.md4.f, ebx, ecx, edx, eax, 15, 19, 0x00000000

	crash.md4.round		crash.md4.g, eax, ebx, ecx, edx,  0,  3, 0x5a827999
	crash.md4.round		crash.md4.g, edx, eax, ebx, ecx,  4,  5, 0x5a827999
	crash.md4.round		crash.md4.g, ecx, edx, eax, ebx,  8,  9, 0x5a827999
	crash.md4.round		crash.md4.g, ebx, ecx, edx, eax, 12, 13, 0x5a827999
	crash.md4.round		crash.md4.g, eax, ebx, ecx, edx,  1,  3, 0x5a827999
	crash.md4.round		crash.md4.g, edx, eax, ebx, ecx,  5,  5, 0x5a827999
	crash.md4.round		crash.md4.g, ecx, edx, eax, ebx,  9,  9, 0x5a827999
	crash.md4.round		crash.md4.g, ebx, ecx, edx, eax, 13, 13, 0x5a827999
	crash.md4.round		crash.md4.g, eax, ebx, ecx, edx,  2,  3, 0x5a827999
	crash.md4.round		crash.md4.g, edx, eax, ebx, ecx,  6,  5, 0x5a827999
	crash.md4.round		crash.md4.g, ecx, edx, eax, ebx, 10,  9, 0x5a827999
	crash.md4.round		crash.md4.g, ebx, ecx, edx, eax, 14, 13, 0x5a827999
	crash.md4.round		crash.md4.g, eax, ebx, ecx, edx,  3,  3, 0x5a827999
	crash.md4.round		crash.md4.g, edx, eax, ebx, ecx,  7,  5, 0x5a827999
	crash.md4.round		crash.md4.g, ecx, edx, eax, ebx, 11,  9, 0x5a827999
	crash.md4.round		crash.md4.g, ebx, ecx, edx, eax, 15, 13, 0x5a827999

	crash.md4.round		crash.md4.h, eax, ebx, ecx, edx,  0,  3, 0x6ed9eba1
	crash.md4.round		crash.md4.h, edx, eax, ebx, ecx,  8,  9, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ecx, edx, eax, ebx,  4, 11, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ebx, ecx, edx, eax, 12, 15, 0x6ed9eba1
	crash.md4.round		crash.md4.h, eax, ebx, ecx, edx,  2,  3, 0x6ed9eba1
	crash.md4.round		crash.md4.h, edx, eax, ebx, ecx, 10,  9, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ecx, edx, eax, ebx,  6, 11, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ebx, ecx, edx, eax, 14, 15, 0x6ed9eba1
	crash.md4.round		crash.md4.h, eax, ebx, ecx, edx,  1,  3, 0x6ed9eba1
	crash.md4.round		crash.md4.h, edx, eax, ebx, ecx,  9,  9, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ecx, edx, eax, ebx,  5, 11, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ebx, ecx, edx, eax, 13, 15, 0x6ed9eba1
	crash.md4.round		crash.md4.h, eax, ebx, ecx, edx,  3,  3, 0x6ed9eba1
	crash.md4.round		crash.md4.h, edx, eax, ebx, ecx, 11,  9, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ecx, edx, eax, ebx,  7, 11, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ebx, ecx, edx, eax, 15, 15, 0x6ed9eba1

	mov	edi, [_md4]
	add	[edi + 0x0], eax
	add	[edi + 0x4], ebx
	add	[edi + 0x8], ecx
	add	[edi + 0xc], edx
	add	esi, 64
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
	mov	dword[edi], eax
	mov	dword[edi + 4], edx
	add	[_len], 8
	mov	[final], 1
	jmp	.first
  .quit:
	ret
endp

