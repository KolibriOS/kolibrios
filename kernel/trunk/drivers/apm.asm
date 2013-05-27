;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2009-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; 11.09.2009 staper@inbox.ru
; see kernel\docs\apm.txt

use32

        org     0x0

        db      'MENUET01'
        dd      0x1
        dd      START
        dd      I_END
        dd      (I_END+100) and not 3
        dd      (I_END+100) and not 3
        dd      0x0,0x0

include 'macros.inc'

START:
        mcall   40,0x7

        mcall   49,0x0001,0x0001,0x5308 ;CX = FFFFh APM v1.0
;        mcall   49,0x0001,0x0001,0x530d
;        mcall   49,0x0001,0x0001,0x530f

;        mcall   49,0x0000,,0x5310      ;bl - number of batteries
redraw:
        mcall   49,0x0000,,0x530c
        dec     cl
        jz      still
        mcall   49,0x0001,0x0001,0x5308
        mcall   49,0x01ff,,0x530c
        test    cl, cl
        jz      @f
        mcall   49,0x0000,0x0001,0x530d
        mcall   49,0x0000,0x0000,0x5307
        mcall   49,0x0000,0x0001,0x5308
  @@:
        mcall   12,1
        mcall   0,100*65536+235,100*65536+90,0x34ffffff,0x000000,title
        mcall   49,0x0000,,0x5300
        jnc     @f
        mcall   4,10*65536+3,0x80000000,text.4
        bts     [flags], 1
        jmp     .end
    @@:
        cmp     al, 0
        jne     @f
        mov     edx, text.1
        jmp     .0
    @@:
        cmp     al, 1
        jne     @f
        mov     edx, text.2
        jmp     .0
    @@:
        mov     edx, text.3
    .0:
        push    edx
        mcall   4,169*65536+3,0x80dddddd,text.0
        pop     edx
        add     ebx, 47*65536
        mcall
        mcall   49,0x0001,,0x530a
        jc      .error
        push    si dx cx bx     ;time of battery life, b. flag, b. status, AC line status

                                ;AC line status
        cmp     bh, 0
        jne     @f
        mov     edx, text.01
        jmp     .1
    @@:
        cmp     bh, 1
        jne     @f
        mov     edx, text.02
        jmp     .1
    @@:
        cmp     bh, 2
        jne     @f
        mov     edx, text.03
        jmp     .1
    @@:
        mov     edx, text.04
   .1:
        push    edx
        mcall   4,10*65536+10,0x80000000,text.00
        pop     edx
        mcall   ,100*65536+10,;0x80000000

                                ;battery status
        pop     bx
        cmp     bl, 0
        jne     @f
        mov     edx, text.11
        jmp     .2
    @@:
        cmp     bl, 1
        jne     @f
        mov     edx, text.12
        jmp     .2
    @@:
        cmp     bl, 2
        jne     @f
        mov     edx, text.13
        jmp     .2
    @@:
        cmp     bl, 3
        jne     @f
        mov     edx, text.14
        jmp     .2
    @@:
        mov     edx, text.04
   .2:
        push    edx
        mcall   4,10*65536+20,0x80000000,text.10
        pop     edx
        mcall   ,100*65536+20,

                                ;battery life, percentage and minutes/seconds
        mcall   ,10*65536+30,,text.20
        pop     cx
        cmp     cl, 0xff
        jne     @f
        mcall   ,100*65536+30,0x80000000,text.04
        pop     eax
        jmp     .end
    @@:
        shl     ecx, 24
        shr     ecx, 24
        mcall   47,0x80030000,,100*65536+30,0x347636
    .3:
        mcall   4,115*65536+30,0x80000000,text.15
        mov     dx, [esp]
        shl     edx, 17
        shr     edx, 17
        mov     ecx, edx
        mcall   47,0x80030000,,140*65536+30
        pop     cx
        mov     edx, text.21
        bt      cx, 15
        jc      @f
        mov     edx, text.22
    @@:
        mcall   4,160*65536+30,0x80000000
        pop     si
  .error:
  .end:
        ;buttons
        mcall   8,148*65536+16,45*65536+15,3,0x00677ab0
        mcall   ,166*65536+16,,4,
        mcall   ,184*65536+16,,5,
        mcall   ,202*65536+16,,6,
        bt      [flags], 1
        jc      @f
        mcall   ,65*65536+45,,2,
  @@:
        mcall   4,10*65536+50,0x80564242,text.30
        mcall   12,2

still:
;        mcall   10
        mcall   23,12000
        test    eax, eax
        jz      redraw

        dec     al
        jz      redraw
        dec     al
        jz      key
        dec     al
        jz      button
        jmp     still




key:
        mcall   2
        jmp     still

button:
        mcall   17
        cmp     ah, 1
        jne     @f
        mcall   -1

  @@:
        cmp     ah, 2
        jne     @f
        mcall   5,50
        mcall   49,0x0001,0x0001,0x5307
        jmp     redraw

  @@:
        cmp     ah, 4
        jg      @f
        mov     edx, 0x01f7      ;primary chan.
        call    reserv_ports
        jc      redraw
        sub     bh, 3
  .1:
        call    set_drive
        btc     [flags], 2
        jnc     .2
        call    device_reset
        jmp     .3
  .2:
        call    standby_hdd
  .3:
        call    free_ports
        jmp     redraw

  @@:
        cmp     ah, 6
        jg      redraw
        mov     edx, 0x0177      ;secondary chan.
        call    reserv_ports
        jc      redraw
        sub     bh, 5
        jmp     .1

set_drive:
        dec     dx
        in      al, dx
        test    bh, bh
        jnz     @f
        btr     ax, 4
.1:
        out     dx, al
        inc     dx
        ret
@@:
        bts     ax, 4
        jmp     .1


standby_hdd:
; 94h E0h nondata       standby immediate
; 95h E1h nondata       idle immediate
; 96h E2h nondata       standby
; 97h E3h nondata       idle
; 98h E5h nondata       check power mode
; 99h E6h nondata       set sleep mode
        xor     ecx, ecx
    @@:
        in      al, dx
        dec     cx
        jz      @f
        bt      ax, 6
        jnc     @b
        mov     al, 0x96
        out     dx, al
        mov     al, 0xe2
        out     dx, al
@@:
        ret

reserv_ports:
        mov     ecx, edx
        dec     ecx
        push    ax
        mcall   46,0
        test    al, al
        jnz     @f
        pop     bx
        clc
        ret
@@:
        pop     bx
        stc
        ret

device_reset:
        xor     ecx, ecx
    @@:
        in      al, dx
        dec     cx
        jz      @f
        bt      ax, 6
        jnc     @b
        mov     al, 0x10
        out     dx, al
@@:
        ret

free_ports:
        mov     ecx, edx
        dec     ecx
        mcall   46,1
        ret


; ДАННЫЕ ПРОГРАММЫ
title db '',0
flags dw 0

text:
.0:
    db 'APM v.1.',0
.1:
    db '0',0
.2:
    db '1',0
.3:
    db '2',0
.4:
    db 'APM not supported',0

.00:
     db 'power status:',0
.01:
     db 'off-line',0
.02:
     db 'on-line',0
.03:
     db 'on backup power',0
.04:
     db 'unknown',0

.10:
     db 'battery flag:',0
.11:
     db 'high',0
.12:
     db 'low',0
.13:
     db 'critical',0
.14:
     db 'charging',0
.15:
     db ' % ,',0

.20:
     db 'battery life:',0
.21:
     db 'min',0
.22:
     db 'sec',0

.30:
     db 'STAND-BY: SYSTEM  HDD:  0  1  2  3',0

I_END:
