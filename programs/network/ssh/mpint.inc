;    mpint.inc - Multi precision integer procedures
;
;    Copyright (C) 2015-2021 Jeffrey Amelynck
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

; Note:
;
; These procedures have been designed to work with unsigned integers.
; For compatibility reasons, the highest bit must always be 0.
;
; You have been warned!

MPINT_MAX_LEN = MAX_BITS/8


;;===========================================================================;;
proc mpint_to_little_endian uses esi edi ecx, dst, src ;/////////////////////;;
;;---------------------------------------------------------------------------;;
;? Convert big endian MPINT to little endian MPINT.                          ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to big endian MPINT                                         ;;
;> dst = pointer to buffer for little endian MPINT                           ;;
;;---------------------------------------------------------------------------;;
;< eax = MPINT number length                                                 ;;
;;===========================================================================;;

        mov     esi, [src]
        mov     edi, [dst]
; Load length dword
        lodsd
; Convert to little endian
        bswap   eax
        stosd
        test    eax, eax
        jz      .zero
; Copy data, convert to little endian meanwhile
        push    eax
        add     esi, eax
        dec     esi
        mov     ecx, eax
        std
  @@:
        lodsb
        mov     byte[edi], al
        inc     edi
        dec     ecx
        jnz     @r

        cld
        pop     eax
  .zero:
        ret

endp

;;===========================================================================;;
proc mpint_to_big_endian uses esi edi ecx, dst, src ;////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Convert little endian MPINT to big endian MPINT.                          ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;> dst = pointer to buffer for big endian MPINT                              ;;
;;---------------------------------------------------------------------------;;
;< eax = MPINT number length                                                 ;;
;;===========================================================================;;

        mov     esi, [src]
        mov     edi, [dst]
; Load length dword
        lodsd
        test    eax, eax
        jz      .zero
        mov     ecx, eax
        add     esi, eax
        dec     esi
        push    eax     ; we'll return length to the caller later
        bswap   eax
        stosd
; Copy data, convert to big endian meanwhile
        std
  @@:
        lodsb
        mov     byte[edi], al
        inc     edi
        dec     ecx
        jnz     @r
        cld
        pop     eax
        ret

  .zero:
        stosd           ; Number 0 has 0 data bytes
        ret

endp

;;===========================================================================;;
proc mpint_print uses ecx esi eax, src ;/////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Print MPINT to the debug board.                                           ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< -                                                                         ;;
;;===========================================================================;;

        DEBUGF  1, "0x"
        mov     esi, [src]
        mov     ecx, [esi]
        test    ecx, ecx
        jz      .zero
        lea     esi, [esi + ecx + 4 - 1]
        pushf
        std
  .loop:
        lodsb
        DEBUGF  1, "%x", eax:2
        dec     ecx
        jnz     .loop
        DEBUGF  1, "\n"
        popf

        ret

  .zero:
        DEBUGF  1, "00\n"
        ret

endp

;;===========================================================================;;
proc mpint_bits uses esi ecx, dst ;//////////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Count the number of bits in the MPINT                                     ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< eax = highest order bit number + 1                                        ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_bits(0x%x): ", [dst]

        mov     esi, [dst]
        mov     eax, [esi]
        test    eax, eax
        jz      .zero
        add     esi, 4-1
; Find highest order byte
  .byteloop:
        cmp     byte[esi+eax], 0
        jne     .nz
        dec     eax
        jnz     .byteloop
  .zero:
        DEBUGF  1, "%u\n", eax
        ret
  .nz:
        mov     cl, byte[esi+eax]
; multiply (eax - 1) by 8 to get nr of bits before this byte
        dec     eax
        shl     eax, 3
; Now shift bits of the highest order byte right, until the byte reaches zero, counting bits meanwhile
  .bitloop:
        inc     eax
        shr     cl, 1
        jnz     .bitloop
        DEBUGF  1, "%u\n", eax
        ret

endp

;;===========================================================================;;
proc mpint_bytes uses esi, dst ;/////////////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Count the number of bytes in the MPINT                                    ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< eax = highest order byte number + 1                                       ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_bytes(0x%x): ", [dst]

        mov     esi, [dst]
        mov     eax, [esi]
        test    eax, eax
        jz      .done
        add     esi, 4-1
; Find highest order byte
  .byteloop:
        cmp     byte[esi+eax], 0
        jne     .done
        dec     eax
        jnz     .byteloop
  .done:
        DEBUGF  1, "%u\n", eax
        ret

endp

;;===========================================================================;;
proc mpint_cmp uses esi edi edx ecx ebx eax, src, dst ;//////////////////////;;
;;---------------------------------------------------------------------------;;
;? Compare two MPINTS.                                                       ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;> src = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< flags are set as for single precision CMP instruction                     ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_cmp(0x%x, 0x%x)\n", [dst], [src]

; First, check the size of both numbers
        stdcall mpint_bytes, [dst]
        mov     ecx, eax
        stdcall mpint_bytes, [src]
; If one number has more bytes, it is bigger
        cmp     eax, ecx
        jne     .got_answer
; If both numbers have 0 bytes, they are equal
        test    ecx, ecx
        jz      .got_answer
; Numbers have equal amount of bytes
; Start comparing from the MSB towards the LSB
        mov     esi, [src]
        mov     edi, [dst]
        lea     esi, [esi + 4 + ecx]
        lea     edi, [edi + 4 + ecx]
; If remaining bytes is not divisible by 4, compare only one byte at a time
  .loop_1:
        test    ecx, 111b
        jz      .done_1
        dec     esi
        dec     edi
        mov     al, byte[esi]
        cmp     al, byte[edi]
        jne     .got_answer
        dec     ecx
        jmp     .loop_1
; Remaining bytes is divisable by 8, compare dwords
  .done_1:
        shr     ecx, 3
        jz      .got_answer

align 8
  .loop_8:
        lea     esi, [esi-8]
        lea     edi, [edi-8]
        mov     eax, [esi+04]
        mov     ebx, [esi+00]
        cmp     eax, [edi+04]
        jne     .got_answer
        cmp     ebx, [edi+00]
        jne     .got_answer
        dec     ecx
        jnz     .loop_8

  .got_answer:
        ret

endp

;;===========================================================================;;
proc mpint_mov uses esi edi edx ecx ebx eax, dst, src ;//////////////////////;;
;;---------------------------------------------------------------------------;;
;? Copy MPINT.                                                               ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to buffer for little endian MPINT                           ;;
;> src = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = src                                                                 ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_mov(0x%x, 0x%x)\n", [dst], [src]

        mov     esi, [src]
        mov     edi, [dst]
        mov     ecx, [esi]      ; Get dword count + 1
        add     ecx, 7          ;
        shr     ecx, 2          ;
        mov     edx, ecx

        shr     ecx, 3
        test    ecx, ecx
        jz      .no32
align 8
  .loop32:
        mov     eax, [esi+00]
        mov     ebx, [esi+04]
        mov     [edi+00], eax
        mov     [edi+04], ebx

        mov     eax, [esi+08]
        mov     ebx, [esi+12]
        mov     [edi+08], eax
        mov     [edi+12], ebx

        mov     eax, [esi+16]
        mov     ebx, [esi+20]
        mov     [edi+16], eax
        mov     [edi+20], ebx

        mov     eax, [esi+24]
        mov     ebx, [esi+28]
        mov     [edi+24], eax
        mov     [edi+28], ebx

        lea     esi, [esi+32]
        lea     edi, [edi+32]
        dec     ecx
        jnz     .loop32
  .no32:

        test    edx, 100b
        jz      .no16

        mov     eax, [esi+00]
        mov     ebx, [esi+04]
        mov     [edi+00], eax
        mov     [edi+04], ebx

        mov     eax, [esi+08]
        mov     ebx, [esi+12]
        mov     [edi+08], eax
        mov     [edi+12], ebx

        lea     esi, [esi+16]
        lea     edi, [edi+16]
  .no16:

        test    edx, 010b
        jz      .no8

        mov     eax, [esi+00]
        mov     ebx, [esi+04]
        mov     [edi+00], eax
        mov     [edi+04], ebx

        lea     esi, [esi+08]
        lea     edi, [edi+08]
  .no8:

        test    edx, 001b
        jz      .no4

        mov     eax, [esi+00]
        mov     [edi+00], eax
  .no4:

        ret

endp

;;===========================================================================;;
proc mpint_shl1 uses edi ecx, dst ;//////////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Shift little endian MPINT one bit to the left.                            ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = dst SHL 1                                                           ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_shl1(0x%x)\n", [dst]

        mov     edi, [dst]
        mov     ecx, [edi]
        test    ecx, 11b
        jnz     .adjust_needed
        shr     ecx, 2
        jz      .done

  .length_ok:
        add     edi, 4
; Do the lowest order dword first
        shl     dword[edi], 1
        lea     edi, [edi+4]
        dec     ecx
        jz      .done
; And the remaining dwords
  .loop:
        rcl     dword[edi], 1
        lea     edi, [edi+4]
        dec     ecx
        jnz     .loop
  .done:
        jc      .carry
        test    dword[edi-4], 0x10000000
        jnz     .add0
        ret

  .carry:
        mov     ecx, [dst]
        cmp     dword[ecx], MPINT_MAX_LEN
        je      .ovf
        add     dword[ecx], 4
        mov     dword[edi], 1
        ret

  .add0:
        mov     ecx, [dst]
        cmp     dword[ecx], MPINT_MAX_LEN
        je      .ovf
        add     dword[ecx], 4
        mov     dword[edi], 0
        ret

  .ovf:
        int3
;;;;
        ret

  .adjust_needed:
        add     ecx, 3
        and     ecx, not 3
        stdcall mpint_grow, edi, ecx
        jmp     .length_ok

endp

;;===========================================================================;;
proc mpint_shr1 uses edi ecx, dst ;//////////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Shift little endian MPINT one bit to the right.                           ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = dst SHR 1                                                           ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_shr1(0x%x)\n", [dst]

        mov     edi, [dst]
        mov     ecx, [edi]
        test    ecx, 11b
        jnz     .adjust_needed

  .length_ok:
        shr     ecx, 2
        jz      .done
        lea     edi, [edi+ecx*4]
; Do the highest order dword first
        shr     dword[edi], 1
        lea     edi, [edi-4]
        dec     ecx
        jz      .done
; And the remaining dwords
  .loop:
        rcr     dword[edi], 1
        lea     edi, [edi-4]
        dec     ecx
        jnz     .loop
  .done:
        ret

  .adjust_needed:
        add     ecx, 3
        and     ecx, not 3
        stdcall mpint_grow, edi, ecx
        jmp     .length_ok

endp

;;===========================================================================;;
proc mpint_shl uses eax ebx ecx edx esi edi, dst, shift ;////////////////////;;
;;---------------------------------------------------------------------------;;
;? Left shift little endian MPINT by x bits.                                 ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;> shift = number of bits to shift the MPINT                                 ;;
;;---------------------------------------------------------------------------;;
;< -                                                                         ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_shl(0x%x, %u)\n", [dst], [shift]

; Calculate new size
        stdcall mpint_bits, [dst]
        add     eax, [shift]
        shr     eax, 3
        cmp     eax, MPINT_MAX_LEN
        jae     .overflow     ;;
        inc     eax
        mov     esi, [dst]
        mov     [esi], eax

        mov     ecx, [shift]
        shr     ecx, 3                  ; 8 bits in one byte
        add     esi, MPINT_MAX_LEN+4-4
        mov     edi, esi
        and     ecx, not 11b
        sub     esi, ecx
        mov     edx, MPINT_MAX_LEN/4-1
        shr     ecx, 2                  ; 4 bytes in one dword
        push    ecx
        sub     edx, ecx
        mov     ecx, [shift]
        and     ecx, 11111b
        std
  .loop:
        lodsd
        mov     ebx, [esi]
        shld    eax, ebx, cl
        stosd
        dec     edx
        jnz     .loop
        lodsd
        shl     eax, cl
        stosd

        ; fill the LSBs with zeros
        pop     ecx
        test    ecx, ecx
        jz      @f
        xor     eax, eax
        rep stosd
  @@:
        cld
        ret

  .zero:
        mov     eax, [dst]
        mov     dword[eax], 0
        ret

  .overflow:
        int3
        ret

endp

;;===========================================================================;;
proc mpint_shlmov uses eax ebx ecx edx esi edi, dst, src, shift ;////////////;;
;;---------------------------------------------------------------------------;;
;? Left shift by x bits and copy little endian MPINT.                        ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;> dst = pointer to little endian MPINT                                      ;;
;> shift = number of bits to shift the MPINT to the left                     ;;
;;---------------------------------------------------------------------------;;
;< dst = src SHL shift                                                       ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_shlmov(0x%x, 0x%x, %u)\n", [dst], [src], [shift]

        stdcall mpint_bits, [src]
        test    eax, eax
        jz      .zero
        add     eax, [shift]
        shr     eax, 3
        inc     eax
        mov     edi, [dst]
        mov     [edi], eax

        cmp     eax, MPINT_MAX_LEN
        jae     .overflow

        mov     esi, [src]
        add     esi, MPINT_MAX_LEN+4-4
        add     edi, MPINT_MAX_LEN+4-4
        mov     ecx, [shift]
        shr     ecx, 3                  ; 8 bits in one byte
        and     ecx, not 11b
        sub     esi, ecx
        mov     edx, MPINT_MAX_LEN/4-1
        shr     ecx, 2                  ; 4 bytes in one dword
        push    ecx
        sub     edx, ecx
        mov     ecx, [shift]
        and     ecx, 11111b
        std
  .loop:
        lodsd
        mov     ebx, [esi]
        shld    eax, ebx, cl
        stosd
        dec     edx
        jnz     .loop
        lodsd
        shl     eax, cl
        stosd

        ; fill the lsb bytes with zeros
        pop     ecx
        test    ecx, ecx
        jz      @f
        xor     eax, eax
        rep stosd
  @@:
        cld
        ret

  .zero:
        mov     eax, [dst]
        mov     dword[eax], 0
        ret

  .overflow:
        int3
        ret

endp

;;===========================================================================;;
proc mpint_add uses esi edi edx ecx ebx eax, dst, src ;//////////////////////;;
;;---------------------------------------------------------------------------;;
;? Add a little endian MPINT to another little endian MPINT.                 ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;> dst = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = dst + src                                                           ;;
;;===========================================================================;;

locals
        dd_cnt  dd ?
endl

        DEBUGF  1, "mpint_add(0x%x, 0x%x)\n", [dst], [src]

; Grow both numbers to same 4-byte boundary, if not already the case
        mov     esi, [src]
        mov     edi, [dst]
        mov     ecx, [esi]
        test    ecx, 11b
        jnz     .adjust_needed
        cmp     ecx, [edi]
        jne     .adjust_needed

; Do the additions
  .length_ok:
        add     esi, 4
        add     edi, 4
        shr     ecx, 2
        jz      .done
        mov     eax, ecx
        and     eax, 111b
        mov     [dd_cnt], eax
        shr     ecx, 3
        test    ecx, ecx                ; Clear carry flag
        jz      .no32

  .loop32:
        mov     eax, [esi+00]
        mov     edx, [esi+04]
        adc     [edi+00], eax
        adc     [edi+04], edx

        mov     eax, [esi+08]
        mov     edx, [esi+12]
        adc     [edi+08], eax
        adc     [edi+12], edx

        mov     eax, [esi+16]
        mov     edx, [esi+20]
        adc     [edi+16], eax
        adc     [edi+20], edx

        mov     eax, [esi+24]
        mov     edx, [esi+28]
        adc     [edi+24], eax
        adc     [edi+28], edx

        lea     esi, [esi + 32]
        lea     edi, [edi + 32]
        dec     ecx
        jnz     .loop32

  .no32:
        mov     ecx, [dd_cnt]
        dec     ecx
        js      .check_ovf
        inc     ecx
  .dword_loop:
        mov     eax, [esi+0]
        adc     [edi+0], eax
        lea     esi, [esi + 4]
        lea     edi, [edi + 4]
        dec     ecx
        jnz     .dword_loop

  .check_ovf:
        jc      .add_1                  ; Carry
        test    byte[edi-1], 0x80
        jnz     .add_0                  ; Currently highest bit set
  .done:
        ret

; Highest bit was set, add a 0 byte as MSB if possible
  .add_0:
        mov     ecx, [dst]
        cmp     dword[ecx], MPINT_MAX_LEN
        jae     .ovf_0
        mov     byte[edi], 0
        inc     dword[ecx]
        ret

  .ovf_0:
        int3
        clc
; TODO: set overflow flag?
        ret

; Carry bit was set, add a 1 byte as MSB if possible
  .add_1:
        mov     ecx, [dst]
        cmp     dword[ecx], MPINT_MAX_LEN
        jae     .ovf_1
        mov     byte[edi], 1
        inc     dword[ecx]
        ret

  .ovf_1:
        int3
        stc
; TODO: set overflow flag?
        ret

  .adjust_needed:
;        mov     ecx, [esi]
        mov     eax, [edi]
; find the maximum of the two in ecx
        mov     edx, ecx
        sub     edx, eax
        sbb     ebx, ebx
        and     ebx, edx
        sub     ecx, ebx
; align to 4 byte boundary
        add     ecx, 3
        and     ecx, not 3
; adjust both mpints
        stdcall mpint_grow, esi, ecx
        stdcall mpint_grow, edi, ecx
        jmp     .length_ok

endp

;;===========================================================================;;
proc mpint_sub uses esi edi edx ecx ebx eax, dst, src ;//////////////////////;;
;;---------------------------------------------------------------------------;;
;? Subtract a little endian MPINT to another little endian MPINT.            ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;> dst = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = dst - src                                                           ;;
;;===========================================================================;;

locals
        dd_cnt  dd ?
endl

        DEBUGF  1, "mpint_sub(0x%x, 0x%x)\n", [dst], [src]

; Grow both numbers to same 4-byte boundary, if not already the case
        mov     esi, [src]
        mov     edi, [dst]
        mov     ecx, [esi]
        test    ecx, 11b
        jnz     .adjust_needed
        cmp     ecx, [edi]
        jne     .adjust_needed

; Do the subtractions
  .length_ok:
        add     esi, 4
        add     edi, 4
        shr     ecx, 2
        jz      .done
        mov     eax, ecx
        and     eax, 111b
        mov     [dd_cnt], eax
        shr     ecx, 3
        test    ecx, ecx                ; Clear carry flag
        jz      .no32

  .loop32:
        mov     eax, [esi+00]
        mov     edx, [esi+04]
        sbb     [edi+00], eax
        sbb     [edi+04], edx

        mov     eax, [esi+08]
        mov     edx, [esi+12]
        sbb     [edi+08], eax
        sbb     [edi+12], edx

        mov     eax, [esi+16]
        mov     edx, [esi+20]
        sbb     [edi+16], eax
        sbb     [edi+20], edx

        mov     eax, [esi+24]
        mov     edx, [esi+28]
        sbb     [edi+24], eax
        sbb     [edi+28], edx

        lea     esi, [esi + 32]
        lea     edi, [edi + 32]
        dec     ecx
        jnz     .loop32

  .no32:
        mov     ecx, [dd_cnt]
        dec     ecx
        js      .done
        inc     ecx
  .dword_loop:
        mov     eax, [esi+0]
        sbb     [edi+0], eax
        lea     esi, [esi + 4]
        lea     edi, [edi + 4]
        dec     ecx
        jnz     .dword_loop

  .done:
        ret

  .adjust_needed:
;        mov     ecx, [esi]
        mov     eax, [edi]
; find the maximum of the two in ecx
        mov     edx, ecx
        sub     edx, eax
        sbb     ebx, ebx
        and     ebx, edx
        sub     ecx, ebx
; align to 4 byte boundary
        add     ecx, 3
        and     ecx, not 3
; adjust both mpints
        stdcall mpint_grow, esi, ecx
        stdcall mpint_grow, edi, ecx
        jmp     .length_ok

endp


;;===========================================================================;;
proc mpint_shrink uses eax edi, dst ;////////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Get rid of unnescessary leading zeroes on a little endian MPINT.          ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;<                                                                           ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_shrink(0x%x)\n", [dst]

        stdcall mpint_bits, [dst]
        shr     eax, 3
        inc     eax
        mov     edi, [dst]
        mov     [edi], eax

        ret

endp

;;===========================================================================;;
proc mpint_grow uses eax edi ecx, dst, length ;//////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Add leading zeroes on a little endian MPINT.                              ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;> length = total length of the new MPINT in bytes                           ;;
;;---------------------------------------------------------------------------;;
;<                                                                           ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_grow(0x%x, %u): ", [dst], [length]

        mov     edi, [dst]
        mov     eax, [edi]
        mov     ecx, [length]
        sub     ecx, eax
        jbe     .dontgrow
        lea     edi, [edi + 4 + eax]
        xor     al, al
        rep stosb
        mov     eax, [length]
        mov     edi, [dst]
        mov     [edi], eax
        DEBUGF  1, "ok\n"
        ret

  .dontgrow:
        DEBUGF  1, "already large enough!\n"
        ret

endp

;;===========================================================================;;
proc mpint_mul uses eax ebx ecx edx esi edi, dst, a, b ;///////////////////////;;
;;---------------------------------------------------------------------------;;
;? Multiply a little endian MPINT with another little endian MPINT and store ;;
;? in a third one.                                                           ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;> a = pointer to little endian MPINT                                        ;;
;> b = pointer to little endian MPINT                                        ;;
;;---------------------------------------------------------------------------;;
;< dst = a * b                                                               ;;
;;===========================================================================;;

locals
        asize   dd ?
        bsize   dd ?
        counter dd ?
        esp_    dd ?
endl

        DEBUGF  1, "mpint_mul(0x%x, 0x%x, 0x%x)\n", [dst], [a], [b]

; Grow both numbers to individual 4-byte boundary, if not already the case
        mov     esi, [a]
        mov     edx, [b]
        mov     ecx, [esi]
        mov     ebx, [edx]
        test    ecx, 11b
        jnz     .adjust_needed
        test    ebx, 11b
        jnz     .adjust_needed
  .length_ok:

; Must have Asize >= Bsize.
        cmp     ebx, ecx
        ja      .swap_a_b
  .conditions_ok:

; D size will be A size + B size
        lea     eax, [ebx + ecx]
        cmp     eax, MPINT_MAX_LEN
        ja      .ovf

; [Asize] = number of dwords in x
        shr     ecx, 2
        jz      .zero
        mov     [asize], ecx
; esi = x ptr
        add     esi, 4

; [Bsize] = number of dwords in y
        shr     ebx, 2
        jz      .zero
        mov     [bsize], ebx
; edx = y ptr (temporarily)
        add     edx, 4

; store D size
        mov     edi, [dst]
        mov     [edi], eax
; edi = D ptr
        add     edi, 4

; Use esp as frame pointer instead of ebp
; ! Use the stack with extreme caution from here on !
        mov     [esp_], esp
        mov     esp, ebp

; ebp = B ptr
        mov     ebp, edx

; Do the first multiplication
        mov     eax, [esi]              ; load A[0]
        mul     dword[ebp]              ; multiply by B[0]
        mov     [edi], eax              ; store to D[0]
;        mov     ecx, [Asize]            ; Asize
        dec     ecx                     ; if Asize = 1, Bsize = 1 too
        jz      .done

; Prepare to enter loop1
        mov     eax, [asize-ebp+esp]

        mov     ebx, edx
        lea     esi, [esi + eax * 4]    ; make A ptr point at end
        lea     edi, [edi + eax * 4]    ; offset D ptr by Asize
        neg     ecx                     ; negate j size/index for inner loop
        xor     eax, eax                ; clear carry

align 8
  .loop1:
        adc     ebx, 0
        mov     eax, [esi + ecx * 4]    ; load next dword at A[j]
        mul     dword[ebp]
        add     eax, ebx
        mov     [edi + ecx * 4], eax
        inc     ecx
        mov     ebx, edx
        jnz     .loop1

        adc     ebx, 0
        mov     eax, [bsize-ebp+esp]
        mov     [edi], ebx              ; most significant dword of the product
        add     edi, 4                  ; increment dst
        dec     eax
        jz      .skip
        mov     [counter-ebp+esp], eax  ; set index i to Bsize

  .outer:
        add     ebp, 4                  ; make ebp point to next B dword
        mov     ecx, [asize-ebp+esp]
        neg     ecx
        xor     ebx, ebx

  .loop2:
        adc     ebx, 0
        mov     eax, [esi + ecx * 4]
        mul     dword[ebp]
        add     eax, ebx
        mov     ebx, [edi + ecx * 4]
        adc     edx, 0
        add     ebx, eax
        mov     [edi + ecx * 4], ebx
        inc     ecx
        mov     ebx, edx
        jnz     .loop2

        adc     ebx, 0

        mov     [edi], ebx
        add     edi, 4
        mov     eax, [counter-ebp+esp]
        dec     eax
        mov     [counter-ebp+esp], eax
        jnz     .outer

  .skip:
; restore esp, ebp
        mov     ebp, esp
        mov     esp, [esp_]

        ret

  .done:
        mov     [edi+4], edx    ; store to D[1]
; restore esp, ebp
        mov     ebp, esp
        mov     esp, [esp_]

        ret

  .ovf:
        int3

  .zero:
        mov     eax, [dst]
        mov     dword[eax], 0

        ret

  .adjust_needed:
; align to 4 byte boundary
        add     ecx, 3
        and     ecx, not 3
        add     ebx, 3
        and     ebx, not 3
; adjust both mpints
        stdcall mpint_grow, esi, ecx
        stdcall mpint_grow, edx, ebx
        jmp     .length_ok

  .swap_a_b:
        mov     eax, esi
        mov     esi, edx
        mov     edx, eax

        mov     eax, ebx
        mov     ebx, ecx
        mov     ecx, eax
        jmp     .conditions_ok

endp

;;===========================================================================;;
proc mpint_mod uses eax ebx ecx, dst, m ;////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Find the modulo (remainder after division) of dst by mod.                 ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;> mod = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = dst MOD m                                                           ;;
;;===========================================================================;;

locals
        mpint_tmp       rb MPINT_MAX_LEN+4
endl

        DEBUGF  1, "mpint_mod(0x%x, 0x%x)\n", [dst], [m]

        stdcall mpint_cmp, [m], [dst]
        ja      .done                           ; if mod > dst, dst = dst
        je      .zero                           ; if mod == dst, dst = 0

        ; left shift mod until the high order bits of mod and dst are aligned

        stdcall mpint_bits, [dst]
        mov     ecx, eax
        stdcall mpint_bits, [m]
        test    eax, eax
        jz      .zero                           ; if mod is zero, return
        sub     ecx, eax
        lea     ebx, [mpint_tmp]
        stdcall mpint_shlmov, ebx, [m], ecx
        inc     ecx

        ; For every bit in dst (starting from the high order bit):
  .bitloop:
        stdcall mpint_cmp, [dst], ebx           ; If dst > mpint_tmp
        jb      @f
        stdcall mpint_sub, [dst], ebx           ; dst = dst - mpint_tmp
  @@:
        dec     ecx
        jz      .done

        stdcall mpint_shr1, ebx                 ; mpint = mpint >> 1
        jmp     .bitloop

  .done:
; adjust size of dst so it is no larger than mod
        mov     ebx, [dst]
        mov     ecx, [ebx]      ; current size
        mov     eax, [m]
        mov     eax, [eax]      ; size of mod
        cmp     ecx, eax
        jb      .ret
        mov     [ebx], eax
  .ret:
        ret

  .zero:
        mov     ebx, [dst]
        mov     dword[ebx], 0
        ret

endp

;;===========================================================================;;
proc mpint_modexp uses edi eax ebx ecx edx, dst, b, e, m ;///////////////////;;
;;---------------------------------------------------------------------------;;
;? Find the modulo (remainder after division) of dst by mod.                 ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to buffer for little endian MPINT                           ;;
;> base = pointer to little endian MPINT                                     ;;
;> exp = pointer to little endian MPINT                                      ;;
;> mod = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = b ** e MOD m                                                        ;;
;;===========================================================================;;

locals
        mpint_tmp       rb MPINT_MAX_LEN+4
endl

        DEBUGF  1, "mpint_modexp(0x%x, 0x%x, 0x%x, 0x%x)\n", [dst], [b], [e], [m]

        ; If mod is zero, return
        stdcall mpint_bytes, [m]
        test    eax, eax
        jz      .mod_zero
        test    eax, 3
        jnz     .grow_mod
  .modsize_ok:

        ; Find highest order byte in exponent
        stdcall mpint_bytes, [e]
        test    eax, eax
        jz      .exp_zero
        mov     ecx, eax
        mov     edi, [e]
        lea     edi, [edi + 4 + ecx - 1]

        ; Find the highest order bit in this byte
        mov     al, [edi]
        test    al, al
        jz      .invalid
        mov     bl, 9
  @@:
        dec     bl
        shl     al, 1
        jnc     @r

        ; Make pointer to tmp mpint for convenient access
        lea     edx, [mpint_tmp]

        ; Initialise result to base, to take care of the highest order bit
        stdcall mpint_mov, [dst], [b]
        dec     bl
        jz      .next_byte
  .bit_loop:
        ; For each bit, square result
        stdcall mpint_mov, edx, [dst]
        stdcall mpint_mul, [dst], edx, edx
        stdcall mpint_mod, [dst], [m]

        ; If the bit is set, multiply result by the base
        shl     al, 1
        jnc     .next_bit
        stdcall mpint_mov, edx, [dst]
        stdcall mpint_mul, [dst], [b], edx
        stdcall mpint_mod, [dst], [m]
  .next_bit:
        dec     bl
        jnz     .bit_loop
  .next_byte:
        dec     ecx
        jz      .done
        dec     edi
        mov     al, [edi]
        mov     bl, 8
        jmp     .bit_loop
  .done:
        ret

  .mod_zero:
        DEBUGF  3, "modexp with modulo 0\n"
        ; if mod is zero, result = 0
        mov     eax, [dst]
        mov     dword[eax], 0
        ret

  .exp_zero:
        DEBUGF  3, "modexp with exponent 0\n"
        ; if exponent is zero, result = 1
        mov     eax, [dst]
        mov     dword[eax], 1
        mov     byte[eax+4], 1
        ret

  .invalid:
        DEBUGF  3, "modexp: Invalid input!\n"
        ret

  .grow_mod:
        add     eax, 3
        and     eax, not 3
        stdcall mpint_grow, [m], eax
        jmp     .modsize_ok

endp

