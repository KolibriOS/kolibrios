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

struct ctx_aes_ctr
        aes ctx_aes
        ctr ctx_ctr
ends

assert sizeof.ctx_aes_ctr <= LIBCRASH_CTX_LEN

; _crypt: 0/1 = encrypt/decrypt
proc aes256ctr.init uses ebx, _ctx, _key, _iv, _flags
        mov     ebx, [_ctx]
        stdcall aes256.init, ebx, [_key], LIBCRASH_CIPHER_ENCRYPT
        add     ebx, ctx_aes_ctr.ctr
        stdcall ctr.init, [_iv]
        ret
endp

proc a22es_ctr._.block_init _ctx
        mov     edi, [_ctx]
        lea     esi, [edi+ctx_aes_ctr.ctr.block_counter]
        mov     ecx, AES_BLOCK_SIZE/4
        rep movsd
        ret
endp

proc aes256ctr.update uses ebx esi edi, _ctx, _in, _len, _out
        mov     eax, [_len]
        pushad
        mov     ebx, [_ctx]
        mov     edi, [_in]
        mov     edx, [ebx+ctx_aes_ctr.ctr.partial_cnt]
.next_chunk:
        mov     ecx, [_len]
        test    ecx, ecx
        jz      .done
        test    edx, edx
        jnz     @f
        pushad
        lea     ecx, [ebx+ctx_aes_ctr.ctr.block_counter]
        lea     edx, [ebx+ctx_aes_ctr.aes.state]
        stdcall aes.encrypt, ebx, ecx, edx
        popad
        mov     edx, AES_BLOCK_SIZE

        pushad
        mov     esi, ebx
        mov     eax, dword[esi+ctx_aes_ctr.ctr.block_counter+4*0]
        mov     ebx, dword[esi+ctx_aes_ctr.ctr.block_counter+4*1]
        mov     ecx, dword[esi+ctx_aes_ctr.ctr.block_counter+4*2]
        mov     edx, dword[esi+ctx_aes_ctr.ctr.block_counter+4*3]

        bswap   eax
        bswap   ebx
        bswap   ecx
        bswap   edx

        add     edx, 1
        adc     ecx, 0
        adc     ebx, 0
        adc     eax, 0

        bswap   eax
        bswap   ebx
        bswap   ecx
        bswap   edx

        mov     dword[esi+ctx_aes_ctr.ctr.block_counter+4*0], eax
        mov     dword[esi+ctx_aes_ctr.ctr.block_counter+4*1], ebx
        mov     dword[esi+ctx_aes_ctr.ctr.block_counter+4*2], ecx
        mov     dword[esi+ctx_aes_ctr.ctr.block_counter+4*3], edx
        popad

@@:
        cmp     ecx, edx
        jbe     @f
        mov     ecx, edx
@@:
        lea     esi, [ebx+ctx_aes_ctr.aes.state]
        add     esi, AES_BLOCK_SIZE
        sub     esi, edx
        sub     [_len], ecx
        sub     edx, ecx
        push    ebx
        mov     edi, [_out]
        mov     ebx, [_in]
        add     [_in], ecx
        add     [_out], ecx
@@:
        lodsb
        xor     al, [ebx]
        inc     ebx
        stosb
        loop    @b
        pop     ebx
        jmp     .next_chunk
.done:
        mov     [ebx+ctx_aes_ctr.ctr.partial_cnt], edx
        popad
        ret
endp

proc aes256ctr.finish _ctx, _out
        xor     eax, eax
        ret
endp

proc aes256ctr.oneshot _ctx, _key, _iv, _flags, _in, _len, _out
locals
        .done dd ?
endl
        mov     [.done], 0
        stdcall aes256ctr.init, [_ctx], [_key], [_iv], [_flags]
        stdcall aes256ctr.update, [_ctx], [_in], [_len], [_out]
        add     [_out], eax
        add     [.done], eax
        stdcall aes256ctr.finish, [_ctx], [_out]
        add     eax, [.done]
        ret
endp
