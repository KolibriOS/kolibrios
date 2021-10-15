; libcrash -- cryptographic hash (and other) functions
;
; Copyright (C) <2012-2013,2016,2019,2021> Ivan Baravy
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

SHA2_224256_BLOCK_SIZE = 64
SHA2_224_BLOCK_SIZE    = SHA2_224256_BLOCK_SIZE
SHA2_256_BLOCK_SIZE    = SHA2_224256_BLOCK_SIZE

SHA2_224256_INIT_SIZE  = 32
SHA2_224256_ALIGN      = 4
SHA2_224256_ALIGN_MASK = SHA2_224256_ALIGN - 1

struct ctx_sha2_224256
        hash            rb SHA2_224256_INIT_SIZE
        block           rb SHA2_224256_BLOCK_SIZE
        index           rd 1
        msglen_0        rd 1
        msglen_1        rd 1
ends

assert sizeof.ctx_sha2_224256 <= LIBCRASH_CTX_LEN

macro sha2_224256._.chn x, y, z
{
        mov     eax, [y]
        xor     eax, [z]
        and     eax, [x]
        xor     eax, [z]
}

macro sha2_224256._.maj x, y, z
{
        mov     eax, [x]
        xor     eax, [y]
        and     eax, [z]
        mov     ecx, [x]
        and     ecx, [y]
        xor     eax, ecx
}

macro sha2_224256._.Sigma0 x
{
        mov     eax, x
        mov     ecx, eax
        ror     ecx, 2
        ror     eax, 13
        xor     eax, ecx
        mov     ecx, x
        ror     ecx, 22
        xor     eax, ecx
}

macro sha2_224256._.Sigma1 x
{
        mov     eax, x
        mov     ecx, eax
        ror     ecx, 6
        ror     eax, 11
        xor     eax, ecx
        mov     ecx, x
        ror     ecx, 25
        xor     eax, ecx
}

macro sha2_224256._.sigma0 x
{
        mov     eax, x
        mov     ecx, eax
        ror     ecx, 7
        ror     eax, 18
        xor     eax, ecx
        mov     ecx, x
        shr     ecx, 3
        xor     eax, ecx
}

macro sha2_224256._.sigma1 x
{
        mov     eax, x
        mov     ecx, eax
        ror     ecx, 17
        ror     eax, 19
        xor     eax, ecx
        mov     ecx, x
        shr     ecx, 10
        xor     eax, ecx
}

macro sha2_224256._.recalculate_w n
{
        mov     edx, [w + ((n-2) and 15)*4]
        sha2_224256._.sigma1  edx
        add     eax, [w + ((n-7) and 15)*4]
        push    eax
        mov     edx, [w + ((n-15) and 15)*4]
        sha2_224256._.sigma0  edx
        pop     ecx
        add     eax, ecx
        add     [w + (n)*4], eax
}

macro sha2_224256._.round a, b, c, d, e, f, g, h, k
{
        mov     ebx, [h]
        mov     edx, [e]
        sha2_224256._.Sigma1  edx

        add     ebx, eax
        sha2_224256._.chn     e, f, g

        add     ebx, eax
        add     ebx, [k]
        add     ebx, edi

        add     [d], ebx

        mov     edx, [a]
        sha2_224256._.Sigma0  edx
        add     ebx, eax
        sha2_224256._.maj     a, b, c
        add     eax, ebx
        mov     [h], eax
}


macro sha2_224256._.round_1_16 a, b, c, d, e, f, g, h, n
{

        mov     eax, [esi + (n)*4]
        bswap   eax

        mov     dword[w + (n)*4], eax
        mov     edi, eax
        sha2_224256._.round a, b, c, d, e, f, g, h, (sha2_256_table + (n)*4)
}

macro sha2_224256._.round_17_64 a, b, c, d, e, f, g, h, n, rep_num
{
        sha2_224256._.recalculate_w n
        mov     edi, [w + (n)*4]
        sha2_224256._.round a, b, c, d, e, f, g, h, (sha2_256_table + (n+16*rep_num)*4)
}


proc sha2_224.init uses ebx esi edi, _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha2_224256.hash]
        mov     esi, sha2_224._.hash_init
        mov     ecx, SHA2_224256_INIT_SIZE/4
        rep movsd
        xor     eax, eax
        mov     [ebx + ctx_sha2_224256.index], eax
        mov     [ebx + ctx_sha2_224256.msglen_0], eax
        mov     [ebx + ctx_sha2_224256.msglen_1], eax
        ret
endp


proc sha2_256.init uses ebx esi edi, _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha2_224256.hash]
        mov     esi, sha2_256._.hash_init
        mov     ecx, SHA2_224256_INIT_SIZE/4
        rep movsd
        xor     eax, eax
        mov     [ebx + ctx_sha2_224256.index], eax
        mov     [ebx + ctx_sha2_224256.msglen_0], eax
        mov     [ebx + ctx_sha2_224256.msglen_1], eax
        ret
endp


proc sha2_224256._.block _hash
locals
        w       rd 64
        A       rd 1
        B       rd 1
        C       rd 1
        D       rd 1
        E       rd 1
        F       rd 1
        G       rd 1
        H       rd 1
endl
        mov     edi, [_hash]
        mov     eax, [edi + 0x00]
        mov     [A], eax
        mov     eax, [edi + 0x04]
        mov     [B], eax
        mov     eax, [edi + 0x08]
        mov     [C], eax
        mov     eax, [edi + 0x0c]
        mov     [D], eax
        mov     eax, [edi + 0x10]
        mov     [E], eax
        mov     eax, [edi + 0x14]
        mov     [F], eax
        mov     eax, [edi + 0x18]
        mov     [G], eax
        mov     eax, [edi + 0x1c]
        mov     [H], eax

        sha2_224256._.round_1_16  A, B, C, D, E, F, G, H,  0
        sha2_224256._.round_1_16  H, A, B, C, D, E, F, G,  1
        sha2_224256._.round_1_16  G, H, A, B, C, D, E, F,  2
        sha2_224256._.round_1_16  F, G, H, A, B, C, D, E,  3
        sha2_224256._.round_1_16  E, F, G, H, A, B, C, D,  4
        sha2_224256._.round_1_16  D, E, F, G, H, A, B, C,  5
        sha2_224256._.round_1_16  C, D, E, F, G, H, A, B,  6
        sha2_224256._.round_1_16  B, C, D, E, F, G, H, A,  7
        sha2_224256._.round_1_16  A, B, C, D, E, F, G, H,  8
        sha2_224256._.round_1_16  H, A, B, C, D, E, F, G,  9
        sha2_224256._.round_1_16  G, H, A, B, C, D, E, F, 10
        sha2_224256._.round_1_16  F, G, H, A, B, C, D, E, 11
        sha2_224256._.round_1_16  E, F, G, H, A, B, C, D, 12
        sha2_224256._.round_1_16  D, E, F, G, H, A, B, C, 13
        sha2_224256._.round_1_16  C, D, E, F, G, H, A, B, 14
        sha2_224256._.round_1_16  B, C, D, E, F, G, H, A, 15

repeat 3
        sha2_224256._.round_17_64 A, B, C, D, E, F, G, H,  0, %
        sha2_224256._.round_17_64 H, A, B, C, D, E, F, G,  1, %
        sha2_224256._.round_17_64 G, H, A, B, C, D, E, F,  2, %
        sha2_224256._.round_17_64 F, G, H, A, B, C, D, E,  3, %
        sha2_224256._.round_17_64 E, F, G, H, A, B, C, D,  4, %
        sha2_224256._.round_17_64 D, E, F, G, H, A, B, C,  5, %
        sha2_224256._.round_17_64 C, D, E, F, G, H, A, B,  6, %
        sha2_224256._.round_17_64 B, C, D, E, F, G, H, A,  7, %
        sha2_224256._.round_17_64 A, B, C, D, E, F, G, H,  8, %
        sha2_224256._.round_17_64 H, A, B, C, D, E, F, G,  9, %
        sha2_224256._.round_17_64 G, H, A, B, C, D, E, F, 10, %
        sha2_224256._.round_17_64 F, G, H, A, B, C, D, E, 11, %
        sha2_224256._.round_17_64 E, F, G, H, A, B, C, D, 12, %
        sha2_224256._.round_17_64 D, E, F, G, H, A, B, C, 13, %
        sha2_224256._.round_17_64 C, D, E, F, G, H, A, B, 14, %
        sha2_224256._.round_17_64 B, C, D, E, F, G, H, A, 15, %
end repeat

        mov     edi, [_hash]
        mov     eax, [A]
        add     [edi + 0x00], eax
        mov     eax, [B]
        add     [edi + 0x04], eax
        mov     eax, [C]
        add     [edi + 0x08], eax
        mov     eax, [D]
        add     [edi + 0x0c], eax
        mov     eax, [E]
        add     [edi + 0x10], eax
        mov     eax, [F]
        add     [edi + 0x14], eax
        mov     eax, [G]
        add     [edi + 0x18], eax
        mov     eax, [H]
        add     [edi + 0x1c], eax

        ret
endp


sha2_224.update = sha2_224256.update
sha2_256.update = sha2_224256.update
proc sha2_224256.update uses ebx esi edi, _ctx, _msg, _size
        mov     ebx, [_ctx]
        mov     ecx, [_size]
        add     [ebx + ctx_sha2_224256.msglen_0], ecx
        adc     [ebx + ctx_sha2_224256.msglen_1], 0

.next_block:
        mov     ebx, [_ctx]
        mov     esi, [_msg]
        mov     eax, [ebx + ctx_sha2_224256.index]
        and     eax, SHA2_224256_BLOCK_SIZE-1
        jnz     .copy_to_buf
        test    esi, SHA2_224256_ALIGN_MASK
        jnz     .copy_to_buf
.no_copy:
        ; data is aligned, hash it in place without copying
        mov     ebx, [_ctx]
        cmp     [_size], SHA2_224256_BLOCK_SIZE
        jb      .copy_quit
        lea     eax, [ebx + ctx_sha2_224256.hash]
        stdcall sha2_224256._.block, eax
        sub     [_size], SHA2_224256_BLOCK_SIZE
        add     esi, SHA2_224256_BLOCK_SIZE           ; FIXME
        jmp     .no_copy

.copy_to_buf:
        lea     edi, [ebx + ctx_sha2_224256.block]
        add     edi, eax
        mov     ecx, SHA2_224256_BLOCK_SIZE
        sub     ecx, eax
        cmp     [_size], ecx
        jb      .copy_quit
        sub     [_size], ecx
        add     [_msg], ecx
        add     [ebx + ctx_sha2_224256.index], ecx
        rep movsb
        lea     eax, [ebx + ctx_sha2_224256.hash]
        lea     esi, [ebx + ctx_sha2_224256.block]
        stdcall sha2_224256._.block, eax
        jmp     .next_block

.copy_quit:
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha2_224256.block]
        mov     eax, [ebx + ctx_sha2_224256.index]
        and     eax, SHA2_224256_BLOCK_SIZE-1
        add     edi, eax
        mov     ecx, [_size]
        add     [ebx + ctx_sha2_224256.index], ecx
        rep movsb
.quit:

        ret
endp


sha2_224.finish = sha2_224256.finish
sha2_256.finish = sha2_224256.finish
proc sha2_224256.finish uses ebx esi edi, _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha2_224256.block]
        mov     ecx, [ebx + ctx_sha2_224256.msglen_0]
        and     ecx, SHA2_224256_BLOCK_SIZE-1
        add     edi, ecx
        mov     byte[edi], 0x80
        inc     edi
        neg     ecx
        add     ecx, SHA2_224256_BLOCK_SIZE
        cmp     ecx, 8
        ja      .last

        dec     ecx
        xor     eax, eax
        rep stosb
        lea     esi, [ebx + ctx_sha2_224256.block]
        lea     eax, [ebx + ctx_sha2_224256.hash]
        stdcall sha2_224256._.block, eax
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha2_224256.block]
        mov     ecx, SHA2_224256_BLOCK_SIZE+1
.last:
        dec     ecx
        sub     ecx, 8
        xor     eax, eax
        rep stosb
        mov     eax, [ebx + ctx_sha2_224256.msglen_0]
        mov     edx, [ebx + ctx_sha2_224256.msglen_1]
        shld    edx, eax, 3
        shl     eax, 3
        bswap   eax
        bswap   edx
        mov     dword[edi], edx
        mov     dword[edi+4], eax
        lea     esi, [ebx + ctx_sha2_224256.block]
        lea     eax, [ebx + ctx_sha2_224256.hash]
        stdcall sha2_224256._.block, eax

        mov     ebx, [_ctx]
        lea     eax, [ebx + ctx_sha2_224256.hash]
        stdcall sha2_224256._.postprocess, ebx, eax

        ret
endp


proc sha2_224256._.postprocess _ctx, _hash
        mov     ecx, 8
        mov     esi, [_hash]
        mov     edi, esi
@@:
        lodsd
        bswap   eax
        stosd
        dec     ecx
        jnz     @b
        ret
endp


proc sha2_224.oneshot _ctx, _data, _len
        stdcall sha2_224.init, [_ctx]
        stdcall sha2_224.update, [_ctx], [_data], [_len]
        stdcall sha2_224.finish, [_ctx]
        ret
endp


proc sha2_256.oneshot _ctx, _data, _len
        stdcall sha2_256.init, [_ctx]
        stdcall sha2_256.update, [_ctx], [_data], [_len]
        stdcall sha2_256.finish, [_ctx]
        ret
endp


iglobal
align SHA2_224256_ALIGN
sha2_224._.hash_init    dd 0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939,\
                           0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4

sha2_256._.hash_init    dd 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,\
                           0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19

sha2_256_table          dd 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,\
                           0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,\
                           0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,\
                           0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,\
                           0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,\
                           0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,\
                           0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,\
                           0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,\
                           0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,\
                           0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,\
                           0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,\
                           0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,\
                           0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,\
                           0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,\
                           0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,\
                           0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
endg
