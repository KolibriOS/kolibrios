;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2017. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; This crc32 routine doesn't use precomputed table to allow different
; polynomials, which is the first param.
; Partial hash in assumed to be eax (both in and out).
; Usage:
; 1. mov eax, -1
; 2. stdcall crypto.crc32 zero or more times
; 3. xor eax, -1
proc crc_32 _poly, _buffer, _length
        push    ebx ecx edx esi

        mov     esi, [_buffer]
.next_byte:
        dec     [_length]
        js      .done
        movzx   ebx, byte[esi]
        inc     esi
        mov     ecx, 8
.next_bit:
        mov     edx, eax
        xor     edx, ebx
        shr     eax, 1
        test    edx, 1
        jz      @f
        xor     eax, [_poly]
@@:
        shr     ebx, 1
        dec     ecx
        jnz     .next_bit
        jmp     .next_byte
.done:
        pop     esi edx ecx ebx
        ret
endp
