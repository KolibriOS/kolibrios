;    aes256-cbc.inc - AES256 Cipher Block Chaining
;
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

struct aes256_cbc_context aes256_context
        vector  rb AES256_BLOCKSIZE
ends


proc aes256_cbc_init _vector
        push    ebx esi edi

        mcall   68, 12, sizeof.aes256_cbc_context
        ; handle errors
        mov     ecx, AES256_BLOCKSIZE/4
        mov     esi, [_vector]
        lea     edi, [eax + aes256_cbc_context.vector]
        rep movsd
        ; rep movsd is slow, but we don't care while init

        pop     edi esi ebx
        ret
endp

proc aes256_cbc_encrypt _ctx, _in, _out
        push    ebx esi edi

        DEBUGF  1,'plain  : '
        stdcall dump_hex, [_in], 4

        mov     edi, [_ctx]
        lea     edi, [edi + aes256_cbc_context.vector]
        mov     esi, [_in]
repeat AES256_BLOCKSIZE/4
        lodsd
        xor     eax, [edi]
        stosd
end repeat

        mov     esi, [_ctx]
        lea     eax, [esi + aes256_cbc_context.key]
        lea     ebx, [esi + aes256_cbc_context.vector]
        stdcall aes256_encrypt, eax, ebx, [_out]   ; Key, in, out

        mov     esi, [_out]
        mov     eax, [_ctx]
        lea     edi, [eax + aes256_cbc_context.vector]
repeat AES256_BLOCKSIZE/4
        movsd
end repeat

        DEBUGF  1,'cipher : '
        stdcall dump_hex, [_out], 4

        pop     edi esi ebx
        ret
endp

proc aes256_cbc_decrypt _ctx, _in, _out

locals
        temp_iv rb AES256_BLOCKSIZE
endl

        push    ebx esi edi

        DEBUGF  1,'cipher : '
        stdcall dump_hex, [_in], 4

        mov     esi, [_in]
        lea     edi, [temp_iv]
repeat AES256_BLOCKSIZE/4
        movsd
end repeat

        mov     esi, [_ctx]
        lea     eax, [esi + aes256_cbc_context.key]
        stdcall aes256_decrypt, eax, [_in], [_out]   ; Key, in, out

        mov     esi, [_ctx]
        lea     esi, [esi + aes256_cbc_context.vector]
        mov     edi, [_out]
repeat AES256_BLOCKSIZE/4
        lodsd
        xor     eax, [edi]
        stosd
end repeat

        lea     esi, [temp_iv]
        mov     edi, [_ctx]
        lea     edi, [edi + aes256_cbc_context.vector]
repeat AES256_BLOCKSIZE/4
        movsd
end repeat

        DEBUGF  1,'plain  : '
        stdcall dump_hex, [_out], 4

        pop     edi esi ebx
        ret
endp