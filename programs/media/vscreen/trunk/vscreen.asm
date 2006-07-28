;
;    VIRTUAL SCREEN 2
;    based on the original VSCREEN
;
;    Compile with FASM for Menuet
;'

use32

      org     0x0

      db      'MENUET01'      ; 8 byte id
      dd      0x01            ; required os
      dd      START           ; program start
      dd      I_END           ; program image size
      dd      0x600000        ; required amount of memory
      dd      0xfff0
      dd      0,0

scr   equ     0x20000

include 'lang.inc'
include 'macros.inc'

START:                          ; start of execution

    mov  eax,14                 ; get screen size
    int  0x40
    push eax
    and  eax,0x0000ffff
    inc  eax
    mov  [size_y],eax
    pop  eax
    shr  eax,16
    inc  eax
    mov  [size_x],eax

    mov  eax,[size_x]
    shr  eax,2
    mov  [cmp_ecx],eax

    mov  eax,[size_x]
    xor  edx,edx
    mov  ebx,3
    mul  ebx
    mov  [add_esi],eax

    mov  eax,[size_y]
    shr  eax,2
    mov  [cmp_edx],eax

    mov   eax,[size_y]
    imul  eax,[size_x]
    imul  eax,3
    mov   [i_size],eax

red:
    call draw_window            ; at first, draw the window

still:

    call draw_screen

    mov  eax,23                 ; wait here for event with timeout
    mov  ebx,[delay]
    int  0x40

    dec  eax                    ; redraw request ?
    jz   red
    dec  eax                    ; key in buffer ?
    jz   key
    dec  eax                    ; button in buffer ?
    jz   button

    jmp  still

  key:                          ; key
    mov  eax,2
    int  0x40                   ; just read it and ignore
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jnz  noclose
exit:
    or   eax,-1                 ; close this program
    int  0x40
  noclose:

    cmp  ah,2
    jnz  still

    mov  eax,51
    xor  ebx,ebx
    inc  ebx
    mov  ecx,thread_start
    mov  edx,0xe000
    int  0x40

    jmp  exit

thread_start:
    mov  eax,5     ; wait for termination (1 sec.)
    mov  ebx,100
    int  0x40

    call save_screen
    jmp  exit      ; exit thread

save_screen:

     pusha

;     mov  eax,5
;     mov  ebx,500
;     int  0x40

;     xor  ebx,ebx
;     mov  edi,0x10036    ;0x10000
;     mov  esi,edi        ;0x10000
;     add  esi,[i_size]

;   ss1:

;     mov  eax,35
;     int  0x40

;     inc  ebx

;     mov  [edi],eax
;     add  edi,3

;     cmp  edi,esi
;     jb   ss1

; 1) READ SCREEN
     mov  edi,0x10036

     mov  eax,[size_y]
     dec  eax
     mov  [temp_y],eax

  ynew:

     xor  eax,eax
     mov  [temp_x],eax

  xnew:

     mov  eax,[temp_x]
     mov  ebx,[temp_y]
     call read_pixel

     mov  [edi],eax
     add  edi,3

     inc  [temp_x]

     mov  eax,[size_x]
     cmp  [temp_x],eax
     jb   xnew

     dec  [temp_y]

     cmp  [temp_y],0
     jge  ynew

; 2) BMP HEADER

     mov  [0x10000],word 'BM'     ; bmp signature
     mov  eax,[i_size]
     mov  [0x10000+34],eax        ; image size
     mov  ebx,0x36
     mov  [0x10000+10],ebx        ; headers size
     add  eax,ebx
     mov  [0x10000+2],eax         ; file size
     mov  [0x10000+14],dword 0x28
     mov  eax,[size_x]
     mov  [0x10000+18],eax        ; x size
     mov  eax,[size_y]
     mov  [0x10000+22],eax        ; y size
     mov  [0x10000+26],word 1
     mov  [0x10000+28],word 0x18  ; bpp = 24 = 0x18

