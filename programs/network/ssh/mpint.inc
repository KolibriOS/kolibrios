;    mpint.inc - Multi precision integer procedures
;
;    Copyright (C) 2015-2017 Jeffrey Amelynck
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
; However, leading 0 bytes MUST at all other times be omitted.
;
; You have been warned!

MPINT_MAX_LEN = MAX_BITS/8


;;===========================================================================;;
proc mpint_to_little_endian uses esi edi ecx ;///////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Convert big endian MPINT to little endian MPINT.                          ;;
;;---------------------------------------------------------------------------;;
;> esi = pointer to big endian MPINT                                         ;;
;> edi = pointer to buffer for little endian MPINT                           ;;
;;---------------------------------------------------------------------------;;
;< eax = MPINT number length                                                 ;;
;;===========================================================================;;

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
        push    esi
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
        pop     esi eax
  .zero:
        ret

endp

;;===========================================================================;;
proc mpint_to_big_endian uses esi edi ecx ;//////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Convert little endian MPINT to big endian MPINT.                          ;;
;;---------------------------------------------------------------------------;;
;> esi = pointer to little endian MPINT                                      ;;
;> edi = pointer to buffer for big endian MPINT                              ;;
;;---------------------------------------------------------------------------;;
;< eax = MPINT number length                                                 ;;
;;===========================================================================;;

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
proc mpint_hob uses edi ecx eax, dst ;///////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Return an index number giving the position of the highest order bit.      ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< eax = highest order bit number                                            ;;
;;===========================================================================;;

        mov     edi, [dst]
        lodsd
        dec     eax                     ; total length minus one
        mov     cl, [edi+eax]           ; load the highest order byte
        shl     eax, 3                  ; multiply eax by 8 to get nr of bits

; Now shift bits of the highest order byte right, until the byte reaches zero, counting bits meanwhile
        test    cl, cl
        jz      .end
  @@:
        inc     eax
        shr     cl, 1
        jnz     @r
  .end:
        ret

endp

;;===========================================================================;;
proc mpint_cmp uses esi edi ecx eax, dst, src ;//////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Compare two mpints.                                                       ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to little endian MPINT                                      ;;
;> src = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< flags are set as for single precision CMP instruction                     ;;
;;===========================================================================;;

; First, check if number of significant bytes is the same
; If not, number with more bytes is bigger
        mov     esi, [src]
        mov     edi, [dst]
        mov     ecx, [esi]
        cmp     ecx, [edi]
        jne     .got_answer

; Numbers have equal amount of bytes, compare starting from the high order byte
        add     edi, ecx
        add     esi, ecx
        std
  .do_byte:
        test    ecx, 11b
        jz      .do_dword
        dec     esi
        dec     edi
        cmpsb
        jne     .got_answer
        dec     ecx
        jmp     .do_byte
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
;? Copy mpint.                                                               ;;
;;---------------------------------------------------------------------------;;
;> dst = pointer to buffer for little endian MPINT                           ;;
;> src = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;< dst = src                                                                 ;;
;;===========================================================================;;

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

        mov     edi, [dst]
        mov     ecx, [edi]
        test    ecx, ecx
        jz      .done

; Do the highest order byte first
        dec     ecx
        shr     byte[edi+ecx+3], 1
; Was it 0? If so, we must decrement total length
        jnz     @f
        jc      @f
        mov     [edi], ecx
  @@:
        test    ecx, ecx
        jz      .done
; Now do the trailing bytes
        add     edi, 4
        add     edi, ecx
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

        mov     ecx, [shift]
        shr     ecx, 3                  ; 8 bits in one byte
        cmp     ecx, MPINT_MAX_LEN
        jge     .zero
        mov     esi, [dst]
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

        mov     ecx, [shift]
        shr     ecx, 3                  ; 8 bits in one byte
        cmp     ecx, MPINT_MAX_LEN
        jge     .zero
        mov     esi, [src]
        add     esi, MPINT_MAX_LEN+4-4
        mov     edi, [dst]
        add     edi, MPINT_MAX_LEN+4-4
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

        mov     esi, [src]
        mov     edi, [dst]
        mov     ecx, [esi]      ; source number length
        sub     ecx, [dst]
        jbe     .length_ok
; Length of the destination is currently smaller then the source, pad with 0 bytes
        add     edi, [edi]
        add     edi, 4
        mov     al, 0
        rep stosb
  .length_ok:
        mov     ecx, [esi]
        mov     edi, [dst]
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
        jc      .carry
        cmp     byte[edi], 0x80
        jnz     .high_bit_set

        ret

  .carry:
        inc     edi
        mov     byte[edi], 1
        mov     eax, [dst]
        inc     dword[eax]

        ret

  .high_bit_set:
        inc     edi
        mov     byte[edi], 0
        mov     eax, [dst]
        inc     dword[eax]

        ret

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

        mov     esi, [src]
        mov     edi, [dst]
        mov     ecx, [esi]      ; destination number length
        cmp     ecx, [edi]
        ja      .overflow

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
        stdcall mpint_shrink, [dst]
        ret

  .overflow:
        mov     dword[edi], 0
        stc
        ret

endp


;;===========================================================================;;
proc mpint_shrink uses eax edi ecx, dst ;////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Get rid of leading zeroes on a little endian MPINT.                       ;;
;;---------------------------------------------------------------------------;;
;> src = pointer to little endian MPINT                                      ;;
;;---------------------------------------------------------------------------;;
;<                                                                           ;;
;;===========================================================================;;

        mov     edi, [dst]
        lodsd
        std
        mov     ecx, eax
        dec     eax             ; total length minus one
        add     edi, eax
        xor     al, al
        repe cmpsb
        inc     ecx
        mov     edi, [dst]
        mov     [edi], ecx
        cld

        ret

endp

;;===========================================================================;;
proc mpint_mul uses esi edi ecx ebx eax, dst, A, B ;/////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Multiply to little endian MPINTS and store them in a new one.             ;;
;;---------------------------------------------------------------------------;;
;> A = pointer to little endian MPINT                                        ;;
;> B = pointer to little endian MPINT                                        ;;
;> dst = pointer to buffer for little endian MPINT                           ;;
;;---------------------------------------------------------------------------;;
;< dst = A * B                                                               ;;
;;===========================================================================;;

        ; Set result to zero
        mov     eax, [dst]
        mov     dword[eax], 0

        ; first, find the byte in A containing the highest order bit
        mov     edi, [A]
        mov     eax, [edi]
        test    eax, eax
        jz      .zero
        add     edi, eax
        mov     al, [edi+1]
        mov     esi, edi
        mov     bl, 8
  @@:
        shl     al, 1
        jc      .first_hit
        dec     bl
        jnz     @r

        ; Then, starting from this byte, iterate through the bits in A,
        ; starting from the highest order bit down to the lowest order bit.
  .next_byte:
        mov     al, [edi]
        dec     edi
        mov     bl, 8
  .next_bit:
        stdcall mpint_shl1, [dst]
        shl     al, 1
        jnc     .zero_bit
  .first_hit:
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

locals
        mpint_tmp       rb MPINT_MAX_LEN+4
endl

        ; if mod is zero, return
        mov     eax, [mod]
        cmp     dword[eax], 0
        je      .zero

        stdcall mpint_cmp, eax, [dst]
        jb      .done                           ; if dst < mod, dst = dst
        je      .zero                           ; if dst == mod, dst = 0

        lea     ebx, [mpint_tmp]

        ; left shift mod until the high order bits of mod and dst are aligned
        stdcall mpint_hob, [dst]
        mov     ecx, eax
        stdcall mpint_hob, [mod]
        sub     ecx, eax
        stdcall mpint_shlmov, ebx, [mod], ecx
        inc     ecx

        ; For every bit in dst (starting from the high order bit):
  .loop:
        ;   determine if dst is bigger than mpint_tmp
        stdcall mpint_cmp, [dst], ebx
        ja      @f
        ;   if so, subtract mpint_tmp from dst
        stdcall mpint_sub, [dst], ebx
  @@:
        dec     ecx
        jz      .done
        ;   shift mpint_tmp right by 1
        stdcall mpint_shr1, ebx
        jmp     .loop

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

locals
        mpint_tmp       rb MPINT_MAX_LEN+4
endl

        ; If mod is zero, return
        mov     eax, [mod]
        cmp     dword[eax], 0
        je      .mod_zero

        ; Find the highest order byte in exponent
        mov     edi, [exp]
        mov     ecx, [edi]
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