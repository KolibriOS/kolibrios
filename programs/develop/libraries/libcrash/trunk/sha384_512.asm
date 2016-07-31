;    libcrash -- cryptographic hash functions
;
;    Copyright (C) 2012-2013,2016 Ivan Baravy (dunkaist)
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.


macro sha384512._.chn x, y, z
{
        movq    mm0, [y]
        pxor    mm0, [z]
        pand    mm0, [x]
        pxor    mm0, [z]
}

macro sha384512._.maj x, y, z
{
        movq    mm0, [x]
        pxor    mm0, [y]
        pand    mm0, [z]
        movq    mm2, [x]
        pand    mm2, [y]
        pxor    mm0, mm2
}

macro sha384512._.Sigma0 x
{
        movq    mm0, x
        movq    mm2, mm0
        movq    mm7, mm2
        psrlq   mm2, 28
        psllq   mm7, 36
        por     mm2, mm7
        movq    mm7, mm0
        psrlq   mm0, 34
        psllq   mm7, 30
        por     mm0, mm7
        pxor    mm0, mm2
        movq    mm2, x
        movq    mm7, mm2
        psrlq   mm2, 39
        psllq   mm7, 25
        por     mm2, mm7
        pxor    mm0, mm2
}

macro sha384512._.Sigma1 x
{
        movq    mm0, x
        movq    mm2, mm0
        movq    mm7, mm2
        psrlq   mm2, 14
        psllq   mm7, 50
        por     mm2, mm7
        movq    mm7, mm0
        psrlq   mm0, 18
        psllq   mm7, 46
        por     mm0, mm7
        pxor    mm0, mm2
        movq    mm2, x
        movq    mm7, mm2
        psrlq   mm2, 41
        psllq   mm7, 23
        por     mm2, mm7
        pxor    mm0, mm2
}

macro sha384512._.sigma0 x
{
        movq    mm0, x
        movq    mm2, mm0
        movq    mm7, mm2
        psrlq   mm2, 1
        psllq   mm7, 63
        por     mm2, mm7
        movq    mm7, mm0
        psrlq   mm0, 8
        psllq   mm7, 56
        por     mm0, mm7
        pxor    mm0, mm2
        movq    mm2, x
        psrlq   mm2, 7
        pxor    mm0, mm2
}

macro sha384512._.sigma1 x
{
        movq    mm0, x
        movq    mm2, mm0
        movq    mm7, mm2
        psrlq   mm2, 19
        psllq   mm7, 45
        por     mm2, mm7
        movq    mm7, mm0
        psrlq   mm0, 61
        psllq   mm7, 3
        por     mm0, mm7
        pxor    mm0, mm2
        movq    mm2, x
        psrlq   mm2, 6
        pxor    mm0, mm2
}

macro sha384512._.recalculate_w n
{
        movq    mm3, [w + ((n-2) and 15)*8]
        sha384512._.sigma1  mm3
        paddq   mm0, [w + ((n-7) and 15)*8]
        movq    mm6, mm0
        movq    mm3, [w + ((n-15) and 15)*8]
        sha384512._.sigma0  mm3
        movq    mm2, mm6
        paddq   mm0, mm2
        movq    mm7, [w + (n)*8]
        paddq   mm7, mm0
        movq    [w + (n)*8], mm7
}

macro sha384512._.round a, b, c, d, e, f, g, h, k
{
        movq    mm1, [h]
        movq    mm3, [e]
        sha384512._.Sigma1  mm3
        paddq   mm1, mm0
        sha384512._.chn     e, f, g
        paddq   mm1, mm0
        paddq   mm1, [k]
        paddq   mm1, mm5
        movq    mm7, [d]
        paddq   mm7, mm1
        movq    [d], mm7
        movq    mm3, [a]
        sha384512._.Sigma0  mm3
        paddq   mm1, mm0
        sha384512._.maj     a, b, c
        paddq   mm0, mm1
        movq    [h], mm0
}


macro sha384512._.round_1_16 a, b, c, d, e, f, g, h, n
{

        movq    mm0, [esi + (n)*8]
        movq    [temp], mm0
        mov     eax, dword[temp]
        bswap   eax
        push    eax
        mov     eax, dword[temp + 4]
        bswap   eax
        mov     dword[temp], eax
        pop     eax
        mov     dword[temp + 4], eax
        movq    mm0, [temp]
        movq    [w + (n)*8], mm0
        movq    mm5, mm0
        sha384512._.round a, b, c, d, e, f, g, h, (sha384512._.table + (n)*8)
}

macro sha384512._.round_17_64 a, b, c, d, e, f, g, h, n, rep_num
{
        sha384512._.recalculate_w n
        movq    mm5, [w + (n)*8]
        sha384512._.round a, b, c, d, e, f, g, h, (sha384512._.table + (n+16*rep_num)*8)
}


