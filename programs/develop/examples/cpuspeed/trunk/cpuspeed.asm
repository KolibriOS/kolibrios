;
;    CPU SPEED INDICATIOR
;
;    Compile with FASM for Menuet
;

  use32
  org     0x0

  db      'MENUET00'   ; 8 byte id
  dd      38           ; required os
  dd      START        ; program start
  dd      I_END        ; program image size
  dd      0x1000       ; required amount of memory
  dd      0x1000       ; esp
  dd      0x00000000   ; reserved=no extended header

include 'lang.inc'
include '..\..\..\..\macros.inc'

START:                          ; start of execution

    mov  eax,18
    mov  ebx,5
    mcall

    xor  edx,edx
    mov  ebx,1000000
    div  ebx
    mov  ebx,10
    mov  edi,text+19
    mov  ecx,5
  newnum:
    xor  edx,edx
    mov  ebx,10
    div  ebx
    add  dl,48
    mov  [edi],dl
    sub  edi,1
    loop newnum
    
    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    mcall

red:
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
    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; button id=1 ?
    jnz  still
    or   eax,-1                 ; close this program
    mcall


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+200         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+65          ; [y start] *65536 + [y size]
    mov  edx,[sc.work]             ; color of work area RRGGBB,8->color glide
    or   edx,0x33000000            ; color of grab bar  RRGGBB,8->color
    mov  edi,title                ; WINDOW LABEL
    mcall


    mov  ebx,20*65536+14           ; draw info text with function 4
    mov  ecx,[sc.work_text]
    mov  edx,text
    mov  esi,24
    mov  eax,4
    mcall
    
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret


; DATA AREA


text:
    db 'CPU RUNNING AT       MHZ'

title    db   'CPU SPEED',0

I_END:

sc system_colors

