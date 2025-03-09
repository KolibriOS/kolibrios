;
;    Fire for Kolibri  - Compile with FASM
;

use32
	org     0
	db      'MENUET01'           ; 8 byte id
	dd      1                    ; header version
	dd      START                ; program start
	dd      image_end            ; image size
	dd      mem_end              ; reguired amount of memory
	dd      stacktop
	dd      0,0                  ; no parameters, no path

include '..\..\macros.inc'
include '..\..\KOSfuncs.inc'

Screen_W equ 480


START:

; ************************************************
; ********* WINDOW DEFINITIONS AND DRAW **********
; ************************************************

draw_window:

    mcall SF_REDRAW, SSF_BEGIN_DRAW

    mcall SF_CREATE_WINDOW,<100,Screen_W+1>,<70,222>,0x01000000,0,0
	
	mcall SF_DRAW_RECT,<0,Screen_W+1>,<0,30>,0
	mcall   ,<0,1>,<0,222>
	mcall   ,<Screen_W+1,1>,<0,223>
	mcall   ,<0,Screen_W+1>,<222,1>
	
	mcall SF_DRAW_TEXT,<(Screen_W-108)/2,8>,dword 0x00FFFFFF,text,textlen-text

	mcall SF_DEFINE_BUTTON,<(Screen_W+1-19),12>,<5,12>,1,0x009a0000

	mov  ebx,ecx ;5*65536+12
	inc  edx
	mcall

    inc  edx
    mcall ,<18,12>

    inc  edx
    mcall ,<31,12>

    mcall SF_REDRAW, SSF_END_DRAW

sta:                                         ; calculate fire image

    mov  esi, FireScreen.end+Screen_W*3-Screen_W/4
    mov  ecx, Screen_W/4
    mov  eax, [FireSeed]

  NEWLINE:

    mul  [RandSeedConst]
    inc  eax
    mov  [esi], dl
    inc  esi
    loop NEWLINE

    mov  [FireSeed], eax

    mov  ecx, (Screen_W*200)/8+Screen_W*3-Screen_W/4
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
    mov  dl, [esi + Screen_W/4]
    jmp  typedone
  notype1:

;    cmp  [type],1
;    jnz  notype2
    mov  dl, [esi - 2]
;    add  eax, edx
;    mov  dl, [esi - 2]
;    add  eax, edx
    lea  eax, [eax + edx*2]
    mov  dl, [esi + Screen_W/4-2]
;  notype2:

; type 2 is never used
;    cmp  [type],2
;    jnz  notype3
;    mov  dl, [esi - 2]
;    add  eax, edx
;    mov  dl, [esi]
;    add  eax, edx
;    mov  dl, [esi + Screen_W/4]
;  notype3:

typedone:
    add  eax, edx
    shr  eax, 2
    jz   ZERO
    dec  eax

  ZERO:

    mov  [esi - Screen_W/4-1], al
    loop FIRELOOP

    mcall SF_SLEEP,[delay]

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
    lea  ebx, [edi+Screen_W*3]
    mov  [ebx+0],ecx
    mov  [ebx+3],ecx
    mov  [ebx+6],ecx
    mov  [ebx+9],ecx

    add  edi,12
    inc  edx
    cmp  edx,Screen_W/4
    jnz  nnl
    xor  edx,edx
    add  edi,Screen_W*3
  nnl:
    cmp  esi,FireScreen.end
    jnz  newc

    pop  ebx
    mcall SF_PUT_IMAGE,,<Screen_W,200>,<1,22>

  nodrw:

    mcall SF_CHECK_EVENT         ; check if os wants to talk to us
    dec  eax
    jz   draw_window
    cmp  al, 3-1
    jnz  nob4

  button:                        ; get button id
    mcall SF_GET_BUTTON
    shr  eax, 8

    dec  eax
    jnz  @f
    mcall SF_TERMINATE_PROCESS   ; close this program
  @@:

    dec  eax                     ; change fire type
    jnz  @f
    xor  [type], 1
   @@:

    dec  eax                     ; change delay
    jnz  @f
    xor  [delay], 1
  @@:

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
    mov  ecx,(Screen_W*200*3)/4
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
delay     dd  1
FireSeed  dd  0x1234
text:     db 'Fire for Kolibri'
textlen:

align 4
image_end:

FireScreen:
	rb (Screen_W*200)/8 ;wisible fire
.end:
	rb Screen_W*3       ;hidden fire (need for generation)
ImageData:
	rb Screen_W*200*3

; stack
align 512
	rb 512
stacktop:
mem_end:
