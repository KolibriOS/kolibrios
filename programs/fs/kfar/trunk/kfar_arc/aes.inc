; Implementation of AES crypto algorithm.
; Buffer size is 0x10 bytes (128 bits), key size is not fixed.
; Written by diamond in 2007.
uglobal
aes.pow_table   rb      256     ; pow[a] = 3^a
aes.log_table   rb      256     ; log[3^a] = a
aes.sbox        rb      256     ; ShiftBytes(a)
aes.sbox_rev    rb      256     ; ShiftBytes^{-1}(a)
aes.mctable     rd      256     ; MixColumns(ShiftBytes(a,0,0,0))
aes.mcrtable    rd      256     ; MixColumns^{-1}(a,0,0,0)
endg

init_aes:
; Byte values in SubBytes transform are interpreted as items of
;   GF(2^8) \cong F_2[x]/(x^8+x^4+x^3+x+1)F_2[x].
; x+1 is primitive item in this field.
        xor     ebx, ebx
        push    1
        pop     eax
.1:
        mov     [aes.pow_table+ebx], al
        mov     [aes.log_table+eax], bl
; Multiplication by x+1...
        mov     cl, al  ; save value
; ...multiply by x with mod (x^8+x^4+x^3+x+1) = 0x11B...
        add     al, al
        jnc     @f
        xor     al, 0x1B
@@:
; ...and add operand
        xor     al, cl
        inc     bl
        jnz     .1
; generate table for SubBytes transform
        mov     [aes.sbox+0], 0x63
        mov     [aes.sbox_rev+0x63], bl
        inc     ebx
.2:
; calculate inverse in GF(2^8)
        mov     al, [aes.log_table+ebx]
        xor     al, 0xFF        ; equivalent to "al = 0xFF - al"
        mov     cl, [aes.pow_table+eax]
; linear transform of byte as vector over F_2
        mov     al, cl
        rol     cl, 1
        xor     al, cl
        rol     cl, 1
        xor     al, cl
        rol     cl, 1
        xor     al, cl
        rol     cl, 1
        xor     al, cl
        xor     al, 0x63
        mov     [aes.sbox+ebx], al
        mov     [aes.sbox_rev+eax], bl
        inc     bl
        jnz     .2
; generate table for SubBytes + MixColumn transforms
.3:
        mov     al, [aes.sbox+ebx]      ; SubBytes transform
        mov     cl, al
        add     cl, cl
        jnc     @f
        xor     cl, 0x1B
@@:
        mov     byte [aes.mctable+ebx*4], cl    ; low byte of MixColumn(a,0,0,0)
        mov     byte [aes.mctable+ebx*4+1], al
        mov     byte [aes.mctable+ebx*4+2], al
        xor     cl, al
        mov     byte [aes.mctable+ebx*4+3], cl  ; high byte of MixColumn(a,0,0,0)
        inc     bl
        jnz     .3
; generate table for reverse MixColumn transform
        mov     dword [aes.mcrtable+0], ebx
        inc     ebx
.4:
; log_table[9]=0xC7, log_table[0xB]=0x68, log_table[0xD]=0xEE, log_table[0xE]=0xDF
        mov     cl, [aes.log_table+ebx]
        mov     al, cl
        add     al, 0xDF
        adc     al, 0
        mov     al, [aes.pow_table+eax]
        mov     byte [aes.mcrtable+ebx*4], al
        mov     al, cl
        add     al, 0xC7
        adc     al, 0
        mov     al, [aes.pow_table+eax]
        mov     byte [aes.mcrtable+ebx*4+1], al
        mov     al, cl
        add     al, 0xEE
        adc     al, 0
        mov     al, [aes.pow_table+eax]
        mov     byte [aes.mcrtable+ebx*4+2], al
        mov     al, cl
        add     al, 0x68
        adc     al, 0
        mov     al, [aes.pow_table+eax]
        mov     byte [aes.mcrtable+ebx*4+3], al
        inc     bl
        jnz     .4
        ret

aes_setkey:
; in: esi->key, edx=key size in dwords, edi->AES data struc
        lea     eax, [edx+6]    ; calc number of rounds (buffer size=4)
        stosd
        shl     eax, 4
        lea     ebx, [edi+eax+16]
        mov     ecx, edx
        rep     movsd
        push    ebx
        mov     bl, 1
.0:
        push    4
        pop     ecx
@@:
        movzx   esi, byte [edi-5+ecx]
        mov     al, [aes.sbox+esi]
        rol     eax, 8
        loop    @b
        ror     eax, 16
        mov     esi, edx
        neg     esi
        xor     eax, [edi+esi*4]
        xor     al, bl
        add     bl, bl
        jnc     @f
        xor     bl, 0x1B
@@:
        stosd
        lea     ecx, [edx-1]
.1:
        cmp     edi, [esp]
        jz      .ret
        cmp     edx, 8
        jnz     @f
        cmp     ecx, 4
        jnz     @f
        push    eax
        movzx   eax, al
        mov     al, [aes.sbox+eax]
        mov     [esp], al
        mov     al, byte [esp+1]
        mov     al, [aes.sbox+eax]
        mov     [esp+1], al
        mov     al, byte [esp+2]
        mov     al, [aes.sbox+eax]
        mov     [esp+2], al
        mov     al, byte [esp+3]
        mov     al, [aes.sbox+eax]
        mov     [esp+3], al
        pop     eax
@@:
        xor     eax, [edi+esi*4]
        stosd
        loop    .1
        cmp     edi, [esp]
        jnz     .0
.ret:
        pop     eax
        ret

aes_decode:
; in: esi->in, ebx->out, edi->AES state
        push    ebx ebp
        push    dword [esi+12]
        push    dword [esi+8]
        push    dword [esi+4]
        push    dword [esi]
        mov     esi, esp
; reverse final round
        mov     ebp, [edi]      ; number of rounds
        mov     ecx, ebp
        shl     ecx, 4
        lea     edi, [edi+ecx+4]        ; edi->last round key
; load buffer into registers
        mov     eax, [esi]
        mov     ebx, [esi+4]
        mov     ecx, [esi+8]
        mov     edx, [esi+12]
; (AddRoundKey)
        xor     eax, [edi]
        xor     ebx, [edi+4]
        xor     ecx, [edi+8]
        xor     edx, [edi+12]
; (ShiftRows)
.loop0:
        xchg    ch, dh
        xchg    bh, ch
        xchg    ah, bh
        rol     eax, 16
        rol     ebx, 16
        rol     ecx, 16
        rol     edx, 16
        xchg    al, cl
        xchg    bl, dl
        xchg    ah, bh
        xchg    bh, ch
        xchg    ch, dh
        rol     eax, 16
        rol     ebx, 16
        rol     ecx, 16
        rol     edx, 16
; (SubBytes)
        mov     [esi], eax
        mov     [esi+4], ebx
        mov     [esi+8], ecx
        mov     [esi+12], edx
        mov     ecx, 16
@@:
        movzx   eax, byte [esi]
        mov     al, [aes.sbox_rev+eax]
        mov     byte [esi], al
        add     esi, 1
        sub     ecx, 1
        jnz     @b
        sub     esi, 16
        sub     edi, 16
; reverse normal rounds
        sub     ebp, 1
        jz      .done
        mov     eax, [esi]
        mov     ebx, [esi+4]
        mov     ecx, [esi+8]
        mov     edx, [esi+12]
        push    esi edi
; (AddRoundKey)
        xor     eax, [edi]
        xor     ebx, [edi+4]
        xor     ecx, [edi+8]
        xor     edx, [edi+12]
; (MixColumns)
macro mix_reg reg {
        movzx   esi, reg#l
        mov     edi, [aes.mcrtable+esi*4]
        movzx   esi, reg#h
        rol     e#reg#x, 16
        mov     esi, [aes.mcrtable+esi*4]
        rol     esi, 8
        xor     edi, esi
        movzx   esi, reg#l
        mov     esi, [aes.mcrtable+esi*4]
        rol     esi, 16
        xor     edi, esi
        movzx   esi, reg#h
        mov     esi, [aes.mcrtable+esi*4]
        ror     esi, 8
        xor     edi, esi
        mov     e#reg#x, edi
}
        mix_reg a
        mix_reg b
        mix_reg c
        mix_reg d
purge mix_reg
        pop     edi esi
        jmp     .loop0
.done:
; (AddRoundKey)
        mov     esi, [esp+20]
        pop     eax
        xor     eax, [edi]
        mov     [esi], eax
        pop     eax
        xor     eax, [edi+4]
        mov     [esi+4], eax
        pop     eax
        xor     eax, [edi+8]
        mov     [esi+8], eax
        pop     eax
        xor     eax, [edi+12]
        mov     [esi+12], eax
        pop     ebp ebx
        ret
