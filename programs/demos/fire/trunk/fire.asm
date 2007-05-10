;
;    FIRE for MENUET  - Compile with FASM
;

use32

           org     0x0
           db      'MENUET01'           ; 8 byte id
           dd      1                    ; header version
           dd      START                ; program start
           dd      I_END                ; image size
           dd      mem_end              ; reguired amount of memory
           dd      mem_end
           dd      0,0                  ; no parameters, no path

include '..\..\..\macros.inc'

START:

red:

; ************************************************
; ********* WINDOW DEFINITIONS AND DRAW **********
; ************************************************

draw_window:

    mov  eax,12                    ; tell os about redraw
    mov  ebx,1
    mcall

    xor  eax,eax                   ; define and draw window
    mov  ebx,100*65536+321
    mov  ecx,70*65536+222
    mov  edx,0x00000000
    mov  esi,0x00000000
    mov  edi,0x00000000
    mcall

    mov  al,4      ; 'FIRE FOR MENUET'
    mov  ebx,110*65536+8
    mov  ecx,dword 0x00FFFFFF
    mov  edx,text
    mov  esi,textlen-text
    mcall

    mov  al,8
    mov  ebx,(321-19)*65536+12     ; button start x & size
    mov  ecx,5*65536+12            ; button start y & size
    mov  edx,1                     ; button number
    mov  esi,0x009a0000
    mcall

    mov  ebx,ecx ;5*65536+12
    inc  edx
    mcall

    mov  ebx,18*65536+12
    inc  edx
    mcall

    mov  ebx,31*65536+12
    inc  edx
    mcall

    mov  al,12                    ; tell os about redraw end
    mov  ebx,2
    mcall

sta:                                         ; calculate fire image

    mov  esi, FireScreen+0x2300-80
    mov  ecx, 80
    mov  eax, [FireSeed]

  NEWLINE:

    mul  [RandSeedConst]
    inc  eax
    mov  [esi], dl
    inc  esi
    loop NEWLINE

    mov  [FireSeed], eax

    mov  ecx, 0x2300-80
    sub  esi, ecx
    xor  edx, edx
    xor  eax, eax

  FIRELOOP:
    lodsb

    cmp  [type], ah
    jnz  notype1
    mov  dl, [esi + 1]
    add  eax, edx
    mov  dl, [esi]
    add  eax, edx
    mov  dl, [esi + 80]
    jmp  typedone
  notype1:

;    cmp  [type],1
;    jnz  notype2
    mov  dl, [esi - 2]
;    add  eax, edx
;    mov  dl, [esi - 2]
;    add  eax, edx
    lea  eax, [eax + edx*2]
    mov  dl, [esi + 78]
;  notype2:

; type 2 is never used
;    cmp  [type],2
;    jnz  notype3
;    mov  dl, [esi - 2]
;    add  eax, edx
;    mov  dl, [esi]
;    add  eax, edx
;    mov  dl, [esi + 80]
;  notype3:

typedone:
    add  eax, edx
    shr  eax, 2
    jz   ZERO
    dec  eax

  ZERO:

    mov  [esi - 81], al
    loop FIRELOOP

    mov  al, 5              ; in this moment always high 24 bits of eax are zero!
    mov  ebx,[delay]
    mcall

    inc  [calc]
    cmp  [calc], byte 2
    jnz  nodrw

  pdraw:

    mov  byte [calc],ah ;byte 0

    mov  edi,ImageData
    push edi     ; pointer for sysfunction 7, draw image
    add  edi,[fcolor]
    mov  esi,FireScreen
    xor  edx,edx

  newc:
    xor   eax, eax
    lodsb

    mov   ecx,eax
    shr   eax,1
    add   cl,al
    mov   ch,al

    mov  [edi+0],ecx
    mov  [edi+3],ecx
    mov  [edi+6],ecx
    mov  [edi+9],cx
    lea  ebx, [edi+320*3]
    mov  [ebx+0],ecx
    mov  [ebx+3],ecx
    mov  [ebx+6],ecx
    mov  [ebx+9],ecx

    add  edi,12
    inc  edx
    cmp  edx,80
    jnz  nnl
    xor  edx,edx
    add  edi,320*3
  nnl:
    cmp  esi,FireScreen+8000; 0x2000
    jnz  newc

    mov  al,7           ; display image
                        ; high 24 bits of eax are zero!
    pop  ebx
    mov  ecx,4*80*65536+200
    mov  edx,1*65536+22
    mcall

  nodrw:

    mov  eax,11                  ; check if os wants to talk to us
    mcall
    dec  eax
    jz   red
    cmp  al, 3-1
    jnz  nob4

  button:                        ; get button id
    mov  al,17
    mcall
    shr  eax, 8

    dec  eax
    jnz  noclose
    or   eax,-1                  ; close this program
    mcall
  noclose:

    dec  eax                     ; change fire type
    jnz  nob2
    xor  [type], 1
   nob2:

    dec  eax                     ; change delay
    jnz  nob3
    xor  [delay], 1
  nob3:

    dec  eax                     ; change color
    jnz  nob4
    mov  eax,[fcolor]
    inc  eax
;    cmp  al,2
;    jbe  fcok
    jnp  fcok
    xor  eax,eax
  fcok:
    mov  [fcolor],eax
    mov  edi,ImageData
    mov  ecx,(320*600)/4
    xor  eax,eax
    rep  stosd

  nob4:
    jmp  sta

; DATA SECTION
RandSeedConst dd 0x8405
fcolor    dd  2
xx        db  'x'
type      db  0
calc      db  0
delay     dd  0
FireSeed  dd  0x1234
text:     db 'FIRE FOR MENUET'
textlen:

I_END:

FireScreen:
          rb 0x2300
ImageData:
          rb 320*200*3

; stack
          align 512
          rb 512
mem_end:
