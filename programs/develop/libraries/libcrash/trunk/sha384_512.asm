macro chn x, y, z
{
	movq	mm0, [y]
	pxor	mm0, [z]
	pand	mm0, [x]
	pxor	mm0, [z]
}

macro maj x, y, z
{
	movq	mm0, [x]
	pxor	mm0, [y]
	pand	mm0, [z]
	movq	mm2, [x]
	pand	mm2, [y]
	pxor	mm0, mm2
}

macro Sigma0 x
{
	movq	mm0, x
	movq	mm2, mm0
	movq	mm7, mm2
	psrlq	mm2, 28
	psllq	mm7, 36
	por	mm2, mm7
	movq	mm7, mm0
	psrlq	mm0, 34
	psllq	mm7, 30
	por	mm0, mm7
	pxor	mm0, mm2
	movq	mm2, x
	movq	mm7, mm2
	psrlq	mm2, 39
	psllq	mm7, 25
	por	mm2, mm7
	pxor	mm0, mm2
}

macro Sigma1 x
{
	movq	mm0, x
	movq	mm2, mm0
	movq	mm7, mm2
	psrlq	mm2, 14
	psllq	mm7, 50
	por	mm2, mm7
	movq	mm7, mm0
	psrlq	mm0, 18
	psllq	mm7, 46
	por	mm0, mm7
	pxor	mm0, mm2
	movq	mm2, x
	movq	mm7, mm2
	psrlq	mm2, 41
	psllq	mm7, 23
	por	mm2, mm7
	pxor	mm0, mm2
}

macro sigma0 x
{
	movq	mm0, x
	movq	mm2, mm0
	movq	mm7, mm2
	psrlq	mm2, 1
	psllq	mm7, 63
	por	mm2, mm7
	movq	mm7, mm0
	psrlq	mm0, 8
	psllq	mm7, 56
	por	mm0, mm7
	pxor	mm0, mm2
	movq	mm2, x
	psrlq	mm2, 7
	pxor	mm0, mm2
}

macro sigma1 x
{
	movq	mm0, x
	movq	mm2, mm0
	movq	mm7, mm2
	psrlq	mm2, 19
	psllq	mm7, 45
	por	mm2, mm7
	movq	mm7, mm0
	psrlq	mm0, 61
	psllq	mm7, 3
	por	mm0, mm7
	pxor	mm0, mm2
	movq	mm2, x
	psrlq	mm2, 6
	pxor	mm0, mm2
}

macro recalculate_w n
{
	movq	mm3, [w + ((n-2) and 15)*8]
	sigma1	mm3
	paddq	mm0, [w + ((n-7) and 15)*8]
	movq	mm6, mm0
	movq	mm3, [w + ((n-15) and 15)*8]
	sigma0	mm3
	movq	mm2, mm6
	paddq	mm0, mm2
	movq	mm7, [w + (n)*8]
	paddq	mm7, mm0
	movq	[w + (n)*8], mm7
}

macro crash.sha512.round a, b, c, d, e, f, g, h, k
{
	movq	mm1, [h]
	movq	mm3, [e]
	Sigma1	mm3
	paddq	mm1, mm0
	chn	e, f, g
	paddq	mm1, mm0
	paddq	mm1, [k]
	paddq	mm1, mm5
	movq	mm7, [d]
	paddq	mm7, mm1
	movq	[d], mm7
	movq	mm3, [a]
	Sigma0	mm3
	paddq	mm1, mm0
	maj	a, b, c
	paddq	mm0, mm1
	movq	[h], mm0
}


macro crash.sha512.round_1_16 a, b, c, d, e, f, g, h, n
{

	movq	mm0, [esi + (n)*8]
	movq	[temp], mm0
	mov	eax, dword[temp]
	bswap	eax
	push	eax
	mov	eax, dword[temp + 4]
	bswap	eax
	mov	dword[temp], eax
	pop	eax
	mov	dword[temp + 4], eax
	movq	mm0, [temp]
	movq	[w + (n)*8], mm0
	movq	mm5, mm0
	crash.sha512.round a, b, c, d, e, f, g, h, (crash._.sha512_table + (n)*8)
}

macro crash.sha512.round_17_64 a, b, c, d, e, f, g, h, n, rep_num
{
	recalculate_w n
	movq	mm5, [w + (n)*8]
	crash.sha512.round a, b, c, d, e, f, g, h, (crash._.sha512_table + (n+16*rep_num)*8)
}


