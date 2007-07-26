; LZMA decoder for *.7z archives.
; Based on C decoder in LZMA SDK (c) Igor Pavlov.
; Portions by Diamond, 2006, 2007.
lzma_decoder:
virtual at 0
.outStream      rb      streamInfo.size
.inStream       dd      ?

; RangeDecoder data
.inLen          dd      ?
.inPtr          dd      ?
.code           dd      ?
.range          dd      ?

; parameters
.pb             db      ?       ; pos state bits (0 - 4)
.lp             db      ?       ; literal pos state bits (0 - 4)
.lc             db      ?       ; literal context bits (0 - 8)
.previousByte   db      ?
.posStateMask   dd      ?       ; (1 shl .pb)-1
.literalPosMask dd      ?       ; (1 shl .lp)-1

; constants
.kNumPosBitsMax	=	4
.kNumPosStatesMax =	(1 shl .kNumPosBitsMax)

.kLenNumLowBits         =       3
.kLenNumLowSymbols      =       (1 shl .kLenNumLowBits)
.kLenNumMidBits         =       3
.kLenNumMidSymbols      =       (1 shl .kLenNumMidBits)
.kLenNumHighBits        =       8
.kLenNumHighSymbols     =       (1 shl .kLenNumHighBits)

.LenChoice	=	0
.LenChoice2	=	1
.LenLow		=	2
.LenMid		=	(.LenLow + (.kNumPosStatesMax shl .kLenNumLowBits))
.LenHigh	=	(.LenMid + (.kNumPosStatesMax shl .kLenNumMidBits))
.kNumLenProbs	=	(.LenHigh + .kLenNumHighSymbols)

.kNumStates	=	12
.kNumLitStates	=	7
.kStartPosModelIndex =	4
.kEndPosModelIndex =	14
.kNumFullDistances =	(1 shl (.kEndPosModelIndex/2))
.kNumPosSlotBits =	6
.kNumLenToPosStates =	4
.kNumAlignBits	=	4
.kAlignTableSize =	(1 shl .kNumAlignBits)
.kMatchMinLen	=	2

.IsMatch	=	0
.IsRep		=	(.IsMatch + (.kNumStates shl .kNumPosBitsMax))
.IsRepG0	=	(.IsRep + .kNumStates)
.IsRepG1	=	(.IsRepG0 + .kNumStates)
.IsRepG2	=	(.IsRepG1 + .kNumStates)
.IsRep0Long	=	(.IsRepG2 + .kNumStates)
.PosSlot	=	(.IsRep0Long + (.kNumStates shl .kNumPosBitsMax))
.SpecPos	=	(.PosSlot + (.kNumLenToPosStates shl .kNumPosSlotBits))
.Align_		=	(.SpecPos + .kNumFullDistances - .kEndPosModelIndex)
.Lencoder	=	(.Align_ + .kAlignTableSize)
.RepLencoder	=	(.Lencoder + .kNumLenProbs)
.Literal	=	(.RepLencoder + .kNumLenProbs)

LZMA_BASE_SIZE	=	1846	; must be ==Literal
LZMA_LIT_SIZE	=	768

.kNumTopBits	=	24
.kTopValue	=	(1 shl .kNumTopBits)

.kNumBitModelTotalBits =	11
.kBitModelTotal	=	(1 shl .kNumBitModelTotalBits)
.kNumMoveBits	=	5

; variables
.continue       dd      ?
.ecx            dd      ?
.outEnd         dd      ?
.dictSize       dd      ?
.state          dd      ?
.rep0           dd      ?
.rep1           dd      ?
.rep2           dd      ?
.rep3           dd      ?
.p              rd      LZMA_BASE_SIZE
.basesize = $
;               rd      LZMA_LIT_SIZE shl (.lc+.lp)
end virtual

.fillBuf:
        mov     ebp, eax
        mov     ebx, [ebp+.state]
	jecxz	.nodata
        add     ecx, edi
        mov     [ebp+.outEnd], ecx
        mov     esi, [ebp+.inPtr]
        jmp     [ebp+.continue]
.nodata:
	popad
	ret
.start:
        mov     eax, [ebp+.inStream]
        call    fillBuf
        mov     esi, [eax+streamInfo.bufPtr]
        mov     eax, [eax+streamInfo.bufDataLen]
        sub     eax, 5
        jb      return.err
        mov     [ebp+.inLen], eax
        inc     esi
	lodsd
        bswap   eax
	mov	[ebp+.code], eax
	or	[ebp+.range], -1
