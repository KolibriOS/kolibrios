;* Call: ***************************************************************
lzma_decompress:
	push	esi
	push	edi
	push	ebx
	push	ebp
	mov	esi,[esp+20]
	xor	ebp,ebp
	mov	edi,unpack.code_
	inc	esi
	lodsd 
	bswap	eax
	stosd 
	xor	eax,eax
	dec	eax
	stosd 
	stosd 
	stosd 
	stosd 
	xchg	esi,eax
	stosd 
	mov	ecx,0x1F36
	mov	eax,1024
	mov	edi,unpack.p
	rep	stosd 
	mov	edi,[esp+24]
	mov	ebx, edi
	add	ebx,[esp+28]
;--------------------------------------------------------------------
.main_loop:
	cmp	edi,ebx
	jnb	.main_loop_done

	mov	edx,edi
	and	edx,3
	push	eax
	mov	eax,ebp
	shl	eax,6
	lea	eax,[eax+edx*4+unpack.p]
	call	RangeDecoderBitDecode
	pop	eax
	jb	.labl_06

	movzx	eax,al
	shr	eax,5
	imul	eax,eax,3072
	add	eax,unpack.p+0x1CD8
	mov	cl,1
	cmp	ebp,7
	jb	.labl_02

	mov	dl,[edi+esi]
;--------------------------------------------------------------------
.labl_01:
	add	dl,dl
	setb	ch
	push	eax
	lea	eax,[eax+ecx*4+1024]
	call	RangeDecoderBitDecode
	pop	eax
	adc	cl,cl
	jb	.labl_03

	xor	ch,cl
	test	ch,1
	mov	ch,0
	je	.labl_01
;--------------------------------------------------------------------
.labl_02:
	push	eax
	lea	eax,[eax+ecx*4]
	call	RangeDecoderBitDecode
	pop	eax
	adc	cl,cl
	jnb	.labl_02
;--------------------------------------------------------------------
.labl_03:
	mov	eax,ebp
	cmp	al,4
	jb	.labl_04

	cmp	al,10
	mov	al,3
	jb	.labl_04

	mov	al,6
;--------------------------------------------------------------------
.labl_04:
	sub	ebp,eax
	xchg	ecx,eax
;--------------------------------------------------------------------
.main_loop_1:
	stosb 
	jmp	.main_loop
;--------------------------------------------------------------------
.labl_06:
	lea	eax,[unpack.p+768+ebp*4]
	call	RangeDecoderBitDecode
	jnb	.labl_09

	add	eax,48
	call	RangeDecoderBitDecode
	jb	.labl_07

	mov	eax, ebp
	shl	eax,6
	lea	eax,[eax+edx*4+unpack.p+0x3C0]
	call	RangeDecoderBitDecode
	jb	.labl_08

	cmp	ebp,7
	sbb	ebp,ebp
	lea	ebp,[ebp+ebp+11]
	mov	al,[edi+esi]
	jmp	.main_loop_1
;--------------------------------------------------------------------
.labl_07:
	add	eax,48
	call	RangeDecoderBitDecode
	xchg	esi,[unpack.rep0]
	jnb	.labl_08

	add	eax,48
	call	RangeDecoderBitDecode
	xchg	esi,[unpack.rep1]
	jnb	.labl_08

	xchg	esi,[unpack.rep2]
;--------------------------------------------------------------------
.labl_08:
	mov	eax,unpack.p+0x14D0
	call	LzmaLenDecode
	push	8
	jmp	.labl_17
;--------------------------------------------------------------------
.labl_09:
	xchg	esi,[unpack.rep0]
	xchg	esi,[unpack.rep1]
	mov	[unpack.rep2],esi
	mov	eax,unpack.p+0xCC8
	call	LzmaLenDecode
	push	3
	pop	eax
	cmp	eax,ecx
	jb	.labl_10

	mov	eax,ecx
;--------------------------------------------------------------------
.labl_10:
	push	ecx
	push	6
	pop	ecx
	shl	eax,cl
	shl	eax,2
	add	eax,unpack.p+0x6C0
	call	RangeDecoderBitTreeDecode
	mov	esi,ecx
	cmp	ecx,4
	jb	.labl_16

	push	ecx
	xor	eax,eax
	inc	eax
	shr	ecx,1
	adc	al,al
	dec	ecx
	shl	eax,cl
	mov	esi, eax
	pop	edx
	cmp	edx,14
	jnb	.labl_11

	sub	eax,edx
	shl	eax,2
	add	eax,unpack.p+0xABC
	jmp	.labl_14
