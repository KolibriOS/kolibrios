; Implementation of SHA-256 hash algorithm.
; Written by diamond in 2007.

iglobal
align 4
sha256_start_digest:
        dd      0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A
        dd      0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
sha256_const:
        dd      0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5
        dd      0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5
        dd      0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3
        dd      0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174
        dd      0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC
        dd      0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA
        dd      0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7
        dd      0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967
        dd      0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13
        dd      0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85
        dd      0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3
        dd      0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070
        dd      0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5
        dd      0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3
        dd      0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208
        dd      0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
endg

uglobal
align 4
sha256_buf      rb      64
sha256_digest   rd      8
sha256_count    dd      ?
sha256_size     dq      ?
endg

sha256_init:
        mov     edi, sha256_digest
        mov     esi, sha256_start_digest
        push    8
        pop     ecx
        rep     movsd
        xor     eax, eax
        stosd           ; 0 bytes in buffer
        stosd
        stosd           ; 0 bytes processed
        ret

; Core of SHA-256: transform 64-byte 'sha256_buf' to updated 'sha256_digest'
sha256_transform:
        mov     esi, sha256_buf
        mov     edi, esi
rept 16
{
        lodsd
        bswap   eax
        stosd
}
        push    ebp
        mov     ebp, [esi+7*4]
        mov     edi, [esi+6*4]
        push    dword [esi+5*4]
        push    dword [esi+4*4]
        push    dword [esi+3*4]
        push    dword [esi+2*4]
        push    dword [esi+1*4]
        push    dword [esi+0*4]
        xor     ecx, ecx
.loop:
macro cmd1 cmd,a,b
{
if (b and 7) = 7
        cmd     a, ebp
else if (b and 7) = 6
        cmd     a, edi
else
        cmd     a, [esp+(b and 7)*4]
end if
}
macro cmd2 cmd,a,b
{
if (a and 7) = 7
        cmd     ebp, b
else if (a and 7) = 6
        cmd     edi, b
else
        cmd     [esp+(a and 7)*4], b
end if
}
rept 16 counter
{
cmd1    mov,    eax, (5-counter)
        ror     eax, 6
        mov     edx, eax
        ror     eax, 5
        xor     edx, eax
        ror     eax, 14
        xor     edx, eax
cmd1    mov,    eax, (6-counter)
cmd1    mov,    esi, (7-counter)
        xor     eax, esi
cmd1    and,    eax, (5-counter)
        xor     eax, esi
        add     edx, eax
        add     edx, [sha256_const+ecx+(counter-1)*4]
        add     edx, dword [sha256_buf+(counter-1)*4]
        test    ecx, ecx
        jz      @f
        mov     eax, dword [sha256_buf+((counter-3) and 15)*4]
        mov     esi, eax
        ror     eax, 17
        shr     esi, 10
        xor     esi, eax
        ror     eax, 2
        xor     esi, eax
        add     esi, dword [sha256_buf+((counter-8) and 15)*4]
        mov     eax, dword [sha256_buf+(counter and 15)*4]
        mov     ebx, eax
        ror     eax, 7
        shr     ebx, 3
        xor     ebx, eax
        ror     eax, 11
        xor     ebx, eax
        add     esi, ebx
        add     dword [sha256_buf+(counter-1)*4], esi
        add     edx, esi
@@:
cmd1    add,    edx, (8-counter)
cmd2    mov,    (8-counter), edx
cmd2    add,    (4-counter), edx
cmd1    mov,    ebx, (1-counter)
        mov     eax, ebx
cmd1    mov,    edx, (2-counter)
        mov     esi, ebx
        ror     eax, 2
        or      esi, edx
        and     ebx, edx
cmd1    and,    esi, (3-counter)
        mov     edx, eax
        or      esi, ebx
        ror     eax, 11
        xor     edx, eax
        ror     eax, 9
        xor     edx, eax
        add     esi, edx
cmd2    add,    (8-counter), esi
}
purge cmd1,cmd2
        add     cl, 64
        jnz     .loop
        mov     esi, sha256_digest
        pop     eax
        add     [esi+0*4], eax
        pop     eax
        add     [esi+1*4], eax
        pop     eax
        add     [esi+2*4], eax
        pop     eax
        add     [esi+3*4], eax
        pop     eax
        add     [esi+4*4], eax
        pop     eax
        add     [esi+5*4], eax
        add     [esi+6*4], edi
        add     [esi+7*4], ebp
        pop     ebp
        ret

sha256_update.transform:
        push    esi edx
        call    sha256_transform
        pop     edx esi
        mov     [sha256_count], ecx

sha256_update:
; in: esi->data, edx=size
        mov     eax, 64
        sub     eax, [sha256_count]
        sub     eax, edx
        sbb     ecx, ecx
        and     ecx, eax
        add     ecx, edx
        sub     edx, ecx
        mov     edi, sha256_buf
        add     edi, [sha256_count]
        add     [sha256_count], ecx
        add     dword [sha256_size], ecx
        adc     dword [sha256_size+4], 0
        rep     movsb
        cmp     edi, sha256_buf+64
        jz      .transform
.ret:
        ret

sha256_final:
; out: edi->digest
        push    edi
        mov     eax, [sha256_count]
        mov     [sha256_buf+eax], 0x80
        inc     eax
        cmp     al, 64-8
        jbe     @f
        lea     edi, [sha256_buf+eax]
        push    64
        pop     ecx
        sub     ecx, eax
        xor     eax, eax
        rep     stosb
        push    edx
        call    sha256_transform
        pop     edx
        xor     eax, eax
@@:
        push    64-8
        pop     ecx
        sub     ecx, eax
        lea     edi, [sha256_buf+eax]
        xor     eax, eax
        rep     stosb
        mov     eax, dword [sha256_size]
        mov     edx, dword [sha256_size+4]
        shld    edx, eax, 3
        shl     eax, 3
        bswap   edx
        bswap   eax
        xchg    eax, edx
        stosd
        xchg    eax, edx
        stosd
        call    sha256_transform
        mov     esi, sha256_digest
        mov     cl, 8
        pop     edi
@@:
        lodsd
        bswap   eax
        stosd
        loop    @b
        ret
