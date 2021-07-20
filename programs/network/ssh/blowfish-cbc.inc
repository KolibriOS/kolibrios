;    blowfish-cbc.inc - Blowfish Cipher Block Chaining
;
;    Copyright (C) 2018 Jeffrey Amelynck
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

struct blowfish_cbc_context blowfish_context
        vector  rb BLOWFISH_BLOCKSIZE
ends


proc blowfish_cbc_init _vector
        push    ebx esi edi

        mcall   68, 12, sizeof.blowfish_cbc_context
        ; handle errors
        mov     ecx, BLOWFISH_BLOCKSIZE/4
        mov     esi, [_vector]
        lea     edi, [eax + blowfish_cbc_context.vector]
        rep movsd
        ; rep movsd is slow, but we don't care while init

        pop     edi esi ebx
        ret
endp

proc blowfish_cbc_encrypt _ctx, _in, _out
        push    ebx esi edi

        DEBUGF  1,'plain  : '
        stdcall dump_hex, [_in], 4

        mov     edi, [_ctx]
        lea     edi, [edi + blowfish_cbc_context.vector]
        mov     esi, [_in]
repeat blowfish_BLOCKSIZE/4
        lodsd
        xor     eax, [edi]
        stosd
end repeat

        mov     esi, [_ctx]
        lea     eax, [esi + blowfish_cbc_context.pbox]
        lea     ebx, [esi + blowfish_cbc_context.vector]
        stdcall blowfish_encrypt, eax, ebx, [_out]   ; Key, in, out

        mov     esi, [_out]
        mov     eax, [_ctx]
        lea     edi, [eax + blowfish_cbc_context.vector]
repeat blowfish_BLOCKSIZE/4
        movsd
end repeat

        DEBUGF  1,'cipher : '
        stdcall dump_hex, [_out], 4

        pop     edi esi ebx
        ret
endp

proc blowfish_cbc_decrypt _ctx, _in, _out

locals
        temp_iv rb BLOWFISH_BLOCKSIZE
endl

        push    ebx esi edi

        DEBUGF  1,'cipher : '
        stdcall dump_hex, [_in], 4

        mov     esi, [_in]
        lea     edi, [temp_iv]
repeat BLOWFISH_BLOCKSIZE/4
        movsd
end repeat

        mov     esi, [_ctx]
        lea     eax, [esi + blowfish_cbc_context.pbox]
        stdcall blowfish_decrypt, eax, [_in], [_out]   ; Key, in, out

        mov     esi, [_ctx]
        lea     esi, [esi + blowfish_cbc_context.vector]
        mov     edi, [_out]
repeat BLOWFISH_BLOCKSIZE/4
        lodsd
        xor     eax, [edi]
        stosd
end repeat

        lea     esi, [temp_iv]
        mov     edi, [_ctx]
        lea     edi, [edi + blowfish_cbc_context.vector]
repeat BLOWFISH_BLOCKSIZE/4
        movsd
end repeat

        DEBUGF  1,'plain  : '
        stdcall dump_hex, [_out], 4

        pop     edi esi ebx
        ret
endp