; 3) SAVE FILE

     mov  eax,56
     mov  ebx,filename
     mov  edx,0x10000
     mov  ecx,[i_size]
     add  ecx,0x36
     mov  esi,path
     int  0x40

     popa
     ret

filename  db  'SCREEN  BMP'
path      db  0

read_pixel:
pushad

mov esi,eax
mov eax,[size_x]
mul ebx
add eax,esi

xchg eax,ebx
mov eax,35
int 0x40
mov [esp+28],eax

popad
ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
  movzx  ebx,word [size_x]
    shr  ebx,2
    add  ebx,100*65536+40

  movzx  ecx,word [size_y]
    shr  ecx,2
    add  ecx,100*65536+75

    mov  edx,0x027777dd  ;82       ; color of work area RRGGBB
    mov  esi,0x807777dd            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x007777dd            ; color of frames    RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ffffff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

                                   ; CLOSE BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,[size_x]
    shr  ebx,2
    add  ebx,40-19
    shl  ebx,16
    mov  bx,12                     ; [x start] *65536 + [x size]
    mov  ecx,5*65536+12            ; [y start] *65536 + [y size]
    xor  edx,edx                   ; button id
    inc  edx
    mov  esi,0x7777dd              ; button color RRGGBB
    int  0x40

                                   ; save image
    mov  ebx,20*65536
    mov  bx,word [size_x]
    shr  bx,2
    mov  cx,word [size_y]
    shr  cx,2
    add  cx,49
    shl  ecx,16
    mov  cx,12
    mov  edx,2
    mov  esi,0x4f4f9f
    int  0x40

    shr  ecx,16
    mov  ebx,25*65536
    mov  bx,cx
    add  bx,3
    mov  eax,4
    mov  ecx,0xffffff
    mov  edx,savetext
    mov  esi,24 ;22
    int  0x40

    call draw_screen

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret

draw_screen:

    pusha

    mov  edi,scr

    xor  ecx,ecx
    xor  edx,edx

    xor  esi,esi

  ds1:

    mov  eax,35
    mov  ebx,esi
    int  0x40
    stosd
    dec  edi

    add  esi,4
    inc  ecx
    cmp  ecx,[cmp_ecx] ; 800/4
    jb   ds1

    add  esi,[add_esi] ; 800*3
    xor  ecx,ecx
    inc  edx
    cmp  edx,[cmp_edx] ; 600/4
    jb   ds1

    mov  eax,7
    mov  ebx,scr
    mov  ecx,200*65536+160
    mov  ecx,[size_x]
    shr  ecx,2
    shl  ecx,16
    mov  cx,word [size_y]
    shr  cx,2
    mov  edx,20*65536+35
    int  0x40

    popa

    ret

draw_magnify:

    pusha

    mov  [m_x],dword 0x0
    mov  [m_y],dword 0x0

    xor  ecx,ecx
    xor  edx,edx

  dm1:

    push edx
    mov  eax,edx
    mul  [size_x]
    pop  edx
    add  eax,ecx

    mov  ebx,eax
    mov  eax,35
    int  0x40

    pusha
    mov  ebx,ecx
    mov  ecx,edx
    shl  ebx,3
    add  ebx,20
    shl  ebx,16
    mov  bx,8
    shl  ecx,3
    add  ecx,35
    shl  ecx,16
    mov  cx,8

    mov  edx,eax
    mov  eax,13
    int  0x40
    popa

    inc  ecx
    cmp  ecx,40
    jnz  dm1
    xor  ecx,ecx
    inc  edx
    cmp  edx,32
    jnz  dm1

    popa
    ret

; DATA AREA

i_size   dd  0x1

m_x      dd  100
m_y      dd  100

cmp_ecx  dd  0
add_esi  dd  0
cmp_edx  dd  0

delay    dd  100

labelt:
    db   'VIRTUAL SCREEN 2'
labellen:

savetext  db  'SAVE AS /HD/1/SCREEN.BMP   '

I_END:

temp_x dd ?
temp_y dd ?

size_x dd ?
size_y dd ?
