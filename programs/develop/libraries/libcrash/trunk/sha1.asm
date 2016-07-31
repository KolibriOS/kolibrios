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


proc sha1._.f
        push    ebx ecx edx
        xor     ecx, edx
        and     ebx, ecx
        xor     ebx, edx
        mov     esi, ebx
        pop     edx ecx ebx
        ret
endp

proc sha1._.g
        push    ebx ecx edx
        xor     ebx, ecx
        xor     ebx, edx
        mov     esi, ebx
        pop     edx ecx ebx
        ret
endp

proc sha1._.h
        push    ebx ecx edx
        mov     esi, ebx
        and     ebx, ecx
        and     ecx, edx
        and     esi, edx
        or      ebx, ecx
        or      esi, ebx
        pop     edx ecx ebx
        ret
endp

macro sha1._.round f, k, c
{
        mov     esi, eax
        rol     esi, 5
        mov     [temp], esi
        call    f

        add     esi, edi
        add     [temp], esi
        mov     esi, [w + (c)*4]
        add     esi, k
        add     [temp], esi

        mov     edi, edx
        mov     edx, ecx
        mov     ecx, ebx
        rol     ecx, 30
        mov     ebx, eax
        mov     eax, [temp]
}


proc sha1.init _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha1.hash]
        mov     esi, sha1._.hash_init
        mov     ecx, SHA1_HASH_SIZE/4
        rep     movsd
        xor     eax, eax
        mov     [ebx + ctx_sha1.index], eax
        mov     [ebx + ctx_sha1.msglen_0], eax
        mov     [ebx + ctx_sha1.msglen_1], eax
        ret
endp


proc sha1._.block _hash
locals
        temp     rd 1
        w        rd 80
endl
        lea     edi, [w]
        xor     ecx, ecx
    @@:
        mov     eax, [esi]
        add     esi, 4
        bswap   eax
        mov     [edi], eax
        add     edi, 4
        add     ecx, 1
        cmp     ecx, 16
        jne     @b
    @@:
        mov     eax, [w + (ecx -  3)*4]
        xor     eax, [w + (ecx -  8)*4]
        xor     eax, [w + (ecx - 14)*4]
        xor     eax, [w + (ecx - 16)*4]
        rol     eax, 1
        mov     [w + ecx*4], eax
        add     ecx, 1
        cmp     ecx, 80
        jne     @b

        mov     edi, [_hash]
        mov     eax, [edi + 0x00]
        mov     ebx, [edi + 0x04]
        mov     ecx, [edi + 0x08]
        mov     edx, [edi + 0x0c]
        mov     edi, [edi + 0x10]

        push    esi

repeat 20
        sha1._.round    sha1._.f, 0x5a827999, %-1
end repeat

repeat 20
        sha1._.round    sha1._.g, 0x6ed9eba1, %-1+20
end repeat

repeat 20
        sha1._.round    sha1._.h, 0x8f1bbcdc, %-1+40
end repeat

repeat 20
        sha1._.round    sha1._.g, 0xca62c1d6, %-1+60
end repeat

        pop     esi

        mov     [temp], edi
        mov     edi, [_hash]
        add     [edi + 0x00], eax
        add     [edi + 0x04], ebx
        add     [edi + 0x08], ecx
        add     [edi + 0x0c], edx
        mov     eax, [temp]
        add     [edi + 0x10], eax

        ret
endp


proc sha1.update _ctx, _msg, _size
        mov     ebx, [_ctx]
        mov     ecx, [_size]
        add     [ebx + ctx_sha1.msglen_0], ecx
        adc     [ebx + ctx_sha1.msglen_1], 0

  .next_block:
        mov     ebx, [_ctx]
        mov     esi, [_msg]
        mov     eax, [ebx + ctx_sha1.index]
        and     eax, SHA1_BLOCK_SIZE-1
        jnz     .copy_to_buf
        test    esi, SHA1_ALIGN_MASK
        jnz     .copy_to_buf
  .no_copy:
        ; data is aligned, hash it in place without copying
        mov     ebx, [_ctx]
        cmp     [_size], SHA1_BLOCK_SIZE
        jb      .copy_quit
        lea     eax, [ebx + ctx_sha1.hash]
        stdcall sha1._.block, eax
        sub     [_size], SHA1_BLOCK_SIZE
;        add     esi, SHA1_BLOCK_SIZE           ; FIXME
        jmp     .no_copy

  .copy_to_buf:
        lea     edi, [ebx + ctx_sha1.block]
        add     edi, eax
        mov     ecx, SHA1_BLOCK_SIZE
        sub     ecx, eax
        cmp     [_size], ecx
        jb      .copy_quit
        sub     [_size], ecx
        add     [_msg], ecx
        add     [ebx + ctx_sha1.index], ecx
        rep     movsb
        lea     eax, [ebx + ctx_sha1.hash]
        lea     esi, [ebx + ctx_sha1.block]
        stdcall sha1._.block, eax
        jmp     .next_block

  .copy_quit:
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha1.block]
        mov     eax, [ebx + ctx_sha1.index]
        and     eax, SHA1_BLOCK_SIZE-1
        add     edi, eax
        mov     ecx, [_size]
        add     [ebx + ctx_sha1.index], ecx
        rep     movsb
  .quit:

        ret
endp


proc sha1.final _ctx
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha1.block]
        mov     ecx, [ebx + ctx_sha1.msglen_0]
        and     ecx, SHA1_BLOCK_SIZE-1
        add     edi, ecx
        mov     byte[edi], 0x80
        inc     edi
        neg     ecx
        add     ecx, SHA1_BLOCK_SIZE
        cmp     ecx, 8
        ja      .last

        dec     ecx
        xor     eax, eax
        rep     stosb
        lea     esi, [ebx + ctx_sha1.block]
        lea     eax, [ebx + ctx_sha1.hash]
        stdcall sha1._.block, eax
        mov     ebx, [_ctx]
        lea     edi, [ebx + ctx_sha1.block]
        mov     ecx, SHA1_BLOCK_SIZE+1
  .last:
        dec     ecx
        sub     ecx, 8
        xor     eax, eax
        rep     stosb
        mov     eax, [ebx + ctx_sha1.msglen_0]
        mov     edx, [ebx + ctx_sha1.msglen_1]
        shld    edx, eax, 3
        shl     eax, 3
        bswap   eax
        bswap   edx
        mov     dword[edi], edx
        mov     dword[edi+4], eax
        lea     esi, [ebx + ctx_sha1.block]
        lea     eax, [ebx + ctx_sha1.hash]
        stdcall sha1._.block, eax

        mov     ebx, [_ctx]
        lea     eax, [ebx + ctx_sha1.hash]
        stdcall sha1._.postprocess, ebx, eax

        ret
endp


proc sha1._.postprocess _ctx, _hash
        mov     ecx, 5
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


align SHA1_ALIGN

sha1._.hash_init dd 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0