;--------------------------------------------------------------------
.labl_11:
	sub	ecx,4
	xor	eax,eax
;--------------------------------------------------------------------
.labl_12:
	shr	dword [unpack.range],1
	add	eax, eax
	mov	edx,[unpack.code_]
	sub	edx,[unpack.range]
	jb	.labl_13

	mov	[unpack.code_],edx
	inc	eax
;--------------------------------------------------------------------
.labl_13:
	call	RangeDecoderBitDecode_1
	loop	.labl_12
	mov	cl,4
	shl	eax,cl
	add	esi,eax
	mov	eax,unpack.p+0xC88
;--------------------------------------------------------------------
.labl_14:
	push	edi
	push	ecx
	xor	edx,edx
	inc	edx
	xor	edi,edi
;--------------------------------------------------------------------
.labl_15:
	push	eax
	lea	eax,[eax+edx*4]
	call	RangeDecoderBitDecode
	lahf 
	adc	edx,edx
	sahf 
	rcr	edi,1
	pop	eax
	loop	.labl_15
	pop	ecx
	rol	edi,cl
	add	esi,edi
	pop	edi
;--------------------------------------------------------------------
.labl_16:
	pop	ecx
	not	esi
	push	7
;--------------------------------------------------------------------
.labl_17:
	cmp	ebp,7
	pop	ebp
	jb	.labl_18

	inc	ebp
	inc	ebp
	inc	ebp
;--------------------------------------------------------------------
.labl_18:
	inc	ecx
	push	esi
	add	esi,edi
	rep	movsb 
	lodsb 
	pop	esi
	jmp	.main_loop_1
;--------------------------------------------------------------------
.main_loop_done:
	pop	ebp
	pop	ebx
	pop	edi
	pop	esi
	ret	12
;*****************************************************************************

;* Call: ***************************************************************
RangeDecoderBitDecode:
; in: eax->prob
; out: CF=bit; destroys eax
	push	edx
	mov	edx,[unpack.range]
	shr	edx,11
	imul	edx,[eax]
	cmp	[unpack.code_],edx
	jnb	.2

	mov	[unpack.range],edx
	mov	edx,2048
	sub	edx,[eax]
	shr	edx,5
	add	[eax],edx
;--------------------------------------------------------------------
.1:
	pushfd 
	call	RangeDecoderBitDecode_1
	popfd 
	pop	edx
	ret 
;--------------------------------------------------------------------
.2:
	sub	[unpack.range],edx
	sub	[unpack.code_],edx
	mov	edx,[eax]
	shr	edx,5
	sub	[eax],edx
	stc 
	jmp	.1
;***********************************************************************

;* Call: ***************************************************************
RangeDecoderBitDecode_1:
	cmp	byte [unpack.range+3],0
	jne	@f

	shl	dword [unpack.range],8
	shl	dword [unpack.code_],8
	push	eax
	mov	eax,[unpack.rep3]
	mov	al,[eax]
	inc	dword [unpack.rep3]
	mov	[unpack.code_],al
	pop	eax
;--------------------------------------------------------------------
@@:
	ret 
;***********************************************************************

;* Call: ***************************************************************
LzmaLenDecode:
; in: eax->prob, edx=posState
; out: ecx=len
	call	RangeDecoderBitDecode
	jnb	.2

	add	eax,4
	call	RangeDecoderBitDecode
	jb	.1

	mov	cl,3
	shl	edx,cl
	lea	eax,[eax+edx*4+516]
	call	RangeDecoderBitTreeDecode
	add	ecx,8
	ret 
;--------------------------------------------------------------------
.1:
	add	eax,1028
	mov	cl,8
	call	RangeDecoderBitTreeDecode
	add	ecx,16
	ret 
;--------------------------------------------------------------------
.2:
	mov	cl,3
	shl	edx,cl
	lea	eax,[eax+edx*4+8]
;***********************************************************************

;* Call: ***************************************************************
RangeDecoderBitTreeDecode:
; in: eax->probs,ecx=numLevels
; out: ecx=length; destroys edx
	push	edi
	xor	edx,edx
	inc	edx
	mov	edi,edx
	xchg	edi, eax
;--------------------------------------------------------------------
@@:
	push	eax
	lea	eax,[edi+edx*4]
	call	RangeDecoderBitDecode
	pop	eax
	adc	dl,dl
	add	al,al
	loop	@b
	sub	dl,al
	pop	edi
	mov	ecx,edx
	ret 
;***********************************************************************

