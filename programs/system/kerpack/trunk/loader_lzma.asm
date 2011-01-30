loader_start:
; start address; this code will be injected after the init code
; (some commands below "B32" in the kernel)
	mov	edi, 0x280000
	lea	ebx, [edi+loader_size1+16]
	lea	edx, [ebx+4]
loader_patch1:
	mov	esi, 0		; will be patched: start address to copy
	mov	ecx, 0		; will be patched: size of data to copy
	push	esi
	rep	movsb
	jmp	edx
loader_size1 = $ - loader_start

loader_patch2:
	dd	0x280000 + loader_size
	dd	0		; will be patched: start value for code
				; 	(LZMA-specific)
	dd	-1
	dd	_RangeDecoderBitDecode_edx - loader_start + 0x280000
	dd	_RangeDecoderBitDecode - loader_start + 0x280000
RangeDecoderBitDecode	equ	dword [ebx]
RangeDecoderBitDecode_edx equ	dword [ebx-4]
code_	equ	ebx-12
range	equ	ebx-8

rep1		equ	ebx-28
rep2		equ	ebx-24
rep3		equ	ebx-20
inptr_ldr		equ	ebx-16

pb	equ	0	; pos state bits
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

uninit_base	=	2C0000h

p	=	uninit_base

unpacker:
	xor	ebp, ebp
	xor	eax, eax
	dec	eax
	lea	edi, [rep1]
	stosd
	stosd
	stosd
	xchg	eax, esi
;	mov	ecx, Literal + (LZMA_LIT_SIZE shl (lc+lp))
	mov	ch, (Literal + (LZMA_LIT_SIZE shl (lc+lp)) + 0xFF) shr 8
	mov	eax, kBitModelTotal/2
	mov	edi, p
	rep	stosd
	pop	edi
	push	edi
.main_loop:
..loader_patch3:
	cmp	edi, dword 0		; will be patched: end of data to unpack
	jae	.main_loop_done
if posStateMask
	mov	edx, edi
	and	edx, posStateMask
else
	xor	edx, edx
end if
	push	eax	; al = previous byte
	lea	eax, [ebp + ((p+IsMatch*4) shr (kNumPosBitsMax+2))]
	shl	eax, kNumPosBitsMax+2
if posStateMask
	call	RangeDecoderBitDecode_edx
else
	call	RangeDecoderBitDecode
end if
	pop	eax
	jc	.1
	movzx	eax, al
if literalPosMask
	mov	ah, dl
	and	ah, literalPosMask
end if
if ((LZMA_LIT_SIZE*4) and ((1 shl (8-lc)) - 1)) <> 0
	shr	eax, 8-lc
	imul	eax, LZMA_LIT_SIZE*4
else
	and	al, not ((1 shl (8-lc)) - 1)
	imul	eax, (LZMA_LIT_SIZE*4) shr (8-lc)
end if
	add	eax, p+Literal*4
	mov	dl, 1
	cmp	ebp, kNumLitStates
	jb	.literal
	mov	cl, [edi + esi]
.lx0:
	add	cl, cl
	adc	dh, 1
	call	RangeDecoderBitDecode_edx
	adc	dl, dl
	jc	.lx1
	xor	dh, dl
	test	dh, 1
	mov	dh, 0
	jnz	.lx0
.literal:
@@:
	call	RangeDecoderBitDecode_edx
	adc	dl, dl
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
	xchg	eax, edx
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
	add	eax, p + IsRep0Long*4
	call	RangeDecoderBitDecode_edx
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
	pop	edx
	cmp	edx, ecx
	jb	@f
	mov	edx, ecx
@@:
	push	ecx
	push	kNumPosSlotBits
	pop	ecx
	mov	eax, p+PosSlot*4
	shl	edx, cl
	call	RangeDecoderBitTreeDecode
	mov	esi, ecx
	cmp	ecx, kStartPosModelIndex
	jb	.l6
	mov	edx, ecx
	xor	eax, eax
	shr	ecx, 1
	adc	al, 2
	dec	ecx
	shl	eax, cl
	mov	esi, eax
	sub	eax, edx
	lea	eax, [p + (SpecPos - 1)*4 + eax*4]
	cmp	edx, kEndPosModelIndex
	jb	.l59
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
	add	al, 1 shl kNumAlignBits
@@:
	call	update_decoder
	dec	ecx
	cmp	ecx, kNumAlignBits
	jnz	.l
;	ret
	add	esi, eax
	mov	eax, p+Align_*4
.l59:
;	call	RangeDecoderReverseBitTreeDecode_addesi
;_RangeDecoderReverseBitTreeDecode_addesi:
; in: eax->probs,ecx=numLevels
; out: esi+=length; destroys edx
	push	edi
	xor	edx, edx
	inc	edx
	mov	edi, edx
@@:
	call	RangeDecoderBitDecode_edx
	jnc	.591
	add	esi, edi
	stc
.591:
	adc	edx, edx
	add	edi, edi
	loop	@b
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
	add	ebp, 3
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
include 'calltrick2.asm'
	ret

_RangeDecoderBitDecode:
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
	mov	eax, [inptr_ldr]
	mov	al, [eax]
	inc	dword [inptr_ldr]
	mov	byte [code_], al
	pop	eax
@@:	ret

_RangeDecoderBitDecode_edx:
	push	eax
	lea	eax, [eax+edx*4]
	call	RangeDecoderBitDecode
	pop	eax
	ret

LzmaLenDecode:
; in: eax->prob, edx=posState
; out: ecx=len

; LenChoice==0
;	add	eax, LenChoice*4
if kLenNumMidBits <> kLenNumLowBits
error in optimization
end if
	mov	cl, kLenNumMidBits
	call	RangeDecoderBitDecode
	jnc	.0
	add	eax, (LenChoice2-LenChoice)*4
	call	RangeDecoderBitDecode
	jc	@f
if (kLenNumMidBits <> 3) | (LenMid-LenChoice2 > 0x7F + kLenNumMidBits)
	shl	edx, cl
	add	edx, LenMid-LenChoice2
else
	lea	edx, [ecx + edx*8 - kLenNumMidBits + LenMid-LenChoice2]
end if
	push	kLenNumLowSymbols
	jmp	RangeDecoderBitTreeDecode.1
@@:
	mov	edx, LenHigh-LenChoice2
	mov	cl, kLenNumHighBits
	push	kLenNumLowSymbols + kLenNumMidSymbols
	jmp	RangeDecoderBitTreeDecode.1
.0:
	shl	edx, cl
if LenLow = 2
	inc	edx
	inc	edx
else
	add	edx, LenLow
end if
RangeDecoderBitTreeDecode:
; in: eax+edx*4->probs,ecx=numLevels
; out: ecx=length; destroys edx
	push	0
.1:
	lea	eax, [eax+edx*4]
	xor	edx, edx
	inc	edx
	push	ecx
@@:
	call	RangeDecoderBitDecode_edx
	adc	edx, edx
	loop	@b
	pop	ecx
	btc	edx, ecx
	pop	ecx
	add	ecx, edx
	ret

loader_size = $ - loader_start
