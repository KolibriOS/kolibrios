;
;    INFRARED
;
;    Compile with FASM
;

use32

                org     0x0

                db      'MENUET01'              ; 8 byte id
                dd      1                       ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x1000                  ; required amount of memory
                dd      0x1000                  ; esp = 0x7FFF0
                dd      0, 0


include '..\..\..\..\macros.inc'

START:                          ; start of execution

set_variables:

    mov  eax,46           ; reserve ports 0x3f0 - 0x3ff
    mov  ebx,0
    mov  ecx,0x3f0
    mov  edx,0x3ff
    mcall

    mov  eax,45           ; reserve irq 4
    mov  ebx,0
    mov  ecx,4
    mcall

    mov  eax,44           ; set read ports for irq 4
    mov  ebx,irqtable
;    mov  ecx,4
    mcall

        mov     dh, 3     ; all ports have number 3xx hex

        mov     dl, 0xf3+8
        mov     al, 0x80
        out     dx, al

        mov     dl, 0xf1+8
        mov     al, 0
        out     dx, al

        mov     dl, 0xf0+8
        mov     al, 0x30 / 4
        out     dx, al

        mov     dl, 0xf3+8
        mov     al, 3
        out     dx, al

        mov     dl, 0xf4+8
        mov     al, 0xB
        out     dx, al

        mov     dl, 0xf1+8
        mov     al, 1
        out     dx, al

    mov  eax,5
    mov  ebx,100
    mcall

        mov     dl, 0xf8
        mov     al, 'I'
        out     dx, al

    mov  eax,5
    mov  ebx,10
    mcall

        mov     al, 'R'
        out     dx, al

    mov  eax,40                                 ; get com 1 data with irq 4
    mov  ebx,0000000000010000b shl 16 + 101b
    mcall

red:
        call    draw_window

still:

    mov  eax,10                 ; wait here for event
    mcall
        dec     eax
        jz      red
        dec     eax
        dec     eax
        jnz     readir

  button:                       ; button
    mov  al,17                  ; get id
    mcall

; we have only one button, close

    mov  eax,45                 ; free irq
    mov  ebx,1
    mov  ecx,4
    mcall

    mov  eax,46                 ; free ports 0x3f0-0x3ff
    mov  ebx,1
    mov  ecx,0x3f0
    mov  edx,0x3ff
    mcall

    or   eax,-1                 ; close this program
    mcall

pos dd 0x0

cdplayer:
        dd      7
        dd      0
        dd      0
        dd      0
        dd      0
        db      '/sys/CDP',0

  readir:
    mov  eax,42
    mov  ebx,4
    mcall

    cmp  ebx,80
    jne  nocd

    mov  eax,70
    mov  ebx,cdplayer
    mcall


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
    mcall
    mov  eax,10*19+1
  noeaxz:
    mov  [pos],eax
    pop  ebx
    and  ebx,0xff
    call draw_data
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



;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+300         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+250         ; [y start] *65536 + [y size]
    mov  edx,[wcolor]              ; color of work area RRGGBB,8->color
    mov  edi,labelt                ; caption string
    mcall

;                                   ; WINDOW LABEL
;    mov  eax,4                     ; function 4 : write text to window
;    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
;    mov  ecx,0x00ffffff            ; color of text RRGGBB
;    mov  edx,labelt                ; pointer to text beginning
;    mov  esi,labellen-labelt       ; text length
;    mcall

                                   ; CLOSE BUTTON
;    mov  eax,8                     ; function 8 : define and draw button
;    mov  ebx,(300-19)*65536+12     ; [x start] *65536 + [x size]
;    mov  ecx,5*65536+12            ; [y start] *65536 + [y size]
;    mov  edx,1                     ; button id
;    mov  esi,0x5599cc              ; button color RRGGBB
;    mcall

draw_text:

    mov  ebx,25*65536+35           ; draw info text with function 4
    mov  ecx,0xffffff
    mov  edx,text
    mov  esi,40
    mov  edi,20
  newline:
    mov  eax,4
    mcall
    add  ebx,10
    add  edx,esi
    dec  edi
    jne  newline

    mov  eax,12
    mov  ebx,2
    mcall

    ret


; DATA AREA

wcolor  dd  0x13000000

labelt  db  'INFRARED RECEIVER FOR IRMAN IN COM 1',0

text:

I_END:
