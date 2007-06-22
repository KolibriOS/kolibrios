;
;    Ok, this is the sceleton that MENUET 0.01 understands.
;    Do not change the header bits for now. Compile with nasm.
;

include 'lang.inc'
include '..\..\..\macros.inc'

use32
 org	0x0
 db	'MENUET01'    ; header
 dd	0x01	      ; header version
 dd	START	      ; entry point
 dd	I_END	      ; image size
 dd	0x1000        ; required memory
 dd	0x1000        ; esp
 dd	0x0 , 0x0     ; I_Param , I_Path


START:

  red:
    call  draw_window

still:

    mov       eax,10                 ; redraw ?
    mcall

    cmp    eax,1
    jz     red
    cmp    eax,3
    jz     button
    jmp    still

  button:
    mov  eax,17
    mcall

    cmp  al,byte 0
    jnz  still

    cmp  ah,1
    jnz  noexit

    mov  eax,0xffffffff
    mcall

  noexit:

    cmp  ah,2
    jz   note1

    mov  eax,20   ; reset midi device
    mov  ebx,1
    mov  ecx,0
    mcall

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
    mcall
    popa
    mov  ebx,0
;    call noteout

    add  eax,3

    mov  ebx,100
    call noteout
    pusha
    mov  eax,5
    mov  ebx,8
    mcall
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
    mcall

    mov       eax,0                     ; define and draw window
    mov       ebx,20*65536+250
    mov       ecx,20*65536+120
    mov       edx,0x14ffffff
    mov       edi,title
    mcall

    mov       eax,8
    mov       ebx,10*65536+200          ; button start x & size
    mov       ecx,40 *65536+17          ; button start y & size
    mov       edx,2                     ; button number
    mov       esi,0x4060b0              ; button color
    mcall

    mov       ecx,60 *65536+17          ; button start y & size
    mov       edx,3                     ; button number
    mcall

    mov       eax,4
    mov       ebx,25*65536+45
    mov       ecx,dword 0xffffff
    mov       edx,buttont
    mov       esi,buttontlen-buttont
    mcall

    mov       ebx,25*65536+65
    mov       edx,buttont2
    mov       esi,buttontlen2-buttont2
    mcall

    mov       eax,12                    ; tell os about redraw end
    mov       ebx,2
    mcall

    popa
    ret


noteout:

    pusha

    push ebx
    push eax

    mov  eax,20
    mov  ebx,2
    mov  ecx,0x9f
    mcall
    mov  eax,20
    mov  ebx,2
    pop  ecx
    mcall
    mov  eax,20
    mov  ebx,2
    pop  ecx
    mcall

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
    mcall

    mov       eax,dword 4
    mov       ebx,15*65536+95
     mov       ecx,0x000000
     mov       edx,error2
    mov       esi,errorlen2-error2
    mcall

    ret


; DATA AREA


title    db   'MIDI TEST',0

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



