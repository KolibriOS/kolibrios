;
;    FIRE for MENUET  - Compile with FASM
;

use32

           org     0x0
           db      'MENUET00'           ; 8 byte id
           dd      38                   ; required os
           dd      START                ; program start
           dd      I_END                ; image size
           dd      0x80000               ; reguired amount of memory
           dd      0x80000
           dd      0x00000000           ; reserved=no extended header

include 'lang.inc'
include 'macros.inc'

START:

    call draw_window

    mov  edi,0x40000
    mov  ecx,320*600+100
    mov  eax,0
    cld
    rep  stosb

sta:                                         ; calculate fire image

    mov  esi, FireScreen
    add  esi, 0x2300
    sub  esi, 80
    mov  ecx, 80
    xor  edx, edx

  NEWLINE:

    mov  eax,dword [FireSeed]                ; New number
    mov  edx, 0x8405
    mul  edx
    inc  eax
    mov  dword [FireSeed], eax               ; Store seed

    mov  [esi], dl
    inc  esi
    dec  ecx
    jnz  NEWLINE

    mov  ecx, 0x2300
    sub  ecx, 80
    mov  esi, FireScreen
    add  esi, 80

  FIRELOOP:

    xor  eax,eax

    cmp  [type],0
    jnz  notype1
    mov  al, [esi]
    add  al, [esi + 2]
    adc  ah, 0
    add  al, [esi + 1]
    adc  ah, 0
    add  al, [esi + 81]
    adc  ah, 0
  notype1:

    cmp  [type],1
    jnz  notype2
    mov  al, [esi]
    add  al, [esi - 1]
    adc  ah, 0
    add  al, [esi - 1]
    adc  ah, 0
    add  al, [esi + 79]
    adc  ah,0
  notype2:

    cmp  [type],2
    jnz  notype3
    mov  al, [esi]
    add  al, [esi - 1]
    adc  ah,0
    add  al, [esi + 1]
    adc  ah, 0
    add  al, [esi + 81]
    adc  ah,0
  notype3:

    shr  eax, 2
    jz   ZERO
    dec  eax

  ZERO:

    mov  [esi - 80], al
    inc  esi
    dec  ecx
    jnz  FIRELOOP

    pusha

    mov  eax,5
    mov  ebx,[delay]
    int  0x40

    mov  al,byte [calc]
    inc  al
    mov  byte [calc],al
    cmp  al,byte 2
    jz   pdraw

    jmp  nodrw

  pdraw:

    mov  byte [calc],byte 0

    mov  edi,0x40000
    add  edi,[fcolor]
    mov  esi,FireScreen
    xor  edx,edx

  newc:

    movzx eax,byte [esi]
    mov   ebx,eax
    mov   ecx,eax
    shl   ax,8
    shr   bx,1
    mov   al,bl
    add   ecx,eax
    shl   ax,8
    mov   ch,ah

    mov  [edi+0],cx
    mov  [edi+3],cx
    mov  [edi+6],cx
    mov  [edi+9],cx
    mov  [edi+0+320*3],cx
    mov  [edi+3+320*3],cx
    mov  [edi+6+320*3],cx
    mov  [edi+9+320*3],cx

    add  edi,12
    inc  edx
    cmp  edx,80
    jnz  nnl
    xor  edx,edx
    add  edi,320*3
  nnl:
    inc  esi
    cmp  esi,FireScreen+0x2000
    jnz  newc

    mov  eax,dword 0x00000007           ; display image
    mov  ebx,0x40000
    mov  ecx,4*80*65536+200
    mov  edx,1*65536+22
    int  0x40

  nodrw:

    popa

    mov  eax,11                  ; check if os wants to talk to us
    int  0x40
    cmp  eax,1
    jz   red
    cmp  eax,3
    jz   button

    jmp  sta

  red:                           ; draw window
    call draw_window
    jmp  sta

  button:                        ; get button id
    mov  eax,17
    int  0x40

    cmp  ah,1
    jnz  noclose
    mov  eax,-1                  ; close this program
    int  0x40
  noclose:

    cmp  ah,2                    ; change fire type
    jnz  nob2
    mov  eax,[type]
    add  eax,1
    and  eax,1
    mov  [type],eax
   nob2:

    cmp  ah,3                    ; change delay
    jnz  nob3
    mov  eax,[delay]
    sub  eax,1
    and  eax,1
    mov  [delay],eax
  nob3:

    cmp  ah,4                    ; change color
    jnz  nob4
    mov  eax,[fcolor]
    add  eax,1
    cmp  eax,2
    jbe  fcok
    mov  eax,0
  fcok:
    mov  [fcolor],eax
    mov  eax,0
    mov  ecx,0x10000
    mov  edi,0x40000
    cld
    rep  stosd

  nob4:

    jmp  sta



; ************************************************
; ********* WINDOW DEFINITIONS AND DRAW **********
; ************************************************


draw_window:

    pusha

    mov  eax,12                    ; tell os about redraw
    mov  ebx,1
    int  0x40

    mov  eax,0                     ; define and draw window
    mov  ebx,100*65536+321
    mov  ecx,70*65536+222
    mov  edx,0x00000000
    mov  esi,0x00000000
    mov  edi,0x00000000
    int  0x40

    mov  eax,dword 0x00000004      ; 'FIRE FOR MENUET'
    mov  ebx,110*65536+8
    mov  ecx,dword 0x00FFFFFF
    mov  edx,text
    mov  esi,textlen-text
    int  0x40

    mov  eax,8
    mov  ebx,(321-19)*65536+12     ; button start x & size
    mov  ecx,5*65536+12            ; button start y & size
    mov  edx,1                     ; button number
    mov  esi,0x009a0000
    int  0x40

    mov  eax,8
    mov  ebx,5*65536+12
    mov  ecx,5*65536+12
    mov  edx,2
    int  0x40

    mov  eax,8
    mov  ebx,18*65536+12
    mov  ecx,5*65536+12
    mov  edx,3
    int  0x40

    mov  eax,8
    mov  ebx,31*65536+12
    mov  ecx,5*65536+12
    mov  edx,4
    int  0x40

    mov  eax,12                    ; tell os about redraw end
    mov  ebx,2
    int  0x40

    popa

    ret


; DATA SECTION

calc      dd  0
fcolor    dd  2
xx        db  'x'
type      dd  0
delay     dd  0
FireSeed  dd  0x1234
text:     db 'FIRE FOR MENUET'
textlen:

FireScreen:

I_END:



