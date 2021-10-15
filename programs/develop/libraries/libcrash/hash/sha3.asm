; libcrash -- cryptographic hash (and other) functions
;
; Copyright (C) <2013,2016,2019,2021> Ivan Baravy
;
; SPDX-License-Identifier: GPL-2.0-or-later
;
; This program is free software: you can redistribute it and/or modify it under
; the terms of the GNU General Public License as published by the Free Software
; Foundation, either version 2 of the License, or (at your option) any later
; version.
;
; This program is distributed in the hope that it will be useful, but WITHOUT
; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
; FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along with
; this program. If not, see <http://www.gnu.org/licenses/>.

SHA3_224_BLOCK_SIZE = 144
SHA3_256_BLOCK_SIZE = 136
SHA3_384_BLOCK_SIZE = 104
SHA3_512_BLOCK_SIZE = 72
SHA3_MAX_BLOCK_SIZE = SHA3_224_BLOCK_SIZE

SHA3_INIT_SIZE          = 200
SHA3_ALIGN              = 16
SHA3_ALIGN_MASK         = SHA3_ALIGN-1

struct ctx_sha3
        hash            rb SHA3_INIT_SIZE
                        rb SHA3_ALIGN - (SHA3_INIT_SIZE mod SHA3_ALIGN)
        block           rb SHA3_MAX_BLOCK_SIZE
                        rb SHA3_ALIGN - (SHA3_MAX_BLOCK_SIZE mod SHA3_ALIGN)
        index           rd 1
        block_size      rd 1
        rounds_cnt      rd 1
                        rd 1    ; align
        ; tmp vars
        C               rq 5
        D               rq 5
ends

assert sizeof.ctx_sha3 <= LIBCRASH_CTX_LEN

macro sha3._.rol_xor nd, ncl, ncr
{
        movq    mm0, [C + 8*(ncl)]
        movq    mm1, mm0
        psllq   mm0, 1
        psrlq   mm1, 63
        por     mm0, mm1
        pxor    mm0, [C + 8*(ncr)]
        movq    [D + 8*(nd)], mm0
}

proc sha3._.theta
;locals
;        C       rq 5
;        D       rq 5
;endl
C equ ebx + ctx_sha3.C
D equ ebx + ctx_sha3.D

repeat 5
        movq    mm0, [edi + 8*(%-1 +  0)]
        pxor    mm0, [edi + 8*(%-1 +  5)]
        pxor    mm0, [edi + 8*(%-1 + 10)]
        pxor    mm0, [edi + 8*(%-1 + 15)]
        pxor    mm0, [edi + 8*(%-1 + 20)]
        movq    [C + 8*(%-1)], mm0
end repeat

        sha3._.rol_xor  0, 1, 4
        sha3._.rol_xor  1, 2, 0
        sha3._.rol_xor  2, 3, 1
        sha3._.rol_xor  3, 4, 2
        sha3._.rol_xor  4, 0, 3

repeat 5
        movq    mm1, [D + 8*(%-1)]
        movq    mm0, mm1
        pxor    mm0, [edi + 8*(%-1 +  0)]
        movq    [edi + 8*(%-1 +  0)], mm0
        movq    mm0, mm1
        pxor    mm0, [edi + 8*(%-1 +  5)]
        movq    [edi + 8*(%-1 +  5)], mm0
        movq    mm0, mm1
        pxor    mm0, [edi + 8*(%-1 + 10)]
        movq    [edi + 8*(%-1 + 10)], mm0
        movq    mm0, mm1
        pxor    mm0, [edi + 8*(%-1 + 15)]
        movq    [edi + 8*(%-1 + 15)], mm0
        movq    mm0, mm1
        pxor    mm0, [edi + 8*(%-1 + 20)]
        movq    [edi + 8*(%-1 + 20)], mm0
end repeat

restore C,D
        ret
endp


proc sha3._.pi
        movq    mm1, [edi + 8*1]
        movq    mm0, [edi + 8*6]
        movq    [edi + 8*1], mm0
        movq    mm0, [edi + 8*9]
        movq    [edi + 8*6], mm0
        movq    mm0, [edi + 8*22]
        movq    [edi + 8*9], mm0
        movq    mm0, [edi + 8*14]
        movq    [edi + 8*22], mm0
        movq    mm0, [edi + 8*20]
        movq    [edi + 8*14], mm0
        movq    mm0, [edi + 8*2]
        movq    [edi + 8*20], mm0
        movq    mm0, [edi + 8*12]
        movq    [edi + 8*2], mm0
        movq    mm0, [edi + 8*13]
        movq    [edi + 8*12], mm0
        movq    mm0, [edi + 8*19]
        movq    [edi + 8*13], mm0
        movq    mm0, [edi + 8*23]
        movq    [edi + 8*19], mm0
        movq    mm0, [edi + 8*15]
        movq    [edi + 8*23], mm0
        movq    mm0, [edi + 8*4]
        movq    [edi + 8*15], mm0
        movq    mm0, [edi + 8*24]
        movq    [edi + 8*4], mm0
        movq    mm0, [edi + 8*21]
        movq    [edi + 8*24], mm0
        movq    mm0, [edi + 8*8]
        movq    [edi + 8*21], mm0
        movq    mm0, [edi + 8*16]
        movq    [edi + 8*8], mm0
        movq    mm0, [edi + 8*5]
        movq    [edi + 8*16], mm0
        movq    mm0, [edi + 8*3]
        movq    [edi + 8*5], mm0
        movq    mm0, [edi + 8*18]
        movq    [edi + 8*3], mm0
        movq    mm0, [edi + 8*17]
        movq    [edi + 8*18], mm0
        movq    mm0, [edi + 8*11]
        movq    [edi + 8*17], mm0
        movq    mm0, [edi + 8*7]
        movq    [edi + 8*11], mm0
        movq    mm0, [edi + 8*10]
        movq    [edi + 8*7], mm0
        movq    [edi + 8*10], mm1

        ret
endp


proc sha3._.chi

        mov     eax, 0xffffffff
        movd    mm0, eax
        movq    mm2, mm0
        punpckldq mm2, mm0

repeat 5
        movq    mm6, [edi + 8*(0 + 5*(%-1))]
        movq    mm7, [edi + 8*(1 + 5*(%-1))]

        movq    mm0, [edi + 8*(0 + 5*(%-1))]
        movq    mm1, mm7
        pandn   mm1, mm2
        pand    mm1, [edi + 8*(2 + 5*(%-1))]
        pxor    mm0, mm1
        movq    [edi + 8*(0 + 5*(%-1))], mm0

        movq    mm0, [edi + 8*(1 + 5*(%-1))]
        movq    mm1, [edi + 8*(2 + 5*(%-1))]
        pandn   mm1, mm2
        pand    mm1, [edi + 8*(3 + 5*(%-1))]
        pxor    mm0, mm1
        movq    [edi + 8*(1 + 5*(%-1))], mm0

        movq    mm0, [edi + 8*(2 + 5*(%-1))]
        movq    mm1, [edi + 8*(3 + 5*(%-1))]
        pandn   mm1, mm2
        pand    mm1, [edi + 8*(4 + 5*(%-1))]
        pxor    mm0, mm1
        movq    [edi + 8*(2 + 5*(%-1))], mm0

        movq    mm0, [edi + 8*(3 + 5*(%-1))]
        movq    mm1, [edi + 8*(4 + 5*(%-1))]
        pandn   mm1, mm2
        pand    mm1, mm6
        pxor    mm0, mm1
        movq    [edi + 8*(3 + 5*(%-1))], mm0

        movq    mm0, [edi + 8*(4 + 5*(%-1))]
        movq    mm1, mm6
        pandn   mm1, mm2
        pand    mm1, mm7
        pxor    mm0, mm1
        movq    [edi + 8*(4 + 5*(%-1))], mm0
end repeat
        ret
endp


macro sha3._.rol_mov n, c
{
        movq    mm0, [edi + 8*(n)]
        movq    mm1, mm0
        psllq   mm0, (c)
        psrlq   mm1, (64-(c))
        por     mm0, mm1
        movq    [edi + 8*(n)], mm0
}

proc sha3._.permutation

repeat 24
        stdcall sha3._.theta

        sha3._.rol_mov   1,  1
        sha3._.rol_mov   2, 62
        sha3._.rol_mov   3, 28
        sha3._.rol_mov   4, 27
        sha3._.rol_mov   5, 36
        sha3._.rol_mov   6, 44
        sha3._.rol_mov   7,  6
        sha3._.rol_mov   8, 55
        sha3._.rol_mov   9, 20
        sha3._.rol_mov  10,  3
        sha3._.rol_mov  11, 10
        sha3._.rol_mov  12, 43
        sha3._.rol_mov  13, 25
        sha3._.rol_mov  14, 39
        sha3._.rol_mov  15, 41
        sha3._.rol_mov  16, 45
        sha3._.rol_mov  17, 15
        sha3._.rol_mov  18, 21
        sha3._.rol_mov  19,  8
        sha3._.rol_mov  20, 18
        sha3._.rol_mov  21,  2
        sha3._.rol_mov  22, 61
        sha3._.rol_mov  23, 56
        sha3._.rol_mov  24, 14

        stdcall sha3._.pi
        stdcall sha3._.chi

        movq    mm0, [edi + 8*(0)]
        pxor    mm0, [sha3._.round + 8*(%-1)]
        movq    [edi + 8*(0)], mm0
end repeat

        ret
endp


proc sha3._.init uses edi
        mov     [ebx + ctx_sha3.block_size], eax
        shr     eax, 3
        dec     eax
        mov     [ebx + ctx_sha3.rounds_cnt], eax
        xor     eax, eax
        lea     edi, [ebx + ctx_sha3.hash]
        mov     ecx, SHA3_INIT_SIZE/4
        rep stosd
        mov     [ebx + ctx_sha3.index], eax
        ret
endp


proc sha3_224.init uses ebx, _ctx
        mov     ebx, [_ctx]
        mov     eax, SHA3_224_BLOCK_SIZE
        stdcall sha3._.init
        ret
endp


proc sha3_256.init uses ebx, _ctx
        mov     ebx, [_ctx]
        mov     eax, SHA3_256_BLOCK_SIZE
        stdcall sha3._.init
        ret
endp


proc sha3_384.init uses ebx, _ctx
        mov     ebx, [_ctx]
        mov     eax, SHA3_384_BLOCK_SIZE
        stdcall sha3._.init
        ret
endp


proc sha3_512.init uses ebx, _ctx
        mov     ebx, [_ctx]
        mov     eax, SHA3_512_BLOCK_SIZE
        stdcall sha3._.init
        ret
endp


proc sha3._.block _hash
        mov     ecx, [ebx + ctx_sha3.rounds_cnt]
        mov     edi, [_hash]

    @@:
        movq    mm0, [esi + 8*ecx]
        pxor    mm0, [edi + 8*ecx]
        movq    [edi + 8*ecx], mm0
        dec     ecx
        jns     @b

        stdcall sha3._.permutation

        ret
endp


sha3_224.update = sha3.update
sha3_256.update = sha3.update
sha3_384.update = sha3.update
sha3_512.update = sha3.update
proc sha3.update uses ebx esi edi, _ctx, _msg, _size
.next_block:
        mov     ebx, [_ctx]
        mov     esi, [_msg]
        mov     eax, [ebx + ctx_sha3.index]
        test    eax, eax
        jnz     .copy_to_buf
        test    esi, SHA3_ALIGN_MASK
        jnz     .copy_to_buf
.no_copy:
        ; data is aligned, hash it in place without copying
        mov     ebx, [_ctx]
        mov     eax, [ebx + ctx_sha3.block_size]
        cmp     [_size], eax
        jb      .copy_quit
        lea     eax, [ebx + ctx_sha3.hash]
        push    ebx esi
        stdcall sha3._.block, eax
        pop     esi ebx
        mov     eax, [ebx + ctx_sha3.block_size]
        sub     [_size], eax
        add     esi, [ebx + ctx_sha3.block_size]
        jmp     .no_copy

.copy_to_buf:
        lea     edi, [ebx + ctx_sha3.block]
        add     edi, eax
        mov     ecx, [ebx + ctx_sha3.block_size]
        sub     ecx, eax
        cmp     [_size], ecx
        jb      .copy_quit
        sub     [_size], ecx
        add     [_msg], ecx
        add     [ebx + ctx_sha3.index], ecx
        mov     eax, [ebx + ctx_sha3.block_size]
        cmp     [ebx + ctx_sha3.index], eax
        jb      @f
        sub     [ebx + ctx_sha3.index], eax
    @@:
        rep movsb
        lea     eax, [ebx + ctx_sha3.hash]
        lea     esi, [ebx + ctx_sha3.block]
        stdcall sha3._.block, eax
        jmp     .next_block

.copy_quit:
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha3.block]
        mov     eax, [ebx + ctx_sha3.index]
        add     edi, eax
        mov     ecx, [_size]
        add     [ebx + ctx_sha3.index], ecx
        rep movsb
.quit:
        ret
endp


sha3_224.finish = sha3.finish
sha3_256.finish = sha3.finish
sha3_384.finish = sha3.finish
sha3_512.finish = sha3.finish
proc sha3.finish uses ebx esi edi, _ctx
        mov     ebx, [_ctx]
        mov     eax, [ebx + ctx_sha3.index]
        mov     ecx, [ebx + ctx_sha3.block_size]
        sub     ecx, eax
        lea     edi, [ebx+ctx_sha3.block]
        add     edi, eax
        mov     byte[edi], 0x06
        inc     edi
        dec     ecx
        xor     eax, eax
        rep stosb
        or      byte[edi - 1], 0x80

        mov     ebx, [_ctx]
        lea     esi, [ebx + ctx_sha3.block]
        lea     eax, [ebx + ctx_sha3.hash]
        stdcall sha3._.block, eax

        mov     ebx, [_ctx]
        lea     eax, [ebx + ctx_sha3.hash]
        stdcall sha3._.postprocess, ebx, eax
        ret
endp


proc sha3._.postprocess _ctx, _hash
        emms
        ret
endp


proc sha3_224.oneshot _ctx, _data, _len
        stdcall sha3_224.init, [_ctx]
        stdcall sha3.update, [_ctx], [_data], [_len]
        stdcall sha3.finish, [_ctx]
        ret
endp


proc sha3_256.oneshot _ctx, _data, _len
        stdcall sha3_256.init, [_ctx]
        stdcall sha3.update, [_ctx], [_data], [_len]
        stdcall sha3.finish, [_ctx]
        ret
endp


proc sha3_384.oneshot _ctx, _data, _len
        stdcall sha3_384.init, [_ctx]
        stdcall sha3.update, [_ctx], [_data], [_len]
        stdcall sha3.finish, [_ctx]
        ret
endp


proc sha3_512.oneshot _ctx, _data, _len
        stdcall sha3_512.init, [_ctx]
        stdcall sha3.update, [_ctx], [_data], [_len]
        stdcall sha3.finish, [_ctx]
        ret
endp


iglobal
align SHA3_ALIGN
sha3._.round    dq 0x0000000000000001, 0x0000000000008082, 0x800000000000808A,\
                   0x8000000080008000, 0x000000000000808B, 0x0000000080000001,\
                   0x8000000080008081, 0x8000000000008009, 0x000000000000008A,\
                   0x0000000000000088, 0x0000000080008009, 0x000000008000000A,\
                   0x000000008000808B, 0x800000000000008B, 0x8000000000008089,\
                   0x8000000000008003, 0x8000000000008002, 0x8000000000000080,\
                   0x000000000000800A, 0x800000008000000A, 0x8000000080008081,\
                   0x8000000000008080, 0x0000000080000001, 0x8000000080008008
endg
