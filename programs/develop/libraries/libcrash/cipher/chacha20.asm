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

; https://datatracker.ietf.org/doc/html/rfc7539

CHACHA20_BLOCK_SIZE = 64
CHACHA20_KEY_SIZE = 32
CHACHA20_NONCE_SIZE = 12
CHACHA20_IV_SIZE = 16

struct ctx_chacha20
        state rd CHACHA20_BLOCK_SIZE/4
        key rd CHACHA20_KEY_SIZE/4
        block_counter dd ?
        nonce rd CHACHA20_NONCE_SIZE/4
        partial_cnt dd ?
ends

assert sizeof.ctx_chacha20 <= LIBCRASH_CTX_LEN

proc chacha20.init uses ebx esi edi, _ctx, _key, _iv, _flags
        mov     ebx, [_ctx]
        mov     esi, [_key]
        lea     edi, [ebx+ctx_chacha20.key]
        mov     ecx, CHACHA20_KEY_SIZE/4
        rep movsd
        mov     esi, [_iv]
        lea     edi, [ebx+ctx_chacha20.block_counter]
        mov     ecx, CHACHA20_IV_SIZE/4
        rep movsd
        mov     [ebx+ctx_chacha20.partial_cnt], 0
        ret
endp

macro chacha20._.quarter_round a, b, c, d {
        ; a = PLUS(a,b); d = ROTATE(XOR(d,a),16);
        mov     eax, [esi+a*4]
        add     eax, [esi+b*4]
        mov     [esi+a*4], eax
        xor     eax, [esi+d*4]
        rol     eax, 16
        mov     [esi+d*4], eax
        ; c = PLUS(c,d); b = ROTATE(XOR(b,c),12);
        mov     eax, [esi+c*4]
        add     eax, [esi+d*4]
        mov     [esi+c*4], eax
        xor     eax, [esi+b*4]
        rol     eax, 12
        mov     [esi+b*4], eax
        ; a = PLUS(a,b); d = ROTATE(XOR(d,a), 8);
        mov     eax, [esi+a*4]
        add     eax, [esi+b*4]
        mov     [esi+a*4], eax
        xor     eax, [esi+d*4]
        rol     eax, 8
        mov     [esi+d*4], eax
        ; c = PLUS(c,d); b = ROTATE(XOR(b,c), 7);
        mov     eax, [esi+c*4]
        add     eax, [esi+d*4]
        mov     [esi+c*4], eax
        xor     eax, [esi+b*4]
        rol     eax, 7
        mov     [esi+b*4], eax
}

proc chacha20._.inner_block _state
        mov     esi, [_state]
        chacha20._.quarter_round 0, 4,  8, 12
        chacha20._.quarter_round 1, 5,  9, 13
        chacha20._.quarter_round 2, 6, 10, 14
        chacha20._.quarter_round 3, 7, 11, 15
        chacha20._.quarter_round 0, 5, 10, 15
        chacha20._.quarter_round 1, 6, 11, 12
        chacha20._.quarter_round 2, 7,  8, 13
        chacha20._.quarter_round 3, 4,  9, 14
        ret
endp

proc chacha20._.block_init _ctx
        mov     edi, [_ctx]
        lea     esi, [edi+ctx_chacha20.key]
        mov     [edi+ctx_chacha20.state+0*4], 'expa'    ; magic
        mov     [edi+ctx_chacha20.state+1*4], 'nd 3'    ; constants
        mov     [edi+ctx_chacha20.state+2*4], '2-by'    ; from
        mov     [edi+ctx_chacha20.state+3*4], 'te k'    ; the RFC
        add     edi, 4*4
        mov     ecx, CHACHA20_BLOCK_SIZE/4-4    ; the four dwords above
        rep movsd
        ret
endp


proc chacha20._.block _state
locals
        .working_state rd CHACHA20_BLOCK_SIZE/4
        .i dd ?
endl
        stdcall chacha20._.block_init, [_state]

        mov     esi, [_state]
        lea     edi, [.working_state]
        mov     ecx, CHACHA20_BLOCK_SIZE/4
        rep movsd

        mov     [.i], 10
@@:
        lea     eax, [.working_state]
        stdcall chacha20._.inner_block, eax
        dec     [.i]
        jnz     @b

        lea     esi, [.working_state]
        mov     edi, [_state]
        mov     ecx, CHACHA20_BLOCK_SIZE/4-1
@@:
        mov     eax, [esi+ecx*4]
        add     [edi+ecx*4], eax
        dec     ecx
        jns     @b

        ret
endp

proc chacha20.update uses ebx esi edi, _ctx, _in, _len, _out
locals
        .bytes_done dd ?
endl
        mov     eax, [_len]
        mov     [.bytes_done], eax
        mov     ebx, [_ctx]
        mov     edx, [ebx+ctx_chacha20.partial_cnt]
.next_chunk:
        mov     ecx, [_len]
        test    ecx, ecx
        jz      .done
        test    edx, edx
        jnz     @f
        pushad
        stdcall chacha20._.block, [_ctx]
        popad
        mov     edx, CHACHA20_BLOCK_SIZE
        inc     [ebx+ctx_chacha20.block_counter]
@@:
        cmp     ecx, edx
        jbe     @f
        mov     ecx, edx
@@:
        lea     esi, [ebx+ctx_chacha20.state]
        add     esi, CHACHA20_BLOCK_SIZE
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
        mov     [ebx+ctx_chacha20.partial_cnt], edx
        mov     eax, [.bytes_done]
        ret
endp

proc chacha20.finish _ctx, _out
        xor     eax, eax
        ret
endp

proc chacha20.oneshot _ctx, _key, _iv, _flags, _in, _len, _out
locals
        .done dd ?
endl
        mov     [.done], 0
        stdcall chacha20.init, [_ctx], [_key], [_iv], [_flags]
        stdcall chacha20.update, [_ctx], [_in], [_len], [_out]
        add     [_out], eax
        add     [.done], eax
        stdcall chacha20.finish, [_ctx], [_out]
        add     eax, [.done]
        ret
endp
