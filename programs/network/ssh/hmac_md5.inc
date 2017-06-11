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

struct hmac_md5_context
        hash            rb MD5_HASH_SIZE
        ipad_ctx        ctx_md5
        opad_ctx        ctx_md5
ends

; We will precompute partial hashes of K XOR ipad and K XOR opad,
; and store them in the context structure.

proc hmac_md5_setkey ctx, key, key_length

locals
        k_temp  rb MD5_BLOCK_SIZE
endl

        pusha

; input esi = key, ecx=key_length
        mov     ecx, [key_length]
        cmp     ecx, MD5_BLOCK_SIZE
        ja      .hash_it
; Key is smaller then or equal to blocksize,
; copy key to ipad
        mov     esi, [key]
        lea     edi, [k_temp]
        rep movsb
        mov     ecx, MD5_BLOCK_SIZE
        sub     ecx, [key_length]
        jz      .finish
; append zeros to the key
        xor     al, al
        rep stosb
        jmp     .finish

; Given key is larger then key size, hash it
  .hash_it:
        invoke  md5_init, [ctx]
        invoke  md5_update, [ctx], [key], [key_length]
        invoke  md5_final, [ctx]
        mov     esi, [ctx]
        lea     edi, [k_temp]
        mov     ecx, MD5_HASH_SIZE/4
        rep movsd
        xor     eax, eax
        mov     ecx, (MD5_BLOCK_SIZE-MD5_HASH_SIZE)/4
        rep stosd

  .finish:
; xor ipad buffer with 0x36363...
        lea     esi, [k_temp]
        mov     ecx, MD5_BLOCK_SIZE/4
  @@:
        xor     dword[esi], 0x36363636          ; ipad constant
        add     esi, 4
        dec     ecx
        jnz     @r

; Init our hash with k_xor_ipad
        mov     ebx, [ctx]
        lea     edi, [ebx+hmac_md5_context.ipad_ctx]
        invoke  md5_init, edi

        lea     esi, [k_temp]
        DEBUGF  1, "HASH: "
        stdcall dump_hex, esi, MD5_BLOCK_SIZE/4

        mov     ebx, [ctx]
        lea     edi, [ebx+hmac_md5_context.ipad_ctx]
        invoke  md5_update, edi, esi, MD5_BLOCK_SIZE

; xor opad buffer with 0x5c5c5...
        lea     esi, [k_temp]
        mov     ecx, MD5_BLOCK_SIZE/4
  @@:
        xor     dword[esi], 0x36363636 xor 0x5c5c5c5c   ; opad constant
        add     esi, 4
        dec     ecx
        jnz     @r

; Init our hash with k_xor_opad
        mov     ebx, [ctx]
        lea     edi, [ebx+hmac_md5_context.opad_ctx]
        invoke  md5_init, edi

        lea     esi, [k_temp]
        DEBUGF  1, "HASH: "
        stdcall dump_hex, esi, MD5_BLOCK_SIZE/4

        mov     ebx, [ctx]
        lea     edi, [ebx+hmac_md5_context.opad_ctx]
        invoke  md5_update, edi, esi, MD5_BLOCK_SIZE

        popa
        ret

endp

; Copy our pre-computed partial hashes to the stack, complete and finalize them.
; TODO: prevent unnescessary copying of output hash
; TODO: remove unnescessary pushing/popping

proc hmac_md5 ctx, _data, _length

locals
        inner_ctx        ctx_md5
        outer_ctx        ctx_md5
endl

        pusha
        DEBUGF  1, "HMAC: "
        mov     ebx, [_length]
        shr     ebx, 2
        stdcall dump_hex, [_data], ebx

; Copy partial hashes of ipad and opad to our temporary buffers
        mov     esi, [ctx]
        lea     esi, [esi+hmac_md5_context.ipad_ctx]
        lea     edi, [inner_ctx]
repeat (sizeof.ctx_md5)/4*2
        movsd
end repeat

; Append provided data to inner hash and finalize
        lea     ebx, [inner_ctx]
        invoke  md5_update, ebx, [_data], [_length]
        lea     ebx, [inner_ctx]
        invoke  md5_final, ebx

        DEBUGF  1, "Inner Hash: "
        lea     esi, [inner_ctx.hash]
        stdcall dump_hex, esi, MD5_HASH_SIZE/4

; Calculate outer hash
        lea     ebx, [outer_ctx]
        lea     esi, [inner_ctx.hash]
        invoke  md5_update, ebx, esi, MD5_HASH_SIZE
        lea     ebx, [outer_ctx]
        invoke  md5_final, ebx
; Copy output hash to ctx structure     ; FIXME
        lea     esi, [outer_ctx.hash]
        mov     edi, [ctx]
repeat MD5_HASH_SIZE/4
        movsd
end repeat

        popa
        ret

endp
