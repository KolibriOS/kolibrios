;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2011. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; All parsers are called with ds:si -> value of the variable,
; possibly with spaces before, and dx = limit of config file.

; Three subroutines parse_char, parse_number and parse_bool set CF
; if something has failed, otherwise return the value in al/ax.

parse_timeout:
; timeout is a number not greater than 9
        call    parse_number
        jc      .nothing
        cmp     ax, 9
        jbe     @f
        mov     ax, 9
@@:
        imul    ax, 18
        mov     [es:preboot_timeout], ax
.nothing:
        ret

parse_resolution:
; resolution is <width>*<height>, 'x' can be used instead of '*'
; parse width
        call    parse_number
        jc      .nothing
; save width
        xchg    ax, bx
; test for 'x' or '*'
        call    parse_char
        cmp     al, 'x'
        jz      @f
        cmp     al, '*'
        jnz     .nothing
@@:
; parse height
        call    parse_number
        jc      .nothing
; write width and height
        mov     [es:x_save], bx
        mov     [es:y_save], ax
.nothing:
        ret

parse_vbemode:
; vbemode is a number
        call    parse_number
        jc      .nothing
        mov     [es:number_vm], ax
.nothing:
        ret

;parse_vrr:
;; vrr is a boolean setting
;        call    parse_bool
;        jc      .nothing
;; convert 0 to 2, 1 to 1
;        inc     ax
;        xor     al, 3
;        mov     [es:preboot_vrrm], al
;.nothing:
;        ret

parse_biosdisks:
; using biosdisks is a boolean setting
        call    parse_bool
        jc      .nothing
; convert 0 to 2, 1 to 1
        inc     ax
        xor     al, 3
        mov     [es:preboot_biosdisk], al
.nothing:
        ret

parse_imgfrom:
; boot device (1-floppy 2-kolibri.img using primary loader)
        call    parse_number
        jc      .nothing
        cmp     al, 1
        jb      .nothing
        cmp     al, 2
        ja      .nothing
        mov     [es:preboot_device], al
.nothing:
        ret

parse_char:
; skip spaces and return the next character or CF if EOF.
        cmp     si, dx
        jae     .eof
        lodsb
        cmp     al, ' '
        jbe     parse_char
        ret
.eof:
        stc
        ret

parse_number:
; initialize high part of ax to zero
        xor     ax, ax
; skip spaces
        call    parse_char
        jc      .bad
; al should be a digit
        sub     al, '0'
        cmp     al, 9
        ja      .bad
; accumulate the value in cx
        xchg    cx, ax
@@:
        cmp     si, dx
        jae     .eof
        lodsb
        sub     al, '0'
        cmp     al, 9
        ja      .end
        imul    cx, 10
        add     cx, ax
        jmp     @b
; if the end is caused by non-digit, unwind the last character
.end:
        dec     si
.eof:
        xchg    cx, ax
        clc
        ret
.bad:
        stc
        ret

parse_bool:
; skip spaces
        call    parse_char
        jc      .bad
; Boolean false can be represented as 0=no=off,
; boolean true can be represented as 1=yes=on.
        cmp     al, '0'
        jz      .false
        cmp     al, '1'
        jz      .true
        mov     ah, al
        cmp     si, dx
        jae     .bad
        lodsb
        cmp     ax, 'n'*256 + 'o'
        jz      .false
        cmp     ax, 'o'*256 + 'f'
        jz      .false
        cmp     ax, 'y'*256 + 'e'
        jz      .true
        cmp     ax, 'o'*256 + 'n'
        jz      .true
.bad:
        stc
        ret
.true:
        xor     ax, ax
        inc     ax
        ret
.false:
        xor     ax, ax
        ret
