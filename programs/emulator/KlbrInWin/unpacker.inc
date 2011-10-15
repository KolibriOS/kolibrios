; void __stdcall unpack(void* packed_data, void* unpacked_data);
unpack:
        pushad
        mov     esi, [esp+32+4]
        mov     edi, [esp+32+8]
        mov     eax, [esi+8]
        and     al, 0xC0
        cmp     al, 0xC0
        jz      .failed
        mov     eax, [esi+8]
        push    eax
        add     esi, 12
        and     al, not 0xC0
        dec     eax
        jz      .lzma
.failed:
        pop     eax
        popad
        ret     8
.lzma:
        call    .lzma_unpack
.common:
        pop     eax
        test    al, 0x80
        jnz     .ctr1
        test    al, 0x40
        jz      .ok
        lodsd
        mov     ecx, eax
        jecxz   .ok
        mov     dl, [esi]
        mov     esi, [esp+32+8]
.c1:
        lodsb
        sub     al, 0E8h
        cmp     al, 1
        ja      .c1
        cmp     byte [esi], dl
        jnz     .c1
        lodsd
; "bswap eax" is not supported on i386
        shr     ax, 8
        ror     eax, 16
        xchg    al, ah
        sub     eax, esi
        add     eax, [esp+32+8]
        mov     [esi-4], eax
        loop    .c1
.ok:
        popad
        ret     8
.ctr1:
        lodsd
        mov     ecx, eax
        jecxz   .ok
        mov     dl, [esi]
        mov     esi, [esp+32+8]
.c2:
        lodsb
@@:
        cmp     al, 0xF
        jnz     .f
        lodsb
        cmp     al, 80h
        jb      @b
        cmp     al, 90h
        jb      @f
.f:
        sub     al, 0E8h
        cmp     al, 1
        ja      .c2
@@:
        cmp     byte [esi], dl
        jnz     .c2
        lodsd
        shr     ax, 8
        ror     eax, 16
        xchg    al, ah
        sub     eax, esi
        add     eax, [esp+32+8]
        mov     [esi-4], eax
        loop    .c2
        jmp     .ok

.lzma_unpack:

.pb	=	2	; pos state bits
.lp	=	0	; literal pos state bits
.lc	=	3	; literal context bits
.posStateMask	=	((1 shl .pb)-1)
.literalPosMask	=	((1 shl .lp)-1)

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

.LZMA_BASE_SIZE	=	1846	; must be ==Literal
.LZMA_LIT_SIZE	=	768

.kNumTopBits	=	24
.kTopValue	=	(1 shl .kNumTopBits)

.kNumBitModelTotalBits =	11
.kBitModelTotal	=	(1 shl .kNumBitModelTotalBits)
.kNumMoveBits	=	5

        push    edi
; int state=0;
	xor	ebx, ebx
	mov	[.previousByte], bl
; unsigned rep0=1,rep1=1,rep2=1,rep3=1;
	mov	eax, 1
	mov	edi, .rep0
	stosd
	stosd
	stosd
	stosd
; int len=0;
; result=0;
	mov	ecx, .Literal + (.LZMA_LIT_SIZE shl (.lc+.lp))
	mov	eax, .kBitModelTotal/2
	mov	edi, .p
	rep	stosd
; RangeDecoderInit
; rd->ExtraBytes = 0
; rd->Buffer = stream
; rd->BufferLim = stream+bufferSize
; rd->Range = 0xFFFFFFFF
        pop     edi
	mov	ebp, [esi-8]	; dest_length
	add	ebp, edi	; ebp = destination limit
	lodsd
; rd->code_ = eax
	mov	[.code_], eax
	or	[.range], -1
.main_loop:
	cmp	edi, ebp
	jae	.main_loop_done
	mov	edx, edi
	and	edx, .posStateMask
	mov	eax, ebx
	shl	eax, .kNumPosBitsMax+2
	lea	eax, [.p + .IsMatch*4 + eax + edx*4]
	call	.RangeDecoderBitDecode
	jc	.1
	movzx	eax, [.previousByte]
if .literalPosMask
	mov	ah, dl
	and	ah, .literalPosMask
end if
	shr	eax, 8-.lc
	imul	eax, .LZMA_LIT_SIZE*4
	add	eax, .p+.Literal*4
	cmp	ebx, .kNumLitStates
	jb	.literal
	xor	edx, edx
	sub	edx, [.rep0]
	mov	dl, [edi + edx]
	call	.LzmaLiteralDecodeMatch
	jmp	@f
.literal:
	call	.LzmaLiteralDecode
