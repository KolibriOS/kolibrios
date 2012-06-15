;
; MenuesOS Boot Loader
;
; Author: Trans
; Date:   14.02.03
; Version: 1.0
; 
; Current Version: 1.1
; Date of modification: 11.06.03
;
; Compile with FASM
;

;--------Code------------
    org 100h
start:
    push cs
    pop ds
    mov ax,0003h
    int 10h

    mov dx,title
    call print
    mov dx,mes1
    call print

    mov dx,img0
    call file_open
    test ax,ax
    jnz loader_continue
    mov dx,img2
    call file_open
    test ax,ax
    jne loader_continue
    mov dx,img3
    call file_open
    test ax,ax
    je loader_not_find_file
loader_continue:
    mov [handle],ax

;---------Read boot-sector-----------
    mov bx,ax
    mov dx,buffer
    mov cx,512
    call file_read

;--------Read parametrs--------------
    mov ax,[buffer+0bh]
    mov [SectSize],ax
    mov al,[buffer+0dh]
    mov [ClustSect],al
    mov ax,[buffer+0eh]
    mov [ResSect],ax
    add ax,[ResRgn]
    mov [FATRgn],ax
    mov al,[buffer+10h]
    mov [FATCnt],al
    xor bx,bx
    mov bl,al
    mov ax,[buffer+11h]
    mov [RootEnt],ax
    shr ax,4        ; ax=ax*32/512
    mov [RootDirRgnSz],ax
    mov ax,[buffer+16h]
    mov [FATSect],ax
    mul bx
    mov [FATRgnSz],ax
    add ax,[FATRgn]
    mov [RootDirRgn],ax
    add ax,[RootDirRgnSz]
    mov [DataRgn],ax

;------Read FATs and RootDir---------
    xor eax,eax
    xor edx,edx
    mov ax,[FATRgn]
    mul [SectSize]
    mov cx,dx
    mov dx,ax
    mov bx,[handle]
    call file_offset
    mov ax,[FATRgnSz]
    mul [SectSize]
    mov cx,ax
    mov dx,buffer
    mov bx,[handle]
    call file_read
    mov cx,[RootEnt]
    shl cx,5
    mov dx,Root
    mov bx,[handle]
    call file_read

;-------Search entry pointer in RootDir---------
    push ds
    pop es
    mov cx,[RootEnt]
    mov bx,Root
loader_loc_00:
    push cx
    mov cx,11
    mov di,bx
    mov si,kernel
    repe cmpsb
    jnz loader_notfound
    pop cx
    jmp loader_find
loader_notfound:
    pop cx
    add bx,32
    loop loader_loc_00
loader_find:

    mov ax,[bx+1ah]
    mov [FirstClust],ax
    mov eax,[bx+1ch]
    mov [filesize],eax

;------Read Kernel----------------------
    call read_kernel

;---------------------------------------

    mov bx,[handle]
    call file_close
;;;;;;;;;;;;;;;;;;;;;;
    jmp loader_yes
;;;;;;;;;;;;;;;;;;;;;;
;    mov dx,mes2
;    call print
;loader_key:
;    mov ah,00h
;    int 16h
;    cmp al,1bh    ;ESC
;    je loader_no
;    cmp al,6eh    ;'n'
;    je loader_no
;    cmp al,4eh    ;'N'
;    je loader_no
;    cmp al,79h    ;'y'
;    je loader_yes
;    cmp al,59h    ;'Y'
;    je loader_yes
;    jmp loader_key

loader_not_find_file:
    mov dx,mes4
    call print
    jmp loader_exit

loader_yes:
    mov dx,yes
    call print
    mov ax,7000h
    mov es,ax
    mov si,move_kernel
    xor di,di
    mov cx,len_mk
    rep movsb
    jmp far 7000h:0000h
;    jmp loader_exit
loader_no:
    mov dx,no
    call print

loader_exit:
    mov dx,mes3
    call print
    retn
;----Subprogramms--------

print:
; in: dx - offset of ASCII string
; out:
    mov ah,09h
    int 21h
    retn

file_open:
; in: ds:dx - offset of ASCIIZ filename string
; out: ax - file handle (ax=0 - not found)
    mov ax,3d00h
    int 21h
    jnc fo_exit
    xor ax,ax
fo_exit:
    retn

file_close:
; in: bx - file handle
; out:
    mov ah,3eh
    int 21h
    retn

file_read:
; in: bx - file handle
;     ds:dx - buffer
;     cx - numbers of bytes
; out:
    mov ah,3fh
    int 21h
    retn

file_offset:
; in: bx - file handle
;     cx:dx - offset in bytes (cx*65535+dx)
; out:
    mov ax,4200h
    int 21h
    retn

sector_find:
; in: ax - No of Cluster
; out: ax - 1st sector of Cluster
    dec ax
    dec ax
    push bx
    xor bx,bx
    mov bl,[ClustSect]
    mul bx
    pop bx
    add ax,[DataRgn]
    retn

read_cluster:
; in: ax - No of Cluster
;     ds:dx - buffer
; out:
    push dx
    call sector_find
    push ax
    xor eax,eax
    xor ebx,ebx
    pop ax
    mov bx,[SectSize]
    mul ebx
    mov dx,ax
    shr eax,16
    mov cx,ax
    mov bx,[handle]
    call file_offset
    xor ax,ax
    mov al,[ClustSect]
    mul [SectSize]
    mov cx,ax
    mov bx,[handle]
    pop dx
    call file_read
    retn

read_kernel:
; in:
; out:
    mov ax,8000h
    mov es,ax    ;8000:0000 = 80000h  - Temporal location of kernel
    xor di,di    ;
    mov ax,[FirstClust]
    mov bp,ax
rk_loc_00:
    push es
    mov dx,Root
    call read_cluster
    xor ax,ax        ; Moving cluster to area of location kernel
    mov al,[ClustSect]    ;
    mul [SectSize]        ;
    mov cx,ax        ;
    pop es                  ;
    mov si,Root        ;
    rep movsb        ;
    cmp di,00h
    jne rk_continue
    mov ax,es
    add ax,1000h
    mov es,ax
rk_continue:
    mov ax,bp
    cmp ax,0ff8h
    jae rk_done
    shl ax,1    ; Val=Clustrer*1,5 //(Cluster*3)/2
    add ax,bp    ;
    shr ax,1    ;
    mov bx,ax
    add bx,buffer
    mov ax,[bx]
    bt bp,0
    jc rk_nechet
    and ax,0fffh
    jmp rk_chet
rk_nechet:
    shr ax,4
rk_chet:
    mov bp,ax
    jmp rk_loc_00
rk_done:
    retn

move_kernel:
; in:
; out:
    mov ax,8000h
    mov ds,ax
    mov ax,1000h
    mov es,ax
    xor si,si
    xor di,di
    mov cx,8000h
    rep movsb
    mov cx,8000h
    rep movsb
    mov bx,es
    add bx,1000h
    mov es,bx
    mov bx,ds
    add bx,1000h
    mov ds,bx
    mov cx,8000h
    rep movsb
    mov cx,8000h
    rep movsb
    mov ax,1000h
    mov ds,ax
    mov es,ax
    jmp far 1000h:0000h
    retn
len_mk=$-move_kernel

;--------Data------------
title    db 'MenuetOS/KolibriOS Boot Loader. Ver.1.1   Copyright(C) 2003, Trans.',0ah,0dh,0ah,0dh,'$'
mes1    db 'It is alternative of boot from floppy.',0ah,0dh
    db 'You MUST select HD booting !!!',0ah,0dh,0ah,0dh,'$'
mes2    db 'Are you sure loading KolibriOS? (Y/N):','$'
yes    db 'Y','$'
no    db 'N','$'
mes3    db 0ah,0dh,0ah,0dh,'See you later ...',0ah,0dh,'$'
mes4    db 0ah,0dh,0ah,0dh,'Not Found: '
img0    db 'kolibri\kolibri.img',0,', '
img2    db 'kolibri.img',0,', '
img3    db 'menuet.img',0,' :($'
kernel    db 'KERNEL  MNT',0
handle    dw ?
SectSize    dw ?    ; +0bh
ClustSect    db ?    ; +0dh
ResSect     dw ?    ; +0eh
FATCnt        db ?    ; +10h
RootEnt     dw ?    ; +11h
FATSect     dw ?    ; +16h
filesize    dd ?    ; +1ch
FirstClust    dw ?    ; +1ah

ResRgn        dw 0    ; = VolumeStart
FATRgn        dw ?    ; = ResRgn+ResSect
RootDirRgn    dw ?    ; = FATRgn+(FATCnt*FATSect)
DataRgn     dw ?    ; = RootDirRgn+((RootEnt*32)/SectSize)
ResRgnSz    dw ?    ; = ResSect
FATRgnSz    dw ?    ; = FATCnt*FATSect
RootDirRgnSz    dw ?    ; = (RootEnt*32)/SectSize
;First sector of cluster N = DataRgn+((N-2)*ClustSect)

buffer:
    org 3000h
Root:

