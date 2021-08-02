;    blowfish-ctr.inc - Blowfish Counter Mode
;
;    Copyright (C) 2016 Ivan Baravy (dunkaist)
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

struct blowfish_ctr_context blowfish_context
        counter rb BLOWFISH_BLOCKSIZE
        output  rb BLOWFISH_BLOCKSIZE     ; counter after blowfish_crypt
ends


proc blowfish_ctr_init _counter
        push    ebx esi edi

        mcall   68, 12, sizeof.blowfish_ctr_context
        ; handle errors
        mov     ecx, BLOWFISH_BLOCKSIZE/4
        mov     esi, [_counter]
        lea     edi, [eax + blowfish_ctr_context.counter]
        rep movsd
        ; rep movsd is slow, but we don't care while init

        pop     edi esi ebx
        ret
endp


proc blowfish_ctr_crypt _ctx, _in, _out

        push    ebx esi edi

        DEBUGF  1,'plain  : '
        stdcall dump_hex, [_in], 4

        mov     esi, [_ctx]
        lea     eax, [esi + blowfish_ctr_context.key]
        lea     ebx, [esi + blowfish_ctr_context.counter]
        lea     ecx, [esi + blowfish_ctr_context.output]

        stdcall blowfish_encrypt, eax, ebx, ecx   ; Key, in, out

        mov     ebx, [_ctx]
        mov     esi, [_in]
        mov     edi, [_out]

        mov     eax, [esi + 4*0]
        xor     eax, dword[ebx + blowfish_ctr_context.output + 4*0]
        mov     [edi + 4*0], eax

        mov     eax, [esi + 4*1]
        xor     eax, dword[ebx + blowfish_ctr_context.output + 4*1]
        mov     [edi + 4*1], eax

        mov     eax, [esi + 4*2]
        xor     eax, dword[ebx + blowfish_ctr_context.output + 4*2]
        mov     [edi + 4*2], eax

        mov     eax, [esi + 4*3]
        xor     eax, dword[ebx + blowfish_ctr_context.output + 4*3]
        mov     [edi + 4*3], eax

; Increment counter
        mov     esi, [_ctx]

        mov     eax, dword[esi + blowfish_ctr_context.counter + 4*0]
        mov     ebx, dword[esi + blowfish_ctr_context.counter + 4*1]
        mov     ecx, dword[esi + blowfish_ctr_context.counter + 4*2]
        mov     edx, dword[esi + blowfish_ctr_context.counter + 4*3]

        bswap   eax
        bswap   ebx
        bswap   ecx
        bswap   edx

        adc     edx, 1
        adc     ecx, 0
        adc     ebx, 0
        adc     eax, 0

        bswap   eax
        bswap   ebx
        bswap   ecx
        bswap   edx

        mov     dword[esi + blowfish_ctr_context.counter + 4*0], eax
        mov     dword[esi + blowfish_ctr_context.counter + 4*1], ebx
        mov     dword[esi + blowfish_ctr_context.counter + 4*2], ecx
        mov     dword[esi + blowfish_ctr_context.counter + 4*3], edx

        DEBUGF  1,'cipher : '
        stdcall dump_hex, [_out], 4

        pop     edi esi ebx
        ret
endp