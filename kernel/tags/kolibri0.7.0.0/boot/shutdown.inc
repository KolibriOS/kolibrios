$Revision$
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
        call pause_key
        cmp  al,6
        jae  nbw
        mov  bl,al
nbw2:
        in   al,0x60
        call pause_key
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

        dec  ax    ; 1 = write floppy
        js   nbw
        jnz  no_floppy_write
        call floppy_write
        jmp  temp_3456 ;nbw
no_floppy_write:

        dec  ax    ; 2 = power off
        jnz  no_apm_off
        call APM_PowerOff
        jmp  $
no_apm_off:

        dec  ax    ; 3 = reboot
        jnz  restart_kernel        ; 4 = restart kernel
        push 0x40
        pop  ds
        mov  word[0x0072],0x1234
        jmp  0xF000:0xFFF0

pause_key:
        mov cx,100
pause_key_1:
        loop  pause_key_1
        ret

rdelay:
        ret

floppy_write:   ; write diskette image to physical floppy

        cmp  [flm],byte 1
        je   fwwritedone
        mov  [flm],byte 1

        xor    ax, ax        ; reset drive
        xor    dx, dx
        int     0x13

        mov     cx,0x0001               ; startcyl,startsector
        xor    dx, dx                    ; starthead,drive
        mov    ax, 80*2               ; read no of sect

fwwrites:
        push    ax

        ; move 1mb+ -> 0:a000

         pusha
        mov     si, fwmovedesc
        mov     cx,256*18
        mov     ah,0x87
        push    ds
        pop    es
        int     0x15
        add    dword [fwmovedesc+0x12], 512*18
        popa

        xor     si,si
        mov    es,si
fwnewwrite:
        mov     bx,0xa000               ; es:bx -> data area
        mov     ax,0x0300+18            ; read, no of sectors to read
        int     0x13

         test    ah, ah
        jz      fwgoodwrite

        inc    si
        cmp     si,10
        jnz     fwnewwrite

; can't access diskette - return
        pop    ax
        ret

fwgoodwrite:
        inc     dh
        cmp     dh,2
        jnz     fwbb2
        mov     dh,0
        inc     ch
fwbb2:
        pop     ax
        dec     ax
        jnz     fwwrites
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
fwwritedone:
        ret

restart_kernel:

        mov     ax,0x0003      ; set text mode for screen
        int     0x10
        jmp     0x4000:0000

restart_kernel_4000:
        cli

        push    ds
        pop    es
        mov    cx, 0x8000
        push    cx
        mov    ds, cx
        xor    si, si
        xor    di, di
        rep    movsw
        push    0x9000
        pop    ds
        push    0x2000
        pop    es
        pop    cx
        rep    movsw

        wbinvd  ; write and invalidate cache

        mov    al, 00110100b
        out    43h, al
        jcxz    $+2
        mov    al, 0xFF
        out    40h, al
        jcxz    $+2
        out    40h, al
        jcxz    $+2
        sti

; (hint by Black_mirror)
; We must read data from keyboard port,
; because there may be situation when previous keyboard interrupt is lost
; (due to return to real mode and IRQ reprogramming)
; and next interrupt will not be generated (as keyboard waits for handling)
        in      al, 0x60

; bootloader interface
        push    0x1000
        pop    ds
        mov    si, kernel_restart_bootblock
        mov    ax, 'KL'
        jmp     0x1000:0000


