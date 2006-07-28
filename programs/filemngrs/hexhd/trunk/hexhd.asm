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
                dd      0x5000                  ; required amount of memory
                dd      0x5000                  ; esp = 0x7FFF0
                dd      0x00000000              ; reserved=no extended header

include 'lang.inc'
include 'macros.inc'

START:                          ; start of execution

    call open_file

    call draw_window            ; at first, draw the window

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
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose
    mov  eax,-1                 ; close this program
    int  0x40
  noclose:

    cmp  ah,2
    jne  noexam
    jmp  still
  noexam:

    cmp  ah,3
    jne  noback
    mov  edi,block
    cmp  dword [edi],0
    je   nonext
    dec  dword [edi]
    call open_file
    call draw_window
  noback:

    cmp  ah,4
    jne  nonext
    mov  edi,block
    inc  dword [edi]
    call open_file
    call draw_window
  nonext:

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
    mov  ebx,0*65536+400         ; [x start] *65536 + [x size]
    mov  ecx,0*65536+270+200       ; [y start] *65536 + [y size]
    mov  edx,0x038899aa;1111cc     ; color of work area RRGGBB,8->c
    int  0x40

    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,144*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

    mov  eax,47          ;decimal
    mov  ecx,[block]
    mov  edx,64*65536+34
    mov  ebx,10
    shl  ebx,16
    mov  esi,0x00ffffff
    int  0x40

    add  edx,128*65536    ;hex
    mov  bh,1
    int  0x40

    mov  eax,8
    mov  ebx,280*65536+16*6
    mov  ecx,30*65536+14

    mov  edx,2
    mov  esi,0x3f49df;5599cc
    int  0x40

    mov  ebx,15*65536+32
    add  edx,1
    mov  eax,8
    int  0x40

    add  ebx,127*65536
    add  edx,1
    mov  eax,8
    int  0x40


    mov  eax,4
    mov  ebx,14*65536+33
    mov  ecx,0xffffff
    mov  edx,buttons
    mov  esi,blen-buttons
    int  0x40


    mov  ebx,280*65536+65           ; draw info text with function 4
    mov  ecx,0xffffff
    mov  edx,text
    mov  esi,16
    mov  edi,16*2
  newline:

    pusha                          ; hext

    mov  edi,16

    mov  ecx,edx

    mov  edx,ebx
    sub  edx,265*65536

   newhex:

    mov  eax,47
    mov  ebx,0x00020101
    xor  esi,0xffff00
    int  0x40

    add  edx,16*65536
    add  ecx,1

    dec  edi
    jne  newhex

    popa

    mov  eax,4                     ; text
    int  0x40
    add  ebx,12
    add  edx,16
    dec  edi
    jnz  newline


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret



open_file:

    pusha

    mov  eax,58
    mov  ebx,fileinfo
    int  0x40

    popa

    ret




; DATA AREA

fileinfo:
      dd 8
block dd 0
      dd 1
      dd text
      dd os
      db '/HARDDISK/FIRST',0


labelt:
      db   'EDITOR HEXADECIMAL'
labellen:

buttons db '  <<                   >>                    <<  OPTIONS  >> '
blen:

os:
times 16384 db ?
text:

I_END:
