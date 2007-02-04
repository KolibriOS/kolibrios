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
include 'macros.inc'

START:                          ; start of execution
    mov  [procinfo.x_start], 100
    mov  [procinfo.x_size],  200
    mov  [procinfo.y_start], 80
    mov  [procinfo.y_size],  300
    call draw_window
red:    
    call get_transparent
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

  key:                          ; key
    mov  al,2                  ; just read it and ignore
    int  0x40
    jmp  still

  button:                       ; button
    mov  al,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose
    or   eax,-1                 ; close this program
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
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,[procinfo.x_start]
    shl  ebx,16
    add  ebx,[procinfo.x_size]
    mov  ecx,[procinfo.y_start]
    shl  ecx,16
    add  ecx,[procinfo.y_size]
    mov  edx,0x33000000            ; color of work area RRGGBB,8->color gl
    mov  edi,header                ; WINDOW LABEL
    int  0x40

    call draw_transparent
                                   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret




draw_transparent:

    pusha

    mov  eax,7
    mov  ebx,0x1000
    mov  ecx,[procinfo.x_size]
    shl  ecx,16
    add  ecx,[procinfo.y_size]
    xor  edx,edx
    int  0x40

    popa
    ret


get_transparent:

    pusha

    mov  eax,9
    mov  ebx,I_END
    mov  ecx,-1
    int  0x40

    mov  eax,14
    int  0x40

    shr  eax,16
    inc  eax
    mov  [scx],eax

    add  [procinfo.x_start], 4
    sub  [procinfo.x_size],  4+4
    add  [procinfo.y_start], 22
    sub  [procinfo.y_size],  22+4

    mov  eax,[procinfo.x_start]
    add  eax,[procinfo.x_size]
    mov  [x_end],eax
    mov  eax,[procinfo.y_start]
    add  eax,[procinfo.y_size]
    mov  [y_end],eax

    mov  eax,[procinfo.x_start]
    mov  ebx,[procinfo.y_start]

  dtpl1:

    push  eax
    push  ebx

    imul  ebx,[scx]
    add   ebx,eax
    mov   eax,35
    int   0x40

    or    eax, 0x4e4e4e

    mov   ebx,[esp+4]
    mov   ecx,[esp]
    sub   ebx,[procinfo.x_start]
    sub   ecx,[procinfo.y_start]
    imul  ecx,[procinfo.x_size]
    imul  ebx,3
    imul  ecx,3
    add   ebx,ecx
    mov   [0x1000+ebx],eax

    pop   ebx
    pop   eax

    inc   eax
    cmp   eax,[x_end]
    jb    dtpl1
    mov   eax,[procinfo.x_start]
    inc   ebx
    cmp   ebx,[y_end]
    jb    dtpl1

    popa

    ret




; DATA AREA

;x_start  dd 100
;y_start  dd 80

;x_size   dd 160
;y_size   dd 200

x_end    dd 0
y_end    dd 0

scx      dd 640

header     db   'Transparent',0

I_END:
procinfo process_information

