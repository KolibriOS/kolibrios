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

; Based on rfc7539 and implementation of libressl

POLY1305_BLOCK_SIZE = 16

POLY1305_ALIGN = 16
POLY1305_ALIGN_MASK = POLY1305_ALIGN-1

struct ctx_poly1305
        mac             rd 5
                        rd 3
        block           rb POLY1305_BLOCK_SIZE
                        rb POLY1305_ALIGN-(POLY1305_BLOCK_SIZE mod POLY1305_ALIGN)
        index           dd ?
        block_size      dd ?
        hibit           dd ?
                        rd 2    ; align
        ; tmp vars
        r               rd 5
        s               rd 4
        d               rd 5*2  ; 5 dq
ends

assert sizeof.ctx_poly1305 <= LIBCRASH_CTX_LEN

proc poly1305.init uses ebx, _ctx, _key, _key_len
        mov     ebx, [_ctx]
        mov     [ebx+ctx_poly1305.block_size], POLY1305_BLOCK_SIZE
        mov     [ebx+ctx_poly1305.hibit], 1 SHL 24
        ; accumulator
        mov     [ebx+ctx_poly1305.mac+0*4], 0
        mov     [ebx+ctx_poly1305.mac+1*4], 0
        mov     [ebx+ctx_poly1305.mac+2*4], 0
        mov     [ebx+ctx_poly1305.mac+3*4], 0
        mov     [ebx+ctx_poly1305.mac+4*4], 0
        ; r &= 0xffffffc0ffffffc0ffffffc0fffffff
        mov     ecx, [_key]
        mov     eax, [ecx+0]
        and     eax, 0x3ffffff
        mov     [ebx+ctx_poly1305.r+0*4], eax
        mov     eax, [ecx+3]
        shr     eax, 2
        and     eax, 0x3ffff03
        mov     [ebx+ctx_poly1305.r+1*4], eax
        mov     eax, [ecx+6]
        shr     eax, 4
        and     eax, 0x3ffc0ff
        mov     [ebx+ctx_poly1305.r+2*4], eax
        mov     eax, [ecx+9]
        shr     eax, 6
        and     eax, 0x3f03fff
        mov     [ebx+ctx_poly1305.r+3*4], eax
        mov     eax, [ecx+12]
        shr     eax, 8
        and     eax, 0xfffff
        mov     [ebx+ctx_poly1305.r+4*4], eax
        ; s
        mov     eax, [ecx+4*4]
        mov     [ebx+ctx_poly1305.s+0*4], eax
        mov     eax, [ecx+5*4]
        mov     [ebx+ctx_poly1305.s+1*4], eax
        mov     eax, [ecx+6*4]
        mov     [ebx+ctx_poly1305.s+2*4], eax
        mov     eax, [ecx+7*4]
        mov     [ebx+ctx_poly1305.s+3*4], eax
        ret
endp


proc poly1305._.block _mac
;        mov     ecx, [ebx+ctx_poly1305.rounds_cnt]
        mov     edi, [_mac]
        ; a += m[i]
        mov     eax, [esi+0]
        and     eax, 0x3ffffff
        add     [ebx+ctx_poly1305.mac+0*4], eax
        mov     eax, [esi+3]
        shr     eax, 2
        and     eax, 0x3ffffff
        add     [ebx+ctx_poly1305.mac+1*4], eax
        mov     eax, [esi+6]
        shr     eax, 4
        and     eax, 0x3ffffff
        add     [ebx+ctx_poly1305.mac+2*4], eax
        mov     eax, [esi+9]
        shr     eax, 6
        and     eax, 0x3ffffff
        add     [ebx+ctx_poly1305.mac+3*4], eax
        mov     eax, [esi+12]
        shr     eax, 8
        or      eax, [ebx+ctx_poly1305.hibit]
        add     [ebx+ctx_poly1305.mac+4*4], eax

        ; a *= r
        ; d0
        ; r0*a0
        mov     eax, [ebx+ctx_poly1305.r+0*4]
        mul     [ebx+ctx_poly1305.mac+0*4]
        mov     ecx, eax
        mov     edi, edx
        ; s4*a1
        mov     eax, [ebx+ctx_poly1305.r+4*4]
        lea     eax, [eax*5]
        mul     [ebx+ctx_poly1305.mac+1*4]
        add     ecx, eax
        adc     edi, edx
        ; s3*a2
        mov     eax, [ebx+ctx_poly1305.r+3*4]
        lea     eax, [eax*5]
        mul     [ebx+ctx_poly1305.mac+2*4]
        add     ecx, eax
        adc     edi, edx
        ; s2*a3
        mov     eax, [ebx+ctx_poly1305.r+2*4]
        lea     eax, [eax*5]
        mul     [ebx+ctx_poly1305.mac+3*4]
        add     ecx, eax
        adc     edi, edx
        ; s1*a4
        mov     eax, [ebx+ctx_poly1305.r+1*4]
        lea     eax, [eax*5]
        mul     [ebx+ctx_poly1305.mac+4*4]
        add     ecx, eax
        adc     edi, edx
        mov     [ebx+ctx_poly1305.d+0*8+0], ecx
        mov     [ebx+ctx_poly1305.d+0*8+4], edi
        ; d1
        ; r1*a0
        mov     eax, [ebx+ctx_poly1305.r+1*4]
        mul     [ebx+ctx_poly1305.mac+0*4]
        mov     ecx, eax
        mov     edi, edx
        ; r0*a1
        mov     eax, [ebx+ctx_poly1305.r+0*4]
        mul     [ebx+ctx_poly1305.mac+1*4]
        add     ecx, eax
        adc     edi, edx
        ; s4*a2
        mov     eax, [ebx+ctx_poly1305.r+4*4]
        lea     eax, [eax*5]
        mul     [ebx+ctx_poly1305.mac+2*4]
        add     ecx, eax
        adc     edi, edx
        ; s3*a3
        mov     eax, [ebx+ctx_poly1305.r+3*4]
        lea     eax, [eax*5]
        mul     [ebx+ctx_poly1305.mac+3*4]
        add     ecx, eax
        adc     edi, edx
        ; s2*a4
        mov     eax, [ebx+ctx_poly1305.r+2*4]
        lea     eax, [eax*5]
        mul     [ebx+ctx_poly1305.mac+4*4]
        add     ecx, eax
        adc     edi, edx
        mov     [ebx+ctx_poly1305.d+1*8+0], ecx
        mov     [ebx+ctx_poly1305.d+1*8+4], edi
        ; d2
        ; r2*a0
        mov     eax, [ebx+ctx_poly1305.r+2*4]
        mul     [ebx+ctx_poly1305.mac+0*4]
        mov     ecx, eax
        mov     edi, edx
        ; r1*a1
        mov     eax, [ebx+ctx_poly1305.r+1*4]
        mul     [ebx+ctx_poly1305.mac+1*4]
        add     ecx, eax
        adc     edi, edx
        ; r0*a2
        mov     eax, [ebx+ctx_poly1305.r+0*4]
        mul     [ebx+ctx_poly1305.mac+2*4]
        add     ecx, eax
        adc     edi, edx
        ; s4*a3
        mov     eax, [ebx+ctx_poly1305.r+4*4]
        lea     eax, [eax*5]
        mul     [ebx+ctx_poly1305.mac+3*4]
        add     ecx, eax
        adc     edi, edx
        ; s3*a4
        mov     eax, [ebx+ctx_poly1305.r+3*4]
        lea     eax, [eax*5]
        mul     [ebx+ctx_poly1305.mac+4*4]
        add     ecx, eax
        adc     edi, edx
        mov     [ebx+ctx_poly1305.d+2*8+0], ecx
        mov     [ebx+ctx_poly1305.d+2*8+4], edi
        ; d3
        ; r3*a0
        mov     eax, [ebx+ctx_poly1305.r+3*4]
        mul     [ebx+ctx_poly1305.mac+0*4]
        mov     ecx, eax
        mov     edi, edx
        ; r2*a1
        mov     eax, [ebx+ctx_poly1305.r+2*4]
        mul     [ebx+ctx_poly1305.mac+1*4]
        add     ecx, eax
        adc     edi, edx
        ; r1*a2
        mov     eax, [ebx+ctx_poly1305.r+1*4]
        mul     [ebx+ctx_poly1305.mac+2*4]
        add     ecx, eax
        adc     edi, edx
        ; r0*a3
        mov     eax, [ebx+ctx_poly1305.r+0*4]
        mul     [ebx+ctx_poly1305.mac+3*4]
        add     ecx, eax
        adc     edi, edx
        ; s4*a4
        mov     eax, [ebx+ctx_poly1305.r+4*4]
        lea     eax, [eax*5]
        mul     [ebx+ctx_poly1305.mac+4*4]
        add     ecx, eax
        adc     edi, edx
        mov     [ebx+ctx_poly1305.d+3*8+0], ecx
        mov     [ebx+ctx_poly1305.d+3*8+4], edi
        ; d4
        ; r4*a0
        mov     eax, [ebx+ctx_poly1305.r+4*4]
        mul     [ebx+ctx_poly1305.mac+0*4]
        mov     ecx, eax
        mov     edi, edx
        ; r3*a1
        mov     eax, [ebx+ctx_poly1305.r+3*4]
        mul     [ebx+ctx_poly1305.mac+1*4]
        add     ecx, eax
        adc     edi, edx
        ; r2*a2
        mov     eax, [ebx+ctx_poly1305.r+2*4]
        mul     [ebx+ctx_poly1305.mac+2*4]
        add     ecx, eax
        adc     edi, edx
        ; r1*a3
        mov     eax, [ebx+ctx_poly1305.r+1*4]
        mul     [ebx+ctx_poly1305.mac+3*4]
        add     ecx, eax
        adc     edi, edx
        ; r0*a4
        mov     eax, [ebx+ctx_poly1305.r+0*4]
        mul     [ebx+ctx_poly1305.mac+4*4]
        add     ecx, eax
        adc     edi, edx
        mov     [ebx+ctx_poly1305.d+4*8+0], ecx
        mov     [ebx+ctx_poly1305.d+4*8+4], edi

        ; (partial) a %= p
        mov     eax, [ebx+ctx_poly1305.d+0*8+0]
        mov     edx, [ebx+ctx_poly1305.d+0*8+4]
        ; d0
        mov     ecx, edx
        shld    ecx, eax, 6
        and     eax, 0x3ffffff
        mov     [ebx+ctx_poly1305.mac+0*4], eax
        mov     eax, [ebx+ctx_poly1305.d+1*8+0]
        mov     edx, [ebx+ctx_poly1305.d+1*8+4]
        add     eax, ecx
        adc     edx, 0
        ; d1
        mov     ecx, edx
        shld    ecx, eax, 6
        and     eax, 0x3ffffff
        mov     [ebx+ctx_poly1305.mac+1*4], eax
        mov     eax, [ebx+ctx_poly1305.d+2*8+0]
        mov     edx, [ebx+ctx_poly1305.d+2*8+4]
        add     eax, ecx
        adc     edx, 0
        ; d2
        mov     ecx, edx
        shld    ecx, eax, 6
        and     eax, 0x3ffffff
        mov     [ebx+ctx_poly1305.mac+2*4], eax
        mov     eax, [ebx+ctx_poly1305.d+3*8+0]
        mov     edx, [ebx+ctx_poly1305.d+3*8+4]
        add     eax, ecx
        adc     edx, 0
        ; d3
        mov     ecx, edx
        shld    ecx, eax, 6
        and     eax, 0x3ffffff
        mov     [ebx+ctx_poly1305.mac+3*4], eax
        mov     eax, [ebx+ctx_poly1305.d+4*8+0]
        mov     edx, [ebx+ctx_poly1305.d+4*8+4]
        add     eax, ecx
        adc     edx, 0
        ; d4
        mov     ecx, edx
        shld    ecx, eax, 6
        and     eax, 0x3ffffff
        mov     [ebx+ctx_poly1305.mac+4*4], eax
        lea     ecx, [ecx*5]
        add     ecx, [ebx+ctx_poly1305.mac+0*4]
        mov     eax, ecx
        shr     ecx, 26
        and     eax, 0x3ffffff
        mov     [ebx+ctx_poly1305.mac+0*4], eax
        add     [ebx+ctx_poly1305.mac+1*4], ecx
        ret
endp



proc poly1305.update uses ebx esi edi, _ctx, _msg, _size
.next_block:
        mov     ebx, [_ctx]
        mov     esi, [_msg]
        mov     eax, [ebx+ctx_poly1305.index]
        test    eax, eax
        jnz     .copy_to_buf
        test    esi, POLY1305_ALIGN_MASK
        jnz     .copy_to_buf
.no_copy:
        ; data is aligned, process it in place without copying
        mov     ebx, [_ctx]
        mov     eax, [ebx+ctx_poly1305.block_size]
        cmp     [_size], eax
        jb      .copy_quit
        lea     eax, [ebx+ctx_poly1305.mac]
        push    ebx esi
        stdcall poly1305._.block, eax
        pop     esi ebx
        mov     eax, [ebx+ctx_poly1305.block_size]
        sub     [_size], eax
        add     esi, [ebx+ctx_poly1305.block_size]
        jmp     .no_copy

.copy_to_buf:
        lea     edi, [ebx+ctx_poly1305.block]
        add     edi, eax
        mov     ecx, [ebx+ctx_poly1305.block_size]
        sub     ecx, eax
        cmp     [_size], ecx
        jb      .copy_quit
        sub     [_size], ecx
        add     [_msg], ecx
        add     [ebx+ctx_poly1305.index], ecx
        mov     eax, [ebx+ctx_poly1305.block_size]
        cmp     [ebx+ctx_poly1305.index], eax
        jb      @f
        sub     [ebx+ctx_poly1305.index], eax
@@:
        rep movsb
        lea     eax, [ebx+ctx_poly1305.mac]
        lea     esi, [ebx+ctx_poly1305.block]
        stdcall poly1305._.block, eax
        jmp     .next_block

.copy_quit:
        mov     ebx, [_ctx]
        lea     edi, [ebx+ctx_poly1305.block]
        mov     eax, [ebx+ctx_poly1305.index]
        add     edi, eax
        mov     ecx, [_size]
        add     [ebx+ctx_poly1305.index], ecx
        rep movsb
.quit:
        ret
endp

proc poly1305.finish uses ebx esi edi, _ctx
        mov     ebx, [_ctx]
        mov     eax, [ebx+ctx_poly1305.index]
        test    eax, eax
        jz      .skip
        mov     ecx, [ebx+ctx_poly1305.block_size]
        sub     ecx, eax
        lea     edi, [ebx+ctx_poly1305.block]
        add     edi, eax
        mov     byte[edi], 0x01
        inc     edi
        dec     ecx
        xor     eax, eax
        rep stosb

        mov     ebx, [_ctx]
        mov     [ebx+ctx_poly1305.hibit], 0
        lea     esi, [ebx+ctx_poly1305.block]
        lea     eax, [ebx+ctx_poly1305.mac]
        stdcall poly1305._.block, eax
.skip:
        mov     ebx, [_ctx]
        lea     eax, [ebx+ctx_poly1305.mac]
        stdcall poly1305._.postprocess, ebx, eax

        ; fully carry a
        mov     ecx, [ebx+ctx_poly1305.mac+1*4]
        shr     ecx, 26
        and     [ebx+ctx_poly1305.mac+1*4], 0x3ffffff
        add     ecx, [ebx+ctx_poly1305.mac+2*4]
        mov     eax, ecx
        and     eax, 0x3ffffff
        shr     ecx, 26
        mov     [ebx+ctx_poly1305.mac+2*4], eax
        add     ecx, [ebx+ctx_poly1305.mac+3*4]
        mov     eax, ecx
        and     eax, 0x3ffffff
        shr     ecx, 26
        mov     [ebx+ctx_poly1305.mac+3*4], eax
        add     ecx, [ebx+ctx_poly1305.mac+4*4]
        mov     eax, ecx
        and     eax, 0x3ffffff
        shr     ecx, 26
        mov     [ebx+ctx_poly1305.mac+4*4], eax
        lea     ecx, [ecx*5]
        add     ecx, [ebx+ctx_poly1305.mac+0*4]
        mov     eax, ecx
        and     eax, 0x3ffffff
        shr     ecx, 26
        mov     [ebx+ctx_poly1305.mac+0*4], eax
        add     [ebx+ctx_poly1305.mac+1*4], ecx

        ; compute a + -p
        mov     ecx, [ebx+ctx_poly1305.mac+0*4]
        add     ecx, 5
        mov     eax, ecx
        and     eax, 0x3ffffff
        shr     ecx, 26
        mov     [ebx+ctx_poly1305.r+0*4], eax
        add     ecx, [ebx+ctx_poly1305.mac+1*4]
        mov     eax, ecx
        and     eax, 0x3ffffff
        shr     ecx, 26
        mov     [ebx+ctx_poly1305.r+1*4], eax
        add     ecx, [ebx+ctx_poly1305.mac+2*4]
        mov     eax, ecx
        and     eax, 0x3ffffff
        shr     ecx, 26
        mov     [ebx+ctx_poly1305.r+2*4], eax
        add     ecx, [ebx+ctx_poly1305.mac+3*4]
        mov     eax, ecx
        and     eax, 0x3ffffff
        shr     ecx, 26
        mov     [ebx+ctx_poly1305.r+3*4], eax
        add     ecx, [ebx+ctx_poly1305.mac+4*4]
        sub     ecx, 1 SHL 26
        mov     [ebx+ctx_poly1305.r+4*4], ecx

        ; select a if a < p, or a + -p if a >= p
        shr     ecx, 31
        dec     ecx
        and     [ebx+ctx_poly1305.r+0*4], ecx
        and     [ebx+ctx_poly1305.r+1*4], ecx
        and     [ebx+ctx_poly1305.r+2*4], ecx
        and     [ebx+ctx_poly1305.r+3*4], ecx
        and     [ebx+ctx_poly1305.r+4*4], ecx
        not     ecx
        mov     eax, [ebx+ctx_poly1305.r+0*4]
        and     [ebx+ctx_poly1305.mac+0*4], ecx
        or      [ebx+ctx_poly1305.mac+0*4], eax
        mov     eax, [ebx+ctx_poly1305.r+1*4]
        and     [ebx+ctx_poly1305.mac+1*4], ecx
        or      [ebx+ctx_poly1305.mac+1*4], eax
        mov     eax, [ebx+ctx_poly1305.r+2*4]
        and     [ebx+ctx_poly1305.mac+2*4], ecx
        or      [ebx+ctx_poly1305.mac+2*4], eax
        mov     eax, [ebx+ctx_poly1305.r+3*4]
        and     [ebx+ctx_poly1305.mac+3*4], ecx
        or      [ebx+ctx_poly1305.mac+3*4], eax
        mov     eax, [ebx+ctx_poly1305.r+4*4]
        and     [ebx+ctx_poly1305.mac+4*4], ecx
        or      [ebx+ctx_poly1305.mac+4*4], eax

        ; a = a % (2^128)
        ; a0
        mov     eax, [ebx+ctx_poly1305.mac+0*4]
        mov     ecx, [ebx+ctx_poly1305.mac+1*4]
        shl     ecx, 26
        or      eax, ecx
        mov     [ebx+ctx_poly1305.mac+0*4], eax
        ; a1
        mov     eax, [ebx+ctx_poly1305.mac+1*4]
        shr     eax, 6
        mov     ecx, [ebx+ctx_poly1305.mac+2*4]
        shl     ecx, 20
        or      eax, ecx
        mov     [ebx+ctx_poly1305.mac+1*4], eax
        ; a2
        mov     eax, [ebx+ctx_poly1305.mac+2*4]
        shr     eax, 12
        mov     ecx, [ebx+ctx_poly1305.mac+3*4]
        shl     ecx, 14
        or      eax, ecx
        mov     [ebx+ctx_poly1305.mac+2*4], eax
        ; a3
        mov     eax, [ebx+ctx_poly1305.mac+3*4]
        shr     eax, 18
        mov     ecx, [ebx+ctx_poly1305.mac+4*4]
        shl     ecx, 8
        or      eax, ecx
        mov     [ebx+ctx_poly1305.mac+3*4], eax

        ; mac = (a + pad) % (2^128)
        xor     edx, edx
        ; a0
        mov     eax, [ebx+ctx_poly1305.mac+0*4]
        add     eax, [ebx+ctx_poly1305.s+0*4]
        mov     [ebx+ctx_poly1305.mac+0*4], eax
        ; a1
        mov     eax, [ebx+ctx_poly1305.mac+1*4]
        adc     eax, [ebx+ctx_poly1305.s+1*4]
        mov     [ebx+ctx_poly1305.mac+1*4], eax
        ; a2
        mov     eax, [ebx+ctx_poly1305.mac+2*4]
        adc     eax, [ebx+ctx_poly1305.s+2*4]
        mov     [ebx+ctx_poly1305.mac+2*4], eax
        ; a3
        mov     eax, [ebx+ctx_poly1305.mac+3*4]
        adc     eax, [ebx+ctx_poly1305.s+3*4]
        mov     [ebx+ctx_poly1305.mac+3*4], eax
        ret
endp

proc poly1305._.postprocess _ctx, _mac
        ret
endp

proc poly1305.oneshot _ctx, _in, _len, _key, _key_len
        stdcall poly1305.init, [_ctx], [_key], [_key_len]
        stdcall poly1305.update, [_ctx], [_in], [_len]
        stdcall poly1305.finish, [_ctx]
        ret
endp