proc crash.sha512 _sha512, _data, _len, _callback, _msglen
locals
	final	rd 1
	w	rq 80
	A	rq 1
	B	rq 1
	C	rq 1
	D	rq 1
	E	rq 1
	F	rq 1
	G	rq 1
	H	rq 1
	temp	rq 1
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
	sub	[_len], 128
	jnc	@f
	add	[_len], 128
	jmp	.endofblock
    @@:
	mov	edi, [_sha512]
	movq	mm0, [edi + 0x00]
	movq	[A], mm0
	movq	mm0, [edi + 0x08]
	movq	[B], mm0
	movq	mm0, [edi + 0x10]
	movq	[C], mm0
	movq	mm0, [edi + 0x18]
	movq	[D], mm0
	movq	mm0, [edi + 0x20]
	movq	[E], mm0
	movq	mm0, [edi + 0x28]
	movq	[F], mm0
	movq	mm0, [edi + 0x30]
	movq	[G], mm0
	movq	mm0, [edi + 0x38]
	movq	[H], mm0


	crash.sha512.round_1_16		A, B, C, D, E, F, G, H,  0
	crash.sha512.round_1_16		H, A, B, C, D, E, F, G,  1
	crash.sha512.round_1_16		G, H, A, B, C, D, E, F,  2
	crash.sha512.round_1_16		F, G, H, A, B, C, D, E,  3
	crash.sha512.round_1_16		E, F, G, H, A, B, C, D,  4
	crash.sha512.round_1_16		D, E, F, G, H, A, B, C,  5
	crash.sha512.round_1_16		C, D, E, F, G, H, A, B,  6
	crash.sha512.round_1_16		B, C, D, E, F, G, H, A,  7
	crash.sha512.round_1_16		A, B, C, D, E, F, G, H,  8
	crash.sha512.round_1_16		H, A, B, C, D, E, F, G,  9
	crash.sha512.round_1_16		G, H, A, B, C, D, E, F, 10
	crash.sha512.round_1_16		F, G, H, A, B, C, D, E, 11
	crash.sha512.round_1_16		E, F, G, H, A, B, C, D, 12
	crash.sha512.round_1_16		D, E, F, G, H, A, B, C, 13
	crash.sha512.round_1_16		C, D, E, F, G, H, A, B, 14
	crash.sha512.round_1_16		B, C, D, E, F, G, H, A, 15

repeat 4
	crash.sha512.round_17_64	A, B, C, D, E, F, G, H,  0, %
	crash.sha512.round_17_64	H, A, B, C, D, E, F, G,  1, %
	crash.sha512.round_17_64	G, H, A, B, C, D, E, F,  2, %
	crash.sha512.round_17_64	F, G, H, A, B, C, D, E,  3, %
	crash.sha512.round_17_64	E, F, G, H, A, B, C, D,  4, %
	crash.sha512.round_17_64	D, E, F, G, H, A, B, C,  5, %
	crash.sha512.round_17_64	C, D, E, F, G, H, A, B,  6, %
	crash.sha512.round_17_64	B, C, D, E, F, G, H, A,  7, %
	crash.sha512.round_17_64	A, B, C, D, E, F, G, H,  8, %
	crash.sha512.round_17_64	H, A, B, C, D, E, F, G,  9, %
	crash.sha512.round_17_64	G, H, A, B, C, D, E, F, 10, %
	crash.sha512.round_17_64	F, G, H, A, B, C, D, E, 11, %
	crash.sha512.round_17_64	E, F, G, H, A, B, C, D, 12, %
	crash.sha512.round_17_64	D, E, F, G, H, A, B, C, 13, %
	crash.sha512.round_17_64	C, D, E, F, G, H, A, B, 14, %
	crash.sha512.round_17_64	B, C, D, E, F, G, H, A, 15, %
end repeat


	mov	edi, [_sha512]
	movq	mm0, [A]
	paddq	mm0, [edi + 0x00]
	movq	[edi + 0x00], mm0
	movq	mm0, [B]
	paddq	mm0, [edi + 0x08]
	movq	[edi + 0x08], mm0
	movq	mm0, [C]
	paddq	mm0, [edi + 0x10]
	movq	[edi + 0x10], mm0
	movq	mm0, [D]
	paddq	mm0, [edi + 0x18]
	movq	[edi + 0x18], mm0
	movq	mm0, [E]
	paddq	mm0, [edi + 0x20]
	movq	[edi + 0x20], mm0
	movq	mm0, [F]
	paddq	mm0, [edi + 0x28]
	movq	[edi + 0x28], mm0
	movq	mm0, [G]
	paddq	mm0, [edi + 0x30]
	movq	[edi + 0x30], mm0
	movq	mm0, [H]
	paddq	mm0, [edi + 0x38]
	movq	[edi + 0x38], mm0
	add	esi, 128
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
	and	eax, 127
	mov	ecx, 112
	sub	ecx, eax
	ja	@f
	add	ecx, 128
    @@:
	add	[_len], ecx
	mov	byte[edi], 0x80
	add	edi, 1
	sub	ecx, 1
	mov	al, 0
	rep	stosb
	xor	eax, eax
	stosd
	stosd
	mov	eax, [_msglen]
	mov	eax, [eax]
	mov	edx, 8
	mul	edx
	bswap	eax
	bswap	edx
	mov	dword[edi], edx
	mov	dword[edi + 4], eax
	add	[_len], 16
	mov	[final], 1
	jmp	.first
  .quit:
	mov	esi, [_sha512]
	mov	edi, esi
	mov	ecx, 8
    @@:
	lodsd
	bswap	eax
	mov	ebx, eax
	lodsd
	bswap	eax
	stosd
	mov	eax, ebx
	stosd
	sub	ecx, 1
	jnz	@b
	emms
	ret
endp

