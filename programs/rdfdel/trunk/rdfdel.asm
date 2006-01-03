;
;    FILE COPIER
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
   dd      0x1000
   dd      0            ; reserved=no extended header

include "lang.inc"
include "macros.inc"

START:                          ; start of execution

    call draw_window            ; at first, draw the window

still:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button

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
    jnz  noclose
    mov  eax,0xffffffff         ; close this program
    int  0x40
  noclose:


    cmp  ah,3    ; delete ?
    jnz  nodelete
    mov  eax,32
    mov  ebx,destination
    int  0x40
    jmp  still
  nodelete:


    cmp  ah,4
    jz   f2
    jmp  nof12

  f2:
    mov  [addr],dword destination
    mov  [ya],dword 35
    jmp  rk

  rk:
    mov  edi,[addr]
    mov  al,' '
    mov  ecx,11
    rep  stosb

    call print_text

    mov  edi,[addr]

  f11:
    mov  eax,10
    int  0x40
    cmp  eax,2
    jz   fbu
    jmp  still
  fbu:
    mov  eax,2
    int  0x40
    shr  eax,8
    cmp  eax,8
    jnz  nobs
    cmp  edi,[addr]
    jz   f11
    sub  edi,1
    mov  [edi],byte 0
    call print_text
    jmp  f11
  nobs:
    cmp  eax,dword 31
    jbe  f11
    cmp  eax,dword 95
    jb   keyok
    sub  eax,32
  keyok:
    mov  [edi],al

    call print_text

    add  edi,1
    mov  esi,[addr]
    add  esi,11
    cmp  esi,edi
    jnz  f11

    jmp  still

print_text:

    mov  eax,13
    mov  ebx,109*65536+11*6
    mov  ecx,[ya]
    shl  ecx,16
    mov  cx,8
    mov  edx,0xffffff
    int  0x40

    mov  eax,4
    mov  ebx,109*65536
    add  ebx,[ya]
    mov  ecx,0x000000
    mov  edx,[addr]
    mov  esi,11
    int  0x40

    ret

  nof12:
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
    mov  ebx,100*65536+230         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+105         ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ffffff            ; color of text RRGGBB
    mov  edx,header                ; pointer to text beginning
    mov  esi,header.len            ; text length
    int  0x40

    mov  esi,0xbbbbbb

    mov  eax,8              ; DELETE BUTTON
    mov  ebx,20*65536+190
    mov  ecx,63*65536+15
    mov  edx,3
    int  0x40

    mov  eax,8
    mov  ebx,200*65536+10
    mov  ecx,33*65536+10
    mov  edx,4
    int  0x40

    mov  esi,destination
    mov  edi,text+14
    mov  ecx,11
    cld
    rep  movsb

    mov  ebx,25*65536+35           ; draw info text with function 4
    mov  ecx,0x0
    mov  edx,text
    mov  esi,40
  newline:
    mov  eax,4
    int  0x40
    add  ebx,16*2
    add  edx,40
    cmp  [edx],byte 'x'
    jnz  newline


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


; DATA AREA

destination  db  'EXAMPLE.ASM'

addr  dd  0x0
ya    dd  0x0

text:
    db 'FILENAME   :  yyyyyyyyyyy               '
    db '     DELETE FROM RAMDISK                '

    db 'x' ; <- END MARKER, DONT DELETE


header:
    db   'RAMDISK FILE DELETE'
 .len = $ - header

I_END:
