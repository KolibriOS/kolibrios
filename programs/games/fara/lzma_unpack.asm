; Exports only one function:
; void __stdcall lzma_decompress(
;	const void* source,
;	void* destination,
;	unsigned dest_length);

	format COFF

section '.text' code

pb	equ	2	; pos state bits
lp	equ	0	; literal pos state bits
lc	equ	3	; literal context bits
posStateMask	equ	((1 shl pb)-1)
literalPosMask	equ	((1 shl lp)-1)

kNumPosBitsMax	=	4
kNumPosStatesMax =	(1 shl kNumPosBitsMax)

kLenNumLowBits	=	3
kLenNumLowSymbols =	(1 shl kLenNumLowBits)
kLenNumMidBits	=	3
kLenNumMidSymbols =	(1 shl kLenNumMidBits)
kLenNumHighBits	=	8
kLenNumHighSymbols =	(1 shl kLenNumHighBits)

LenChoice	=	0
LenChoice2	=	1
LenLow		=	2
LenMid		=	(LenLow + (kNumPosStatesMax shl kLenNumLowBits))
LenHigh		=	(LenMid + (kNumPosStatesMax shl kLenNumMidBits))
kNumLenProbs	=	(LenHigh + kLenNumHighSymbols)

kNumStates	=	12
kNumLitStates	=	7
kStartPosModelIndex =	4
kEndPosModelIndex =	14
kNumFullDistances =	(1 shl (kEndPosModelIndex/2))
kNumPosSlotBits =	6
kNumLenToPosStates =	4
kNumAlignBits	=	4
kAlignTableSize	=	(1 shl kNumAlignBits)
kMatchMinLen	=	2

IsMatch		=	0
IsRep		=	0xC0	; (IsMatch + (kNumStates shl kNumPosBitsMax))
IsRepG0		=	0xCC	; (IsRep + kNumStates)
IsRepG1		=	0xD8	; (IsRepG0 + kNumStates)
IsRepG2		=	0xE4	; (IsRepG1 + kNumStates)
IsRep0Long	=	0xF0	; (IsRepG2 + kNumStates)
PosSlot		=	0x1B0	; (IsRep0Long + (kNumStates shl kNumPosBitsMax))
SpecPos		=	0x2B0	; (PosSlot + (kNumLenToPosStates shl kNumPosSlotBits))
Align_		=	0x322	; (SpecPos + kNumFullDistances - kEndPosModelIndex)
Lencoder	=	0x332	; (Align_ + kAlignTableSize)
RepLencoder	=	0x534	; (Lencoder + kNumLenProbs)
Literal		=	0x736	; (RepLencoder + kNumLenProbs)

LZMA_BASE_SIZE	=	1846	; must be ==Literal
LZMA_LIT_SIZE	=	768

kNumTopBits	=	24
kTopValue	=	(1 shl kNumTopBits)

kNumBitModelTotalBits =	11
kBitModelTotal	=	(1 shl kNumBitModelTotalBits)
kNumMoveBits	=	5

RangeDecoderBitDecode:
; in: eax->prob
; out: CF=bit
	push	edx
	mov	edx, [range]
	shr	edx, kNumBitModelTotalBits
	imul	edx, [eax]
	cmp	[code_], edx
	jae	.ae
	mov	[range], edx
	mov	edx, kBitModelTotal
	sub	edx, [eax]
	shr	edx, kNumMoveBits
	add	[eax], edx
.n:
	pushfd
	call	update_decoder
	popfd
	pop	edx
	ret
.ae:
	sub	[range], edx
	sub	[code_], edx
	mov	edx, [eax]
	shr	edx, kNumMoveBits
	sub	[eax], edx
	stc
	jmp	.n

update_decoder:
	cmp	byte [range+3], 0	;cmp	dword [range], kTopValue
	jnz	@f			;jae	@f
	shl	dword [range], 8
	shl	dword [code_], 8
	push	eax
	mov	eax, [inptr]
	mov	al, [eax]
	inc	dword [inptr]
	mov	byte [code_], al
	pop	eax
@@:	ret

LzmaLenDecode:
; in: eax->prob, edx=posState
; out: ecx=len

; LenChoice==0
;	add	eax, LenChoice*4
	call	RangeDecoderBitDecode
	jnc	.0
	add	eax, (LenChoice2-LenChoice)*4
	call	RangeDecoderBitDecode
	jc	@f
	mov	cl, kLenNumMidBits
	shl	edx, cl
	lea	eax, [eax + (LenMid-LenChoice2)*4 + edx*4]
	call	RangeDecoderBitTreeDecode
	add	ecx, kLenNumLowSymbols
	ret
@@:
	add	eax, (LenHigh-LenChoice2)*4
	mov	cl, kLenNumHighBits
	call	RangeDecoderBitTreeDecode
	add	ecx, kLenNumLowSymbols + kLenNumMidSymbols
	ret
.0:
	mov	cl, kLenNumLowBits
	shl	edx, cl
	lea	eax, [eax + LenLow*4 + edx*4]
RangeDecoderBitTreeDecode:
; in: eax->probs,ecx=numLevels
; out: ecx=length; destroys edx
	push	edi
	xor	edx, edx
	inc	edx
	mov	edi, edx
	xchg	eax, edi
@@:
	push	eax
	lea	eax, [edi+edx*4]
	call	RangeDecoderBitDecode
	pop	eax
	adc	dl, dl
	add	al, al
	loop	@b
	sub	dl, al
	pop	edi
	mov	ecx, edx
	ret

; void __stdcall lzma_decompress(
;	const void* source,
;	void* destination,
;	unsigned dest_length);
lzma_decompress equ _lzma_decompress@12
public lzma_decompress
lzma_decompress:
	push	esi edi ebx ebp
	mov	esi, [esp+4*4+4]	; source
	xor	ebp, ebp
	mov	edi, code_
	inc	esi
	lodsd
	bswap	eax
	stosd
	xor	eax, eax
	dec	eax
	stosd
	stosd
	stosd
	stosd
	xchg	eax, esi
	stosd
	mov	ecx, Literal + (LZMA_LIT_SIZE shl (lc+lp))
	mov	eax, kBitModelTotal/2
	mov	edi, p
	rep	stosd
	mov	edi, [esp+4*4+8]	; destination
	mov	ebx, edi
	add	ebx, [esp+4*4+12]	; dest_length
.main_loop:
	cmp	edi, ebx
	jae	.main_loop_done
	mov	edx, edi
	and	edx, posStateMask
	push	eax	; al = previous byte
	mov	eax, ebp
	shl	eax, kNumPosBitsMax+2
	lea	eax, [p + IsMatch*4 + eax + edx*4]
	call	RangeDecoderBitDecode
	pop	eax
	jc	.1
	movzx	eax, al
if literalPosMask
	mov	ah, dl
	and	ah, literalPosMask
end if
	shr	eax, 8-lc
	imul	eax, LZMA_LIT_SIZE*4
	add	eax, p+Literal*4
	mov	cl, 1
	cmp	ebp, kNumLitStates
	jb	.literal
	mov	dl, [edi + esi]
.lx0:
	add	dl, dl
	setc	ch
	push	eax
	lea	eax, [eax+ecx*4+0x100*4]
	call	RangeDecoderBitDecode
	pop	eax
	adc	cl, cl
	jc	.lx1
	xor	ch, cl
	test	ch, 1
	mov	ch, 0
	jz	.lx0
.literal:
@@:
	push	eax
	lea	eax, [eax+ecx*4]
	call	RangeDecoderBitDecode
	pop	eax
	adc	cl, cl
	jnc	@b
.lx1:
	mov	eax, ebp
	cmp	al, 4
	jb	@f
	cmp	al, 10
	mov	al, 3
	jb	@f
	mov	al, 6
@@:	sub	ebp, eax
	xchg	eax, ecx
.stosb_main_loop:
	stosb
	jmp	.main_loop
.1:
	lea	eax, [p + IsRep*4 + ebp*4]
	call	RangeDecoderBitDecode
	jnc	.10
	add	eax, (IsRepG0 - IsRep)*4	;lea	eax, [p + IsRepG0*4 + ebp*4]
	call	RangeDecoderBitDecode
	jc	.111
	mov	eax, ebp
	shl	eax, kNumPosBitsMax+2
	lea	eax, [p + IsRep0Long*4 + eax + edx*4]
	call	RangeDecoderBitDecode
	jc	.1101
	cmp	ebp, 7
	sbb	ebp, ebp
	lea	ebp, [ebp+ebp+11]
	mov	al, [edi + esi]
	jmp	.stosb_main_loop
.111:
	add	eax, (IsRepG1 - IsRepG0) * 4	;lea	eax, [p + IsRepG1*4 + ebp*4]
	call	RangeDecoderBitDecode
	xchg	esi, [rep1]
	jnc	@f
	add	eax, (IsRepG2 - IsRepG1) * 4	;lea	eax, [p + IsRepG2*4 + ebp*4]
	call	RangeDecoderBitDecode
	xchg	esi, [rep2]
	jnc	@f
	xchg	esi, [rep3]
@@:
.1101:
	mov	eax, p + RepLencoder*4
	call	LzmaLenDecode
	push	8
	jmp	.rmu
.10:
	xchg	esi, [rep1]
	xchg	esi, [rep2]
	mov	[rep3], esi
	mov	eax, p + Lencoder*4
	call	LzmaLenDecode
	push	kNumLenToPosStates-1
	pop	eax
	cmp	eax, ecx
	jb	@f
	mov	eax, ecx
@@:
	push	ecx
	push	kNumPosSlotBits
	pop	ecx
	shl	eax, cl
	shl	eax, 2
	add	eax, p+PosSlot*4
	call	RangeDecoderBitTreeDecode
	mov	esi, ecx
	cmp	ecx, kStartPosModelIndex
	jb	.l6
	push	ecx
	xor	eax, eax
	inc	eax
	shr	ecx, 1
	adc	al, al
	dec	ecx
	shl	eax, cl
	mov	esi, eax
	pop	edx
	cmp	edx, kEndPosModelIndex
	jae	.l5
	sub	eax, edx
	shl	eax, 2
	add	eax, p + (SpecPos - 1)*4
	jmp	.l59
.l5:
	sub	ecx, kNumAlignBits
;	call	RangeDecoderDecodeDirectBits
;RangeDecoderDecodeDirectBits:
	xor	eax, eax
.l:
	shr	dword [range], 1
	add	eax, eax
	mov	edx, [code_]
	sub	edx, [range]
	jb	@f
	mov	[code_], edx
	inc	eax
@@:
	call	update_decoder
	loop	.l
;	ret
	mov	cl, kNumAlignBits
	shl	eax, cl
	add	esi, eax
	mov	eax, p+Align_*4
.l59:
;	call	RangeDecoderReverseBitTreeDecode_addesi
;_RangeDecoderReverseBitTreeDecode_addesi:
; in: eax->probs,ecx=numLevels
; out: esi+=length; destroys edx
	push	edi ecx
	xor	edx, edx
	inc	edx
	xor	edi, edi
@@:
	push	eax
	lea	eax, [eax+edx*4]
	call	RangeDecoderBitDecode
	lahf
	adc	edx, edx
	sahf
	rcr	edi, 1
	pop	eax
	loop	@b
	pop	ecx
	rol	edi, cl
	add	esi, edi
	pop	edi
;	ret
.l6:
	pop	ecx
	not	esi
	push	7
.rmu:
	cmp	ebp, 7
	pop	ebp
	jb	@f
	inc	ebp
	inc	ebp
	inc	ebp
@@:
.repmovsb:
	inc	ecx
	push	esi
	add	esi, edi
	rep	movsb
	lodsb
	pop	esi
	jmp	.stosb_main_loop
.main_loop_done:
	pop	ebp ebx edi esi
	ret	12

section '.bss' data
p	rd	LZMA_BASE_SIZE + (LZMA_LIT_SIZE shl (lc+lp))
code_	dd	?
range	dd	?
rep1	dd	?
rep2	dd	?
rep3	dd	?
inptr	dd	?
previousByte db	?
