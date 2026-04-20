; Branch filters for 7-Zip archives: BCJ and BCJ2.
; Ported from C++ sources of 7-Zip (c) Igor Pavlov.
virtual at 0
bcj_decoder:
.outStream      rb      streamInfo.size
.inStream       dd      ?
.inPtr          dd      ?
.inSize         dd      ?
.nowPos         dd      ?               ; offset in stream
.prevPos        dd      ?               ; pointer in buffer
.prevMask       db      ?
.numRest        db      ?
                rw      1
.dwordRest      dd      ?
.tempSize       dd      ?
.tempDword      dd      ?
.size = $
end virtual

bcj_get_buf_size:
        mov     eax, bcj_decoder.size
        mov     edx, 0x4000
        ret

bcj_init_decoder:
        mov     [ebp+streamInfo.fillBuf], bcj_fillBuf
        xor     edx, edx
        mov     [ebp+bcj_decoder.inPtr], edx
        mov     [ebp+bcj_decoder.inSize], edx
        mov     [ebp+bcj_decoder.prevPos], edx
        mov     [ebp+bcj_decoder.nowPos], edx
        mov     [ebp+bcj_decoder.numRest], dl
        ret

bcj_fillBuf:
        add     [eax+bcj_decoder.nowPos], ecx
        mov     ebp, ecx        ; save output size
        mov     esi, [eax+bcj_decoder.inPtr]
        mov     ebx, [eax+bcj_decoder.inStream]
        mov     ecx, [eax+bcj_decoder.inSize]
        add     esi, [ebx+streamInfo.bufPtr]
        mov     ebx, eax
        cmp     [eax+bcj_decoder.prevPos], 0
        jz      @f
        add     [eax+bcj_decoder.prevPos], edi
@@:
        cmp     [ebx+bcj_decoder.numRest], 0
        jz      .mainloop
        sub     ebp, 1
        js      .mainloopdone
        dec     [ebx+bcj_decoder.numRest]
        mov     eax, [ebx+bcj_decoder.dwordRest]
        stosb
        shr     eax, 8
        mov     [ebx+bcj_decoder.dwordRest], eax
        jmp     @b
.mainloop:
        sub     ebp, 1
        js      .mainloopdone
        sub     ecx, 1
        js      .refill1
.filled1:
        lodsb
.filled2:
        stosb
        cmp     al, 0xE8
        jz      .filter
        cmp     al, 0xE9
        jnz     .mainloop
.filter:
        and     [ebx+bcj_decoder.tempSize], 0
        sub     ecx, 4
        jb      .nopos
        js      .nopos2
        lodsd
        push    eax
.posok:
        xor     edx, edx
        mov     eax, edi
        sub     eax, [ebx+bcj_decoder.prevPos]
        cmp     eax, 5
        ja      .maskok
        movzx   edx, [ebx+bcj_decoder.prevMask]
@@:
        and     edx, 0x77
        add     edx, edx
        sub     eax, 1
        jnz     @b
.maskok:
        mov     [ebx+bcj_decoder.prevMask], dl
        mov     [ebx+bcj_decoder.prevPos], edi
        mov     al, [esp+3]
        add     al, 1
        cmp     al, 2
        jae     .miss
        cmp     dl, 0x20
        jae     .miss2
        lea     eax, [edx-1]
        test    eax, edx
        jnz     .miss2
        pop     eax
        shr     edx, 1
        push    ecx
        mov     cl, [bcj_kMaskToBitNumber+edx]
iglobal
bcj_kMaskToBitNumber db 24,16,8,8,0,0,0,0
endg
@@:
        sub     eax, [ebx+bcj_decoder.nowPos]
        add     eax, [ebx+streamInfo.bufDataLen]
        sub     eax, edi
        sub     eax, 4
        add     eax, [ebx+streamInfo.bufPtr]
        cmp     cl, 24
        jz      @f
        push    eax
        shr     eax, cl
        add     al, 1
        cmp     al, 2
        pop     eax
        jae     @f
        mov     edx, 0x100
        shl     edx, cl
        sub     edx, 1
        xor     eax, edx
        jmp     @b
@@:
        pop     ecx
        shl     eax, 7
        sar     eax, 7
        sub     ebp, 4
        jb      .finalize_dword
        stosd
        jmp     .mainloop
.miss2:
        or      [ebx+bcj_decoder.prevMask], 10h
.miss:
        or      [ebx+bcj_decoder.prevMask], 1
        cmp     [ebx+bcj_decoder.tempSize], 0
        jz      @f
        lea     esi, [ebx+bcj_decoder.tempDword]
        pop     dword [esi]
        mov     ecx, [ebx+bcj_decoder.tempSize]
        jmp     .mainloop
@@:
        pop     eax
        sub     esi, 4
        add     ecx, 4
        jmp     .mainloop
.finalize_dword:
        add     ebp, 4
        mov     [ebx+bcj_decoder.numRest], 4
@@:
        dec     ebp
        js      .save_dword
        stosb
        dec     [ebx+bcj_decoder.numRest]
        shr     eax, 8
        jmp     @b
.save_dword:
        mov     [ebx+bcj_decoder.dwordRest], eax
.mainloopdone:
        mov     eax, [ebx+bcj_decoder.prevPos]
        test    eax, eax
        jz      .noprev
        sub     eax, edi
        mov     [ebx+bcj_decoder.prevPos], eax
.noprev:
        mov     eax, [ebx+bcj_decoder.inStream]
        sub     esi, [eax+streamInfo.bufPtr]
        mov     [ebx+bcj_decoder.inPtr], esi
        mov     [ebx+bcj_decoder.inSize], ecx
        popad
        ret

.refill1:
        cmp     ecx, -1
        jz      .refill0
        lodsb
        cmp     ecx, -4
        jnz     @f
        mov     ecx, [ebx+bcj_decoder.inStream]
        mov     esi, [ecx+streamInfo.bufPtr]
        mov     ecx, [ecx+streamInfo.bufDataLen]
@@:
        jmp     .filled2
.refill0:
        mov     eax, [ebx+bcj_decoder.inStream]
        call    fillBuf
        mov     esi, [eax+streamInfo.bufPtr]
        mov     ecx, [eax+streamInfo.bufDataLen]
        sub     ecx, 1
        js      return.err
        jmp     .filled1

.nopos:
        mov     eax, [ebx+bcj_decoder.inStream]
        cmp     dword [eax+streamInfo.fullSize+4], 0
        jnz     .hasdata
        push    ecx
        add     ecx, dword [eax+streamInfo.fullSize]
        pop     ecx
        jc      .hasdata
        add     ecx, 4
        jmp     .mainloop
.hasdata:
        mov     [ebx+bcj_decoder.tempSize], ecx
        push    0
        push    edi
        lea     edi, [esp+4]
        add     ecx, 4
        rep     movsb
        sub     esi, ebx
        sub     esi, 1
        cmp     esi, bcj_decoder.tempDword+4
        jbe     @f
        call    fillBuf
@@:
        mov     esi, [eax+streamInfo.bufPtr]
        mov     ecx, [ebx+bcj_decoder.tempSize]
        neg     ecx
        rep     movsb
        pop     edi
        mov     ecx, [eax+streamInfo.bufDataLen]
        add     ecx, [ebx+bcj_decoder.tempSize]
        cmp     [ebx+bcj_decoder.tempSize], -4
        jnz     .posok
        and     [ebx+bcj_decoder.tempSize], 0
        jmp     .posok
.nopos2:
        mov     eax, [ebx+bcj_decoder.inStream]
        add     ecx, 4
        jmp     .hasdata

virtual at 0
bcj2_decoder:
.outStream      rb      streamInfo.size
.mainInStream   dd      ?
.callStream     dd      ?
.jumpStream     dd      ?
.rangeDecoder   dd      ?
.dwordRest      dd      ?
.prevByte       db      ?
.numRest        db      ?
.bInited        db      ?
                rb      1
.inPtr          dd      ?
.inSize         dd      ?
.callPtr        dd      ?
.jumpPtr        dd      ?
.callSize       dd      ?
.jumpSize       dd      ?
.rangeDecPtr    dd      ?
.rangeDecSize   dd      ?
.nowPos         dd      ?
.range          dd      ?
.code           dd      ?
.statusE9Decoder dd     ?
.statusJccDecoder dd    ?
.statusE8Decoder rd     256
.size = $
end virtual

bcj2_get_buf_size:
        mov     eax, bcj2_decoder.size
        mov     edx, 0x4000
        ret

bcj2_init_decoder:
        mov     [ebp+streamInfo.fillBuf], bcj2_fillBuf
	mov	eax, lzma_decoder.kBitModelTotal/2
        mov     ecx, 256+1+1
	lea	edi, [ebp+bcj2_decoder.statusE9Decoder]
	rep	stosd
        mov     dword [ebp+bcj2_decoder.prevByte], ecx
        mov     [ebp+bcj2_decoder.inSize], ecx
        mov     [ebp+bcj2_decoder.callSize], ecx
        mov     [ebp+bcj2_decoder.jumpSize], ecx
        mov     [ebp+bcj2_decoder.rangeDecSize], ecx
        mov     [ebp+bcj2_decoder.nowPos], ecx
        ret

bcj2_fillBuf.init:
        mov     eax, [eax+bcj2_decoder.rangeDecoder]
        call    fillBuf
        mov     edx, [eax+streamInfo.bufDataLen]
        sub     edx, 5
        jb      return.err
        mov     [ebp+bcj2_decoder.rangeDecSize], edx
        mov     edx, [eax+streamInfo.bufPtr]
        add     edx, 5
        mov     [ebp+bcj2_decoder.rangeDecPtr], edx
        mov     edx, [edx-4]
        bswap   edx
        mov     [ebp+bcj2_decoder.code], edx
        or      [ebp+bcj2_decoder.range], -1
        mov     [ebp+bcj2_decoder.bInited], 1
        mov     eax, ebp
        jmp     bcj2_fillBuf.inited

bcj2_fillBuf:
        mov     ebp, eax
        cmp     [eax+bcj2_decoder.bInited], 0
        jz      .init
.inited:
        add     [eax+bcj2_decoder.nowPos], ecx
        mov     esi, [eax+bcj2_decoder.inPtr]
@@:
        cmp     [ebp+bcj2_decoder.numRest], 0
        jz      .mainloop
        sub     ecx, 1
        js      .mainloopdone
        dec     [ebp+bcj2_decoder.numRest]
        mov     eax, [ebp+bcj2_decoder.dwordRest]
        stosb
        mov     [ebp+bcj2_decoder.prevByte], al
        shr     eax, 8
        mov     [ebp+bcj2_decoder.dwordRest], eax
        jmp     @b
.mainloop:
        sub     ecx, 1
        js      .mainloopdone
        sub     [ebp+bcj2_decoder.inSize], 1
        js      .refill1
.filled1:
        lodsb
        stosb
        cmp     al, 0xE8
        jz      .e8
        cmp     al, 0xE9
        jz      .e9
        cmp     [ebp+bcj2_decoder.prevByte], 0xF
        mov     [ebp+bcj2_decoder.prevByte], al
        jnz     .mainloop
        and     al, 0xF0
        cmp     al, 0x80
        jnz     .mainloop
.jcc:
        lea     eax, [ebp+bcj2_decoder.statusJccDecoder]
        call    .RangeDecoderBitDecode
        jnc     .mainloop
        jmp     .getptrj
.e8:
        movzx   eax, al
        xchg    al, [ebp+bcj2_decoder.prevByte]
        lea     eax, [ebp+bcj2_decoder.statusE8Decoder+eax*4]
        call    .RangeDecoderBitDecode
        jnc     .mainloop
        lea     eax, [ebp+bcj2_decoder.callPtr]
        jmp     .getptr
.e9:
        mov     [ebp+bcj2_decoder.prevByte], al
        lea     eax, [ebp+bcj2_decoder.statusE9Decoder]
        call    .RangeDecoderBitDecode
        jnc     .mainloop
.getptrj:
        lea     eax, [ebp+bcj2_decoder.jumpPtr]
.getptr:
        sub     dword [eax+8], 4
        js      .refill2
.filled2:
        add     dword [eax], 4
        mov     eax, [eax]
        mov     eax, [eax-4]
        bswap   eax
        sub     eax, [ebp+bcj2_decoder.nowPos]
        add     eax, [ebp+streamInfo.bufDataLen]
        sub     eax, edi
        sub     eax, 4
        add     eax, [ebp+streamInfo.bufPtr]
        sub     ecx, 4
        jb      .finalize_dword
        stosd
        shr     eax, 24
        mov     [ebp+bcj2_decoder.prevByte], al
        jmp     .mainloop
.finalize_dword:
        add     ecx, 4
        mov     [ebp+bcj2_decoder.numRest], 4
@@:
        dec     ecx
        js      .save_dword
        stosb
        dec     [ebp+bcj2_decoder.numRest]
        shr     eax, 8
        jmp     @b
.save_dword:
        mov     [ebp+bcj2_decoder.dwordRest], eax
.mainloopdone:
        mov     [ebp+bcj2_decoder.inPtr], esi
        popad
        ret

.refill1:
        mov     eax, [ebp+bcj2_decoder.mainInStream]
        call    fillBuf
        mov     edx, [eax+streamInfo.bufDataLen]
        dec     edx
        js      return.err
        mov     [ebp+bcj2_decoder.inSize], edx
        mov     esi, [eax+streamInfo.bufPtr]
        jmp     .filled1

.refill2:
        push    eax
        mov     eax, [eax-bcj2_decoder.callPtr+bcj2_decoder.callStream]
        call    fillBuf
        mov     edx, [eax+streamInfo.bufDataLen]
        sub     edx, 4
        js      return.err
        push    [eax+streamInfo.bufPtr]
        mov     eax, [esp+4]
        pop     dword [eax]
        pop     eax
        mov     [eax+8], edx
        jmp     .filled2

.refill3:
        push    eax
        mov     eax, [ebp+bcj2_decoder.rangeDecoder]
        call    fillBuf
        mov     edx, [eax+streamInfo.bufDataLen]
        dec     edx
        js      return.err
        mov     [ebp+bcj2_decoder.rangeDecSize], edx
        mov     edx, [eax+streamInfo.bufPtr]
        mov     [ebp+bcj2_decoder.rangeDecPtr], edx
        pop     eax
        jmp     .filled3

.RangeDecoderBitDecode:
; in: eax->prob
; out: CF=bit; destroys eax,edx
	mov	edx, [ebp+bcj2_decoder.range]
	shr	edx, lzma_decoder.kNumBitModelTotalBits
	imul	edx, [eax]
	cmp	[ebp+bcj2_decoder.code], edx
	jae	.ae
	mov	[ebp+bcj2_decoder.range], edx
	mov	edx, lzma_decoder.kBitModelTotal
	sub	edx, [eax]
	shr	edx, lzma_decoder.kNumMoveBits
	add	[eax], edx
	clc
.n:
	lahf
	cmp	[ebp+bcj2_decoder.range], lzma_decoder.kTopValue
	jae	@f
	shl	[ebp+bcj2_decoder.range], 8
	shl	[ebp+bcj2_decoder.code], 8
        dec     [ebp+bcj2_decoder.rangeDecSize]
        js      .refill3
.filled3:
        mov     edx, [ebp+bcj2_decoder.rangeDecPtr]
        mov     al, [edx]
        add     edx, 1
        mov     [ebp+bcj2_decoder.rangeDecPtr], edx
	mov	byte [ebp+bcj2_decoder.code], al
@@:
	sahf
	ret
.ae:
	sub	[ebp+bcj2_decoder.range], edx
	sub	[ebp+bcj2_decoder.code], edx
	mov	edx, [eax]
	shr	edx, lzma_decoder.kNumMoveBits
	sub	[eax], edx
	stc
	jmp	.n
