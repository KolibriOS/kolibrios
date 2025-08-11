	pop	esi
	push	esi
loader_patch4:
	mov	ecx, 0		; will be patched: number of calltrick entries
ctrloop:
	lodsb
@@:
	cmp	al, 0xF
	jnz	.f
	lodsb
	cmp	al, 80h
	jb	@b
	cmp	al, 90h
	jb	@f
.f:
	sub	al, 0E8h
	cmp	al, 1
	ja	ctrloop
@@:
	cmp	byte [esi], 0	; will be patched: code in calltrick entries
loader_patch5:
	jnz	ctrloop
	lodsd
; "bswap eax" is not supported on i386
; mov al,0/bswap eax = 4 bytes, following instructions = 9 bytes
	shr	ax, 8
	ror	eax, 16
	xchg	al, ah
	sub	eax, esi
	add	eax, [esp]
	mov	[esi-4], eax
	loop	ctrloop
