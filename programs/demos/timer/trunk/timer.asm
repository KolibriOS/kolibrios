;
;   TIMER
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
include 'macros.inc'

START:                      ; start of execution
    mov  eax, 40
    mov  ebx, 101b
    int  0x40

red:
    call draw_window

still:

    mov  eax,23                 ; wait here for event
    mov  ebx,50
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,3                  ; button in buffer ?
    je   button

    call draw_clock

    jmp  still


  button:                       ; button
    or   eax,-1                 ; close this program
    int  0x40


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_clock:
    mov  eax, 13           ; clear area
    mov  ebx, 10*65536+55
    mov  ecx, 30*65536+10
    mov  edx, [sc.work]
    int  0x40

    mov  eax, 26           ; get system counter
    mov  ebx, 9
    int  0x40

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
  or   esi,0x10000000
  mov  ebx,0x00020000
  mov  edx,10*65536+30
  int  0x40

  pop  eax              ; MM
  imul ecx,ecx,60
  sub  eax,ecx
  mov  ecx,eax
  mov  eax,47
  add  edx,20*65536
  int  0x40

  pop  ecx
  pop  eax

  imul ecx,ecx,60
  sub  eax,ecx

  mov  ecx,eax          ; SS
  mov  eax,47
  add  edx,20*65536
  int  0x40

  ret

draw_window:
    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    int  0x40

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+75          ; [x start] *65536 + [x size]
    mov  ecx,100*65536+45          ; [y start] *65536 + [y size]
    mov  edx,[sc.work]             ; color of work area RRGGBB,8->color gl
    mov  esi,[sc.grab]            ; color of grab bar  RRGGBB,8->color gl
    or   esi,0x80000000
    mov  edi,[sc.frame]            ; color of frames    RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,6*65536+7             ; [x start] *65536 + [y start]
    mov  ecx,[sc.grab_text]            ; font 1 & color ( 0xF0RRGGBB )
    or   ecx,0x10000000
    mov  edx,header                ; pointer to text beginning
    mov  esi,header.len            ; text length
    int  0x40

                                   ; CLOSE BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,(75-16)*65536+12      ; [x start] *65536 + [x size]
    mov  ecx,4*65536+12            ; [y start] *65536 + [y size]
    mov  edx,1                     ; button id
    mov  esi,[sc.grab_button]      ; button color RRGGBB
    int  0x40

    call draw_clock

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


; DATA AREA

if lang eq ru
    header:
         db   'íÄâåÖê'
      .len = $ - header
else
    header:
         db   'TIMER'
      .len = $ - header
end if


I_END:

temp dd ?
sc system_colors
