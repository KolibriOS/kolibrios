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

MD5_BLOCK_SIZE = 64

MD5_ALIGN      = 4
MD5_ALIGN_MASK = MD5_ALIGN - 1

struct ctx_md5
        hash            rb MD5_LEN
        block           rb MD5_BLOCK_SIZE
        index           rd 1
        msglen_0        rd 1
        msglen_1        rd 1
ends

assert sizeof.ctx_md5 <= LIBCRASH_CTX_LEN

macro md5._.f b, c, d
{
        push    c
        xor     c, d
        and     b, c
        xor     b, d
        pop     c
}

macro md5._.g b, c, d
{
        push    c  d
        and     b, d
        not     d
        and     c, d
        or      b, c
        pop     d  c
}

macro md5._.h b, c, d
{
        xor     b, c
        xor     b, d
}

macro md5._.i b, c, d
{
        push    d
        not     d
        or      b, d
        xor     b, c
        pop     d
}

macro md5._.round func, a, b, c, d, index, shift, ac
{
        push    b
        func    b, c, d
        lea     a, [a + b + ac]
        add     a, [esi + index*4]
        rol     a, shift
        pop     b
        add     a, b
}


proc md5.init uses ebx esi edi, _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_md5.hash]
        mov     esi, md5._.hash_init
        mov     ecx, MD5_LEN/4
        rep movsd
        xor     eax, eax
        mov     [ebx + ctx_md5.index], eax
        mov     [ebx + ctx_md5.msglen_0], eax
        mov     [ebx + ctx_md5.msglen_1], eax
        ret
endp


proc md5._.block _hash

        mov     edi, [_hash]
        mov     eax, [edi + 0x0]
        mov     ebx, [edi + 0x4]
        mov     ecx, [edi + 0x8]
        mov     edx, [edi + 0xc]

        md5._.round     md5._.f, eax, ebx, ecx, edx,  0,  7, 0xd76aa478
        md5._.round     md5._.f, edx, eax, ebx, ecx,  1, 12, 0xe8c7b756
        md5._.round     md5._.f, ecx, edx, eax, ebx,  2, 17, 0x242070db
        md5._.round     md5._.f, ebx, ecx, edx, eax,  3, 22, 0xc1bdceee
        md5._.round     md5._.f, eax, ebx, ecx, edx,  4,  7, 0xf57c0faf
        md5._.round     md5._.f, edx, eax, ebx, ecx,  5, 12, 0x4787c62a
        md5._.round     md5._.f, ecx, edx, eax, ebx,  6, 17, 0xa8304613
        md5._.round     md5._.f, ebx, ecx, edx, eax,  7, 22, 0xfd469501
        md5._.round     md5._.f, eax, ebx, ecx, edx,  8,  7, 0x698098d8
        md5._.round     md5._.f, edx, eax, ebx, ecx,  9, 12, 0x8b44f7af
        md5._.round     md5._.f, ecx, edx, eax, ebx, 10, 17, 0xffff5bb1
        md5._.round     md5._.f, ebx, ecx, edx, eax, 11, 22, 0x895cd7be
        md5._.round     md5._.f, eax, ebx, ecx, edx, 12,  7, 0x6b901122
        md5._.round     md5._.f, edx, eax, ebx, ecx, 13, 12, 0xfd987193
        md5._.round     md5._.f, ecx, edx, eax, ebx, 14, 17, 0xa679438e
        md5._.round     md5._.f, ebx, ecx, edx, eax, 15, 22, 0x49b40821

        md5._.round     md5._.g, eax, ebx, ecx, edx,  1,  5, 0xf61e2562
        md5._.round     md5._.g, edx, eax, ebx, ecx,  6,  9, 0xc040b340
        md5._.round     md5._.g, ecx, edx, eax, ebx, 11, 14, 0x265e5a51
        md5._.round     md5._.g, ebx, ecx, edx, eax,  0, 20, 0xe9b6c7aa
        md5._.round     md5._.g, eax, ebx, ecx, edx,  5,  5, 0xd62f105d
        md5._.round     md5._.g, edx, eax, ebx, ecx, 10,  9, 0x02441453
        md5._.round     md5._.g, ecx, edx, eax, ebx, 15, 14, 0xd8a1e681
        md5._.round     md5._.g, ebx, ecx, edx, eax,  4, 20, 0xe7d3fbc8
        md5._.round     md5._.g, eax, ebx, ecx, edx,  9,  5, 0x21e1cde6
        md5._.round     md5._.g, edx, eax, ebx, ecx, 14,  9, 0xc33707d6
        md5._.round     md5._.g, ecx, edx, eax, ebx,  3, 14, 0xf4d50d87
        md5._.round     md5._.g, ebx, ecx, edx, eax,  8, 20, 0x455a14ed
        md5._.round     md5._.g, eax, ebx, ecx, edx, 13,  5, 0xa9e3e905
        md5._.round     md5._.g, edx, eax, ebx, ecx,  2,  9, 0xfcefa3f8
        md5._.round     md5._.g, ecx, edx, eax, ebx,  7, 14, 0x676f02d9
        md5._.round     md5._.g, ebx, ecx, edx, eax, 12, 20, 0x8d2a4c8a

        md5._.round     md5._.h, eax, ebx, ecx, edx,  5,  4, 0xfffa3942
        md5._.round     md5._.h, edx, eax, ebx, ecx,  8, 11, 0x8771f681
        md5._.round     md5._.h, ecx, edx, eax, ebx, 11, 16, 0x6d9d6122
        md5._.round     md5._.h, ebx, ecx, edx, eax, 14, 23, 0xfde5380c
        md5._.round     md5._.h, eax, ebx, ecx, edx,  1,  4, 0xa4beea44
        md5._.round     md5._.h, edx, eax, ebx, ecx,  4, 11, 0x4bdecfa9
        md5._.round     md5._.h, ecx, edx, eax, ebx,  7, 16, 0xf6bb4b60
        md5._.round     md5._.h, ebx, ecx, edx, eax, 10, 23, 0xbebfbc70
        md5._.round     md5._.h, eax, ebx, ecx, edx, 13,  4, 0x289b7ec6
        md5._.round     md5._.h, edx, eax, ebx, ecx,  0, 11, 0xeaa127fa
        md5._.round     md5._.h, ecx, edx, eax, ebx,  3, 16, 0xd4ef3085
        md5._.round     md5._.h, ebx, ecx, edx, eax,  6, 23, 0x04881d05
        md5._.round     md5._.h, eax, ebx, ecx, edx,  9,  4, 0xd9d4d039
        md5._.round     md5._.h, edx, eax, ebx, ecx, 12, 11, 0xe6db99e5
        md5._.round     md5._.h, ecx, edx, eax, ebx, 15, 16, 0x1fa27cf8
        md5._.round     md5._.h, ebx, ecx, edx, eax,  2, 23, 0xc4ac5665

        md5._.round     md5._.i, eax, ebx, ecx, edx,  0,  6, 0xf4292244
        md5._.round     md5._.i, edx, eax, ebx, ecx,  7, 10, 0x432aff97
        md5._.round     md5._.i, ecx, edx, eax, ebx, 14, 15, 0xab9423a7
        md5._.round     md5._.i, ebx, ecx, edx, eax,  5, 21, 0xfc93a039
        md5._.round     md5._.i, eax, ebx, ecx, edx, 12,  6, 0x655b59c3
        md5._.round     md5._.i, edx, eax, ebx, ecx,  3, 10, 0x8f0ccc92
        md5._.round     md5._.i, ecx, edx, eax, ebx, 10, 15, 0xffeff47d
        md5._.round     md5._.i, ebx, ecx, edx, eax,  1, 21, 0x85845dd1
        md5._.round     md5._.i, eax, ebx, ecx, edx,  8,  6, 0x6fa87e4f
        md5._.round     md5._.i, edx, eax, ebx, ecx, 15, 10, 0xfe2ce6e0
        md5._.round     md5._.i, ecx, edx, eax, ebx,  6, 15, 0xa3014314
        md5._.round     md5._.i, ebx, ecx, edx, eax, 13, 21, 0x4e0811a1
        md5._.round     md5._.i, eax, ebx, ecx, edx,  4,  6, 0xf7537e82
        md5._.round     md5._.i, edx, eax, ebx, ecx, 11, 10, 0xbd3af235
        md5._.round     md5._.i, ecx, edx, eax, ebx,  2, 15, 0x2ad7d2bb
        md5._.round     md5._.i, ebx, ecx, edx, eax,  9, 21, 0xeb86d391

        mov     edi, [_hash]
        add     [edi + 0x0], eax
        add     [edi + 0x4], ebx
        add     [edi + 0x8], ecx
        add     [edi + 0xc], edx

        ret
