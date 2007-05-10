
;
;    MIXER
;
;    Compile with FASM
;

include 'lang.inc'
include '..\..\..\macros.inc'


use32
 org	0x0
 db	'MENUET01'    ; header
 dd	0x01	      ; header version
 dd	START	      ; entry point
 dd	I_END	      ; image size
 dd	0x1000        ; required memory
 dd	0x1000        ; esp
 dd	0x0 , 0x0     ; I_Param , I_Path



START:                          ; start of execution

  red:                          ; redraw
    call draw_window            ; at first, draw the window


still:

    mov  eax,10                 ; wait here for event
    mcall

    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button

    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall

    jmp  still

  button:                       ; button
    mov  eax,17
    mcall

    cmp  ah,1                   ; button id=1 ?
    jnz  noclose
    or   eax,-1                 ; close this program
    mcall
  noclose:

    cmp  ah,101
    jnz  nochange
    mov  al,byte [usecard]
    cld
    inc  al
    and  al,3
    cmp  al,0
    jnz  nozer
    mov  al,1
  nozer:
    mov  byte [usecard],al

    call drawusedcard

  nochange:

    cmp  byte [usecard],byte 1
    jz   usesb16

    cmp  byte [usecard],byte 2
    jnz  nousesb16II
    jmp  usesb16II
  nousesb16II:

    cmp  byte [usecard],byte 3
    jnz  nousewss
    jmp  usewss
  nousewss:



; SOUND BLASTER 16


  usesb16:


    cld

    mov  al,20
    cmp  ah,al
    jge  nomain

    mov  ecx,0
    cmp  ah,12
    jnz  nomain12
    mov  ecx,3*16+3
  nomain12:
    cmp  ah,13
    jnz  nomain13
    mov  ecx,7*16+7
  nomain13:
    cmp  ah,14
    jnz  nomain14
    mov  ecx,11*16+11
  nomain14:
    cmp  ah,15
    jnz  nomain15
    mov  ecx,15*16+15
  nomain15:

    mov  eax,25
    mov  ebx,1
    mcall

    jmp  still

  nomain:

    mov  al,30
    cmp  ah,al
    jge  nocd

    mov  ecx,0

    cmp  ah,22
    jnz  nocd12
    mov  ecx,3*16+3
  nocd12:
    cmp  ah,23
    jnz  nocd13
    mov  ecx,7*16+7
  nocd13:
    cmp  ah,24
    jnz  nocd14
    mov  ecx,11*16+11
  nocd14:
    cmp  ah,25
    jnz  nocd15
    mov  ecx,15*16+15
  nocd15:

    mov  eax,25
    mov  ebx,2
    mcall

    jmp  still

  nocd:


    jmp  still




; SOUND BLASTER 16 II

  usesb16II:

    cld

    mov  al,20
    cmp  ah,al
    jge  IIwnomain

    mov  ecx,0
    cmp  ah,12
    jnz  IIwnomain12
    mov  ecx,50
  IIwnomain12:
    cmp  ah,13
    jnz  IIwnomain13
    mov  ecx,150
  IIwnomain13:
    cmp  ah,14
    jnz  IIwnomain14
    mov  ecx,200
  IIwnomain14:
    cmp  ah,15
    jnz  IIwnomain15
    mov  ecx,255
  IIwnomain15:

    mov  eax,28
    mov  ebx,1
    mcall

    jmp  still

  IIwnomain:

    mov  al,30
    cmp  ah,al
    jge  IIwnocd

    mov  ecx,0

    cmp  ah,22
    jnz  IIwnocd12
    mov  ecx,50
  IIwnocd12:
    cmp  ah,23
    jnz  IIwnocd13
    mov  ecx,150
  IIwnocd13:
    cmp  ah,24
    jnz  IIwnocd14
    mov  ecx,200
  IIwnocd14:
    cmp  ah,25
    jnz  IIwnocd15
    mov  ecx,255
  IIwnocd15:

    mov  eax,28
    mov  ebx,2
    mcall

    jmp  still

  IIwnocd:


    jmp  still









; WINDOWS SOUND SYSTEM

  usewss:

    cld

    mov  al,20
    cmp  ah,al
    jge  wnomain

    mov  ecx,255
    cmp  ah,12
    jnz  wnomain12
    mov  ecx,200
  wnomain12:
    cmp  ah,13
    jnz  wnomain13
    mov  ecx,150
  wnomain13:
    cmp  ah,14
    jnz  wnomain14
    mov  ecx,70
  wnomain14:
    cmp  ah,15
    jnz  wnomain15
    mov  ecx,0
  wnomain15:

    mov  eax,27
    mov  ebx,1
    mcall

    jmp  still

  wnomain:

    mov  al,30
    cmp  ah,al
    jge  wnocd

    mov  ecx,255

    cmp  ah,22
    jnz  wnocd12
    mov  ecx,200
  wnocd12:
    cmp  ah,23
    jnz  wnocd13
    mov  ecx,150
  wnocd13:
    cmp  ah,24
    jnz  wnocd14
    mov  ecx,70
  wnocd14:
    cmp  ah,25
    jnz  wnocd15
    mov  ecx,0
  wnocd15:

    mov  eax,27
    mov  ebx,2
    mcall

    jmp  still

  wnocd:


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
    mov  ebx,100*65536+195         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+140         ; [y start] *65536 + [y size]
    mov  edx,[wcolor]              ; color of work area RRGGBB
    mov  edi,title                 ; WINDOW LABEL
    mcall


    mov  edx,16                    ; button id
    mov  ebx,10*65536

  newbut:

    push edx
    push ebx

    mov  esi,[bcolor]

    mov  eax,8                     ; function 8 : define and draw button
    mov  bx,22                     ; [x start] *65536 + [x size]
    mov  ecx,35*65536+8            ; [y start] *65536 + [y size]
    dec  edx
    mcall
    mov  bx,22                     ; [x start] *65536 + [x size]
    mov  ecx,45*65536+8            ; [y start] *65536 + [y size]
    dec  edx
    mcall
    mov  bx,22                     ; [x start] *65536 + [x size]
    mov  ecx,55*65536+8            ; [y start] *65536 + [y size]
    dec  edx
    mcall
    mov  bx,22                     ; [x start] *65536 + [x size]
    mov  ecx,65*65536+8            ; [y start] *65536 + [y size]
    dec  edx
    mcall
    mov  bx,22                     ; [x start] *65536 + [x size]
    mov  ecx,75*65536+8            ; [y start] *65536 + [y size]
    dec  edx
    mcall

    pop  ebx
    pop  edx

    add  ebx,30*65536
    add  edx,10

    cmp  edx,16+6*10
    jz   butdone

    jmp  newbut

  butdone:


    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,10*65536+104          ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; color of text RRGGBB
    mov  edx,text                  ; pointer to text beginning
    mov  esi,29
    mcall

    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,(5)*65536+185         ; [x start] *65536 + [x size]
    mov  ecx,120*65536+14          ; [y start] *65536 + [y size]
    mov  edx,101                   ; button id
    mov  esi,[bcolor]              ; button color RRGGBB
    mcall

    call drawusedcard

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret


drawusedcard:

    pusha

    mov  eax,13
    mov  ebx,14*65536+160
    mov  ecx,123*65536+10
    mov  edx,[bcolor]
    mcall

    mov  eax,[usecard]
    mov  edx,c3

    cmp  al,1
    jnz  nosbc
    mov  edx,c1
  nosbc:
    cmp  al,2
    jnz  nosbcII
    mov  edx,c2
  nosbcII:
    cmp  al,3
    jnz  nowssc
    mov  edx,c3
  nowssc:

    mov  eax,4
    mov  ebx,14*65536+123
    mov  ecx,0x00ffffff
    mov  esi,30
    mcall

    popa

    ret



; DATA AREA

bcolor  dd  0x5577c8

wcolor  dd  0x13000000


text:
    db 'MAIN  CD  WAVE MPU4 AUX1 AUX2'

c1  db 'SOUND BLASTER 16 - MIXER I    '
c2  db 'SOUND BLASTER 16 - MIXER II   '
c3  db 'WINDOWS SOUND SYSTEM          '


usecard dd 0x1

title    db   'MIXER',0

I_END:



