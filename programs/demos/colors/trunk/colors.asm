;
;    COLOR TABLE
;
;    Compile with FASM for Menuet
;

use32

                org     0x0

                db      'MENUET00'              ; 8 byte id
                dd      38                      ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x1000                  ; required amount of memory
                dd      0x1000                  ; esp = 0x7FFF0
                dd      0x00000000              ; reserved=no extended header

include 'lang.inc'
include 'macros.inc'


START:                          ; start of execution

    call shape_window

    call draw_window            ; at first, draw the window

still:

    mov  eax,10                 ; wait here for event
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

    jmp  still


shape_window:

    pusha

    mov  eax,50      ; give the address of reference area
    mov  ebx,0
    mov  ecx,shape_reference
    int  0x40

    mov  eax,50      ; give the scaling ( 5 -> 2^5 )
    mov  ebx,1
    mov  ecx,5
    int  0x40

    popa

    ret


shape_reference:

    db   1,0,1,0,1,0,1,0
    db   0,1,0,1,0,1,0,1
    db   1,0,1,0,1,0,1,0
    db   0,1,0,1,0,1,0,1
    db   1,0,1,0,1,0,1,0
    db   0,1,0,1,0,1,0,1
    db   1,0,1,0,1,0,1,0
    db   0,1,0,1,0,1,0,1



;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+255         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+255         ; [y start] *65536 + [y size]
    mov  edx,0x00000000
    mov  esi,0x8099bbff            ; color of grab bar  RRGGBB,8->color glide
    mov  edi,0x0099bbee            ; color of frames    RRGGBB
    int  0x40

    call draw_colors

                                   ; CLOSE BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,5*65536+12            ; [x start] *65536 + [x size]
    mov  ecx,5*65536+12            ; [y start] *65536 + [y size]
    mov  edx,1                     ; button id
    mov  esi,0xccaa22              ; button color RRGGBB
    int  0x40

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


draw_colors:

    pusha

    mov  [bar],dword 0

    mov  eax,-1
    mov  ebx,0
    mov  ecx,0
    mov  edx,0

    mov  edi,0

  dc1:

    add  eax,1
    cmp  eax,256
    jb   na1
    mov  eax,0
    add  ebx,1
    cmp  ebx,256
    jb   na1
    mov  ebx,0
    add  ecx,5
  na1:

    mov  dl,al
    shl  edx,8
    mov  dl,bl
    shl  edx,8
    mov  dl,cl

  na2:

    pusha
    push edx

    xor  edx,edx
    mov  eax,edi
    mov  ebx,256
    div  ebx
    mov  ebx,edx
    mov  ecx,eax

    add  ebx,[addx]
    add  ecx,[addy]

    pop  edx
    mov  eax,1
    int  0x40
    popa

    add  edi,1

    cmp  edi,256*256
    jb   dc1

    popa
    ret



; DATA AREA

I_END:

bar  dd  ?

add1 dd  ?
add2 dd  ?

addx dd  ?
addy dd  ?