proc sha384.init _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha384512.hash]
        mov     esi, sha384._.hash_init
        mov     ecx, SHA384512_INIT_SIZE/4
        rep     movsd
        xor     eax, eax
        mov     [ebx + ctx_sha384512.index], eax
        mov     [ebx + ctx_sha384512.msglen_0], eax
        mov     [ebx + ctx_sha384512.msglen_1], eax
        mov     [ebx + ctx_sha384512.msglen_2], eax
        mov     [ebx + ctx_sha384512.msglen_3], eax
        ret
endp


proc sha512.init _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha384512.hash]
        mov     esi, sha512._.hash_init
        mov     ecx, SHA384512_INIT_SIZE/4
        rep     movsd
        xor     eax, eax
        mov     [ebx + ctx_sha384512.index], eax
        mov     [ebx + ctx_sha384512.msglen_0], eax
        mov     [ebx + ctx_sha384512.msglen_1], eax
        mov     [ebx + ctx_sha384512.msglen_2], eax
        mov     [ebx + ctx_sha384512.msglen_3], eax
        ret
endp


proc sha384512._.block _hash
;locals
;        w       rq 80
;        A       rq 1
;        B       rq 1
;        C       rq 1
;        D       rq 1
;        E       rq 1
;        F       rq 1
;        G       rq 1
;        H       rq 1
;        temp    rq 1
;endl
w equ ebx + ctx_sha384512.w
A equ ebx + ctx_sha384512.A
B equ ebx + ctx_sha384512.B
C equ ebx + ctx_sha384512.C
D equ ebx + ctx_sha384512.D
E equ ebx + ctx_sha384512.E
F equ ebx + ctx_sha384512.F
G equ ebx + ctx_sha384512.G
H equ ebx + ctx_sha384512.H
temp equ ebx + ctx_sha384512.temp

        mov     edi, [_hash]
        movq    mm0, [edi + 0x00]
        movq    [A], mm0
        movq    mm0, [edi + 0x08]
        movq    [B], mm0
        movq    mm0, [edi + 0x10]
        movq    [C], mm0
        movq    mm0, [edi + 0x18]
        movq    [D], mm0
        movq    mm0, [edi + 0x20]
        movq    [E], mm0
        movq    mm0, [edi + 0x28]
        movq    [F], mm0
        movq    mm0, [edi + 0x30]
        movq    [G], mm0
        movq    mm0, [edi + 0x38]
        movq    [H], mm0


        sha384512._.round_1_16  A, B, C, D, E, F, G, H,  0
        sha384512._.round_1_16  H, A, B, C, D, E, F, G,  1
        sha384512._.round_1_16  G, H, A, B, C, D, E, F,  2
        sha384512._.round_1_16  F, G, H, A, B, C, D, E,  3
        sha384512._.round_1_16  E, F, G, H, A, B, C, D,  4
        sha384512._.round_1_16  D, E, F, G, H, A, B, C,  5
        sha384512._.round_1_16  C, D, E, F, G, H, A, B,  6
        sha384512._.round_1_16  B, C, D, E, F, G, H, A,  7
        sha384512._.round_1_16  A, B, C, D, E, F, G, H,  8
        sha384512._.round_1_16  H, A, B, C, D, E, F, G,  9
        sha384512._.round_1_16  G, H, A, B, C, D, E, F, 10
        sha384512._.round_1_16  F, G, H, A, B, C, D, E, 11
        sha384512._.round_1_16  E, F, G, H, A, B, C, D, 12
        sha384512._.round_1_16  D, E, F, G, H, A, B, C, 13
        sha384512._.round_1_16  C, D, E, F, G, H, A, B, 14
        sha384512._.round_1_16  B, C, D, E, F, G, H, A, 15

repeat 4
        sha384512._.round_17_64 A, B, C, D, E, F, G, H,  0, %
        sha384512._.round_17_64 H, A, B, C, D, E, F, G,  1, %
        sha384512._.round_17_64 G, H, A, B, C, D, E, F,  2, %
        sha384512._.round_17_64 F, G, H, A, B, C, D, E,  3, %
        sha384512._.round_17_64 E, F, G, H, A, B, C, D,  4, %
        sha384512._.round_17_64 D, E, F, G, H, A, B, C,  5, %
        sha384512._.round_17_64 C, D, E, F, G, H, A, B,  6, %
        sha384512._.round_17_64 B, C, D, E, F, G, H, A,  7, %
        sha384512._.round_17_64 A, B, C, D, E, F, G, H,  8, %
        sha384512._.round_17_64 H, A, B, C, D, E, F, G,  9, %
        sha384512._.round_17_64 G, H, A, B, C, D, E, F, 10, %
        sha384512._.round_17_64 F, G, H, A, B, C, D, E, 11, %
        sha384512._.round_17_64 E, F, G, H, A, B, C, D, 12, %
        sha384512._.round_17_64 D, E, F, G, H, A, B, C, 13, %
        sha384512._.round_17_64 C, D, E, F, G, H, A, B, 14, %
        sha384512._.round_17_64 B, C, D, E, F, G, H, A, 15, %
end repeat


        mov     edi, [_hash]
        movq    mm0, [A]
        paddq   mm0, [edi + 0x00]
        movq    [edi + 0x00], mm0
        movq    mm0, [B]
        paddq   mm0, [edi + 0x08]
        movq    [edi + 0x08], mm0
        movq    mm0, [C]
        paddq   mm0, [edi + 0x10]
        movq    [edi + 0x10], mm0
        movq    mm0, [D]
        paddq   mm0, [edi + 0x18]
        movq    [edi + 0x18], mm0
        movq    mm0, [E]
        paddq   mm0, [edi + 0x20]
        movq    [edi + 0x20], mm0
        movq    mm0, [F]
        paddq   mm0, [edi + 0x28]
        movq    [edi + 0x28], mm0
        movq    mm0, [G]
        paddq   mm0, [edi + 0x30]
        movq    [edi + 0x30], mm0
        movq    mm0, [H]
        paddq   mm0, [edi + 0x38]
        movq    [edi + 0x38], mm0

        ret
restore w,A,B,C,D,E,F,G,H,temp
endp

sha512.update = sha384.update
proc sha384.update _ctx, _msg, _size
        mov     ebx, [_ctx]
        mov     ecx, [_size]
        add     [ebx + ctx_sha384512.msglen_0], ecx
        adc     [ebx + ctx_sha384512.msglen_1], 0
        adc     [ebx + ctx_sha384512.msglen_2], 0
        adc     [ebx + ctx_sha384512.msglen_3], 0

  .next_block:
        mov     ebx, [_ctx]
        mov     esi, [_msg]
        mov     eax, [ebx + ctx_sha384512.index]
        and     eax, SHA384512_BLOCK_SIZE-1
        jnz     .copy_to_buf
        test    esi, SHA384512_ALIGN_MASK
        jnz     .copy_to_buf
  .no_copy:
        ; data is aligned, hash it in place without copying
        mov     ebx, [_ctx]
        cmp     [_size], SHA384512_BLOCK_SIZE
        jb      .copy_quit
        lea     eax, [ebx + ctx_sha384512.hash]
        stdcall sha384512._.block, eax
        sub     [_size], SHA384512_BLOCK_SIZE
        add     esi, SHA384512_BLOCK_SIZE           ; FIXME
        jmp     .no_copy

  .copy_to_buf:
        lea     edi, [ebx + ctx_sha384512.block]
        add     edi, eax
        mov     ecx, SHA384512_BLOCK_SIZE
        sub     ecx, eax
        cmp     [_size], ecx
        jb      .copy_quit
        sub     [_size], ecx
        add     [_msg], ecx
        add     [ebx + ctx_sha384512.index], ecx
        rep     movsb
        lea     eax, [ebx + ctx_sha384512.hash]
        lea     esi, [ebx + ctx_sha384512.block]
        stdcall sha384512._.block, eax
        jmp     .next_block

  .copy_quit:
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha384512.block]
        mov     eax, [ebx + ctx_sha384512.index]
        and     eax, SHA384512_BLOCK_SIZE-1
        add     edi, eax
        mov     ecx, [_size]
        add     [ebx + ctx_sha384512.index], ecx
        rep     movsb
  .quit:

        ret
endp


sha512.final = sha384.final
proc sha384.final _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha384512.block]
        mov     ecx, [ebx + ctx_sha384512.msglen_0]
        and     ecx, SHA384512_BLOCK_SIZE-1
        add     edi, ecx
        mov     byte[edi], 0x80
        inc     edi
        neg     ecx
        add     ecx, SHA384512_BLOCK_SIZE
        cmp     ecx, 16
        ja      .last

        dec     ecx
        xor     eax, eax
        rep     stosb
        lea     esi, [ebx + ctx_sha384512.block]
        lea     eax, [ebx + ctx_sha384512.hash]
        stdcall sha384512._.block, eax
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha384512.block]
        mov     ecx, SHA384512_BLOCK_SIZE+1
  .last:
        dec     ecx
        sub     ecx, 16
        xor     eax, eax
        rep     stosb
        mov     eax, [ebx + ctx_sha384512.msglen_1]
        shld    [ebx + ctx_sha384512.msglen_0], eax, 3
        mov     eax, [ebx + ctx_sha384512.msglen_2]
        shld    [ebx + ctx_sha384512.msglen_1], eax, 3
        mov     eax, [ebx + ctx_sha384512.msglen_3]
        shld    [ebx + ctx_sha384512.msglen_2], eax, 3
        shl     eax, 3
        bswap   eax
        mov     dword[edi + 0], eax
        mov     eax, [ebx + ctx_sha384512.msglen_2]
        bswap   eax
        mov     dword[edi + 4], eax
        mov     eax, [ebx + ctx_sha384512.msglen_1]
        bswap   eax
        mov     dword[edi + 8], eax
        mov     eax, [ebx + ctx_sha384512.msglen_0]
        bswap   eax
        mov     dword[edi + 12], eax
        mov     ebx, [_ctx]
        lea     esi, [ebx + ctx_sha384512.block]
        lea     eax, [ebx + ctx_sha384512.hash]
        stdcall sha384512._.block, eax

        mov     ebx, [_ctx]
        lea     eax, [ebx + ctx_sha384512.hash]
        stdcall sha384512._.postprocess, ebx, eax

        ret
endp


proc sha384512._.postprocess _ctx, _hash
        mov     ecx, 8
        mov     esi, [_hash]
        mov     edi, esi
    @@:
        lodsd
        mov     ebx, eax
        lodsd
        bswap   eax
        bswap   ebx
        stosd
        mov     eax, ebx
        stosd
        dec     ecx     ; FIXME: what should I fix here?
        jnz     @b
        emms
        ret
endp


align SHA384512_ALIGN

sha384._.hash_init      dq 0xcbbb9d5dc1059ed8, 0x629a292a367cd507,\
                           0x9159015a3070dd17, 0x152fecd8f70e5939,\
                           0x67332667ffc00b31, 0x8eb44a8768581511,\
                           0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4

sha512._.hash_init      dq 0x6a09e667f3bcc908, 0xbb67ae8584caa73b,\
                           0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,\
                           0x510e527fade682d1, 0x9b05688c2b3e6c1f,\
                           0x1f83d9abfb41bd6b, 0x5be0cd19137e2179

sha384512._.table       dq 0x428a2f98d728ae22, 0x7137449123ef65cd,\
                           0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,\
                           0x3956c25bf348b538, 0x59f111f1b605d019,\
                           0x923f82a4af194f9b, 0xab1c5ed5da6d8118,\
                           0xd807aa98a3030242, 0x12835b0145706fbe,\
                           0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,\
                           0x72be5d74f27b896f, 0x80deb1fe3b1696b1,\
                           0x9bdc06a725c71235, 0xc19bf174cf692694,\
                           0xe49b69c19ef14ad2, 0xefbe4786384f25e3,\
                           0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,\
                           0x2de92c6f592b0275, 0x4a7484aa6ea6e483,\
                           0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,\
                           0x983e5152ee66dfab, 0xa831c66d2db43210,\
                           0xb00327c898fb213f, 0xbf597fc7beef0ee4,\
                           0xc6e00bf33da88fc2, 0xd5a79147930aa725,\
                           0x06ca6351e003826f, 0x142929670a0e6e70,\
                           0x27b70a8546d22ffc, 0x2e1b21385c26c926,\
                           0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,\
                           0x650a73548baf63de, 0x766a0abb3c77b2a8,\
                           0x81c2c92e47edaee6, 0x92722c851482353b,\
                           0xa2bfe8a14cf10364, 0xa81a664bbc423001,\
                           0xc24b8b70d0f89791, 0xc76c51a30654be30,\
                           0xd192e819d6ef5218, 0xd69906245565a910,\
                           0xf40e35855771202a, 0x106aa07032bbd1b8,\
                           0x19a4c116b8d2d0c8, 0x1e376c085141ab53,\
                           0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,\
                           0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb,\
                           0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,\
                           0x748f82ee5defb2fc, 0x78a5636f43172f60,\
                           0x84c87814a1f0ab72, 0x8cc702081a6439ec,\
                           0x90befffa23631e28, 0xa4506cebde82bde9,\
                           0xbef9a3f7b2c67915, 0xc67178f2e372532b,\
                           0xca273eceea26619c, 0xd186b8c721c0c207,\
                           0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,\
                           0x06f067aa72176fba, 0x0a637dc5a2c898a6,\
                           0x113f9804bef90dae, 0x1b710b35131c471b,\
                           0x28db77f523047d84, 0x32caab7b40c72493,\
                           0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,\
                           0x4cc5d4becb3e42b6, 0x597f299cfc657e2a,\
                           0x5fcb6fab3ad6faec, 0x6c44198c4a475817

