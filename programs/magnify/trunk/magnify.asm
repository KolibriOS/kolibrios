;
;    MAGNIFY SCREEN
;
;    Compile with FASM for Menuet
;

use32

                org     0x0

                db      'MENUET00'              ; 8 byte id
                dd      38                      ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x10000                ; required amount of memory
                dd      0x10000
                dd      0x00000000              ; reserved=no extended header

include 'lang.inc'
include 'macros.inc'
scr     equ     0x1000


START:                          ; start of execution

    mov  esp,0xfff0

    mov  eax,14                 ; get screen size
    int  0x40
    push eax
    and  eax,0x0000ffff
    add  eax,1
    mov  [size_y],eax
    pop  eax
    shr  eax,16
    add  eax,1
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



    call draw_window            ; at first, draw the window

still:

    call draw_screen

    mov  eax,23                 ; wait here for event with timeout
    mov  ebx,[delay]
    int  0x40

    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button

    jmp  still

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jnz  noclose
    mov  eax,0xffffffff         ; close this program
    int  0x40
  noclose:

    cmp  ah,2
    jnz  nosave
    call save_screen
  nosave:

    jmp  still



save_screen:

     pusha

     mov  ebx,0
     mov  edi,0x10000

   ss1:

     mov  eax,35
     int  0x40

     add  ebx,1

     mov  [edi],eax
     add  edi,3

     cmp  edi,0xFFFF0
     jb   ss1

     mov  eax,33
     mov  ebx,filename
     mov  ecx,0x10000
     mov  edx,0xEFFF0
     mov  esi,0
     int  0x40

     popa

     ret


filename  db  'SCREEN  RAW'

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536             ; [x start] *65536 + [x size]
    mov  ebx,100*65536+322
    mov  ecx,100*65536+262         ; [y start] *65536 + [y size]
    mov  edx,0x0;01111cc            ; color of work area RRGGBB
    mov  esi,0x809977ff            ; color of grab bar  RRGGBB,8->color glide
    mov  edi,0x00ffff00            ; color of frames    RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

                                   ; CLOSE BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  bx,12                     ; [x start] *65536 + [x size]
    mov  ebx,(322-19)*65536+12
    mov  ecx,5*65536+12            ; [y start] *65536 + [y size]
    mov  edx,1                     ; button id
    mov  esi,0x22aacc              ; button color RRGGBB
    int  0x40

    call draw_screen

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


draw_screen:

    call draw_magnify

    ret

    pusha

    mov  edi,scr

    mov  ecx,0
    mov  edx,0

    mov  esi,0

  ds1:

    mov  eax,35
    mov  ebx,esi
    int  0x40
    stosd
    sub  edi,1

    add  esi,4
    add  ecx,1
    cmp  ecx,[cmp_ecx] ; 800/4
    jb   ds1

    add  esi,[add_esi] ; 800*3
    mov  ecx,0
    add  edx,1
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

    call draw_magnify

    ret


draw_magnify:

    pusha

    mov  eax,37
    mov  ebx,0
    int  0x40

    mov  ecx,eax
    mov  edx,eax
    shr  ecx,16
    and  edx,65535

    sub  ecx,39
    sub  edx,29

    cmp  ecx,3000
    jb   co1
    popa
    ret
  co1:
    cmp  edx,3000
    jb   co2
    popa
    ret

  co2:

    and  ecx,2047
    and  edx,2047

    mov  [m_x],ecx
    mov  [m_y],edx

    add  ecx,40
    add  edx,30

    mov  [m_xe],ecx
    mov  [m_ye],edx

    mov  ecx,[m_x]
    mov  edx,[m_y]

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
    sub  ebx,[m_x]
    mov  ecx,edx
    sub  ecx,[m_y]
    shl  ebx,3
    add  ebx,2
    shl  ebx,16
    mov  bx,7
    shl  ecx,3
    add  ecx,22
    shl  ecx,16
    mov  cx,7

    mov  edx,eax
    mov  eax,13
    int  0x40
    popa

    add  ecx,1
    cmp  ecx,[m_xe]
    jnz  dm1
    mov  ecx,[m_x]
    add  edx,1
    cmp  edx,[m_ye]
    jnz  dm1

    popa

    ret



; DATA AREA

m_x      dd  100
m_y      dd  100

m_xe     dd  110
m_ye     dd  110

size_x   dd  0
size_y   dd  0

cmp_ecx  dd  0
add_esi  dd  0
cmp_edx  dd  0

delay    dd  20

labelt:
    db   'MAGNIFIER - MOVE MOUSE POINTER'
labellen:


I_END:



