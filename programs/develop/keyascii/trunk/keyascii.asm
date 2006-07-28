;   Author: M. Lisovin
;   Compile with FASM for Menuet
;

use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x1000                  ; memory for app
               dd     0x1000                  ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

include 'lang.inc'
include 'macros.inc'

START:                          ; start of execution

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

    jmp  still

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40
    mov  [keyid],ah
    call draw_window
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose

    mov  eax,-1                 ; close this program
    int  0x40
  noclose:

    jmp  still




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+270         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+80          ; [y start] *65536 + [y size]
    mov  edx,0x83ffffff            ; color of work area RRGGBB,8->color gl
    mov  esi,0x805080d0            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x005080d0            ; color of frames    RRGGBB
    int  0x40

    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ddeeff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40
    not  ecx
    mov  esi,4
    add  ebx,23
    mov  edx,tdec
    int  0x40
    add  ebx,23
    mov  edx,thex
    int  0x40

    mov  ecx,[keyid]
    mov  eax,47
    mov  ebx,3*65536
    mov  edx,40*65536+31
    mov  esi,0x224466
    int  0x40
    add  edx,23
    mov  bh,1
    int  0x40

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


; DATA AREA

 tdec: db 'DEC:'
 thex: db 'HEX:'
 labelt:
     db   'KEYBOARD ASCIICODES-PRESS ANY KEY'
labellen:dd 0
keyid:db 0
I_END:

