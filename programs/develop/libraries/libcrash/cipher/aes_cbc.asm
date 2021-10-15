; libcrash -- cryptographic hash (and other) functions
;
; Copyright (C) <2016> Jeffrey Amelynck
; Copyright (C) <2016,2021> Ivan Baravy
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

struct ctx_aes_cbc
        aes ctx_aes
        cbc ctx_cbc
        crypt dd ?
        finish dd ?
        block rd CBC128_BLOCK_SIZE/4
        index dd ?
        padding dd ?
ends

assert sizeof.ctx_aes_cbc <= LIBCRASH_CTX_LEN

; _crypt: 0/1 = encrypt/decrypt
proc aes256cbc.init uses ebx esi edi, _ctx, _key, _iv, _flags
        mov     ebx, [_ctx]
        stdcall aes256.init, ebx, [_key], [_flags]
        mov     ecx, CBC128_BLOCK_SIZE/4
        mov     esi, [_iv]
        lea     edi, [ebx+ctx_aes_cbc.cbc.vector]
        rep movsd
        mov     [ebx+ctx_aes_cbc.cbc.has_data], 0
        mov     [ebx+ctx_aes_cbc.index], 0
        mov     [ebx+ctx_aes_cbc.crypt], aes256cbc._.encrypt_block
        mov     [ebx+ctx_aes_cbc.finish], aes256cbc._.finish_encrypt
        test    [_flags], LIBCRASH_CIPHER_DECRYPT
        jz      @f
        mov     [ebx+ctx_aes_cbc.crypt], aes256cbc._.decrypt_block
        mov     [ebx+ctx_aes_cbc.finish], aes256cbc._.finish_decrypt
@@:
        xor     eax, eax
        test    [_flags], LIBCRASH_CIPHER_PADDING
        setnz   al
        mov     [ebx+ctx_aes_cbc.padding], eax
        ret
endp

proc aes256cbc._.encrypt_block uses ebx esi edi, _ctx, _in, _out
        mov     ebx, [_ctx]
        mov     esi, [_in]
        lea     edi, [ebx+ctx_aes_cbc.cbc.vector]
        mov     ecx, CBC128_BLOCK_SIZE/4
@@:
        lodsd
        xor     [edi], eax
        add     edi, 4
        dec     ecx
        jnz     @b

        lea     ecx, [ebx+ctx_aes_cbc.cbc.vector]
        lea     edx, [ebx+ctx_aes_cbc.aes.state]
        stdcall aes.encrypt, ebx, ecx, edx
        lea     esi, [ebx+ctx_aes_cbc.aes.state]
        lea     edi, [ebx+ctx_aes_cbc.cbc.vector]
        mov     ecx, CBC128_BLOCK_SIZE/4
        rep movsd
        lea     esi, [ebx+ctx_aes_cbc.aes.state]
        mov     edi, [_out]
        mov     ecx, CBC128_BLOCK_SIZE/4
        rep movsd

        mov     eax, CBC128_BLOCK_SIZE
        ret
endp

proc aes256cbc._.decrypt_block uses ebx esi edi, _ctx, _in, _out
locals
        .done dd ?
endl
        mov     [.done], 0
        mov     ebx, [_ctx]

        mov     ecx, [_in]
        lea     edx, [ebx+ctx_aes_cbc.aes.state]
        stdcall aes.decrypt, ebx, ecx, edx

        bts     [ebx+ctx_aes_cbc.cbc.has_data], 0
        jnc     @f
        lea     esi, [ebx+ctx_aes_cbc.cbc.block]
        mov     edi, [_out]
        mov     ecx, CBC128_BLOCK_SIZE/4
        rep movsd
        add     [.done], CBC128_BLOCK_SIZE
@@:
        lea     esi, [ebx+ctx_aes_cbc.aes.state]
        lea     edx, [ebx+ctx_aes_cbc.cbc.vector]
        lea     edi, [ebx+ctx_aes_cbc.cbc.block]
        mov     ecx, CBC128_BLOCK_SIZE/4
@@:
        lodsd
        xor     eax, [edx]
        add     edx, 4
        stosd
        dec     ecx
        jnz     @b

        mov     esi, [_in]
        lea     edi, [ebx+ctx_aes_cbc.cbc.vector]
        mov     ecx, CBC128_BLOCK_SIZE/4
        rep movsd

        mov     eax, [.done]
        ret
endp

proc aes256cbc.update uses ebx esi edi, _ctx, _in, _len, _out
locals
        .done dd ?
endl
        mov     [.done], 0
.next_block:
        mov     ebx, [_ctx]
        mov     eax, [ebx+ctx_aes_cbc.index]
        test    eax, eax
        jnz     .copy_to_buf
        test    [_in], LIBCRASH_ALIGN-1
        jnz     .copy_to_buf
.no_copy:
        ; data is aligned, process it in place without copying
        mov     ebx, [_ctx]
        cmp     [_len], CBC128_BLOCK_SIZE
        jb      .copy_quit
        stdcall [ebx+ctx_aes_cbc.crypt], [_ctx], [_in], [_out]
        add     [_in], CBC128_BLOCK_SIZE
        add     [_out], eax
        add     [.done], eax
        sub     [_len], CBC128_BLOCK_SIZE
        jmp     .no_copy

.copy_to_buf:
        lea     edi, [ebx+ctx_aes_cbc.block]
        add     edi, [ebx+ctx_aes_cbc.index]
        mov     ecx, CBC128_BLOCK_SIZE
        sub     ecx, [ebx+ctx_aes_cbc.index]
        cmp     [_len], ecx
        jb      .copy_quit
        mov     esi, [_in]
        sub     [_len], ecx
        add     [_in], ecx
        rep movsb
        mov     [ebx+ctx_aes_cbc.index], 0
        lea     esi, [ebx+ctx_aes_cbc.block]
        stdcall [ebx+ctx_aes_cbc.crypt], [_ctx], esi, [_out]
        add     [.done], eax
        add     [_out], eax
        jmp     .next_block

.copy_quit:
        mov     ebx, [_ctx]
        mov     esi, [_in]
        lea     edi, [ebx+ctx_aes_cbc.block]
        add     edi, [ebx+ctx_aes_cbc.index]
        mov     ecx, [_len]
        add     [ebx+ctx_aes_cbc.index], ecx
        rep movsb
.quit:
        mov     eax, [.done]
        ret
endp

proc aes256cbc.finish uses ebx esi edi, _ctx, _out
        mov     ebx, [_ctx]
        stdcall [ebx+ctx_aes_cbc.finish], ebx, [_out]
        ret
endp

proc aes256cbc._.finish_encrypt uses ebx esi edi, _ctx, _out
        mov     ebx, [_ctx]
        xor     eax, eax
        cmp     [ebx+ctx_aes_cbc.padding], 0
        jz      .no_padding
        ; add padding
        lea     edi, [ebx+ctx_aes_cbc.block]
        add     edi, [ebx+ctx_aes_cbc.index]
        mov     ecx, CBC128_BLOCK_SIZE
        sub     ecx, [ebx+ctx_aes_cbc.index]
        mov     eax, ecx
        rep stosb

        lea     eax, [ebx+ctx_aes_cbc.block]
        stdcall aes256cbc._.encrypt_block, [_ctx], eax, [_out]
        mov     eax, CBC128_BLOCK_SIZE
.no_padding:
        ret
endp

proc aes256cbc._.finish_decrypt uses ebx esi edi, _ctx, _out
        mov     ebx, [_ctx]
        xor     eax, eax
        cmp     eax, [ebx+ctx_aes_cbc.cbc.has_data]
        jz      .done
        lea     esi, [ebx+ctx_aes_cbc.cbc.block]
        mov     edi, [_out]
        mov     ecx, CBC128_BLOCK_SIZE
        cmp     [ebx+ctx_aes_cbc.padding], eax
        jz      @f
        sub     cl, [esi+CBC128_BLOCK_SIZE-1]
@@:
        mov     eax, ecx
        rep movsb
.done:
        ret
endp

proc aes256cbc.oneshot _ctx, _key, _iv, _flags, _in, _len, _out
locals
        .done dd ?
endl
        mov     [.done], 0
        stdcall aes256cbc.init, [_ctx], [_key], [_iv], [_flags]
        stdcall aes256cbc.update, [_ctx], [_in], [_len], [_out]
        add     [_out], eax
        add     [.done], eax
        stdcall aes256cbc.finish, [_ctx], [_out]
        add     eax, [.done]
        ret
endp