.main_loop:
	cmp	edi, [ebp+.outEnd]
	jae	.main_loop_done
	mov	edx, edi
	and	edx, [ebp+.posStateMask]
	mov	eax, ebx
	shl	eax, .kNumPosBitsMax
        add     eax, edx
	lea	eax, [ebp + .p + .IsMatch*4 + eax*4]
	call	.RangeDecoderBitDecode
	jc	.1
	movzx	eax, [ebp+.previousByte]
	mov	ah, dl
	and	ah, byte [ebp+.literalPosMask]
        mov     cl, 8
        sub     cl, [ebp+.lc]
	shr	eax, cl
	imul	eax, LZMA_LIT_SIZE*4
	lea	eax, [ebp + eax + .p+.Literal*4]
	cmp	ebx, .kNumLitStates
	jb	.literal
	xor	edx, edx
	sub	edx, [ebp+.rep0]
	mov	dl, [edi + edx]
	call	.LzmaLiteralDecodeMatch
	jmp	@f
.literal:
	call	.LzmaLiteralDecode
@@:
	mov	[ebp+.previousByte], al
	stosb
	mov	al, bl
	cmp	bl, 4
	jb	@f
	mov	al, 3
	cmp	bl, 10
	jb	@f
	mov	al, 6
@@:	sub	bl, al
	jmp	.main_loop
.1:
	lea	eax, [ebp + .p + .IsRep*4 + ebx*4]
	call	.RangeDecoderBitDecode
	jnc	.10
	lea	eax, [ebp + .p + .IsRepG0*4 + ebx*4]
	call	.RangeDecoderBitDecode
	jc	.111
	mov	eax, ebx
	shl	eax, .kNumPosBitsMax
        add     eax, edx
	lea	eax, [ebp + .p + .IsRep0Long*4 + eax*4]
	call	.RangeDecoderBitDecode
	jc	.1101
	cmp	bl, 7
	setae	bl
	lea	ebx, [9 + ebx + ebx]
	xor	edx, edx
	sub	edx, [ebp+.rep0]
	mov	al, [edi + edx]
	stosb
	mov	[ebp+.previousByte], al
	jmp	.main_loop
.111:
	lea	eax, [ebp + .p + .IsRepG1*4 + ebx*4]
	call	.RangeDecoderBitDecode
	mov	eax, [ebp+.rep1]
	jnc	.l3
.l1:
	lea	eax, [ebp + .p + .IsRepG2*4 + ebx*4]
	call	.RangeDecoderBitDecode
	mov	eax, [ebp+.rep2]
	jnc	.l2
	xchg	[ebp+.rep3], eax
.l2:
	push	[ebp+.rep1]
	pop	[ebp+.rep2]
.l3:
	xchg	eax, [ebp+.rep0]
	mov	[ebp+.rep1], eax
.1101:
	lea	eax, [ebp + .p + .RepLencoder*4]
	call	.LzmaLenDecode
	cmp	bl, 7
	setc	bl
	adc	bl, bl
	xor	bl, 3
	add	bl, 8
	jmp	.repmovsb
.10:
	mov	eax, [ebp+.rep0]
	xchg	eax, [ebp+.rep1]
	xchg	eax, [ebp+.rep2]
	xchg	eax, [ebp+.rep3]
	cmp	bl, 7
	setc	bl
	adc	bl, bl
	xor	bl, 3
	add	bl, 7
	lea	eax, [ebp + .p + .Lencoder*4]
	call	.LzmaLenDecode
	mov	eax, .kNumLenToPosStates-1
	cmp	eax, ecx
	jb	@f
	mov	eax, ecx
@@:
	push	ecx
	mov	ecx, .kNumPosSlotBits
	shl	eax, cl
        lea     eax, [ebp + .p+.PosSlot*4 + eax*4]
	call	.RangeDecoderBitTreeDecode
	mov	[ebp+.rep0], ecx
	cmp	ecx, .kStartPosModelIndex
	jb	.l6
	push	ecx
	mov	eax, ecx
	and	eax, 1
	shr	ecx, 1
	or	eax, 2
	dec	ecx
	shl	eax, cl
	mov	[ebp+.rep0], eax
	pop	edx
	cmp	edx, .kEndPosModelIndex
	jae	.l5
	sub	eax, edx
        lea     eax, [ebp + .p + (.SpecPos - 1)*4 + eax*4]
	call	.RangeDecoderReverseBitTreeDecode
	add	[ebp+.rep0], ecx
	jmp	.l6
.l5:
	sub	ecx, .kNumAlignBits
	call	.RangeDecoderDecodeDirectBits
	mov	ecx, .kNumAlignBits
	shl	eax, cl
	add	[ebp+.rep0], eax
	lea	eax, [ebp+.p+.Align_*4]
	call	.RangeDecoderReverseBitTreeDecode
	add	[ebp+.rep0], ecx
.l6:
	pop	ecx
	inc	[ebp+.rep0]
	jz	.main_loop_done
.repmovsb:
	add	ecx, .kMatchMinLen
.repmovsbz:
	push	esi
.repmovsbr:
        mov     eax, [ebp+.rep0]
        cmp     eax, [ebp+.dictSize]
        jae     return.err
	mov	esi, edi
	sub	esi, eax
        mov     eax, [ebp+streamInfo.bufPtr]
        sub     eax, esi
        ja      .repmovsb0
	mov	eax, [ebp+.outEnd]
        sub     eax, edi
        push    ecx
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
        sub     [esp], ecx
	rep	movsb
        pop     ecx
        jz      .repmovsb1
        pop     [ebp+.inPtr]
	mov	[ebp+.state], ebx
        mov     [ebp+.ecx], ecx
        mov     [ebp+.continue], .restart_repmovsb
        popad
        ret
.repmovsb0:
	mov	edx, [ebp+.dictSize]
	cmp	edx, [ebp+streamInfo.bufSize]
	jnz	return.err
        add     esi, edx
        push    ecx
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
	mov	eax, [ebp+.outEnd]
	sub	eax, edi
	cmp	ecx, eax
	jb	@f
	mov	ecx, eax
@@:
        sub     [esp], ecx
        rep     movsb
        pop     ecx
        jnz     .repmovsbr
.repmovsb1:
        pop     esi
        mov     al, [edi-1]
        mov     [ebp+.previousByte], al
        jmp     .main_loop
.main_loop_done:
	mov	[ebp+.state], ebx
        mov     [ebp+.continue], .main_loop
        mov     [ebp+.inPtr], esi
        popad
	ret
.restart_repmovsb:
        mov     ecx, [ebp+.ecx]
        jmp     .repmovsbz

.RangeDecoderBitDecode:
; in: eax->prob
; out: CF=bit; destroys eax
	push	edx
	mov	edx, [ebp+.range]
	shr	edx, .kNumBitModelTotalBits
	imul	edx, [eax]
	cmp	[ebp+.code], edx
	jae	.ae
	mov	[ebp+.range], edx
	mov	edx, .kBitModelTotal
	sub	edx, [eax]
	shr	edx, .kNumMoveBits
	add	[eax], edx
	clc
.n:
	lahf
	cmp	[ebp+.range], .kTopValue
	jae	@f
	shl	[ebp+.range], 8
	shl	[ebp+.code], 8
        sub     [ebp+.inLen], 1
        js      .refill1
.refilled1:
	lodsb
	mov	byte [ebp+.code], al
@@:
	sahf
	pop	edx
	ret
.ae:
	sub	[ebp+.range], edx
	sub	[ebp+.code], edx
	mov	edx, [eax]
	shr	edx, .kNumMoveBits
	sub	[eax], edx
	stc
	jmp	.n

.refill1:
        push    eax
        call    .refill
        pop     eax
        jmp     .refilled1

.refill:
        mov     eax, [ebp+.inStream]
        cmp     dword [eax+streamInfo.fullSize+4], 0
        jnz     @f
        cmp     dword [eax+streamInfo.fullSize], 0
        jz      return.err
@@:
        call    fillBuf
        mov     esi, [eax+streamInfo.bufPtr]
        mov     eax, [eax+streamInfo.bufDataLen]
        dec     eax
        js      return.err
        mov     [ebp+.inLen], eax
        ret

.refill2:
        call    .refill
        jmp     .refilled2

.RangeDecoderDecodeDirectBits:
; in: ecx=numTotalBits
; out: eax=result; destroys edx
	xor	eax, eax
.l:
	shr	[ebp+.range], 1
	shl	eax, 1
	mov	edx, [ebp+.code]
	sub	edx, [ebp+.range]
	jb	@f
	mov	[ebp+.code], edx
	or	eax, 1
@@:
	cmp	[ebp+.range], .kTopValue
	jae	@f
	shl	[ebp+.range], 8
	shl	[ebp+.code], 8
	push	eax
        dec     [ebp+.inLen]
        js      .refill2
.refilled2:
	lodsb
	mov	byte [ebp+.code], al
	pop	eax
@@:
	loop	.l
	ret

.LzmaLiteralDecode:
; in: eax->probs
; out: al=byte; destroys edx
	push	ecx
	mov	ecx, 1
@@:
	push	eax
	lea	eax, [eax+ecx*4]
	call	.RangeDecoderBitDecode
	pop	eax
	adc	cl, cl
	jnc	@b
.LzmaLiteralDecode.ret:
	mov	al, cl
	pop	ecx
	ret
.LzmaLiteralDecodeMatch:
; in: eax->probs, dl=matchByte
; out: al=byte; destroys edx
	push	ecx
	mov	ecx, 1
.LzmaLiteralDecodeMatch.1:
	add	dl, dl
	setc	ch
	push	eax
	lea	eax, [eax+ecx*4+0x100*4]
	call	.RangeDecoderBitDecode
	pop	eax
	adc	cl, cl
	jc	.LzmaLiteralDecode.ret
	xor	ch, cl
	test	ch, 1
	mov	ch, 0
	jnz	@b
	jmp	.LzmaLiteralDecodeMatch.1

.LzmaLenDecode:
; in: eax->prob, edx=posState
; out: ecx=len
	push	eax
	add	eax, .LenChoice*4
	call	.RangeDecoderBitDecode
	pop	eax
	jnc	.0
	push	eax
	add	eax, .LenChoice2*4
	call	.RangeDecoderBitDecode
	pop	eax
	jc	@f
	mov	ecx, .kLenNumMidBits
	shl	edx, cl
	lea	eax, [eax + .LenMid*4 + edx*4]
	call	.RangeDecoderBitTreeDecode
	add	ecx, .kLenNumLowSymbols
	ret
@@:
	add	eax, .LenHigh*4
	mov	ecx, .kLenNumHighBits
	call	.RangeDecoderBitTreeDecode
	add	ecx, .kLenNumLowSymbols + .kLenNumMidSymbols
	ret
.0:
	mov	ecx, .kLenNumLowBits
	shl	edx, cl
	lea	eax, [eax + .LenLow*4 + edx*4]
.RangeDecoderBitTreeDecode:
; in: eax->probs,ecx=numLevels
; out: ecx=length; destroys edx
	push	ebx
	mov	edx, 1
	mov	ebx, edx
@@:
	push	eax
	lea	eax, [eax+edx*4]
	call	.RangeDecoderBitDecode
	pop	eax
	adc	dl, dl
	add	bl, bl
	loop	@b
	sub	dl, bl
	pop	ebx
	mov	ecx, edx
	ret
.RangeDecoderReverseBitTreeDecode:
; in: eax->probs,ecx=numLevels
; out: ecx=length; destroys edx
	push	ebx ecx
	mov	edx, 1
	xor	ebx, ebx
@@:
	push	eax
	lea	eax, [eax+edx*4]
	call	.RangeDecoderBitDecode
	lahf
	adc	edx, edx
	sahf
	rcr	ebx, 1
	pop	eax
	loop	@b
	pop	ecx
	rol	ebx, cl
	mov	ecx, ebx
	pop	ebx
	ret

; LZMA parameters:
;       db      lc + 9 * (lp + 5 * pb)
;       dd      dictionarySize

lzma_get_buf_size:
        cmp     dword [esi-4], 5
        jb      return.err
        push    ecx
        lodsb
        aam     9
        mov     cl, al
        mov     al, ah
        aam     5
        add     cl, al
        mov     eax, LZMA_LIT_SIZE
        shl     eax, cl
        lea     eax, [lzma_decoder.basesize+eax*4]
        pop     ecx
        mov     edx, [esi]
        ret

lzma_init_decoder:
        lodsb
        aam     9
        mov     [ebp+lzma_decoder.lc], al
        mov     al, ah
        aam     5
        mov     [ebp+lzma_decoder.lp], al
        mov     [ebp+lzma_decoder.pb], ah
        cmp     ah, lzma_decoder.kNumPosBitsMax
        ja      return.err
        mov     cl, ah
        lodsd
        mov     [ebp+lzma_decoder.dictSize], eax
        push    1
        pop     eax
        shl     eax, cl
        dec     eax
        mov     [ebp+lzma_decoder.posStateMask], eax
        mov     cl, [ebp+lzma_decoder.lp]
        push    1
        pop     eax
        shl     eax, cl
        dec     eax
        mov     [ebp+lzma_decoder.literalPosMask], eax
        mov     [ebp+streamInfo.fillBuf], lzma_decoder.fillBuf
        mov     [ebp+lzma_decoder.continue], lzma_decoder.start
        xor     eax, eax
        mov     [ebp+lzma_decoder.previousByte], al
        mov     [ebp+lzma_decoder.state], eax
        inc     eax
        lea     edi, [ebp+lzma_decoder.rep0]
        stosd
        stosd
        stosd
        mov     eax, LZMA_LIT_SIZE
        mov     cl, [ebp+lzma_decoder.lc]
        add     cl, [ebp+lzma_decoder.lp]
        shl     eax, cl
        lea     ecx, [eax+lzma_decoder.Literal]
	mov	eax, lzma_decoder.kBitModelTotal/2
	lea	edi, [ebp+lzma_decoder.p]
	rep	stosd
        ret
