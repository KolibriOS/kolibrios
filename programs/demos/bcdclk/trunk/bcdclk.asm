;
;   BCD CLOCK
;
;   Compile with FASM for Menuet
;
;

use32

 org 0x0

 db 'MENUET01'
 dd 0x01
 dd START
 dd I_END
 dd 0x1000
 dd 0x1000
 dd 0x0 , 0x0

include 'lang.inc'
include '../../../macros.inc'


START:
red:
     call drawwindow

still:


    mov  eax,23   ; wait for timeout
    mov  ebx,50
    mcall

    cmp  eax,1 ; redraw ?
    je red

    cmp  eax,3 ; button in buffer ?
    je button

    call  drawclock

    jmp still
 
button:
    mov  al,17   ; get id
    mcall

    cmp  ah,1 ; button id=1 ?
    jne  noclose
    or   eax,-1   ; close this program
    mcall
  noclose:

    jmp  still

drawclock:

    mov eax,3 ; get time
    mcall
    bswap eax
    shr  eax,8
    mov  edi,dg1
    mov  ecx,6
dgtomem:
    push eax
    and  al,15
    mov  [edi],al
    inc  edi
    pop  eax
    shr  eax,4
    loop dgtomem
    mov  ebx,74*65536+10
    mov  edi,dg1
digitlp:
    mov  ecx,10*65536+10
    xor  esi,esi
plotlp:
    xor  edx,edx
    test byte[edi],8
    je	 nobit
    mov  edx,0x00ff0000
nobit:
    mov  eax,13  ; plot 8,4,2,1
    mcall
    add  ecx,12*65536
    shl  byte[edi],1
    inc  esi
    cmp  esi,4
    jne  plotlp
    shr  byte[edi],4
    mov  edx,0x00880040
    mov  eax,13 ; draw digit box
    mcall
    pusha
    mov  edx,ebx
    and  edx,0xffff0000
    shr  ecx,16
    or	 edx,ecx
    add  edx,3*65536+2
    mov  ebx,0x00010100
    mov  ecx,[edi]
    mov  esi,0x00ffffff
    mov  eax,47  ; display decimal
    mcall
    popa
    sub  ebx,12*65536
    inc  edi
    cmp  edi,dg1+6
    jne  digitlp
    ret


drawwindow:


    mov  eax,12
    mov  ebx,1 ; start redraw
    mcall

    xor  eax,eax ; window
    mov  ebx,100*65536+107
    mov  ecx,100*65536+105
    mov  edx,0x34400088
    mov  edi,title
    mcall

call drawclock

    mov  eax,12
    mov  ebx,2 ; end redraw
    mcall

    ret

title   db   'BCD Clock',0
I_END:
dg1:   db  ?