macro crash.md5.f b, c, d
{
	push	c
	xor	c, d
	and	b, c
	xor	b, d
	pop	c
}

macro crash.md5.g b, c, d
{
	push	c  d
	and	b, d
	not	d
	and	c, d
	or	b, c
	pop	d  c
}

macro crash.md5.h b, c, d
{
	xor	b, c
	xor	b, d
}

macro crash.md5.i b, c, d
{
	push	d
	not	d
	or	b, d
	xor	b, c
	pop	d
}

macro crash.md5.round func, a, b, c, d, index, shift, ac
{
	push	b
	func	b, c, d
	lea	a, [a + b + ac]
	add	a, [esi + index*4]
	rol	a, shift
	pop	b
	add	a, b
}


proc crash.md5 _md5, _data, _len, _callback, _msglen
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
	mov	edi, [_md5]
	mov	eax, [edi + 0x0]
	mov	ebx, [edi + 0x4]
	mov	ecx, [edi + 0x8]
	mov	edx, [edi + 0xc]

	crash.md5.round		crash.md5.f, eax, ebx, ecx, edx,  0,  7, 0xd76aa478
	crash.md5.round		crash.md5.f, edx, eax, ebx, ecx,  1, 12, 0xe8c7b756
	crash.md5.round		crash.md5.f, ecx, edx, eax, ebx,  2, 17, 0x242070db
	crash.md5.round		crash.md5.f, ebx, ecx, edx, eax,  3, 22, 0xc1bdceee
	crash.md5.round		crash.md5.f, eax, ebx, ecx, edx,  4,  7, 0xf57c0faf
	crash.md5.round		crash.md5.f, edx, eax, ebx, ecx,  5, 12, 0x4787c62a
	crash.md5.round		crash.md5.f, ecx, edx, eax, ebx,  6, 17, 0xa8304613
	crash.md5.round		crash.md5.f, ebx, ecx, edx, eax,  7, 22, 0xfd469501
	crash.md5.round		crash.md5.f, eax, ebx, ecx, edx,  8,  7, 0x698098d8
	crash.md5.round		crash.md5.f, edx, eax, ebx, ecx,  9, 12, 0x8b44f7af
	crash.md5.round		crash.md5.f, ecx, edx, eax, ebx, 10, 17, 0xffff5bb1
	crash.md5.round		crash.md5.f, ebx, ecx, edx, eax, 11, 22, 0x895cd7be
	crash.md5.round		crash.md5.f, eax, ebx, ecx, edx, 12,  7, 0x6b901122
	crash.md5.round		crash.md5.f, edx, eax, ebx, ecx, 13, 12, 0xfd987193
	crash.md5.round		crash.md5.f, ecx, edx, eax, ebx, 14, 17, 0xa679438e
	crash.md5.round		crash.md5.f, ebx, ecx, edx, eax, 15, 22, 0x49b40821

	crash.md5.round		crash.md5.g, eax, ebx, ecx, edx,  1,  5, 0xf61e2562
	crash.md5.round		crash.md5.g, edx, eax, ebx, ecx,  6,  9, 0xc040b340
	crash.md5.round		crash.md5.g, ecx, edx, eax, ebx, 11, 14, 0x265e5a51
	crash.md5.round		crash.md5.g, ebx, ecx, edx, eax,  0, 20, 0xe9b6c7aa
	crash.md5.round		crash.md5.g, eax, ebx, ecx, edx,  5,  5, 0xd62f105d
	crash.md5.round		crash.md5.g, edx, eax, ebx, ecx, 10,  9, 0x02441453
	crash.md5.round		crash.md5.g, ecx, edx, eax, ebx, 15, 14, 0xd8a1e681
	crash.md5.round		crash.md5.g, ebx, ecx, edx, eax,  4, 20, 0xe7d3fbc8
	crash.md5.round		crash.md5.g, eax, ebx, ecx, edx,  9,  5, 0x21e1cde6
	crash.md5.round		crash.md5.g, edx, eax, ebx, ecx, 14,  9, 0xc33707d6
	crash.md5.round		crash.md5.g, ecx, edx, eax, ebx,  3, 14, 0xf4d50d87
	crash.md5.round		crash.md5.g, ebx, ecx, edx, eax,  8, 20, 0x455a14ed
	crash.md5.round		crash.md5.g, eax, ebx, ecx, edx, 13,  5, 0xa9e3e905
	crash.md5.round		crash.md5.g, edx, eax, ebx, ecx,  2,  9, 0xfcefa3f8
	crash.md5.round		crash.md5.g, ecx, edx, eax, ebx,  7, 14, 0x676f02d9
	crash.md5.round		crash.md5.g, ebx, ecx, edx, eax, 12, 20, 0x8d2a4c8a

	crash.md5.round		crash.md5.h, eax, ebx, ecx, edx,  5,  4, 0xfffa3942
	crash.md5.round		crash.md5.h, edx, eax, ebx, ecx,  8, 11, 0x8771f681
	crash.md5.round		crash.md5.h, ecx, edx, eax, ebx, 11, 16, 0x6d9d6122
	crash.md5.round		crash.md5.h, ebx, ecx, edx, eax, 14, 23, 0xfde5380c
	crash.md5.round		crash.md5.h, eax, ebx, ecx, edx,  1,  4, 0xa4beea44
	crash.md5.round		crash.md5.h, edx, eax, ebx, ecx,  4, 11, 0x4bdecfa9
	crash.md5.round		crash.md5.h, ecx, edx, eax, ebx,  7, 16, 0xf6bb4b60
	crash.md5.round		crash.md5.h, ebx, ecx, edx, eax, 10, 23, 0xbebfbc70
	crash.md5.round		crash.md5.h, eax, ebx, ecx, edx, 13,  4, 0x289b7ec6
	crash.md5.round		crash.md5.h, edx, eax, ebx, ecx,  0, 11, 0xeaa127fa
	crash.md5.round		crash.md5.h, ecx, edx, eax, ebx,  3, 16, 0xd4ef3085
	crash.md5.round		crash.md5.h, ebx, ecx, edx, eax,  6, 23, 0x04881d05
	crash.md5.round		crash.md5.h, eax, ebx, ecx, edx,  9,  4, 0xd9d4d039
	crash.md5.round		crash.md5.h, edx, eax, ebx, ecx, 12, 11, 0xe6db99e5
	crash.md5.round		crash.md5.h, ecx, edx, eax, ebx, 15, 16, 0x1fa27cf8
	crash.md5.round		crash.md5.h, ebx, ecx, edx, eax,  2, 23, 0xc4ac5665

	crash.md5.round		crash.md5.i, eax, ebx, ecx, edx,  0,  6, 0xf4292244
	crash.md5.round		crash.md5.i, edx, eax, ebx, ecx,  7, 10, 0x432aff97
	crash.md5.round		crash.md5.i, ecx, edx, eax, ebx, 14, 15, 0xab9423a7
	crash.md5.round		crash.md5.i, ebx, ecx, edx, eax,  5, 21, 0xfc93a039
	crash.md5.round		crash.md5.i, eax, ebx, ecx, edx, 12,  6, 0x655b59c3
	crash.md5.round		crash.md5.i, edx, eax, ebx, ecx,  3, 10, 0x8f0ccc92
	crash.md5.round		crash.md5.i, ecx, edx, eax, ebx, 10, 15, 0xffeff47d
	crash.md5.round		crash.md5.i, ebx, ecx, edx, eax,  1, 21, 0x85845dd1
	crash.md5.round		crash.md5.i, eax, ebx, ecx, edx,  8,  6, 0x6fa87e4f
	crash.md5.round		crash.md5.i, edx, eax, ebx, ecx, 15, 10, 0xfe2ce6e0
	crash.md5.round		crash.md5.i, ecx, edx, eax, ebx,  6, 15, 0xa3014314
	crash.md5.round		crash.md5.i, ebx, ecx, edx, eax, 13, 21, 0x4e0811a1
	crash.md5.round		crash.md5.i, eax, ebx, ecx, edx,  4,  6, 0xf7537e82
	crash.md5.round		crash.md5.i, edx, eax, ebx, ecx, 11, 10, 0xbd3af235
	crash.md5.round		crash.md5.i, ecx, edx, eax, ebx,  2, 15, 0x2ad7d2bb
	crash.md5.round		crash.md5.i, ebx, ecx, edx, eax,  9, 21, 0xeb86d391

	mov	edi, [_md5]
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