@@:
	mov	[.previousByte], al
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
	lea	eax, [.p + .IsRep*4 + ebx*4]
	call	.RangeDecoderBitDecode
	jnc	.10
	lea	eax, [.p + .IsRepG0*4 + ebx*4]
	call	.RangeDecoderBitDecode
	jc	.111
	mov	eax, ebx
	shl	eax, .kNumPosBitsMax+2
	lea	eax, [.p + .IsRep0Long*4 + eax + edx*4]
	call	.RangeDecoderBitDecode
	jc	.1101
	cmp	bl, 7
	setae	bl
	lea	ebx, [9 + ebx + ebx]
	xor	edx, edx
	sub	edx, [.rep0]
	mov	al, [edi + edx]
	stosb
	mov	[.previousByte], al
	jmp	.main_loop
.111:
	lea	eax, [.p + .IsRepG1*4 + ebx*4]
	call	.RangeDecoderBitDecode
	mov	eax, [.rep1]
	jnc	.l3
.l1:
	lea	eax, [.p + .IsRepG2*4 + ebx*4]
	call	.RangeDecoderBitDecode
	mov	eax, [.rep2]
	jnc	.l2
	xchg	[.rep3], eax
.l2:
	push	[.rep1]
	pop	[.rep2]
.l3:
	xchg	eax, [.rep0]
	mov	[.rep1], eax
.1101:
	mov	eax, .p + .RepLencoder*4
	call	.LzmaLenDecode
	cmp	bl, 7
	setc	bl
	adc	bl, bl
	xor	bl, 3
	add	bl, 8
	jmp	.repmovsb
.10:
	mov	eax, [.rep0]
	xchg	eax, [.rep1]
	xchg	eax, [.rep2]
	xchg	eax, [.rep3]
	cmp	bl, 7
	setc	bl
	adc	bl, bl
	xor	bl, 3
	add	bl, 7
	mov	eax, .p + .Lencoder*4
	call	.LzmaLenDecode
	mov	eax, .kNumLenToPosStates-1
	cmp	eax, ecx
	jb	@f
	mov	eax, ecx
@@:
	push	ecx
	mov	ecx, .kNumPosSlotBits
	shl	eax, cl
	shl	eax, 2
	add	eax, .p+.PosSlot*4
	call	.RangeDecoderBitTreeDecode
	mov	[.rep0], ecx
	cmp	ecx, .kStartPosModelIndex
	jb	.l6
	push	ecx
	mov	eax, ecx
	and	eax, 1
	shr	ecx, 1
	or	eax, 2
	dec	ecx
	shl	eax, cl
	mov	[.rep0], eax
	pop	edx
	cmp	edx, .kEndPosModelIndex
	jae	.l5
	sub	eax, edx
	shl	eax, 2
	add	eax, .p + (.SpecPos - 1)*4
	call	.RangeDecoderReverseBitTreeDecode
	add	[.rep0], ecx
	jmp	.l6
.l5:
	sub	ecx, .kNumAlignBits
	call	.RangeDecoderDecodeDirectBits
	mov	ecx, .kNumAlignBits
	shl	eax, cl
	add	[.rep0], eax
	mov	eax, .p+.Align_*4
	call	.RangeDecoderReverseBitTreeDecode
	add	[.rep0], ecx
.l6:
	pop	ecx
	inc	[.rep0]
	jz	.main_loop_done
.repmovsb:
	add	ecx, .kMatchMinLen
	push	esi
	mov	esi, edi
	sub	esi, [.rep0]
	rep	movsb
	pop	esi
	mov	al, [edi-1]
	mov	[.previousByte], al
	jmp	.main_loop
.main_loop_done:
	ret

.RangeDecoderBitDecode:
; in: eax->prob
; out: CF=bit; destroys eax
	push	edx
	mov	edx, [.range]
	shr	edx, .kNumBitModelTotalBits
	imul	edx, [eax]
	cmp	[.code_], edx
	jae	.ae
	mov	[.range], edx
	mov	edx, .kBitModelTotal
	sub	edx, [eax]
	shr	edx, .kNumMoveBits
	add	[eax], edx
	clc
.n:
	lahf
	cmp	[.range], .kTopValue
	jae	@f
	shl	[.range], 8
	shl	[.code_], 8
	lodsb
	mov	byte [.code_], al
@@:
	sahf
	pop	edx
	ret
.ae:
	sub	[.range], edx
	sub	[.code_], edx
	mov	edx, [eax]
	shr	edx, .kNumMoveBits
	sub	[eax], edx
	stc
	jmp	.n

.RangeDecoderDecodeDirectBits:
; in: ecx=numTotalBits
; out: eax=result; destroys edx
	xor	eax, eax
.l:
	shr	[.range], 1
	shl	eax, 1
	mov	edx, [.code_]
	sub	edx, [.range]
	jb	@f
	mov	[.code_], edx
	or	eax, 1
@@:
	cmp	[.range], .kTopValue
	jae	@f
	shl	[.range], 8
	shl	[.code_], 8
	push	eax
	lodsb
	mov	byte [.code_], al
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
