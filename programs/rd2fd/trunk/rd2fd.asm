;WARNING! Spaghetti code, size optimized

  use32
  org    0x0

  db     'MENUET01'              ; 8 byte id
  dd     0x01                    ; header version
  dd     START                   ; start of code
  dd     I_END                   ; size of image
  dd     0x1000                  ; memory for app
  dd     0x1000                  ; esp
  dd     0x0 , 0x0               ; I_Param , I_Icon

include "lang.inc"
include "macros.inc" ; - 30 bytes !!!


START:
  red:                          ; redraw
    call draw_window

still:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

;   jmp  still
  key:                          ; key
                                ; just read it and ignore
    int  0x40
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40
    cmp  ah,1                   ; button id=1 ?
    je   close
    cmp  ah,2
    je   ramdiskcopy
    cmp  ah,3
    je   ramdiskupdate
    jmp  togglewrite
close:
    or   eax,-1                 ; close this program
    jmp  callsys
ramdiskcopy:
    mov eax,16
    xor ebx,ebx
    inc ebx
    jmp callsys
ramdiskupdate:
    mov eax,16
    xor ebx,ebx
    inc ebx
    inc ebx
    jmp callsys
togglewrite:
   call togglefdcwrite
callsys:
   int 0x40
   jmp  still

; get fdc settings for writing & invert them.
togglefdcwrite:
    mov eax,16
    mov ebx,4
    int 0x40
    xchg ecx,eax
    xor ecx,1
    mov eax,16
    dec ebx
    int 0x40

    mov [esp], dword still  ; change return address !
;run trough drawwindow :]

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:


    mov  eax,12                    ; function 12:tell os about windowdraw
    xor  ebx,ebx                   ; 1, start of draw
    inc ebx
    int  0x40

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw win
    mov  ebx,100*65536+250         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+120         ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB,8->color gl
    int  0x40

;The important part, the buttons & text.

mov ebx,9*65536+36
mov ecx,41*65536+14
xor edx,edx
inc edx
inc edx
;mov edx,2
call clickbox

mov ebx,67*65536+49
inc edx
call clickbox

mov ebx,12*65536+12
mov ecx,81*65536+12
inc edx
call clickbox

mov ecx,96*65536+12
xor edx,edx
call clickbox

    mov edi,0x10000000
    mov edx,titlebar
    mov ebx,9*65536+9
    mov ecx,0x10ffffff
    call print

;    mov edx,h1
    mov ebx,11*65536+28
    mov ecx,0x10808080
    call print

;    mov edx,comtext
    add ebx,15
    xchg ecx,edi
    call print

;    mov edx,h2
    add ebx,25
    xchg ecx,edi
    call print

;    mov edx,setwrite
    add ebx,15
    xchg ecx,edi
    call print

;    mov edx,setread
    add ebx,15
    xchg ecx,edi
    call print

    mov eax,16
    mov ebx,4
    int 0x40
    test al,1
    je nowritex
 ;  mov edx,xsign
    mov ebx,14*65536+83
    xchg ecx,edi
    call print
  nowritex:
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int 0x40
    ret

clickbox:
pusha
    mov edi,edx
    cmp edx, 0
    je .disabledbox
    mov  eax,8                     ; function 8 : define and draw button
    int  0x40
    .disabledbox:
    inc ecx
    inc ebx
    mov eax,13
    mov edx, 0x808080
    int 0x40
    cmp edi,0
    je .grayed
    mov edx,0x80
    .grayed:
    sub ebx,65536
    sub ecx,65536
    int 0x40
    add ebx,65534
    add ecx,65534
    mov edx,0xffffff
    int 0x40
popa
ret

print:
    mov eax,edx
    xor esi,esi
    addchar:
    inc eax
    inc esi
    cmp [eax],byte 0
    jne addchar
    mov eax,4
    int 0x40
    add edx,esi
    inc edx
ret


; DATA AREA

titlebar: db 'RD2FD',0
h1: db 'Commands',0
comtext: db 'Copy or Update ramdisk to floppy',0
h2: db 'Settings',0
setwrite: db '   Write directly to floppy',0
setread: db '   Read directly from floppy',0
xsign: db 'X',0
I_END:




