$Revision: 431 $
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


system_shutdown:          ; shut down the system
    call stop_all_services

    push 3        ; stop playing cd
    pop  eax
    call sys_cd_audio
    cld

    mov  al,[0x2f0000+0x9030]
    cmp  al,1
    jl   no_shutdown_parameter
    cmp  al,4
    jle  yes_shutdown_param
 no_shutdown_parameter:

;    movzx ecx,word [0x2f0000+0x900A]
;    movzx esi,word [0x2f0000+0x900C]
;    imul ecx,esi  ;[0xfe04]
;;    mov  ecx,0x500000/4  ;3fff00/4    ; darken screen
;    push ecx
;    mov  esi,[0xfe80]
;    cmp  esi,32*0x100000
;    jbe  no_darken_screen
;    mov  edi,16*0x100000
;    push esi edi
;  sdnewpix:
;    lodsd
;    shr  eax,1
;    and  eax,0x7f7f7f7f
;    stosd
;    loop sdnewpix
;    pop  ecx
;    pop  esi edi
;    rep  movsd
;  no_darken_screen:

; read shutdown code:
; 1) display shutdown "window"

    mov  eax,[0xfe00]
    shr  eax,1
    lea  esi,[eax+220]        ; x end
    sub  eax,220        ; x start

    mov  ebx,[ScreenHeight]
    shr  ebx,1
    mov  [shutdownpos],ebx
    lea  ebp,[ebx+105]        ; y end
    sub  ebx,120        ; y start

    xor  edi,edi
    inc  edi        ; force putpixel & dtext
    mov  ecx,0x0000ff

; vertical loop begin
  sdnewpix1:
    push eax    ; save x start

; horizontal loop begin
  sdnewpix2:

    call [putpixel]

    inc  eax
    cmp  eax,esi
    jnz  sdnewpix2
; horizontal loop end

    dec  ecx    ; color
    pop  eax    ; restore x start

    inc  ebx    ; advance y pos
    cmp  ebx,ebp
    jnz  sdnewpix1
; vertical loop end

; 2) display text strings
;    a) version
    mov  eax,[0xfe00]
    shr  eax,1
    shl  eax,16
    mov  ax,word [shutdownpos]
    push eax
    sub  eax,(220-27)*10000h + 105
    mov  ebx,0xffff00
    mov  ecx,version
    push 34
    pop  edx
    call dtext

;    b) variants
    add  eax,105+33
    push 6
    pop  esi
;    mov  ebx,0xffffff
    mov  bl,0xFF
    mov  ecx,shutdowntext
    mov  dl,40
  newsdt:
    call dtext
    add  eax,10
    add  ecx,edx
    dec  esi
    jnz  newsdt

; 3) load & display rose.txt
    mov  eax,rosef-std_application_base_address          ; load rose.txt
    xor  ebx,ebx
    push 2
    pop  ecx
    mov  edx,0x90000
    push edx
    push 12
    pop  esi
    push edi    ; may be destroyed

    pushad
    push eax
    call file_system_lfn ; by SPraid  fileread
    pop eax
    popad
    pop  edi

    pop  ecx
    inc  ecx    ; do not display stars from rose.txt
    pop  eax
    add  eax,20*10000h - 110

    mov  ebx,0x00ff00
    push 27
    pop  edx

   nrl:
    call dtext
;    sub  ebx,0x050000
        ror     ebx, 16
        sub     bl, 0x05
        ror     ebx, 16
    add  eax,8
    add  ecx,31
    cmp  cx,word 0x0001+25*31
    jnz  nrl

    call checkVga_N13

 yes_shutdown_param:
    cli

    mov  eax,kernel               ; load kernel.mnt to 0x8000:0
    push 12
    pop  esi
    xor  ebx,ebx
    or   ecx,-1
    mov  edx,0x80000
    call fileread

    mov  esi,restart_kernel_4000+0x10000  ; move kernel re-starter to 0x4000:0
    mov  edi,0x40000
    mov  ecx,1000
    rep  movsb

    mov  eax,0x2F0000    ; restore 0x0 - 0xffff
    xor  ebx,ebx
    mov  ecx,0x10000
    call memmove

    call restorefatchain

    mov al, 0xFF
    out 0x21, al
    out 0xA1, al

    mov  word [0x467+0],pr_mode_exit-0x10000
    mov  word [0x467+2],0x1000

    mov  al,0x0F
    out  0x70,al
    mov  al,0x05
    out  0x71,al

    mov  al,0xFE
    out  0x64,al
    hlt

use16

pr_mode_exit:
org $-0x10000

; setup stack
    mov    ax, 3000h
    mov    ss, ax
    mov    esp, 0EC00h
; setup ds
    push    cs
    pop    ds

    lidt [old_ints_h-0x10000]
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

iglobal
  kernel  db  'KERNEL  MNT'
;  shutdown_parameter db 0
endg

restart_kernel:

        mov     ax,0x0003      ; set text mode for screen
        int     0x10

        jmp     0x4000:0000


restart_kernel_4000:
        cli

;        mov     di,0x1000      ; load kernel image from 0x8000:0 -> 0x1000:0
;
;      new_kernel_block_move:
;
;        mov     ebx,0
;
;      new_kernel_byte_move:
;
;        mov     ax,di
;        add     ax,0x7000
;        mov     es,ax
;        mov     dl,[es:bx]
;        mov     es,di
;        mov     [es:bx],dl
;
;        inc     ebx
;        cmp     ebx,65536
;        jbe     new_kernel_byte_move
;
;        add     di,0x1000
;        cmp     di,0x2000
;        jbe     new_kernel_block_move
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

;        mov     ax,0x1000
;        mov     es,ax
;        mov     ax,0x3000
;        mov     ss,ax
;        mov     sp,0xec00
; restore timer
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
    mov    si, kernel_restart_bootblock-0x10000
    mov    ax, 'KL'
        jmp     0x1000:0000

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
org $+0x10000
flm db 0
org $-0x10000

floppy_write:   ; write diskette image to physical floppy

        cmp  [flm-0x10000],byte 1
        je   fwwritedone
        mov  [flm-0x10000],byte 1

    xor    ax, ax        ; reset drive
    xor    dx, dx
        int     0x13

        mov     cx,0x0001               ; startcyl,startsector
;        mov     dx,0x0000               ; starthead,drive
    xor    dx, dx
        mov    ax, 80*2               ; read no of sect

      fwwrites:
    push    ax

        ; move 1mb+ -> 0:a000

    pusha
        mov     si,fwmovedesc -0x10000
        mov     cx,256*18
        mov     ah,0x87
        push    ds
        pop    es
        int     0x15
        add    dword [fwmovedesc-0x10000+0x12], 512*18
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
org $+0x10000
      fwmovedesc:
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0xff,0xff,0x0,0x00,0x10,0x93,0x0,0x0
        db      0xff,0xff,0x0,0xa0,0x00,0x93,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
org $-0x10000
use32
org $+0x10000
uglobal
  shutdownpos dd 0x0
endg

iglobal
if lang eq en
shutdowntext:
    db   "IT'S SAFE TO POWER OFF COMPUTER OR      "
    db   '                                        '
    db   '1) SAVE RAMDISK TO FLOPPY               '
    db   '2) APM - POWEROFF                       '
    db   '3) REBOOT                               '
    db   '4) RESTART KERNEL                       '
else if lang eq ru
shutdowntext:
    db   "Безопасное выключение компьютера или    "
    db   '                                        '
    db   '1) Сохранить рамдиск на дискету         '
    db   '2) APM - выключение питания             '
    db   '3) Перезагрузка системы                 '
    db   '4) Рестарт ядра из ОЗУ                  '
else
shutdowntext:
    db   "SIE KOENNEN DEN COMPUTER NUN AUSSCHALTEN"
    db   '                                        '
    db   '1) RAMDISK AUF DISK SPEICHERN           '
    db   '2) APM - AUSSCHALTEN                    '
    db   '3) NEUSTARTEN                           '
    db   '4) KERNEL NEU STARTEN                   '
end if
rosef:
    dd 0,0,0,1024,0x90000-std_application_base_address
    db '/rd/1/ROSE.TXT',0
endg
