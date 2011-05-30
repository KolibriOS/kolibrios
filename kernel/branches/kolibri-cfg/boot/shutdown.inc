;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  Shutdown for Menuet                                         ;;
;;                                                              ;;
;;  Distributed under General Public License                    ;;
;;  See file COPYING for details.                               ;;
;;  Copyright 2003 Ville Turjanmaa                              ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


align 4
pr_mode_exit:

; setup stack
        mov    ax, 0x3000
        mov    ss, ax
        mov    esp, 0x0EC00
; setup ds
        push    cs
        pop    ds

        lidt [old_ints_h]
;remap IRQs
        mov  al,0x11
        out  0x20,al
        call rdelay
        out  0xA0,al
        call rdelay

        mov  al,0x08
        out  0x21,al
        call rdelay
        mov  al,0x70
        out  0xA1,al
        call rdelay

        mov  al,0x04
        out  0x21,al
        call rdelay
        mov  al,0x02
        out  0xA1,al
        call rdelay

        mov  al,0x01
        out  0x21,al
        call rdelay
        out  0xA1,al
        call rdelay

        mov  al,0xB8
        out  0x21,al
        call rdelay
        mov  al,0xBD
        out  0xA1,al
        sti

temp_3456:
        xor  ax,ax
        mov  es,ax
        mov  al,byte [es:0x9030]
        cmp  al,1
        jl   nbw
        cmp  al,4
        jle  nbw32

nbw:
        in   al,0x60
        cmp  al,6
        jae  nbw
        mov  bl,al
nbw2:
        in   al,0x60
        cmp  al,bl
        je   nbw2
        cmp  al,240  ;ax,240
        jne  nbw31
        mov  al,bl
        dec  ax
        jmp  nbw32
nbw31:
        add  bl,128
        cmp  al,bl
        jne  nbw
        sub  al,129

nbw32:

        dec  ax
        dec  ax    ; 2 = power off
        jnz  no_apm_off
        call APM_PowerOff
        jmp  $
no_apm_off:

        ; 3 = reboot, 4 = obsolete restart kernel
        push 0x40
        pop  ds
        mov  word[0x0072],0x1234
        jmp  0xF000:0xFFF0


rdelay:
        ret

APM_PowerOff:
        mov     ax, 5304h
        xor     bx, bx
        int     15h
;!!!!!!!!!!!!!!!!!!!!!!!!
        mov ax,0x5300
        xor bx,bx
        int 0x15
        push ax

        mov ax,0x5301
        xor bx,bx
        int 0x15

        mov ax,0x5308
        mov bx,1
        mov cx,bx
        int 0x15

        mov ax,0x530E
        xor bx,bx
        pop cx
        int 0x15

        mov ax,0x530D
        mov bx,1
        mov cx,bx
        int 0x15

        mov ax,0x530F
        mov bx,1
        mov cx,bx
        int 0x15

        mov ax,0x5307
        mov bx,1
        mov cx,3
        int 0x15
;!!!!!!!!!!!!!!!!!!!!!!!!
        ret
