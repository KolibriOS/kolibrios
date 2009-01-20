; in: ebx -> pattern (not empty zero-terminated string), esi -> translation table,
;	dword [esp+4] = encoding, byte [esp+8] non-zero for whole words only
; out: edx and ecx -> preprocessed data
; when search will be done, edx must be pgfree()d
search_string_pre:
; FSM is used, number of states is limited by 256, so pattern length must be <= 255
; anyway, for big patterns FSM uses too many memory, so probably it is not best choice
; get pattern length, m
	or	ecx, -1
@@:
	inc	ecx
	cmp	byte [ecx+ebx], 0
	jnz	@b
	cmp	byte [esp+8], 0
	jz	@f
	inc	ecx
	inc	ecx
@@:
	push	ecx
; allocate m*257 bytes for FSM and prefix function
	imul	ecx, 257
	call	xpgalloc
	pop	ecx
	test	eax, eax
	jnz	@f
	ret	8
@@:
	shl	ecx, 8
	push	eax
	add	eax, ecx
; calculate prefix function
	xor	ecx, ecx
	mov	byte [eax], cl
	xor	edi, edi
	cmp	byte [esp+4+8], 0
	jnz	.whole.prefixcalc
.prefixcalc:
	inc	edi
	movzx	edx, byte [ebx+edi]
	mov	dl, [esi+edx]
	test	dl, dl
	jz	.prefixdone
@@:
	push	eax
	movzx	eax, byte [ebx+ecx]
	cmp	dl, [esi+eax]
	pop	eax
	jz	@f
	jecxz	.prefixint
	mov	cl, byte [eax+ecx-1]
	jmp	@b
@@:
	inc	ecx
.prefixint:
	mov	[eax+edi], cl
	jmp	.prefixcalc
.whole.prefixcalc:
	inc	edi
	movzx	edx, byte [ebx+edi-1]
	mov	dl, [esi+edx]
	test	dl, dl
	jz	.whole.prefixdone
.whole.prefixloop:
	jecxz	.whole.testfirst
	push	eax
	movzx	eax, byte [ebx+ecx-1]
	cmp	dl, [esi+eax]
	pop	eax
	jz	@f
	mov	cl, byte [eax+ecx-1]
	jmp	.whole.prefixloop
.whole.testfirst:
	cmp	[isspace_table+edx], 0
	jz	.whole.prefixint
@@:
	inc	ecx
.whole.prefixint:
	mov	[eax+edi], cl
	jmp	.whole.prefixcalc
.whole.prefixdone:
	jecxz	@f
	push	eax
	movzx	eax, byte [ebx+ecx-1]
	mov	al, [esi+eax]
	cmp	[isspace_table+eax], 0
	pop	eax
	jnz	@f
	mov	cl, byte [eax+ecx-1]
	jmp	.whole.prefixdone
@@:
	inc	ecx
	mov	[eax+edi], cl
.prefixdone:
	pop	edx
; create reverse table for encoding+translation
	push	ebp
	movzx	ebp, byte [esp+8]
	cmp	ebp, encodings.unicode
	jb	@f
	xor	ebp, ebp	; no translations for Unicode encodings,
				; they must be handled separately by caller
@@:
	mov	ecx, 256
@@:
	push	0
	loop	@b
	push	ebx eax
	mov	ebx, esp
	shl	ebp, 7
	xor	eax, eax
.createrev:
	dec	cl
	mov	al, cl
	jns	@f
	mov	al, byte [encodings.tables+ebp+ecx-80h]
@@:
	mov	al, [esi+eax]
	pushd	[ebx+8+eax*4]
	pushd	ecx
	mov	[ebx+8+eax*4], esp
	jnz	.createrev
@@:
	dec	cl
	mov	al, [esi+ecx]
	pushd	[ebx+8+eax*4]
	popd	[ebx+8+ecx*4]
	jnz	@b
; create FSM
	xor	ecx, ecx
	cmp	byte [ebx+259*4+8], 0
	mov	eax, [ebx]
	mov	ebx, [ebx+4]
	mov	edi, edx
	jz	.fsmcalc
	mov	esi, isspace_table
	push	256/4
	pop	ecx
	rep	movsd
	inc	ecx
.fsmcalc:
	movzx	esi, byte [eax+ecx]
	push	eax
	push	ecx
	push	256/4
	pop	ecx
	dec	esi
	js	.fsmzero
	shl	esi, 8
	add	esi, edx
	rep	movsd
	jmp	.fsmnext
.fsmzero:
	cmp	byte [esp+261*4+256*8+8], 0
	jnz	.whole.fsmzero
	xor	eax, eax
	rep	stosd
	jmp	.fsmnext
.whole.fsmzero:
	mov	esi, edx
	rep	movsd
.fsmnext:
	pop	ecx
	movzx	esi, byte [ebx]
	inc	ecx
	mov	esi, [esp+4+8*256+8+esi*4]
@@:
	test	esi, esi
	jz	@f
	lodsd
	mov	[edi-256+eax], cl
	mov	esi, [esi]
	jmp	@b
@@:
	inc	ebx
	pop	eax
	cmp	byte [ebx], 0
	jnz	.fsmcalc
	cmp	byte [esp+259*4+256*8+8], 0
	jz	.nowholefin
	movzx	esi, byte [eax+ecx]
	push	ecx
	mov	ecx, 256
	push	256/4
	pop	ecx
	dec	esi
	shl	esi, 8
	add	esi, edx
	rep	movsd
	pop	ecx
	inc	ecx
	xor	eax, eax
.whole.fsmfin:
	cmp	[isspace_table+eax], ah
	jz	@f
	mov	byte [edi-256+eax], cl
@@:
	inc	al
	jnz	.whole.fsmfin
.nowholefin:
; ok, now edx -> FSM, cl = final state
	add	esp, 8*256+8+4*256
	pop	ebp
	ret	8
