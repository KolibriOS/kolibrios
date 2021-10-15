;    hmac.inc - HMAC: Keyed-Hashing for Message Authentication
;
;    Copyright (C) 2016 Denis Karpenko
;    Copyright (C) 2016 Jeffrey Amelynck
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

; Main concept:
; To compute HMAC over the data `text' we perform
; H(K XOR opad, H(K XOR ipad, text))

SHA2_256_BLOCK_SIZE = 64

struct hmac_sha256_context
        hash            rb SHA2_256_LEN
        ipad_ctx        rb LIBCRASH_CTX_LEN
        opad_ctx        rb LIBCRASH_CTX_LEN
ends

; We will precompute partial hashes of K XOR ipad and K XOR opad,
; and store them in the context structure.

proc hmac_sha256_setkey ctx, key, key_length

locals
        k_temp  rb SHA2_256_BLOCK_SIZE
endl

        pusha

; input esi = key, ecx=key_length
        mov     ecx, [key_length]
        cmp     ecx, SHA2_256_BLOCK_SIZE
        ja      .hash_it
; Key is smaller then or equal to blocksize,
; copy key to ipad
        mov     esi, [key]
        lea     edi, [k_temp]
        rep movsb
        mov     ecx, SHA2_256_BLOCK_SIZE
        sub     ecx, [key_length]
        jz      .finish
; append zeros to the key
        xor     al, al
        rep stosb
        jmp     .finish

; Given key is larger then key size, hash it
  .hash_it:
        invoke  sha2_256_init, [ctx]
        invoke  sha2_256_update, [ctx], [key], [key_length]
        invoke  sha2_256_finish, [ctx]
        mov     esi, [ctx]
        lea     edi, [k_temp]
        mov     ecx, SHA2_256_LEN/4
        rep movsd
        xor     eax, eax
        mov     ecx, (SHA2_256_BLOCK_SIZE-SHA2_256_LEN)/4
        rep stosd

  .finish:
; xor ipad buffer with 0x36363...
        lea     esi, [k_temp]
        mov     ecx, SHA2_256_BLOCK_SIZE/4
  @@:
        xor     dword[esi], 0x36363636          ; ipad constant
        add     esi, 4
        dec     ecx
        jnz     @r

; Init our hash with k_xor_ipad
        mov     ebx, [ctx]
        lea     edi, [ebx+hmac_sha256_context.ipad_ctx]
        invoke  sha2_256_init, edi

        lea     esi, [k_temp]
        DEBUGF  1, "HASH: "
        stdcall dump_hex, esi, SHA2_256_BLOCK_SIZE/4

        mov     ebx, [ctx]
        lea     edi, [ebx+hmac_sha256_context.ipad_ctx]
        invoke  sha2_256_update, edi, esi, SHA2_256_BLOCK_SIZE

; xor opad buffer with 0x5c5c5...
        lea     esi, [k_temp]
        mov     ecx, SHA2_256_BLOCK_SIZE/4
  @@:
        xor     dword[esi], 0x36363636 xor 0x5c5c5c5c   ; opad constant
        add     esi, 4
        dec     ecx
        jnz     @r

; Init our hash with k_xor_opad
        mov     ebx, [ctx]
        lea     edi, [ebx+hmac_sha256_context.opad_ctx]
        invoke  sha2_256_init, edi

        lea     esi, [k_temp]
        DEBUGF  1, "HASH: "
        stdcall dump_hex, esi, SHA2_256_BLOCK_SIZE/4

        mov     ebx, [ctx]
        lea     edi, [ebx+hmac_sha256_context.opad_ctx]
        invoke  sha2_256_update, edi, esi, SHA2_256_BLOCK_SIZE

        popa
        ret

endp

; Copy our pre-computed partial hashes to the stack, complete and finalize them.
; TODO: prevent unnescessary copying of output hash
; TODO: remove unnescessary pushing/popping

proc hmac_sha256 ctx, _data, _length

locals
        inner_ctx        rb LIBCRASH_CTX_LEN
        outer_ctx        rb LIBCRASH_CTX_LEN
endl

        pusha
        DEBUGF  1, "HMAC: "
        mov     ebx, [_length]
        shr     ebx, 2
        stdcall dump_hex, [_data], ebx

; Copy partial hashes of ipad and opad to our temporary buffers
        mov     esi, [ctx]
        lea     esi, [esi+hmac_sha256_context.ipad_ctx]
        lea     edi, [inner_ctx]
repeat (LIBCRASH_CTX_LEN)/4*2
        movsd
end repeat

; Append provided data to inner hash and finalize
        lea     ebx, [inner_ctx]
        invoke  sha2_256_update, ebx, [_data], [_length]
        lea     ebx, [inner_ctx]
        invoke  sha2_256_finish, ebx

        DEBUGF  1, "Inner Hash: "
        lea     esi, [inner_ctx]
        stdcall dump_hex, esi, SHA2_256_LEN/4

; Calculate outer hash
        lea     ebx, [outer_ctx]
        lea     esi, [inner_ctx]
        invoke  sha2_256_update, ebx, esi, SHA2_256_LEN
        lea     ebx, [outer_ctx]
        invoke  sha2_256_finish, ebx
; Copy output hash to ctx structure     ; FIXME
        lea     esi, [outer_ctx]
        mov     edi, [ctx]
repeat SHA2_256_LEN/4
        movsd
end repeat

        popa
        ret

endp
