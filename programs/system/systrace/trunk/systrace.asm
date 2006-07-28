;
;   SYSTEM CALL TRACE , V.Turjanmaa
;
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

    call draw_window            ; at first, draw the window

still:

    mov  eax,23                 ; wait here for event
    mov  ebx,50
    int  0x40

    call display_calls

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
    mov  ebx,100*65536+465          ; [x start] *65536 + [x size]
    mov  ecx,50*65536+265          ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB,8->color gl
    mov  esi,0x808899ff            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x008899ff            ; color of frames    RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ddeeff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

    mov  eax,4
    mov  ebx,15*65536+55
    mov  ecx,0x0
    mov  edx,text
    mov  esi,70
    int  0x40

                                   ; CLOSE BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,(465-19)*65536+12     ; [x start] *65536 + [x size]
    mov  ecx,5*65536+12            ; [y start] *65536 + [y size]
    mov  edx,1                     ; button id
    mov  esi,0x6677cc              ; button color RRGGBB
;    int  0x40

    call display_calls

    mov  eax,12
    mov  ebx,2
    int  0x40

    ret



display_calls:


    pusha

    mov  eax,13
    mov  ebx,15*65536+70
    mov  ecx,35*65536+10
    mov  edx,0xffffff
    int  0x40

    mov  eax,59
    mov  ebx,0
    mov  ecx,I_END
    mov  edx,64*16
    int  0x40

    mov  ecx,eax
    mov  eax,47
    mov  ebx,10*65536
    mov  edx,15*65536+35
    mov  esi,0
    int  0x40


    mov  ebx,75
    mov  edx,I_END
  newline:

    push ebx
    push edx

    mov  edi,0

  new_x:

    mov  eax,13
    mov  ebx,edi
    shl  ebx,6+16
    add  ebx,15*65536
    mov  bx,6*8
    mov  ecx,[esp+4]
    shl  ecx,16
    mov  cx,10
    mov  edx,0xffffff
    int  0x40

    mov  eax,47
    mov  ebx,1+1*256+8*65536

    mov  ecx,[pos+edi*4]
    shl  ecx,2
    add  ecx,[esp+0]

    mov  edx,edi
    shl  edx,6+16
    mov  dx,[esp+4]
    or   edx,15*65536
    mov  esi,0x0
    int  0x40

    inc  edi

    cmp  edi,7
    jb   new_x

    pop  edx
    pop  ebx

    add  ebx,10
    add  edx,64

    cmp  edx,I_END+16*64
    jb   newline

    popa
    ret




; DATA AREA

pos dd 0,15,12,14,13,9,8

text:
  db '  PID        EAX        EBX       ECX        EDX        ESI       EDI '


labelt:
      db   'SYSTEM CALL TRACE'
labellen:

I_END:
