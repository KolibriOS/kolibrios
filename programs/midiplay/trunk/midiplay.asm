;
;    Ok, this is the sceleton that MENUET 0.01 understands.
;    Do not change the header bits for now. Compile with nasm.
;

include 'lang.inc'
include 'macros.inc'

use32

    org   0x0
    db    'MENUET00'     ; 8 byte id
    dd    38             ; required os
    dd    START          ; program start
    dd    I_END          ; program image size
    dd    0x1000         ; reguired amount of memory
    dd    0x1000
    dd    0x00000000     ; reserved=no extended header



START:

    call  draw_window

still:

    mov       eax,10                 ; redraw ?
    int       0x40

    cmp    eax,1
    jz     red
    cmp    eax,3
    jz     button
    jmp    still

  red:
    call   draw_window
    jmp    still

  button:
    mov  eax,17
    int  0x40

    cmp  al,byte 0
    jnz  still

    cmp  ah,1
    jnz  noexit

    mov  eax,0xffffffff
    int  0x40

  noexit:

    cmp  ah,2
    jz   note1

    mov  eax,20   ; reset midi device
    mov  ebx,1
    mov  ecx,0
    int  0x40

    cmp  eax,0
    jz   noe1

    call printerror

  noe1:

    jmp  still

  note1:

    mov  eax,50

  nn:

    mov  ebx,100
    call noteout
    pusha
    mov  eax,5
    mov  ebx,8
    int  0x40
    popa
    mov  ebx,0
;    call noteout

    add  eax,3

    mov  ebx,100
    call noteout
    pusha
    mov  eax,5
    mov  ebx,8
    int  0x40
    popa
    mov  ebx,0
;    call noteout

    add  eax,4

    inc  eax
    cmp  eax,90
    jbe  nn

    jmp  still


draw_window:

    pusha

    mov       eax,12                    ; tell os about redraw
    mov       ebx,1
    int       0x40

    mov       eax,0                     ; define and draw window
    mov       ebx,20*65536+250
    mov       ecx,20*65536+120
    mov       edx,0x02ffffff
    mov       esi,0x805070d0;88ccee
    mov       edi,0x005070d0;88ccee
    int       0x40

                                        ; CLOSE BUTTON
     mov       eax,8                     ; function 8 : define and draw
     mov       ebx,(250-19)*65536+12     ; [x start] *65536 + [x size]
    mov       ecx,5*65536+12            ; [y start] *65536 + [y size]
    mov       edx,1                     ; button id
    mov       esi,0x5577cc              ; button color RRGGBB
    int       0x40


    mov       eax,4                     ; 4 = write text
    mov       ebx,8*65536+8
    mov       ecx,dword 0x00ffffff      ; 8b window nro - RR GG BB color
    mov       edx,labelt                ; pointer to text beginning
    mov       esi,labellen-labelt       ; text length
    int       0x40

    mov       eax,8
    mov       ebx,10*65536+200          ; button start x & size
    mov       ecx,40 *65536+17          ; button start y & size
    mov       edx,2                     ; button number
    mov       esi,0x4060b0              ; button color
    int       0x40

    mov       eax,8
    mov       ebx,10*65536+200          ; button start x & size
    mov       ecx,60 *65536+17          ; button start y & size
    mov       edx,3                     ; button number
    mov       esi,0x4060b0              ; button color
    int       0x40

    mov       eax,dword 4
    mov       ebx,25*65536+45
    mov       ecx,dword 0xffffff
    mov       edx,buttont
    mov       esi,buttontlen-buttont
    int       0x40

    mov       eax,dword 4
    mov       ebx,25*65536+65
    mov       ecx,dword 0xffffff
    mov       edx,buttont2
    mov       esi,buttontlen2-buttont2
    int       0x40

    mov       eax,12                    ; tell os about redraw end
    mov       ebx,2
    int       0x40

    popa
    ret


noteout:

    pusha

    push ebx
    push eax

    mov  eax,20
    mov  ebx,2
    mov  ecx,0x9f
    int  0x40
    mov  eax,20
    mov  ebx,2
    pop  ecx
    int  0x40
    mov  eax,20
    mov  ebx,2
    pop  ecx
    int  0x40

    cmp  eax,0
    jz   noe2

    call printerror

  noe2:

    popa
    ret

printerror:

    mov       eax,dword 4
    mov       ebx,15*65536+85
     mov       ecx,0x000000
     mov       edx,error1
    mov       esi,errorlen1-error1
    int       0x40

    mov       eax,dword 4
    mov       ebx,15*65536+95
     mov       ecx,0x000000
     mov       edx,error2
    mov       esi,errorlen2-error2
    int       0x40

    ret


; DATA AREA


labelt:
    db   'MIDI TEST'
labellen:

buttont:
    db   'PLAY A FEW NOTES'
buttontlen:
buttont2:
    db   'RESET MIDI DEVICE'
buttontlen2:

error1:
    db   'NO BASE DEFINED FOR MPU-401'
errorlen1:

error2:
    db   'USE SETUP AND RESET MIDI DEVICE.'
errorlen2:

base db 0x0


I_END:



