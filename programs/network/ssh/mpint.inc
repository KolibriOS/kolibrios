;    mpint.inc - Multi precision integer procedures
;
;    Copyright (C) 2015-2016 Jeffrey Amelynck
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

MPINT_MAX_LEN = MAX_BITS/8

; TODO: make procedures use real number length instead of hardcoded maximum length (MPINT_MAX_LEN)

mpint_to_little_endian:

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
; Fill the rest of the buffer with zeros.
  .zero:
        mov     ecx, MAX_BITS/8
        sub     ecx, eax
        xor     al, al
        rep stosb

        ret

mpint_to_big_endian:

; Load length dword
        lodsd
        test    eax, eax
        jz      .zero
        mov     ecx, eax
        add     esi, ecx
        dec     esi
        test    byte[esi], 0x80   ; Is the highest bit set?
        jz      @f
        inc     eax
  @@:
        push    eax
        bswap   eax
        stosd
; Copy data, convert to big endian meanwhile
        std
; Append zero byte if highest bit is 0
        test    byte[esi], 0x80
        jz      @f
        mov     byte[edi], 0
        inc     edi
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
        stosd
        ret

proc mpint_length uses edi eax ecx, mpint

        mov     edi, [mpint]
        mov     ecx, MPINT_MAX_LEN
        push    edi
        lea     edi, [edi + ecx + 4 - 1]
        xor     al, al
        std
        repe scasb
        cld
        je      @f
        inc     ecx
  @@:
        pop     edi
        mov     [edi], ecx

        ret

endp

proc mpint_print uses ecx esi eax, src

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

proc mpint_zero uses edi ecx eax, dst

        mov     edi, [dst]
        xor     eax, eax
        mov     ecx, MPINT_MAX_LEN/4+1
        rep stosd

        ret

endp

proc mpint_zero? uses edi ecx eax, dst

        mov     edi, [dst]
        add     edi, 4
        mov     ecx, MPINT_MAX_LEN/4
        xor     eax, eax
        repe scasd
        ret

endp

; return an index number giving the position of the highest order bit
proc mpint_hob uses edi ecx, dst

        mov     edi, [dst]
        ; start from the high order byte
        add     edi, MPINT_MAX_LEN+4-1
        mov     ecx, MPINT_MAX_LEN
        xor     eax, eax
        ; scan byte by byte for the first non-zero byte
        std
        repe scasb
        cld
        je      .zero
        ; calculate how many bits this is, plus 7
        lea     eax, [ecx*8-1]
        ; load this high order byte into cl
        mov     cl, [edi+1]
        ; shift bits of this byte right, until the byte reaches zero, counting bits meanwhile
  @@:
        inc     eax
        shr     cl, 1
        jnz     @r
  .zero:
        ret

endp

proc mpint_cmp uses esi edi ecx, dst, src

        mov     esi, [src]
        mov     edi, [dst]
        ; start from the high order byte
        add     esi, MPINT_MAX_LEN+4-4
        add     edi, MPINT_MAX_LEN+4-4
        mov     ecx, MPINT_MAX_LEN/4
        std
        repe cmpsd
        cld
        ret

endp

proc mpint_mov uses esi edi ecx, dst, src

        mov     esi, [src]
        mov     edi, [dst]
        mov     ecx, MPINT_MAX_LEN/4+1
        rep movsd

        ret

endp

proc mpint_mov0 uses esi edi ecx eax, dst, src

        mov     esi, [src]
        mov     edi, [dst]
        mov     ecx, [esi]
        mov     eax, ecx
        neg     eax
        add     esi, 4
        add     edi, 4
        rep movsb
        add     eax, MPINT_MAX_LEN
        jz      @f
        mov     ecx, eax
        xor     eax, eax
        rep stosb
  @@:

        ret

endp

proc mpint_shl1 uses edi ecx eax, dst

        mov     edi, [dst]
        add     edi, 4
        mov     ecx, MPINT_MAX_LEN/4-1

        shl     dword[edi], 1
        lahf
  @@:
        add     edi, 4
        sahf
        rcl     dword[edi], 1
        lahf
        dec     ecx
        jnz     @r
        sahf

        ret

endp

proc mpint_shr1 uses edi ecx eax, dst

        mov     edi, [dst]
        add     edi, MPINT_MAX_LEN+4-4
        mov     ecx, MPINT_MAX_LEN/4-1

        shr     dword[edi], 1
        lahf
  @@:
        sub     edi, 4
        sahf
        rcr     dword[edi], 1
        lahf
        dec     ecx
        jnz     @r
        sahf

        ret

endp

proc mpint_shl uses eax ebx ecx edx esi edi, dst, shift

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
        stdcall mpint_zero, [dst]
        ret

endp

; Left shift and copy
proc mpint_shlmov uses eax ebx ecx edx esi edi, dst, src, shift

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
        stdcall mpint_zero, [dst]
        ret

endp

proc mpint_add uses esi edi ecx eax, dst, src

        mov     esi, [src]
        add     esi, 4
        mov     edi, [dst]
        add     edi, 4
        mov     ecx, MPINT_MAX_LEN/4
        xor     ah, ah          ; clear flags (Carry flag most importantly)
  @@:
        sahf
        lodsd
        adc     [edi], eax
        lahf
        add     edi, 4
        dec     ecx
        jnz     @r
        sahf

        ret

endp

proc mpint_sub uses eax esi edi ecx, dst, src

        mov     esi, [src]
        add     esi, 4
        mov     edi, [dst]
        add     edi, 4
        mov     ecx, MPINT_MAX_LEN/4
  .loop:
        lodsd
        sub     [edi], eax
        jnc     @f
        dec     dword [edi+4]
  @@:
        add     edi, 4
        dec     ecx
        jnz     .loop
        ret

endp

proc mpint_mul uses esi edi ecx ebx eax, dst, A, B

        stdcall mpint_zero, [dst]

        ; first, find the byte in A containing the highest order bit
        mov     ecx, MPINT_MAX_LEN
        mov     edi, [A]
        add     edi, MPINT_MAX_LEN+4-1
        std
        xor     al, al
        repe scasb
        cld
        je      .zero
        inc     ecx
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

proc mpint_mod uses eax ecx, dst, mod

        ; if mod is zero, return
        stdcall mpint_zero?, [mod]
        jz      .zero

        stdcall mpint_cmp, [mod], [dst]
        jb      .done                           ; if dst < mod, dst = dst
        je      .zero                           ; if dst == mod, dst = 0

        ; left shift mod until the high order bits of mod and dst are aligned
        stdcall mpint_hob, [dst]
        mov     ecx, eax
        stdcall mpint_hob, [mod]
        sub     ecx, eax
        stdcall mpint_shlmov, mpint_tmp, [mod], ecx
        inc     ecx

        ; For every bit in dst (starting from the high order bit):
  .loop:
        ;   determine if dst is bigger than mpint_tmp
        stdcall mpint_cmp, [dst], mpint_tmp
        ja      @f
        ;   if so, subtract mpint_tmp from dst
        stdcall mpint_sub, [dst], mpint_tmp
  @@:
        dec     ecx
        jz      .done
        ;   shift mpint_tmp right by 1
        stdcall mpint_shr1, mpint_tmp
        jmp     .loop

  .zero:
        stdcall mpint_zero, [dst]
  .done:
        ret

endp

proc mpint_modexp uses edi eax ebx ecx, dst, base, exp, mod

        ; If mod is zero, return
        stdcall mpint_zero?, [mod]
        jz      .mod_zero

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

        ; Initialise result to base, to take care of the highest order bit
        stdcall mpint_mov0, [dst], [base]
        dec     bl
        jz      .next_byte
  .bit_loop:
        ; For each bit, square result
        stdcall mpint_mov, mpint_tmp, [dst]
        stdcall mpint_mul, [dst], mpint_tmp, mpint_tmp
        stdcall mpint_mod, [dst], [mod]

        ; If the bit is set, multiply result by the base
        shl     al, 1
        jnc     .next_bit
        stdcall mpint_mov, mpint_tmp, [dst]
        stdcall mpint_mul, [dst], [base], mpint_tmp
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
        DEBUGF  1, "modexp with modulo 0\n"
        ; if mod is zero, result = 0
        stdcall mpint_zero, [dst]
        ret

  .exp_zero:
        DEBUGF  1, "modexp with exponent 0\n"
        ; if exponent is zero, result = 1
        stdcall mpint_zero, [dst]
        mov     eax, [dst]
        mov     byte[eax], 1
        mov     byte[eax+4], 1
        ret

  .invalid:
        DEBUGF  1, "modexp: Invalid input!\n"
        ret

endp