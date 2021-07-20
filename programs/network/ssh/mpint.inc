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

; Notes:
;
; These procedures work only with positive integers.
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
proc mpint_cmp uses esi edi ecx eax, src, dst ;//////////////////////////////;;
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
        add     esi, ecx
        add     edi, ecx
        add     esi, 4
        add     edi, 4
        std
; If remaining bytes is not divisible by 4, compare only one byte at a time
  .do_byte:
        test    ecx, 1b
        jz      .do_dword
        dec     esi
        dec     edi
        mov     al, byte[esi]
        cmp     al, byte[edi]
        jne     .got_answer
        dec     ecx
; Remaining bytes is divisable by 4, compare dwords
  .do_dword:
        shr     ecx, 2
        jz      .got_answer
        sub     esi, 4
        sub     edi, 4
        repe cmpsd
  .got_answer:
        cld
        ret

endp

;;===========================================================================;;
proc mpint_mov uses esi edi ecx, dst, src ;//////////////////////////////////;;
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
        mov     ecx, [esi]
        push    ecx
        shr     ecx, 2
        inc     ecx             ; for length dword
        rep movsd
        pop     ecx
        and     ecx, 11b
        jz      @f
        rep movsb
  @@:

        ret

endp

;;===========================================================================;;
proc mpint_shl1 uses esi ecx, dst ;//////////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Shift little endian MPINT one bit to the left.                            ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = dst SHL 1                                                           ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_shl1(0x%x)\n", [dst]

        mov     esi, [dst]
        mov     ecx, [esi]
        test    ecx, ecx
        jz      .done

; Test if high order byte will overflow
; Remember: highest bit must never be set for positive numbers!
        test    byte[esi+ecx+3], 11000000b
        jz      @f
; We must grow a byte in size!
; TODO: check for overflow
        inc     ecx
        mov     [esi], ecx
        mov     byte[esi+ecx+3], 0        ; Add the new MSB
  @@:
        add     esi, 4
; Do the lowest order byte first
        shl     byte[esi], 1
        dec     ecx
        jz      .done
; And the remaining bytes
  @@:
        inc     esi
        rcl     byte[esi], 1
        dec     ecx
        jnz     @r
  .done:
        ret

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
        test    ecx, ecx
        jz      .done

; Do the highest order byte first
        add     edi, 4-1
        add     edi, ecx
        shr     byte[edi], 1
        dec     ecx
        jz      .done
; Now do the trailing bytes
  @@:
        dec     edi
        rcr     byte[edi], 1
        dec     ecx             ; does not affect carry flag, hooray!
        jnz     @r
  .done:
        ret

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
        jae     .overflow        ;;;;

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
proc mpint_add uses esi edi ecx eax, dst, src ;//////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Add a little endian MPINT to another little endian MPINT.                 ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;> dst = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = dst + src                                                           ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_add(0x%x, 0x%x)\n", [dst], [src]

        mov     esi, [src]
        mov     edi, [dst]
        stdcall mpint_bytes, esi
        mov     ecx, eax
        stdcall mpint_bytes, edi
        cmp     ecx, eax
        jb      .grow_src
        ja      .grow_dst
        test    ecx, ecx
        jz      .done

  .length_ok:
        push    ecx
        add     esi, 4
        add     edi, 4
; Add the first byte
        lodsb
        add     byte[edi], al
        dec     ecx
        jz      .done
; Add the other bytes
  @@:
        inc     edi
        lodsb
        adc     byte[edi], al
        dec     ecx
        jnz     @r
  .done:

; check if highest bit OR carry flag is set
; if so, add a byte if we have the buffer space
; TODO: check if we have the buffer space
        pop     ecx
        jc      .carry
        cmp     byte[edi], 0x80
        jnz     .high_bit_set

        ret

  .carry:
        mov     eax, [dst]
        cmp     [eax], ecx
        ja      @f
        inc     dword[eax]
  @@:
        mov     byte[edi+1], 1
        ret

  .high_bit_set:
        mov     eax, [dst]
        cmp     [eax], ecx
        ja      @f
        inc     dword[eax]
  @@:
        mov     byte[edi+1], 0
        ret

  .grow_dst:
        stdcall mpint_grow, edi, ecx
        jmp     .length_ok

  .grow_src:
        mov     ecx, eax
        stdcall mpint_grow, esi, ecx
        jmp     .length_ok

endp

;;===========================================================================;;
proc mpint_sub uses eax esi edi ecx, dst, src ;//////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Subtract a little endian MPINT to another little endian MPINT.            ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;> dst = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = dst - src                                                           ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_sub(0x%x, 0x%x)\n", [dst], [src]

        mov     esi, [src]
        mov     edi, [dst]
        stdcall mpint_bytes, esi
        mov     ecx, eax
        stdcall mpint_bytes, edi
        cmp     ecx, eax
        jb      .grow_src
        ja      .grow_dst
        test    ecx, ecx
        jz      .done

  .length_ok:
        add     esi, 4
        add     edi, 4
; Subtract the first byte
        lodsb
        sub     byte[edi], al
        dec     ecx
        jz      .done
; Subtract the other bytes
  @@:
        inc     edi
        lodsb
        sbb     byte[edi], al
        dec     ecx
        jnz     @r
  .done:
        ret

  .overflow:
        mov     dword[edi], 0
        stc
        ret

  .grow_dst:
        stdcall mpint_grow, edi, ecx
        jmp     .length_ok

  .grow_src:
        mov     ecx, eax
        stdcall mpint_grow, esi, ecx
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

;        mov     edi, [dst]
;        lodsd
;        std
;        mov     ecx, eax
;        dec     eax             ; total length minus one
;        add     edi, eax
;        xor     al, al
;        repe cmpsb
;        inc     ecx
;        mov     edi, [dst]
;        mov     [edi], ecx
;        cld

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
proc mpint_mul uses esi edi ecx ebx eax, dst, A, B ;/////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Multiply two little endian MPINTS and store them in a third one.          ;;
;;---------------------------------------------------------------------------;;
;> A = pointer to little endian MPINT                                        ;;
;> B = pointer to little endian MPINT                                        ;;
;> dst = pointer to buffer for little endian MPINT                           ;;
;;---------------------------------------------------------------------------;;
;< dst = A * B                                                               ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_mul(0x%x, 0x%x, 0x%x)\n", [dst], [A], [B]

        ; Set result to zero
        mov     eax, [dst]
        mov     dword[eax], 0

        mov     edi, [A]
        stdcall mpint_bytes, edi
        test    eax, eax
        jz      .zero
        add     edi, 4-1
        add     edi, eax
        mov     ecx, eax
; Iterate through the bits in A,
; starting from the highest order bit down to the lowest order bit.
  .next_byte:
        mov     al, [edi]
        dec     edi
        mov     bl, 8
  .next_bit:
        stdcall mpint_shl1, [dst]
        shl     al, 1
        jnc     .zero_bit
        stdcall mpint_add, [dst], [B]
  .zero_bit:
        dec     bl
        jnz     .next_bit
        dec     ecx
        jnz     .next_byte
  .zero:
        ret

endp

;;===========================================================================;;
proc mpint_mod uses eax ebx ecx, dst, mod ;//////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Find the modulo (remainder after division) of dst by mod.                 ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;> mod = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = dst MOD mod                                                         ;;
;;===========================================================================;;

        DEBUGF  1, "mpint_mod(0x%x, 0x%x)\n", [dst], [mod]

locals
        mpint_tmp       rb MPINT_MAX_LEN+4
endl

        stdcall mpint_cmp, [mod], [dst]
        ja      .done                           ; if mod > dst, dst = dst    ;;;;;;;
        je      .zero                           ; if mod == dst, dst = 0

        ; left shift mod until the high order bits of mod and dst are aligned

        stdcall mpint_bits, [dst]
        mov     ecx, eax
        stdcall mpint_bits, [mod]
        test    eax, eax
        jz      .zero                           ; if mod is zero, return
        sub     ecx, eax
        lea     ebx, [mpint_tmp]
        stdcall mpint_shlmov, ebx, [mod], ecx
        inc     ecx

        ; For every bit in dst (starting from the high order bit):
  .bitloop:
        stdcall mpint_cmp, [dst], ebx           ; If dst > mpint_tmp
        jb      @f                                                       ;;;;;;;;
        stdcall mpint_sub, [dst], ebx           ; dst = dst - mpint_tmp
  @@:
        dec     ecx
        jz      .done

        stdcall mpint_shr1, ebx                 ; mpint = mpint >> 1
        jmp     .bitloop

  .zero:
        mov     eax, [dst]
        mov     dword[eax], 0
  .done:
        ret

endp

;;===========================================================================;;
proc mpint_modexp uses edi eax ebx ecx edx, dst, base, exp, mod ;////////////;;
;;---------------------------------------------------------------------------;;
;? Find the modulo (remainder after division) of dst by mod.                 ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to buffer for little endian MPINT                           ;;
;> base = pointer to little endian MPINT                                     ;;
;> exp = pointer to little endian MPINT                                      ;;
;> mod = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = base ** exp MOD mod                                                 ;;
;;===========================================================================;;

        ;DEBUGF  1, "mpint_modexp(0x%x, 0x%x, 0x%x, 0x%x)\n", [dst], [base], [exp], [mod]

locals
        mpint_tmp       rb MPINT_MAX_LEN+4
endl

        ; If mod is zero, return
        stdcall mpint_bits, [mod]
        test    eax, eax
        jz      .mod_zero

        ; Find highest order byte in exponent
        stdcall mpint_bytes, [exp]
        test    eax, eax
        jz      .exp_zero
        mov     ecx, eax
        mov     edi, [exp]
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
        stdcall mpint_mov, [dst], [base]
        dec     bl
        jz      .next_byte
  .bit_loop:
        ; For each bit, square result
        stdcall mpint_mov, edx, [dst]
        stdcall mpint_mul, [dst], edx, edx
        stdcall mpint_mod, [dst], [mod]

        ; If the bit is set, multiply result by the base
        shl     al, 1
        jnc     .next_bit
        stdcall mpint_mov, edx, [dst]
        stdcall mpint_mul, [dst], [base], edx
        stdcall mpint_mod, [dst], [mod]
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
        ;stdcall mpint_print, [dst]
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

endp