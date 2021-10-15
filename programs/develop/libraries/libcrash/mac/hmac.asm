; libcrash -- cryptographic hash (and other) functions
;
; Copyright (C) <2021> Ivan Baravy
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

macro max target, [source] {
  common
    target = 0
  forward
    if target < source
      target = source
    end if
}

max MAX_HMAC_HASH_CTX_SIZE, sizeof.ctx_sha2_224256, sizeof.ctx_sha2_384512
max MAX_HMAC_HASH_BLOCK_SIZE, SHA2_224256_BLOCK_SIZE, SHA2_384512_BLOCK_SIZE
max MAX_HMAC_HASH_LEN, SHA2_256_LEN, SHA2_512_LEN

struct ctx_hmac
        ctx_hash        rb MAX_HMAC_HASH_CTX_SIZE
        key_pad         rb MAX_HMAC_HASH_BLOCK_SIZE
        mac             rd MAX_HMAC_HASH_LEN/4
        hash_init       dd ?
        hash_update     dd ?
        hash_finish     dd ?
        hash_oneshot    dd ?
        block_size      dd ?
        hash_size       dd ?
ends

assert sizeof.ctx_hmac <= LIBCRASH_CTX_LEN

; ebx = _ctx
proc hmac._.init uses ebx esi edi, _key, _key_len
        mov     ecx, [_key_len]
        cmp     ecx, [ebx+ctx_hmac.block_size]
        mov     esi, [_key]
        jbe     .pad
        ; hash
        lea     esi, [ebx+ctx_hmac.ctx_hash]
        stdcall [ebx+ctx_hmac.hash_oneshot], esi, [_key], [_key_len]
        mov     ecx, [ebx+ctx_hmac.hash_size]
.pad:
        lea     edi, [ebx+ctx_hmac.key_pad]
        mov     edx, [ebx+ctx_hmac.block_size]
        sub     edx, ecx
        xor     eax, eax
        rep movsb
        mov     ecx, edx
        rep stosb

        ; xor with 0x36
        mov     eax, 0x36363636
        lea     edx, [ebx+ctx_hmac.key_pad]
        mov     ecx, [ebx+ctx_hmac.block_size]
        shr     ecx, 2
@@:
        xor     [edx], eax
        add     edx, 4
        dec     ecx
        jnz     @b

        lea     esi, [ebx+ctx_hmac.ctx_hash]
        stdcall [ebx+ctx_hmac.hash_init], esi
        lea     eax, [ebx+ctx_hmac.key_pad]
        stdcall [ebx+ctx_hmac.hash_update], esi, eax, [ebx+ctx_hmac.block_size]

        ; xor with 0x36 xor 0x5c
        mov     eax, 0x36363636 XOR 0x5c5c5c5c
        lea     edx, [ebx+ctx_hmac.key_pad]
        mov     ecx, [ebx+ctx_hmac.block_size]
        shr     ecx, 2
@@:
        xor     [edx], eax
        add     edx, 4
        dec     ecx
        jnz     @b

        ret
endp

proc hmac_sha2_256.init uses ebx, _ctx, _key, _key_len
        mov     ebx, [_ctx]
        mov     [ebx+ctx_hmac.hash_init], sha2_256.init
        mov     [ebx+ctx_hmac.hash_update], sha2_256.update
        mov     [ebx+ctx_hmac.hash_finish], sha2_256.finish
        mov     [ebx+ctx_hmac.hash_oneshot], sha2_256.oneshot
        mov     [ebx+ctx_hmac.block_size], SHA2_256_BLOCK_SIZE
        mov     [ebx+ctx_hmac.hash_size], SHA2_256_LEN
        stdcall hmac._.init, [_key], [_key_len]
        ret
endp

proc hmac_sha2_512.init uses ebx, _ctx, _key, _key_len
        mov     ebx, [_ctx]
        mov     [ebx+ctx_hmac.hash_init], sha2_512.init
        mov     [ebx+ctx_hmac.hash_update], sha2_512.update
        mov     [ebx+ctx_hmac.hash_finish], sha2_512.finish
        mov     [ebx+ctx_hmac.hash_oneshot], sha2_512.oneshot
        mov     [ebx+ctx_hmac.block_size], SHA2_512_BLOCK_SIZE
        mov     [ebx+ctx_hmac.hash_size], SHA2_512_LEN
        stdcall hmac._.init, [_key], [_key_len]
        ret
endp


hmac_sha2_256.update = hmac._.update
hmac_sha2_512.update = hmac._.update
proc hmac._.update uses ebx esi edi, _ctx, _in, _len
        mov     ebx, [_ctx]
        lea     eax, [ebx+ctx_hmac.ctx_hash]
        stdcall [ebx+ctx_hmac.hash_update], eax, [_in], [_len]
.quit:
        ret
endp

hmac_sha2_256.finish = hmac._.finish
hmac_sha2_512.finish = hmac._.finish
proc hmac._.finish uses ebx esi edi, _ctx
        mov     ebx, [_ctx]
        lea     esi, [ebx+ctx_hmac.ctx_hash]
        stdcall [ebx+ctx_hmac.hash_finish], esi
        lea     edi, [ebx+ctx_hmac.mac]
        mov     ecx, [ebx+ctx_hmac.hash_size]
        rep movsb
        lea     esi, [ebx+ctx_hmac.ctx_hash]
        stdcall [ebx+ctx_hmac.hash_init], esi
        lea     eax, [ebx+ctx_hmac.key_pad]
        stdcall [ebx+ctx_hmac.hash_update], esi, eax, [ebx+ctx_hmac.block_size]
        lea     eax, [ebx+ctx_hmac.mac]
        stdcall [ebx+ctx_hmac.hash_update], esi, eax, [ebx+ctx_hmac.hash_size]
        stdcall [ebx+ctx_hmac.hash_finish], esi
        ret
endp

proc hmac_sha2_256.oneshot _ctx, _in, _len, _key, _key_len
        stdcall hmac_sha2_256.init, [_ctx], [_key], [_key_len]
        stdcall hmac_sha2_256.update, [_ctx], [_in], [_len]
        stdcall hmac_sha2_256.finish, [_ctx]
        ret
endp

proc hmac_sha2_512.oneshot _ctx, _in, _len, _key, _key_len
        stdcall hmac_sha2_512.init, [_ctx], [_key], [_key_len]
        stdcall hmac_sha2_512.update, [_ctx], [_in], [_len]
        stdcall hmac_sha2_512.finish, [_ctx]
        ret
endp
