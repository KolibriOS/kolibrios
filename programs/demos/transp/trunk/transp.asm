;
;   TRANSPARENT EXAMPLE
;
;   Compile with FASM for Menuet
;

use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x30000                 ; memory for app
               dd     0x30000                 ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

include 'lang.inc'
include '..\..\..\macros.inc'

START:                          ; start of execution
    mov  [procinfo.box.left], 100
    mov  [procinfo.box.width],  200
    mov  [procinfo.box.top], 80
    mov  [procinfo.box.height],  300
    call draw_window
red:    
    call get_transparent
    call draw_window            ; at first, draw the window

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
    mov  al,2                  ; just read it and ignore
    mcall
    jmp  still

  button:                       ; button
    mov  al,17                 ; get id
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
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,[procinfo.box.left]
    shl  ebx,16
    add  ebx,[procinfo.box.width]
    mov  ecx,[procinfo.box.top]
    shl  ecx,16
    add  ecx,[procinfo.box.height]
    mov  edx,0x33000000            ; color of work area RRGGBB,8->color gl
    mov  edi,title                ; WINDOW LABEL
    mcall

    call draw_transparent
                                   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret




draw_transparent:

    pusha

    mov  eax,7
    mov  ebx,0x1000
    mov  ecx,[procinfo.box.width]
    shl  ecx,16
    add  ecx,[procinfo.box.height]
    xor  edx,edx
    mcall

    popa
    ret


get_transparent:

    pusha

    mov  eax,9
    mov  ebx,I_END
    mov  ecx,-1
    mcall

    mov  eax,14
    mcall

    shr  eax,16
    inc  eax
    mov  [scx],eax

    add  [procinfo.box.left], 4
    sub  [procinfo.box.width],  4+4
    add  [procinfo.box.top], 22
    sub  [procinfo.box.height],  22+4

    mov  eax,[procinfo.box.left]
    add  eax,[procinfo.box.width]
    mov  [x_end],eax
    mov  eax,[procinfo.box.top]
    add  eax,[procinfo.box.height]
    mov  [y_end],eax

    mov  eax,[procinfo.box.left]
    mov  ebx,[procinfo.box.top]

  dtpl1:

    push  eax
    push  ebx

    imul  ebx,[scx]
    add   ebx,eax
    mov   eax,35
    mcall

    or    eax, 0x4e4e4e

    mov   ebx,[esp+4]
    mov   ecx,[esp]
    sub   ebx,[procinfo.box.left]
    sub   ecx,[procinfo.box.top]
    imul  ecx,[procinfo.box.width]
    imul  ebx,3
    imul  ecx,3
    add   ebx,ecx
    mov   [0x1000+ebx],eax

    pop   ebx
    pop   eax

    inc   eax
    cmp   eax,[x_end]
    jb    dtpl1
    mov   eax,[procinfo.box.left]
    inc   ebx
    cmp   ebx,[y_end]
    jb    dtpl1

    popa

    ret




; DATA AREA


x_end    dd 0
y_end    dd 0

scx      dd 640

title     db   'Transparent',0

I_END:
procinfo process_information

