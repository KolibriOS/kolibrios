;
;   TIMER (show how much system works)
;
;   Compile with flat assembler
;

use32

    org    0x0

    db     'MENUET01'       ; 8 byte id
    dd     0x01             ; header version
    dd     START            ; start of code
    dd     I_END            ; size of image
    dd     0x1000           ; memory for app
    dd     0x1000           ; esp
    dd     0x0 , 0x0        ; I_Param , I_Icon

include 'lang.inc'
include '..\..\..\macros.inc'

START:                      ; start of execution
    mov  eax, 40
    mov  ebx, 101b
    mcall

red:
    call draw_window

still:

    mov  eax,23                 ; wait here for event
    mov  ebx,50
    mcall

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,3                  ; button in buffer ?
    je   button

    call draw_clock

    jmp  still


  button:                       ; button
    or   eax,-1                 ; close this program
    mcall


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_clock:

    mov  eax, 26           ; get system counter
    mov  ebx, 9
    mcall

    cdq ;xor  edx,edx
    mov  ebx,100
    div  ebx

  push eax

  xor  edx,edx
  mov  ebx,60
  div  ebx
  mov  ecx,eax

  push ecx
  push ecx

  xor  edx,edx
  mov  ebx,60
  div  ebx
  mov  ecx,eax

  mov  eax,47           ; HH
  mov  esi,[sc.work_text]
  or   esi,0x50000000
  mov  edi,[sc.work]
  mov  ebx,0x00020000
  mov  edx,15*65536+5
  mcall

  pop  eax              ; MM
  imul ecx,ecx,60
  sub  eax,ecx
  mov  ecx,eax
  mov  eax,47
  add  edx,20*65536
  mcall

  pop  ecx
  pop  eax

  imul ecx,ecx,60
  sub  eax,ecx

  mov  ecx,eax          ; SS
  mov  eax,47
  add  edx,20*65536
  mcall

  ret

draw_window:
    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    mcall

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,100*65536+100         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+40          ; [y start] *65536 + [y size]
    mov  edx,[sc.work]             ; color of work area RRGGBB,8->color gl
    or   edx,0x34000000
    mov  edi,title
    mcall

    call draw_clock

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret


; DATA AREA

if lang eq ru
    title   db   'íÄâåÖê',0
else
    title   db   'TIMER',0
end if


I_END:

temp dd ?
sc system_colors
