;   ix87 specific implementation of pow function.
;   Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
;   This file is part of the GNU C Library.
;   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

;   The GNU C Library is free software; you can redistribute it and/or
;   modify it under the terms of the GNU Library General Public License as
;   published by the Free Software Foundation; either version 2 of the
;   License, or (at your option) any later version.

;   The GNU C Library is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;   Library General Public License for more details.

;   You should have received a copy of the GNU Library General Public
;   License along with the GNU C Library; see the file COPYING.LIB.  If not,
;   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
;   Boston, MA 02111-1307, USA.  */

format MS COFF

include 'proc32.inc'

section '.text' code readable executable

;public _pow_test@8

public _scalbn

align 4
proc _scalbn
	          fild	dword [esp+12]
	          fld	qword [esp+4]
	          fscale
	          fstp	st1
	          ret
endp

proc _pow_test@8 stdcall x:dword, y:dword
           fld [x]
           fld [y]		
           jmp __CIpow


 __CIpow:
;       fldl    12(%esp)        // y

        fxam

        fnstsw ax
        mov dl,ah
        and ah, 0x45
        cmp ah, 0x40      ; is y == 0 ?
        je .L_11

        cmp ah, 0x05      ; is y == ±inf ?
        je .L_12

        cmp ah, 0x01      ; is y == NaN ?
        je .L_30

        fxch

        sub  esp, 8

	fxam
        fnstsw ax
        mov dh, ah
        and ah, 0x45
        cmp ah, 0x40
        je  .L_20          ; x is ±0

        cmp ah, 0x05
        je  .L_15          ; x is ±inf

        fxch               ; y : x

; First see whether `y' is a natural number.  In this case we
; can use a more precise algorithm.  */

        fld st                ; y : y : x
        fistp qword [esp]     ; y : x
        fild  qword [esp]     ; int(y) : y : x
        fucomp  st1           ; y : x
        fnstsw ax
	sahf
        jne .L_2

; OK, we have an integer value for y.  */

        pop eax
        pop edx
        or edx,0
        fstp st0              ; x
        jns .L_4              ; y >= 0, jump
        fidiv dword [one]     ; 1/x          (now referred to as x)
        neg eax
        adc edx,0
        neg edx
.L_4:
        fld1                  ; 1 : x
	fxch
.L_6:
        shrd edx, eax,1
        jnc .L_5
	fxch
        fmul st1,st0            ; x : ST*x
	fxch
.L_5:
        fmul st0, st0        ; x*x : ST*x
        shr edx,1
        mov ecx, eax
        or  ecx, edx
        jnz .L_6
        fstp st0            ; ST*x
.L_30:
        ret

align 4

; y is a real number.  */

.L_2:
        fxch                    ; x : y
        fld1                    ; 1.0 : x : y
        fld      st1            ; x : 1.0 : x : y
        fsub     st0,st1        ; x-1 : 1.0 : x : y
        fabs                    ; |x-1| : 1.0 : x : y
        fcomp qword [limit]     ; 1.0 : x : y
        fnstsw  ax
        fxch                    ; x : 1.0 : y
	sahf
        ja .L_7
        fsub st0, st1           ; x-1 : 1.0 : y
        fyl2xp1                 ; log2(x) : y
        jmp .L_8
.L_7:
        fyl2x                   ; log2(x) : y
.L_8:
        fmul     st0,st1        ; y*log2(x) : y
        fst      st1            ; y*log2(x) : y*log2(x)
        frndint                 ; int(y*log2(x)) : y*log2(x)
        fsubr    st1, st0       ; int(y*log2(x)) : fract(y*log2(x))
        fxch                    ; fract(y*log2(x)) : int(y*log2(x))
        f2xm1                   ; 2^fract(y*log2(x))-1 : int(y*log2(x))
        fld1
        faddp                   ; 2^fract(y*log2(x)) : int(y*log2(x))
        fscale                  ; 2^fract(y*log2(x))*2^int(y*log2(x)) : int(y*log2(x))
        add esp,8
        fstp st1                ; 2^fract(y*log2(x))*2^int(y*log2(x))
	ret


align 4
 ;       // pow(x,±0) = 1

.L_11:
        fstp st0              ; pop y
        fld1
	ret

align 4

; y == ±inf

.L_12:
        fstp st0              ; pop y
;        fld   4(%esp)          ; x
	fabs
        fcomp qword [one]       ; < 1, == 1, or > 1
        fnstsw ax
        and ah,0x45
        cmp ah,0x45
        je  .L_13               ; jump if x is NaN

        cmp ah,0x40
        je  .L_14               ; jump if |x| == 1

        shl ah, 1
        xor ah, dl
        and edx, 2
        fld qword [inf_zero+edx+4]
	ret

align 4
.L_14:
        fld  qword [infinity]
        fmul qword [zero]        ; raise invalid exception
	ret

align 4
.L_13:
;        //fld 4(%esp)         // load x == NaN
	ret

align 4
;        // x is ±inf
.L_15:
        fstp st0               ; y
        test dh, 2
        jz   .L_16               ; jump if x == +inf

;  We must find out whether y is an odd integer.
        fld st                   ; y : y
        fistp qword [esp]        ; y
        fild qword [esp]        ; int(y) : y
        fucompp                  ; <empty>
        fnstsw ax
	sahf
        jne .L_17

; OK, the value is an integer, but is the number of bits small
; enough so that all are coming from the mantissa?
        pop   eax
        pop   edx
        and   al, 1
        jz   .L_18             ;// jump if not odd
        mov  eax, edx
        or   edx, eax
        jns  .L_155
        neg  eax
.L_155:
        cmp  eax, 0x00200000
        ja  .L_18             ;// does not fit in mantissa bits
; It's an odd integer.
        shr edx, 31
        fld qword [minf_mzero+edx+8]
	ret

align 4

.L_16:
        fcomp qword [zero]
        add   esp, 8
        fnstsw ax
        shr eax, 5
        and eax, 8
        fld qword [inf_zero+eax+1]
	ret

align 4
.L_17:
        shl edx, 30             ;// sign bit for y in right position
        add esp ,8
.L_18:
        shr edx, 31
        fld qword [inf_zero+edx+8]
	ret

align 4
                               ; x is ±0
.L_20:
        fstp    st0          ; y
        test    dl,2
        jz      .L_21          ; y > 0

 ;x is ±0 and y is < 0.  We must find out whether y is an odd integer.
        test   dh, 2
        jz     .L_25

        fld     st             ; y : y
        fistp qword [esp]     ; y
        fild  qword [esp]     ; int(y) : y
        fucompp                ; <empty>
        fnstsw  ax
	sahf
        jne     .L_26

   ;OK, the value is an integer, but is the number of bits small
   ;enough so that all are coming from the mantissa?

        pop    eax
        pop    edx
        and    al, 1
        jz     .L_27            ; jump if not odd
        cmp    edx,0xffe00000
        jbe    .L_27             ; does not fit in mantissa bits

; It's an odd integer.
; Raise divide-by-zero exception and get minus infinity value.

        fld1
        fdiv qword [zero]
	fchs
	ret

.L_25:
        fstp    st0
.L_26:
        add    esp,8
.L_27:

 ;Raise divide-by-zero exception and get infinity value.

        fld1
        fdiv qword [zero]
	ret

align 4

; x is ±0 and y is > 0.  We must find out whether y is an odd integer.

.L_21:
        test    dh,2
        jz      .L_22

        fld     st              ; y : y
        fistp qword [esp]       ; y
        fild  qword [esp]       ; int(y) : y
        fucompp                 ; <empty>
        fnstsw ax
	sahf
        jne    .L_23

; OK, the value is an integer, but is the number of bits small
; enough so that all are coming from the mantissa?

        pop    eax
        pop    edx
        and    al,1
        jz     .L_24             ; jump if not odd
        cmp    edx,0xffe00000
        jae    .L_24             ; does not fit in mantissa bits

 ; It's an odd integer.

               fld  qword [mzero]
	ret

.L_22:
        fstp    st0
.L_23:
        add     esp,8             ; Don't use 2 x pop
.L_24:
        fldz
	ret
endp

align 4

inf_zero:
infinity:
           db 0,0,0,0,0,0,0xf0,0x7f
zero:      dq 0.0
minf_mzero:
minfinity:
           db 0,0,0,0,0,0,0xf0,0xff
mzero:
           db 0,0,0,0,0,0,0,0x80
one:
           dq  1.0
limit:
           dq 0.29

