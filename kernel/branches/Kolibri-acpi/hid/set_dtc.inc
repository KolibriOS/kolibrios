;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;setting date,time,clock and alarm-clock
;add sys_settime at servetable as for ex. 22 fcn:
; 22 - SETTING DATE TIME, CLOCK AND ALARM-CLOCK
; ebx =0 - set time ecx - 00SSMMHH
; ebx =1 - set date ecx=00DDMMYY
; ebx =2 - set day of week ecx- 1-7
; ebx =3 - set alarm-clock ecx - 00SSMMHH
; out: 0 -Ok 1 -wrong format 2 -battery low
sys_settime:
      
        cli
        mov     al, 0x0d
        out     0x70, al
        in      al, 0x71
        bt      ax, 7
        jnc     bat_low
        cmp     ebx, 2;day of week
        jne     nosetweek
        test    ecx, ecx   ;test day of week
        je      wrongtime
        cmp     ecx, 7
        ja      wrongtime
        mov     edx, 0x70
        call    startstopclk
        dec     edx
        mov     al, 6
        out     dx, al
        inc     edx
        mov     al, cl
        out     dx, al
        jmp     endsettime
  nosetweek:           ;set date
        cmp     ebx, 1
        jne     nosetdate
        cmp     cl, 0x99;test year
        ja      wrongtime
        shl     ecx, 4
        cmp     cl, 0x90
        ja      wrongtime
        cmp     ch, 0x99;test month
        ja      wrongtime
        shr     ecx, 4
        test    ch, ch
        je      wrongtime
        cmp     ch, 0x12
        ja      wrongtime
        shl     ecx, 8
        bswap   ecx   ;ebx=00YYMMDD
        test    cl, cl ;test day
        je      wrongtime
        shl     ecx, 4
        cmp     cl, 0x90
        ja      wrongtime
        shr     ecx, 4
        cmp     ch, 2 ;February
        jne     testday
        cmp     cl, 0x29
        ja      wrongtime
        jmp     setdate
    testday:
        cmp     ch, 8
        jb      testday1;Aug-Dec
        bt      cx, 8
        jnc     days31
        jmp     days30
    testday1:
        bt      cx, 8 ;Jan-Jul ex.Feb
        jnc     days30
    days31:
        cmp     cl, 0x31
        ja      wrongtime
        jmp     setdate
    days30:
        cmp     cl, 0x30
        ja      wrongtime
    setdate:
        mov     edx, 0x70
        call    startstopclk
        dec     edx
        mov     al, 7    ;set days
        out     dx, al
        inc     edx
        mov     al, cl
        out     dx, al
        dec     edx
        mov     al, 8    ;set months
        out     dx, al
        inc     edx
        mov     al, ch
        out     dx, al
        dec     edx
        mov     al, 9    ;set years
        out     dx, al
        inc     edx
        shr     ecx, 8
        mov     al, ch
        out     dx, al
        jmp     endsettime
    nosetdate:       ;set time or alarm-clock
        cmp     ebx, 3
        ja      wrongtime
        cmp     cl, 0x23
        ja      wrongtime
        cmp     ch, 0x59
        ja      wrongtime
        shl     ecx, 4
        cmp     cl, 0x90
        ja      wrongtime
        cmp     ch, 0x92
        ja      wrongtime
        shl     ecx, 4
        bswap   ecx    ;00HHMMSS
        cmp     cl, 0x59
        ja      wrongtime
        shl     ecx, 4
        cmp     cl, 0x90
        ja      wrongtime
        shr     ecx, 4

        mov     edx, 0x70
        call    startstopclk
        dec     edx
        cmp     ebx, 3
      
        je      setalarm
        xor     eax, eax;al=0-set seconds
        out     dx, al
        inc     edx
        mov     al, cl
        out     dx, al
        dec     edx
        mov     al, 2 ;set minutes
        out     dx, al
        inc     edx
        mov     al, ch
        out     dx, al
        dec     edx
        mov     al, 4 ;set hours
        out     dx, al
        inc     edx
        shr     ecx, 8
        mov     al, ch
        out     dx, al
        jmp     endsettime
   setalarm:
        mov     al, 1;set seconds for al.
        out     dx, al
        inc     edx
        mov     al, cl
        out     dx, al
        dec     edx
        mov     al, 3;set minutes for al.
        out     dx, al
        inc     edx
        mov     al, ch
        out     dx, al
        dec     edx
        mov     al, 5;set hours for al.
        out     dx, al
        inc     edx
        shr     ecx, 8
        mov     al, ch
        out     dx, al
        dec     edx
        mov     al, 0x0b;enable irq's
        out     dx, al
        inc     dx
        in      al, dx
        bts     ax, 5;set bit 5
        out     dx, al
    endsettime:
        dec     edx
        call    startstopclk
        sti
        and     [esp+36-4], dword 0
        ret
   bat_low:
        sti
        mov     [esp+36-4], dword 2
        ret
   wrongtime:
        sti
        mov     [esp+36-4], dword 1
        ret

startstopclk:
        mov     al, 0x0b
        out     dx, al
        inc     dx
        in      al, dx
        btc     ax, 7
        out     dx, al
        ret
