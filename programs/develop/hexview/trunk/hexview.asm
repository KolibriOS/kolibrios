;
;    EXAMPLE APPLICATION
;
;    Compile with FASM for Menuet
;

use32

                org     0x0

                db      'MENUET00'              ; 8 byte id
                dd      38                      ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x8000                  ; required amount of memory
                dd      0x8000                  ; esp = 0x7FFF0
                dd      0x00000000              ; reserved=no extended header

include 'lang.inc'
include '..\..\..\macros.inc'

begin dd 0

START:                          ; start of execution

    call open_file

red:
    call draw_window            ; at first, draw the window

still:

    mov  eax,10                 ; wait here for event
    mcall

    dec  eax                    ; redraw request ?
    je   red
    dec  eax                    ; key in buffer ?
    je   key
    dec  eax                    ; button in buffer ?
    je   button

    jmp  still

;  red:                          ; redraw
;    call draw_window
;    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    mcall

    cmp  ah,3
    jne  no_up
    cmp  [begin],16
    jb   no_up

    add  [begin],-16
    jmp  red
  no_up:

    cmp  ah,4
    jne  no_down
    add  [begin],16
    jmp  red
  no_down:

    dec  ah                     ; button id=1 ?
     jne  still
     or   eax,-1                ; close this program
    mcall


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,100*65536+400         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+270         ; [y start] *65536 + [y size]
    mov  edx,0x13224466            ; color of work area RRGGBB,8->c
    mov  edi,title                 ; WINDOW LABEL
    mcall

                                   
    mov  eax,8
    mov  ebx,280*65536+16*6
    mov  ecx,240*65536+14
    mov  edx,2
    mov  esi,0x5599cc
     mcall

    mov  ebx,15*65536+125
     inc  edx
      mcall


    add  ebx,127*65536
     inc  edx
     mcall


    mov  eax,4
    mov  ebx,15*65536+243
    mov  ecx,0xffffff
    mov  edx,buttons
    mov  esi,blen-buttons
    mcall


    mov  ebx,280*65536+35           ; draw info text with function 4
    mov  ecx,0xffffff
    mov  edx,text
    add  edx,[begin]

    mov  esi,16
     mov  edi,esi
   newline:

 push ebx                          ; hext
 push edx
push edi

    mov  edi,16
    mov  ecx,edx
    mov  edx,ebx
    add  edx,-265*65536

    mov  eax,47
    mov  ebx,0x00020101
    mov  esi,0xffff00

   newhex:

 ;   mov  ebx,0x00020101
 ;   mov  esi,0xffff00
    mcall

    add  edx,16*65536
     inc  ecx
     dec  edi
    jne  newhex

;    popa
pop edi
pop edx
pop ebx

    mov  eax,4                     ; text
    mov  esi,16
    mov  ecx,0xffffff
    mcall
    add  ebx,12
    add  edx,16
    dec  edi
    jnz  newline

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret



file_name  db  'EXAMPLE.ASM   '
      ;    db  'EXAMPLE       '

open_file:

    pusha

    mov  eax,6
    mov  ebx,file_name
    xor  ecx,ecx
    mov  edx,-1
    mov  esi,text
    mcall

    popa

    ret



; DATA AREA

title  db  'HEXVIEW',0

buttons  db  '        UP                   DOWN'
         db  '              EXAMPLE      '
blen:

text:

I_END:
