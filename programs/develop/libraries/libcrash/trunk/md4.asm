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


macro md4._.f b, c, d
{
        mov     eax, c
        xor     eax, d
        and     eax, b
        xor     eax, d
}

macro md4._.g b, c, d
{
        push    c d
        mov     eax, b
        and     eax, c
        and     c, d
        and     d, b
        or      eax, c
        or      eax, d
        pop     d c
}

macro md4._.h b, c, d
{
        mov     eax, b
        xor     eax, c
        xor     eax, d
}

macro md4._.round func, a, b, c, d, index, shift, ac
{
        func    b, c, d
        add     eax, [esi + index*4]
        lea     a, [a + eax + ac]
        rol     a, shift
}


proc md4.init _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_md4.hash]
        mov     esi, md4._.hash_init
        mov     ecx, MD4_HASH_SIZE/4
        rep     movsd
        xor     eax, eax
        mov     [ebx + ctx_md4.index], eax
        mov     [ebx + ctx_md4.msglen_0], eax
        mov     [ebx + ctx_md4.msglen_1], eax
        ret
endp


proc md4._.block _hash

        mov     eax, [_hash]
        mov     edi, [eax + 0x0]
        mov     ebx, [eax + 0x4]
        mov     ecx, [eax + 0x8]
        mov     edx, [eax + 0xc]

        md4._.round     md4._.f, edi, ebx, ecx, edx,  0,  3, 0x00000000
        md4._.round     md4._.f, edx, edi, ebx, ecx,  1,  7, 0x00000000
        md4._.round     md4._.f, ecx, edx, edi, ebx,  2, 11, 0x00000000
        md4._.round     md4._.f, ebx, ecx, edx, edi,  3, 19, 0x00000000
        md4._.round     md4._.f, edi, ebx, ecx, edx,  4,  3, 0x00000000
        md4._.round     md4._.f, edx, edi, ebx, ecx,  5,  7, 0x00000000
        md4._.round     md4._.f, ecx, edx, edi, ebx,  6, 11, 0x00000000
        md4._.round     md4._.f, ebx, ecx, edx, edi,  7, 19, 0x00000000
        md4._.round     md4._.f, edi, ebx, ecx, edx,  8,  3, 0x00000000
        md4._.round     md4._.f, edx, edi, ebx, ecx,  9,  7, 0x00000000
        md4._.round     md4._.f, ecx, edx, edi, ebx, 10, 11, 0x00000000
        md4._.round     md4._.f, ebx, ecx, edx, edi, 11, 19, 0x00000000
        md4._.round     md4._.f, edi, ebx, ecx, edx, 12,  3, 0x00000000
        md4._.round     md4._.f, edx, edi, ebx, ecx, 13,  7, 0x00000000
        md4._.round     md4._.f, ecx, edx, edi, ebx, 14, 11, 0x00000000
        md4._.round     md4._.f, ebx, ecx, edx, edi, 15, 19, 0x00000000

        md4._.round     md4._.g, edi, ebx, ecx, edx,  0,  3, 0x5a827999
        md4._.round     md4._.g, edx, edi, ebx, ecx,  4,  5, 0x5a827999
        md4._.round     md4._.g, ecx, edx, edi, ebx,  8,  9, 0x5a827999
        md4._.round     md4._.g, ebx, ecx, edx, edi, 12, 13, 0x5a827999
        md4._.round     md4._.g, edi, ebx, ecx, edx,  1,  3, 0x5a827999
        md4._.round     md4._.g, edx, edi, ebx, ecx,  5,  5, 0x5a827999
        md4._.round     md4._.g, ecx, edx, edi, ebx,  9,  9, 0x5a827999
        md4._.round     md4._.g, ebx, ecx, edx, edi, 13, 13, 0x5a827999
        md4._.round     md4._.g, edi, ebx, ecx, edx,  2,  3, 0x5a827999
        md4._.round     md4._.g, edx, edi, ebx, ecx,  6,  5, 0x5a827999
        md4._.round     md4._.g, ecx, edx, edi, ebx, 10,  9, 0x5a827999
        md4._.round     md4._.g, ebx, ecx, edx, edi, 14, 13, 0x5a827999
        md4._.round     md4._.g, edi, ebx, ecx, edx,  3,  3, 0x5a827999
        md4._.round     md4._.g, edx, edi, ebx, ecx,  7,  5, 0x5a827999
        md4._.round     md4._.g, ecx, edx, edi, ebx, 11,  9, 0x5a827999
        md4._.round     md4._.g, ebx, ecx, edx, edi, 15, 13, 0x5a827999

        md4._.round     md4._.h, edi, ebx, ecx, edx,  0,  3, 0x6ed9eba1
        md4._.round     md4._.h, edx, edi, ebx, ecx,  8,  9, 0x6ed9eba1
        md4._.round     md4._.h, ecx, edx, edi, ebx,  4, 11, 0x6ed9eba1
        md4._.round     md4._.h, ebx, ecx, edx, edi, 12, 15, 0x6ed9eba1
        md4._.round     md4._.h, edi, ebx, ecx, edx,  2,  3, 0x6ed9eba1
        md4._.round     md4._.h, edx, edi, ebx, ecx, 10,  9, 0x6ed9eba1
        md4._.round     md4._.h, ecx, edx, edi, ebx,  6, 11, 0x6ed9eba1
        md4._.round     md4._.h, ebx, ecx, edx, edi, 14, 15, 0x6ed9eba1
        md4._.round     md4._.h, edi, ebx, ecx, edx,  1,  3, 0x6ed9eba1
        md4._.round     md4._.h, edx, edi, ebx, ecx,  9,  9, 0x6ed9eba1
        md4._.round     md4._.h, ecx, edx, edi, ebx,  5, 11, 0x6ed9eba1
        md4._.round     md4._.h, ebx, ecx, edx, edi, 13, 15, 0x6ed9eba1
        md4._.round     md4._.h, edi, ebx, ecx, edx,  3,  3, 0x6ed9eba1
        md4._.round     md4._.h, edx, edi, ebx, ecx, 11,  9, 0x6ed9eba1
        md4._.round     md4._.h, ecx, edx, edi, ebx,  7, 11, 0x6ed9eba1
        md4._.round     md4._.h, ebx, ecx, edx, edi, 15, 15, 0x6ed9eba1

        mov     eax, [_hash]
        add     [eax + 0x0], edi
        add     [eax + 0x4], ebx
        add     [eax + 0x8], ecx
        add     [eax + 0xc], edx

        ret
endp


proc md4.update _ctx, _msg, _size
        mov     ebx, [_ctx]
        mov     ecx, [_size]
        add     [ebx + ctx_md4.msglen_0], ecx
        adc     [ebx + ctx_md4.msglen_1], 0

  .next_block:
        mov     ebx, [_ctx]
        mov     esi, [_msg]
        mov     eax, [ebx + ctx_md4.index]
        and     eax, MD4_BLOCK_SIZE-1
        jnz     .copy_to_buf
        test    esi, MD4_ALIGN_MASK
        jnz     .copy_to_buf
  .no_copy:
        ; data is aligned, hash it in place without copying
        mov     ebx, [_ctx]
        cmp     [_size], MD4_BLOCK_SIZE
        jb      .copy_quit
        lea     eax, [ebx + ctx_md4.hash]
        stdcall md4._.block, eax
        sub     [_size], MD4_BLOCK_SIZE
        add     esi, MD4_BLOCK_SIZE
        jmp     .no_copy

  .copy_to_buf:
        lea     edi, [ebx + ctx_md4.block]
        add     edi, eax
        mov     ecx, MD4_BLOCK_SIZE
        sub     ecx, eax
        cmp     [_size], ecx
        jb      .copy_quit
        sub     [_size], ecx
        add     [_msg], ecx
        add     [ebx + ctx_md4.index], ecx
        rep     movsb
        lea     eax, [ebx + ctx_md4.hash]
        lea     esi, [ebx + ctx_md4.block]
        stdcall md4._.block, eax
        jmp     .next_block

  .copy_quit:
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_md4.block]
        mov     eax, [ebx + ctx_md4.index]
        and     eax, MD4_BLOCK_SIZE-1
        add     edi, eax
        mov     ecx, [_size]
        add     [ebx + ctx_md4.index], ecx
        rep     movsb
  .quit:
        ret
endp


proc md4.final _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_md4.block]
        mov     ecx, [ebx + ctx_md4.msglen_0]
        and     ecx, MD4_BLOCK_SIZE-1
        add     edi, ecx
        mov     byte[edi], 0x80
        inc     edi
        neg     ecx
        add     ecx, MD4_BLOCK_SIZE
        cmp     ecx, 8
        ja      .last

        dec     ecx
        xor     eax, eax
        rep     stosb
        lea     esi, [ebx + ctx_md4.block]
        lea     eax, [ebx + ctx_md4.hash]
        stdcall md4._.block, eax
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_md4.block]
        mov     ecx, MD4_BLOCK_SIZE+1
  .last:
        dec     ecx
        sub     ecx, 8
        xor     eax, eax
        rep     stosb
        mov     eax, [ebx + ctx_md4.msglen_0]
        mov     edx, [ebx + ctx_md4.msglen_1]
        shld    edx, eax, 3
        shl     eax, 3
        mov     dword[edi], eax
        mov     dword[edi+4], edx
        lea     esi, [ebx + ctx_md4.block]
        lea     eax, [ebx + ctx_md4.hash]
        stdcall md4._.block, eax

        ret
endp


align MD4_ALIGN

md4._.hash_init dd 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0

