; Deflate and Deflate64 decoders for *.zip and *.7z archives.
; Written by diamond in 2007.

deflate_decoder:
virtual at 0
.outStream      rb      streamInfo.size
.inStream       dd      ?
.bDeflate64     db      ?
.dl             db      ?
.bLast          db      ?
                rb      1
.outEnd         dd      ?
.inLen          dd      ?
.inPtr          dd      ?
.continue       dd      ?
.blockLen       dd      ?
.lit_len        rb      19
                rb      1
.lengths        rb      288+32
.huff_bl        rd      18*2
.huff_dist      rd      31*2
.huff_lit       rd      287*2
.size = $
end virtual

.fillBuf:
        mov     ebp, eax
        jecxz   .nodata
        add     ecx, edi
        mov     [ebp+.outEnd], ecx
        mov     esi, [ebp+.inPtr]
        mov     dl, [ebp+.dl]
        jmp     [ebp+.continue]
.nodata:
        popad
        ret
.block_loop_done:
        mov     [ebp+.inPtr], esi
        mov     [ebp+.dl], dl
        mov     [ebp+.continue], .block_loop
        popad
        ret
.start:
        xor     edx, edx
        mov     [ebp+.inLen], edx
        mov     [ebp+.bLast], dl
.block_loop:
        cmp     edi, [ebp+.outEnd]
        jae     .block_loop_done
        cmp     [ebp+.bLast], 0
        jnz     return.err
        call    .get_bit
        setc    [ebp+.bLast]
        call    .get_bit
        jc      .test_block_fh
        call    .get_bit
        jc      .block_dh
.block_stored:  ; Stored
        xor     edx, edx
        xor     eax, eax
        call    .get_word
        mov     ecx, eax
        call    .get_word
        not     ax
        cmp     eax, ecx
        jnz     return.err
        mov     [ebp+.blockLen], ecx
.continue_stored:
        mov     ecx, [ebp+.blockLen]
        mov     eax, [ebp+.outEnd]
        sub     eax, edi
        cmp     eax, ecx
        jae     @f
        mov     ecx, eax
@@:
        sub     [ebp+.blockLen], ecx
.bs_loop:
        mov     eax, [ebp+.inLen]
        test    eax, eax
        jnz     @f
        call    .refill
        mov     eax, [ebp+.inLen]
        inc     eax
        mov     [ebp+.inLen], eax
@@:
        push    ecx
        sub     ecx, eax
        sbb     eax, eax
        and     ecx, eax
        add     ecx, [ebp+.inLen]
        sub     [ebp+.inLen], ecx
        sub     [esp], ecx
        rep     movsb
        pop     ecx
        jnz     .bs_loop
        cmp     [ebp+.blockLen], ecx
        jz      .block_loop
        mov     [ebp+.inPtr], esi
        mov     [ebp+.dl], dl
        mov     [ebp+.continue], .continue_stored
        popad
        ret
.test_block_fh:
        call    .get_bit
        jc      return.err
.block_fh:      ; Fixed Huffman
        push    edi
        lea     edi, [ebp+.huff_dist]
        lea     eax, [edi+8]
        xor     ecx, ecx
        mov     cl, 30
@@:
        stosd
        add     eax, 8
        loop    @b
        xor     eax, eax
        mov     cl, 32
@@:
        stosd
        inc     eax
        loop    @b
        lea     eax, [edi+8]
        mov     cl, 126
@@:
        stosd
        add     eax, 8
        loop    @b
        push    eax
        mov     eax, 256
        mov     cl, 24
@@:
        stosd
        inc     eax
        loop    @b
        pop     eax
        mov     cl, 104
@@:
        stosd
        add     eax, 8
        loop    @b
        push    eax
        xor     eax, eax
        mov     cl, 144
@@:
        stosd
        inc     eax
        loop    @b
        mov     eax, 280
        mov     cl, 8
@@:
        stosd
        inc     eax
        loop    @b
        pop     eax
        mov     cl, 56
@@:
        stosd
        add     eax, 8
        loop    @b
        mov     eax, 144
        mov     cl, 112
@@:
        stosd
        inc     eax
        loop    @b
        jmp     .block_h_start
.block_dh:      ; Dynamic Huffman
        xor     ecx, ecx
        mov     cl, 5
        push    edi
        lea     edi, [ebp+.lit_len]
        push    edi
        xor     eax, eax
        rep     stosd
        pop     edi
        mov     cl, 5
        call    .get_bits
        push    eax
        mov     cl, 5
        call    .get_bits
        push    eax
        mov     cl, 4
        call    .get_bits
        lea     ecx, [eax+4]
        mov     ebx, deflate.CodeLengthOrder
iglobal
deflate.CodeLengthOrder:
        db      16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
endg
@@:
        push    ecx
        mov     cl, 3
        call    .get_bits
        mov     cl, [ebx]
        inc     ebx
        mov     [edi+ecx], al
        pop     ecx
        loop    @b
        lea     ebx, [ebp+.huff_bl]
        mov     cl, 19
        push    18*8
        call    .construct_huffman_tree
        mov     ecx, [esp]
        add     ecx, [esp+4]
        add     ecx, 258
        lea     edi, [ebp+.lengths]
.dhl:
        lea     ebx, [ebp+.huff_bl]
        call    .get_huffman_code
        cmp     eax, 16
        jae     .dh_special
        stosb
        loop    .dhl
        jmp     .dhd
.dh_special:
        push    ecx
        sub     eax, 16
        jnz     .dh_norep
        push    2
        pop     ecx
        call    .get_bits
        pop     ecx
        add     eax, 3
        sub     ecx, eax
        jb      return.err
@@:
        mov     bl, [edi-1]
        mov     [edi], bl
        inc     edi
        dec     eax
        jnz     @b
        test    ecx, ecx
        jnz     .dhl
        jmp     .dhd
.dh_norep:
        dec     eax
        jz      .dh_0
        dec     eax
        jnz     return.err
        push    7
        pop     ecx
        call    .get_bits
        add     eax, 11
        jmp     @f
.dh_0:
        push    3
        pop     ecx
        call    .get_bits
        add     eax, 3
@@:
        pop     ecx
        sub     ecx, eax
        jb      return.err
        push    ecx
        mov     ecx, eax
        xor     eax, eax
        rep     stosb
        pop     ecx
        test    ecx, ecx
        jnz     .dhl
.dhd:
        pop     ecx
        inc     ecx
        lea     ebx, [ebp+.huff_dist]
        pop     edi
        push    edi
        lea     edi, [edi+ebp+.lengths+257]
        push    31*8
        call    .construct_huffman_tree
        pop     ecx
        add     ecx, 257
        lea     ebx, [ebp+.huff_lit]
        lea     edi, [ebp+.lengths]
        push    287*8
        call    .construct_huffman_tree
.block_h_start: ; Huffman
        pop     edi
.block_h:
        lea     ebx, [ebp+.huff_lit]
        call    .get_huffman_code
        sub     eax, 256
        jnc     .not_char
        stosb
.bhc:
        cmp     edi, [ebp+.outEnd]
        jb      .block_h
        mov     [ebp+.inPtr], esi
        mov     [ebp+.dl], dl
        mov     [ebp+.continue], .block_h
        popad
        ret
.not_char:
        jz      .block_loop
        cmp     eax, 285-256
        ja      return.err
        jz      .h_max
iglobal
deflate.LengthCodesStart:
        db      3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59
        db      67,83,99,115,131,163,195,227
deflate.LengthCodesExtra:
        db      0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5
endg
        movzx   ebx, byte [deflate.LengthCodesStart+eax-1]
        movzx   ecx, byte [deflate.LengthCodesExtra+eax-1]
        call    .get_bits
        add     ebx, eax
.length_known:
        push    ebx
        lea     ebx, [ebp+.huff_dist]
        call    .get_huffman_code
        cmp     eax, 32
        jae     return.err
iglobal
align 4
deflate.DistCodesStart:
        dd      1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537
        dd      2049,3073,4097,6145,8193,12289,16385,24577,32769,49153
deflate.DistCodesExtra:
        db      0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13,14,14
endg
        mov     ebx, [deflate.DistCodesStart+eax*4]
        movzx   ecx, byte [deflate.DistCodesExtra+eax]
        call    .get_bits
        add     ebx, eax
        pop     ecx
; ecx=length, ebx=distance
.repmovsbz:
        push    esi
.repmovsbr:
        mov     esi, edi
        sub     esi, ebx
        mov     eax, [ebp+streamInfo.bufPtr]
        sub     eax, esi
        ja      .repmovsb0
        mov     eax, [ebp+.outEnd]
        sub     eax, edi
        push    ecx
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
        sub     [esp], ecx
        rep     movsb
        pop     ecx
        jz      .repmovsb1
.repmovsbc:
        pop     [ebp+.inPtr]
        mov     [ebp+.blockLen], ecx
        mov     dword [ebp+.lit_len], ebx
        mov     [ebp+.dl], dl
        mov     [ebp+.continue], .restart_repmovsb
        popad
        ret
.repmovsb0:
        add     esi, 0x10000
        push    ecx
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
        mov     eax, [ebp+.outEnd]
        sub     eax, edi
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
        sub     [esp], ecx
        rep     movsb
        pop     ecx
        jz      .repmovsb1
        cmp     edi, [ebp+.outEnd]
        jb      .repmovsbr
        jmp     .repmovsbc
.repmovsb1:
        pop     esi
        jmp     .bhc
.restart_repmovsb:
        mov     ecx, [ebp+.blockLen]
        mov     ebx, dword [ebp+.lit_len]
        jmp     .repmovsbz
.h_max:
        mov     ebx, 258
        xor     ecx, ecx
        cmp     [ebp+.bDeflate64], cl
        jz      .length_known
        mov     cl, 16
        call    .get_bits
        lea     ebx, [eax+3]
        jmp     .length_known

align 16
.get_bit:
        shr     dl, 1
        jnz     .ret
        sub     [ebp+.inLen], 1
        js      .gb_refill
@@:
        mov     dl, [esi]
        sub     esi, -1
        rcr     dl, 1
.ret:
        ret
.gb_refill:
        call    .refill
        jmp     @b

.refill:
        push    eax
        mov     eax, [ebp+.inStream]
        call    fillBuf
        mov     esi, [eax+streamInfo.bufPtr]
        mov     eax, [eax+streamInfo.bufDataLen]
        dec     eax
        js      return.err
        mov     [ebp+.inLen], eax
        pop     eax
        ret

.get_bits:
        push    ebx
        mov     ebx, ecx
        xor     eax, eax
        jecxz   .gbr
@@:
        call    .get_bit
        rcr     eax, 1
        loop    @b
        mov     cl, 32
        sub     cl, bl
        shr     eax, cl
.gbr:
        pop     ebx
        ret

.get_word:
        sub     [ebp+.inLen], 1
        jns     @f
        call    .refill
@@:
        lodsb
        sub     [ebp+.inLen], 1
        jns     @f
        call    .refill
@@:
        mov     ah, [esi]
        inc     esi
        ret

.construct_huffman_tree:
; edi->bit lengths array, ecx=number of items, ebx->tree root, [esp+4]=size of tree
        add     [esp+4], ebx
        push    edx esi
        xor     eax, eax
        xor     edx, edx
        mov     dword [ebx], eax
        mov     dword [ebx+4], eax
.cht1:
        cmp     al, [edi+edx]
        ja      @f
        mov     al, [edi+edx]
@@:
        inc     edx
        cmp     edx, ecx
        jb      .cht1
        test    eax, eax
        jz      .chtd
        push    ecx     ; remember number of items
        push    eax     ; remember maximum length
        lea     eax, [ebx+8]
        xor     edx, edx
        inc     edx
        push    2
        pop     ecx
.cht2:
        push    eax
        xor     eax, eax
.cht3:
        cmp     dl, [edi+eax]
        jnz     @f
        dec     ecx
        js      return.err
        cmp     ebx, [esp+24]
        jae     return.err
        mov     [ebx], eax
        add     ebx, 4
@@:
        inc     eax
        cmp     eax, [esp+8]
        jnz     .cht3
        pop     eax
        jecxz   .cht4
        push    ecx
.cht5:
        cmp     eax, [esp+24]
        jb      @f
        or      eax, -1
@@:
        cmp     ebx, [esp+24]
        jae     return.err
        mov     [ebx], eax
        add     ebx, 4
        cmp     eax, -1
        jz      @f
        add     eax, 8
@@:
        loop    .cht5
        pop     ecx
        add     ecx, ecx
.cht4:
        inc     edx
        cmp     edx, [esp]
        jbe     .cht2
        pop     eax
        pop     eax
        jecxz   .chtd
        or      eax, -1
@@:
        cmp     ebx, [esp+12]
        jae     .chtd
        mov     [ebx], eax
        add     ebx, 4
        loop    @b
.chtd:
        pop     esi edx
        ret     4

.get_huffman_code:
; ebx->tree root
        xor     eax, eax
        cmp     dword [ebx+4], eax
        jz      .ghcret
@@:
        call    .get_bit
        setc    al
        mov     ebx, [ebx+4*eax]
        cmp     ebx, -1
        jz      @f
        cmp     ebx, 0x1000
        jae     @b
@@:
        mov     eax, ebx
.ghcret:
        ret

deflate_get_buf_size:
        mov     eax, deflate_decoder.size
        mov     edx, 0x10000
        ret

deflate_init_decoder:
        mov     [ebp+deflate_decoder.bDeflate64], 0
        jmp     @f

deflate64_init_decoder:
        mov     [ebp+deflate_decoder.bDeflate64], 1
@@:
        mov     [ebp+streamInfo.fillBuf], deflate_decoder.fillBuf
        mov     [ebp+deflate_decoder.continue], deflate_decoder.start
        ret