endp


proc md5.update uses ebx esi edi, _ctx, _msg, _size
        mov     ebx, [_ctx]
        mov     ecx, [_size]
        add     [ebx + ctx_md5.msglen_0], ecx
        adc     [ebx + ctx_md5.msglen_1], 0

.next_block:
        mov     ebx, [_ctx]
        mov     esi, [_msg]
        mov     eax, [ebx + ctx_md5.index]
        and     eax, MD5_BLOCK_SIZE-1
        jnz     .copy_to_buf
        test    esi, MD5_ALIGN_MASK
        jnz     .copy_to_buf
.no_copy:
        ; data is aligned, hash it in place without copying
        mov     ebx, [_ctx]
        cmp     [_size], MD5_BLOCK_SIZE
        jb      .copy_quit
        lea     eax, [ebx + ctx_md5.hash]
        stdcall md5._.block, eax
        sub     [_size], MD5_BLOCK_SIZE
        add     esi, MD5_BLOCK_SIZE
        jmp     .no_copy

.copy_to_buf:
        lea     edi, [ebx + ctx_md5.block]
        add     edi, eax
        mov     ecx, MD5_BLOCK_SIZE
        sub     ecx, eax
        cmp     [_size], ecx
        jb      .copy_quit
        sub     [_size], ecx
        add     [_msg], ecx
        add     [ebx + ctx_md5.index], ecx
        rep movsb
        lea     eax, [ebx + ctx_md5.hash]
        lea     esi, [ebx + ctx_md5.block]
        stdcall md5._.block, eax
        jmp     .next_block

.copy_quit:
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_md5.block]
        mov     eax, [ebx + ctx_md5.index]
        and     eax, MD5_BLOCK_SIZE-1
        add     edi, eax
        mov     ecx, [_size]
        add     [ebx + ctx_md5.index], ecx
        rep movsb
.quit:

        ret
endp


proc md5.finish uses ebx esi edi, _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_md5.block]
        mov     ecx, [ebx + ctx_md5.msglen_0]
        and     ecx, MD5_BLOCK_SIZE-1
        add     edi, ecx
        mov     byte[edi], 0x80
        inc     edi
        neg     ecx
        add     ecx, MD5_BLOCK_SIZE
        cmp     ecx, 8
        ja      .last

        dec     ecx
        xor     eax, eax
        rep stosb
        lea     esi, [ebx + ctx_md5.block]
        lea     eax, [ebx + ctx_md5.hash]
        stdcall md5._.block, eax
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_md5.block]
        mov     ecx, MD5_BLOCK_SIZE+1
.last:
        dec     ecx
        sub     ecx, 8
        xor     eax, eax
        rep stosb
        mov     eax, [ebx + ctx_md5.msglen_0]
        mov     edx, [ebx + ctx_md5.msglen_1]
        shld    edx, eax, 3
        shl     eax, 3
        mov     dword[edi], eax
        mov     dword[edi+4], edx
        lea     esi, [ebx + ctx_md5.block]
        lea     eax, [ebx + ctx_md5.hash]
        stdcall md5._.block, eax

        ret
endp


proc md5.oneshot _ctx, _data, _len
        stdcall md5.init, [_ctx]
        stdcall md5.update, [_ctx], [_data], [_len]
        stdcall md5.finish, [_ctx]
        ret
endp


iglobal
align MD5_ALIGN
md5._.hash_init dd 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0
endg
