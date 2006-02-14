;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Shutdown for Menuet
;;
;;  Distributed under General Public License
;;  See file COPYING for details.
;;  Copyright 2003 Ville Turjanmaa
;;


system_shutdown:          ; shut down the system

    mov  eax,3            ; stop playing cd
    call sys_cd_audio
    cli
    cld

    mov  al,[0x2f0000+0x9030]
    cmp  al,1
    jl   no_shutdown_parameter
    cmp  al,4
    jg   no_shutdown_parameter
    jmp  yes_shutdown_param
 no_shutdown_parameter:

    movzx ecx,word [0x2f0000+0x900A]
    movzx esi,word [0x2f0000+0x900C]
    imul ecx,esi  ;[0xfe04]
;    mov  ecx,0x500000/4  ;3fff00/4    ; darken screen
    push ecx
    mov  esi,[0xfe80]
    cmp  esi,32*0x100000
    jbe  no_darken_screen
    mov  edi,16*0x100000
  sdnewpix:
    mov  eax,[esi]
    add  esi,4
    shr  eax,1
    and  eax,0x7f7f7f7f
    stosd
    loop sdnewpix
    pop  ecx
    mov  esi,16*0x100000
    mov  edi,[0xfe80]
    cld
    rep  movsd
  no_darken_screen:

    mov  eax,[0xfe00]
    shr  eax,1
    sub  eax,220

    mov  ebx,[0xfe04]
    shr  ebx,1
    mov  [shutdownpos],ebx
    sub  ebx,120

    mov  edi,1
    mov  ecx,0x0000ff

  sdnewpix2:

    call [putpixel]

    inc  eax
    mov  esi,[0xfe00]
    shr  esi,1
    add  esi,220
    cmp  eax,esi
    jnz  sdnewpix2

    dec  ecx

    mov  eax,[0xfe00]
    shr  eax,1
    sub  eax,220

    inc  ebx

    mov  edx,[shutdownpos]
    add  edx,105
    cmp  ebx,edx
    jnz  sdnewpix2


    mov  esi,[0xfe00]      ; menuet version
    shr  esi,1
    sub  esi,220
    add  esi,27
    shl  esi,16
    mov  eax,esi
    add  eax,[shutdownpos]
    sub  eax,105
    mov  ebx,0xffff00
    mov  ecx,version
    mov  edx,34
    mov  edi,1
    call dtext

    mov  esi,[0xfe00]      ; 'it is safe..'
    shr  esi,1
    sub  esi,220
    add  esi,27
    shl  esi,16
    mov  eax,esi
    add  eax,[shutdownpos]
    add  eax,33
    mov  esi,6
    mov  ebx,0xffffff
    mov  ecx,shutdowntext
    mov  edx,40
    mov  edi,1
  newsdt:
    call dtext
    add  eax,10
    add  ecx,40
    dec  esi
    jnz  newsdt

    mov  eax,rosef          ; load rose.txt
    mov  ebx,0
    mov  ecx,16800
    mov  edx,0x90000
    mov  esi,12
    call fileread

    mov  esi,[0xfe00]       ; draw rose
    shr  esi,1
    add  esi,20
    shl  esi,16
    mov  eax,esi
    add  eax,[shutdownpos]
    sub  eax,110

    mov  ebx,0x00ff00
    mov  ecx,0x90001
    mov  edx,27
    mov  edi,1

   nrl:
    call dtext
    sub  ebx,0x050000
    add  eax,8
    add  ecx,31
    cmp  ecx,dword 0x90001+25*31
    jnz  nrl

    call checkEgaCga

 yes_shutdown_param:

    cli

    mov  eax,kernel               ; load kernel.mnt to 0x8000:0
    mov  esi,12
    mov  ebx,0
    mov  ecx,-1
    mov  edx,0x80000
    call fileread

    mov  esi,restart_kernel_4000  ; move kernel re-starter to 0x4000:0
    mov  edi,0x40000
    mov  ecx,1000
    cld
    rep  movsb

    mov  eax,0x2F0000    ; restore 0x0 - 0xffff
    mov  ebx,0x0000
    mov  ecx,0xffff
    call memmove

    call restorefatchain

    mov  eax,pr_mode_exit
    mov  [0x467+0],ax
    mov  [0x467+2],word 0x1000

    mov  al,0x0F
    out  0x70,al
    mov  al,0x05
    out  0x71,al

    mov  al,0xFE
    out  0x64,al
    hlt

use16

pr_mode_exit:

; setup stack
	mov	ax, 3000h
	mov	ss, ax
	mov	esp, 0EC00h
; setup ds
	push	cs
	pop	ds
;    mov ax,1000
;    mov ds,ax
;    mov es,ax
;    mov fs,ax
;    mov gs,ax
;    mov ss,ax
;    mov bl,[shutdown_parameter]
;    mov [es:shutdown_parameter-0x10000],bl

    jmp real_mode-0x10000       

old_ints_h:
        dw      4*0x20
        dd      0
        dw      0

rdelay:
    ret
real_mode:

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
    
    mov  al,0
    out  0x21,al
    call rdelay
    out  0xA1,al
    sti

    jmp  temp_3456

  nbw:
    xor  ax,ax
    in   al,0x60
    call pause_key
    cmp  al,7
    jge  nbw
    mov  bl,al
  nbw2:
    in   al,0x60
    call pause_key
    cmp  al,bl
    je   nbw2
    cmp  al,240  ;ax,240
    jne  nbw31
    mov  al,bl
    dec  al
    jmp  nbw32
  nbw31:
    add  bl,128
    cmp  al,bl
    jne  nbw
    sub  al,129

  nbw32:
    
    cmp  al,1              ; write floppy
    jne  no_floppy_write
    call floppy_write
    jmp  temp_3456 ;nbw
  no_floppy_write:

    cmp  al,2              ; poweroff
    jne  no_apm_off
    call APM_PowerOff
  no_apm_off:

    cmp  al,3              ; boot 
    jnz  no_sys_boot 
    mov  ax,0x0040 
    mov  ds,ax 
    mov  word[0x0072],0x1234 
    jmp  0xF000:0xFFF0
  no_sys_boot:

    cmp  al,4              ; restart kernel
    je   restart_kernel

  temp_3456:
    push    word 0x0000
    pop     es
    mov  al,byte [es:0x9030]
    cmp  al,1
    jl   nbw
    cmp  al,4
    jg   nbw
    jmp  nbw32
  
;    jmp  nbw
    pause_key:
      mov ecx,100
    pause_key_1:
      loop  pause_key_1
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
	push	ds
	pop	es
	mov	cx, 0x8000
	push	cx
	mov	ds, cx
	xor	si, si
	xor	di, di
	rep	movsw
	mov	ax, 0x9000
	mov	ds, ax
	mov	ax, 0x2000
	mov	es, ax
	pop	cx
	rep	movsw

        wbinvd  ; write and invalidate cache

;        mov     ax,0x1000
;        mov     es,ax
;        mov     ax,0x3000
;        mov     ss,ax
;        mov     sp,0xec00
; restore timer
	mov	al, 00110100b
	out	43h, al
	jcxz	$+2
	mov	al, 0xFF
	out	40h, al
	jcxz	$+2
	out	40h, al
	jcxz	$+2
        sti

; bootloader interface
	push	0x1000
	pop	ds
	mov	si, .bootloader_block-0x10000
	mov	ax, 'KL'
        jmp     0x1000:0000

.bootloader_block:
	db	1	; version
	dw	1	; floppy image is in memory
	dd	0	; cannot save parameters

APM_PowerOff:
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
uglobal
  flm db 0
endg

floppy_write:   ; write diskette image to physical floppy

        pusha

        mov  ax,0x1000
        mov  es,ax
        cmp  [es:flm-0x10000],byte 1
        je   fwwritedone
        mov  [es:flm-0x10000],byte 1

        mov     ax,0x0000               ; reset drive
        mov     dx,0x0000
        int     0x13

        mov     cx,0x0001               ; startcyl,startsector
        mov     dx,0x0000               ; starthead,drive
        push    word 80*2               ; read no of sect

      fwwrites:
        pusha

        ; move 1mb+ -> 0:a000

        pusha
        mov     si,fwmovedesc -0x10000
        push    word 0x1000
        pop     es
        mov     cx,256*18
        mov     ah,0x87
        int     0x15
        mov     eax,[es:fwmovedesc-0x10000+0x12]
        add     eax,512*18
        mov     [es:fwmovedesc-0x10000+0x12],eax
        popa

        xor     si,si
      fwnewwrite:
        push    word 0x0
        pop     es
        mov     bx,0xa000               ; es:bx -> data area
        mov     ax,0x0300+18            ; read, no of sectors to read
        int     0x13

        cmp     ah,0
        jz      fwgoodwrite

        add     si,1
        cmp     si,10
        jnz     fwnewwrite

        add     esp,32+2

        popa                            ; can't access diskette
        ret

      fwgoodwrite:

        popa

        inc     dh
        cmp     dh,2
        jnz     fwbb2
        mov     dh,0
        inc     ch

      fwbb2:

        cld
        pop     ax
        dec     ax
        push    ax
        cmp     ax,0
        jnz     fwrs

        pop     ax

        jmp     fwwritedone
      fwrs:
        jmp     fwwrites

      fwmovedesc:

        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0xff,0xff,0x0,0x00,0x10,0x93,0x0,0x0
        db      0xff,0xff,0x0,0xa0,0x00,0x93,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

      fwwritedone:

        popa

        ret


use32

uglobal
  shutdownpos dd 0x0
endg

iglobal
;shutdowntext:
;    db   "IT'S SAFE TO POWER OFF COMPUTER OR      "
;    db   '                                        '
;    db   '1) SAVE RAMDISK TO FLOPPY               '
;    db   '2) APM - POWEROFF                       '
;    db   '3) REBOOT                               '
;    db   '4) RESTART KERNEL                       ' 
shutdowntext:
    db   "Безопасное выключение компьютера или    "
    db   '                                        '
    db   '1) Сохранить рамдиск на дискету         '
    db   '2) APM - выключение питания             '
    db   '3) Перезагрузка системы                 '
    db   '4) Рестарт ядра из ОЗУ                  '
rosef:
    db 'ROSE    TXT'
endg
