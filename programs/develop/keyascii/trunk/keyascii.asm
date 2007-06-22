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
include '..\..\..\macros.inc'

START:                          ; start of execution
  red: 
     call draw_window

still:

    mov  eax,10                 ; wait here for event
    mcall

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall
    mov  [keyid],ah
    call draw_window
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; button id=1 ?
    jne  noclose

    or   eax,-1                 ; close this program
    mcall
  noclose:

    jmp  still




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+270         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+80          ; [y start] *65536 + [y size]
    mov  edx,0x34ffffff            ; color of work area RRGGBB,8->color gl
    mov  edi,title
    mcall

    mov  eax,4                     ; function 4 : write text to window
    xor  ecx,ecx
    mov  esi,4
    mov  ebx,8*65536+8
    mov  edx,tdec
    mcall
    add  ebx,23
    mov  edx,thex
    mcall

    mov  ecx,[keyid]
    mov  eax,47
    mov  ebx,3*65536
    mov  edx,40*65536+8
    mov  esi,0x224466
    mcall
    add  edx,23
    mov  bh,1
    mcall

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret


; DATA AREA

 tdec:  db 'DEC:'
 thex:  db 'HEX:'
 title db 'KEYBOARD ASCIICODES-PRESS ANY KEY',0
 keyid: db  0
I_END:

