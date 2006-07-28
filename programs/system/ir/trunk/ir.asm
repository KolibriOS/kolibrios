;
;    INFRARED
;
;    Compile with FASM for Menuet
;

use32

                org     0x0

                db      'MENUET00'              ; 8 byte id
                dd      56                      ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x1000                  ; required amount of memory
                dd      0x1000                  ; esp = 0x7FFF0
                dd      0x00000000              ; reserved=no extended header


include 'lang.inc'
include 'macros.inc'

START:                          ; start of execution

    call draw_window            ; at first, draw the window

    call set_variables

still:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button
    cmp  eax,16+4
    je   readir

    jmp  still

pos dd 0x0

cdplayer db  'CDPLAY     '

  readir:
    mov  eax,42
    mov  ebx,4
    int  0x40

    cmp  ebx,80
    jne  nocd

    mov  eax,19
    mov  ebx,cdplayer
    mov  ecx,0
    int  0x40


  nocd:

    push ebx
    mov  eax,[pos]
    add  eax,1
    cmp  eax,10*20+1
    jb   noeaxz
    mov  esi,text+10*4
    mov  edi,text
    mov  ecx,10*21*4
    cld
    rep  movsb
    mov  eax,13
    mov  ebx,20*65536+260
    mov  ecx,22*65536+220
    mov  edx,[wcolor]
    int  0x40
    mov  eax,10*19+1
  noeaxz:
    mov  [pos],eax
    pop  ebx
    and  ebx,0xff
    call draw_data
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
    jne  noclose

    mov  eax,45                 ; free irq
    mov  ebx,1
    mov  ecx,4
    int  0x40

    mov  eax,46                 ; free ports 0x3f0-0x3ff
    mov  ebx,1
    mov  ecx,0x3f0
    mov  edx,0x3ff
    int  0x40

    mov  eax,-1                 ; close this program
    int  0x40
  noclose:

    jmp  still



draw_data:

    pusha

    xchg eax,ebx

    mov  ecx,10
    shl  ebx,2
    mov  esi,3
  newnum:
    xor  edx,edx
    div  ecx
    add  edx,48
    mov  [ebx+text-1],dl
    dec  ebx
    dec  esi
    jnz  newnum

    call draw_text

    popa

    ret


irqtable:

    dd  0x3f8+0x01000000   ;  +  01 = read byte,  02 read word
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0
    dd  0



set_variables:


    pusha

    mov  eax,46           ; reserve ports 0x3f0 - 0x3ff
    mov  ebx,0
    mov  ecx,0x3f0
    mov  edx,0x3ff
    int  0x40

    mov  eax,45           ; reserve irq 4
    mov  ebx,0
    mov  ecx,4
    int  0x40

    mov  eax,46           ; reserve ports 0x3f0-0x3ff
    mov  ebx,0
    mov  ecx,0x3f0
    mov  edx,0x3ff
    int  0x40

    mov  eax,44           ; set read ports for irq 4
    mov  ebx,irqtable
    mov  ecx,4
    int  0x40

    mov  cx,0x3f3+8
    mov  bl,0x80
    mov  eax,43
    int  0x40

    mov  cx,0x3f1+8
    mov  bl,0
    mov  eax,43
    int  0x40

    mov  cx,0x3f0+8
    mov  bl,0x30 / 4
    mov  eax,43
    int  0x40

    mov  cx,0x3f3+8
    mov  bl,3
    mov  eax,43
    int  0x40

    mov  cx,0x3f4+8
    mov  bl,0xB
    mov  eax,43
    int  0x40

    mov  cx,0x3f1+8
    mov  bl,1
    mov  eax,43
    int  0x40

    mov  eax,5
    mov  ebx,100
    int  0x40

    mov  cx,0x3f8
    mov  bl,'I'
    mov  eax,43
    int  0x40

    mov  eax,5
    mov  ebx,10
    int  0x40

    mov  cx,0x3f8
    mov  bl,'R'
    mov  eax,43
    int  0x40

    mov  eax,40                                 ; get com 1 data with irq 4
    mov  ebx,0000000000010000b shl 16 + 111b
    int  0x40

    popa

    ret


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+300         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+250         ; [y start] *65536 + [y size]
    mov  edx,[wcolor]              ; color of work area RRGGBB,8->color
    mov  esi,0x8099bbff            ; color of grab bar  RRGGBB,8->color glide
    mov  edi,0x00ffffff            ; color of frames    RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

                                   ; CLOSE BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,(300-19)*65536+12     ; [x start] *65536 + [x size]
    mov  ecx,5*65536+12            ; [y start] *65536 + [y size]
    mov  edx,1                     ; button id
    mov  esi,0x5599cc              ; button color RRGGBB
;    int  0x40

    call draw_text

    mov  eax,12
    mov  ebx,2
    int  0x40

    ret

draw_text:

    pusha

    mov  ebx,25*65536+35           ; draw info text with function 4
    mov  ecx,0xffffff
    mov  edx,text
    mov  esi,40
    mov  edi,20
  newline:
    mov  eax,4
    int  0x40
    add  ebx,10
    add  edx,40
    dec  edi
    jne  newline

    popa

    ret



; DATA AREA

wcolor  dd  0x03000000

labelt  db  'INFRARED RECEIVER FOR IRMAN IN COM 1'
labellen:

text:

I_END:




