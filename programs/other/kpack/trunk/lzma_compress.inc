kIfinityPrice equ 0x0FFFFFFF

;* Call: ***************************************************************
;lzma_compress(
;	const void* source,
;	void* destination,
;	unsigned length,
;	void* workmem)
lzma_compress:
	push	ebp
	mov	ebp,esp
	and	esp,0xFFFFFFF8
	sub	esp,12
	push	ebx
	push	esi
	push	edi
	push	2
	pop	esi
	xor	ebx,ebx
	mov	[esp+20],esi
	mov	[g_FastPos],bl
	mov	byte [g_FastPos+1],1
	mov	[esp+16],esi
;----------------------------------------------------------
.labl_00:
	mov	ecx,[esp+16]
	xor	edi,edi
	sar	ecx,1
	dec	ecx
	inc	edi
	shl	edi,cl
	cmp	edi,ebx
	jbe	.labl_01

	mov	eax,[esp+20]
	push	edi
	push	dword [esp+20]
	lea	eax,[eax+g_FastPos]
	push	eax
	call	_memset
	add	esp,12
	add	[esp+20],edi
;----------------------------------------------------------
.labl_01:
	inc	dword [esp+16]
	cmp	dword [esp+16],20
	jl	.labl_00

	mov	edi,[ebp+20]
	push	3
	pop	eax
	mov	ecx,edi
	and	ecx,eax
	mov	edx,128
	mov	[_numFastBytes],edx
	mov	[_posStateBits],esi
	mov	[_posStateMask],eax
	mov	[_numLiteralContextBits],eax
	mov	[_numLiteralPosStateBits],ebx
	mov	[_writeEndMark],bl
	mov	[_finished],bl
	je	.labl_02

	sub	edi,ecx
	add	edi,4
;----------------------------------------------------------
.labl_02:
;CLiteralEncoder_Create
	mov	[_numPrevBits],eax
	mov	eax,[_dictionarySize]
	mov	[_posEncoders],edi
	add	edi,0x6000
	inc	eax
	mov	[_numPosBits],ebx
	mov	[_posMask],ebx
;MatchFinder_Create
	mov	[_matchMaxLen],edx
	mov	[_cyclicBufferSize],eax
	mov	[_hash],edi
	mov	dword [_cutValue],0xFF
	call	CEncoder_Init
	call	FillPosSlotPrices
	call	FillDistancesPrices
	call	FillAlignPrices
	push	127
	pop	esi
	mov	[_lenEncoder+0xC88C],esi
	mov	[esp+16],ebx
;----------------------------------------------------------
.labl_03:
	push	dword [esp+16]
	mov	eax,_lenEncoder
	call	CPriceTableEncoder_UpdateTable
	inc	dword [esp+16]
	cmp	dword [esp+16],4
	jb	.labl_03

	mov	[_repMatchLenEncoder+0xC88C],esi
	xor	esi,esi
;----------------------------------------------------------
.labl_04:
	push	esi
	mov	eax,_repMatchLenEncoder
	call	CPriceTableEncoder_UpdateTable
	inc	esi
	cmp	esi,4
	jb	.labl_04

	mov	eax,[ebp+8]
	mov	esi,[ebp+12]
	dec	eax
	mov	[_buffer],eax
	mov	eax,[ebp+16]
	inc	eax
	mov	[_streamPos],eax
	mov	ecx,0x110000
	xor	eax,eax
	mov	[lastPosSlotFillingPos],ebx
	mov	[lastPosSlotFillingPos+4],ebx
	mov	[nowPos64],ebx
	mov	[nowPos64+4],ebx
	mov	[pack_length],esi
	mov	dword [_pos],1
	rep	stosd	
	mov	[pack_pos],ebx
;----------------------------------------------------------
;MatchFinder_Init
.labl_08:
	push	8
	mov	eax,ebx
	pop	ecx
;----------------------------------------------------------
.labl_07:
	test	al,1
	je	.labl_05

	shr	eax,1
	xor	eax,0xEDB88320
	jmp	.labl_06
;----------------------------------------------------------
.labl_05:
	shr	eax,1
;----------------------------------------------------------
.labl_06:
	dec	ecx
	jne	.labl_07

	mov	[crc_table+ebx*4],eax
	inc	ebx
	cmp	ebx,256
	jb	.labl_08
;----------------------------------------------------------
.labl_09:
	call	CodeOneBlock
	test	al,al
	jne	.labl_09

	mov	eax,[pack_length]
	pop	edi
	sub	eax,esi
	pop	esi
	pop	ebx
	mov	esp,ebp
	pop	ebp
	ret	16
;*****************************************************************************

;* Call: ***************************************************************
_memset:
	push	edi
	mov	edi,[esp+8]
	mov	al,[esp+12]
	mov	ecx,[esp+16]
	rep	stosb	
	pop	edi
	ret	
;*****************************************************************************

;* Call: ***************************************************************
CEncoder_Init:
;RangeEncoder_Init
	and	dword [low],0
	and	dword [low+4],0
	or	dword [range],0xFFFFFFFF
	push	ebx
	push	esi
	push	edi
	xor	eax,eax
	mov	edi,_repDistances
	stosd	
	stosd	
	xor	ebx,ebx
	stosd	
	inc	ebx
;CBaseState_Init
	mov	byte [_state],0
	mov	byte [_previousByte],0
	stosd
	mov	[_cacheSize],ebx
	mov	byte [_cache],0
	xor	ecx,ecx
;----------------------------------------------------------
.labl_00:
	mov	eax,ebx
	shl	eax,cl
	push	8
	pop	edx
	sub	edx,ecx
	lea	esi,[Models+eax*4]
	shl	edx,6
;----------------------------------------------------------
.labl_01:
	mov	edi,eax
	shl	edi,6
	shr	edi,cl
	add	edi,edx
	mov	[esi],edi
	add	esi,4
	dec	eax
	jne	.labl_01

	inc	ecx
	cmp	ecx,9
	jl	.labl_00

	push	12
	pop	edx
	mov	esi,1024
	mov	eax,esi
	mov	ecx,edx
	mov	edi,_isRepG2
	rep	stosd
	mov	ecx,edx
	mov	edi,_isRepG1
	rep	stosd	
	mov	ecx,edx
	mov	edi,_isRepG0
	rep	stosd	
	mov	ecx,edx
	mov	edi,_isRep
	rep	stosd	
	xor	ecx,ecx
;----------------------------------------------------------
.labl_02:
	lea	edi,[ecx+_isRep0Long]
	mov	eax,esi
	stosd
	stosd
	stosd
	stosd
	lea	edi,[ecx+_isMatch]
	mov	eax,esi
	stosd
	stosd
	stosd
	add	ecx,64
	cmp	ecx,768
	stosd
	jl	.labl_02

	mov	eax,[_numPrevBits]
	mov	ecx,[_numPosBits]
	add	ecx,eax
	mov	eax,ebx
	shl	eax,cl
	test	eax,eax
	jbe	.labl_04

	xor	edx,edx
	mov	ebx,eax
;----------------------------------------------------------
.labl_03:
	mov	eax,[_posEncoders]
	lea	edi,[edx+eax]
	mov	ecx,768
	mov	eax,esi
	add	edx,3072
	dec	ebx
	rep	stosd	
	jne	.labl_03
;----------------------------------------------------------
.labl_04:
	mov	edi,_posSlotEncoder
;----------------------------------------------------------
.labl_05:
	push	6
	pop	ecx
	mov	eax,edi
	call	CBitTreeEncoder_Init
	add	edi,1028
	cmp	edi,lastPosSlotFillingPos
	jl	.labl_05

	push	114
	mov	eax,esi
	pop	ecx
	mov	edi,_literalEncoder
	mov	esi,_lenEncoder
	rep	stosd
	call	CPriceTableEncoder_Init
	mov	esi,_repMatchLenEncoder
	call	CPriceTableEncoder_Init
	push	4
	pop	ecx
	mov	eax,_posAlignEncoder
	call	CBitTreeEncoder_Init
	pop	edi
	xor	eax,eax
	pop	esi
	mov	byte [_longestMatchWasFound],0
	mov	[_optimumEndIndex],eax
	mov	[_optimumCurrentIndex],eax
	mov	[_additionalOffset],eax
	pop	ebx
	ret	
;*****************************************************************************

;* Call: ***************************************************************
CodeOneBlock:
	sub	esp,0x2C
	cmp	byte [_finished],0
	push	ebx
	push	ebp
	push	esi
	push	edi
	je	.labl_01
;----------------------------------------------------------
.labl_00:
	xor	al,al
	jmp	.labl_28
;----------------------------------------------------------
.labl_01:
	mov	ebp,[nowPos64]
	mov	edi,[nowPos64+4]
	mov	eax,ebp
	or	eax,edi
	mov	byte [_finished],1
	mov	[esp+52],ebp
	mov	[esp+56],edi
	jne	.labl_04

	mov	eax,[_streamPos]
	sub	eax,[_pos]
	jne	.labl_03

	push	5
	pop	esi
;----------------------------------------------------------
.labl_02:
	call	RangeEncoder_ShiftLow
	dec	esi
	jne	.labl_02

	jmp	.labl_00
;----------------------------------------------------------
.labl_03:
	call	ReadMatchDistances
	movzx	esi,byte [_state]
	mov	edx,esi
	shl	edx,6
	xor	edi,edi
	push	edi
	add	edx,_isMatch
	call	CMyBitEncoder_Encode
	mov	al,[esi+LiteralNextStates]
	mov	ecx,[_pos]
	mov	[_state],al
	mov	eax,[_buffer]
	sub	eax,[_additionalOffset]
	mov	bl,[eax+ecx]
	mov	al,[_previousByte]
	mov	[esp+24],bl
	push	dword [esp+24]
	push	edi
	call	CState_IsCharState
	imul	eax,eax,3072
	add	eax,[_posEncoders]
	push	eax
	call	CLiteralEncoder2_Encode
	dec	dword [_additionalOffset]
	xor	ebp,ebp
	inc	ebp
	mov	[_previousByte],bl
	mov	[nowPos64],ebp
	mov	[nowPos64+4],edi
;----------------------------------------------------------
.labl_04:
	mov	eax,[_streamPos]
	sub	eax,[_pos]
	jne	.labl_06

	push	5
	pop	esi
;----------------------------------------------------------
.labl_05:
	call	RangeEncoder_ShiftLow
	dec	esi
	jne	.labl_05

	jmp	.labl_00
;----------------------------------------------------------
.labl_06:
	lea	eax,[esp+20]
	push	eax
	lea	eax,[esp+20]
	push	eax
	mov	ebx,ebp
	push	ebp
	and	ebx,3
	call	GetOptimum
	cmp	dword [esp+20],1
	jne	.labl_09

	cmp	dword [esp+16],0xFFFFFFFF
	jne	.labl_09

	movzx	eax,byte [_state]
	shl	eax,4
	add	eax,ebx
	push	0
	lea	edx,[_isMatch+eax*4]
	call	CMyBitEncoder_Encode
	mov	eax,[_buffer]
	mov	esi,[_additionalOffset]
	mov	ebx,[_pos]
	sub	eax,esi
	mov	al,[eax+ebx]
	mov	[esp+24],al
	mov	al,[_previousByte]
	push	ebp
	call	CState_IsCharState
	imul	eax,eax,3072
	add	eax,[_posEncoders]
	cmp	byte [_state],7
	push	dword [esp+24]
	jb	.labl_07

	mov	ecx,[_buffer]
	sub	ecx,[_repDistances]
	sub	ecx,esi
	mov	cl,[ecx+ebx-1]
	mov	[esp+52],cl
	push	dword [esp+52]
	push	eax
	call	CLiteralEncoder2_EncodeMatched
	jmp	.labl_08
;----------------------------------------------------------
.labl_07:
	push	eax
	call	CLiteralEncoder2_Encode
;----------------------------------------------------------
.labl_08:
	movzx	eax,byte [_state]
	mov	al,[eax+LiteralNextStates]
	mov	[_state],al
	mov	al,[esp+24]
	jmp	.labl_24
;----------------------------------------------------------
.labl_09:
	movzx	edi,byte [_state]
	mov	eax,edi
	shl	eax,4
	add	eax,ebx
	shl	eax,2
	push	1
	lea	edx,[eax+_isMatch]
	mov	[esp+40],eax
	call	CMyBitEncoder_Encode
	cmp	dword [esp+16],4
	jnb	.labl_17

	mov	esi,edi
	shl	esi,2
	push	1
	lea	edx,[esi+_isRep]
	call	CMyBitEncoder_Encode
	cmp	dword [esp+16],0
	lea	edx,[esi+_isRepG0]
	jne	.labl_10

	push	0
	call	CMyBitEncoder_Encode
	mov	edx,[esp+36]
	xor	eax,eax
	cmp	dword [esp+20],1
	lea	edx,[edx+_isRep0Long]
	setne	al
	jmp	.labl_12
;----------------------------------------------------------
.labl_10:
	push	1
	call	CMyBitEncoder_Encode
	cmp	dword [esp+16],1
	lea	edx,[esi+_isRepG1]
	jne	.labl_11

	push	0
	jmp	.labl_13
;----------------------------------------------------------
.labl_11:
	push	1
	call	CMyBitEncoder_Encode
	mov	eax,[esp+16]
	add	eax,0xFFFFFFFE
	lea	edx,[esi+_isRepG2]
;----------------------------------------------------------
.labl_12:
	push	eax
;----------------------------------------------------------
.labl_13:
	call	CMyBitEncoder_Encode
	mov	eax,[esp+20]
	cmp	eax,1
	jne	.labl_14

	mov	al,[edi+ShortRepNextStates]
	jmp	.labl_15
;----------------------------------------------------------
.labl_14:
	add	eax,0xFFFFFFFE
	mov	esi,_repMatchLenEncoder
	call	CPriceTableEncoder_Encode
	mov	al,[edi+RepNextStates]
;----------------------------------------------------------
.labl_15:
	mov	[_state],al
	mov	eax,[esp+16]
	mov	ecx,[_repDistances+eax*4]
	test	eax,eax
	je	.labl_23
;----------------------------------------------------------
.labl_16:
	dec	eax
	mov	edx,[_repDistances+eax*4]
	mov	[_repDistances+4+eax*4],edx
	jne	.labl_16

	mov	[_repDistances],ecx
	jmp	.labl_23
;----------------------------------------------------------
.labl_17:
	push	0
	lea	edx,[_isRep+edi*4]
	call	CMyBitEncoder_Encode
	mov	al,[edi+MatchNextStates]
	mov	edi,[esp+20]
	mov	[_state],al
	add	edi,0xFFFFFFFE
	mov	eax,edi
	mov	esi,_lenEncoder
	call	CPriceTableEncoder_Encode
	sub	dword [esp+16],4
	mov	eax,[esp+16]
	call	GetPosSlot
	cmp	dword [esp+20],6
	mov	ebx,eax
	jb	.labl_18

	push	3
	pop	edi
;----------------------------------------------------------
.labl_18:
	imul	edi,edi,1028
	push	ebx
	add	edi,_posSlotEncoder
	call	cm_pr_16
	cmp	ebx,4
	jb	.labl_21

	mov	esi,[esp+16]
	xor	eax,eax
	inc	eax
	mov	ecx,ebx
	mov	edi,ebx
	and	edi,eax
	shr	ecx,1
	dec	ecx
	or	edi,2
	shl	edi,cl
	sub	esi,edi
	cmp	ebx,14
	jnb	.labl_20

	mov	[esp+32],esi
	mov	[esp+28],eax
	test	ecx,ecx
	jle	.labl_21

	mov	[esp+36],ecx
;----------------------------------------------------------
.labl_19:
	mov	edx,[esp+28]
	mov	esi,[esp+32]
	sub	edx,ebx
	add	edx,edi
	and	esi,1
	push	esi
	lea	edx,[_numLiteralContextBits+edx*4]
	call	CMyBitEncoder_Encode
	mov	eax,[esp+28]
	shr	dword [esp+32],1
	dec	dword [esp+36]
	lea	eax,[esi+eax*2]
	mov	[esp+28],eax
	jne	.labl_19

	jmp	.labl_21
;----------------------------------------------------------
.labl_20:
	lea	eax,[ecx-4]
	mov	ecx,esi
	shr	ecx,4
	push	ecx
	call	RangeEncoder_EncodeDirectBits
	and	esi,0xF
	push	esi
	call	CBitTreeEncoder_ReverseEncode
	dec	dword [_alignPriceCount]
	jne	.labl_21

	call	FillAlignPrices
;----------------------------------------------------------
.labl_21:
	mov	eax,_repDistances+12
;----------------------------------------------------------
.labl_22:
	mov	ecx,[eax-4]
	mov	[eax],ecx
	sub	eax,4
	cmp	eax,_repDistances
	jne	.labl_22

	mov	eax,[esp+16]
	mov	[_repDistances],eax
;----------------------------------------------------------
.labl_23:
	mov	eax,[_buffer]
	mov	esi,[_additionalOffset]
	mov	ebx,[_pos]
	mov	ecx,[esp+20]
	mov	edi,[nowPos64+4]
	sub	eax,esi
	add	eax,ebx
	mov	al,[eax+ecx-1]
;----------------------------------------------------------
.labl_24:
	sub	esi,[esp+20]
	add	ebp,[esp+20]
	mov	[_previousByte],al
	adc	edi,0
	mov	eax,ebp
	sub	eax,[lastPosSlotFillingPos]
	mov	ecx,edi
	sbb	ecx,[lastPosSlotFillingPos+4]
	mov	[_additionalOffset],esi
	mov	[nowPos64],ebp
	mov	[nowPos64+4],edi
	mov	[esp+40],ecx
	jne	.labl_25

	cmp	eax,512
	jb	.labl_26
;----------------------------------------------------------
.labl_25:
	call	FillPosSlotPrices
	call	FillDistancesPrices
	mov	[lastPosSlotFillingPos],ebp
	mov	[lastPosSlotFillingPos+4],edi
;----------------------------------------------------------
.labl_26:
	test	esi,esi
	jne	.labl_06

	mov	eax,[_streamPos]
	sub	eax,ebx
	je	.labl_29

	mov	ecx,ebp
	sub	ecx,[esp+52]
	mov	eax,edi
	sbb	eax,[esp+56]
	test	eax,eax
	ja	.labl_27

	jb	.labl_06

	cmp	ecx,0x1000
	jb	.labl_06
;----------------------------------------------------------
.labl_27:
	mov	byte [_finished],0
	mov	al,1
;----------------------------------------------------------
.labl_28:
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	add	esp,44
	ret	
;----------------------------------------------------------
.labl_29:
	push	5
	pop	esi
;----------------------------------------------------------
.labl_30:
	call	RangeEncoder_ShiftLow
	dec	esi
	jne	.labl_30

	jmp	.labl_00
;*****************************************************************************

;* Call: ***************************************************************
GetOptimum:
	push	ebp
	mov	ebp,esp
	mov	ecx,[_optimumCurrentIndex]
	sub	esp,116
	push	esi
	cmp	[_optimumEndIndex],ecx
	je	.labl_00

	mov	eax,ecx
	imul	eax,eax,40
	add	eax,state.State
	mov	edx,[eax+16]
	mov	eax,[eax+20]
	mov	esi,edx
	sub	esi,ecx
	mov	ecx,[ebp+16]
	mov	[ecx],esi
	mov	ecx,[ebp+12]
	mov	[ecx],eax
	mov	[_optimumCurrentIndex],edx
	jmp	.labl_76
;----------------------------------------------------------
.labl_00:
	push	edi
	xor	edi,edi
	cmp	byte [_longestMatchWasFound],0
	mov	[_optimumEndIndex],edi
	mov	[_optimumCurrentIndex],edi
	jne	.labl_01

	call	ReadMatchDistances
	jmp	.labl_02
;----------------------------------------------------------
.labl_01:
	mov	eax,[_longestMatchLength]
	mov	byte [_longestMatchWasFound],0
;----------------------------------------------------------
.labl_02:
	push	ebx
	mov	ebx,[ebp+8]
	mov	[ebp-16],eax
	mov	eax,ebx
	shl	eax,2
	mov	[ebp-76],eax
;----------------------------------------------------------
.labl_03:
	mov	esi,edi
	shl	esi,2
	mov	eax,[esi+_repDistances]
	push	eax
	mov	[ebp+esi-100],eax
	push	dword 0xFFFFFFFF
	mov	eax,273
	call	GetMatchLen
	mov	[ebp+esi-116],eax
	test	edi,edi
	je	.labl_04

	mov	ecx,[ebp-76]
	cmp	eax,[ebp+ecx-116]
	jbe	.labl_05
;----------------------------------------------------------
.labl_04:
	mov	ebx,edi
	mov	[ebp-76],esi
;----------------------------------------------------------
.labl_05:
	inc	edi
	cmp	edi,4
	jb	.labl_03

	mov	eax,[ebp+ebx*4-116]
	mov	ecx,128
	mov	[ebp-68],eax
	cmp	eax,ecx
	jb	.labl_06

	mov	ecx,[ebp+12]
	mov	[ecx],ebx
	jmp	.labl_07
;----------------------------------------------------------
.labl_06:
	cmp	[ebp-16],ecx
	jb	.labl_08

	mov	eax,[distances+512]
	mov	ecx,[ebp+12]
	add	eax,4
	mov	[ecx],eax
	mov	eax,[ebp-16]
;----------------------------------------------------------
.labl_07:
	mov	ecx,[ebp+16]
	mov	[ecx],eax
	dec	eax
	call	MovePos
	jmp	.labl_75
;----------------------------------------------------------
.labl_08:
	mov	ecx,[_buffer]
	mov	eax,[_pos]
	mov	dl,[eax+ecx-1]
	sub	eax,[_repDistances]
	mov	bl,[_state]
	mov	[state.State],bl
	mov	al,[eax+ecx-2]
	mov	[ebp-52],al
	mov	eax,[ebp+8]
	and	eax,3
	movzx	ecx,bl
	shl	ecx,4
	add	ecx,eax
	mov	esi,[_isMatch+ecx*4]
	mov	[ebp-28],eax
	cmp	bl,7
	sbb	al,al
	mov	[ebp-48],dl
	push	dword [ebp-48]
	inc	al
	push	dword [ebp-52]
	movzx	eax,al
	push	eax
	push	dword [ebp+8]
	mov	al,[_previousByte]
	mov	[ebp-76],esi
	call	CLiteralEncoder_GetPrice
	or	dword [state.BackPrev+40],0xFFFFFFFF
	and	dword [state.PosPrev+40],0
	shr	esi,2
	add	eax,[Models+esi*4]
	mov	byte [state.Prev1IsChar+40],0
	mov	[state.Price+40],eax
	lea	esi,[ebp-100]
	mov	edi,state.Backs
	movsd	
	movsd	
	mov	eax,2048
	mov	ecx,eax
	sub	ecx,[ebp-76]
	movsd	
	shr	ecx,2
	mov	edx,[Models+ecx*4]
	movzx	ecx,bl
	movsd	
	mov	esi,[_isRep+ecx*4]
	sub	eax,esi
	shr	eax,2
	mov	edi,[Models+eax*4]
	mov	al,[ebp-48]
	add	edi,edx
	mov	[ebp-76],edx
	cmp	[ebp-52],al
	jne	.labl_09

	push	dword [ebp-28]
	mov	al,bl
	call	GetRepLen1Price
	add	eax,edi
	cmp	eax,[state.Price+40]
	jnb	.labl_09

	and	dword [state.BackPrev+40],0
	mov	[state.Price+40],eax
	mov	byte [state.Prev1IsChar+40],0
;----------------------------------------------------------
.labl_09:
	push	2
	pop	eax
	cmp	[ebp-16],eax
	jnb	.labl_10

	mov	eax,[ebp+12]
	mov	ecx,[state.BackPrev+40]
	mov	[eax],ecx
	mov	eax,[ebp+16]
	mov	dword [eax],1
	jmp	.labl_75
;----------------------------------------------------------
.labl_10:
	mov	ecx,[ebp-16]
	shr	esi,2
	mov	esi,[Models+esi*4]
	add	esi,[ebp-76]
	mov	[ebp-76],esi
	cmp	ecx,[ebp-68]
	ja	.labl_11

	and	dword [ebp-16],0
;----------------------------------------------------------
.labl_11:
	mov	[ebp-24],eax
	cmp	[ebp-16],eax
	jb	.labl_13

	mov	esi,state.BackPrev+40*2
;----------------------------------------------------------
.labl_12:
	mov	eax,[ebp-24]
	mov	edx,[distances+eax*4]
	push	dword [ebp-28]
	and	dword [esi-4],0
	lea	ecx,[edx+4]
	mov	[esi],ecx
	call	GetPosLenPrice
	add	eax,[ebp-76]
	inc	dword [ebp-24]
	mov	[esi-8],eax
	mov	eax,[ebp-24]
	mov	byte [esi-19],0
	add	esi,40
	cmp	eax,[ebp-16]
	jbe	.labl_12
;----------------------------------------------------------
.labl_13:
	mov	ecx,[ebp-16]
	cmp	ecx,[ebp-68]
	jnb	.labl_14

	mov	ecx,[ebp-68]
	mov	[ebp-16],ecx
;----------------------------------------------------------
.labl_14:
	cmp	[ebp-24],ecx
	ja	.labl_16

	mov	eax,[ebp-24]
	sub	ecx,[ebp-24]
	imul	eax,eax,40
	add	eax,state.Price
	inc	ecx
;----------------------------------------------------------
.labl_15:
	mov	dword [eax],kIfinityPrice
	add	eax,40
	dec	ecx
	jne	.labl_15
;----------------------------------------------------------
.labl_16:
	and	dword [ebp-24],0
;----------------------------------------------------------
.labl_17:
	mov	eax,[ebp-24]
	mov	eax,[ebp+eax*4-116]
	push	2
	pop	ecx
	mov	[ebp-32],ecx
	cmp	eax,ecx
	jb	.labl_20

	mov	esi,state.Price+40*2
;----------------------------------------------------------
.labl_18:
	push	dword [ebp-28]
	mov	edx,[ebp-24]
	movzx	eax,bl
	push	eax
	mov	eax,[ebp-32]
	call	GetRepPrice
	add	eax,edi
	cmp	eax,[esi]
	jnb	.labl_19

	and	dword [esi+4],0
	mov	[esi],eax
	mov	eax,[ebp-24]
	mov	[esi+8],eax
	mov	byte [esi-11],0
;----------------------------------------------------------
.labl_19:
	inc	dword [ebp-32]
	mov	eax,[ebp-24]
	mov	ecx,[ebp-32]
	add	esi,40
	cmp	ecx,[ebp+eax*4-116]
	jbe	.labl_18
;----------------------------------------------------------
.labl_20:
	inc	dword [ebp-24]
	cmp	dword [ebp-24],4
	jb	.labl_17

	mov	eax,[ebp-16]
	xor	ecx,ecx
	inc	ecx
	mov	[ebp-8],eax
	mov	[ebp-12],ecx
	cmp	eax,ecx
	je	.labl_74
;----------------------------------------------------------
.labl_21:
	mov	edi,[ebp-12]
	inc	dword [ebp+8]
	mov	ebx,edi
	imul	ebx,ebx,40
	mov	dl,[ebx+state.Prev1IsChar]
	mov	esi,[ebx+state.PosPrev]
	test	dl,dl
	je	.labl_25

	dec	esi
	cmp	byte [ebx+state.Prev2],0
	je	.labl_23

	mov	eax,[ebx+state.PosPrev2]
	imul	eax,eax,40
	cmp	dword [ebx+state.BackPrev2],4
	mov	al,[eax+state.State]
	movzx	eax,al
	jnb	.labl_22

	mov	al,[eax+RepNextStates]
	jmp	.labl_24
;----------------------------------------------------------
.labl_22:
	mov	al,[eax+MatchNextStates]
	jmp	.labl_24
;----------------------------------------------------------
.labl_23:
	mov	eax,esi
	imul	eax,eax,40
	mov	al,[eax+state.State]
;----------------------------------------------------------
.labl_24:
	movzx	eax,al
	mov	cl,[eax+LiteralNextStates]
	jmp	.labl_26
;----------------------------------------------------------
.labl_25:
	mov	eax,esi
	imul	eax,eax,40
	mov	cl,[eax+state.State]
;----------------------------------------------------------
.labl_26:
	dec	edi
	cmp	esi,edi
	jne	.labl_29

	cmp	dword [ebx+state.BackPrev],0
	movzx	eax,cl
	jne	.labl_27

	mov	al,[eax+ShortRepNextStates]
	jmp	.labl_28
;----------------------------------------------------------
.labl_27:
	mov	al,[eax+LiteralNextStates]
;----------------------------------------------------------
.labl_28:
	mov	[ebp-20],al
	jmp	.labl_35
;----------------------------------------------------------
.labl_29:
	test	dl,dl
	je	.labl_31

	cmp	byte [ebx+state.Prev2],0
	je	.labl_31

	mov	esi,[ebx+state.PosPrev2]
	mov	eax,[ebx+state.BackPrev2]
	movzx	ecx,cl
;----------------------------------------------------------
.labl_30:
	mov	cl,[ecx+RepNextStates]
	jmp	.labl_32
;----------------------------------------------------------
.labl_31:
	mov	eax,[ebx+state.BackPrev]
	movzx	ecx,cl
	cmp	eax,4
	jb	.labl_30

	mov	cl,[ecx+MatchNextStates]
;----------------------------------------------------------
.labl_32:
	imul	esi,esi,40
	add	esi,state.State
	mov	[ebp-20],cl
	mov	edx,esi
	cmp	eax,4
	jnb	.labl_34

	mov	ecx,[edx+eax*4+24]
	xor	esi,esi
	inc	esi
	mov	[ebp-100],ecx
	cmp	eax,esi
	jb	.labl_33

	lea	esi,[edx+24]
	mov	ecx,eax
	lea	edi,[ebp-96]
	rep	movsd	
	lea	esi,[eax+1]
	cmp	esi,4
	jnb	.labl_35
;----------------------------------------------------------
.labl_33:
	push	4
	lea	edx,[edx+esi*4+24]
	pop	ecx
	lea	edi,[ebp+esi*4-100]
	sub	ecx,esi
	mov	esi,edx
	rep	movsd	
	jmp	.labl_35
;----------------------------------------------------------
.labl_34:
	add	eax,0xFFFFFFFC
	mov	[ebp-100],eax
	lea	esi,[edx+24]
	lea	edi,[ebp-96]
	movsd	
	movsd	
	movsd	
;----------------------------------------------------------
.labl_35:
	mov	al,[ebp-20]
	mov	[ebx+state.State],al
	lea	edi,[ebx+state.Backs]
	lea	esi,[ebp-100]
	movsd	
	movsd	
	movsd	
	movsd	
	call	ReadMatchDistances
	mov	[ebp-44],eax
	cmp	eax,128
	jnb	.labl_73

	mov	eax,[ebx+state.Price]
	mov	edi,[_pos]
	mov	esi,[ebp+8]
	mov	[ebp-72],eax
	mov	eax,[_buffer]
	lea	eax,[edi+eax-1]
	mov	cl,[eax]
	mov	[ebp-48],cl
	push	dword [ebp-48]
	mov	ecx,eax
	sub	ecx,[ebp-100]
	and	esi,3
	mov	cl,[ecx-1]
	mov	[ebp-52],cl
	movzx	ecx,byte [ebp-20]
	push	dword [ebp-52]
	mov	[ebp-64],ecx
	shl	ecx,4
	add	ecx,esi
	mov	ecx,[_isMatch+ecx*4]
	cmp	byte [ebp-20],7
	mov	[ebp-76],ecx
	sbb	cl,cl
	inc	cl
	movzx	ecx,cl
	push	ecx
	push	dword [ebp+8]
	mov	[ebp-40],eax
	mov	al,[eax-1]
	mov	[ebp-28],esi
	call	CLiteralEncoder_GetPrice
	mov	esi,eax
	mov	eax,[ebp-76]
	shr	eax,2
	add	esi,[Models+eax*4]
	mov	byte [ebp-1],0
	add	esi,[ebp-72]
	cmp	esi,[ebx+state.Price+40]
	jnb	.labl_36

	mov	eax,[ebp-12]
	or	dword [ebx+state.BackPrev+40],0xFFFFFFFF
	mov	[ebx+state.Price+40],esi
	mov	[ebx+state.PosPrev+40],eax
	mov	byte [ebx+state.Prev1IsChar+40],0
	mov	byte [ebp-1],1
;----------------------------------------------------------
.labl_36:
	mov	edx,[ebp-64]
	mov	edx,[_isRep+edx*4]
	mov	ecx,2048
	mov	eax,ecx
	sub	eax,[ebp-76]
	sub	ecx,edx
	shr	eax,2
	mov	eax,[Models+eax*4]
	add	eax,[ebp-72]
	shr	ecx,2
	mov	ecx,[Models+ecx*4]
	add	ecx,eax
	mov	[ebp-76],eax
	mov	al,[ebp-48]
	mov	[ebp-84],edx
	mov	[ebp-72],ecx
	cmp	[ebp-52],al
	jne	.labl_38

	mov	eax,[ebp-12]
	cmp	[ebx+state.PosPrev+40],eax
	jnb	.labl_37

	cmp	dword [ebx+state.BackPrev+40],0
	je	.labl_38
;----------------------------------------------------------
.labl_37:
	push	dword [ebp-28]
	mov	al,[ebp-20]
	call	GetRepLen1Price
	add	eax,[ebp-72]
	cmp	eax,[ebx+state.Price+40]
	ja	.labl_38

	and	dword [ebx+state.BackPrev+40],0
	mov	[ebx+state.Price+40],eax
	mov	eax,[ebp-12]
	mov	[ebx+state.PosPrev+40],eax
	mov	byte [ebx+state.Prev1IsChar+40],0
;----------------------------------------------------------
.labl_38:
	mov	eax,[_streamPos]
	sub	eax,edi
	mov	ecx,4095
	sub	ecx,[ebp-12]
	inc	eax
	mov	[ebp-56],eax
	cmp	eax,ecx
	jbe	.labl_39

	mov	eax,ecx
	mov	[ebp-56],ecx
;----------------------------------------------------------
.labl_39:
	mov	[ebp-36],eax
	cmp	eax,2
	jb	.labl_72

	mov	ecx,128
	cmp	eax,ecx
	jbe	.labl_40

	mov	[ebp-36],ecx
;----------------------------------------------------------
.labl_40:
	cmp	dword [ebp-36],3
	jb	.labl_45

	cmp	byte [ebp-1],0
	jne	.labl_45

	mov	ecx,[ebp-100]
	xor	eax,eax
	inc	eax
	inc	ecx
	cmp	[ebp-36],eax
	jbe	.labl_42

	mov	edi,[ebp-40]
	mov	edx,edi
	sub	edx,ecx
;----------------------------------------------------------
.labl_41:
	mov	cl,[eax+edi]
	cmp	cl,[edx+eax]
	jne	.labl_42

	inc	eax
	cmp	eax,[ebp-36]
	jb	.labl_41
;----------------------------------------------------------
.labl_42:
	dec	eax
	mov	[ebp-32],eax
	cmp	eax,2
	jb	.labl_45

	mov	eax,[ebp-64]
	mov	al,[eax+LiteralNextStates]
	mov	ecx,[ebp+8]
	movzx	edx,al
	mov	[ebp-60],al
	mov	eax,edx
	shl	eax,4
	inc	ecx
	mov	edi,2048
	and	ecx,3
	add	eax,ecx
	mov	ebx,edi
	sub	ebx,[_isMatch+eax*4]
	mov	eax,edi
	sub	eax,[_isRep+edx*4]
	mov	edx,[ebp-12]
	shr	ebx,2
	mov	edi,[Models+ebx*4]
	shr	eax,2
	add	edi,[Models+eax*4]
	mov	eax,[ebp-32]
	add	edi,esi
	lea	esi,[edx+eax]
	mov	eax,[ebp-8]
	lea	edx,[esi+1]
	cmp	eax,edx
	jnb	.labl_44

	sub	edx,[ebp-8]
	imul	eax,eax,40
	add	eax,state.Price
	add	[ebp-8],edx
;----------------------------------------------------------
.labl_43:
	add	eax,40
	dec	edx
	mov	dword [eax],kIfinityPrice
	jne	.labl_43
;----------------------------------------------------------
.labl_44:
	mov	eax,[ebp-32]
	push	ecx
	push	dword [ebp-60]
	xor	edx,edx
	call	GetRepPrice
	imul	esi,esi,40
	add	eax,edi
	add	esi,state.State+40
	cmp	eax,[esi+12]
	jnb	.labl_45

	mov	[esi+12],eax
	mov	eax,[ebp-12]
	inc	eax
	and	dword [esi+20],0
	mov	[esi+16],eax
	mov	byte [esi+1],1
	mov	byte [esi+2],0
;----------------------------------------------------------
.labl_45:
	and	dword [ebp-24],0
;----------------------------------------------------------
.labl_46:
	mov	eax,[ebp-24]
	mov	eax,[ebp+eax*4-100]
	mov	edx,[ebp-40]
	inc	eax
	mov	ecx,edx
	sub	ecx,eax
	mov	[ebp-68],eax
	mov	al,[edx]
	mov	[ebp-80],ecx
	cmp	al,[ecx]
	jne	.labl_58

	mov	al,[edx+1]
	cmp	al,[ecx+1]
	jne	.labl_58

	push	2
	pop	esi
	cmp	[ebp-36],esi
	jbe	.labl_48
;----------------------------------------------------------
.labl_47:
	mov	al,[edx+esi]
	cmp	al,[ecx+esi]
	jne	.labl_48

	inc	esi
	cmp	esi,[ebp-36]
	jb	.labl_47
;----------------------------------------------------------
.labl_48:
	mov	eax,[ebp-12]
	lea	ebx,[eax+esi]
	mov	[ebp-32],ebx
	imul	ebx,ebx,40
	mov	edi,esi
	add	ebx,state.Price
;----------------------------------------------------------
.labl_49:
	mov	eax,[ebp-8]
	mov	ecx,[ebp-32]
	cmp	eax,ecx
	jnb	.labl_51

	sub	ecx,[ebp-8]
	imul	eax,eax,40
	add	eax,state.Price
	add	[ebp-8],ecx
;----------------------------------------------------------
.labl_50:
	add	eax,40
	dec	ecx
	mov	dword [eax],kIfinityPrice
	jne	.labl_50
;----------------------------------------------------------
.labl_51:
	push	dword [ebp-28]
	mov	edx,[ebp-24]
	push	dword [ebp-20]
	mov	eax,esi
	call	GetRepPrice
	add	eax,[ebp-72]
	cmp	eax,[ebx]
	jnb	.labl_52

	mov	[ebx],eax
	mov	eax,[ebp-12]
	mov	[ebx+4],eax
	mov	eax,[ebp-24]
	mov	[ebx+8],eax
	mov	byte [ebx-11],0
;----------------------------------------------------------
.labl_52:
	dec	esi
	sub	ebx,40
	dec	dword [ebp-32]
	cmp	esi,2
	jnb	.labl_49

	lea	eax,[edi+1]
	lea	esi,[eax+128]
	cmp	esi,[ebp-56]
	jbe	.labl_53

	mov	esi,[ebp-56]
;----------------------------------------------------------
.labl_53:
	cmp	eax,esi
	jnb	.labl_55

	mov	edx,[ebp-40]
	mov	ecx,eax
	sub	ecx,[ebp-68]
	add	ecx,edx
;----------------------------------------------------------
.labl_54:
	mov	bl,[eax+edx]
	cmp	bl,[ecx]
	jne	.labl_55

	inc	eax
	inc	ecx
	cmp	eax,esi
	jb	.labl_54
;----------------------------------------------------------
.labl_55:
	or	ecx,0xFFFFFFFF
	sub	ecx,edi
	add	eax,ecx
	mov	[ebp-16],eax
	cmp	eax,2
	jb	.labl_58

	mov	eax,[ebp-64]
	mov	al,[eax+RepNextStates]
	mov	edx,[ebp+8]
	movzx	eax,al
	mov	ebx,eax
	mov	al,[eax+LiteralNextStates]
	lea	ecx,[edi+edx]
	mov	[ebp-60],al
	movzx	eax,al
	mov	esi,ecx
	and	esi,3
	shl	ebx,4
	add	esi,ebx
	mov	esi,[_isMatch+esi*4]
	lea	ebx,[edi+edx+1]
	mov	[ebp-32],eax
	mov	eax,[ebp-40]
	movzx	edx,byte [edi+eax]
	mov	al,[eax+edi-1]
	push	edx
	mov	edx,[ebp-80]
	movzx	edx,byte [edx+edi]
	push	edx
	shr	esi,2
	mov	esi,[Models+esi*4]
	push	1
	push	ecx
	mov	[ebp-68],esi
	and	ebx,3
	call	CLiteralEncoder_GetPrice
	push	dword [ebp-28]
	mov	edx,[ebp-24]
	push	dword [ebp-20]
	mov	esi,eax
	mov	eax,edi
	call	GetRepPrice
	mov	ecx,[ebp-32]
	add	esi,eax
	shl	ecx,4
	add	ecx,ebx
	mov	eax,2048
	mov	edx,eax
	sub	edx,[_isMatch+ecx*4]
	mov	ecx,[ebp-32]
	sub	eax,[_isRep+ecx*4]
	mov	ecx,[ebp-12]
	shr	edx,2
	add	esi,[Models+edx*4]
	shr	eax,2
	add	esi,[Models+eax*4]
	mov	eax,[ebp-16]
	add	esi,[ebp-68]
	lea	eax,[eax+edi+1]
	add	esi,[ebp-72]
	add	eax,ecx
	mov	[ebp-68],eax
	cmp	[ebp-8],eax
	jnb	.labl_57

	mov	eax,[ebp-8]
	mov	ecx,[ebp-68]
	imul	eax,eax,40
	sub	ecx,[ebp-8]
	add	eax,state.Price
	add	[ebp-8],ecx
;----------------------------------------------------------
.labl_56:
	add	eax,40
	dec	ecx
	mov	dword [eax],kIfinityPrice
	jne	.labl_56
;----------------------------------------------------------
.labl_57:
	mov	eax,[ebp-16]
	push	ebx
	push	dword [ebp-60]
	xor	edx,edx
	call	GetRepPrice
	mov	ecx,[ebp-68]
	imul	ecx,ecx,40
	add	eax,esi
	add	ecx,state.State
	cmp	eax,[ecx+12]
	jnb	.labl_58

	and	dword [ecx+20],0
	mov	[ecx+12],eax
	mov	eax,[ebp-12]
	lea	edx,[edi+eax+1]
	mov	[ecx+4],eax
	mov	eax,[ebp-24]
	mov	[ecx+16],edx
	mov	byte [ecx+1],1
	mov	byte [ecx+2],1
	mov	[ecx+8],eax
;----------------------------------------------------------
.labl_58:
	inc	dword [ebp-24]
	cmp	dword [ebp-24],4
	jb	.labl_46

	mov	eax,[ebp-36]
	cmp	[ebp-44],eax
	jbe	.labl_59

	mov	[ebp-44],eax
;----------------------------------------------------------
.labl_59:
	cmp	dword [ebp-44],2
	jb	.labl_72

	mov	esi,[ebp-44]
	cmp	esi,2
	jne	.labl_60

	cmp	dword [distances+8],0x80
	jnb	.labl_72
;----------------------------------------------------------
.labl_60:
	mov	eax,[ebp-84]
	shr	eax,2
	mov	eax,[Models+eax*4]
	add	eax,[ebp-76]
	mov	[ebp-76],eax
	mov	eax,[ebp-12]
	lea	ecx,[esi+eax]
	cmp	[ebp-8],ecx
	jnb	.labl_62

	mov	eax,[ebp-8]
	imul	eax,eax,40
	mov	edx,ecx
	sub	edx,[ebp-8]
	add	eax,state.Price
	add	[ebp-8],edx
;----------------------------------------------------------
.labl_61:
	add	eax,40
	dec	edx
	mov	dword [eax],kIfinityPrice
	jne	.labl_61
;----------------------------------------------------------
.labl_62:
	imul	ecx,ecx,40
	add	ecx,state.Price
	lea	eax,[distances+4+esi*4]
	lea	edi,[esi+1]
	mov	[ebp-36],eax
	mov	ebx,ecx
	mov	[ebp-72],ecx
;----------------------------------------------------------
.labl_63:
	mov	eax,[ebp-36]
	mov	esi,[eax-4]
	push	dword [ebp-28]
	lea	eax,[edi-1]
	mov	edx,esi
	call	GetPosLenPrice
	add	eax,[ebp-76]
	mov	[ebp-80],eax
	cmp	eax,[ebx]
	jnb	.labl_64

	mov	[ebx],eax
	mov	eax,[ebp-12]
	mov	[ebx+4],eax
	lea	eax,[esi+4]
	mov	[ebx+8],eax
	mov	byte [ebx-11],0
;----------------------------------------------------------
.labl_64:
	lea	eax,[edi-1]
	cmp	eax,[ebp-44]
	je	.labl_65

	mov	eax,[ebp-36]
	cmp	esi,[eax]
	je	.labl_71
;----------------------------------------------------------
.labl_65:
	inc	esi
	lea	ecx,[edi+128]
	mov	[ebp-68],esi
	mov	[ebp-16],edi
	cmp	ecx,[ebp-56]
	jbe	.labl_66

	mov	ecx,[ebp-56]
;----------------------------------------------------------
.labl_66:
	cmp	edi,ecx
	jnb	.labl_68

	mov	eax,edi
	sub	eax,esi
	add	eax,[ebp-40]
;----------------------------------------------------------
.labl_67:
	mov	edx,[ebp-16]
	mov	esi,[ebp-40]
	mov	dl,[edx+esi]
	cmp	dl,[eax]
	jne	.labl_68

	inc	dword [ebp-16]
	inc	eax
	cmp	[ebp-16],ecx
	jb	.labl_67
;----------------------------------------------------------
.labl_68:
	or	ecx,0xFFFFFFFF
	lea	eax,[edi-1]
	sub	ecx,eax
	add	[ebp-16],ecx
	cmp	dword [ebp-16],2
	jb	.labl_71

	mov	eax,[ebp-64]
	mov	al,[eax+MatchNextStates]
	mov	ecx,[ebp+8]
	movzx	eax,al
	mov	esi,eax
	mov	al,[eax+LiteralNextStates]
	lea	edx,[edi+ecx-1]
	mov	[ebp-60],al
	movzx	eax,al
	shl	esi,4
	mov	ebx,edx
	and	ebx,3
	add	esi,ebx
	mov	esi,[_isMatch+esi*4]
	lea	ebx,[edi+ecx]
	mov	[ebp-32],eax
	mov	eax,[ebp-40]
	movzx	ecx,byte [edi+eax-1]
	push	ecx
	mov	ecx,edi
	sub	ecx,[ebp-68]
	shr	esi,2
	movzx	ecx,byte [ecx+eax-1]
	mov	esi,[Models+esi*4]
	mov	al,[edi+eax-2]
	push	ecx
	push	1
	push	edx
	mov	[ebp-84],esi
	and	ebx,3
	call	CLiteralEncoder_GetPrice
	mov	ecx,[ebp-32]
	mov	esi,eax
	shl	ecx,4
	add	ecx,ebx
	mov	eax,2048
	mov	edx,eax
	sub	edx,[_isMatch+ecx*4]
	mov	ecx,[ebp-32]
	sub	eax,[_isRep+ecx*4]
	mov	ecx,[ebp-12]
	shr	edx,2
	add	esi,[Models+edx*4]
	shr	eax,2
	add	esi,[Models+eax*4]
	mov	eax,[ebp-16]
	add	esi,[ebp-84]
	add	eax,edi
	add	esi,[ebp-80]
	add	eax,ecx
	mov	[ebp-32],eax
	cmp	[ebp-8],eax
	jnb	.labl_70

	mov	eax,[ebp-8]
	mov	ecx,[ebp-32]
	imul	eax,eax,40
	sub	ecx,[ebp-8]
	add	eax,state.Price
	add	[ebp-8],ecx
;----------------------------------------------------------
.labl_69:
	add	eax,40
	dec	ecx
	mov	dword [eax],kIfinityPrice
	jne	.labl_69
;----------------------------------------------------------
.labl_70:
	mov	eax,[ebp-16]
	push	ebx
	push	dword [ebp-60]
	xor	edx,edx
	call	GetRepPrice
	mov	ecx,[ebp-32]
	mov	ebx,[ebp-72]
	imul	ecx,ecx,40
	add	eax,esi
	add	ecx,state.State
	cmp	eax,[ecx+12]
	jnb	.labl_71

	and	dword [ecx+20],0
	mov	[ecx+12],eax
	mov	eax,[ebp-12]
	lea	edx,[edi+eax]
	mov	[ecx+4],eax
	mov	eax,[ebp-68]
	add	eax,3
	mov	[ecx+16],edx
	mov	byte [ecx+1],1
	mov	byte [ecx+2],1
	mov	[ecx+8],eax
;----------------------------------------------------------
.labl_71:
	sub	dword [ebp-36],4
	sub	ebx,40
	dec	edi
	lea	eax,[edi-1]
	mov	[ebp-72],ebx
	cmp	eax,2
	jnb	.labl_63
;----------------------------------------------------------
.labl_72:
	inc	dword [ebp-12]
	mov	eax,[ebp-12]
	cmp	eax,[ebp-8]
	jne	.labl_21

	jmp	.labl_74
;----------------------------------------------------------
.labl_73:
	mov	eax,[ebp-44]
	mov	[_longestMatchLength],eax
	mov	byte [_longestMatchWasFound],1
;----------------------------------------------------------
.labl_74:
	push	dword [ebp-12]
	push	dword [ebp+12]
	call	Backward
	mov	ecx,[ebp+16]
	mov	[ecx],eax
;----------------------------------------------------------
.labl_75:
	pop	ebx
	pop	edi
;----------------------------------------------------------
.labl_76:
	pop	esi
	leave	
	ret	12
;*****************************************************************************

;* Call: ***************************************************************
CLiteralEncoder2_EncodeMatched:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	ebx
	push	esi
	xor	ebx,ebx
	push	edi
	inc	ebx
	mov	dword [ebp-4],8
;----------------------------------------------------------
.labl_00:
	dec	dword [ebp-4]
	movzx	ecx,byte [ebp-4]
	movzx	esi,byte [ebp+12]
	movzx	edi,byte [ebp+16]
	shr	esi,cl
	shr	edi,cl
	mov	ecx,[ebp+8]
	and	esi,1
	lea	eax,[esi+1]
	shl	eax,8
	add	eax,ebx
	and	edi,1
	push	edi
	lea	edx,[ecx+eax*4]
	call	CMyBitEncoder_Encode
	add	ebx,ebx
	or	ebx,edi
	cmp	esi,edi
	jne	.labl_02

	cmp	dword [ebp-4],0
	jne	.labl_00

	jmp	.labl_03
;----------------------------------------------------------
.labl_01:
	dec	dword [ebp-4]
	movzx	esi,byte [ebp+16]
	mov	cl,[ebp-4]
	mov	eax,[ebp+8]
	shr	esi,cl
	lea	edx,[eax+ebx*4]
	and	esi,1
	push	esi
	call	CMyBitEncoder_Encode
	add	ebx,ebx
	or	ebx,esi
;----------------------------------------------------------
.labl_02:
	cmp	dword [ebp-4],0
	jne	.labl_01
;----------------------------------------------------------
.labl_03:
	pop	edi
	pop	esi
	pop	ebx
	leave	
	ret	12
;*****************************************************************************

;* Call: ***************************************************************
RangeEncoder_EncodeDirectBits:
	push	esi
	mov	esi,eax
	jmp	.labl_02
;----------------------------------------------------------
.labl_00:
	mov	eax,[esp+8]
	shr	dword [range],1
	dec	esi
	mov	ecx,esi
	shr	eax,cl
	test	al,1
	je	.labl_01

	mov	eax,[range]
	add	[low],eax	
	adc	dword [low+4],0
;----------------------------------------------------------
.labl_01:
	cmp	dword [range],0x1000000
	jnb	.labl_02

	shl	dword [range],8
	call	RangeEncoder_ShiftLow
;----------------------------------------------------------
.labl_02:
	test	esi,esi
	jne	.labl_00

	pop	esi
	ret	4
;*****************************************************************************

;* Call: ***************************************************************
CBitTreeEncoder_ReverseEncode:
	push	esi
	xor	esi,esi
	push	edi
	mov	edi,[i_01]
	inc	esi
;----------------------------------------------------------
.labl_00:
	mov	eax,[esp+12]
	and	eax,1
	lea	edx,[_posAlignEncoder+esi*4]
	push	eax
	lea	esi,[eax+esi*2]
	call	CMyBitEncoder_Encode
	shr	dword [esp+12],1
	dec	edi
	jne	.labl_00

	pop	edi
	pop	esi
	ret	4
;*****************************************************************************

;* Call: ***************************************************************
MovePos:
	push	esi
	mov	esi,eax
	test	esi,esi
	je	.labl_01

	add	[_additionalOffset],esi
;----------------------------------------------------------
.labl_00:
	dec	esi
	call	lz_cm_09
	call	MatchFinder_MovePos
	test	esi,esi
	jne	.labl_00
;----------------------------------------------------------
.labl_01:
	pop	esi
	ret	
;*****************************************************************************

;* Call: ***************************************************************
Backward:
	push	ebp
	mov	ebp,esp
	push	ecx
	mov	ecx,[ebp+12]
	mov	eax,ecx
	imul	eax,eax,40
	mov	edx,[eax+state.PosPrev]
	mov	eax,[eax+state.BackPrev]
	push	ebx
	push	esi
	mov	[_optimumEndIndex],ecx
	mov	[ebp-4],eax
	push	edi
;----------------------------------------------------------
.labl_00:
	imul	ecx,ecx,40
	cmp	byte [ecx+state.Prev1IsChar],0
	je	.labl_01

	mov	eax,edx
	imul	eax,eax,40
	or	dword [eax+state.BackPrev],0xFFFFFFFF
	lea	esi,[eax+state.Prev1IsChar]
	lea	edi,[eax+state.PosPrev]
	lea	ebx,[edx-1]
	mov	byte [esi],0
	mov	[edi],ebx
	cmp	byte [ecx+state.Prev2],0
	je	.labl_01

	mov	byte [esi-40],0
	mov	esi,[ecx+state.PosPrev2]
	mov	[edi-40],esi
	mov	ecx,[ecx+state.BackPrev2]
	mov	[eax+state.BackPrev-40],ecx
;----------------------------------------------------------
.labl_01:
	mov	ecx,[ebp-4]
	mov	eax,edx
	imul	eax,eax,40
	mov	esi,edx
	mov	edx,[eax+state.BackPrev]
	mov	[eax+state.BackPrev],ecx
	mov	ecx,[ebp+12]
	mov	[ebp-4],edx
	mov	edx,[eax+state.PosPrev]
	mov	[eax+state.PosPrev],ecx
	mov	ecx,esi
	mov	[ebp+12],ecx
	test	esi,esi
	jne	.labl_00

	mov	eax,[state.BackPrev]
	mov	ecx,[ebp+8]
	pop	edi
	mov	[ecx],eax
	mov	eax,[state.PosPrev]
	pop	esi
	mov	[_optimumCurrentIndex],eax
	pop	ebx
	leave
	ret	8
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_09:
	push	ebp
	mov	ebp,esp
	mov	edx,[_matchMaxLen]
	mov	eax,[_pos]
	mov	ecx,[_streamPos]
	sub	esp,44
	push	esi
	lea	esi,[edx+eax]
	cmp	esi,ecx
	ja	.labl_00

	mov	[ebp-24],edx
	jmp	.labl_01
;----------------------------------------------------------
.labl_00:
	sub	ecx,eax
	mov	[ebp-24],ecx
	cmp	ecx,3
	jb	.labl_13
;----------------------------------------------------------
.labl_01:
	mov	ecx,[_cyclicBufferSize]
	mov	edx,eax
	sub	edx,ecx
	cmp	ecx,eax
	mov	ecx,[_buffer]
	lea	esi,[ecx+eax]
	movzx	ecx,byte [esi+2]
	push	ebx
	movzx	ebx,word [esi]
	push	edi
	sbb	edi,edi
	and	dword [ebp-28],0
	and	dword [ebp-40],0
	shl	ecx,8
	and	edi,edx
	movzx	edx,byte [esi]
	xor	ecx,[crc_table+edx*4]
	movzx	edx,byte [esi+1]
	xor	ecx,edx
	mov	edx,[_hash]
	and	ecx,0xFFFFF
	lea	ecx,[edx+ecx*4]
	mov	[edx+ebx*4+0x400000],eax
	mov	ebx,[ecx]
	mov	[ecx],eax
	mov	ecx,[pack_pos]
	lea	ecx,[edx+ecx*8+0x440000]
	mov	[ebp-4],ebx
	lea	ebx,[ecx+4]
	mov	[ebp-12],ecx
	mov	ecx,[_cutValue]
	mov	[ebp-16],ebx
	mov	[ebp-20],ecx
	cmp	[ebp-4],edi
	jbe	.labl_11
;----------------------------------------------------------
.labl_02:
	mov	ecx,[ebp-20]
	dec	dword [ebp-20]
	test	ecx,ecx
	je	.labl_11

	mov	ecx,[_buffer]
	mov	ebx,[ebp-4]
	add	ecx,ebx
	mov	[ebp-36],ecx
	mov	ecx,[ebp-40]
	cmp	ecx,[ebp-28]
	jb	.labl_03

	mov	ecx,[ebp-28]
;----------------------------------------------------------
.labl_03:
	mov	ebx,[ebp-36]
	mov	[ebp-8],ecx
	add	ecx,esi
	sub	ebx,esi
	mov	[ebp-32],ebx
	jmp	.labl_05
;----------------------------------------------------------
.labl_04:
	mov	ecx,[ebp-44]
	mov	ebx,[ebp-32]
;----------------------------------------------------------
.labl_05:
	mov	bl,[ebx+ecx]
	cmp	bl,[ecx]
	jne	.labl_06

	inc	dword [ebp-8]
	inc	ecx
	mov	[ebp-44],ecx
	mov	ecx,[ebp-8]
	cmp	ecx,[ebp-24]
	jne	.labl_04
;----------------------------------------------------------
.labl_06:
	mov	ecx,[pack_pos]
	mov	ebx,eax
	sub	ebx,[ebp-4]
	cmp	ebx,ecx
	ja	.labl_07

	sub	ecx,ebx
	jmp	.labl_08
;----------------------------------------------------------
.labl_07:
	sub	ecx,ebx
	add	ecx,[_cyclicBufferSize]
;----------------------------------------------------------
.labl_08:
	lea	ecx,[edx+ecx*8+0x440000]
	mov	[ebp-32],ecx
	mov	ecx,[ebp-8]
	cmp	ecx,[ebp-24]
	je	.labl_14

	mov	ebx,[ebp-36]
	mov	bl,[ecx+ebx]
	cmp	bl,[ecx+esi]
	mov	ecx,[ebp-4]
	jnb	.labl_09

	mov	ebx,[ebp-12]
	mov	[ebx],ecx
	mov	ecx,[ebp-32]
	mov	ebx,[ebp-8]
	add	ecx,4
	mov	[ebp-12],ecx
	mov	[ebp-28],ebx
	jmp	.labl_10
;----------------------------------------------------------
.labl_09:
	mov	ebx,[ebp-16]
	mov	[ebx],ecx
	mov	ecx,[ebp-32]
	mov	ebx,[ebp-8]
	mov	[ebp-16],ecx
	mov	[ebp-40],ebx
;----------------------------------------------------------
.labl_10:
	mov	ecx,[ecx]
	mov	[ebp-4],ecx
	cmp	ecx,edi
	ja	.labl_02
;----------------------------------------------------------
.labl_11:
	mov	eax,[ebp-12]
	and	dword [eax],0
	mov	eax,[ebp-16]
	and	dword [eax],0
;----------------------------------------------------------
.labl_12:
	pop	edi
	pop	ebx
;----------------------------------------------------------
.labl_13:
	pop	esi
	leave	
	ret	
;----------------------------------------------------------
.labl_14:
	mov	eax,[ebp-32]
	mov	ecx,[eax]
	mov	edx,[ebp-12]
	mov	[edx],ecx
	mov	eax,[eax+4]
	mov	ecx,[ebp-16]
	mov	[ecx],eax
	jmp	.labl_12
;*****************************************************************************

;* Call: ***************************************************************
GetPosSlot:
	cmp	eax,1024
	jnb	.labl_00

	movzx	eax,byte [eax+g_FastPos]
	ret 
;----------------------------------------------------------
.labl_00:
	cmp	eax,0x80000
	jnb	.labl_01

	shr	eax,9
	movzx	eax,byte [eax+g_FastPos]
	add	eax,18
	ret 
;----------------------------------------------------------
.labl_01:
	shr	eax,18
	movzx	eax,byte [eax+g_FastPos]
	add	eax,36 ;'$'
	ret 
;*****************************************************************************

;* Call: ***************************************************************
CPriceTableEncoder_Init:
	push	ebx
	push	edi
	mov	eax,1024
	push	4
	mov	[esi],eax
	mov	[esi+4],eax
	lea	edi,[esi+0x4048]
	pop	ebx
;----------------------------------------------------------
.labl_00:
	push	3
	lea	eax,[edi-0x4040]
	pop	ecx
	call	CBitTreeEncoder_Init
	push	3
	pop	ecx
	mov	eax,edi
	call	CBitTreeEncoder_Init
	add	edi,1028
	dec	ebx
	jne	.labl_00

	push	8
	pop	ecx
	pop	edi
	lea	eax,[esi+0x8088]
	pop	ebx
	jmp	CBitTreeEncoder_Init
;*****************************************************************************

;* Call: ***************************************************************
CPriceTableEncoder_UpdateTable:
	push	ebx
	push	edi
	mov	edi,eax
	xor	ebx,ebx
	cmp	[edi+0xC88C],ebx
	jbe	.labl_01

	mov	eax,[esp+12]
	push	esi
	lea	esi,[edi+eax*4+0x848C]
;----------------------------------------------------------
.labl_00:
	mov	eax,[esp+16]
	mov	edx,ebx
	call	NLength_CEncoder_GetPrice
	mov	[esi],eax
	inc	ebx
	add	esi,64
	cmp	ebx,[edi+0xC88C]
	jb	.labl_00

	pop	esi
;----------------------------------------------------------
.labl_01:
	mov	eax,[edi+0xC88C]
	mov	ecx,[esp+12]
	mov	[edi+ecx*4+0xC890],eax
	pop	edi
	pop	ebx
	ret	4
;*****************************************************************************

;* Call: ***************************************************************
NLength_CEncoder_GetPrice:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	esi
	cmp	edx,8
	jnb	.labl_00

	imul	eax,eax,1028
	lea	esi,[eax+edi+8]
	call	CBitTreeEncoder_GetPrice
	mov	ecx,[edi]
	shr	ecx,2
	add	eax,[Models+ecx*4]
	jmp	.labl_03
;----------------------------------------------------------
.labl_00:
	push	ebx
	mov	ebx,2048
	mov	ecx,ebx
	sub	ecx,[edi]
	shr	ecx,2
	mov	ecx,[Models+ecx*4]
	mov	[ebp-4],ecx
	cmp	edx,16
	jnb	.labl_01

	imul	eax,eax,1028
	add	edx,0xFFFFFFF8
	lea	esi,[eax+edi+0x4048]
	call	CBitTreeEncoder_GetPrice
	mov	ecx,[edi+4]
	shr	ecx,2
	add	eax,[Models+ecx*4]
	jmp	.labl_02
;----------------------------------------------------------
.labl_01:
	add	edx,0xFFFFFFF0
	lea	esi,[edi+0x8088]
	call	CBitTreeEncoder_GetPrice
	sub	ebx,[edi+4]
	shr	ebx,2
	add	eax,[Models+ebx*4]
;----------------------------------------------------------
.labl_02:
	add	[ebp-4],eax
	mov	eax,[ebp-4]
	pop	ebx
;----------------------------------------------------------
.labl_03:
	pop	esi
	leave	
	ret	
;*****************************************************************************

;* Call: ***************************************************************
CPriceTableEncoder_Encode:
	push	edi
	mov	edi,eax
	mov	edx,esi
	cmp	edi,8
	jnb	.labl_00

	push	0
	call	CMyBitEncoder_Encode
	mov	eax,ebx
	imul	eax,eax,1028
	push	edi
	lea	edi,[eax+esi+8]
	jmp	.labl_02
;----------------------------------------------------------
.labl_00:
	push	1
	call	CMyBitEncoder_Encode
	lea	edx,[esi+4]
	cmp	edi,16
	jnb	.labl_01

	push	0
	call	CMyBitEncoder_Encode
	add	edi,0xFFFFFFF8
	mov	eax,ebx
	imul	eax,eax,1028
	push	edi
	lea	edi,[eax+esi+0x4048]
	jmp	.labl_02
;----------------------------------------------------------
.labl_01:
	push	1
	call	CMyBitEncoder_Encode
	add	edi,0xFFFFFFF0
	push	edi
	lea	edi,[esi+0x8088]
;----------------------------------------------------------
.labl_02:
	call	cm_pr_16
	dec	dword [esi+ebx*4+0xC890]
	pop	edi
	jne	.labl_03

	push	ebx
	mov	eax,esi
	call	CPriceTableEncoder_UpdateTable
;----------------------------------------------------------
.labl_03:
	ret	
;*****************************************************************************

;* Call: ***************************************************************
CLiteralEncoder2_Encode:
	push	ebx
	push	esi
	push	edi
	xor	edi,edi
	push	8
	inc	edi
	pop	ebx
;----------------------------------------------------------
.labl_00:
	movzx	esi,byte [esp+20]
	mov	eax,[esp+16]
	dec	ebx
	mov	cl,bl
	shr	esi,cl
	lea	edx,[eax+edi*4]
	and	esi,1
	push	esi
	call	CMyBitEncoder_Encode
	add	edi,edi
	or	edi,esi
	test	ebx,ebx
	jne	.labl_00

	pop	edi
	pop	esi
	pop	ebx
	ret	8
;*****************************************************************************

;* Call: ***************************************************************
CState_IsCharState:
	mov	edx,[_numPrevBits]
	push	esi
	mov	esi,[_posMask]
	and	esi,[esp+8]
	push	8
	pop	ecx
	movzx	eax,al
	sub	cl,dl
	shr	eax,cl
	mov	ecx,edx
	shl	esi,cl
	add	eax,esi
	pop	esi
	ret	4
;*****************************************************************************

;* Call: ***************************************************************
CLiteralEncoder_GetPrice:
	push	dword [esp+16]
	push	dword [esp+16]
	push	dword [esp+16]
	push	dword [esp+16]
	call	CState_IsCharState
	imul	eax,eax,3072
	add	eax,[_posEncoders]
	push	eax
	call	lz_cm_11
	ret	16
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_11:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	ebx
	push	esi
	xor	esi,esi
	xor	eax,eax
	inc	esi
	push	edi
	mov	dword [ebp-4],8
	cmp	[ebp+12],al
	je	.labl_02

	movzx	ecx,byte [ebp+20]
	mov	[ebp+12],ecx
;----------------------------------------------------------
.labl_00:
	dec	dword [ebp-4]
	movzx	ecx,byte [ebp-4]
	movzx	edi,byte [ebp+16]
	mov	edx,[ebp+12]
	mov	ebx,[ebp+8]
	shr	edi,cl
	shr	edx,cl
	and	edi,1
	mov	ecx,edi
	shl	ecx,8
	add	ecx,esi
	mov	ecx,[ebx+ecx*4+1024]
	and	edx,1
	sub	ecx,edx
	mov	ebx,edx
	neg	ebx
	xor	ecx,ebx
	shr	ecx,2
	and	ecx,0x1FF
	add	eax,[Models+ecx*4]
	add	esi,esi
	or	esi,edx
	cmp	edi,edx
	jne	.labl_01

	cmp	dword [ebp-4],0
	jne	.labl_00

	jmp	.labl_04
;----------------------------------------------------------
.labl_01:
	cmp	dword [ebp-4],0
	je	.labl_04
;----------------------------------------------------------
.labl_02:
	movzx	edi,byte [ebp+20]
;----------------------------------------------------------
.labl_03:
	dec	dword [ebp-4]
	mov	cl,[ebp-4]
	mov	edx,edi
	shr	edx,cl
	mov	ecx,[ebp+8]
	mov	ecx,[ecx+esi*4]
	add	esi,esi
	and	edx,1
	sub	ecx,edx
	mov	ebx,edx
	neg	ebx
	xor	ecx,ebx
	shr	ecx,2
	and	ecx,0x1FF
	add	eax,[Models+ecx*4]
	or	esi,edx
	cmp	dword [ebp-4],0
	jne	.labl_03
;----------------------------------------------------------
.labl_04:
	pop	edi
	pop	esi
	pop	ebx
	leave	
	ret	16
;*****************************************************************************

;* Call: ***************************************************************
ReadMatchDistances:
	push	esi
	call	GetLongestMatch
	mov	ecx,eax
	mov	esi,0x80
	cmp	ecx,esi
	jne	.labl_00

	push	dword [distances+512]
	lea	eax,[esi+17]
	push	esi
	call	GetMatchLen
	mov	ecx,eax
	add	ecx,esi
;----------------------------------------------------------
.labl_00:
	inc	dword [_additionalOffset]
	call	MatchFinder_MovePos
	mov	eax,ecx
	pop	esi
	ret	
;*****************************************************************************

;* Call: ***************************************************************
GetLongestMatch:
	push	ebp
	mov	ebp,esp
	sub	esp,0x34
	mov	edx,[_matchMaxLen]
	mov	ecx,[_pos]
	mov	eax,[_streamPos]
	and	dword [ebp-24],0
	push	esi
	lea	esi,[edx+ecx]
	cmp	esi,eax
	ja	.labl_02

	mov	[ebp-32],edx
;----------------------------------------------------------
.labl_00:
	mov	eax,[_cyclicBufferSize]
	mov	edx,ecx
	sub	edx,eax
	cmp	eax,ecx
	sbb	eax,eax
	and	eax,edx
	mov	[ebp-40],eax
	mov	eax,[_buffer]
	push	ebx
	push	edi
	lea	edi,[eax+ecx]
	movzx	edx,byte [edi+2]
	movzx	esi,byte [edi]
	movzx	eax,word [edi]
	shl	edx,8
	xor	edx,[crc_table+esi*4]
	movzx	esi,byte [edi+1]
	xor	edx,esi
	mov	esi,[_hash]
	and	edx,0xFFFFF
	lea	edx,[esi+edx*4]
	mov	ebx,[edx]
	lea	eax,[esi+eax*4+0x400000]
	mov	[ebp-8],ebx
	mov	ebx,[eax]
	mov	[eax],ecx
	or	dword [distances+8],0xFFFFFFFF
	cmp	ebx,[ebp-40]
	jbe	.labl_01

	mov	eax,ecx
	sub	eax,ebx
	dec	eax
	mov	[distances+8],eax
	mov	dword [ebp-24],2
;----------------------------------------------------------
.labl_01:
	mov	ebx,[pack_pos]
	and	dword [ebp-36],0
	mov	[edx],ecx
	or	dword [distances+12],0xFFFFFFFF
	and	dword [ebp-48],0
	lea	edx,[esi+ebx*8+0x440000]
	lea	eax,[edx+4]
	mov	[ebp-20],eax
	mov	eax,[_cutValue]
	mov	[ebp-28],eax
	mov	eax,[ebp-8]
	mov	[ebp-16],edx
	jmp	.labl_13
;----------------------------------------------------------
.labl_02:
	sub	eax,ecx
	mov	[ebp-32],eax
	cmp	eax,3
	jnb	.labl_00

	xor	eax,eax
	jmp	.labl_16
;----------------------------------------------------------
.labl_03:
	mov	eax,[ebp-28]
	dec	dword [ebp-28]
	test	eax,eax
	je	.labl_14

	mov	eax,[_buffer]
	mov	edx,[ebp-8]
	add	eax,edx
	mov	[ebp-44],eax
	mov	eax,[ebp-48]
	cmp	eax,[ebp-36]
	jb	.labl_04

	mov	eax,[ebp-36]
;----------------------------------------------------------
.labl_04:
	mov	edx,[ebp-44]
	mov	[ebp-4],eax
	add	eax,edi
	sub	edx,edi
	mov	[ebp-12],edx
	jmp	.labl_06
;----------------------------------------------------------
.labl_05:
	mov	edx,[ebp-12]
	mov	eax,[ebp-52]
;----------------------------------------------------------
.labl_06:
	mov	dl,[edx+eax]
	cmp	dl,[eax]
	jne	.labl_07

	inc	dword [ebp-4]
	inc	eax
	mov	[ebp-52],eax
	mov	eax,[ebp-4]
	cmp	eax,[ebp-32]
	jne	.labl_05
;----------------------------------------------------------
.labl_07:
	mov	eax,ecx
	sub	eax,[ebp-8]
	mov	[ebp-12],eax
	mov	eax,[ebp-24]
	cmp	eax,[ebp-4]
	jnb	.labl_09
;----------------------------------------------------------
.labl_08:
	mov	edx,[ebp-12]
	inc	eax
	dec	edx
	mov	[distances+eax*4],edx
	cmp	eax,[ebp-4]
	jb	.labl_08

	mov	[ebp-24],eax
;----------------------------------------------------------
.labl_09:
	mov	eax,ebx
	sub	eax,[ebp-12]
	cmp	[ebp-12],ebx
	jbe	.labl_10

	add	eax,[_cyclicBufferSize]
;----------------------------------------------------------
.labl_10:
	lea	eax,[esi+eax*8+0x440000]
	mov	[ebp-12],eax
	mov	eax,[ebp-4]
	cmp	eax,[ebp-32]
	je	.labl_17

	mov	edx,[ebp-44]
	mov	dl,[eax+edx]
	cmp	dl,[eax+edi]
	mov	eax,[ebp-8]
	jnb	.labl_11

	mov	edx,[ebp-16]
	mov	[edx],eax
	mov	eax,[ebp-12]
	mov	edx,[ebp-4]
	add	eax,4
	mov	[ebp-16],eax
	mov	[ebp-36],edx
	jmp	.labl_12
;----------------------------------------------------------
.labl_11:
	mov	edx,[ebp-20]
	mov	[edx],eax
	mov	eax,[ebp-12]
	mov	edx,[ebp-4]
	mov	[ebp-20],eax
	mov	[ebp-48],edx
;----------------------------------------------------------
.labl_12:
	mov	eax,[eax]
	mov	[ebp-8],eax
;----------------------------------------------------------
.labl_13:
	cmp	eax,[ebp-40]
	ja	.labl_03
;----------------------------------------------------------
.labl_14:
	mov	eax,[ebp-16]
	and	dword [eax],0
	mov	eax,[ebp-20]
	and	dword [eax],0
;----------------------------------------------------------
.labl_15:
	mov	eax,[ebp-24]
	pop	edi
	pop	ebx
;----------------------------------------------------------
.labl_16:
	pop	esi
	leave	
	ret	
;----------------------------------------------------------
.labl_17:
	mov	eax,[ebp-12]
	mov	ecx,[eax]
	mov	edx,[ebp-16]
	mov	[edx],ecx
	mov	eax,[eax+4]
	mov	ecx,[ebp-20]
	mov	[ecx],eax
	jmp	.labl_15
;*****************************************************************************

;* Call: ***************************************************************
FillPosSlotPrices:
	push	ecx
	push	ebx
	push	ebp
	push	esi
	push	edi
	xor	ebp,ebp
	mov	esi,_posSlotEncoder
	mov	dword [esp+16],4
;----------------------------------------------------------
.labl_00:
	xor	edi,edi
;----------------------------------------------------------
.labl_01:
	mov	ebx,[_distTableSize]
	cmp	edi,ebx
	jnb	.labl_03

	mov	edx,edi
	call	CBitTreeEncoder_GetPrice
	lea	ecx,[edi+ebp]
	inc	edi
	mov	[_posSlotPrices+ecx*4],eax
	cmp	edi,14
	jb	.labl_01

	cmp	edi,ebx
	jnb	.labl_03

	lea	ebx,[edi+ebp]
	lea	ebx,[_posSlotPrices+ebx*4]
;----------------------------------------------------------
.labl_02:
	mov	edx,edi
	call	CBitTreeEncoder_GetPrice
	mov	ecx,edi
	shr	ecx,1
	sub	ecx,5
	shl	ecx,6
	add	eax,ecx
	mov	[ebx],eax
	inc	edi
	add	ebx,4
	cmp	edi,[_distTableSize]
	jb	.labl_02
;----------------------------------------------------------
.labl_03:
	add	esi,1028
	add	ebp,64
	dec	dword [esp+16]
	jne	.labl_00

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	ecx
	ret	
;***********************************************************************

;* Call: ***************************************************************
FillDistancesPrices:
	push	ebp
	mov	ebp,esp
	sub	esp,36
	and	dword [ebp-20],0
	push	ebx
	push	esi
	push	edi
	push	4
	pop	ebx
	mov	dword [ebp-24],_distancesPrices+16
	mov	dword [ebp-16],_posSlotPrices
	mov	[ebp-36],ebx
;----------------------------------------------------------
.labl_00:
	mov	eax,[ebp-24]
	mov	esi,[ebp-16]
	lea	edi,[eax-16]
	movsd	
	movsd	
	movsd	
	movsd	
	mov	[ebp-4],ebx
	mov	[ebp-12],eax
;----------------------------------------------------------
.labl_01:
	mov	esi,[ebp-4]
	mov	eax,esi
	call	GetPosSlot
	mov	edx,eax
	mov	ecx,edx
	xor	eax,eax
	inc	eax
	mov	[ebp-32],edx
	and	edx,eax
	shr	ecx,1
	dec	ecx
	or	edx,2
	shl	edx,cl
	xor	edi,edi
	mov	[ebp-8],eax
	mov	[ebp-28],ecx
	sub	esi,edx
	test	ecx,ecx
	je	.labl_03
;----------------------------------------------------------
.labl_02:
	mov	eax,[ebp-8]
	sub	eax,[ebp-32]
	mov	ecx,esi
	add	eax,edx
	mov	eax,[_numLiteralContextBits+eax*4]
	and	ecx,1
	sub	eax,ecx
	mov	ebx,ecx
	neg	ebx
	xor	eax,ebx
	shr	eax,2
	and	eax,0x1FF
	add	edi,[Models+eax*4]
	mov	eax,[ebp-8]
	shr	esi,1
	dec	dword [ebp-28]
	lea	eax,[ecx+eax*2]
	mov	[ebp-8],eax
	jne	.labl_02

	push	4
	pop	ebx
;----------------------------------------------------------
.labl_03:
	mov	eax,[ebp-32]
	mov	ecx,[ebp-20]
	add	ecx,eax
	mov	eax,[_posSlotPrices+ecx*4]
	mov	ecx,[ebp-12]
	add	[ebp-12],ebx
	add	eax,edi
	inc	dword [ebp-4]
	cmp	dword [ebp-4],0x80
	mov	[ecx],eax
	jb	.labl_01

	add	dword [ebp-16],256
	add	dword [ebp-20],64
	add	dword [ebp-24],512
	dec	dword [ebp-36]
	jne	.labl_00

	pop	edi
	pop	esi
	pop	ebx
	leave	
	ret	
;***********************************************************************

;* Call: ***************************************************************
FillAlignPrices:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	ecx
	push	ebx
	push	esi
	xor	esi,esi
	push	edi
	mov	[ebp-8],esi
;----------------------------------------------------------
.labl_00:
	mov	eax,[i_01]
	xor	edx,edx
	xor	ecx,ecx
	inc	edx
	mov	[ebp-4],eax
;----------------------------------------------------------
.labl_01:
	mov	edi,[_posAlignEncoder+edx*4]
	mov	eax,esi
	and	eax,1
	sub	edi,eax
	mov	ebx,eax
	neg	ebx
	xor	edi,ebx
	shr	edi,2
	and	edi,0x1FF
	add	ecx,[Models+edi*4]
	shr	esi,1
	dec	dword [ebp-4]
	lea	edx,[eax+edx*2]
	jne	.labl_01

	mov	esi,[ebp-8]
	push	16
	mov	[_alignPrices+esi*4],ecx
	inc	esi
	pop	eax
	mov	[ebp-8],esi
	cmp	esi,eax
	jb	.labl_00

	pop	edi
	pop	esi
	mov	[_alignPriceCount],eax
	pop	ebx
	leave	
	ret	
;***********************************************************************

;* Call: ***************************************************************
GetRepLen1Price:
	movzx	eax,al
	mov	edx,[_isRepG0+eax*4]
	mov	ecx,eax
	shl	ecx,4
	add	ecx,[esp+4]
	shr	edx,2
	mov	ecx,[_isRep0Long+ecx*4]
	shr	ecx,2
	mov	eax,[Models+ecx*4]
	add	eax,[Models+edx*4]
	ret	4
;***********************************************************************

;* Call: ***************************************************************
GetRepPrice:
	movzx	ecx,byte [esp+4]
	shl	eax,4
	add	eax,[esp+8]
	push	esi
	mov	eax,[_repMatchLenEncoder+0x840C+eax*4]
	mov	esi,2048
	test	edx,edx
	jne	.labl_00

	mov	edx,ecx
	mov	ecx,[_isRepG0+ecx*4]
	shl	edx,4
	add	edx,[esp+12]
	sub	esi,[_isRep0Long+edx*4]
	shr	ecx,2
	jmp	.labl_02
;----------------------------------------------------------
.labl_00:
	push	edi
	shl	ecx,2
	mov	edi,esi
	sub	edi,[ecx+_isRepG0]
	shr	edi,2
	add	eax,[Models+edi*4]
	pop	edi
	cmp	edx,1
	jne	.labl_01

	mov	ecx,[ecx+_isRepG1]
	shr	ecx,2
	add	eax,[Models+ecx*4]
	jmp	.labl_03
;----------------------------------------------------------
.labl_01:
	sub	esi,[ecx+_isRepG1]
	mov	ecx,[ecx+_isRepG2]
	add	edx,0xFFFFFFFE
	sub	ecx,edx
	neg	edx
	xor	ecx,edx
	shr	ecx,2
	and	ecx,0x1FF
;----------------------------------------------------------
.labl_02:
	shr	esi,2
	mov	edx,[Models+esi*4]
	add	edx,[Models+ecx*4]
	add	eax,edx
;----------------------------------------------------------
.labl_03:
	pop	esi
	ret	8
;***********************************************************************

;* Call: ***************************************************************
GetPosLenPrice:
	push	esi
	mov	esi,eax
	mov	ecx,0x80
	cmp	esi,2
	jne	.labl_00

	cmp	edx,ecx
	jb	.labl_01

	mov	eax,kIfinityPrice
	jmp	.labl_09
;----------------------------------------------------------
.labl_00:
	cmp	esi,6
	jnb	.labl_02
;----------------------------------------------------------
.labl_01:
	lea	eax,[esi-2]
	jmp	.labl_03
;----------------------------------------------------------
.labl_02:
	push	3
	pop	eax
;----------------------------------------------------------
.labl_03:
	cmp	edx,ecx
	jnb	.labl_04

	shl	eax,7
	add	eax,edx
	mov	ecx,[_distancesPrices+eax*4]
	jmp	.labl_08
;----------------------------------------------------------
.labl_04:
	mov	ecx,edx
;GetPosSlot2
	cmp	edx,0x10000	;if (pos < (1<<16))
	jnb	.labl_05

	shr	ecx,6
	movzx	ecx,byte [ecx+g_FastPos]
	add	ecx,12
	jmp	.labl_07
;----------------------------------------------------------
.labl_05:
	cmp	edx,0x2000000	;if (pos < (1<<25))
	jnb	.labl_06

	shr	ecx,15
	movzx	ecx,byte [ecx+g_FastPos]
	add	ecx,30
	jmp	.labl_07
;----------------------------------------------------------
.labl_06:
	shr	ecx,24
	movzx	ecx,byte [ecx+g_FastPos]
	add	ecx,48
;----------------------------------------------------------
.labl_07:
	shl	eax,6
	add	eax,ecx
	mov	ecx,[_posSlotPrices+eax*4]
	and	edx,15
	add	ecx,[_alignPrices+edx*4]
;----------------------------------------------------------
.labl_08:
	shl	esi,4
	add	esi,[esp+8]
	mov	eax,[_prices+esi*4]
	add	eax,ecx
;----------------------------------------------------------
.labl_09:
	pop	esi
	ret	4
;***********************************************************************

;* Call: ***************************************************************
RangeEncoder_ShiftLow:
	push	ecx
	push	ecx
	mov	eax,[low]
	push	esi
	mov	esi,[low+4]
	cmp	eax,0xFF000000
	jb	.labl_00

	test	esi,esi
	je	.labl_02
;----------------------------------------------------------
.labl_00:
	mov	cl,[_cache]
	and	dword [esp+8],0
	mov	edx,esi
	push	ebx
;----------------------------------------------------------
.labl_01:
	mov	bl,dl
	add	bl,cl
	mov	ecx,[pack_length]
	inc	dword [pack_length]
	mov	[ecx],bl
	or	cl,0xFF
	dec	dword [_cacheSize]
	jne	.labl_01

	mov	cl,[low+3]
	mov	[_cache],cl
	pop	ebx
;----------------------------------------------------------
.labl_02:
	inc	dword [_cacheSize]
	shl	eax,8
	and	dword [low+4],0
	mov	[low],eax
	pop	esi
	pop	ecx
	pop	ecx
	ret	
;***********************************************************************

;* Call: ***************************************************************
CMyBitEncoder_Encode:
	mov	ecx,[range]
	mov	eax,[edx]
	shr	ecx,0xB
	imul	ecx,eax
	cmp	dword [esp+4],0
	jne	.labl_00

	mov	[range],ecx
	mov	ecx,2048
	sub	ecx,eax
	shr	ecx,5
	add	ecx,eax
	mov	[edx],ecx
	jmp	.labl_01
;----------------------------------------------------------
.labl_00:
	add	[low],ecx
	adc	dword [low+4],0
	sub	[range],ecx
	mov	ecx,eax
	shr	ecx,5
	sub	eax,ecx
	mov	[edx],eax
;----------------------------------------------------------
.labl_01:
	cmp	dword [range],0x1000000
	jnb	.labl_02

	shl	dword [range],8
	call	RangeEncoder_ShiftLow
;----------------------------------------------------------
.labl_02:
	ret	4
;***********************************************************************

;* Call: ***************************************************************
CBitTreeEncoder_Init:
	xor	edx,edx
	inc	edx
	shl	edx,cl
	mov	[eax+1024],ecx
	lea	ecx,[edx-1]
	test	ecx,ecx
	jbe	.labl_00

	push	edi
	lea	edi,[eax+4]
	mov	eax,1024
	rep	stosd	
	pop	edi
;----------------------------------------------------------
.labl_00:
	ret	
;***********************************************************************

;* Call: ***************************************************************
cm_pr_16:
	mov	eax,[esp+4]
	push	ebx
	mov	ebx,[edi+1024]
	push	esi
	xor	esi,esi
	inc	esi
	add	eax,eax
	mov	[esp+12],eax
;----------------------------------------------------------
.labl_00:
	mov	eax,[esp+12]
	mov	ecx,ebx
	shr	eax,cl
	lea	edx,[edi+esi*4]
	and	eax,1
	push	eax
	lea	esi,[eax+esi*2]
	call	CMyBitEncoder_Encode
	dec	ebx
	jne	.labl_00

	pop	esi
	pop	ebx
	ret	4
;***********************************************************************

;* Call: ***************************************************************
CBitTreeEncoder_GetPrice:
	mov	ecx,[esi+1024]
	push	edi
	xor	edi,edi
	inc	edi
	shl	edi,cl
	xor	eax,eax
	or	edx,edi
;----------------------------------------------------------
.labl_00:
	mov	ecx,edx
	shr	edx,1
	mov	edi,[esi+edx*4]
	and	ecx,1
	sub	edi,ecx
	neg	ecx
	xor	edi,ecx
	shr	edi,2
	and	edi,0x1FF
	add	eax,[Models+edi*4]
	cmp	edx,1
	jne	.labl_00

	pop	edi
	ret	
;***********************************************************************

;* Call: ***************************************************************
GetMatchLen:
	mov	ecx,[_streamPos]
	push	edi
	mov	edi,eax
	mov	eax,[_pos]
	sub	ecx,eax
	sub	ecx,[esp+8]
	cmp	edi,ecx
	jbe	.labl_00

	mov	edi,ecx
;----------------------------------------------------------
.labl_00:
	mov	ecx,[_buffer]
	inc	dword [esp+12]
	add	ecx,eax
	add	ecx,[esp+8]
	xor	eax,eax
	test	edi,edi
	jbe	.labl_03

	push	esi
	mov	esi,ecx
	sub	esi,[esp+16]
;----------------------------------------------------------
.labl_01:
	mov	dl,[eax+ecx]
	cmp	dl,[esi+eax]
	jne	.labl_02

	inc	eax
	cmp	eax,edi
	jb	.labl_01
;----------------------------------------------------------
.labl_02:
	pop	esi
;----------------------------------------------------------
.labl_03:
	pop	edi
	ret	8
;***********************************************************************

;* Call: ***************************************************************
MatchFinder_MovePos:
	inc	dword [pack_pos]
	mov	eax,[pack_pos]
	cmp	eax,[_cyclicBufferSize]
	jne	.labl_00

	and	dword [pack_pos],0
;----------------------------------------------------------
.labl_00:
	inc	dword [_pos]
	ret	
;***********************************************************************
